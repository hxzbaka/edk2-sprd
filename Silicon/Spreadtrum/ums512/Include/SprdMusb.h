/** @file
  Register definitions for the UMS512 (SharkL5Pro) USB2 Mentor controller.

  Values are derived from the SharkL5Pro U-Boot headers and Linux MUSB glue.
**/
#ifndef SPRD_MUSB_H_
#define SPRD_MUSB_H_

#define SPRD_MUSB_BASE                 0x5FFF0000U
#define SPRD_MUSB_SIZE                 0x00002000U

#define SPRD_AON_APB_BASE              0x327D0000U
#define SPRD_AON_APB_EB1               (SPRD_AON_APB_BASE + 0x0004U)
#define SPRD_AON_APB_RST1              (SPRD_AON_APB_BASE + 0x0010U)
#define SPRD_AON_APB_OTG_PHY_TEST      (SPRD_AON_APB_BASE + 0x0204U)
#define SPRD_AON_APB_OTG_PHY_CTRL      (SPRD_AON_APB_BASE + 0x0208U)
#define SPRD_ANLG_PHY_G2_UTMI_CTL1     0x323B0058U

/* T610W SC2730 vddusb33, using the UMS512 15-bit ADI slave window. */
#define SPRD_SC2730_LDO_USB33_REG0     0x321219E8U
#define SPRD_SC2730_LDO_USB33_PD       BIT0

#define SPRD_AON_OTG_UTMI_EB           BIT8
#define SPRD_AON_ANA_EB                BIT12
#define SPRD_AON_OTG_UTMI_RST          BIT8
#define SPRD_AON_OTG_PHY_RST           BIT9
#define SPRD_AON_PHY_VBUS_VALID        BIT24
#define SPRD_AON_UTMI_16BIT            BIT30
#define SPRD_ANLG_UTMI_VBUS_VALID      BIT16
#define SPRD_ANLG_UTMI_16BIT           BIT28

#define MUSB_FADDR                     0x00
#define MUSB_POWER                     0x01
#define MUSB_INTRTX                    0x02
#define MUSB_INTRRX                    0x04
#define MUSB_INTRTXE                   0x06
#define MUSB_INTRRXE                   0x08
#define MUSB_INTRUSB                   0x0A
#define MUSB_INTRUSBE                  0x0B
#define MUSB_INDEX                     0x0E
#define MUSB_DEVCTL                    0x60
#define MUSB_TXFIFOSZ                  0x62
#define MUSB_RXFIFOSZ                  0x63
#define MUSB_TXFIFOADD                 0x64
#define MUSB_RXFIFOADD                 0x66
#define MUSB_OTG_EXT_CSR               0x34B

#define MUSB_FIFO(Endpoint)            (0x20 + ((Endpoint) * 4))
#define MUSB_EPREG(Offset)             (0x10 + (Offset))
#define MUSB_BUSCTL(Endpoint, Offset)  (0x80 + ((Endpoint) * 8) + (Offset))
#define MUSB_TXMAXP                    0x00
#define MUSB_TXCSR                     0x02
#define MUSB_CSR0                      MUSB_TXCSR
#define MUSB_RXMAXP                    0x04
#define MUSB_RXCSR                     0x06
#define MUSB_RXCOUNT                   0x08
#define MUSB_COUNT0                    MUSB_RXCOUNT
#define MUSB_TXTYPE                    0x0A
#define MUSB_TXINTERVAL                0x0B
#define MUSB_RXTYPE                    0x0C
#define MUSB_RXINTERVAL                0x0D
#define MUSB_TXFUNCADDR                0x00
#define MUSB_TXHUBADDR                 0x02
#define MUSB_TXHUBPORT                 0x03
#define MUSB_RXFUNCADDR                0x04
#define MUSB_RXHUBADDR                 0x06
#define MUSB_RXHUBPORT                 0x07

#define MUSB_POWER_HSENAB              BIT5
#define MUSB_POWER_SOFTCONN            BIT6
#define MUSB_POWER_HSMODE              BIT4
#define MUSB_POWER_RESET               BIT3
#define MUSB_DEVCTL_BDEVICE            BIT7
#define MUSB_DEVCTL_FSDEV              BIT6
#define MUSB_DEVCTL_LSDEV              BIT5
#define MUSB_DEVCTL_HM                 BIT2
#define MUSB_DEVCTL_SESSION            BIT0
#define MUSB_HOST_FORCE_EN             BIT0
#define MUSB_TX_CMPL_MODE              BIT6

#define MUSB_INTR_SUSPEND              BIT0
#define MUSB_INTR_RESUME               BIT1
#define MUSB_INTR_RESET                BIT2
#define MUSB_INTR_SOF                  BIT3
#define MUSB_INTR_CONNECT              BIT4
#define MUSB_INTR_DISCONNECT           BIT5
#define MUSB_INTR_VBUSERROR            BIT7

