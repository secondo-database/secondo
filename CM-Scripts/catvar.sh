# Shell script which displays the current 
# environment settings for SECONDO 

echo ""
echo "Environment variables used by SECONDO:"
echo "--------------------------------------"
env | grep -e "\(SECONDO_\|^LD.\|_DIR\|PL_\|CVSR\)" | sort | sed -e's#\(.*\)=\(.*\)#\1 = "\2"\n--#g'
