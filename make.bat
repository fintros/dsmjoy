@echo off

rem Change avrdude programmer and port here
set AVRDUDE_PROGRAMMER=ftbb
set AVRDUDE_PORT=ft0

rem Change LUFA path to actual
set LUFA_PATH=C:/Data/work/AlSp/AVR/LUFA-130303/LUFA

set PATH=bin;%PATH%

make.exe %*

