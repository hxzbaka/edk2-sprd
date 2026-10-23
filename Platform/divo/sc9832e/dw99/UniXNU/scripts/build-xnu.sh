#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-2-Clause-Patent
set -euo pipefail

DW99_TREE=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
exec python3 - "$DW99_TREE" "$@" <<'PY'
import argparse
import contextlib
import json
import os
import plistlib
import secrets
import shlex
import shutil
import struct
import subprocess
import sys
import tempfile
import tomllib
import zipfile
from pathlib import Path

parser = argparse.ArgumentParser(description="构建 DW99 的 XNU 源码内核及平台驱动库")
parser.add_argument("tree", type=Path)
parser.add_argument("--config", type=Path)
mode = parser.add_mutually_exclusive_group()
mode.add_argument("--prepare", action="store_true")
mode.add_argument("--extract-stock", type=Path, metavar="IPSW")
mode.add_argument("--prelink", action="store_true")
mode.add_argument("--payload", action="store_true")
parser.add_argument("--unique-chip-id", type=lambda value: int(value, 16), metavar="HEX")
parser.add_argument("--out", type=Path)
args = parser.parse_args()
if args.unique_chip_id is not None and not args.payload:
    parser.error("--unique-chip-id 只能与 --payload 一起使用")
if args.unique_chip_id is not None and not 0 <= args.unique_chip_id < 1 << 64:
    parser.error("--unique-chip-id 必须是 64 位无符号十六进制整数")
tree = args.tree.resolve()
config_file = args.config or tree / "scripts/build-xnu.toml"
config = tomllib.loads(config_file.read_text())
if args.prepare:
    board = tomllib.loads((tree / "board.toml").read_text())
    build = board["build"]
    repository = Path(subprocess.check_output(
        ["git", "-C", str(tree), "rev-parse", "--show-toplevel"], text=True
    ).strip())
    upstream = repository / build["upstream"]
    subprocess.run(["git", "-C", str(repository), "submodule", "update", "--init", build["upstream"]], check=True, stdout=sys.stderr)
    subprocess.run(["git", "-C", str(upstream), "submodule", "update", "--init", *build["submodules"]], check=True, stdout=sys.stderr)
    sys.path.insert(0, str(upstream / build["python_modules"]))
    from unixnu.config import Context, KINDS
    from unixnu import feeds
    from unixnu.util import Error

    options = argparse.Namespace(
        device=str(tree), out=str((args.out or repository / build["output"]).resolve()),
        jobs=None, local=None, local_module=None,
    )
    with contextlib.redirect_stdout(sys.stderr):
        try:
            context = Context(options)
            required = {("package", "main")}
            required.update(("refer", name) for name in config["references"])
            required.update(("driver", Path(driver["source"]).parts[1]) for driver in config["driver"])
            required.update(("patch", name) for name in build["patch_modules"])
            module_kinds = {folder: kind for kind, folder in KINDS.items()}
            required.update((module_kinds[Path(path).parts[0]], Path(path).name) for path in build["submodules"])
            resolved = feeds.resolve(context)
            missing = required - resolved.keys()
            if missing:
                raise SystemExit(f"缺少构建模块：{sorted(missing)}")
            modules = {key: resolved[key] for key in sorted(required)}
            feeds.fetch(context, modules, update=True)
        except Error as error:
            raise SystemExit(str(error)) from error
    context.tree.mkdir(parents=True, exist_ok=True)
    for (kind, name), module in modules.items():
        destination = context.tree / KINDS[kind]
        if kind != "package":
            destination /= name
        destination.parent.mkdir(parents=True, exist_ok=True)
        if destination.is_symlink():
            destination.unlink()
        elif destination.exists():
            raise SystemExit(f"构建模块路径已被非链接文件占用：{destination}")
        destination.symlink_to(module.path.resolve(), target_is_directory=True)
    scripts = context.tree / "scripts"
    scripts.mkdir(exist_ok=True)
    for name in build["scripts"]:
        shutil.copy2(tree / "scripts" / name, scripts / name)
    python_package = scripts / "unixnu"
    if python_package.is_symlink():
        python_package.unlink()
    elif python_package.exists():
        raise SystemExit(f"Python 模块路径已被非链接文件占用：{python_package}")
    python_package.symlink_to((upstream / build["python_modules"] / "unixnu").resolve(), target_is_directory=True)
    shutil.copy2(tree / "board.toml", context.tree / "board.toml")
    shutil.copy2(tree / "configs" / board["kernel"]["defconfig"], context.config_path)
    print(json.dumps({"phase": "prepared", "tree": str(context.tree), "config": str(context.config_path)}))
    raise SystemExit(0)
