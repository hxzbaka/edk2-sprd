#include <PiDxe.h>

#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/HobLib.h>
#include <Library/SerialPortLib.h>

#include <Resources/FbColor.h>
#include <Resources/font5x12.h>

#include "Library/FrameBufferSerialPortLib.h"
#include <Library/IoLib.h>
#include <Library/TimerLib.h>

// RGB565 Color Definitions
#define RGB565_BLACK 0x0000
#define RGB565_WHITE 0xFFFF
#define RGB565_YELLOW 0xFFE0

#define BGRA8888_BLACK 0x00000000U
#define BGRA8888_WHITE 0x00FFFFFFU
#define BGRA8888_YELLOW 0x00FFFF00U

FBCON_POSITION m_Position;
FBCON_POSITION m_MaxPosition;
FBCON_COLOR    m_Color;
BOOLEAN        m_Initialized = FALSE;

UINTN gWidth = FixedPcdGet32(PcdMipiFrameBufferWidth);
// Reserve half screen for output
UINTN gHeight = FixedPcdGet32(PcdMipiFrameBufferHeight);
UINTN gBpp    = FixedPcdGet32(PcdMipiFrameBufferPixelBpp);
UINTN delay = FixedPcdGet32(PcdMipiFrameBufferDelay);

STATIC
UINTN
FbConBytesPerPixel (
  VOID
  )
{
  return gBpp == 32 ? sizeof(UINT32) : sizeof(UINT16);
}

STATIC
UINTN
FbConColor (
  IN UINT16  Rgb565,
  IN UINT32  Bgra8888
  )
{
  return gBpp == 32 ? Bgra8888 : Rgb565;
}

STATIC
VOID
FbConWritePixel (
  OUT UINT8  *Pixel,
  IN  UINTN  Color,
  IN  UINTN  BytesPerPixel
  )
{
  if (BytesPerPixel == sizeof(UINT32)) {
    *(UINT32 *)Pixel = (UINT32)Color;
  }
  else {
    *(UINT16 *)Pixel = (UINT16)Color;
  }
}

// Internal helpers
void FbConPutCharWithFactor(char c, int type, unsigned scale_factor);

void FbConDrawglyph(
    char *pixels, unsigned stride, unsigned bpp, unsigned *glyph,
    unsigned scale_factor);

void FbConReset(void);
void FbConScrollUp(void);
void FbConFlush(void);

RETURN_STATUS
EFIAPI
SerialPortInitialize(VOID)
{
  UINTN InterruptState = 0;

  // Prevent dup initialization
  if (m_Initialized)
    return RETURN_SUCCESS;

  // Interrupt Disable
  InterruptState = ArmGetInterruptState();
  ArmDisableInterrupts();

  // Reset console
  FbConReset();

  // Set flag
  m_Initialized = TRUE;

  if (InterruptState)
    ArmEnableInterrupts();
  return RETURN_SUCCESS;
}

void ResetFb(void)
{
  UINTN BytesPerPixel = FbConBytesPerPixel();
  UINTN FrameBufferSize = gWidth * gHeight * BytesPerPixel;

  ZeroMem(
      (VOID *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress),
      FrameBufferSize);

  WriteBackInvalidateDataCacheRange(
      (void *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress),
      FrameBufferSize);
}

void FbConReset(void)
{
  // Reset position.
  m_Position.x = 0;
  m_Position.y = 0;

  // Calc max position.
  m_MaxPosition.x = gWidth / (FONT_WIDTH + 1);
  m_MaxPosition.y = (gHeight - 1) / FONT_HEIGHT;

  m_Color.Foreground = FbConColor(RGB565_WHITE, BGRA8888_WHITE);
  m_Color.Background = FbConColor(RGB565_BLACK, BGRA8888_BLACK);
}

