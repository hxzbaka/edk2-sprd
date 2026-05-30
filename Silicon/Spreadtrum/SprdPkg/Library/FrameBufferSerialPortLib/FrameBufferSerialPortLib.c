#include <PiDxe.h>

#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/SerialPortLib.h>
#include <Library/TimerLib.h>

#include <Resources/font5x12.h>

#include "Library/FrameBufferSerialPortLib.h"

// ----- RGB565 Color Definitions -----
#define RGB565_BLACK   0x0000
#define RGB565_WHITE   0xFFFF
#define RGB565_YELLOW  0xFFE0

FBCON_POSITION m_Position;
FBCON_POSITION m_MaxPosition;
FBCON_COLOR    m_Color;
BOOLEAN        m_Initialized = FALSE;

UINTN gWidth  = FixedPcdGet32(PcdMipiFrameBufferWidth);
UINTN gHeight = FixedPcdGet32(PcdMipiFrameBufferHeight);
UINTN gBpp    = FixedPcdGet32(PcdMipiFrameBufferPixelBpp);
UINTN delay   = FixedPcdGet32(PcdMipiFrameBufferDelay);

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

  if (m_Initialized)
    return RETURN_SUCCESS;

  InterruptState = ArmGetInterruptState();
  ArmDisableInterrupts();

  FbConReset();
  m_Initialized = TRUE;

  if (InterruptState)
    ArmEnableInterrupts();
  return RETURN_SUCCESS;
}

// Clear the whole frame buffer to black
void ResetFb(void)
{
  UINT16 *Pixel = (UINT16 *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress);
  UINTN   Count = gWidth * gHeight;

  while (Count--) {
    *Pixel++ = RGB565_BLACK;
  }

  WriteBackInvalidateDataCacheRange(
      (void *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress),
      gWidth * gHeight * sizeof(UINT16));
}

void FbConReset(void)
{
  m_Position.x = 0;
  m_Position.y = 0;

  m_MaxPosition.x = gWidth / (FONT_WIDTH + 1);
  m_MaxPosition.y = (gHeight - 1) / FONT_HEIGHT;

  m_Color.Foreground = RGB565_WHITE;
  m_Color.Background = RGB565_BLACK;
}

void FbConPutCharWithFactor(char c, int type, unsigned scale_factor)
{
  char *Pixels;

  if (!m_Initialized)
    return;

paint:
  if ((unsigned char)c > 127)
    return;

  if ((unsigned char)c < 32) {
    if (c == '\n') {
      goto newline;
    } else if (c == '\r') {
      m_Position.x = 0;
      return;
    } else {
      return;
    }
  }

  // Suppress leading spaces (except for special message types)
  if (m_Position.x == 0 && (unsigned char)c == ' ' &&
      type != FBCON_SUBTITLE_MSG && type != FBCON_TITLE_MSG)
    return;

  BOOLEAN intstate = ArmGetInterruptState();
  ArmDisableInterrupts();

  Pixels = (char *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress);
  Pixels += m_Position.y * (FONT_HEIGHT * gWidth * sizeof(UINT16));
  Pixels += m_Position.x * scale_factor * ((FONT_WIDTH + 1) * sizeof(UINT16));

  FbConDrawglyph(
      Pixels, gWidth, sizeof(UINT16), font5x12 + (c - 32) * 2, scale_factor);

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
    // Scroll the screen
    ResetFb();
    FbConFlush();
    m_Position.y = 0;
    if (intstate)
      ArmEnableInterrupts();
    goto paint;
  } else {
    // Clear the new line's background
    Pixels = (char *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress);
    Pixels += m_Position.y * (FONT_HEIGHT * gWidth * sizeof(UINT16));
    ZeroMem(Pixels, (FONT_HEIGHT * gWidth * sizeof(UINT16)) * scale_factor);
    FbConFlush();
    if (intstate)
      ArmEnableInterrupts();
  }
}

/*
 * Draw a single glyph into the pixel buffer.
 * Assumption: bpp == sizeof(UINT16) (RGB565)
 */
void FbConDrawglyph(
    char *pixels, unsigned stride, unsigned bpp, unsigned *glyph,
    unsigned scale_factor)
{
  UINT16 *pixel_ptr = (UINT16 *)pixels;
  UINT16  bg        = (UINT16)m_Color.Background;
  UINT16  fg        = (UINT16)m_Color.Foreground;

  unsigned x, y, i, j;
  unsigned data;

  // Each character is FONT_HEIGHT tall, glyph[0] upper half, glyph[1] lower half.
  // First, fill the entire glyph cell with background color.
  for (y = 0; y < FONT_HEIGHT; y++) {
    for (i = 0; i < scale_factor; i++) {
      for (x = 0; x < FONT_WIDTH; x++) {
        for (j = 0; j < scale_factor; j++) {
          *pixel_ptr++ = bg;
        }
      }
      // Move to next line: stride is display width, subtract drawn width
      pixel_ptr += stride - (FONT_WIDTH * scale_factor);
    }
  }

  // Reset pointer to beginning of cell
  pixel_ptr = (UINT16 *)pixels;

  // Draw upper half (glyph[0])
  data = glyph[0];
  for (y = 0; y < FONT_HEIGHT / 2; y++) {
    unsigned row_bits = data;
    for (i = 0; i < scale_factor; i++) {
      data = row_bits;
      for (x = 0; x < FONT_WIDTH; x++) {
        if (data & 1) {
          for (j = 0; j < scale_factor; j++) {
            *pixel_ptr++ = fg;
          }
        } else {
          // Skip background pixels (already painted)
          pixel_ptr += scale_factor;
        }
        data >>= 1;
      }
      // Next scaled line
      pixel_ptr += stride - (FONT_WIDTH * scale_factor);
    }
  }

  // Draw lower half (glyph[1])
  data = glyph[1];
  for (y = 0; y < FONT_HEIGHT / 2; y++) {
    unsigned row_bits = data;
    for (i = 0; i < scale_factor; i++) {
      data = row_bits;
      for (x = 0; x < FONT_WIDTH; x++) {
        if (data & 1) {
          for (j = 0; j < scale_factor; j++) {
            *pixel_ptr++ = fg;
          }
        } else {
          pixel_ptr += scale_factor;
        }
        data >>= 1;
      }
      pixel_ptr += stride - (FONT_WIDTH * scale_factor);
    }
  }
}

/* Simple scroll up using 16-bit access */
void FbConScrollUp(void)
{
  UINT16 *dst   = (UINT16 *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress);
  UINT16 *src   = dst + (gWidth * FONT_HEIGHT);
  UINTN   count = gWidth * (gHeight - FONT_HEIGHT);

  while (count--) {
    *dst++ = *src++;
  }

  count = gWidth * FONT_HEIGHT;
  while (count--) {
    *dst++ = (UINT16)m_Color.Background;
  }

  FbConFlush();
}

void FbConFlush(void)
{
  WriteBackInvalidateDataCacheRange(
      (void *)(UINTN)FixedPcdGet32(PcdMipiFrameBufferAddress),
      gWidth * gHeight * sizeof(UINT16));
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
  UINT8 *CONST Final             = &Buffer[NumberOfBytes];
  UINTN        PreviousForeground = m_Color.Foreground;
  UINTN        InterruptState    = ArmGetInterruptState();

  ArmDisableInterrupts();
  m_Color.Foreground = RGB565_YELLOW;

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