@echo off
set Game=Noon.exe

pushd %~dp0
:: stop SignallingWebServer
taskkill /f /im stunserver.exe
:: stop turnserver
taskkill /f /im turnserver.exe

:: stop SignallingWebServer
taskkill /fi "windowtitle eq %Game%SignallingWebServer"
:: stop WebRTCProxy
taskkill /fi "windowtitle eq %Game%WebRTCProxy"
:: stop Game
taskkill /f /im %Game%.exe
taskkill /f /im %Game%WatchDog.exe

popd
