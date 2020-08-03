; GoodRobot.nsi
;
; This script is for the NSIS install tool. It *should* produce a working 
; installer for Win32 platform. Note that the entire script is designed 
; under the assumption that it will be executed in /GoodRobot/ and that all
; data and exe files will be found in a peer directory named /Release

;--------------------------------
!include MUI2.nsh

; The name of the installer
Name "Good Robot ALPHA"

; The file to write
OutFile "..\GoodRobot_v052.exe"

; The default installation directory
InstallDir $PROGRAMFILES\GoodRobot

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages
!insertmacro MUI_PAGE_LICENSE "license.rtf"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
;Page directory
;Page instfiles


;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
;--------------------------------

; The stuff to install
Section "Install" SecInstall

  !cd ..
  !cd Release
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Root files
  File GoodRobot.exe
  File alut.dll
  File DevIL.dll
  File freeglut.dll
  File freetype6.dll
  File glut32.dll
  File SDL.dll
  File zlib1.dll
  File Aldrich-Regular.ttf
  File VT323-Regular.ttf
  File icon.bmp
  ; Data files
  SetOutPath $INSTDIR\core\data
  File core\data\gameplay.ini 
  File core\data\levels.ini
  File core\data\message.ini
  File core\data\robots.ini
  File core\data\sprite.ini
  ; Other files
  SetOutPath $INSTDIR\core\textures
  File core\textures\*
  SetOutPath $INSTDIR\core\sounds
  File core\sounds\*
  SetOutPath $INSTDIR\core\shaders
  File core\shaders\*
  SetOutPath $INSTDIR\core\music
  File core\music\*
  
  WriteUninstaller Uninst.exe
  
  
SectionEnd ; end the section


;--------------------------------
;Descriptions

;Language strings
  LangString DESC_SecInstall ${LANG_ENGLISH} "A test section."

;Assign language strings to sections
;!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
 ; !insertmacro MUI_DESCRIPTION_TEXT ${SecInstall} $(DESC_SecInstall)
;!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"
  Delete $INSTDIR\Uninst.exe ; delete self (see explanation below why this works)
  Delete $INSTDIR\GoodRobot.exe
  Delete $INSTDIR\alut.dll
  Delete $INSTDIR\DevIL.dll
  Delete $INSTDIR\freeglut.dll
  Delete $INSTDIR\freetype6.dll
  Delete $INSTDIR\glut32.dll
  Delete $INSTDIR\SDL.dll
  Delete $INSTDIR\zlib1.dll
  Delete $INSTDIR\Aldrich-Regular.ttf
  Delete $INSTDIR\VT323-Regular.ttf
  Delete $INSTDIR\icon.bmp
  Delete $INSTDIR\core\textures\*
  Delete $INSTDIR\core\sounds\*
  Delete $INSTDIR\core\shaders\*
  Delete $INSTDIR\core\music\*
  Delete $INSTDIR\core\*
  RMDir  $INSTDIR\core\textures
  RMDir  $INSTDIR\core\sounds
  RMDir  $INSTDIR\core\shaders
  RMDir  $INSTDIR\core\music
  RMDir  $INSTDIR\core
  RMDir $INSTDIR
SectionEnd


