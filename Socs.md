# SoC adaptation status

UEFI:

|SoC|Board|GIC/Timer|Display|EMMC/UFS|Acpi|GPIO|I2C|Pmic|Usb|Otg|Charger|Boot|
|-|-|-|-|-|-|-|-|-|-|-|-|-|
|SC9832E|sp9832e_1h10|√|√|√|×|√|×|×|×|×|×|Linux|
|UMS512|ums512_1h10|√|√|√|√|√|√|√|×|×|√|Linux/Win|
|UWS6152|uws6152_1h10|√|√|√|√|√|×|×|×|×|×|Linux/Win|
|UD710|ud710_1h10|√|√|√|√|√|×|×|×|×|×|Linux/Win|
|T157|ums9620_1h10|√|√|√|√|√|×|×|×|×|×|Linux/Win|

# Device adaptation

|Device|OEM|SoC|Screen|Buttons|Touch|
|-|-|-|-|-|-|
|RT|SPRD|SC9832E|LCD|√|×|
|t610yp|YEPEN|UMS512|OLED/Video|√|√|
|s16b|XinTaiQi|UWS6152|OLED/Command|√|×|
|bbka2|EEBBK|UMS512|LCD|√|×|
|icc8pro|iFLYTEK|UD710|LCD|√|×|
|y60|Coolpad|UMS9620_T157|LCD|√|×|