; bigtest.nsi
;
; This script attempts to test most of the functionality of the NSIS exehead.

;--------------------------------

!include "MUI.nsh"

;--------------------------------

Name "Polychora Screensaver"


SetDateSave on
SetDatablockOptimize on
CRCCheck on
SilentInstall normal
XPStyle on

Caption "Polychora Screensaver"
OutFile "setup_polychora_screensaver_v1_00.exe"

CompletedText "Installation complete."
InstallDir "$SYSDIR"
BrandingText "*"
;--------------------------------

!define MUI_ICON "inst.ico"

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

!undef MUI_ICON

;--------------------------------



AutoCloseWindow false
ShowInstDetails show

Section "!!"

  SetOutPath $SYSDIR

  File  /oname=Polychora.scr Polychora.exe

WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Polychora" \
                 "DisplayName" "Polychora Screensaver"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Polychora" \
                 "UninstallString" "$INSTDIR\PolychoraUninstall.exe"

  WriteUninstaller PolychoraUninstall.exe

SectionEnd


Section "Uninstall"
  Delete $INSTDIR\Polychora.scr

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Polychora"
SectionEnd