/** @file
  Diagnostic driver to test ARM Generic Timer and GIC.

  This driver repeatedly checks the system counter, reads GIC registers,
  and installs a periodic timer event to verify if interrupts are working.
**/

#include <Uefi.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/ArmLib.h>
#include <Library/TimerLib.h>
#include <Library/BaseMemoryLib.h>

#define DIAG_LOOPS                5
#define POLL_DELAY_MS             1000   // 1 second between polls
#define TIMER_PERIOD_MS           2000   // 2 seconds periodic timer

STATIC EFI_EVENT mTimerEvent;
STATIC UINT32    mTimerFireCount = 0;
STATIC BOOLEAN   mTimerFired = FALSE;

//
// GIC register offsets (GICv2)
//
#define GICD_CTLR         0x0000
#define GICD_TYPER        0x0004
#define GICD_IIDR         0x0008
#define GICD_IGROUPR      0x0080
#define GICD_ISENABLER    0x0100
#define GICD_ICPENDR      0x0280
#define GICD_ISPENDR      0x0200
#define GICD_ITARGETSR    0x0800
#define GICD_SPENDSGIR    0x0F20

#define GICC_CTLR         0x0000
#define GICC_PMR          0x0004
#define GICC_IAR          0x000C
#define GICC_EOIR         0x0010
#define GICC_RPR          0x0014
#define GICC_HPPIR        0x0018
#define GICC_DIR          0x1000

/**
  Print GIC Distributor and CPU Interface registers.
**/
VOID
PrintGicRegisters (
  VOID
  )
{
  UINTN GicDBase = PcdGet64 (PcdGicDistributorBase);
  UINTN GicCBase = PcdGet64 (PcdGicInterruptInterfaceBase);

  DEBUG ((EFI_D_ERROR, "\n=== GIC Register Dump ===\n"));
  if (GicDBase != 0) {
    DEBUG ((EFI_D_ERROR, "GICD base: 0x%lx\n", GicDBase));
    DEBUG ((EFI_D_ERROR, "  GICD_CTLR   = 0x%08x\n", MmioRead32 (GicDBase + GICD_CTLR)));
    DEBUG ((EFI_D_ERROR, "  GICD_TYPER  = 0x%08x (lines=%d)\n",
            MmioRead32 (GicDBase + GICD_TYPER),
            ((MmioRead32 (GicDBase + GICD_TYPER) & 0x1F) + 1) * 32));
    DEBUG ((EFI_D_ERROR, "  GICD_IIDR   = 0x%08x\n", MmioRead32 (GicDBase + GICD_IIDR)));
    // Check if timer interrupt is enabled (assuming INTID 30 for non-secure timer)
    UINT32 TimerIntId = PcdGet32 (PcdArmArchTimerIntrNum);
    if (TimerIntId < 32) {
      UINT32 RegIdx = TimerIntId / 32;
      UINT32 Bit = TimerIntId % 32;
      UINT32 EnableReg = MmioRead32 (GicDBase + GICD_ISENABLER + RegIdx * 4);
      DEBUG ((EFI_D_ERROR, "  Timer INT %d: ISENABLER[%d] = 0x%08x (bit %d is %s)\n",
              TimerIntId, RegIdx, EnableReg, Bit,
              (EnableReg & (1 << Bit)) ? "ENABLED" : "DISABLED"));
    }
  } else {
    DEBUG ((EFI_D_ERROR, "GICD base is 0, cannot read\n"));
  }

  UINT32 TimerIntId = PcdGet32 (PcdArmArchTimerIntrNum);
  if (TimerIntId < 32) {
    UINT32 TargetReg = MmioRead32 (GicDBase + GICD_ITARGETSR + (TimerIntId & ~3));
    UINT8 TargetCpu = (TargetReg >> ((TimerIntId % 4) * 8)) & 0xFF;
    DEBUG ((EFI_D_ERROR, "  Timer INT %d: ITARGETSR[%d] = 0x%08x, target CPU mask = 0x%02x\n",
            TimerIntId, TimerIntId / 4, TargetReg, TargetCpu));
  }

  if (GicCBase != 0) {
    DEBUG ((EFI_D_ERROR, "GICC base: 0x%lx\n", GicCBase));
    DEBUG ((EFI_D_ERROR, "  GICC_CTLR   = 0x%08x\n", MmioRead32 (GicCBase + GICC_CTLR)));
    DEBUG ((EFI_D_ERROR, "  GICC_PMR    = 0x%08x\n", MmioRead32 (GicCBase + GICC_PMR)));
    // Reading IAR would consume interrupt, so skip unless needed.
  } else {
    DEBUG ((EFI_D_ERROR, "GICC base is 0\n"));
  }
}

/**
  Timer event callback.
**/
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT Event,
  IN VOID      *Context
  )
{
  mTimerFireCount++;
  mTimerFired = TRUE;
  DEBUG ((EFI_D_ERROR, "[TimerCallback] Fired! Count = %d\n", mTimerFireCount));
}

