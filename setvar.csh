# csh Script for setting up a secondo environment
#
# The first argument is interpreted as SECONDO_BUILD_DIR
# copy this file to an apropriate location and set up
# an shell alias in your .cshrc file:
#
#    alias setvar  source <newpath>/setvar.bash
#
# Adjust the directory entries in the configurable
# part.
#
# if you want this script executed at startup of the shell
# add the command 
#
#    source <newpath>/setvar.csh <secondo_directory>
#
# in your login or login_user file.
#
# Note: This file is not suitable for setting the environment
# on windows. You can only use the setvar.bash there.

if (! $?FIRST_CALL) then
  setenv FIRST_CALL "true"

if (! $?COPY_OF_PATH) then
  if (! -$?PATH) then
    setenv COPY_OF_PATH "."
  else
    setenv COPY_OF_PATH "$PATH"
  endif
endif

if (! $?COPY_OF_LD_PATH) then
  if (! $?LD_LIBRARY_PATH) then
    setenv COPY_OF_LD_PATH "."
  else
    setenv COPY_OF_LD_PATH "$LD_LIBRARY_PATH"
  endif
endif

endif

setenv SECONDO_BUILD_DIR "$argv[1]"

#########################################################
##
## start configurable part
##
#########################################################

setenv SECONDO_PLATFORM "solaris"

setenv SECONDO_CONFIG "$HOME/SecBase/MyConfig.ini"

setenv BERKELEY_DB_DIR "/usr/local/Berkeley-DB"

setenv CVSROOT=":pserver:spieker@robinson.fernuni-hagen.de:2401/cvs-projects/CVS_REPOS"

setenv PL_INCLUDE_DIR="/Programme/pl/include"
setenv PL_LIBRARY_DIR="/Programme/pl/lib"

#########################################################
##
## end of configurable part
##
#########################################################

setenv PATH $COPY_OF_PATH":$SECONDO_BUILD_DIR/bin"

setenv LD_LIBRARY_PATH $COPY_OF_LD_PATH":$SECONDO_BUILD_DIR/lib:$BERKELEY_DB_DIR/lib"
