/******************************************
 * @file
 * Reconfigures ROC1 DPU R3P0 Layer0 from RGB565 (left by U-Boot logo)
 * to ARGB8888, and DSI packet format from RGB565 to RGB888
 *
 * DPU          : sprd,r3p0
 * SoC/board    : ROC1 / UD710_1H10
 * DPU base     : 0x20300000
 * DSI base     : 0x20400000
 * Mode     : Video
 *
 * Reference    : drivers/video/sprd/dispc/dpu_r3p0.c
 *              : drivers/video/sprd/dsi/core/dwc_mipi_dsi_host.c
 *
 * Copyright (c) 2026 hxzbaka
 ******************************************/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/FrameBufferSerialPortLib.h>
#include <Uefi.h>

extern EFI_GUID gSprdDisplayReadyProtocolGuid;

// ROC1 DPU R3P0 register layout.
#define DPU_BASE                  0x20300000UL
#define DPU_REG32(offset)         (*((volatile UINT32 *)(UINTN)(DPU_BASE + (offset))))

#define REG_DPU_CTRL              DPU_REG32(0x004)
#define REG_DPU_CFG0              DPU_REG32(0x008)
#define REG_PANEL_SIZE            DPU_REG32(0x020)
#define REG_BLEND_SIZE            DPU_REG32(0x024)
#define REG_BG_COLOR              DPU_REG32(0x02C)

#define LAYER_BASE(i)             (0x030 + (i) * 0x030)
#define REG_LAYER_ADDR0(i)        DPU_REG32(LAYER_BASE(i) + 0x000)
#define REG_LAYER_CTRL(i)         DPU_REG32(LAYER_BASE(i) + 0x010)
#define REG_LAYER_SIZE(i)         DPU_REG32(LAYER_BASE(i) + 0x014)
#define REG_LAYER_PITCH(i)        DPU_REG32(LAYER_BASE(i) + 0x018)
#define REG_LAYER_POS(i)          DPU_REG32(LAYER_BASE(i) + 0x01C)
#define REG_LAYER_ALPHA(i)        DPU_REG32(LAYER_BASE(i) + 0x020)
#define REG_LAYER_CROP_START(i)   DPU_REG32(LAYER_BASE(i) + 0x02C)

#define REG_DPU_INT_EN            DPU_REG32(0x1E0)
#define REG_DPU_INT_CLR           DPU_REG32(0x1E4)
#define REG_DPU_INT_RAW           DPU_REG32(0x1EC)

#define INT_DONE_MASK             (1U << 0)
#define INT_UPDATE_DONE_MASK      (1U << 4)
#define DPU_FMT_ARGB8888          3U
#define LAYER_CTRL_ARGB8888_NOBLEND  ((1U << 0) | (DPU_FMT_ARGB8888 << 4) | (1U << 2)) // 0x35

// DSI definitions
#define DSI_BASE                 0x20400000UL
#define DSI_DPI_COLOR_CODING     0x10
#define DSI_DPI_COLOR_CODING_MASK 0xFU
#define COLOR_CODE_24BIT         5

STATIC
BOOLEAN
DpuPollBit (
  IN volatile UINT32 *Reg,
  IN UINT32           Mask,
  IN UINT32           TimeoutMs
  )
{
  while (TimeoutMs--) {
    if ((*Reg) & Mask)
      return TRUE;
    gBS->Stall (1000);
  }
  return FALSE;
}

STATIC
VOID
ConfigureDsiRgb888 (
  VOID
  )
{
  UINT32 ColorCoding;

  // Preserve the DSI host's existing non-color-coding configuration.
  ColorCoding = MmioRead32 (DSI_BASE + DSI_DPI_COLOR_CODING);
  ColorCoding = (ColorCoding & ~DSI_DPI_COLOR_CODING_MASK) | COLOR_CODE_24BIT;
  MmioWrite32 (DSI_BASE + DSI_DPI_COLOR_CODING, ColorCoding);
}

