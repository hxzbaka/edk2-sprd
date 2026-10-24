/** @file UMS512/SharkL5Pro USB2 OTG definitions. */

#ifndef SPRD_USB_H_
#define SPRD_USB_H_

#include <Uefi.h>
#include <Protocol/I2cMaster.h>
#include <Protocol/SprdAonI2c.h>
#include <Protocol/Usb2HostController.h>
#include <Protocol/SprdUsbInit.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

#define SPRD_MUSB_BASE                  0x5FFF0000U
#define SPRD_MUSB_SIZE                  0x2000U
#define SPRD_AON_APB_BASE               0x327D0000U
#define SPRD_ANLG_PHY_G2_BASE           0x323B0000U
#define SPRD_AON_CLK_BASE               0x32080000U
#define SPRD_AON_HW_I2C_BASE            0x32060000U
#define SPRD_ADI_BASE                   0x32100000U
#define SPRD_ADI_SLAVE_BASE             (SPRD_ADI_BASE + 0x20000U)

/* UMS512 maps the SC2730's 15-bit ADI slave window at ADI + 0x20000. */
#define SPRD_SC2730_PMIC_BASE           SPRD_ADI_SLAVE_BASE
#define SPRD_SC2730_TYPEC_BASE          (SPRD_SC2730_PMIC_BASE + 0x0380U)
#define SPRD_SC2730_TYPEC_REG           0x0380U
#define SPRD_SC2730_LDO_USB33_PD_REG    (SPRD_SC2730_PMIC_BASE + 0x19E8U)
#define SPRD_SC2730_LDO_USB33_VOL_REG   (SPRD_SC2730_PMIC_BASE + 0x19ECU)
#define SPRD_SC2730_LDO_USB33_PD_OFFSET 0x19E8U
#define SPRD_SC2730_LDO_USB33_VOL_OFFSET 0x19ECU
#define SPRD_SC2730_LDO_USB33_PD        BIT0
#define SPRD_SC2730_LDO_USB33_VOL_MASK  0x00FFU
/* SC2730 LDO USB33: 1.2 V + selector * 10 mV; 0xd2 is 3.3 V. */
#define SPRD_SC2730_LDO_USB33_3V3       0x00D2U
#define SPRD_ADI_RD_CMD                 0x0028U
#define SPRD_ADI_RD_DATA                0x002CU
#define SPRD_ADI_RD_BUSY                BIT31

/* SC2730 Type-C block registers. */
#define SC2730_TYPEC_EN                 0x00U
#define SC2730_TYPEC_MODE               0x04U
#define SC2730_TYPEC_INT_EN             0x0CU
#define SC2730_TYPEC_INT_CLR            0x10U
#define SC2730_TYPEC_STATUS             0x1CU
#define SC2730_TYPEC_USB20_ONLY         BIT4
#define SC2730_TYPEC_MODE_MASK          0x03U
#define SC2730_TYPEC_MODE_SRC           0x01U
#define SC2730_TYPEC_ATTACH_INT_EN      BIT0
#define SC2730_TYPEC_DETACH_INT_EN      BIT1
#define SC2730_TYPEC_STATE_MASK         0x1FU
#define SC2730_TYPEC_ATTACHED_SRC       0x05U

/* AON APB register offsets and bit fields from sprd,sharkl5-{regs,mask}.h. */
#define AON_APB_EB0                     0x0000U
#define AON_APB_EB1                     0x0004U
#define AON_APB_RST1                    0x0010U
#define AON_CGM_REG1                    0x0138U
#define AON_OTG_PHY_TEST                0x0204U
#define AON_OTG_PHY_CTRL                0x0208U
#define AON_OTG_UTMI_EB                 BIT8
#define AON_ADI_EB                      BIT16
#define AON_ANA_EB                      BIT12
#define AON_OTG_UTMI_SOFT_RST           BIT8
#define AON_OTG_PHY_SOFT_RST            BIT9
#define AON_CGM_DPHY_REF_EN             BIT10
#define AON_CGM_OTG_REF_EN              BIT12
#define AON_OTG_VBUS_VALID_PHYREG       BIT24
#define AON_UTMI_WIDTH_SEL              BIT30
#define AON_USB2_PHY_IDDIG              BIT3

