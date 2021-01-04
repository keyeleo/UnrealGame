# Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

param(
[string]$publicIp=$(throw "Parameter missing: -publicIp PublicIp"),
[int]$httpPort=$(throw "Parameter missing: -httpPort HttpPort"),
[int]$streamerPort=$(throw "Parameter missing: -streamerPort StreamerPort"),
[int]$stunPort=$(throw "Parameter missing: -stunPort StunPort"),
[int]$turnPort=$(throw "Parameter missing: -turnPort TurnPort"),
[int]$heartbeat=$(throw "Parameter missing: -heartbeat Heartbeat"),
[string]$cirrus=$(throw "Parameter missing: -cirrus CirrusPath"),
[string]$HomepageFile=$(throw "Parameter missing: -cirrus CirrusPath")
)


# $publicIp = Invoke-WebRequest -Uri "http://169.254.169.254/latest/meta-data/public-ipv4"

$peerConnectionOptions = "{ \""iceServers\"": [{\""urls\"": [\""stun:" + $publicIp + ":" + $stunPort + "\"",\""turn:" + $publicIp + ":" + $turnPort + "\""], \""username\"": \""PixelStreamingUser\"", \""credential\"": \""Another TURN in the road\""}] }"

$ProcessExe = "node.exe"
$Arguments = @("$cirrus", "--peerConnectionOptions=""$peerConnectionOptions""", "--publicIp=$publicIp", "--httpPort=$httpPort", "--streamerPort=$streamerPort", "--heartbeat=$heartbeat", "--HomepageFile=$HomepageFile")
# Add arguments passed to script to Arguments for executable
# $Arguments += $args

Write-Output "Running: $ProcessExe $Arguments"
Start-Process -FilePath $ProcessExe -ArgumentList $Arguments -Wait -NoNewWindow
