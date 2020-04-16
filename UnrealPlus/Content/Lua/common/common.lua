--------------------------------------------------------------------------------
--      Copyright (c) 2015 - 2020 , Vic
--      All rights reserved.
--      Use, modification and distribution are subject to the "MIT License"
--------------------------------------------------------------------------------

--Ancestor of all class
class={}
function class:new(o)
    o = o or {}             --new object or exists
    setmetatable(o,self)    --getmetatable(o)==self(self might not be class)
    self.__index = self     --index from self
    return o
end

common={}

function string.split(input, delimiter)
    input = tostring(input)
    delimiter = tostring(delimiter)
    if (delimiter == '') then return false end
    local pos, arr = 0, { }
    -- for each divider found
    for st, sp in function() return string.find(input, delimiter, pos, true) end do
        table.insert(arr, string.sub(input, pos, st - 1))
        pos = sp + 1
    end
    table.insert(arr, string.sub(input, pos))
    return arr
end

function string.isnull(str)
    return str==nil or #str==0
end

function table.indexof(t, value, iBegin)
    for i = iBegin or 1, #t do
        if t[i] == value then
            return i
        end
    end
    return -1
end

function table.nums( t )
    local count = 0
    for _,_ in pairs( t ) do
        count = count + 1
    end
    return count
end

function math.round(value)
    value = tonumber(value) or 0
    return math.floor(value + 0.5)
end
