# bash Script for setting up a secondo environment
#
# The first argument is interpreted as SECONDO_BUILD_DIR
# copy this file to an apropriate location and set up
# an shell alias in your .bashrc file:
#
#    alias setvar  source <newpath>/setvar.bash
#
# Adjust only the directory entries in the configurable
# part.
#
# if you want this script executed at startup of the shell
# add the command 
#
#    source <newpath>/setvar.bash <secondo_directory>
#
# in your profile.

if [ -z "$FIRST_CALL" ]; then
  export FIRST_CALL="true"

if [ -z "$COPY_OF_PATH" ]; then
  if [ -z "$PATH" ]; then
    export COPY_OF_PATH="."
  else
    export COPY_OF_PATH="$PATH"
  fi
fi

if [ -z "$COPY_OF_LD_PATH" ]; then
  if [ -z "$LD_LIBRARY_PATH" ]; then
    export COPY_OF_LD_PATH="."
  else
    export COPY_OF_LD_PATH="$LD_LIBRARY_PATH"
  fi
fi

fi

export SECONDO_BUILD_DIR=$1

if [ "$OSTYPE" = "msys" ]; then
   export SECONDO_PLATFORM="win32"
else 
   export SECONDO_PLATFORM="linux"
fi

SECONDO_SDK="$HOME/secondo-sdk"

#########################################################
##
## start configurable part
##
#########################################################

export SECONDO_CONFIG="$1/bin/SecondoConfig.ini"

export JNI_INIT="$1/bin/JNI.ini"

export CVSROOT=":pserver:$CVSUSER@robinson.fernuni-hagen.de:2401/cvs-projects/CVS_REPOS"

if [ $SECONDO_PLATFORM != "win32" ]; then 
   
   # unix
   export PL_INCLUDE_DIR="$SECONDO_SDK/include"
   export PL_LIB_DIR="$SECONDO_SDK/lib/pl-5.0.10/runtime/i686-linux"
   export J2SDK_ROOT="$SECONDO_SDK/j2sdk1.4.2_01"
else 

   # windows 
   # It is important to start with C:/ to use / as separator!
   export SWI_HOME_DIR="C:/Programme/pl"
   export J2SDK_ROOT="C:/j2sdk1.4.2"
fi


#########################################################
##
## end of configurable part
##
#########################################################

if [ $SECONDO_PLATFORM != "win32" ]; then
   export BISON_SIMPLE="$SECONDO_SDK/share/bison/bison.simple"
   export DVI_VIEWER=kdvi
   export BERKELEY_DB_DIR="$SECONDO_SDK"
   export PATH=".:$J2SDK_ROOT/bin:$SECONDO_SDK/bin:$COPY_OF_PATH"
   export LD_LIBRARY_PATH=".:$J2SDK_ROOT/jre/lib/i386:$J2SDK_ROOT/jre/lib/i386/client:$COPY_OF_LD_PATH:$SECONDO_BUILD_DIR/lib:$BERKELEY_DB_DIR/lib:$PL_LIBRARY_DIR:$JNIDIR" 
else
   export DVI_VIEWER=yap
   export BERKELEY_DB_DIR="/usr/local"
   SWI_HOME_DIR2=$(echo $SWI_HOME_DIR | awk 'BEGIN {FS=":"}; /:/ {print "/"tolower($1)$2}')
   J2SDK_ROOT2=$(echo $J2SDK_ROOT | awk 'BEGIN {FS=":"}; /:/ {print "/"tolower($1)$2}')
   export PATH=".:$J2SDK_ROOT2/bin:$J2SDK_ROOT2/jre/bin/client:$COPY_OF_PATH:$SECONDO_BUILD_DIR/lib:$BERKELEY_DB_DIR/lib:$SWI_HOME_DIR2/bin"
  
   # gcc on windows needs special treatment
   export CPLUS_INCLUDE_PATH="/usr/local/include"
fi

export PD_HEADER="$SECONDO_BUILD_DIR/Tools/pd/pd.header"
export PATH=$PATH:"$SECONDO_BUILD_DIR/Tools/pd"
