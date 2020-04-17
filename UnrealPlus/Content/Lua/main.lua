

local actor={}

-- override event from blueprint
function actor:ReceiveBeginPlay()
    -- self.bCanEverTick = true
    self.bCanBeDamaged = false
    self.Super:ReceiveBeginPlay()

    print('-------- main run');
end

return actor