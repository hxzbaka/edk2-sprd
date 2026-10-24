/******************************************
 * @file
 * Keeps DPU R4P0 Layer0 in RGB565 format
 * to match the RGB565 framebuffer and 16-bit DSI packet format
 *
 * DPU          : sprd,r4p0
 * DSI          : SharkL5Pro
 * DPU base     : 0x20300000
 * DSI base     : 0x20400000
 * Mode     : Video
 *
 * Reference    : drivers/video/sprd/dispc/dpu_r4p0.c
 *              : drivers/video/sprd/dsi/core/dwc_mipi_dsi_host.c
 *
 * Copyright (c) 2026 hxzbaka
 ******************************************/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/FrameBufferSerialPortLib.h>
#include <Uefi.h>

extern EFI_GUID gSprdDisplayReadyProtocolGuid;

// DPU definitions (same as before)
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
#define DPU_FMT_RGB565            0U
#define LAYER_CTRL_RGB565_NOBLEND    ((1U << 0) | (DPU_FMT_RGB565 << 4) | (1U << 2))

// DSI definitions
#define DSI_BASE                 0x20400000UL
#define DSI_DPI_COLOR_CODING     0x10
#define COLOR_CODE_16BIT         0


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


  FbSize = Width * Height * 2;   // RGB565 = 2 bytes per pixel
  ZeroMem ((VOID *)(UINTN)FbAddr, FbSize);
  WriteBackInvalidateDataCacheRange ((VOID *)(UINTN)FbAddr, FbSize);
  DEBUG ((DEBUG_INFO, "SprdDisplayDxe: Framebuffer cleared (%u bytes)\n", FbSize));

  /* LK already configured the panel and RGB565 scanout. Do not access DPU/DSI here. */

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