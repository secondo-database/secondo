#!/bin/sh
# install.bash - untar files and run make for
# various software packages
#
# 04/19/05 M. Spiekermann
# 04/22/09 M. Spiekermann, error handling for starting xterm &
# 05/16/02 M. Spiekermann, MSYS-Mingw and Linux script merged into this version
# 05/12/10 M. Spiekermann, Compilation of 3d party packages improved and uninstall function added  

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

function abort {

  if isRunning $xtermPID; then
    ps
    kill -9 $xtermPID
  fi
  if isRunning $j2XtermPID; then
    printx "\n%s\n" "Waiting for child process $j2XtermPID"
    wait $j2XtermPID
  fi
exit $?

}

#default options
testMode="false"

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
      printf "%s\n"   "The script installs or compiles all 3rd party tools"
      printf "%s\n\n" "needed to compile SECONDO."
      exit 0;;
   
   t) testMode="true"

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
    HOME=$msysdir/home/$USER
    instpath=$HOME/tmp_drv_c
    msysdir="$instpath/msys/1.0"
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


function unInstall {

    dirs="$sdk $temp $build"
    printf "%s\n" "About to delete the following directories and files:"
    for xdir in $dirs $HOME/.secondo*rc; do
      printf "%s\n" "  $xdir"
    done
    printf "%s" "Cancel with Ctrl-C. Deletion starts in 5 seconds!  "
    for ctr in 5 4 3 2 1; do
      printf "$ctr "
      sleep 1
    done
    echo -e "\a\a\a"
    for xdir in $dirs $HOME/.secondo*rc; do
      rm -rf $xdir
    done

}


printf "\n%s\n" "This procedure will install various 3rd party tools on your computer."
printf "%s\n"   "We assume that you have read the installation guide. What would you like to do now?"

opt1="Install"
opt2="Uninstall"
opt3="Abort"
select choice in "$opt1" "$opt2" "$opt3"; do

  if [ "$choice" == "$opt1" ]; then
    break;
  else 
    if [ "$choice" == "$opt2"  ]; then
      unInstall;
      exit $?
    else 
      exit $?;
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
if [ ! -e $logfile ]; then
touch $logfile
fi
checkCmd_log=$logfile
echo "#########################################################" >> $logfile
date >> $logfile
echo "#########################################################" >> $logfile

printSep "SECONDO Source Files"
if [ ! -e $HOME/secondo/makefile ]; then
  cd $HOME
  printSep "Uncompressing SECONDO source files ..."
  checkCmd "tar -xzf $cdpath/secondo-*.*gz"
  if [ $? != 0 ]; then
    printx "\n%s\n" "Can't extract Secondo's sources. Please download them from \"www.informatik.fernuni-hagen.de/secondo\"."
    abort
  fi
fi

printSep "JAVA SDK"
if [ ! -e $sdk/j2sdk*/LICENSE ]; then

  j2dir=$cdpath/$platform/j2sdk
  if [ ! -e $j2dir ]; then
    printx "\n%s\n" "Warning: The script needs Sun's J2SDK installation kit in directory $j2dir."
    printx "%s\n"   "  But this directory is not present. Hence this script will not install"
    printx "%s\n"   "  a JAVA-SDK. Please install it later manually. Depending on which version"
    printx "%s\n"   "  will be been installed adjust the variable \$J2SDK_ROOT in the file \$HOME/.secondo${platform}rc"
  else
    printx "\n%s\n" "Installing Java SDK ..."
    if [ "$platform" == "linux" ]; then
      cd $sdk
      j2file=$j2dir/j2sdk*.bin
      $xterm -title "JAVA 2 Installation" -e $j2file &
      j2XtermPID=$!
    else
      cd $j2dir
      checkCmd j2sdk*windows*.exe
    fi
  fi

else  
  printx "%s\n" "J2SDK seems to be already installed!"
fi


if [ -e $cdpath/extras/jcvs/jcvs*.*gz ]; then
  printSep "Uncompressing JCVS, a java cvs client ... "
  cd $sdk
  checkCmd "tar -xzf $cdpath/extras/jcvs/jcvs*.*gz"
fi


if [ "$platform" != "linux" ]; then

  # run windows specific installation commands and exit
  source $cdpath/scripts/winsetup.sh
  abort  
fi

if [ ! -d $temp/gcc* ]; then
cd $temp
printSep "Uncompressing 3d-party tools ..."
uncompressFolders "$platformdir/gnu" "$platformdir/non-gnu" "$platformdir/prolog"
fi

$xterm -title "Installation Protocol" -e tail -f $logfile & 
xtermPID=$!
sleep 1
if ! isRunning $!; then
  printx "\n%s\n" "Warning: Could not start $xterm in backgound."
  printx "%s\n\n" "Make's output will not be displayed but kept in $logfile."
fi

# $1 = package info
# $2 = directory
# $3 = target lib for testing if compilation is necessary
# $4 = make targets
# $5 = configure options
# $6 = optional configure cmd
#
function installPackage {

conf="./configure"
if [ "$6" != "" ]; then
conf=$6
fi

if [ ! -e $sdk/include/$3 ]; then
  printSep "Compiling package $1 ..."
  cd "$2"
  checkCmd "$conf --prefix=$sdk $5 --disable-nls $configVars"  
  checkCmd "make"
  for target in $4; do
    checkCmd "make $target"
  done
else
  printx "%s\n" "Package $1 seems to be already installed!"
fi

}

installPackage "GCC with C++ support" $temp/gcc-* c++ "bootstrap install"

# set path for using gcc 3.2 and check version
export PATH=".:$sdk/bin:$PATH"
checkCmd "gcc --version >> $logfile"
checkCmd "gcc --print-search-dirs >> $logfile"
checkCmd "env >> $logfile"

gccVersion=$(gcc --version | sed -ne '1 s#\([^0-9]*\)\([\.0-9]*\)#\2#g p')
if [ "$gccVersion" != "3.2.3" ]; then
  printx "%s\n" "Version number of gcc is not correct! Giving up"
  abort;
fi
printx "\n%s\n" "Compiling packages ..."

configVars="CFLAGS=-I$sdk/include LDFLAGS=-L$sdk/lib"

installPackage "Berkeley-DB" $temp/db-*/build_unix db_cxx.h install --enable-cxx ../dist/configure
installPackage "Lib curses" $temp/ncurses-* ncurses install
installPackage "Lib readline" $temp/readline-* readline install --with-curses
installPackage "SWI-Prolog" $temp/pl-* SWI-Prolog.h install
installPackage "Lib jpeg" $temp/jpeg-* jpeglib.h  "install install-lib" 
installPackage "Bison, a parser generator" $temp/bison-* ../share/bison install 
installPackage "Flex, a scanner generator" $temp/flex-* FlexLexer.h install

copyConfigFiles
abort
