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

#########################################################
##
## start configurable part
##
#########################################################

export SECONDO_CONFIG="$1/bin/SecondoConfig.ini"

export JNI_INIT="$1/bin/JNI.ini"

export CVSROOT=":pserver:$SECONDO_CVS_USER@zeppelin.fernuni-hagen.de:2401/home/cvsroot"

if [ $SECONDO_PLATFORM != "win32" ]; then 
   
   # unix
   export SWI_HOME_DIR="$SECONDO_SDK/lib/pl-5.0.10"
   export PL_INCLUDE_DIR="$SECONDO_SDK/include"
   export PL_LIB_DIR="$SECONDO_SDK/lib/pl-5.0.10/runtime/i686-linux"
   export J2SDK_ROOT="$SECONDO_SDK/j2sdk1.4.2_01"
else 

   # windows - SWI_HOME_DIR must be specified with drive letter 
   #           and \ as path separator 
   export SWI_HOME_DIR="C:\secondo-sdk\pl"
   export PL_INCLUDE_DIR="$SECONDO_SDK/pl/include"
   export PL_LIB_DIR="$SECONDO_SDK/pl/lib"
   export J2SDK_ROOT="$SECONDO_SDK/j2sdk1.4.2"
fi


#########################################################
##
## end of configurable part
##
#########################################################

## Path declarations for the GCC

export BERKELEY_DB_DIR="$SECONDO_SDK"
export BISON_SIMPLE="$SECONDO_SDK/share/bison/bison.simple"
export PATH=".:$J2SDK_ROOT/bin:$SECONDO_SDK/bin:$COPY_OF_PATH"
export CPLUS_INCLUDE_PATH="$SECONDO_SDK/include"
export LIBRARY_PATH="$SECONDO_SDK/lib:$SECONDO_BUILD_DIR/lib"

## The runtime linker uses different variables on linux and windows
##
if [ $SECONDO_PLATFORM != "win32" ]; then
   export LD_LIBRARY_PATH=".:$J2SDK_ROOT/jre/lib/i386:$J2SDK_ROOT/jre/lib/i386/client:$COPY_OF_LD_PATH:$SECONDO_BUILD_DIR/lib:$SECONDO_SDK/lib:" 
else
   export PATH="$PATH:$J2SDK_ROOT/jre/bin/client:$SECONDO_BUILD_DIR/lib:$SECONDO_SDK/lib:$SECONDO_SDK/pl/bin"
fi

## PD-System

if [ $SECONDO_PLATFORM != "win32" ]; then
   export PD_DVI_VIEWER=kdvi
   export PD_PS_VIEWER=gv
else
   export PD_DVI_VIEWER=yap
   export PD_PS_VIEWER=gsview
fi

export PD_HEADER="$SECONDO_BUILD_DIR/Tools/pd/pd.header"
export PATH="$PATH:$SECONDO_BUILD_DIR/Tools/pd"
