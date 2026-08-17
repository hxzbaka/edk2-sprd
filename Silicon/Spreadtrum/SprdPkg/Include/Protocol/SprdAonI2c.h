/** @file Private protocol identifying the UMS512 AON hardware I2C bus. */

#ifndef SPRD_AON_I2C_PROTOCOL_H_
#define SPRD_AON_I2C_PROTOCOL_H_

#include <Protocol/I2cMaster.h>

#define SPRD_AON_I2C_PROTOCOL_GUID \
  { 0x5c48a887, 0x79c4, 0x4c73, { 0xa4, 0x58, 0x7e, 0xaf, 0x7d, 0x13, 0x22, 0xd0 } }

extern EFI_GUID  gSprdAonI2cProtocolGuid;

typedef struct {
  EFI_I2C_MASTER_PROTOCOL  *I2c;
} SPRD_AON_I2C_PROTOCOL;

#endif