void FbConPutCharWithFactor(char c, int type, unsigned scale_factor)
{
  char   *Pixels;
  BOOLEAN intstate = FALSE;
  UINTN BytesPerPixel = FbConBytesPerPixel();

  if (!m_Initialized)
    return;

paint:

  if ((unsigned char)c > 127)
    return;

  if ((unsigned char)c < 32) {
    if (c == '\n') {
      goto newline;
    }
    else if (c == '\r') {
      m_Position.x = 0;
      return;
    }
    else {
      return;
    }
  }

  // Suppress leading spaces (except for special message types)
  if (m_Position.x == 0 && (unsigned char)c == ' ' &&
      type != FBCON_SUBTITLE_MSG && type != FBCON_TITLE_MSG)
    return;

  intstate = ArmGetInterruptState();
  ArmDisableInterrupts();

  Pixels = (char *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress);
  Pixels += m_Position.y * (FONT_HEIGHT * gWidth * BytesPerPixel);
  Pixels += m_Position.x * scale_factor * ((FONT_WIDTH + 1) * BytesPerPixel);

  FbConDrawglyph(
      Pixels, gWidth, BytesPerPixel, font5x12 + (c - 32) * 2, scale_factor);

  m_Position.x++;

  if (m_Position.x >= (int)(m_MaxPosition.x / scale_factor))
    goto newline;

  if (intstate)
    ArmEnableInterrupts();
  return;

newline:
  MicroSecondDelay(delay);
  m_Position.y += scale_factor;
  m_Position.x = 0;
  if (m_Position.y >= m_MaxPosition.y - scale_factor) {
    ResetFb();
    FbConFlush();
    m_Position.y = 0;

    if (intstate)
      ArmEnableInterrupts();
    goto paint;
  }
  else {
    // Clear the new line's background
    Pixels = (char *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress);
    Pixels += m_Position.y * (FONT_HEIGHT * gWidth * BytesPerPixel);
    ZeroMem(Pixels, (FONT_HEIGHT * gWidth * BytesPerPixel) * scale_factor);
    FbConFlush();
    if (intstate)
      ArmEnableInterrupts();
  }
}

// Draw a single glyph into the pixel buffer.
void FbConDrawglyph(
    char *pixels, unsigned stride, unsigned bpp, unsigned *glyph,
    unsigned scale_factor)
{
  UINT8 *pixel_ptr = (UINT8 *)pixels;
  UINTN  bg        = m_Color.Background;
  UINTN  fg        = m_Color.Foreground;

  unsigned x, y, i, j;
  unsigned data, temp;

  // Each character is FONT_HEIGHT tall, glyph[0] upper half, glyph[1] lower
  // half. First, fill the entire glyph cell with background color.
  for (y = 0; y < FONT_HEIGHT; y++) {
    for (i = 0; i < scale_factor; i++) {
      for (x = 0; x < FONT_WIDTH; ++x) {
        for (j = 0; j < scale_factor; j++) {
          FbConWritePixel(pixel_ptr, bg, bpp);
          pixel_ptr += bpp;
        }
      }
      // Move to next line: stride is display width, subtract drawn width
      pixel_ptr += (stride - (FONT_WIDTH * scale_factor)) * bpp;
    }
  }

  // Reset pointer to beginning of cell
  pixel_ptr = (UINT8 *)pixels;

  data = glyph[0];
  for (y = 0; y < FONT_HEIGHT / 2; ++y) {
    temp = data;
    for (i = 0; i < scale_factor; i++) {
      data = temp;
      for (x = 0; x < FONT_WIDTH; ++x) {
        if (data & 1) {
          for (j = 0; j < scale_factor; j++) {
            FbConWritePixel(pixel_ptr, fg, bpp);
            pixel_ptr += bpp;
          }
        }
        else {
          // Skip background pixels (already painted)
          pixel_ptr += scale_factor * bpp;
        }
        data >>= 1;
      }
      // Next scaled line
      pixel_ptr += (stride - (FONT_WIDTH * scale_factor)) * bpp;
    }
  }

  data = glyph[1];
  for (y = 0; y < FONT_HEIGHT / 2; ++y) {
    temp = data;
    for (i = 0; i < scale_factor; i++) {
      data = temp;
      for (x = 0; x < FONT_WIDTH; ++x) {
        if (data & 1) {
          for (j = 0; j < scale_factor; j++) {
            FbConWritePixel(pixel_ptr, fg, bpp);
            pixel_ptr += bpp;
          }
        }
        else {
          pixel_ptr += scale_factor * bpp;
        }
        data >>= 1;
      }
      pixel_ptr += (stride - (FONT_WIDTH * scale_factor)) * bpp;
    }
  }
}

