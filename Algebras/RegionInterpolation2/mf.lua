
function matchFaces (src, dst)
	print("X " .. offset.x .. " Y " .. offset.y);
	ret = {}
	c = 1;
	for i,reg in ipairs(src) do
		ret[c] = {}
		ret[c]["src"] = reg
		ret[c]["dst"] = dst[i]
		c = c + 1
	end

	return ret
end
