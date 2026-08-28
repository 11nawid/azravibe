@echo off
setlocal
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install.ps1" "%~dp0"
exit /b %ERRORLEVEL%