/* Analog PHY G2 USB20 register offsets and bit fields. */
#define USB20_UTMI_CTL1                 0x0058U
#define USB20_BATTER_PLL                0x005CU
#define USB20_UTMI_CTL2                 0x0060U
#define USB20_TRIMMING                  0x0064U
#define USB20_ISO_SW                    0x0070U
#define USB20_REG_SEL_CFG_0             0x0074U
#define USB20_VBUSVLDEXT                BIT16
#define USB20_DATABUS16_8               BIT28
#define USB20_PS_PD_L                   BIT3
#define USB20_PS_PD_S                   BIT4
#define USB20_ISO_SW_EN                 BIT0
#define USB20_DPPULLDOWN                BIT4
#define USB20_DMPULLDOWN                BIT3
#define USB20_REG_SEL_DPPULLDOWN        BIT2
#define USB20_REG_SEL_DMPULLDOWN        BIT1
#define USB20_TUNEHSAMP                 (0x3U << 25)
#define USB20_TFREGRES                  (0x1FU << 19)
#define USB20_TUNEHSAMP_2_6MA           (0x3U << 25)
#define USB20_TFREGRES_TUNE_VALUE       (0x14U << 19)

/* MUSB indexed register layout. */
#define MUSB_FADDR                      0x00U
#define MUSB_POWER                      0x01U
#define MUSB_INTRTX                     0x02U
#define MUSB_INTRRX                     0x04U
#define MUSB_INTRTXE                    0x06U
#define MUSB_INTRRXE                    0x08U
#define MUSB_INTRUSB                    0x0AU
#define MUSB_INTRUSBE                   0x0BU
#define MUSB_INDEX                      0x0EU
#define MUSB_TESTMODE                   0x0FU
#define MUSB_DEVCTL                     0x60U
#define MUSB_TXFIFOSZ                   0x62U
#define MUSB_RXFIFOSZ                   0x63U
#define MUSB_TXFIFOADD                  0x64U
#define MUSB_RXFIFOADD                  0x66U
#define MUSB_OTG_EXT_CSR                0x34BU
#define MUSB_FIFO_OFFSET(Ep)            (0x20U + ((Ep) * 4U))
#define MUSB_FIFOSZ_DPB                 BIT4

/* Spreadtrum MUSB DMA extension.  EP n maps to DMA channel n + 15. */
#define MUSB_DMA_CHN_PAUSE(N)           (0x1C00U + (((N) - 1U) * 0x20U))
#define MUSB_DMA_CHN_CFG(N)             (0x1C04U + (((N) - 1U) * 0x20U))
#define MUSB_DMA_CHN_INTR(N)            (0x1C08U + (((N) - 1U) * 0x20U))
#define MUSB_DMA_CHN_ADDR(N)            (0x1C0CU + (((N) - 1U) * 0x20U))
#define MUSB_DMA_CHN_LEN(N)             (0x1C10U + (((N) - 1U) * 0x20U))
#define MUSB_DMA_CHN_LLIST_PTR(N)       (0x1C14U + (((N) - 1U) * 0x20U))
#define MUSB_DMA_CHN_BYTE_CNT(N)        (0x1C18U + (((N) - 1U) * 0x20U))
#define MUSB_DMA_CHN_REQ(N)             (0x1C1CU + (((N) - 1U) * 0x20U))
#define MUSB_DMA_CHN_ENABLE             BIT0
#define MUSB_DMA_LLIST_INT_ENABLE       BIT2
#define MUSB_DMA_USBRX_INT_ENABLE       BIT4
#define MUSB_DMA_CLEAR_INT_ENABLE       BIT5
#define MUSB_DMA_FRAG_INT_CLEAR         BIT24
#define MUSB_DMA_BLOCK_INT_CLEAR        BIT25
#define MUSB_DMA_LLIST_INT_STATUS       BIT18
#define MUSB_DMA_LLIST_INT_CLEAR        BIT26
#define MUSB_DMA_START_INT_CLEAR        BIT27
#define MUSB_DMA_USBRX_LAST_INT_CLEAR   BIT28

