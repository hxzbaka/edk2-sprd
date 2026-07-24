[Components.common]

  Silicon/Spreadtrum/uws6152/Drivers/SprdButtonDxe/SprdButtonDxe.inf
  Silicon/Spreadtrum/uws6152/Drivers/SprdEmmcDxe/SprdEmmcDxe.inf
  Silicon/Spreadtrum/uws6152/Drivers/SprdDisplayDxe/SprdDisplayDxe.inf {
    <LibraryClasses>
!ifdef $(SPRD_DISPLAY_PANEL_LIB)
      SprdDisplayPanelLib|$(SPRD_DISPLAY_PANEL_LIB)
!else
      SprdDisplayPanelLib|Silicon/Spreadtrum/uws6152/Library/SprdDisplayPanelNullLib/SprdDisplayPanelNullLib.inf
!endif
  }
  Silicon/Spreadtrum/uws6152/Drivers/SimpleFbDxe/SimpleFbDxe.inf
  Silicon/Spreadtrum/uws6152/Drivers/GraphicsConsoleDxe/GraphicsConsoleDxe.inf