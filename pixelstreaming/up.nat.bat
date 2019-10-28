:: Scripts to startup PixelStreaming servers

@echo off
:: ip and ports should be exposed
set stunPort=19302
set turnPort=19303

set Path=%Path%;%cd%\Engine\Source\Programs\PixelStreaming\WebServers\SignallingWebServer;%cd%\Engine\Source\Programs\PixelStreaming\WebRTCProxy\bin;%cd%\Engine\Source\ThirdParty\WebRTC\rev.23789\programs\Win64\VS2017\release
pushd %~dp0

@echo on

:: run STUNServer
start "STUNServer" stunserver.exe 0.0.0.0:%stunPort%

:: run TURNServer
start "TURNServer" Powershell.exe -executionpolicy unrestricted -File Start_AWS_TURNServer.ps1 -Port %turnPort%

@popd
cd ..