/* MHDRC multipoint target registers, one eight-byte bank per endpoint. */
#define MUSB_BUSCTL_OFFSET(Ep, Offset)  (0x80U + ((Ep) * 8U) + (Offset))
#define MUSB_TXFUNCADDR                 0x00U
#define MUSB_TXHUBADDR                  0x02U
#define MUSB_TXHUBPORT                  0x03U
#define MUSB_RXFUNCADDR                 0x04U
#define MUSB_RXHUBADDR                  0x06U
#define MUSB_RXHUBPORT                  0x07U

/* Endpoint registers are selected by MUSB_INDEX and reside at 0x10..0x1d. */
#define MUSB_INDEXED_REG(Offset)        (0x10U + (Offset))
#define MUSB_TXMAXP                     MUSB_INDEXED_REG (0x00U)
#define MUSB_TXCSR                      MUSB_INDEXED_REG (0x02U)
#define MUSB_RXMAXP                     MUSB_INDEXED_REG (0x04U)
#define MUSB_RXCSR                      MUSB_INDEXED_REG (0x06U)
#define MUSB_RXCOUNT                    MUSB_INDEXED_REG (0x08U)
#define MUSB_TXTYPE                     MUSB_INDEXED_REG (0x0AU)
#define MUSB_TXINTERVAL                 MUSB_INDEXED_REG (0x0BU)
#define MUSB_RXTYPE                     MUSB_INDEXED_REG (0x0CU)
#define MUSB_RXINTERVAL                 MUSB_INDEXED_REG (0x0DU)

/*
 * UMS512 U-Boot configures non-zero endpoints through the flat endpoint
 * bank (base + 0x100 + ep * 0x10). EP0 keeps the indexed control offsets.
 */
#define MUSB_EP_TXMAXP(Ep)              (((Ep) == 0U) ? 0x10U : (0x100U + ((Ep) * 0x10U)))
#define MUSB_EP_TXCSR(Ep)               (((Ep) == 0U) ? 0x12U : (0x102U + ((Ep) * 0x10U)))
#define MUSB_EP_TXCSRH(Ep)              (((Ep) == 0U) ? 0x13U : (0x103U + ((Ep) * 0x10U)))
#define MUSB_EP_RXMAXP(Ep)              (((Ep) == 0U) ? 0x14U : (0x104U + ((Ep) * 0x10U)))
#define MUSB_EP_RXCSR(Ep)               (((Ep) == 0U) ? 0x16U : (0x106U + ((Ep) * 0x10U)))
#define MUSB_EP_RXCSRH(Ep)              (((Ep) == 0U) ? 0x17U : (0x107U + ((Ep) * 0x10U)))
#define MUSB_EP_RXCOUNT(Ep)             (((Ep) == 0U) ? 0x18U : (0x108U + ((Ep) * 0x10U)))

#define MUSB_POWER_SOFTCONN             BIT6
#define MUSB_POWER_HSENAB               BIT5
#define MUSB_POWER_HSMODE               BIT4
#define MUSB_POWER_ISOUPDATE            BIT7
#define MUSB_POWER_RESET                BIT3
#define MUSB_DEVCTL_BDEVICE             BIT7
#define MUSB_DEVCTL_FSDEV               BIT6
#define MUSB_DEVCTL_LSDEV               BIT5
#define MUSB_DEVCTL_HM                  BIT2
#define MUSB_DEVCTL_SESSION             BIT0
#define MUSB_HOST_FORCE_EN              BIT0
#define MUSB_CLEAR_TXBUFF               BIT4
#define MUSB_CLEAR_RXBUFF               BIT5
#define MUSB_TX_CMPL_MODE               BIT6
#define MUSB_INTR_CONNECT               BIT4
#define MUSB_INTR_DISCONNECT            BIT5
#define MUSB_INTR_VBUSERROR             BIT7
#define MUSB_INTRUSBE_HOST_MASK         0xF7U