if args.out is not None:
    parser.error("--out 只能与 --prepare 一起使用")
paths = {name: (tree / value).resolve() for name, value in config["paths"].items()}
if args.extract_stock is not None:
    board = tomllib.loads((tree / "board.toml").read_text())
    if not (tree / "scripts" / "unixnu").is_dir():
        raise SystemExit("缺少 UniXNU Python 模块，请先运行 --prepare")
    sys.path.insert(0, str(tree / "scripts"))
    from unixnu.ipsw import im4p_payload
    from unixnu.util import Error

    spec = board["ipsw"]
    with zipfile.ZipFile(args.extract_stock) as archive:
        manifest = plistlib.loads(archive.read("BuildManifest.plist"))
        if ((manifest["ProductVersion"], manifest["ProductBuildVersion"]) != (spec["version"], spec["build"])
                or spec["product"] not in manifest["SupportedProductTypes"]):
            raise SystemExit("IPSW 版本、构建号或产品类型与 board.toml 不符")
        identities = [item for item in manifest["BuildIdentities"]
                      if item["Info"]["DeviceClass"] == spec["device_class"]
                      and item["Info"]["RestoreBehavior"] == "Erase"
                      and int(item["ApChipID"], 16) == spec["chip_id"]
                      and int(item["ApBoardID"], 16) == spec["board_id"]]
        if len(identities) != 1:
            raise SystemExit(f"IPSW 必须有唯一匹配的 Erase BuildIdentity，实际为 {len(identities)}")
        member = identities[0]["Manifest"]["KernelCache"]["Info"]["Path"]
        try:
            payload = im4p_payload(archive.read(member))
            ramdisk_member = identities[0]["Manifest"]["RestoreRamDisk"]["Info"]["Path"]
            ramdisk = im4p_payload(archive.read(ramdisk_member))
            trust_member = identities[0]["Manifest"]["RestoreTrustCache"]["Info"]["Path"]
            trust_cache = im4p_payload(archive.read(trust_member))
        except Error as error:
            raise SystemExit(str(error)) from error
    if len(ramdisk) < 1536 or struct.unpack_from(">HH", ramdisk, 1024) not in ((0x482b, 4), (0x4858, 5)):
        raise SystemExit("恢复 RAM disk 不是原始 HFS+/HFSX 卷")
    block_size, block_count = struct.unpack_from(">II", ramdisk, 1064)
    if block_size < 512 or block_size & (block_size - 1) or not block_count or block_size * block_count > len(ramdisk):
        raise SystemExit("恢复 RAM disk 的卷尺寸无效")
    header_size = 384
    if len(payload) < header_size or payload[:8] != b"complzss":
        raise SystemExit("KernelCache 仅支持带完整 384 字节头的 complzss 格式")
    expected_size, compressed_size = struct.unpack_from(">II", payload, 12)
    if not expected_size or not compressed_size or header_size + compressed_size > len(payload):
        raise SystemExit("complzss 头中的压缩或解压长度无效")
    packed = memoryview(payload)[header_size:header_size + compressed_size]
    ring = bytearray(b" " * 4096)
    ring_cursor = 4096 - 18
    cursor = flags = 0
    output = bytearray()
    # Okumura LZSS uses low-bit-first flags and a 4 KiB history ring.
    while cursor < len(packed):
        flags >>= 1
        if not flags & 0x100:
            flags = packed[cursor] | 0xff00
            cursor += 1
        if flags & 1:
            if cursor == len(packed):
                raise SystemExit("LZSS 字面量被截断")
            if len(output) == expected_size:
                raise SystemExit("LZSS 输出超过头中声明的长度")
            value = packed[cursor]
            cursor += 1
            output.append(value)
            ring[ring_cursor] = value
            ring_cursor = (ring_cursor + 1) & 4095
        else:
            if cursor + 2 > len(packed):
                raise SystemExit("LZSS 回溯引用被截断")
            low, high = packed[cursor:cursor + 2]
            cursor += 2
            offset = low | ((high & 0xf0) << 4)
            length = (high & 0x0f) + 3
            if len(output) + length > expected_size:
                raise SystemExit("LZSS 输出超过头中声明的长度")
            # Matches can overlap bytes written by this same token.
            for index in range(length):
                value = ring[(offset + index) & 4095]
                output.append(value)
                ring[ring_cursor] = value
                ring_cursor = (ring_cursor + 1) & 4095
    if len(output) != expected_size:
        raise SystemExit(f"LZSS 输出长度不符：预期 {expected_size}，实际 {len(output)}")
    if len(output) < 32 or struct.unpack_from("<4I", output) != (0xfeedfacf, 0x100000c, 0, 2):
        raise SystemExit("解压结果不是普通 arm64 MH_EXECUTE Mach-O")
    destination = paths["stock_kernelcache"]
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(output)
    paths["stock_ramdisk"].parent.mkdir(parents=True, exist_ok=True)
    paths["stock_ramdisk"].write_bytes(ramdisk)
    paths["stock_trust_cache"].parent.mkdir(parents=True, exist_ok=True)
    paths["stock_trust_cache"].write_bytes(trust_cache)
    print(json.dumps({"phase": "stock-extracted", "kernelcache": str(destination),
                      "member": member, "compression": "complzss",
                      "compressed_bytes": compressed_size, "decompressed_bytes": len(output),
                      "ramdisk": str(paths["stock_ramdisk"]), "ramdisk_bytes": len(ramdisk),
                      "trust_cache": str(paths["stock_trust_cache"])}))
    raise SystemExit(0)
