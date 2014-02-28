function matchFacesOL (src,dst,depth)

    local sp,dp = checkOverlap(src, dst)
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
    local ret = {}

    if (depth < 1) then
	return matchFacesOverlap(src, dst, depth)
    else
	return {
	{ src = src[1] , dst = dst[2] },
	{ src = src[2] , dst = dst[1] }
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
	ret[i] = { src = src[i1], dst = dst[i2] }
	src[i1] = nil
	dst[i2] = nil
	i = i + 1
    end

    return ret
end

function distancex (src, dst)
    return distance(src,dst)
end

function matchFacesDistance (src, dst, depth)
    return matchFacesCriterion (src, dst, depth, distancex)
end

function overlapx (src, dst)
    local ia,sa,da = overlap(src,dst)

    return 100-((ia/sa*100+ia/da*100)/2)
end


function matchFacesOverlap (src, dst, depth, minoverlap)
    minoverlap = minoverlap or 10 -- default is 10 percent

    print("Using Overlap with " .. minoverlap .. "% minimum")
    
    return matchFacesCriterion (src, dst, depth, overlapx, 100 - minoverlap)
end

function matchFacesCloud (src, dst, depth, direction)
    if (depth > 0) then
        return matchFacesOverlap(src, dst, depth, 30)
    end

    degree = tonumber(direction) or 0

    return matchFacesCriterion (src, dst, depth, clouddistance)
end

function clouddistance (src, dst)
   local cs = centroid(src)
   local ds = centroid(dst)

   local dx = cs.x - ds.x
   local dy = cs.y - ds.y

   local xx = math.sin(math.rad(degree))
   local xy = math.cos(math.rad(degree))

   local dist = dx*xx + dy*xy

   print("Dist ".. dist )

   if (dist > 0) then
       return dist
   end
end

function matchFacesWood (src, dst, depth)
   if (depth == 0) then
      srcoff, dstoff, srcscale, dstscale = nil
   end

   return matchFacesOverlap(src, dst, depth, 10)
end

function matchFacesCriterion (src, dst, depth, func, thres)
    ret = {}
    
    nrsrc = #src
    nrdst = #dst

    k = 1
    dist = {}
    for i=1,nrsrc do
	for j=1,nrdst do
	    s = src[i]
	    d = dst[j]
	    val = func(s,d)
	    if (val ~= nil and ((thres == nil) or (val < thres))) then
		dist[k] = {}
		dist[k].criterion = val
		dist[k].s = s
		dist[k].d = d
		k = k + 1
	    end
	end
    end

    table.sort(dist, function (a,b) return a.criterion < b.criterion end);

    used = {}

    i = 1
    for x,y in pairs(dist) do
	if (used[y.s] ~= 1 and used[y.d] ~= 1) then
	    ret[i] = { src=y.s, dst=y.d }
	    used[y.s] = 1
	    used[y.d] = 1
	    print (x.." = "..y.criterion)
	    i = i + 1
	end
    end
    
    return ret
end
