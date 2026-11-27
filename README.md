# EDK2 UEFI firmware for Spreadtrum/Unisoc platforms

![banner_wide_dark](https://user-images.githubusercontent.com/17036722/199902341-b086ec31-8d5c-4766-953a-8b9e1492de8b.png)

![GitHub Downloads](https://img.shields.io/github/downloads/hxzbaka/edk2-sprd/total)
![GitHub Release](https://img.shields.io/github/v/release/hxzbaka/edk2-sprd?include_prereleases)

> [!WARNING]
> Due to power management limitations, Windows currently only can run on a single core. Please set `numproc 1` in BCD.

## Description

This repository provides an EDK2 UEFI environment for Spreadtrum/Unisoc SoCs.

It can be used as a boot manager for multi-booting mainline Linux and Android, as well as Windows and XNU on certain SoCs.

This project is based on [edk2-msm](https://github.com/edk2-porting/edk2-msm).

See [Socs.md](Socs.md) for chip compatibility.

## XNU Boot Menu

Common/XNUBootstrap ports the [UniXNU](https://github.com/UniXNU/UniXNU) boot menu ([da14170](https://github.com/UniXNU/UniXNU/commit/da14170ba5478a2bd68546136d59630c4a70189e)) and the [DarwinBoot](https://github.com/UniXNU/DarwinBoot) XNU loader ([3325e35](https://github.com/UniXNU/DarwinBoot/commit/3325e352d7df68a9f9ad74832ed94a6acbbd7390)).

Place the menu configuration at EFI/UniXNU/unixnu.toml on any UEFI-accessible volume. Relative paths are resolved against the configuration file's directory. Each XNU entry requires a kernel and an .adt file, and optionally accepts a .trustcache file and a RAM disk. Required kexts must be prelinked into the kernel.

上一级 UniXNU 菜单的 `type = "efi"` 条目可在 `args` 中传入 `unixnu:config=\EFI\UniXNU\other.toml` 让磁盘上的 UniXNU EFI 应用读取同卷指定的配置。XNU 条目的可选 `loader` 指定相对于配置文件的 DarwinBoot EFI 驱动路径。UniXNU 通过 UEFI 镜像加载服务启动该驱动，并从该驱动的镜像句柄取得启动协议。

The XNU command-line buffer size and memory window are configured via PCDs. The ADT darwinboot-memory-size property can override the default memory window size within those PCD limits.

`/chosen/darwinboot-memory-outside` 接收一张预先清零的区间表。每项依次保存 64 位物理起址和字节数，使用小端序。DarwinBoot 根据退出 Boot Services 前取得的 UEFI Conventional Memory 区间填表。表项不足时交出空表，XNU 继续使用启动窗口。窗口外区间需要由支持 LPAE 的物理页分配器接管。

## Acknowledgements
- Renegade Project's [edk2-msm](https://github.com/edk2-porting/edk2-msm)
- Gustave Monce and his [SurfaceDuoPkg](https://github.com/WOA-Project/SurfaceDuoPkg)
- [DuoWoa Project](https://github.com/WOA-Project)
- [EFIDroid](https://github.com/efidroid)
- [Ben (Bingxing) Wang](https://github.com/imbushuo/)
- [Lumia950XLPkg](https://github.com/WOA-Project/Lumia950XLPkg)
- BigfootACA and his [SimpleInit](https://github.com/BigfootACA/simple-init) and [LinuxSimpleMassStorage](https://github.com/BigfootACA/linux-simple-mass-storage)
- fxsheep and his original edk2-sagit
- All the developers and members of [Renegade Project](https://github.com/edk2-porting/) for their efforts, equipment, valuable documents, and more
- [UniXNU](https://github.com/UniXNU/UniXNU)
- [DarwinBoot](https://github.com/UniXNU/DarwinBoot)

## License
Except for the GPLDriver directory, Sprd-specific drivers, and files in Common/XNUBootstrap, all code is licensed under the BSD 2-Clause license. The GPL drivers are licensed under the GPLv2 license.

Common/XNUBootstrap is licensed under BSD 2-Clause-Patent; see the license files in that directory.
