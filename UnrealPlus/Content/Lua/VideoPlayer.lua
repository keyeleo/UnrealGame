
local actor={}

-- override event from blueprint
function actor:ReceiveBeginPlay()
    self.bCanEverTick = true
    self.bCanBeDamaged = false
    self.Super:ReceiveBeginPlay()
    print('----video player begin play')
end

function actor:Tick(dt)
    print('----video player tick---')
end

function actor:ReceiveTick(dt)
    print('----video player tick')
    if not self.initialized then
        self.initialized=true;
        local parent=self: GetOwner();
        print('--- actor=',parent)
        if parent then
            print('---- player=', parent.MediaPlayer, ', texture=', parent.MediaTexture)
        end
    end
end

-- override event from blueprint
function actor:ReceiveEndPlay(reason)
    -- call super ReceiveEndPlay
    print('----video player end play')
    self.Super:ReceiveEndPlay(reason)
end

return actor