STATIC
VOID
ConfigureDisplay (
  IN UINT32 FbAddr,
  IN UINT32 Width,
  IN UINT32 Height
  )
{
  UINT32 PanelSize;
  UINT32 i;

  DEBUG ((DEBUG_INFO, "SprdDisplayDxe: Configuring DPU/DSI for ARGB8888 %ux%u\n", Width, Height));

  // Stop DPU
  REG_DPU_INT_EN = 0;
  REG_DPU_INT_CLR = 0xFFFFFFFF;
  REG_DPU_CTRL |= (1U << 1);   // stop
  DpuPollBit ((volatile UINT32 *)(UINTN)(DPU_BASE + 0x1EC), INT_DONE_MASK, 200);
  REG_DPU_INT_CLR = INT_DONE_MASK;

  // The ROC1 DWC DSI host uses code 5 for 24-bit RGB888 video packets.
  ConfigureDsiRgb888 ();
  DEBUG ((DEBUG_INFO, "DSI color coding set to 24-bit (5)\n"));

  // Clear all layers
  for (i = 0; i < 8; i++) {
    REG_LAYER_CTRL (i) = 0;
  }

  // Configure layer 0
  PanelSize = ((Height & 0xFFFF) << 16) | (Width & 0xFFFF);
  REG_LAYER_ADDR0 (0)      = FbAddr;
  REG_LAYER_SIZE  (0)      = PanelSize;
  REG_LAYER_PITCH (0)      = Width;
  REG_LAYER_POS   (0)      = 0;
  REG_LAYER_CROP_START (0) = 0;
  REG_LAYER_ALPHA (0)      = 0xFF;
  REG_LAYER_CTRL  (0)      = LAYER_CTRL_ARGB8888_NOBLEND;

  REG_PANEL_SIZE = PanelSize;
  REG_BLEND_SIZE = PanelSize;
  REG_BG_COLOR   = 0;

  // Run DPU
  REG_DPU_CTRL |= (1U << 0);   // run
  gBS->Stall (50000);
  if (DpuPollBit ((volatile UINT32 *)(UINTN)(DPU_BASE + 0x1EC), INT_UPDATE_DONE_MASK, 100)) {
    REG_DPU_INT_CLR = INT_UPDATE_DONE_MASK;
  }
  DEBUG ((DEBUG_INFO, "DPU running in ARGB8888 mode\n"));
}

EFI_STATUS
EFIAPI
SprdDisplayDxeInitialize (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  Handle = NULL;

  // Check if display was initialized by U-Boot
  BOOLEAN DisplayInitialized = FixedPcdGetBool (PcdMipiDisplayInitialized);
  if (!DisplayInitialized) {
    DEBUG ((DEBUG_WARN, "SprdDisplayDxe: Display not initialized by U-Boot, skipping DPU/DSI config\n"));
    // Still install the ready protocol so SimpleFbDxe can proceed (if it can handle it)
    Status = gBS->InstallProtocolInterface (
                    &Handle,
                    &gSprdDisplayReadyProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    NULL
                    );
    ASSERT_EFI_ERROR (Status);
    return EFI_SUCCESS;
  }

  // Framebuffer was initialized, proceed with reconfiguration
  UINT32 FbAddr = FixedPcdGet32 (PcdMipiFrameBufferAddress);
  UINT32 Width  = FixedPcdGet32 (PcdMipiFrameBufferWidth);
  UINT32 Height = FixedPcdGet32 (PcdMipiFrameBufferHeight);
  UINT32 FbSize;

  if (FbAddr == 0 || Width == 0 || Height == 0) {
    DEBUG ((DEBUG_ERROR, "SprdDisplayDxe: Display initialized but PCD parameters invalid!\n"));
    return EFI_DEVICE_ERROR;
  }


  FbSize = Width * Height * 4;   // ARGB8888 = 4 bytes per pixel
  ZeroMem ((VOID *)(UINTN)FbAddr, FbSize);
  WriteBackInvalidateDataCacheRange ((VOID *)(UINTN)FbAddr, FbSize);
  DEBUG ((DEBUG_INFO, "SprdDisplayDxe: Framebuffer cleared (%u bytes)\n", FbSize));

  ConfigureDisplay (FbAddr, Width, Height);

  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gSprdDisplayReadyProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_ERROR, "SprdDisplayDxe: Display configuration complete\n"));
  return EFI_SUCCESS;
}
