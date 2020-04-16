
import ('GameplayStatics')
local actor={}
-- override event from blueprint
function actor:ReceiveBeginPlay()
    self.bCanEverTick = true
    -- set bCanBeDamaged property in parent
    self.bCanBeDamaged = false
	print("actor:ReceiveBeginPlay")
	self.Super:ReceiveBeginPlay()
end

-- override event from blueprint
function actor:ReceiveEndPlay(reason)
    print("actor:ReceiveEndPlay")
end

function actor:Tick(dt)
    print("actor:Tick",self,dt)
    -- call actor function
    local pos = self:K2_GetActorLocation()
    -- can pass self as Actor*
    local dist = self:GetHorizontalDistanceTo(self)
    print("actor pos",pos,dist)
end

function hello(arg)
	print('hello from ue4:' , arg)
	local testA = import('TestA');
	testA.Print();
end

-- function actor:export_scene()
	-- GameplayStatics.GetAllActorsOfClass(actor,'')
	--local areas = self:GetAllActorsOfClass();


return actor