:: Scripts to startup PixelStreaming servers

@echo off
set Game=Noon.exe
:: ip and ports should be exposed, signallingPort is for web client
set publicIp=172.18.137.168
set stunPort=19302
set turnPort=19303
:: private port
set signallingPort=80
set webRTCPort=8888
set gamePort=8124
:: framerate and bitrate
set frameRate=-1
set averageBitRate=10000
set cirrusHeartbeat=4
set ue4Heartbeat=3
set proxyHeartbeat=20
:: NAT or local
set NAT=false

set cirrus=%cd%\Engine\Source\Programs\PixelStreaming\WebServers\SignallingWebServer\cirrus
set Path=%Path%;%cd%\Engine\Source\Programs\PixelStreaming\WebServers\SignallingWebServer;%cd%\Engine\Source\Programs\PixelStreaming\WebRTCProxy\bin;%cd%\Engine\Source\ThirdParty\WebRTC\rev.23789\programs\Win64\VS2017\release
pushd %~dp0

:: stop all first
call down.bat
timeout /T 1
@echo on

if %NAT%==true (
	:: run SignallingWebServer
	@start "%Game%SignallingWebServer" node %cirrus% --publicIp %publicIp% --httpPort %signallingPort% --proxyPort %webRTCPort% --heartbeat %cirrusHeartbeat% --HomepageFile=custom_html/PixelDemo.htm --stunPort %stunPort% --turnPort %turnPort%
) else (
	:: run SignallingWebServer
	@start "%Game%SignallingWebServer" node %cirrus% --publicIp %publicIp% --httpPort %signallingPort% --proxyPort %webRTCPort% --heartbeat %cirrusHeartbeat% --HomepageFile=custom_html/PixelDemo.htm
)

:: run WebRTCProxy
start "%Game%WebRTCProxy" WebRTCProxy.exe -LocalTime -Heartbeat=%proxyHeartbeat% -Cirrus=%publicIp%:%webRTCPort% -UE4Port=%gamePort%

@popd
@pushd %~dp0

:: start Game with PixelStreaming
start "Game" %Game%.exe -dx11 -RenderOffScreen -PixelStreamingPort=%gamePort% -NvEncFrameRateNum=%frameRate% -NvEncAverageBitRate=%averageBitRate% -Heartbeat=%ue4Heartbeat% -nosound

@popd
cd ..
