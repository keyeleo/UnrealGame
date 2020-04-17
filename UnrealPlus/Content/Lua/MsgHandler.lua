require('Cache')
require('Config')
require('WebClient')
local json=require('common/json')

Topic={
	system	="sgcc/system/v1/1",		--system informations
	thing	="sgcc/thing/v1/1",		--things
	hotfix	="sgcc/hotfix/v1/1",		--hotfix things
	data	="sgcc/data/v1/1",		--real-time data
	warn	="sgcc/warn/v1/1",		--warning data
}

MsgHandler={}

function MsgHandler.handle(topic, message)
	print("------- Received: topic=",topic)
    print("------- Received: message=", message)
    if topic==Topic.system then
    	local system=json.decode(message)
	    print("------- system: message=", system)

    elseif topic==Topic.thing then
    elseif topic=='test/2' then
    	local vic=json.decode(message)
	    print("------- test: name=", vic.name, ", message=", vic.message)

		local config=Config:instance()
		local url=config.webHost..'/api/sysinit/1'
	    WebClient:request(url)
    end
end