if args.prelink:
    for name in ("kernel", "stock_kernelcache"):
        if not paths[name].is_file():
            raise SystemExit(f"预链接输入不存在：{paths[name]}")
    prelink = config["prelink"]
    source_work = paths["prelink_source"]
    if not source_work.is_relative_to(paths["output"]) or source_work == paths["output"]:
        raise SystemExit("预链接工具工作目录必须位于配置的构建输出目录内")
    with tempfile.TemporaryDirectory(prefix="dw99-prelink-") as temporary:
        prepared = Path(temporary) / "source"
        shutil.copytree(tree / prelink["source"], prepared, ignore=shutil.ignore_patterns(".git", "__pycache__"))
        for patch in prelink["patches"]:
            subprocess.run(["git", "apply", "--check", tree / patch], cwd=prepared, check=True, stdout=sys.stderr)
            subprocess.run(["git", "apply", tree / patch], cwd=prepared, check=True, stdout=sys.stderr)
        if source_work.exists():
            shutil.rmtree(source_work)
        shutil.copytree(prepared, source_work)
    prelink_values = {**paths, **prelink}
    command = [value.format(**prelink_values) for value in prelink["command"]]
    for identifier in prelink["roots"]:
        command.extend(["--root", identifier])
    for identifier, address in prelink["early_security_initializers"].items():
        command.extend(["--early-security-init", f"{identifier}={address}"])
    subprocess.run(command, check=True, stdout=sys.stderr)
    print(json.dumps({"phase": "prelinked", "kernel": str(paths["prelinked_kernel"])}))
    raise SystemExit(0)
