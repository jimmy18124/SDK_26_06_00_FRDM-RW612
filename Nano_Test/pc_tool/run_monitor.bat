@echo off
setlocal
cd /d "%~dp0"
python nano_tcp_monitor.py
if errorlevel 1 pause
