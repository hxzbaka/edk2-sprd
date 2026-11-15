/** @file
  SprdClockDxe.c

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Uefi.h>
#include <Protocol/EFIClock.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include "SprdClock.h"

//
// 时钟描述表结构
//
typedef struct {
  CHAR8  *Name;
  UINTN  Id;
  UINTN  GateReg;     // 门控寄存器物理地址
  UINT32 BitMask;     // 对应位掩码
} CLOCK_ENTRY;

//
// 完整时钟表（AON + AP AHB + AP APB）
//
STATIC CLOCK_ENTRY mClockTable[] = {
  // ----- AON 域 (EB0) -----
  {"eic",            CLK_AON_EIC,      AON_CLK_BASE + AON_APB_EB0, (1U << 14)},
  {"adi",            CLK_AON_ADI,      AON_CLK_BASE + AON_APB_EB0, (1U << 16)},
  {"efuse",          CLK_AON_EFUSE,    AON_CLK_BASE + AON_APB_EB0, (1U << 13)},
  {"gpio",           CLK_AON_GPIO,     AON_CLK_BASE + AON_APB_EB0, (1U << 3)},
  {"tpc",            CLK_AON_TPC,      AON_CLK_BASE + AON_APB_EB0, (1U << 2)},
  {"fm",             CLK_AON_FM,       AON_CLK_BASE + AON_APB_EB0, (1U << 1)},
  {"adc",            CLK_AON_ADC,      AON_CLK_BASE + AON_APB_EB0, (1U << 0)},
  {"kpd",            CLK_AON_KPD,      AON_CLK_BASE + AON_APB_EB0, (1U << 8)},
  {"pwm0",           CLK_AON_PWM0,     AON_CLK_BASE + AON_APB_EB0, (1U << 4)},
  {"pwm1",           CLK_AON_PWM1,     AON_CLK_BASE + AON_APB_EB0, (1U << 5)},
  {"pwm2",           CLK_AON_PWM2,     AON_CLK_BASE + AON_APB_EB0, (1U << 6)},
  {"pwm3",           CLK_AON_PWM3,     AON_CLK_BASE + AON_APB_EB0, (1U << 7)},
  {"ap_tmr0",        CLK_AON_AP_TMR0,  AON_CLK_BASE + AON_APB_EB0, (1U << 12)},
  {"aon_tmr",        CLK_AON_AON_TMR,  AON_CLK_BASE + AON_APB_EB0, (1U << 11)},
  {"ap_syst",        CLK_AON_AP_SYST,  AON_CLK_BASE + AON_APB_EB0, (1U << 10)},
  {"aon_syst",       CLK_AON_AON_SYST, AON_CLK_BASE + AON_APB_EB0, (1U << 9)},
  {"aon_intc",       CLK_AON_AON_INTC, AON_CLK_BASE + AON_APB_EB0, (1U << 15)},
  {"aud",            CLK_AON_AUD,      AON_CLK_BASE + AON_APB_EB0, (1U << 18)},
  {"audif",          CLK_AON_AUDIF,    AON_CLK_BASE + AON_APB_EB0, (1U << 17)},
  {"vbc",            CLK_AON_VBC,      AON_CLK_BASE + AON_APB_EB0, (1U << 19)},
  {"pin",            CLK_AON_PIN,      AON_CLK_BASE + AON_APB_EB0, (1U << 20)},
  {"ipi",            CLK_AON_IPI,      AON_CLK_BASE + AON_APB_EB0, (1U << 21)},
  {"splk",           CLK_AON_SPLK,     AON_CLK_BASE + AON_APB_EB0, (1U << 22)},
  {"ap_wdg",         CLK_AON_AP_WDG,   AON_CLK_BASE + AON_APB_EB0, (1U << 24)},
  {"mm",             CLK_AON_MM,       AON_CLK_BASE + AON_APB_EB0, (1U << 25)},
  {"ckg",            CLK_AON_CKG,      AON_CLK_BASE + AON_APB_EB0, (1U << 26)},
  {"gpu",            CLK_AON_GPU,      AON_CLK_BASE + AON_APB_EB0, (1U << 27)},
  {"ca53_ts0",       CLK_AON_CA53_TS0, AON_CLK_BASE + AON_APB_EB0, (1U << 28)},
  {"wtlcp_intc",     CLK_AON_WTLCP_INTC, AON_CLK_BASE + AON_APB_EB0, (1U << 29)},
  {"pubcp_intc",     CLK_AON_PUBCP_INTC, AON_CLK_BASE + AON_APB_EB0, (1U << 30)},
  {"ca53_dap",       CLK_AON_CA53_DAP, AON_CLK_BASE + AON_APB_EB0, (1U << 31)},

  // ----- AP AHB 域 -----
  {"dsi",            CLK_AP_AHB_DSI,    AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 0)},
  {"dispc",          CLK_AP_AHB_DISPC,  AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 1)},
  {"vsp",            CLK_AP_AHB_VSP,    AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 2)},
  {"gsp",            CLK_AP_AHB_GSP,    AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 3)},
  {"otg",            CLK_AP_AHB_OTG,    AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 4)},
  {"dma_pub",        CLK_AP_AHB_DMA_PUB,AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 5)},
  {"ce_pub",         CLK_AP_AHB_CE_PUB, AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 6)},
  {"ahb_ckg",        CLK_AP_AHB_CKG,    AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 7)},
  {"sdio0",          CLK_AP_AHB_SDIO0,  AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 8)},
  {"sdio1",          CLK_AP_AHB_SDIO1,  AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 9)},
  {"nandc",          CLK_AP_AHB_NANDC,  AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 10)},
  {"emmc",           CLK_AP_AHB_EMMC,   AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 11)},
  {"spinlock",       CLK_AP_AHB_SPINLOCK, AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 13)},
  {"ahbreg",         CLK_AP_AHB_AHBREG, AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 23)},
  {"emmc_32k",       CLK_AP_AHB_EMMC_32K,AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 27)},
  {"sdio0_32k",      CLK_AP_AHB_SDIO0_32K, AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 28)},
  {"sdio1_32k",      CLK_AP_AHB_SDIO1_32K, AP_AHB_CLK_BASE + AP_AHB_EB, (1U << 29)},

  // ----- AP APB 域 -----
  {"sim0",           CLK_AP_APB_SIM0,   AP_APB_CLK_BASE + AP_APB_EB, (1U << 0)},
  {"iis0",           CLK_AP_APB_IIS0,   AP_APB_CLK_BASE + AP_APB_EB, (1U << 1)},
  {"apb_reg",        CLK_AP_APB_APBREG, AP_APB_CLK_BASE + AP_APB_EB, (1U << 2)},
  {"spi0",           CLK_AP_APB_SPI0,   AP_APB_CLK_BASE + AP_APB_EB, (1U << 5)},
  {"spi2",           CLK_AP_APB_SPI2,   AP_APB_CLK_BASE + AP_APB_EB, (1U << 7)},
  /* sharkl5pro AP_APB_EB: I2C0 through I2C4 occupy bits 9 through 13. */
  {"i2c0",           CLK_AP_APB_I2C0,   AP_APB_CLK_BASE + AP_APB_EB, (1U << 9)},
  {"i2c1",           CLK_AP_APB_I2C1,   AP_APB_CLK_BASE + AP_APB_EB, (1U << 10)},
  {"i2c2",           CLK_AP_APB_I2C2,   AP_APB_CLK_BASE + AP_APB_EB, (1U << 11)},
  {"i2c3",           CLK_AP_APB_I2C3,   AP_APB_CLK_BASE + AP_APB_EB, (1U << 12)},
  {"i2c4",           CLK_AP_APB_I2C4,   AP_APB_CLK_BASE + AP_APB_EB, (1U << 13)},
  {"uart1",          CLK_AP_APB_UART1,  AP_APB_CLK_BASE + AP_APB_EB, (1U << 14)},
  {"sim0_32k",       CLK_AP_APB_SIM0_32K, AP_APB_CLK_BASE + AP_APB_EB, (1U << 18)},
  {"intc0",          CLK_AP_APB_INTC0,  AP_APB_CLK_BASE + AP_APB_EB, (1U << 19)},
  {"intc1",          CLK_AP_APB_INTC1,  AP_APB_CLK_BASE + AP_APB_EB, (1U << 20)},
  {"intc2",          CLK_AP_APB_INTC2,  AP_APB_CLK_BASE + AP_APB_EB, (1U << 21)},
  {"intc3",          CLK_AP_APB_INTC3,  AP_APB_CLK_BASE + AP_APB_EB, (1U << 22)},
};