if args.payload:
    board = tomllib.loads((tree / "board.toml").read_text())
    spec, payload = board["payload"], config["payload"]
    missing = [name for name in payload["required_tools"] if shutil.which(name) is None]
    if missing:
        raise SystemExit("缺少载荷打包工具：" + ", ".join(missing))
    if not paths["prelinked_kernel"].is_file():
        raise SystemExit(f"缺少预链接内核：{paths['prelinked_kernel']}")
    if not paths["stock_ramdisk"].is_file():
        raise SystemExit(f"缺少恢复 RAM disk：{paths['stock_ramdisk']}，请先运行 --extract-stock")
    trust_cache = paths["stock_trust_cache"].read_bytes()
    if len(trust_cache) < 24 or struct.unpack_from("<I", trust_cache)[0] != 1:
        raise SystemExit("恢复 trust cache 必须使用原厂 v1 格式")
    entries = struct.unpack_from("<I", trust_cache, 20)[0]
    if len(trust_cache) != 24 + entries * 22:
        raise SystemExit("恢复 trust cache 的条目长度无效")
    previous = None
    for index in range(entries):
        offset = 24 + index * 22
        cdhash = trust_cache[offset:offset + 20]
        if previous is not None and previous > cdhash:
            raise SystemExit("恢复 trust cache 的条目未按 cdhash 排序")
        previous = cdhash
    for name in ("payload_stage", "payload_image", "payload_report", "platform_seed"):
        if not paths[name].is_relative_to(paths["output"]) or paths[name] == paths["output"]:
            raise SystemExit(f"载荷输出必须位于配置的构建输出目录内：{paths[name]}")
    arguments = spec["kernel_args"]
    if any(not isinstance(value, str) or "\0" in value for value in arguments):
        raise SystemExit("内核参数必须是不含 NUL 的字符串")
    line = " ".join(arguments)
    if not line.isascii():
        raise SystemExit("DarwinBoot 内核命令行只支持 ASCII")
    if len(line.encode()) >= payload["command_line_size"]:
        raise SystemExit(f"内核命令行必须小于 {payload['command_line_size']} 字节")
    size = spec["size_mib"]
    if not isinstance(size, int) or size <= 0:
        raise SystemExit("payload.size_mib 必须是正整数")
    stage = paths["payload_stage"]
    seed_path = paths["platform_seed"]
    if seed_path.is_relative_to(stage):
        raise SystemExit("平台 seed 不能保存在会重建的 staging 目录内")
    seed = args.unique_chip_id
    if seed is None:
        seed = int(seed_path.read_text().strip(), 16) if seed_path.exists() else secrets.randbits(64)
    if not 0 <= seed < 1 << 64:
        raise SystemExit("保存的平台 seed 不是 64 位无符号整数")
    seed_path.parent.mkdir(parents=True, exist_ok=True)
    seed_path.write_text(f"{seed:016x}\n")
    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True)
    destinations = {}
    for name in ("kernel_path", "adt_path", "ramdisk_path", "trust_cache_path", "menu_path"):
        destination = (stage / payload[name]).resolve()
        if not destination.is_relative_to(stage) or destination == stage:
            raise SystemExit(f"载荷文件必须位于 staging 目录内：{destination}")
        destination.parent.mkdir(parents=True, exist_ok=True)
        destinations[name] = destination
    shutil.copyfile(paths["prelinked_kernel"], destinations["kernel_path"])
    shutil.copyfile(paths["stock_ramdisk"], destinations["ramdisk_path"])
    destinations["trust_cache_path"].write_bytes(struct.pack("<II", 1, 8) + trust_cache)
    adt = spec["adt"]
    command = ["python3", str(tree / adt["tool"]), "--output", str(destinations["adt_path"]),
               "--dram-size", str(spec["dram_size"]), "--unique-chip-id", f"{seed:016x}", "--data-ramdisk", "--trust-cache"]
    for key, flag in (("owned_memory_size", "--owned-memory-size"), ("chip_id", "--chip-id"),
                      ("cpu_regs", "--cpu-regs"), ("psci_method", "--psci-method")):
        if key in adt:
            command.extend([flag, str(adt[key])])
    subprocess.run(command, cwd=tree, check=True, stdout=sys.stderr)
    menu = payload["menu"].format(kernel_args=json.dumps(line, ensure_ascii=False))
    destinations["menu_path"].write_text(menu)
    content_bytes = sum(path.stat().st_size for path in stage.rglob("*") if path.is_file())
    if content_bytes > size * 1024 * 1024 * payload["capacity_fraction"]:
        raise SystemExit("载荷文件超过 FAT 镜像容量预算，请调整 board.toml 的 payload.size_mib")
    image = paths["payload_image"]
    image.parent.mkdir(parents=True, exist_ok=True)
    image.unlink(missing_ok=True)
    values = {**paths, "sectors": size * 2048, "volume": spec["volume"]}
    for name in ("format_command", "copy_command"):
        command = [value.format(**values) for value in payload[name]]
        subprocess.run(command, cwd=tree, check=True, stdout=sys.stderr)
    report = {"phase": "payload", "image": str(image), "stage": str(stage),
              "bytes": image.stat().st_size, "content_bytes": content_bytes,
              "kernel_args": line, "platform_seed": str(seed_path), "hardware_tested": False}
    paths["payload_report"].parent.mkdir(parents=True, exist_ok=True)
    paths["payload_report"].write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report))
    raise SystemExit(0)
jobs = os.environ.get(config["jobs_environment"], str(os.cpu_count() or 1))
tools = {name: shutil.which(name) for name in config["required_tools"]}
missing_tools = [name for name, path in tools.items() if path is None]
if missing_tools:
    raise SystemExit("缺少构建工具：" + ", ".join(missing_tools))
