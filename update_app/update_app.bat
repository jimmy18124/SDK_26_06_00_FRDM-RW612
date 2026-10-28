@echo off

set JLINK_EXE=C:\Program Files\SEGGER\JLink_V980\JLink.exe

"%JLINK_EXE%" -CommanderScript "%~dp0update_app.jlink"

pause