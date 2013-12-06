
function matchFaces (src, dst, depth)
	print("\nEntering depth "..depth);
	print("X " .. offset.x .. " Y " .. offset.y);
	print("Srcregs " .. #src .. " and Dstregs " .. #dst)

	ret = {}

	if (depth == 0) then
	    ret = { { src=src[1], dst=dst[1] } }
	end
	
--	return ret
	
	if (depth == 1) then
	    ret = { { src=src[1], dst=dst[1] }, { src=src[2], dst=nil }, { src=src[3], dst=dst[2] } }
	end

	if (depth == 2) then
	    ret = { { src=src[1], dst=dst[1] } }
	end

	print("Leaving depth "..depth.."\n");
	return ret
end