#define MUSB_CSR0_TXPKTRDY              BIT1
#define MUSB_CSR0_RXPKTRDY              BIT0
#define MUSB_CSR0_FLUSHFIFO             BIT8
#define MUSB_CSR0_H_SETUPPKT            BIT3
#define MUSB_CSR0_H_ERROR               BIT4
#define MUSB_CSR0_H_REQPKT              BIT5
#define MUSB_CSR0_H_STATUSPKT           BIT6
#define MUSB_CSR0_H_NAKTIMEOUT          BIT7
#define MUSB_CSR0_H_RXSTALL             BIT2
#define MUSB_TXCSR_TXPKTRDY             BIT0
#define MUSB_TXCSR_FLUSHFIFO            BIT3
#define MUSB_TXCSR_H_ERROR              BIT2
#define MUSB_TXCSR_H_RXSTALL            BIT5
#define MUSB_TXCSR_H_NAKTIMEOUT         BIT7
#define MUSB_TXCSR_MODE                 BIT13
#define MUSB_TXCSR_P_CLRDATATOG         BIT6
#define MUSB_RXCSR_P_CLRDATATOG         BIT7
#define MUSB_TXCSR_P_FLUSHFIFO          BIT3
#define MUSB_RXCSR_P_FLUSHFIFO          BIT4
#define MUSB_TXCSR_P_UNDERRUN           BIT2
#define MUSB_TXCSR_P_WZC                (BIT7 | BIT5 | BIT2 | BIT1)
#define MUSB_RXCSR_RXPKTRDY             BIT0
#define MUSB_RXCSR_P_WZC                (BIT6 | BIT2 | BIT0)
#define MUSB_RXCSR_H_ERROR              BIT2
#define MUSB_RXCSR_H_REQPKT             BIT5
#define MUSB_RXCSR_H_RXSTALL            BIT6
#define MUSB_RXCSR_H_DATATOGGLE         BIT9
#define MUSB_RXCSR_H_WR_DATATOGGLE      BIT10
#define MUSB_RXCSR_FLUSHFIFO            BIT4

#define MUSB_TYPE_SPEED_FULL             0x80U
#define MUSB_TYPE_SPEED_LOW              0xC0U
#define MUSB_TYPE_PROTO_CONTROL          0x00U
#define MUSB_TYPE_PROTO_ISO              0x10U
#define MUSB_TYPE_PROTO_BULK             0x20U
#define MUSB_TYPE_PROTO_INTERRUPT        0x30U

#define SPRD_MUSB_MAX_ENDPOINTS          16U
#define SPRD_USB_TRANSFER_TIMEOUT_MS     3000U
#define SPRD_MUSB_DEVICE_SIGNATURE        SIGNATURE_32 ('S', 'M', 'U', 'S')

typedef struct {
  UINT32                Signature;
  EFI_USB2_HC_PROTOCOL  Usb2Hc;
  EFI_HANDLE            Handle;
  EFI_PHYSICAL_ADDRESS  Base;
  EFI_USB_HC_STATE      State;
  BOOLEAN               PortConnected;
  BOOLEAN               RootPortPollSeen;
  BOOLEAN               RootPortSnapshotValid;
  BOOLEAN               RootPortSpeedPresent;
  BOOLEAN               HidInputEnabled;
  UINT32                RootPortPollCount;
  UINT8                 LastDevCtl;
  UINT8                 LastPower;
  UINT8                 LastOtgExtCsr;
  UINT8                 LastUsbInterrupt;
  UINT16                PortChange;
  EFI_EVENT             AsyncTimer[SPRD_MUSB_MAX_ENDPOINTS];
  VOID                  *AsyncContext[SPRD_MUSB_MAX_ENDPOINTS];
  EFI_EVENT             HidInputGateTimer;
  EFI_EVENT             ReadyToBootEvent;
} SPRD_MUSB_DEVICE;

EFI_STATUS
EFIAPI
SprdMusbInstall (
  IN SPRD_USB_INIT_PROTOCOL  *Init
  );

#endif
