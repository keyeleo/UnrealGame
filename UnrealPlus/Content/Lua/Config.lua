local VictoryBPFunctionLibrary=import('VictoryBPFunctionLibrary')

Config={}

function Config:instance()
	if self._instance == nil then
		local config = {}
		config.webHost=VictoryBPFunctionLibrary.VictoryGetCustomConfigVar_String('Server', 'WebServer', true, '')
		config.mqttHost=VictoryBPFunctionLibrary.VictoryGetCustomConfigVar_String('MQTT', 'MqttHost', true, '')
		config.mqttClientID=VictoryBPFunctionLibrary.VictoryGetCustomConfigVar_String('MQTT', 'MqttClientID', true, '')
	    print("------- webHost="..config.webHost)
	    print("------- mqttHost="..config.mqttHost)

		self._instance=config
	end
	return self._instance
end
