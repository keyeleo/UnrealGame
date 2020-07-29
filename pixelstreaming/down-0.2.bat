@echo off
set Game=CloudRender

pushd %~dp0

taskkill /fi "windowtitle eq %Game%SignallingWebServer"

taskkill /f /im %Game%.exe
taskkill /f /im %Game%WatchDog.exe
taskkill /f /im UE4.exe

popd
