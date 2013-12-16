
function matchFacesold (src, dst, depth)
    print("\nDepth "..depth..": Srcregs " .. #src .. ", Dstregs " .. #dst)
    
    ret = {}
    
    if (depth == 0) then
	ret = { { src=src[1], dst=dst[1] } }
    end
    
    
    if (depth == 1) then
	ret = { { src=src[1], dst=dst[2] }, { src=src[2], dst=nil }, { src=src[3], dst=dst[1] } }
    end
    
    if (depth > 1) then
	ret = { { src=src[1], dst=dst[1] } }
    end
    
    print("Leaving depth "..depth.."\n");
    return ret
end

function matchFaces (src, dst, depth)
    ret = {}
    
    print("\nDepth "..depth..": Srcregs " .. #src .. ", Dstregs " .. #dst)

    print("Offset src "..srcoff.x.."/"..srcoff.y.."  Scale src "..srcscale.x.."/"..srcscale.y);
    print("Offset dst "..dstoff.x.."/"..dstoff.y.."  Scale src "..dstscale.x.."/"..dstscale.y);
    
    
    
    print("Sregs:");
    for key,reg in pairs(src) do
	pt = getmiddle(reg);
	b1,b2 = bb(reg);
	print("Reg " .. pt.x .. " / " .. pt.y);
	print("BBox ".. b1.x .. "/"..b1.y.." "..b2.x.."/"..b2.y);
    end
    
    print("Dregs:");
    for key,reg in pairs(dst) do
	pt = getmiddle(reg);
	b1,b2 = bb(reg);
	print("Reg " .. pt.x .. " / " .. pt.y);
	print("BBox ".. b1.x .. "/"..b1.y.." "..b2.x.."/"..b2.y);
    end

    max = (#src > #dst) and #src or #dst;
    for i=1,max do
	ret[i] = { src=src[i], dst=dst[i] }
    end
    
    print("Leaving depth "..depth.."\n");
    return ret
end
