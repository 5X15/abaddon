OutFile "${OUTPUT}"
InstallDir "$PROGRAMFILES\Abaddon"
Page Directory
Page InstFiles

Section "Install"
  SetOutPath "$INSTDIR"
  File /r "${INPUT_DIR}\*"
SectionEnd
