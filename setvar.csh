# csh Script for setting up a secondo environment
#
# The first argument is interpreted as SECONDO_BUILD_DIR

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


setenv SECONDO_PLATFORM "solaris"

setenv SECONDO_BUILD_DIR "$argv[1]"

setenv SECONDO_CONFIG "$HOME/SecBase/MyConfig.ini"

setenv BERKELEY_DB_DIR "/usr/local/Berkeley-DB"

setenv PATH $COPY_OF_PATH":$SECONDO_BUILD_DIR/bin"

setenv LD_LIBRARY_PATH $COPY_OF_LD_PATH":$SECONDO_BUILD_DIR/lib:$BERKELEY_DB_DIR/lib"

setenv CVSROOT=":pserver:spieker@robinson.fernuni-hagen.de:2401/cvs-projects/CVS_REPOS"

echo "SECONDO_PLATFORM: " $SECONDO_PLATFORM
echo "SECONDO_BUILD_DIR: " $SECONDO_BUILD_DIR
echo "SECONDO_CONFIG: " $SECONDO_CONFIG
echo "BERKELEY_DB_DIR: " $BERKELEY_DB_DIR
echo "PATH: " $PATH
echo "LD_LIBRARY_PATH: " $LD_LIBRARY_PATH
