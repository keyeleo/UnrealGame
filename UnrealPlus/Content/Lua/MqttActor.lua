require('common/common')
require('MsgHandler')

local actor={}

function actor:initialize()
	local config=Config:instance()
	local MqttHost=config.mqttHost
	local MqttClientID=config.mqttClientID
	local url=MqttHost

	url='graphic.biad.com.cn:39018'

	local arr=string.split(url,':')
	local mqtt = require 'paho.mqtt'
	self.client = mqtt.client.create(arr[1],arr[2],function(topic, message)
		if MsgHandler then
			MsgHandler.handle(topic, message)
		end
	end)

	self.client:connect(MqttClientID)
	self.client:subscribe({Topic.system, Topic.thing, Topic.hotfix, Topic.data, Topic.warn, "test/2"})
	self.client:publish("test/2","{\"name\":\"vic\",\"message\":\"from unreal\"}")
end

-- override event from blueprint
function actor:ReceiveBeginPlay()
    self.bCanEverTick = true
    self.bCanBeDamaged = false
    self:initialize()
    self.Super:ReceiveBeginPlay()
end

-- override event from blueprint
function actor:ReceiveEndPlay(reason)
    -- call super ReceiveEndPlay
    self.Super:ReceiveEndPlay(reason)
end

function actor:ReceiveTick(dt)
    self.client:handler()
end

return actor