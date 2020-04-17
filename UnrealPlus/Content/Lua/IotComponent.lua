

local actor={}

-- override event from blueprint
function actor:ReceiveBeginPlay()
    self.bCanEverTick = true
    self.bCanBeDamaged = false
    self.Super:ReceiveBeginPlay()
end

function actor:Tick(dt)

end

function actor:Say(word)
    print("===== say from lua: ",word)
    -- self:Jump(11.8)
    -- self:Turn(30)

    return word
end

return actor