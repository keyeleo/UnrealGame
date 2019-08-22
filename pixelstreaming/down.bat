@echo off
set Game=Noon.exe

pushd %~dp0
:: stop SignallingWebServer
taskkill /f /im stunserver.exe

:: stop turnserver
taskkill /f /im turnserver.exe

:: stop SignallingWebServer
taskkill /f /im node.exe

:: run WebRTCProxy
taskkill /f /im WebRTCProxy.exe

:: start Game with PixelStreaming
taskkill /f /im %Game%

popd
