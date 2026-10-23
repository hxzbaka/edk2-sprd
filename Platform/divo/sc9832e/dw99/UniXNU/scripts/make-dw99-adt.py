#!/usr/bin/env python3
"""Generate an Apple device tree from the board configuration."""

# SPDX-License-Identifier: BSD-2-Clause-Patent
# Copyright (c) 2026, UniXNU contributors.

import argparse
import secrets
import struct
import tomllib
import zlib
from pathlib import Path


def node(name, properties=(), children=()):
    properties = [("name", name.encode() + b"\0"), *properties]
    result = bytearray(struct.pack("<II", len(properties), len(children)))
    for key, data in properties:
        result.extend(struct.pack("<32sI", key.encode(), len(data)))
        result.extend(data)
        result.extend(bytes(-len(data) % 4))
    return bytes(result) + b"".join(children)


def nvram_proxy():
    def header(name, size):
        raw = bytearray(struct.pack("<BBH12s", 0x70, 0, size // 16, name))
        checksum = sum(raw)
        while checksum > 255:
            checksum = (checksum & 255) + (checksum >> 8)
        raw[1] = checksum
        return raw

    image = bytearray(8192)
    image[:16] = header(b"nvram", 32)
    image[32:48] = header(b"common", 16 + 2048)
    free_offset = 32 + 16 + 2048
    image[free_offset:free_offset + 16] = header(b"wwwwwwwwwwww", len(image) - free_offset)
    struct.pack_into("<I", image, 16, zlib.adler32(image[20:]))
    return [
        ("nvram-bank-size", struct.pack("<I", len(image))),
        ("nvram-bank-count", struct.pack("<I", 1)),
        ("nvram-current-bank", struct.pack("<I", 0)),
        ("nvram-proxy-data", bytes(image)),
    ]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--hardware", type=Path, default=Path(__file__).with_name("dw99-hardware.toml"))
    parser.add_argument("--dram-size", type=lambda value: int(value, 0))
    parser.add_argument("--owned-memory-size", type=lambda value: int(value, 0))
    parser.add_argument("--cpu-regs")
    parser.add_argument("--psci-method", choices=["smc", "hvc"])
    parser.add_argument("--chip-id", type=lambda value: int(value, 0))
    parser.add_argument("--unique-chip-id", type=lambda value: int(value, 16), required=True)
    parser.add_argument("--data-ramdisk", action="store_true")
    parser.add_argument("--packed-data-ramdisk", action="store_true")
    parser.add_argument("--trust-cache", action="store_true")
    args = parser.parse_args()
    hardware = tomllib.loads(args.hardware.read_text())
    dram_size = hardware["dram_size"]
    owned_memory_size = hardware["owned_memory_size"] if args.owned_memory_size is None else args.owned_memory_size
    if args.dram_size is not None and args.dram_size != dram_size:
        parser.error("DRAM size must match the DW99 hardware configuration")
    if args.cpu_regs is not None and args.cpu_regs != str(hardware["cpu_mpidr"]):
        parser.error("only the DW99 boot CPU is supported at this stage")
    if args.psci_method is not None and args.psci_method != hardware["psci_method"]:
        parser.error("PSCI method must match the DW99 hardware configuration")
    if not hardware["minimum_owned_memory_size"] <= owned_memory_size <= dram_size or owned_memory_size % hardware["owned_memory_alignment"]:
        parser.error("owned memory must satisfy the hardware configuration's size and alignment limits")
    chip_id = hardware["platform_chip_id"] if args.chip_id is None else args.chip_id
    if not 0 <= chip_id < 1 << 32 or not 0 <= args.unique_chip_id < 1 << 64:
        parser.error("chip ID must fit u32 and unique chip ID must fit u64")
    if args.data_ramdisk and args.packed_data_ramdisk:
        parser.error("select either a dense or a packed RAM disk")

    memory_map = []
    chosen = [
        ("random-seed", secrets.token_bytes(64)),
        ("dram-base", struct.pack("<Q", hardware["dram_base"])),
        ("dram-size", struct.pack("<Q", dram_size)),
        ("darwinboot-memory-size", struct.pack("<Q", owned_memory_size)),
        ("chip-id", struct.pack("<I", chip_id)),
        ("unique-chip-id", struct.pack("<Q", args.unique_chip_id)),
        *nvram_proxy(),
    ]
    if args.data_ramdisk or args.packed_data_ramdisk:
        chosen.append(("ephemeral-storage", struct.pack("<I", 1)))
        memory_map.append(("PackedRAMDisk" if args.packed_data_ramdisk else "RAMDisk", bytes(16)))
    if args.trust_cache:
        memory_map.append(("TrustCache", bytes(16)))

    arm_io = hardware["arm_io"]
    gic = hardware["gic"]
    timer = hardware["timer"]
    display = hardware["display"]
    pstore = hardware["pstore"]
    emmc = hardware["emmc"]
    tree = node("device-tree", [
        ("model", hardware["model"].encode() + b"\0"),
        ("compatible", hardware["model"].encode() + b"\0" + hardware["soc"].encode() + b"\0"),
    ], [
        node("chosen", chosen, [node("memory-map", memory_map)]),
        node("cpus", children=[node("cpu0", [
            ("state", b"running\0"),
            ("reg", struct.pack("<I", hardware["cpu_mpidr"])),
            ("device_type", b"cpu\0"),
            ("compatible", hardware["cpu_compatible"].encode() + b"\0"),
            ("timebase-frequency", struct.pack("<I", hardware["timebase_frequency"])),
        ])]),
        node("psci", [("compatible", b"arm,psci-1.0\0"), ("method", hardware["psci_method"].encode() + b"\0")]),
        node("pstore", [
            ("compatible", b"ramoops\0"),
            ("reg", struct.pack("<QQ", pstore["base"], pstore["size"])),
            ("record-size", struct.pack("<I", pstore["record_size"])),
        ]),
        node("defaults", [("sleep-disabled", struct.pack("<I", 1))]),
        node("options"),
        node("product", [("product-name", hardware["product_name"].encode() + b"\0"), ("model", hardware["model"].encode() + b"\0")]),
        node("arm-io", [
            ("device_type", b"soc\0"),
            ("#address-cells", struct.pack("<I", 2)),
            ("#size-cells", struct.pack("<I", 2)),
            ("ranges", struct.pack("<QQQ", 0, arm_io["base"], arm_io["size"])),
        ], [
            node("intc", [
                ("compatible", gic["compatible"].encode() + b"\0"),
                ("reg", struct.pack("<4Q", gic["distributor"] - arm_io["base"], gic["distributor_size"], gic["cpu_interface"] - arm_io["base"], gic["cpu_interface_size"])),
            ]),
            node("timer", [
                ("compatible", b"arm,armv8-timer\0"),
                ("clock-frequency", struct.pack("<I", hardware["timebase_frequency"])),
                ("physical-interrupt", struct.pack("<I", timer["physical_interrupt"])),
                ("virtual-interrupt", struct.pack("<I", timer["virtual_interrupt"])),
            ]),
            node("dpu", [
                ("compatible", display["compatible"].encode() + b"\0"),
                ("reg", struct.pack("<QQ", display["base"] - arm_io["base"], display["size"])),
                ("interrupt", struct.pack("<I", display["interrupt"])),
                ("frame-timeout-us", struct.pack("<I", display["frame_timeout_us"])),
            ]),
            node("emmc", [
                ("compatible", emmc["compatible"].encode() + b"\0"),
                ("reg", struct.pack("<6Q", emmc["base"] - arm_io["base"], emmc["size"],
                                    emmc["clock_base"] - arm_io["base"], emmc["clock_size"],
                                    emmc["gate_base"] - arm_io["base"], emmc["gate_size"])),
                ("interrupt", struct.pack("<I", emmc["interrupt"])),
                ("clock-frequency", struct.pack("<I", emmc["clock_frequency"])),
                ("clock-selector", struct.pack("<I", emmc["clock_selector"])),
                ("max-frequency", struct.pack("<I", emmc["max_frequency"])),
                ("bus-width", struct.pack("<I", emmc["bus_width"])),
                ("read-only", struct.pack("<I", emmc["read_only"])),
                ("max-transfer-bytes", struct.pack("<I", emmc["max_transfer_bytes"])),
                ("command-timeout-us", struct.pack("<I", emmc["command_timeout_us"])),
                ("data-timeout-us", struct.pack("<I", emmc["data_timeout_us"])),
                ("busy-timeout-us", struct.pack("<I", emmc["busy_timeout_us"])),
                ("initialization-timeout-us", struct.pack("<I", emmc["initialization_timeout_us"])),
            ]),
        ]),
    ])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(tree)


if __name__ == "__main__":
    main()