compiler_resource = subprocess.check_output(config["profile_runtime"]["resource_directory_command"], text=True).strip()
paths["profile_runtime"] = Path(config["profile_runtime"]["archive"].format(compiler_resource=compiler_resource))
if not paths["profile_runtime"].is_file():
    raise SystemExit(f"缺少 LLVM profile runtime：{paths['profile_runtime']}")
paths["platform_sdk"] = Path(subprocess.check_output(config["platform_sdk"]["path_command"], text=True).strip())
archives = []


def run(command, **options):
    print(shlex.join(map(str, command)), file=sys.stderr)
    subprocess.run(list(map(str, command)), check=True, stdout=sys.stderr, **options)


def compile_from_record(record, source, destination, remove_options=(), remove_flags=()):
    command = []
    arguments = record["arguments"]
    index = 0
    while index < len(arguments):
        value = arguments[index]
        if value in remove_options:
            index += 2
            continue
        if value in remove_flags or any(value.startswith(option) for option in remove_options if option in ("-D", "-I")):
            index += 1
            continue
        if value in ("-MF", "-MJ", "-MT", "-MQ"):
            index += 2
            continue
        if value == "-o":
            index += 2
            continue
        if value == record["file"]:
            command.append(str(source))
        elif value not in ("-MD", "-MMD", "-MP"):
            command.append(value)
        index += 1
    if str(source) not in command:
        raise SystemExit(f"编译记录中没有源文件参数：{record['file']}")
    command.extend(["-o", str(destination)])
    return command


def global_symbols(*inputs):
    output = subprocess.check_output(["xcrun", "nm", "-g", "-U", "-j", *map(str, inputs)], text=True)
    return {line.strip() for line in output.splitlines() if line.startswith("_")}


sources = {}
for feeds_file in config["feeds"]:
    for entry in tomllib.loads((tree / feeds_file).read_text())["source"]:
        sources[entry["name"]] = entry
for name, directory in config["references"].items():
    revision = subprocess.check_output(["git", "-C", str(tree / directory), "rev-parse", "HEAD"], text=True).strip()
    if revision != sources[name]["commit"]:
        raise SystemExit(f"源码版本不匹配：{name}，预期 {sources[name]['commit']}，实际 {revision}")
xnu = sources["xnu"]
environment = dict(os.environ)
environment.update({name: value.format(**paths) for name, value in config["environment"].items()})
for name, expected in config["features"].items():
    if name in environment and environment[name] != str(expected):
        raise SystemExit(f"DW99 当前配置要求 {name}={expected}，实际为 {environment[name]}")

# This source directory is a generated build product, separate from all reference checkouts.
for name in config["generated_sources"]:
    directory = paths[name]
    if not directory.is_relative_to(paths["output"]) or directory == paths["output"]:
        raise SystemExit(f"源码工作目录必须位于配置的构建输出目录内：{directory}")
with tempfile.TemporaryDirectory(prefix="dw99-xnu-") as temporary:
    prepared = Path(temporary) / "source"
    shutil.copytree(tree / config["references"]["xnu"], prepared, ignore=shutil.ignore_patterns(".git"))
    for directory in config["patch_directories"]:
        patches = sorted((tree / directory).glob("*.patch"))
        if not patches:
            raise SystemExit(f"补丁目录为空：{directory}")
        for patch in patches:
            run(["git", "apply", "--check", patch], cwd=prepared)
            run(["git", "apply", patch], cwd=prepared)
    if paths["source"].exists():
        shutil.rmtree(paths["source"])
    shutil.copytree(prepared, paths["source"])

ctf = config["ctf"]
ctf_source = Path(ctf["source_work"].format(**paths))
if ctf_source.exists():
    shutil.rmtree(ctf_source)
shutil.copytree(tree / config["references"][ctf["reference"]], ctf_source, ignore=shutil.ignore_patterns(".git"))
run([argument.format(**paths, jobs=jobs) for argument in ctf["command"]], cwd=ctf_source)
for executable in ctf["executables"]:
    executable = Path(executable.format(**paths))
    if not executable.is_file() or not os.access(executable, os.X_OK):
        raise SystemExit(f"CTF 工具构建完成但未生成可执行文件：{executable}")

