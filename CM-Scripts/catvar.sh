# Shell script which displays the current 
# environment settings for SECONDO 

echo ""
echo "Environment variables used by SECONDO:"
echo "--------------------------------------"
env | grep -e "\(SECONDO_\|LD_LIB\|^PATH\|INCLUDE\|BERKELEY\|PL_\|J2SDK\|CVSROOT\)" \
    | sort \
    | sed -e 's#\(.*\)=\(.*\)#\1 = "\2" \
--#g'
