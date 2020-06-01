local Exporter=import('FFMPEGMediaExporter')

local actor={}

-- override event from blueprint
function actor:ReceiveBeginPlay()
    self.Super:ReceiveBeginPlay()
    local urls={
    	'rtsp://admin:iotree123@192.168.3.5:554/Streaming/Channels/101',
    	'rtsp://admin:iotree123@192.168.3.7:554/Streaming/Channels/101',
    	'rtsp://admin:iotree123@192.168.3.14:554/Streaming/Channels/101',
    	'rtsp://admin:iotree123@192.168.3.17:554/Streaming/Channels/101',
    }
    for i,v in pairs(self.MediaPlayers) do
	    v: OpenUrl(urls[i+1])
    end
    Exporter.Export('rtsp://admin:iotree123@192.168.3.150:554/Streaming/tracks/401?starttime=20200523t080808z&endtime=20200523t080909z','camera.mp4',10);
end

return actor