/**
  Test system counter advancement.
**/
BOOLEAN
TestCounterAdvancement (
  VOID
  )
{
  UINT64 Start, End;
  UINTN  DelayUs = 500000; // 0.5 seconds

  Start = ArmReadCntPct ();
  DEBUG ((EFI_D_ERROR, "CNTPCT_EL0 start = %llu\n", Start));
  gBS->Stall (DelayUs);
  End = ArmReadCntPct ();
  DEBUG ((EFI_D_ERROR, "CNTPCT_EL0 end   = %llu (delta = %llu)\n", End, End - Start));
  if (End > Start) {
    DEBUG ((EFI_D_ERROR, "✅ Timer counter is advancing.\n"));
    return TRUE;
  } else {
    DEBUG ((EFI_D_ERROR, "❌ Timer counter is NOT advancing!\n"));
    return FALSE;
  }
}

/**
  Wait for timer event and check GIC pending interrupts.
**/
VOID
TestTimerEvent (
  VOID
  )
{
  EFI_STATUS Status;
  UINTN      GicCBase = PcdGet64 (PcdGicInterruptInterfaceBase);
  UINT32     TimerIntId = PcdGet32 (PcdArmArchTimerIntrNum);
  UINTN      PollMs = 500;
  UINTN      MaxPolls = (TIMER_PERIOD_MS * 3) / PollMs; // wait up to 3 periods
  UINTN      i;

  DEBUG ((EFI_D_ERROR, "\n=== Testing periodic timer event (period = %d ms) ===\n", TIMER_PERIOD_MS));

  // Create timer event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "❌ CreateEvent failed: %r\n", Status));
    return;
  }

  Status = gBS->SetTimer (mTimerEvent, TimerPeriodic, TIMER_PERIOD_MS * 10000);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "❌ SetTimer failed: %r\n", Status));
    gBS->CloseEvent (mTimerEvent);
    return;
  }

  DEBUG ((EFI_D_ERROR, "Waiting for timer to fire (max %d ms)...\n", MaxPolls * PollMs));

  mTimerFireCount = 0;
  mTimerFired = FALSE;

  for (i = 0; i < MaxPolls; i++) {
    gBS->Stall (PollMs * 1000);
    if (mTimerFireCount > 0) {
      DEBUG ((EFI_D_ERROR, "✅ Timer fired %d time(s) after %d ms.\n", mTimerFireCount, (i+1)*PollMs));
      break;
    }
    // Additionally, check GIC pending status for the timer interrupt
    if (GicCBase != 0 && TimerIntId < 1020) {
      // Read GICC_HPPIR to see highest priority pending interrupt
      UINT32 Hppir = MmioRead32 (GicCBase + GICC_HPPIR);
      UINT32 PendingInt = Hppir & 0x3FF;
      if (PendingInt != 1023) {
        DEBUG ((EFI_D_ERROR, "  GICC_HPPIR = 0x%08x -> pending INT %d\n", Hppir, PendingInt));
        if (PendingInt == TimerIntId) {
          DEBUG ((EFI_D_ERROR, "  ⚠️ Timer interrupt is pending but not handled? Possibly missing interrupt handler.\n"));
        }
      }
    }
  }

  if (mTimerFireCount == 0) {
    DEBUG ((EFI_D_ERROR, "❌ Timer never fired! Possible interrupt routing issue.\n"));
  }

  // Cancel timer and close event
  gBS->SetTimer (mTimerEvent, TimerCancel, 0);
  gBS->CloseEvent (mTimerEvent);
}

/**
  Driver entry point.
**/
EFI_STATUS
EFIAPI
TimerGicDiagEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  UINTN      Loop;

  DEBUG ((EFI_D_ERROR, "\n========== Timer & GIC Diagnostic Driver Started ==========\n"));
  DEBUG ((EFI_D_ERROR, "Platform: SC9832E (Spreadtrum)\n"));

  // 1. Print PCD configuration
  DEBUG ((EFI_D_ERROR, "\n--- PCD Values ---\n"));
  DEBUG ((EFI_D_ERROR, "PcdArmArchTimerFreqInHz    = %u\n", PcdGet32 (PcdArmArchTimerFreqInHz)));
  DEBUG ((EFI_D_ERROR, "PcdArmArchTimerIntrNum     = %u\n", PcdGet32 (PcdArmArchTimerIntrNum)));
  DEBUG ((EFI_D_ERROR, "PcdArmArchTimerSecIntrNum  = %u\n", PcdGet32 (PcdArmArchTimerSecIntrNum)));
  DEBUG ((EFI_D_ERROR, "PcdGicDistributorBase      = 0x%lx\n", PcdGet64 (PcdGicDistributorBase)));
  DEBUG ((EFI_D_ERROR, "PcdGicInterruptInterfaceBase = 0x%lx\n", PcdGet64 (PcdGicInterruptInterfaceBase)));

  // 2. Dump GIC registers
  PrintGicRegisters ();

  // 3. Test system counter advancement (multiple times)
  DEBUG ((EFI_D_ERROR, "\n--- Testing System Counter Advancement ---\n"));
  for (Loop = 0; Loop < DIAG_LOOPS; Loop++) {
    if (!TestCounterAdvancement ()) {
      DEBUG ((EFI_D_ERROR, "Counter test failed at loop %d, aborting further tests.\n", Loop+1));
      break;
    }
    gBS->Stall (POLL_DELAY_MS * 1000);
  }

  // 4. Test timer event and interrupt handling
  TestTimerEvent ();

  // 5. Final summary
  DEBUG ((EFI_D_ERROR, "\n========== Diagnostic Finished ==========\n"));

  return EFI_SUCCESS;
}