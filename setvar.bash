# csh Script for setting up a secondo environment
#
# The first argument is interpreted as SECONDO_BUILD_DIR

export SECONDO_PLATFORM="win32"

export SECONDO_BUILD_DIR="$1"

export SECONDO_CONFIG="$HOME/SecBase/secondo/bin/MyConfig.ini"

export BERKELEY_DB_DIR="/usr/local/db-4.0.14"

export PATH="$PATH:$SECONDO_BUILD_DIR/bin"

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$SECONDO_BUILD_DIR/lib:$BERKELEY_DB_DIR/lib"

export CVSROOT=":pserver:spieker@robinson.fernuni-hagen.de:2401/cvs-projects/CVS_REPOS"

echo "SECONDO_PLATFORM: " $SECONDO_PLATFORM
echo "SECONDO_BUILD_DIR: " $SECONDO_BUILD_DIR
echo "SECONDO_CONFIG: " $SECONDO_CONFIG
echo "BERKELEY_DB_DIR: " $BERKELEY_DB_DIR
echo "PATH: " $PATH
echo "LD_LIBRARY_PATH: " $LD_LIBRARY_PATH
echo "CVSROOT: " $CVSROOT