// Scroll the framebuffer up by one text row.
void FbConScrollUp(void)
{
  UINT8 *Dst = (UINT8 *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress);
  UINTN BytesPerPixel = FbConBytesPerPixel();
  UINTN RowSize = gWidth * BytesPerPixel;
  UINTN CopySize = (gHeight - FONT_HEIGHT) * RowSize;

  CopyMem(Dst, Dst + FONT_HEIGHT * RowSize, CopySize);
  ZeroMem(Dst + CopySize, FONT_HEIGHT * RowSize);

  FbConFlush();
}

void FbConFlush(void)
{
  UINTN BytesPerPixel = FbConBytesPerPixel();

  WriteBackInvalidateDataCacheRange(
      (void *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress),
      gWidth * gHeight * BytesPerPixel);

  if (FixedPcdGetBool(PcdDisplayCmdMode)) {
    UINT32 DpuBase = FixedPcdGet32(PcdDpuBaseAddress);
    UINT32 DoneBit = FixedPcdGet32(PcdDpuIntDoneBit);

    if ((MmioRead32(DpuBase + FixedPcdGet32(PcdDpuCfg0Reg)) & BIT0) == 0) {
      return;
    }

    MmioWrite32(DpuBase + FixedPcdGet32(PcdDpuIntClrReg), DoneBit);
    MmioOr32(DpuBase + FixedPcdGet32(PcdDpuCtrlReg), BIT0);
    MicroSecondDelay(1000);
  }
}

UINTN
EFIAPI
SerialPortWrite(IN UINT8 *Buffer, IN UINTN NumberOfBytes)
{
  UINT8 *CONST Final          = &Buffer[NumberOfBytes];
  UINTN        InterruptState = ArmGetInterruptState();
  ArmDisableInterrupts();

  while (Buffer < Final) {
    FbConPutCharWithFactor(*Buffer++, FBCON_COMMON_MSG, SCALE_FACTOR);
  }

  if (InterruptState)
    ArmEnableInterrupts();
  return NumberOfBytes;
}

UINTN
EFIAPI
SerialPortWriteCritical(IN UINT8 *Buffer, IN UINTN NumberOfBytes)
{
  UINT8 *CONST Final              = &Buffer[NumberOfBytes];
  UINTN        PreviousForeground = m_Color.Foreground;
  UINTN        InterruptState     = ArmGetInterruptState();

  ArmDisableInterrupts();
  m_Color.Foreground = FbConColor(RGB565_YELLOW, BGRA8888_YELLOW);

  while (Buffer < Final) {
    FbConPutCharWithFactor(*Buffer++, FBCON_COMMON_MSG, SCALE_FACTOR);
  }

  m_Color.Foreground = PreviousForeground;

  if (InterruptState)
    ArmEnableInterrupts();
  return NumberOfBytes;
}

UINTN
EFIAPI
SerialPortRead(OUT UINT8 *Buffer, IN UINTN NumberOfBytes) { return 0; }

BOOLEAN
EFIAPI
SerialPortPoll(VOID) { return FALSE; }

RETURN_STATUS
EFIAPI
SerialPortSetControl(IN UINT32 Control) { return RETURN_UNSUPPORTED; }

RETURN_STATUS
EFIAPI
SerialPortGetControl(OUT UINT32 *Control) { return RETURN_UNSUPPORTED; }

RETURN_STATUS
EFIAPI
SerialPortSetAttributes(
    IN OUT UINT64 *BaudRate, IN OUT UINT32 *ReceiveFifoDepth,
    IN OUT UINT32 *Timeout, IN OUT EFI_PARITY_TYPE *Parity,
    IN OUT UINT8 *DataBits, IN OUT EFI_STOP_BITS_TYPE *StopBits)
{
  return RETURN_UNSUPPORTED;
}

UINTN SerialPortFlush(VOID) { return 0; }

VOID EnableSynchronousSerialPortIO(VOID)
{
  // Already synchronous
}