for provider in config["provider"]:
    reference = tree / config["references"][provider["reference"]]
    source_work = Path(provider["source_work"].format(**paths))
    if source_work not in (paths[name] for name in config["generated_sources"]):
        raise SystemExit(f"依赖源码工作目录未列入生成目录配置：{source_work}")
    if source_work.exists():
        shutil.rmtree(source_work)
    with tempfile.TemporaryDirectory(prefix="dw99-provider-") as temporary:
        prepared = Path(temporary) / "source"
        shutil.copytree(reference, prepared, ignore=shutil.ignore_patterns(".git"))
        for patch in provider.get("patches", []):
            run(["git", "apply", "--check", tree / patch], cwd=prepared)
            run(["git", "apply", tree / patch], cwd=prepared)
        shutil.copytree(prepared, source_work)

for directory in config["prepare_directories"]:
    Path(directory.format(**paths)).mkdir(parents=True, exist_ok=True)
for item in config["header_copy"]:
    destination = Path(item["destination"].format(**paths))
    if "pattern" in item:
        destination.mkdir(parents=True, exist_ok=True)
        for header in (tree / item["source"].format(**paths)).glob(item["pattern"]):
            shutil.copy2(header, destination / header.name)
    else:
        shutil.copytree(tree / item["source"].format(**paths), destination, dirs_exist_ok=True)
header_output = Path(config["dispatch_header_output"].format(**paths))
header_output.mkdir(parents=True, exist_ok=True)
for header in (tree / config["dispatch_headers"]).glob("*.h"):
    # unifdef returns 1 when it changes input and 2 for an actual error.
    result = subprocess.run(["unifdef", *config["unifdef_options"], str(header)], stdout=subprocess.PIPE)
    if result.returncode not in (0, 1):
        result.check_returncode()
    (header_output / header.name).write_bytes(result.stdout)

make_variables = {name: value.format(**paths) for name, value in config["make"].items()}
make_variables.update(RC_DARWIN_KERNEL_VERSION=xnu["darwin"], RC_ProjectSourceVersion=xnu["version"], MAKEJOBS=f"-j{jobs}")
header_variables = {**make_variables, **{name: value.format(**paths) for name, value in config["headers_make"].items()}}
run(["make", "-C", paths["source"], "installhdrs", *[f"{name}={value}" for name, value in header_variables.items()]], env=environment)

if not paths["firehose_archive"].is_file():
    if paths["firehose_source"].exists():
        shutil.rmtree(paths["firehose_source"])
    with tempfile.TemporaryDirectory(prefix="dw99-firehose-") as temporary:
        prepared = Path(temporary) / "source"
        shutil.copytree(tree / config["references"]["libdispatch"], prepared, ignore=shutil.ignore_patterns(".git"))
        for patch in config["firehose"]["patches"]:
            run(["git", "apply", "--check", tree / patch], cwd=prepared)
            run(["git", "apply", tree / patch], cwd=prepared)
        shutil.copytree(prepared, paths["firehose_source"])
    project_config = paths["firehose_source"] / config["firehose"]["config"]
    text = project_config.read_text()
    for replacement in config["firehose"]["replace"]:
        text = text.replace(replacement["old"], replacement["new"])
    project_config.write_text("\n".join(
        line + config["firehose"]["header_suffix"].format(**paths) if line.startswith("HEADER_SEARCH_PATHS =") else line
        for line in text.splitlines()
    ) + "\n")
    run([argument.format(**paths, jobs=jobs) for argument in config["firehose"]["command"]])
    if not paths["firehose_archive"].is_file():
        raise SystemExit(f"Firehose 构建完成但未生成静态库：{paths['firehose_archive']}")

# Explicit Clang records let the driver archives inherit the kernel's compiler and ABI.
for variable, value in config["compile_options"].items():
    make_variables[variable] = value.format(**paths)
make_command = ["make", "-C", paths["source"], config["target"], *[f"{name}={value}" for name, value in make_variables.items()]]
run([*make_command, "BUILD_STATIC_LINK=1", "BUILD_XNU_LIBRARY=1"], env=environment)
if not paths["startup_record"].is_file() or not paths["compile_record"].is_file() or not paths["kernel_archive"].is_file():
    raise SystemExit("XNU 静态库构建完成，但未生成必需的 archive 或 Clang 编译记录")
