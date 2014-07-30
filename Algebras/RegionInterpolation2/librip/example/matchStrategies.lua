-- matchStrategies.lua
-- This file is included from matchFaces.lua and
-- contains the actual strategy-implementations

function matchFacesNull (src,dst,depth)
    -- Do not create any pairings
    return {}
end

function matchFacesMW (src, dst, depth)
    -- Create pairings on top-level faces, but
    -- don't pair concavities then (McKenney&Webb)
    if (depth == 0) then
	return matchFacesDistance(src, dst, depth)
    else
	return matchFacesNull(src, dst, depth)
    end
end

function matchFacesSpecial (src, dst, depth)
    -- Use this function for temporary tests
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
    -- Assing faces randomly, for testing mainly
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

function matchFacesDistance (src, dst, depth)
    return matchFacesCriterion (src, dst, depth, distance)
end

function overlapx (src, dst)
    local ia,sa,da = overlap(src,dst)
    debug(3, "Overlap: ia: " .. ia .. " sa: " .. sa .. " da: " .. da)

    return 100-((ia/sa*100+ia/da*100)/2)
end


function matchFacesOverlap (src, dst, depth, minoverlap)
    minoverlap = minoverlap or 10 -- default is 10 percent

    debug(2, "Using Overlap with " .. minoverlap .. "% minimum")
    
    return matchFacesCriterion (src, dst, depth, overlapx, 100 - minoverlap)
end

function matchFacesWood (src, dst, depth)
   -- Do not compensate translation and scaling on top-level
   if (depth == 0) then
      srcoff, dstoff, srcscale, dstscale = nil
   end

   -- otherwise, just behave like Overlap with 9% threshold
   return matchFacesOverlap(src, dst, depth, 9)
end

-- Try to find best pairings using a scoring-function
function matchFacesCriterion (src, dst, depth, func, thres)
    ret = {}
    
    nrsrc = #src
    nrdst = #dst

    -- Call the scoring function for each pairing and store the
    -- result in a table
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

    -- Sort this table with ascending score
    table.sort(dist, function (a,b) return a.criterion < b.criterion end);

    used = {}

    -- Try to get a low overall score by prefering pairs at the beginning
    -- of the sorted table
    i = 1
    for x,y in pairs(dist) do
	if (used[y.s] ~= 1 and used[y.d] ~= 1) then
	    ret[i] = { src=y.s, dst=y.d }
	    used[y.s] = 1
	    used[y.d] = 1
	    debug(4, x.." = "..y.criterion)
	    i = i + 1
	end
    end
    
    return ret
end