STATIC UINTN mNumClocks = sizeof(mClockTable) / sizeof(CLOCK_ENTRY);

//
// 根据名称查找时钟条目
//
STATIC
CLOCK_ENTRY *
FindClockByName (
  IN CONST CHAR8 *Name
  )
{
  UINTN i;
  for (i = 0; i < mNumClocks; i++) {
    if (AsciiStrCmp (Name, mClockTable[i].Name) == 0) {
      return &mClockTable[i];
    }
  }
  return NULL;
}

//
// 根据 ID 查找时钟条目（ID 即索引）
//
STATIC
CLOCK_ENTRY *
FindClockById (
  IN UINTN ClockId
  )
{
  if (ClockId < mNumClocks) {
    return &mClockTable[ClockId];
  }
  return NULL;
}

//
// 实现协议接口函数
//
EFI_STATUS
EFIAPI
SprdClockGetId (
  IN  EFI_CLOCK_PROTOCOL *This,
  IN  CONST CHAR8        *ClockName,
  OUT UINTN              *ClockId
  )
{
  CLOCK_ENTRY *Clock = FindClockByName (ClockName);
  if (Clock == NULL) {
    return EFI_NOT_FOUND;
  }
  *ClockId = Clock->Id;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SprdClockEnable (
  IN EFI_CLOCK_PROTOCOL *This,
  IN UINTN              ClockId
  )
{
  CLOCK_ENTRY *Clock = FindClockById (ClockId);
  UINT32       Val;

  if (Clock == NULL) {
    return EFI_NOT_FOUND;
  }

  Val = MmioRead32 (Clock->GateReg);
  Val |= Clock->BitMask;
  MmioWrite32 (Clock->GateReg, Val);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SprdClockDisable (
  IN EFI_CLOCK_PROTOCOL *This,
  IN UINTN              ClockId
  )
{
  CLOCK_ENTRY *Clock = FindClockById (ClockId);
  UINT32       Val;

  if (Clock == NULL) {
    return EFI_NOT_FOUND;
  }

  Val = MmioRead32 (Clock->GateReg);
  Val &= ~Clock->BitMask;
  MmioWrite32 (Clock->GateReg, Val);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SprdClockIsEnabled (
  IN  EFI_CLOCK_PROTOCOL *This,
  IN  UINTN              ClockId,
  OUT BOOLEAN            *IsEnabled
  )
{
  CLOCK_ENTRY *Clock = FindClockById (ClockId);

  if (Clock == NULL || IsEnabled == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *IsEnabled = (MmioRead32 (Clock->GateReg) & Clock->BitMask) != 0;
  return EFI_SUCCESS;
}

//
// 未实现的功能统一返回不支持
//
STATIC EFI_STATUS EFIAPI Unsupported (VOID) { return EFI_UNSUPPORTED; }

//
// 构建协议实例
//
STATIC EFI_CLOCK_PROTOCOL mClockProtocol = {
  .Version                     = EFI_CLOCK_PROTOCOL_VERSION,
  .GetClockID                  = SprdClockGetId,
  .EnableClock                 = SprdClockEnable,
  .DisableClock                = SprdClockDisable,
  .IsClockEnabled              = SprdClockIsEnabled,
  .IsClockOn                   = (EFI_CLOCK_IS_ON)Unsupported,
  .SetClockFreqHz              = (EFI_CLOCK_SET_FREQ_HZ)Unsupported,
  .GetClockFreqHz              = (EFI_CLOCK_GET_FREQ_HZ)Unsupported,
  .CalcClockFreqHz             = (EFI_CLOCK_CALC_FREQ_HZ)Unsupported,
  .SelectExternalSource        = (EFI_CLOCK_SELECT_EXTERNAL_SOURCE)Unsupported,
  .GetClockPowerDomainID       = (EFI_POWER_DOMAIN_GET_ID)Unsupported,
  .EnableClockPowerDomain      = (EFI_POWER_DOMAIN_ENABLE)Unsupported,
  .DisableClockPowerDomain     = (EFI_POWER_DOMAIN_DISABLE)Unsupported,
  .EnterLowPowerMode           = (EFI_CLOCK_ENTER_LOW_POWER_MODE)Unsupported,
  .ExitLowPowerMode            = (EFI_CLOCK_EXIT_LOW_POWER_MODE)Unsupported,
  .SetClockDivider             = (EFI_CLOCK_SET_CLOCK_DIVIDER)Unsupported,
  .ResetClock                  = (EFI_CLOCK_RESET)Unsupported,
  .GetMaxPerfLevel             = (EFI_GET_MAX_PERF_LEVEL)Unsupported,
  .GetMinPerfLevel             = (EFI_GET_MIN_PERF_LEVEL)Unsupported,
  .GetCpuPerfLevelFrequency    = (EFI_GET_CPU_PERF_LEVEL_FREQUENCY)Unsupported,
  .SetCpuPerfLevel             = (EFI_SET_CPU_PERF_LEVEL)Unsupported,
  .GetCpuPerfLevel             = (EFI_GET_CPU_PERF_LEVEL)Unsupported,
  .GetCPUFrequencyLevels       = (EFI_GET_CPU_FREQUENCY_LEVELS)Unsupported,
  .GetCPUFrequencyPlan         = (EFI_GET_CPU_FREQUECNY_PLAN)Unsupported,
  .SetCPUFrequency             = (EFI_SET_CPU_FREQUENCY)Unsupported,
  .GetCPUFrequency             = (EFI_GET_CPU_FREQUENCY)Unsupported,
  .PostInit                    = (EFI_CLOCK_POST_INIT)Unsupported,
  .GetClockDividerID           = (EFI_CLOCK_DIVIDER_GET_ID)Unsupported,
  .ResetClockBranch            = (EFI_CLOCK_RESET_BRANCH)Unsupported,
  .InvertClock                 = (EFI_CLOCK_INVERT)Unsupported,
  .MeasureFreqHz               = (EFI_CLOCK_MEASURE_FREQ_HZ)Unsupported,
  .GetID                       = (EFI_GET_ID)Unsupported,
};

/**
  SprdClockDxe 入口函数，安装 EFI_CLOCK_PROTOCOL。
**/
EFI_STATUS
EFIAPI
SprdClockDxeInitialize (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;
  EFI_HANDLE Handle = NULL;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiClockProtocolGuid,
                  &mClockProtocol,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_ERROR, "[SprdClockDxe] Clock protocol installed\n"));
  return Status;
}