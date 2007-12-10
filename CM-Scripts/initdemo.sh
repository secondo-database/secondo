#
# Preconditions:
#
# - We assume that you have alread installed MSYS
# - Moreover, there must be a java runtime environment JDK 1.4.2 or above
#
# If this is true, adjust the variables ROOT and SWI_HOME_DIR
# afterwards always run 
# 
# source initdemo.sh

# Please configure the root path
# C: -> /c
ROOT="/home/Markus/secondo"

export PATH=$PATH:$ROOT/bin

# Prolog Settings
export SWI_HOME_DIR="C:\msys\1.0\home\Markus\secondo"

# SECONDO Settings
export SECONDO_PLATFORM="win32"
export SECONDO_CONFIG="$ROOT/bin/SecondoConfig.ini"

export JPL_JAR=$ROOT/bin/jpl.jar
export JPL_DLL="true"
export PL_DLL_DIR=$ROOT/bin

