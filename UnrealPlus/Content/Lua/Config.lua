local AssetLibrary=import('AssetLibrary')

Config={}

function Config:instance()
	if self._instance == nil then
		local config = {}
		config.webHost=""
		config.mqttHost="127.0.0.1:1883"
		-- config.mqttHost='graphic.biad.com.cn:39017'
		config.mqttClientID="hellovic"

		-- config.webHost=AssetLibrary.GetGameConfig('WebServer', 'Server', false)
		-- config.mqttHost=AssetLibrary.GetGameConfig('MqttHost', 'MQTT', false)
		-- config.mqttClientID=AssetLibrary.GetGameConfig('MqttClientID', 'MQTT', false)
	    print("mqttHost="..config.mqttHost)

		self._instance=config
	end
	return self._instance
end
