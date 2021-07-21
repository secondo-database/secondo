#!/bin/sh

function assert {
   if [ "$#" == "0" ]; then
     return 0;
   fi
   $*
   RC=$?
   if [ $RC -ne 0 ]; then
      echo "PWD = $PWD"
      echo "Command failed"
      echo "command $* returned $RC"
      exit 1
   fi
   return 0 
}



gcc --version
RC=$?

if [ $RC -ne 0 ]; then
  echo "Please install XCode first"
  echo "You will find XCode in the App Store for free"
  exit 1
fi

START_DIR=$PWD

SDK_DIR=$HOME/secondo-sdk

if [ -e $SDK_DIR ]; then
  echo "The directory $SDK_DIR already exists. "
  echo "Please remove or rename it before you can install this SDK"
  exit 1
fi


assert mkdir -p $SDK_DIR
assert mkdir -p $SDK_DIR/temp

echo "****** readline installation******"

assert  cd $SDK_DIR/temp
assert  cp $START_DIR/linux/gnu/readline* .
assert  tar -xzf readline*
assert  cd readline*
assert  ./configure --prefix="$SDK_DIR" --with-curses
assert make all
assert make install


echo "***** gsl installation *****"
assert cd $SDK_DIR/temp
assert cp $START_DIR/linux/gnu/gsl* .
assert tar -xzf gsl*
assert cd gsl*
assert ./configure --prefix=$SDK_DIR
assert make
assert make install

echo "***** install findutils *****"
assert cd $SDK_DIR/temp
assert cp $START_DIR/linux/gnu/findutils* .
assert tar -xzf findutils*
assert cd findutils*
assert ./configure --prefix=$SDK_DIR
assert make
assert make install

echo "***** install bison **** "
assert cd $SDK_DIR/temp
assert cp $START_DIR/linux/gnu/bison* .
assert tar -xzf bison*
assert cd bison*
assert ./configure --prefix=$SDK_DIR
assert make install

echo "**** install flex *****"
assert cd $SDK_DIR/temp
assert cp $START_DIR/linux/non-gnu/flex* .
assert tar -xzf flex*
assert cd flex*
assert ./configure --prefix=$SDK_DIR
assert make install


echo "**** remove temp dir *****"
assert cd $SDK_DIR
assert rm -rf temp

echo "Copy configuration files"
assert cd $SDK_DIR
assert cp $START_DIR/scripts/secondo* .

if [ ! -d ~/secondo ]; then
echo "**** Downloading the latest SECONDO version ****"
git clone https://github.com/secondo-database/secondo.git ~/secondo
fi


## Arm based CPUs
if [ $(uname -m | grep arm64 | wc -l) -eq 1 ]; then

   SWI_VERSION=$(brew list --versions swi-prolog | tail -1 | cut -d " " -f 2)

cat << EOF
Please ensure that $HOME/.profile contains the following lines

export SECONDO_SDK=\$HOME/secondo-sdk
export SECONDO_PLATFORM=mac_osx
export SECONDO_BUILD_DIR=\$HOME/secondo
source $SECONDO_SDK/secondorc

export LIBREADLINE="$HOME/secondo-sdk/lib/libreadline.a"
export PL_LIB=swipl

export SWI_VERSION=$SWI_VERSION
export PATH=/opt/homebrew/Cellar/swi-prolog/$SWI_VERSION/bin:\$PATH
export SWI_HOME_DIR=/opt/homebrew/Cellar/swi-prolog/$SWI_VERSION/libexec/lib/swipl
export PL_LIB_DIR=/opt/homebrew/Cellar/swi-prolog/$SWI_VERSION/libexec/lib/swipl/lib/arm64-darwin
export PL_DLL_DIR=$PL_LIB_DIR
export PL_LIB=swipl
export PL_INCLUDE_DIR=$SWI_HOME_DIR/include
export PL_VERSION=82000
export JPL_DLL=$SWI_HOME_DIR/lib/arm64-darwin/libjpl.dylib
export JPL_JAR=$SWI_HOME_DIR/lib/jpl.jar

export BDB_DIR=/opt/homebrew
export BERKELEY_DB_DIR=$BDB_DIR
export BERKELEY_DB_LIB=db_cxx
export BDB_INCLUDE_DIR=$BDB_DIR/include
export BDB_LIB_DIR=$BDB_DIR/lib
EOF
  exit 0
fi

## Intel based CPUs

# Integrate SWI prolog
echo "modifying rpath for SWI prolog"
JPL=$(find /Applications -name "libjpl.dylib")

PL=$(find /Applications -name "libswipl.dylib")
SWIPLDIR=$(dirname $PL)

if [ -n "$LD_LIBRARY_PATH" ]; then
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SWIPLDIR
else
  export LD_LIBRARY_PATH=$SWIPLDIR
fi

echo "libjpl.dylib is located at $JPL"
echo "libswipl.dylib is located at $PL ($SWIPLDIR)"

install_name_tool -add_rpath $SWIPLDIR $JPL

if [ -e $HOME/.profile ]; then
  echo "File $HOME/.profile already exists"
  echo " ensure that this files contains the following lines"
  echo 'export PATH=$PATH:/Applications/SWI-Prolog.app/Contents/MacOS'
  echo 'export SECONDO_SDK=$HOME/secondo-sdk'
  echo 'export SECONDO_PLATFORM=mac_osx'
  echo 'export SECONDO_BUILD_DIR=$HOME/secondo'
  echo 'source $SECONDO_SDK/secondorc'
  echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SWIPLDIR"
else
  echo "Create file $HOME/.profile"
  echo 'export PATH=$PATH:/Applications/SWI-Prolog.app/Contents/MacOS'   >>$HOME/.profile
  echo 'export SECONDO_SDK=$HOME/secondo-sdk'   >>$HOME/.profile
  echo 'export SECONDO_PLATFORM=mac_osx'        >>$HOME/.profile
  echo 'export SECONDO_BUILD_DIR=$HOME/secondo' >>$HOME/.profile
  echo 'source $SECONDO_SDK/secondorc'          >>$HOME/.profile
  echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SWIPLDIR" >>$HOME/.profile
fi


