--------------------------------------------------------------------------------
--      Copyright (c) 2015 - 2016 , 蒙占志(topameng) topameng@gmail.com
--      All rights reserved.
--      Use, modification and distribution are subject to the "MIT License"
--------------------------------------------------------------------------------
local create = coroutine.create
local running = coroutine.running
local resume = coroutine.resume
local yield = coroutine.yield
local error = error
local unpack =unpack or table.unpack
local debug = debug
local FrameTimer = FrameTimer
local CoTimer = CoTimer
local Time=CS.UnityEngine.Time
local comap = {}
local pool = {}
setmetatable(comap, {__mode = "kv"})

function coroutine.start(f, ...)	
	local co = create(f)
	
	if running() == nil then
		local flag, msg = resume(co, ...)
	
		if not flag then					
			error(debug.traceback(co, msg))
		end					
	else
		local args = {...}
		local timer = nil
		
		local action = function()												
			local flag, msg = resume(co, unpack(args))			
			timer.func = nil
			table.insert(pool, timer)
	
			if not flag then														
				error(debug.traceback(co, msg))						
			end		
		end
			
		if #pool > 0 then
			timer = table.remove(pool)
			timer:Reset(action, 0, 1)
		else
			timer = FrameTimer.New(action, 0, 1)
		end
		
		comap[co] = timer
		timer:Start()		
	end

	return co
end

function coroutine.wait(t, co, ...)
	local args = {...}
	co = co or running()		
	local timer = nil
		
	local action = function()				
		local flag, msg = resume(co, unpack(args))
		
		if not flag then	
			timer:Stop()						
			error(debug.traceback(co, msg))			
			return
		end
	end
	
	timer = CoTimer.New(action, t, 1)
	comap[co] = timer	
	timer:Start()
	return yield()
end

function coroutine.step(t, co, ...)
	local args = {...}
	co = co or running()		
	local timer = nil
	
	local action = function()						
		local flag, msg = resume(co, unpack(args))
		timer.func = nil
		table.insert(pool, timer)
	
		if not flag then																			
			error(debug.traceback(co, msg))
			return	
		end		
	end
				
	if #pool > 0 then
		timer = table.remove(pool)
		timer:Reset(action, t or 1, 1)
	else
		timer = FrameTimer.New(action, t or 1, 1)
	end

	comap[co] = timer
	timer:Start()
	return yield()
end
-- 原理是每次update执行一次action
--自己增加了超时处理
function coroutine.www(www,timeout,co)			
	co = co or running()			
	local timer = nil			
			
	local action = function()				
			if not www.isDone then
			if timeout == nil then
				return
			elseif timeout.time > 0 then
				timeout.time = timeout.time - Time.deltaTime
				return
			else
				if timeout.dotimeout ~= nil then
					timeout.dotimeout()
				end
				timer:Stop()
				return
			end
		end	
				
		timer:Stop()	
		local flag, msg = resume(co)			
		timer.func = nil
		table.insert(pool, timer)	
			
		if not flag then												
			error(debug.traceback(co, msg))			
			return			
		end				
	end		
				
	if #pool > 0 then
		timer = table.remove(pool)
		timer:Reset(action, 1, -1)
	else	
		timer = FrameTimer.New(action, 1, -1)	
	end
	comap[co] = timer	
 	timer:Start()
 	return yield()
end

function coroutine.stop(co)
	if co~=nil then
		local timer = comap[co]
		if timer ~= nil then
			comap[co] = nil
			timer:Stop()
		end
	end
end