#define MUSB_CSR0_RXPKTRDY             BIT0
#define MUSB_CSR0_TXPKTRDY             BIT1
#define MUSB_CSR0_H_RXSTALL            BIT2
#define MUSB_CSR0_H_SETUPPKT           BIT3
#define MUSB_CSR0_H_ERROR              BIT4
#define MUSB_CSR0_H_REQPKT             BIT5
#define MUSB_CSR0_H_STATUSPKT          BIT6
#define MUSB_CSR0_H_NAKTIMEOUT         BIT7
#define MUSB_CSR0_FLUSHFIFO            BIT8
#define MUSB_CSR0_H_WZC                (MUSB_CSR0_H_RXSTALL | MUSB_CSR0_H_NAKTIMEOUT | MUSB_CSR0_RXPKTRDY)

/* Endpoint zero CSR bits when the controller is a USB peripheral. */
#define MUSB_CSR0_P_SENTSTALL           BIT2
#define MUSB_CSR0_P_DATAEND             BIT3
#define MUSB_CSR0_P_SETUPEND            BIT4
#define MUSB_CSR0_P_SENDSTALL           BIT5
#define MUSB_CSR0_P_SVDRXPKTRDY         BIT6
#define MUSB_CSR0_P_SVDSETUPEND         BIT7

#define MUSB_TXCSR_TXPKTRDY            BIT0
#define MUSB_TXCSR_P_UNDERRUN          BIT2
#define MUSB_TXCSR_FLUSHFIFO           BIT3
#define MUSB_TXCSR_P_SENDSTALL         BIT4
#define MUSB_TXCSR_P_SENTSTALL         BIT5
#define MUSB_TXCSR_CLRDATATOG          BIT6
#define MUSB_TXCSR_P_INCOMPTX          BIT7
#define MUSB_TXCSR_MODE                BIT13
#define MUSB_TXCSR_H_ERROR             BIT2
#define MUSB_TXCSR_H_RXSTALL           BIT5
#define MUSB_TXCSR_H_NAKTIMEOUT        BIT7
#define MUSB_TXCSR_H_DATATOGGLE        BIT8
#define MUSB_TXCSR_H_WR_DATATOGGLE     BIT9
#define MUSB_TXCSR_H_WZC               (MUSB_TXCSR_H_ERROR | MUSB_TXCSR_H_RXSTALL | MUSB_TXCSR_H_NAKTIMEOUT)
#define MUSB_RXCSR_RXPKTRDY            BIT0
#define MUSB_RXCSR_P_OVERRUN           BIT2
#define MUSB_RXCSR_H_ERROR             BIT2
#define MUSB_RXCSR_FLUSHFIFO           BIT4
#define MUSB_RXCSR_H_RXSTALL           BIT6
#define MUSB_RXCSR_H_REQPKT            BIT5
#define MUSB_RXCSR_H_DATATOGGLE        BIT9
#define MUSB_RXCSR_H_WR_DATATOGGLE     BIT10
#define MUSB_RXCSR_H_WZC               (MUSB_RXCSR_H_ERROR | MUSB_RXCSR_H_RXSTALL | MUSB_RXCSR_RXPKTRDY)
#define MUSB_RXCSR_P_SENDSTALL         BIT5
#define MUSB_RXCSR_P_SENTSTALL         BIT6
#define MUSB_RXCSR_CLRDATATOG          BIT7

#define MUSB_TXCSR_P_WZC               (MUSB_TXCSR_P_INCOMPTX | MUSB_TXCSR_P_SENTSTALL | MUSB_TXCSR_P_UNDERRUN | BIT1)
#define MUSB_RXCSR_P_WZC               (MUSB_RXCSR_P_SENTSTALL | MUSB_RXCSR_P_OVERRUN | MUSB_RXCSR_RXPKTRDY)

/*
  UMS512 Linux device-mode FIFO table starts with EP1 TX/RX, both as
  double-buffered 512-byte FIFOs. FIFO addresses are in 8-byte units.
*/
#define SPRD_MUSB_BULK_IN_EP           1
#define SPRD_MUSB_BULK_OUT_EP          1
#define SPRD_MUSB_BULK_FIFO_SIZE       0x16
#define SPRD_MUSB_BULK_TX_FIFO_ADDRESS 0x008
#define SPRD_MUSB_BULK_RX_FIFO_ADDRESS 0x088

#define MUSB_TYPE_PROTO_BULK           0x20
#define MUSB_TYPE_PROTO_INTERRUPT      0x30
#define MUSB_TYPE_SPEED_HIGH            0x40
#define MUSB_TYPE_SPEED_FULL            0x80
#define MUSB_TYPE_SPEED_LOW             0xC0

#endif
