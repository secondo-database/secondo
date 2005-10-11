#!/bin/sh
# install.bash - untar files and run make for
# various software packages
#
# 04/19/05 M. Spiekermann
# 04/22/09 M. Spiekermann, error handling for starting xterm &
# 05/16/02 M. Spiekermann, MSYS-Mingw and Linux script merged into this version

# include function definitions
# libutil.sh must be in the search PATH 
if ! source ./scripts/libutil.sh; then 
  printf "%s\n" "This script needs routines from libutil.sh"
  exit 1; 
fi

xterm=$(which rxvt)
if [ -z $xterm ]; then
  xterm=$(which xterm)
  if [ -z $xterm ]; then
    printf "%s\n" "ERROR: Neither rxvt nor xterm avaliable."
    exit 1;
  fi
fi

# some function declarations
function copyConfigFiles() {

  printSep "Copying configuration files ..."
  make -C $build SECONDO_SDK=$sdk platform=$platform -f makefile.cm update-environment
  printf  "\n\n%s\n\n" "Proceed with the installation guide ..."
}


#default options
installJava="true"
testMode="false"
exec=""

declare -i numOfArgs=$#
let numOfArgs++

while [ $numOfArgs -ne $OPTIND ]; do

  getopts "hnt" optKey
  if [ "$optKey" == "?" ]; then
    optKey="h"
  fi

  case $optKey in

   h) showGPL
      printf "\n%s\n" "Usage of ${0##*/}:" 
      printf "%s\n"   "  -h print this message and exit."
      printf "%s\n"   "  -t test mode"
      printf "%s\n"   "  -n do not install Java SDK"
      printf "%s\n"   "The script installs or compiles all 3rd party tools"
      printf "%s\n\n" "needed to compile SECONDO."
      exit 0;;
   
   n) installJava="false";;

   t) exec="echo "
      testMode="true"

  esac

done

printf "\n"
showGPL

# Do some OS-specific settings
if [ $platform != "linux" ]; then

  # set up $USER and $HOME
  USER=$LOGNAME 
  if [ "$USER" == "" ]; then
    USER="nobody"
    HOME=/home/$USER
    if [ ! -d $HOME ]; then
      printf "\n%s\n" "You have no home dir! Creating directory \"$HOME\"."
      mkdir -p $HOME
    fi
  fi

  instpath=/c
  if [ ! -d $instpath ]; then instpath=$HOME; fi # useful for testing on linux
  msysdir="$instpath/msys/1.0"
  mingwdir="$instpath/mingw"
  if [ "$testMode" == "true" ]; then
    HOME=$msysDir/home/$USER
    instpath=$HOME/tmp_drv_c
    msysdir="$instpath/msys/1.0"
    mingwdir="$instpath/mingw"
  fi
  platformdir=$PWD/windows
 
else

  instpath=$HOME
  if [ "$testMode" == "true" ]; then
    HOME=$HOME/tmp_home_dir
    instpath=$HOME
  fi
  platformdir=$PWD/linux

fi

if [ "$testMode" == "true" ]; then
  printf "\n%s\n" "(!) Running in test mode. \$HOME set to \"$HOME\" "
fi



# set variables for important directories
cdpath=$PWD
sdk=$instpath/secondo-sdk
temp=$HOME/temp-build
build=$HOME/secondo


printf "\n%s\n" "*** Installation of the SECONDO DEVELOPMENT TOOLKIT ***" 
printf "\n%s\n" "    Installation source: $cdpath"
printf "%s\n"   "    Target for tools   : $sdk"
printf "%s\n"   "    Target for SECONDO : $build"
printf "%s\n"   "    Temporary directory: $temp"
printf "%s\n"   "    Recognized platform: $platform"

for xdir in "$sdk" "$sdk/pl" "$sdk/bin" "$temp" "$build"; do
  if [ -d $xdir ]; then
    printf "\n%s\n" "WARNING: Directory $xdir already exists."
  fi 
done


printf "\n%s\n" "This procedure will install various 3rd party tools on your computer."
printf "%s\n"   "We assume that you have read the installation guide. What would you like to do now?"
select proceed in "Start Installation." "Abort."; do

  if [ "$proceed" == "Start Installation." ]; then
    break;
  else if [ "$proceed" == "Abort." ]; then
    exit 0;
  fi 
  fi
done

if [ "$testMode" == "true" ]; then
  mkdir -p $HOME
  if [ "$platform" != "linux" ]; then
    mkdir -p $instpath
  fi
fi


# create directories and logfile
for xdir in "$sdk/bin" "$temp" "$build"; do
  if [ ! -d $xdir ]; then
    mkdir -p $xdir
  fi
done
logfile="$temp/secondo-install.log"
touch $logfile
checkCmd_log=$logfile

if [ "$installJava" == "true" ]; then 

  printSep "Installing Java SDK ..."
  if [ "$platform" == "linux" ]; then
    cd $sdk
    $xterm -title "JAVA 2 Installation" -e $cdpath/j2sdk-${platform}/j2sdk*.bin &
    if [ $? -ne 0 ]; then
      printf "%s\n" "Problems starting $xterm. Please install j2sdk by hand."
    fi
  else
    cd $cdpath/j2sdk-$platform
    checkCmd j2sdk*windows*.exe
  fi  

else

  printf "%s\n" "The Java 2 SDK will not be installed. Please adjust the variable"
  printf "%s\n" "\$J2SDK_ROOT in the file \$HOME/.secondo${platform}rc later."
fi

printSep "Uncompressing JCVS, a java cvs client ... "
cd $sdk
checkCmd "tar -xzf $cdpath/extras/jcvs/jcvs*.tgz"


cd $HOME
printSep "Uncompressing SECONDO source files ..."
checkCmd "tar -xzf secondo-*.tgz"

if [ "$platform" != "linux" ]; then

  # run windows specific installation commands and exit
  source $cdpath/scripts/winsetup.sh
  exit 
fi

cd $temp
printSep "Uncompressing 3d-party tools ..."

uncompressFolders "$platformdir/gnu" "$platformdir/non-gnu" "$platformdir/prolog"

$xterm -title "Installation Protocol" -e tail -f $logfile &
if [ $? -ne 0 ]; then
  printf "%s\n" "Problems starting $xterm. Open another shell and run tail -f $logfile"
  printf "%s\n" "in order to monitor the compiler messages."
fi

printf "\n"
printSep "Compiling GCC ... this will take the most time ..."
cd $temp/gcc-*
checkCmd "./configure --prefix=$sdk"
checkCmd "make bootstrap"
checkCmd "make install"

export PATH=".:$sdk/bin:$PATH"
printf "%s\n" "extending \$PATH and testing GCC version"
printf "\n%s\n" "PATH=$PATH"
gcc --version


export CFLAGS="-I$sdk/bin/include" 
export LDFLAGS="-L$sdk/lib"

# $1 = package
# $2 = directory
# $3 = options
#
function installPackage {

  printSep "Compiling package $1 ..."
  cd $2
  checkCmd "../dist/configure --prefix=$sdk $3 CFLAGS=$CFLAGS LDFLAGS=$LDFLAGS" 
  checkCmd "make && make install"

}

#installPackage "Berkeley-DB" $temp/db-*/build_unix --enable-cxx
installPackage "lib curses" $temp/ncurses-*
installPackage "lib readline" $temp/readline-* --with-curses 
installPackage "SWI-Prolog" $temp/pl-* 
installPackage "flex,a scanner generator" $temp/flex-* 
installPackage "bison,a parser generator" $temp/bison-* 
installPackage "lib jpeg" $temp/jpeg-* 

copyConfigFiles

exit
