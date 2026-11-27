// SPDX-License-Identifier: BSD-2-Clause-Patent
#ifndef DARWINBOOT_WINDOW_SIZE_H
#define DARWINBOOT_WINDOW_SIZE_H
// Allocation still requires one real conventional-memory run. This is a
// parser bound, never a declaration of available RAM or an allocation fallback.
static inline int DarwinWindowSizeValid(unsigned long long size, unsigned long long minimum, unsigned long long maximum) {
  return size >= minimum && size <= maximum && !(size & 0x1fffffULL);
}
#endif
