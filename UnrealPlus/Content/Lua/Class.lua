import('common/common')

--'base view'
LView = class:new()
LView.gameObject = nil

--‘register'
function LView:register(key,cls)
	LView[key] = cls
end

--'start'
function LView:start(gameObject)
	self.gameObject=gameObject
end


LDragView=LView:new()

function LDragView:start(gameObject)
	LView.start(self,gameObject)
	gameObject:AddComponent(typeof(CS.pointerEventBridge)):Init(self)
end
