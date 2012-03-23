


if [ ! -e $SECONDO_BUILD_DIR/Tools/findInc/findInc.exe ];then
   make -C $SECONDO_BUILD_DIR/Tools/findInc  2>&1 >/dev/null
fi

$SECONDO_BUILD_DIR/Tools/findInc/findInc $1 
