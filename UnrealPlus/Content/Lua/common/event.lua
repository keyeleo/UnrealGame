--------------------------------------------------------------------------------
--      Copyright (c) 2015 - 2016 , 蒙占志(topameng) topameng@gmail.com
--      All rights reserved.
--      Use, modification and distribution are subject to the "MIT License"
--------------------------------------------------------------------------------

local setmetatable = setmetatable
local xpcall = xpcall
local pcall = pcall
local assert = assert
local rawget = rawget
local error = error
local print = print
local traceback = debug.traceback
local ilist = ilist
local _xpcall = {}
_xpcall.__call = function(self, ...)	
	if jit then
		if nil == self.obj then
			return xpcall(self.func, traceback, ...)					
		else		
			return xpcall(self.func, traceback, self.obj, ...)					
		end
	else
		local args = {...}
			
		if nil == self.obj then
			local func = function() self.func(unpack(args)) end
			return xpcall(func, traceback)					
		else		
			local func = function() self.func(self.obj, unpack(args)) end
			return xpcall(func, traceback)
		end
	end	
end

_xpcall.__eq = function(lhs, rhs)
	return lhs.func == rhs.func and lhs.obj == rhs.obj
end

local function xfunctor(func, obj)	
	return setmetatable({func = func, obj = obj}, _xpcall)			
end

local _pcall = {}

_pcall.__call = function(self, ...)
	if nil == self.obj then
		return pcall(self.func, ...)					
	else		
		return pcall(self.func, self.obj, ...)					
	end	
end

_pcall.__eq = function(lhs, rhs)
	return lhs.func == rhs.func and lhs.obj == rhs.obj
end

local function functor(func, obj)	
	return setmetatable({func = func, obj = obj}, _pcall)			
end

_event = {}
_event.__index = _event

--废弃
function _event:Add(func, obj)
	assert(func)		

	if self.keepSafe then			
		func = xfunctor(func, obj)
	else
		func = functor(func, obj)
	end	

	if self.lock then
		local node = {value = func, _prev = 0, _next = 0}
		table.insert(self.addList, node)
		return node
	else
		return self.list:push(func)
	end	
end

--废弃
function _event:Remove(func, obj)	
	for i, v in ilist(self.list) do							
		if v.func == func and v.obj == obj then
			if self.lock and self.current ~= i then
				table.insert(self.rmList, i)
			else
				self.list:remove(i)
			end

			break
		end
	end		
end

function _event:CreateListener(func, obj)
	if self.keepSafe then			
		func = xfunctor(func, obj)
	else
		func = functor(func, obj)
	end	
	
	return {value = func, _prev = 0, _next = 0}		
end

function _event:AddListener(handle)
	if self.lock then
		table.insert(self.addList, handle)
	else
		self.list:pushnode(handle)
	end
end

function _event:RemoveListener(handle)		
	if self.lock and self.current ~= handle then		
		table.insert(self.rmList, handle)
	else
		self.list:remove(handle)
	end
end

function _event:Count()
	return self.list.length
end	

function _event:Clear()
	self.list:clear()
	self.rmList = {}
	self.addList = {}
	self.lock = false
	self.keepSafe = false
	self.current = nil
end

function _event:Dump()
	local count = 0
	
	for _, v in ilist(self.list) do
		if v.obj then
			print("update function:", v.func, "object name:", v.obj.name)
		else
			print("update function: ", v.func)
		end
		
		count = count + 1
	end
	
	print("all function is:", count)
end

_event.__call = function(self, ...)			
	local _list = self.list	
	self.lock = true
	local ilist = ilist			
	local flag, msg = false, nil

	for i, f in ilist(_list) do		
		self.current = i						
		flag, msg = f(...)
		
		if not flag then
			if self.keepSafe then								
				_list:remove(i)
			end
			self.lock = false		
			error(msg)				
		end
	end	

	for _, i in ipairs(self.rmList) do							
		_list:remove(i)		
	end

	self.rmList = {}
	self.lock = false		

	for _, i in ipairs(self.addList) do
		_list:pushnode(i)
	end

	self.addList = {}
		end

function event(name, safe)
	safe = safe or false
	return setmetatable({name = name, keepSafe = safe, lock = false, rmList = {}, addList = {}, list = list:new()}, _event)				
end

UpdateBeat 		= event("Update", true)
LateUpdateBeat	= event("LateUpdate", true)
FixedUpdateBeat	= event("FixedUpdate", true)
CoUpdateBeat	= event("CoUpdate")				--只在协同使用

local Time = CS.UnityEngine.Time
local UpdateBeat = UpdateBeat
local LateUpdateBeat = LateUpdateBeat
local FixedUpdateBeat = FixedUpdateBeat
local CoUpdateBeat = CoUpdateBeat

--逻辑update
function Update(deltaTime, unscaledDeltaTime)
	--Time:SetDeltaTime(deltaTime, unscaledDeltaTime)
	UpdateBeat()	
end

function LateUpdate()	
	LateUpdateBeat()		
	CoUpdateBeat()		
	--Time:SetFrameCount()
end

--物理update
function FixedUpdate(fixedDeltaTime)
	--Time:SetFixedDelta(fixedDeltaTime)
	FixedUpdateBeat()
end

function PrintEvents()
	UpdateBeat:Dump()
	FixedUpdateBeat:Dump()
end

Event=_event