# EDK2 UEFI firmware for Spreadtrum/Unisoc platforms

![banner_wide_dark](https://user-images.githubusercontent.com/17036722/199902341-b086ec31-8d5c-4766-953a-8b9e1492de8b.png)

![GitHub Downloads](https://img.shields.io/github/downloads/hxzbaka/edk2-sprd/total)
![GitHub Release](https://img.shields.io/github/v/release/hxzbaka/edk2-sprd?include_prereleases)

> [!WARNING]
> Due to power management limitations, Windows currently only can run on a single core. Please set `numproc 1` in BCD.

## Description

This repository provides an EDK2 UEFI environment for Spreadtrum/Unisoc SoCs.

It can be used as a boot manager for multi-booting mainline Linux and Android, as well as Windows on certain SoCs.

This project is based on [edk2-msm](https://github.com/edk2-porting/edk2-msm).

See [Socs.md](Socs.md) for chip compatibility.

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

## License
Except for the GPLDriver directory and Sprd-specific drivers, all code is licensed under the BSD 2-Clause license. The GPL drivers are licensed under the GPLv2 license.
