// SPDX-License-Identifier: BSD-2-Clause-Patent
#include <AsmMacroIoLibV8.h>

ASM_FUNC(DarwinCurrentEl)
  mrs x0, CurrentEL
  ret

// x0=boot_args PA, x1=entry PA, x2=owned PA, x3=owned size, x4=frame buffer
// PA (0 for none), x5=its size in bytes.
ASM_FUNC(DarwinEnter)
  uxtb w9, w6
  msr daifset, #15
  add x5, x2, x3
  mrs x6, ctr_el0
  ubfx x6, x6, #16, #4
  mov x7, #4
  lsl x6, x7, x6
  sub x7, x6, #1
  bic x4, x2, x7
  dsb sy
1:
  dc civac, x4
  add x4, x4, x6
  cmp x4, x5
  b.lo 1b
  dsb sy
  ic iallu
  dsb sy
  isb
  mrs x4, CurrentEL
  cmp x4, #8
  b.eq DarwinEnterEl2
  mrs x4, sctlr_el1
  bic x4, x4, #1
  bic x4, x4, #4
  bic x4, x4, #0x1000
  msr sctlr_el1, x4
  isb
  tlbi vmalle1
  dsb sy
  isb
  br x1

DarwinEnterEl2:
  mrs x4, sctlr_el2
  bic x4, x4, #1
  bic x4, x4, #4
  bic x4, x4, #0x1000
  msr sctlr_el2, x4
  isb
  tlbi alle2
  dsb sy
  isb
  mov x4, #0x80000000
  cmp x9, #0
  csel x4, x4, xzr, eq
  msr hcr_el2, x4
  isb
  mov x4, #3
  msr cnthctl_el2, x4
  msr cntvoff_el2, xzr
  msr cnthp_ctl_el2, xzr
  mov x4, #0x33ff
  msr cptr_el2, x4
  msr hstr_el2, xzr
  msr vttbr_el2, xzr
  mrs x4, id_aa64dfr0_el1
  sbfx x4, x4, #8, #4
  cmp x4, #1
  b.lt DarwinEnterNoPmu
  mrs x4, pmcr_el0
  ubfx x4, x4, #11, #5
  b DarwinEnterMdcr
DarwinEnterNoPmu:
  mov x4, xzr
DarwinEnterMdcr:
  msr mdcr_el2, x4
  mrs x4, midr_el1
  msr vpidr_el2, x4
  mrs x4, mpidr_el1
  msr vmpidr_el2, x4
  cbnz x9, DarwinEnterArm32
  mov x4, #0x0800
  movk x4, #0x30d0, lsl #16
  msr sctlr_el1, x4
  isb
  tlbi vmalle1
  dsb sy
  isb
  mov x4, #0x3c5
  msr spsr_el2, x4
  adr x4, DarwinEnterEl1
  msr elr_el2, x4
  eret
DarwinEnterEl1:
  br x1


// ARM32 XNU starts in ARM state with r0 pointing to its physical boot_args.
DarwinEnterArm32:
  mov x4, #0x0078
  movk x4, #0x00c5, lsl #16
  msr sctlr_el1, x4
  msr tcr_el1, xzr
  msr ttbr0_el1, xzr
  msr ttbr1_el1, xzr
  isb
  tlbi vmalle1
  dsb sy
  isb
  mov x4, #0x1d3
  msr spsr_el2, x4
  msr elr_el2, x1
  mov x1, xzr
  eret
