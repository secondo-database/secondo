
function matchFaces (src, dst, depth)
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
