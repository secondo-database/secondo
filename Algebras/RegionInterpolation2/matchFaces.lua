-- matchFaces.lua
-- This is the main Lua-file, which is loaded by default from
-- matchFacesLua.

debuglevel = 0 -- Increase debuglevel for more verbosity

function matchFaces (src, dst, depth, args)
    local default = "Overlap:30" -- the default-strategy
    local ret = {}, key, reg
    
    -- First, print some debugmessages if the debuglevel is set
    -- accordingly
    debug(1, "LUA Start with args "..args)
    debug(1, "Depth "..depth..": Srcregs " .. #src .. ", Dstregs " .. #dst)
    debug(2, "Src: Offset " .. srcoff.x .. "/" .. srcoff.y ..
                 " Scale " ..srcscale.x .. "/" .. srcscale.y)
    debug(2, "Dst: Offset " .. dstoff.x .. "/" .. dstoff.y ..
                 " Scale " ..dstscale.x .. "/" .. dstscale.y)
    debug(3, "Sregs:");
    for key,reg in pairs(src) do
	pt = middle(reg)
	b1,b2 = boundingbox(reg)
	debug(3, "SReg "..key..": "..pt.x.." / "..pt.y..
	         "  BBox ".. b1.x .. "/"..b1.y.." "..b2.x.."/"..b2.y)
    end
    debug(3, "Dregs:");
    for key,reg in pairs(dst) do
	pt = middle(reg)
	b1,b2 = boundingbox(reg)
	debug(3, "DReg "..key..": "..pt.x.." / "..pt.y..
	         "  BBox "..b1.x.."/"..b1.y.." "..b2.x.."/"..b2.y)
    end

    -- Try to parse the arguments. These should be in the format
    -- <strategyname>[:arg1,...,argn]
    if (args == "") then args = default end
    local argv = {}
    local i = 1
    for w in string.gmatch(args, "%w+") do
	argv[i] = w
	i = i + 1
    end
    -- The function name is matchFaces<strategyname>
    local funname = "matchFaces"..(argv[1] or default) 
    table.remove(argv, 1)
    
    -- Now include the file with the strategy-implementations
    -- and try to call the function
    local status, err = pcall(
    function () 
	dofile("matchStrategies.lua")
	ret = _G[funname](src, dst, depth, unpack(argv))
    end
    );

    -- Output any errors here
    if err then
	print("An error occurred: " .. err)
    end

    -- If the debuglevel is high enough, print the generated pairings here
    for idx,r in pairs(ret) do
	local s,d = middle(r.src),middle(r.dst)
        if s ~= nil and d ~= nil then
	    debug(3, "Matched " .. s.x .. "/" .. s.y .. " with " .. d.x .. "/" .. d.y)
        end
    end

    return ret;
end

-- Print debug-messages based on their level and the global debuglevel
function debug (lvl, msg)
    if (lvl <= debuglevel) then
       print(msg)
    end
end
