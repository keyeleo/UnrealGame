-- local VictoryBPFunctionLibrary=import('VictoryBPFunctionLibrary')

Config={}

function Config:instance()
	if self._instance == nil then
		local config = {}
		config.webHost=""
		config.mqttHost="127.0.0.1:1883"
		-- config.mqttHost='graphic.biad.com.cn:39017'
		config.mqttClientID="hellovic"

		-- config.webHost=VictoryBPFunctionLibrary.VictoryGetCustomConfigVar_String('Server', 'WebServer', true, '')
		-- config.mqttHost=VictoryBPFunctionLibrary.VictoryGetCustomConfigVar_String('MQTT', 'MqttHost', true, '')
		-- config.mqttClientID=VictoryBPFunctionLibrary.VictoryGetCustomConfigVar_String('MQTT', 'MqttClientID', true, '')
	    print("mqttHost="..config.mqttHost)

		self._instance=config
	end
	return self._instance
end
