# bash Script for setting up a secondo environment
#
# The first argument is interpreted as SECONDO_BUILD_DIR
# copy this file to an apropriate location and set up
# an shell alias in your .bashrc file:
#
#    alias setvar  source <newpath>/setvar.bash
#
# Adjust the directory entries in the configurable
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

#########################################################
##
## start configurable part
##
#########################################################

export SECONDO_PLATFORM="linux"

export SECONDO_CONFIG=$1"/SecondoConfig.ini"

export BERKELEY_DB_DIR="/usr/local/BerkeleyDB.4.1/"

export CVSROOT=":pserver:behr@robinson.fernuni-hagen.de:2401/cvs-projects/CVS_REPOS"



if [ $SECONDO_PLATFORM != "win32" ]; then 
   
   # unix
   export PL_INCLUDE_DIR=""
   export PL_LIB_DIR=""

else 

   # windows 
   # for the next variable use \ as separator	
   export SWI_HOME_DIR=""
fi

export JNIDIR="/usr/java/j2sdk1.4.2/jre/lib/i386:/usr/java/j2sdk1.4.2/jre/lib/i386/client"

#########################################################
##
## end of configurable part
##
#########################################################

if [ $SECONDO_PLATFORM != "win32" ]; then

   export PATH="$COPY_OF_PATH:$SECONDO_BUILD_DIR/bin"
   export LD_LIBRARY_PATH="$COPY_OF_LD_PATH:$SECONDO_BUILD_DIR/lib:$BERKELEY_DB_DIR/lib:$PL_LIBRARY_DIR:$JNIDIR" 
else
   export PATH="$COPY_OF_PATH:$SECONDO_BUILD_DIR/bin:$SECONDO_BUILD_DIR/lib:$SWI_HOME_DIR\\bin"
fi



