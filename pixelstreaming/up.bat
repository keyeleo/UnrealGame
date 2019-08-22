:: Scripts to startup PixelStreaming servers

@echo off
set UnrealEngine=C:\project\UnrealEngine
set Game=Noon.exe
:: ip and ports should be exposed, signallingPort is for web client
set publicIp=172.18.137.168
set signallingPort=80
set webRTCPort=8888
set stunPort=19302
set turnPort=19303
:: private port
set gamePort=8124
:: framerate and bitrate
set frameRate=30
set averageBitRate=20000000
:: heartbeat
set cirrusHeartbeat=4
set ue4Heartbeat=3
set proxyHeartbeat=20
:: NAT or local
set NAT=true

set cirrus=%UnrealEngine%\Engine\Source\Programs\PixelStreaming\WebServers\SignallingWebServer\cirrus
set Path=%Path%;%UnrealEngine%\Engine\Source\Programs\PixelStreaming\WebServers\SignallingWebServer;%UnrealEngine%\Engine\Source\Programs\PixelStreaming\WebRTCProxy\bin;%UnrealEngine%\Engine\Source\ThirdParty\WebRTC\rev.23789\programs\Win64\VS2017\release
pushd %~dp0

:: stop all first
call down.bat
timeout /T 1
@echo on

if %NAT%==true (
	:: run STUNServer
	start "STUNServer" stunserver.exe 0.0.0.0:%stunPort%

	:: run TURNServer
	start "TURNServer" Powershell.exe -executionpolicy unrestricted -File Start_AWS_TURNServer.ps1 -Port %turnPort%

	:: run SignallingWebServer
	::start "SignallingWebServer" cmd /k call run.bat --publicIp %publicIp% --httpPort %signallingPort% --proxyPort %webRTCPort%
	start "SignallingWebServer" Powershell.exe -executionpolicy unrestricted -File Start_AWS_WithTURN_SignallingServer.ps1 -cirrus %cirrus% -publicIp %publicIp% -port %signallingPort% -webRTCPort %webRTCPort% -stunPort %stunPort% -turnPort %turnPort% -heartbeat %cirrusHeartbeat%
) else (
	:: run SignallingWebServer
	start "SignallingWebServer" node %cirrus% --publicIp %publicIp% --httpPort %signallingPort% --proxyPort %webRTCPort% --heartbeat %cirrusHeartbeat%
)

:: run WebRTCProxy
start "WebRTCProxy" WebRTCProxy.exe -Cirrus=%publicIp%:%webRTCPort% -UE4Port=%gamePort%

@popd
@pushd %~dp0

:: start Game with PixelStreaming
start "Game" %Game% -RenderOffScreen -PixelStreamingPort=%gamePort% -NvEncFrameRateNum=%frameRate% -NvEncAverageBitRate=%averageBitRate% -Heartbeat=%ue4Heartbeat%

@popd