kernel_symbols = global_symbols(*(Path(value.format(**paths)) for value in config["kernel_symbol_inputs"]))
for provider in config["provider"]:
    provider_record = json.loads(paths[provider["record"]].read_text().strip().removesuffix(","))
    source_work = Path(provider["source_work"].format(**paths))
    object_directory = Path(provider["objects"].format(**paths))
    archive = Path(provider["archive"].format(**paths))
    namespace = Path(provider["namespace"].format(**paths))
    provider_paths = dict(paths, provider_source=source_work)
    provider_record_options = {
        "remove_options": provider["record_remove_options"],
        "remove_flags": provider["record_remove_flags"],
    }
    command = [value.format(**provider_paths) for value in provider["compile_options"]]
    for directory in provider["include_directories"]:
        command.extend(["-I", directory.format(**provider_paths)])
    provider_sources = [source_work / source for source in provider["sources"]]
    object_directory.mkdir(parents=True, exist_ok=True)
    objects = [object_directory / f"{index:04d}-{source.stem}.o" for index, source in enumerate(provider_sources)]
    for source in provider_sources:
        if not source.resolve().is_relative_to(source_work) or not source.is_file():
            raise SystemExit(f"依赖源文件缺失或位于工作目录外：{source}")
    for source, obj in zip(provider_sources, objects):
        run([*compile_from_record(provider_record, source, obj, **provider_record_options), *command], cwd=provider_record["directory"])
    archive.unlink(missing_ok=True)
    run(["xcrun", "ar", "rcs", archive, *objects])
    collisions = sorted(global_symbols(archive) & kernel_symbols)
    if any(not symbol[1:].isascii() or not symbol[1:].isidentifier() for symbol in collisions):
        raise SystemExit(f"依赖库包含无法隔离的同名符号：{collisions}")
    namespace.write_text("".join(f"#define {symbol[1:]} {provider['namespace_prefix']}{symbol[1:]}\n" for symbol in collisions))
    if collisions:
        for source, obj in zip(provider_sources, objects):
            run([*compile_from_record(provider_record, source, obj, **provider_record_options), *command, "-include", namespace], cwd=provider_record["directory"])
    archive.unlink()
    run(["xcrun", "ar", "rcs", archive, *objects])
    defined = global_symbols(archive)
    remaining = sorted(defined & kernel_symbols)
    if remaining:
        raise SystemExit(f"依赖库仍有同名全局符号：{remaining}")
    kernel_symbols.update(defined)
    archives.append((archive, provider["force_load"]))

record = json.loads(paths["compile_record"].read_text().strip().removesuffix(","))
for driver in config["driver"]:
    source = tree / driver["source"]
    output = tree / driver["output"]
    output.mkdir(parents=True, exist_ok=True)
    if driver.get("patches"):
        with tempfile.TemporaryDirectory(prefix="dw99-platform-") as directory:
            shutil.copyfile(source, Path(directory) / source.name)
            for patch in driver["patches"]:
                run(["git", "apply", "--check", tree / patch], cwd=directory)
                run(["git", "apply", tree / patch], cwd=directory)
            source = output / source.name
            shutil.copyfile(Path(directory) / source.name, source)
    obj = output / f"{driver['name']}.o"
    command = compile_from_record(record, source, obj)
    for directory in config["include_directories"]:
        command.extend(["-I", directory.format(**paths)])
    command.extend(value.format(**paths) for value in driver.get("compile_options", []))
    run(command, cwd=record["directory"])
    archive = output / f"lib{driver['name']}.a"
    archive.unlink(missing_ok=True)
    run(["xcrun", "ar", "rcs", archive, obj])
    archives.append((archive, True))
link_inputs = [*config["link_options"], *[f"-Wl,-force_load,{path}" if force else str(path) for path, force in archives]]
make_command.extend([
    "LD_KERNEL_ARCHIVES=" + " ".join(shlex.quote(option.format(**paths)) for option in link_inputs),
    "XNU_EXTERNAL_KERNEL_ARCHIVES=" + " ".join([str(path) for path, _ in archives] + [value.format(**paths) for value in config["link_dependencies"]]),
])
run(make_command, env=environment)
if not paths["kernel"].is_file():
    raise SystemExit(f"构建完成但未生成目标内核：{paths['kernel']}")
print(json.dumps({"phase": "kernel", "kernel": str(paths["kernel"]), "archives": [str(path) for path, _ in archives]}))
PY
