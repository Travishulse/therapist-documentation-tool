@echo off
setlocal
title Liz Wegner Proprietary Software - Update Manager
echo ========================================================
echo   Liz Wegner Proprietary Software - Remote Updater
echo ========================================================
echo.

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0update_app.ps1"

echo.
pause
