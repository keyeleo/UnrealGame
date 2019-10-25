@echo off

pushd %~dp0
:: stop STUNE
taskkill /f /im stunserver.exe

:: stop TURN
taskkill /f /im turnserver.exe

popd
