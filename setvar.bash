# bash Script for setting up a secondo environment
#
# The first argument is interpreted as SECONDO_BUILD_DIR

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

export SECONDO_PLATFORM="win32"

export SECONDO_BUILD_DIR="$1"

export SECONDO_CONFIG="/SecBase/secondo/bin/MyConfig.ini"

export BERKELEY_DB_DIR="/usr/local/db-4.0.14"

if [ $SECONDO_PLATFORM != "win32" ]; then
   export PATH="$COPY_OF_PATH:$SECONDO_BUILD_DIR/bin"
   export LD_LIBRARY_PATH="$COPY_OF_LD_PATH:$SECONDO_BUILD_DIR/lib:$BERKELEY_DB_DIR/lib"
else
   # windows looks for dlls in the directories included in the PATH variable
   export PATH="$COPY_OF_PATH:$SECONDO_BUILD_DIR/bin:$SECONDO_BUILD_DIR/lib"	
fi


export CVSROOT=":pserver:spieker@robinson.fernuni-hagen.de:2401/cvs-projects/CVS_REPOS"

echo "SECONDO_PLATFORM: " $SECONDO_PLATFORM
echo "SECONDO_BUILD_DIR: " $SECONDO_BUILD_DIR
echo "SECONDO_CONFIG: " $SECONDO_CONFIG
echo "BERKELEY_DB_DIR: " $BERKELEY_DB_DIR
echo "PATH: " $PATH
echo "LD_LIBRARY_PATH: " $LD_LIBRARY_PATH
echo "CVSROOT: " $CVSROOT
