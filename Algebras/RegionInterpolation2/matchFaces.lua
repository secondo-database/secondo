function matchFaces (src, dst, depth, args)
    print("\nLUA Start with args "..args)
    local default = "Overlap:30" -- the default-strategy
    local ret = {}, key, reg
    
    print("\nDepth "..depth..": Srcregs " .. #src .. ", Dstregs " .. #dst)

    print("Offset src "..srcoff.x.."/"..srcoff.y.."  Scale src "..srcscale.x.."/"..srcscale.y)
    print("Offset dst "..dstoff.x.."/"..dstoff.y.."  Scale dst "..dstscale.x.."/"..dstscale.y)
    
    print("Sregs:");
    for key,reg in pairs(src) do
	pt = middle(reg)
	b1,b2 = boundingbox(reg)
	print("SReg "..key..": "..pt.x.." / "..pt.y..
	      "  BBox ".. b1.x .. "/"..b1.y.." "..b2.x.."/"..b2.y)
    end
    
    print("Dregs:");
    for key,reg in pairs(dst) do
	pt = middle(reg)
	b1,b2 = boundingbox(reg)
	print("DReg "..key..": "..pt.x.." / "..pt.y..
	      "  BBox "..b1.x.."/"..b1.y.." "..b2.x.."/"..b2.y)
    end

    if (args == "") then args = default end

    local argv = {}
    local i = 1
    for w in string.gmatch(args, "%w+") do
	argv[i] = w
	i = i + 1
    end

    local funname = "matchFaces"..(argv[1] or default)
    table.remove(argv, 1)
    
    local status, err = pcall(
    function () 
	dofile("matchStrategies.lua")
	ret = _G[funname](src, dst, depth, unpack(argv))
    end
    );

    if err then
	print("An error occurred: " .. err)
    end

    for idx,r in pairs(ret) do
	local s,d = middle(r.src),middle(r.dst)
        if s ~= nil and d ~= nil then
	    print("Matched " .. s.x .. "/" .. s.y .. " with " .. d.x .. "/" .. d.y)
        end
    end

    return ret;
end

