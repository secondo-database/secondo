# Shell script which displays the current 
# environment settings for SECONDO 

# the MSYS sed does not support \n hence we define
nl="\\n"

echo ""
echo "Environment variables used by SECONDO:"
echo "--------------------------------------"
env | grep -e "\(SECONDO_\|LD_LIB\|^PATH\|INCLUDE\|BERKELEY\|PL_\|CVSROOT\)" \
    | sort \
    | sed -e 's#\(.*\)=\(.*\)#\1 = "\2"'${nl}'--#g'
