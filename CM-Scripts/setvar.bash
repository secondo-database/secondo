# bash Script for setting up a secondo environment
#
# The first argument is interpreted as SECONDO_BUILD_DIR
# copy this file to an apropriate location and set up
# an shell alias in your .bashrc file:

if [ -z $SETVAR_FIRST_CALL ]; then

  export SETVAR_FIRST_CALL="true"
  export COPY_OF_PATH="$PATH"
  export COPY_OF_LD_PATH="$LD_LIBRARY_PATH"

fi

export SECONDO_BUILD_DIR=$1
export SECONDO_CONFIG=${1}/bin/SecondoConfig.ini

# Path declarations for the GCC
export CPLUS_INCLUDE_PATH=${SECONDO_SDK}/include
export LIBRARY_PATH=${SECONDO_SDK}/lib:${1}/lib

# Bison
export BISON_SIMPLE=${SECONDO_SDK}/share/bison/bison.simple

# JNI
export JNI_INIT=${1}/bin/JNI.ini

## The runtime linker uses different variables on linux and windows

PATH=.:${J2SDK_ROOT}/bin:${SECONDO_SDK}/bin:${COPY_OF_PATH}

if [ $SECONDO_PLATFORM != "win32" ]; then

   pathList=${J2SDK_ROOT}/jre/lib/i386
   pathList=${pathList}:${J2SDK_ROOT}/jre/lib/i386/client
   pathList=${pathList}:${COPY_OF_LD_PATH}
   pathList=${pathList}:${1}/lib
   pathList=${pathList}:${SECONDO_SDK}/lib
   pathList=${pathList}:${BERKELEY_DB_DIR}/lib

   LD_LIBRARY_PATH=.:${pathList}

else

   pathList=${J2SDK_ROOT}/jre/bin/client
   pathList=${pathList}:${1}/lib
   pathList=${pathList}:${SECONDO_SDK}/lib
   pathList=${pathList}:${SECONDO_SDK}/pl/bin

   PATH=.:${PATH}:${pathList}
fi

PATH=${PATH}:${1}/Tools/pd

export PATH LD_LIBRARY_PATH

export PD_HEADER=${1}/Tools/pd/pd.header
