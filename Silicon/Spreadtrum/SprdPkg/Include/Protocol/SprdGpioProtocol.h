#ifndef __SPRD_GPIO_PROTOCOL_H__
#define __SPRD_GPIO_PROTOCOL_H__

#include <Uefi.h>

#define SPRD_GPIO_PROTOCOL_REVISION  0x00010000

typedef enum {
  SprdGpioTypeApGpio,
  SprdGpioTypeApEic,
  SprdGpioTypePmicEic
} SPRD_GPIO_TYPE;

typedef
EFI_STATUS
(EFIAPI *SPRD_GPIO_DIRECTION_INPUT)(
  UINT32 Gpio
  );

typedef
EFI_STATUS
(EFIAPI *SPRD_GPIO_DIRECTION_OUTPUT)(
  UINT32 Gpio,
  BOOLEAN Value
  );

typedef
BOOLEAN
(EFIAPI *SPRD_GPIO_GET)(
  UINT32 Gpio
  );

typedef
EFI_STATUS
(EFIAPI *SPRD_GPIO_SET)(
  UINT32 Gpio,
  BOOLEAN Value
  );

typedef struct {

  UINT32 Revision;

  SPRD_GPIO_DIRECTION_INPUT  DirectionInput;
  SPRD_GPIO_DIRECTION_OUTPUT DirectionOutput;

  SPRD_GPIO_GET             Get;
  SPRD_GPIO_SET             Set;

} SPRD_GPIO_PROTOCOL;

extern EFI_GUID gSprdGpioProtocolGuid;

#endif