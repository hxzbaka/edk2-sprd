// SPDX-License-Identifier: BSD-2-Clause-Patent
#include <Library/PcdLib.h>
#include <AsmMacroIoLibV8.h>

.equ DPU_INT_RAW, 0x016c
.equ DPU_CTRL_RUN, 0x1
.equ MICROSECONDS_PER_SECOND, 1000000

// x0=validated BGRA framebuffer PA, x1=framebuffer bytes, w2=stage color.
// No stack, callee-saved register, or firmware service is used.
ASM_FUNC(DarwinBootMarkStage)
  cbz x0, DarwinBootMarkFailed
  lsr x14, x1, #4
  bic x14, x14, #7
  cbz x14, DarwinBootMarkFailed
  add x14, x0, x14
  add x13, x0, x1

  MOV32 (x3, FixedPcdGet32 (PcdDpuBaseAddress))
  cbz x3, DarwinBootMarkFailed
  MOV32 (x4, FixedPcdGet32 (PcdDpuCtrlReg))
  add x4, x3, x4
  MOV32 (x5, FixedPcdGet32 (PcdDpuIntClrReg))
  add x5, x3, x5
  add x6, x3, #DPU_INT_RAW
  MOV32 (w7, FixedPcdGet32 (PcdDpuIntDoneBit))
  cbz w7, DarwinBootMarkFailed

  MOV32 (x9, FixedPcdGet32 (PcdDarwinBootHandoffTimeoutUs))
  cbz x9, DarwinBootMarkFailed
  mrs x8, cntfrq_el0
  cbz x8, DarwinBootMarkFailed
  mul x8, x8, x9
  MOV32 (x9, MICROSECONDS_PER_SECOND)
  udiv x8, x8, x9
  cbz x8, DarwinBootMarkFailed

  mrs x9, cntpct_el0
DarwinBootWaitPreviousFrame:
  ldr w11, [x6]
  tst w11, w7
  b.ne DarwinBootPaintStage
  mrs x10, cntpct_el0
  sub x10, x10, x9
  cmp x10, x8
  b.lo DarwinBootWaitPreviousFrame
  b DarwinBootMarkFailed

DarwinBootPaintStage:
  mov x15, x0
DarwinBootPaintPixels:
  stp w2, w2, [x15], #8
  cmp x15, x14
  b.lo DarwinBootPaintPixels

  mrs x12, ctr_el0
  ubfx x12, x12, #16, #4
  mov x11, #4
  lsl x12, x11, x12
  sub x11, x12, #1
  bic x15, x0, x11
  dsb sy
DarwinBootCleanFramebuffer:
  dc cvac, x15
  add x15, x15, x12
  cmp x15, x13
  b.lo DarwinBootCleanFramebuffer
  dsb sy

  str w7, [x5]
  ldr w11, [x4]
  orr w11, w11, #DPU_CTRL_RUN
  str w11, [x4]
  dsb sy

  mrs x9, cntpct_el0
DarwinBootWaitStageFrame:
  ldr w11, [x6]
  tst w11, w7
  b.ne DarwinBootMarkSucceeded
  mrs x10, cntpct_el0
  sub x10, x10, x9
  cmp x10, x8
  b.lo DarwinBootWaitStageFrame
DarwinBootMarkFailed:
  mov w0, #0
  ret

DarwinBootMarkSucceeded:
  // The next handoff stage and the kernel console consume this DONE latch.
  dsb sy
  mov w0, #1
  ret
