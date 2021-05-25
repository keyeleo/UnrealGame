:: Scripts to startup PixelStreaming servers

@echo off
set Game=CloudRender
set publicIp=172.18.137.168
set iceIp=172.18.137.168
set signallingPort=80
set streamerPort=8888
set stunPort=19302
set turnPort=19303

set frameRate=60
set averageBitRate=160000
set cirrusHeartbeat=4
set ue4Heartbeat=3
set NAT=false
set cloudrenderGate=http://172.18.137.13:39004

set bin=%cd%\%Game%\Plugins\CloudRender\Source
set cirrus=%bin%\Programs\PixelStreaming\WebServers\SignallingWebServer\cirrus
set Path=%Path%;%bin%\Programs\PixelStreaming\WebServers\SignallingWebServer;%bin%\ThirdParty\WebRTC\rev.23789\programs\Win64\VS2017\release
pushd %~dp0

@echo on

if %NAT%==true (
	:: run SignallingWebServer
	start "%Game%SignallingWebServer" Powershell.exe -executionpolicy unrestricted -File Start_AWS_WithTURN_SignallingServer.ps1 -cirrus %cirrus% -publicIp %publicIp% --httpPort %signallingPort% --streamerPort %streamerPort% -iceIp %iceIp% -stunPort %stunPort% -turnPort %turnPort% -heartbeat %cirrusHeartbeat% -HomepageFile custom_html/PixelDemo.htm
) else (
	:: run SignallingWebServer
	start "%Game%SignallingWebServer" node %cirrus% --publicIp %publicIp% --httpPort %signallingPort% --streamerPort %streamerPort% --heartbeat %cirrusHeartbeat% --HomepageFile=custom_html/PixelDemo.htm
	rem start "%Game%SignallingWebServer" node %cirrus% --cloudrenderGate=%cloudrenderGate% --publicIp %publicIp% --httpPort %signallingPort% --streamerPort %streamerPort% --heartbeat %cirrusHeartbeat% --HomepageFile=custom_html/PixelDemo.htm
)

@popd
@pushd %~dp0

start "Game" %Game%.exe -dx11 -RenderOffScreen -PixelStreamingIP=%publicIp% -PixelStreamingPort=%streamerPort% -NvEncFrameRateNum=%frameRate% -NvEncAverageBitRate=%averageBitRate% -Heartbeat=%ue4Heartbeat% -AllowPixelStreamingCommands

@popd
