/** @file UMS512 I2C controller selection protocol. */

#ifndef SPRD_I2C_BUS_PROTOCOL_H_
#define SPRD_I2C_BUS_PROTOCOL_H_

#include <Protocol/I2cMaster.h>

#define SPRD_I2C_BUS_PROTOCOL_GUID \
  { 0xe906f555, 0x3863, 0x4e34, { 0x9d, 0x7c, 0x16, 0x8c, 0x44, 0xf3, 0xac, 0x32 } }

extern EFI_GUID  gSprdI2cBusProtocolGuid;

#define SPRD_I2C_BUS_AON0  0U
#define SPRD_I2C_BUS_AP3   3U

typedef struct {
  UINT32                   Bus;
  EFI_I2C_MASTER_PROTOCOL  *I2c;
} SPRD_I2C_BUS_PROTOCOL;

#endif
