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


function matchFaces (src, dst, depth, args)
    print("\nLUA Start with args "..args)
    ret = {}

    print("\nDepth "..depth..": Srcregs " .. #src .. ", Dstregs " .. #dst)

    print("Offset src "..srcoff.x.."/"..srcoff.y.."  Scale src "..srcscale.x.."/"..srcscale.y);
    print("Offset dst "..dstoff.x.."/"..dstoff.y.."  Scale dst "..dstscale.x.."/"..dstscale.y);
    
    
    
    print("Sregs:");
    for key,reg in pairs(src) do
	pt = getmiddle(reg);
	b1,b2 = bb(reg);
	print("SReg "..key..": "..pt.x.." / "..pt.y..
	      "  BBox ".. b1.x .. "/"..b1.y.." "..b2.x.."/"..b2.y);
    end
    
    print("Dregs:");
    for key,reg in pairs(dst) do
	pt = getmiddle(reg);
	b1,b2 = bb(reg);
	print("DReg "..key..": "..pt.x.." / "..pt.y..
	      "  BBox "..b1.x.."/"..b1.y.." "..b2.x.."/"..b2.y);
    end

    ret = matchFacesDistance (src, dst, depth)
--    ret = matchFacesOL (src, dst, depth)
--    ret = matchFacesSpecial (src, dst, depth)
    
    print("\nLUA End")

    return ret;
end

function checkOverlap (src, dst)
    i,s,d = overlap(src[1],dst[1])

    return i/s*100,i/d*100
end

function matchFacesOL (src,dst,depth)

    sp,dp = checkOverlap(src, dst)
    print("SP " .. sp .. " DP " .. dp)
    
    return {}
end

function matchFacesNull (src,dst,depth)
    return {}
end

function matchFacesMW (src, dst, depth)
    if (depth == 0) then
	return matchFacesDistance(src, dst, depth)
    else
	return {}
    end
end

function matchFacesSpecial (src, dst, depth)
    ret = {}

    if (depth < 1) then
	return matchFacesDistance(src, dst, depth)
    else
	return { 
--	{ src = src[1] , dst = dst[1] }
--	{ src = src[2] , dst = dst[1] }
	}
    end

    return ret
end

function matchFacesRandom (src, dst, depth)
    ret = {}
    i = 1
    while (#src > 0 and #dst > 0) do
	i1 = math.random(#src)
	i2 = math.random(#dst)
	print("Matching "..i1.." with "..i2);
	ret[i] = { src = src[i1], dst = dst[i2] }
	src[i1] = nil
	dst[i2] = nil
	i = i + 1
    end

    return ret
end

function matchFacesDistance (src, dst, depth)
    ret = {}
    
    nrsrc = #src
    nrdst = #dst

    k = 1
    dist = {}
    for i=1,nrsrc do
	for j=1,nrdst do
	    s = src[i]
	    d = dst[j]
	    dist[k] = {}
	    dist[k].distance = distance(s,d)
	    dist[k].s = s;
	    dist[k].d = d;
	    k = k + 1
	end
    end

    table.sort(dist, function (a,b) return a.distance < b.distance end);

    used = {}

    i = 1
    for x,y in pairs(dist) do
	if (used[y.s] ~= 1 and used[y.d] ~= 1) then
	    ret[i] = { src=y.s, dst=y.d }
	    used[y.s] = 1
	    used[y.d] = 1
	    print (x.." = "..y.distance)
	    i = i + 1
	end
    end
    
    
--    max = (#src > #dst) and #src or #dst;
--    for i=1,max do
--	ret[i] = { src=src[i], dst=dst[i] }
--    end
    
    print("Leaving depth "..depth.."\n");
    return ret
end
