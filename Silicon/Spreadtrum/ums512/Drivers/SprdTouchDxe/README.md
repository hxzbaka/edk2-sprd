# SprdTouchDxe for UMS512

This DXE module is a UMS512/ums512_2h10 adapter for the Synaptics TCM touch
controller described in `t610.dts`. It is intentionally a debug-only driver:
it does not install an input or pointer protocol and does not report touch
coordinates to UEFI applications.

The module polls the controller every 20 ms. The enabled UEFI debug console
will show state changes in this form:

```
SprdTouch: DOWN x=540 y=1170
SprdTouch: MOVE x=556 y=1181
SprdTouch: UP x=556 y=1181
```

## Board defaults from t610.dts

| Item | Value |
| --- | --- |
| I2C controller | `i2c@70600000` (`i2c3`) |
| I2C address | `0x22` |
| I2C speed | 400 kHz |
| IRQ | AP GPIO 144, active low |
| Reset | AP GPIO 145, active low |
| Panel resolution | 1080 x 2340 |

The defaults are in `SprdUms512TcmPlatform.h` and may be overridden from the
platform DSC for a board revision with different wiring.

## EDK2 integration

For the supplied `edk2-sprd` tree, the driver is installed at:

```
Silicon/Spreadtrum/ums512/Drivers/SprdTouchDxe
```

It is listed in both `Platform/Spreadtrum/ums512/dxe.dsc.inc` and
`Platform/Spreadtrum/ums512/dxe.fdf.inc`. The platform memory map also exposes
the AP I2C3 (`0x70600000`) MMIO area. The existing AP APB (`0x71000000`) MMIO
area enables the I2C3 clock gate at AP_APB_EB bit 12.

The I2C3 SCL/SDA pin mux must already be configured as I2C by the firmware
stage before DXE. The driver configures its touch IRQ and reset pins as AP GPIO
144 and 145.
