#!/bin/sh
# install.bash - untar files and run make for
# various software packages
#
# 04/19/05 M. Spiekermann
# 04/22/09 M. Spiekermann, error handling for starting xterm &
# 05/16/02 M. Spiekermann, MSYS-Mingw and Linux script merged into this version
# 05/12/10 M. Spiekermann, Compilation of 3d party packages improved and uninstall function added  
# 05/18/10 M. Spiekermann, some additonal messages. Improvements for win32 installation  

# include function definitions
# libutil.sh must be in the search PATH 
if ! source ./scripts/libutil.sh; then 
  printf "%s\n" "This script needs routines from the file ./scripts/libutil.sh"
  exit 1; 
fi

xterm=$(which rxvt)
if [ -z $xterm ]; then
  xterm=$(which xterm)
  if [ -z $xterm ]; then
    printf "%s\n" "Warning: No grapichal console like  rxvt or xterm avaliable."
  else
    xterm=""
  fi
fi

# some function declarations
function copyConfigFiles() {

  printSep "Copying configuration files ..."
  make -C $build SECONDO_SDK=$sdk platform=$platform -f makefile.cm update-environment
  printf  "\n\n%s\n\n" "Proceed with the installation guide ..."
}


function showBashrcMsg {

  printx "%s\n"   "* Before compiling SECONDO run \"source \$HOME/.secondorc"
  printx "%s\n"   "* For convenience you may store the command in \$HOME/.bashrc"
  printx "%s\n"   "* in order to execute it automatically at startup of a new shell."
}


# $1 = package info
# $2 = package file 
# $3 = compilation dir 
# $4 = make targets
# $5 = configure options
# $6 = optional configure cmd
#
function installPackage {

  local configVars="CFLAGS=-I$sdk/include LDFLAGS=-L$sdk/lib"
  local conf="./configure"
  if [ "$6" != "" ]; then
    conf=$6
  fi

  local instOK="$sdk/.INST_OK_$2"
  if [ ! -e $instOK ]; then
    
    # extract files for package
    uncompress $2 $temp
    if [ $? -eq 0 ]; then
      touch $instOK;
    fi

    # compile package if necessary
    if [ "$3" != "" ]; then
      printSep "Compiling package $1 ..."
      cd "$2"
      checkCmd "$conf --prefix=$sdk $5 --disable-nls $configVars"  
      checkCmd "make"
      for target in $4; do
        checkCmd "make $target"
      done
    fi
  else
    printx "%s\n" "Package $1 seems to be already installed!"
  fi
}

# $1 = package info
# $2 = mode = [ dir, files ]
# $3 [ $* ] files or dir  
function uncompressPackage {

  if [ $# -ne 3 ]; then
    return 1;
  fi

  pintx "%s\n" "Installing package $1"

  local mode=$2
  shift
  shift
  
  # uncompress all zip files of a given directory
  if [ "$mode" == "dir" ]; then
    local dir="$*"
    local folder=${dir##*/}
    local checkFile=$sdk/.INST_OK_DIR_$folder
    if [ ! -e $checkFile ]; then
      uncompressFolders $dir
      if [ $? -eq 0 ]; then
        touch $checkFile
      fi
    fi
  fi

  # uncompress all given files
  if [ "$mode" == "files" ]; then
    local checkFile=$sdk/.INST_OK_FILES_$1
    if [ ! -e $checkFile ]; then
      uncompressFiles $sdk $*
      if [ $? -eq 0 ]; then
        touch $checkFile
      fi
    fi
  fi

  # uncompress a single file
  local checkFile=$sdk/.INST_OK_$1
  if [ ! -e $checkFile ]; then
    uncompressFiles $sdk $*
    if [ $? -eq 0 ]; then
      touch $checkFile
    fi
  fi

}


function unInstall {

    dirs="$sdk $temp $build"
    if win32Host; then
      if [ -e $mingwdir ]; then
        dirs="$dirs $mingwdir"
      fi
    fi 
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
    printf "\n%s\n" "Deleting files ..." 
    for xdir in $dirs $HOME/.secondo*rc; do
      rm -rf $xdir
    done
}

# $1 requested version number
function checkGCC {

  checkCmd "gcc --version >> $logfile"
  checkCmd "gcc --print-search-dirs >> $logfile"
  checkCmd "env >> $logfile"

  local gccVersion=$(gcc --version | sed -ne '1 s#[^0-9]*\([\.0-9]*\)#\1#g p')
  if [ "$gccVersion" != "$1" ]; then
    printx "%s\n" "Version number of gcc must be "$1" but gcc --version returns $gccVersion! Giving up."
    abort;
  fi
}

function abort {

  if isRunning $xtermPID; then
    killProcess $xtermPID
  fi
  if isRunning $j2XtermPID; then
    printx "\n%s\n" "Waiting for child process $j2XtermPID"
    wait $j2XtermPID
  fi
  exit $?
}

function finish {

  showBashrcMsg
  abort
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
if win32Host; then

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
  platformdir=$PWD/win32
  encoding="CP1252"	
 
else

  instpath=$HOME
  if [ "$testMode" == "true" ]; then
    HOME=$HOME/tmp_home_dir
    instpath=$HOME
  fi
  platformdir=$PWD/linux
  encoding="LAT1"

fi

if [ "$testMode" == "true" ]; then
  printf "\n%s\n" "(!) Running in test mode. \$HOME set to \"$HOME\" "
fi

# set variables for important directories
cdpath=$PWD
sdk=$instpath/secondo-sdk
temp=$HOME/temp-build
build=$HOME/secondo
prologdir=$sdk/pl

printf "\n%s\n" "*** Installation of the SECONDO DEVELOPMENT TOOLKIT ***" 
printf "\n%s\n" "    Installation source: $cdpath"
printf "%s\n"   "    Target for tools   : $sdk"
printf "%s\n"   "    Target for SECONDO : $build"
printf "%s\n"   "    Temporary directory: $temp"
printf "%s\n"   "    Recognized platform: $platform"

for xdir in "$sdk" "$prologdir" "$sdk/bin" "$temp" "$build"; do
  if [ -d $xdir ]; then
    printf "\n%s\n" "WARNING: Directory $xdir already exists."
  fi 
done

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
  if win32Host; then
    mkdir -p $instpath
  fi
fi


# create directories and logfile
for xdir in "$sdk/bin" "$temp"; do
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

# On windows we need to install unzip first
if win32Host; then
  export PATH="$sdk/bin:$sdk/lib:/c/mingw/bin:$PATH"

  if [ ! -e $sdk/bin/unzip.exe ]; then
    printSep "Installing unzip ..."
    cd $sdk/bin
    checkCmd "$platformdir/non-gnu/unzip/unz550xN.exe" 
    checkCmd "unzip -q -o $platformdir/non-gnu/unzip/zip23xN.zip"
  fi
fi

printSep "SECONDO Source Files"
if [ ! -d $HOME/secondo ]; then
  printx "\n%s\n" "Uncompressing SECONDO source files ..."
  srcfile=$cdpath/secondo-*${encoding}.*
  uncompress $srcfile $HOME
  if [ $? -ne 0 ]; then
    printx "\n%s\N" "Can't extract Secondo's sources. Please download them from \"www.informatik.fernuni-hagen.de/secondo\" and put the zip or tar.gz archive into this directory."
    abort
  fi
else
  printx "\n%s\n" "Source directory is already present!"
fi

printSep "JAVA SDK"
if [ ! -e $sdk/j2sdk*/LICENSE ]; then

  j2dir=$cdpath/$platform/j2sdk
  if [ ! -e $j2dir ]; then
    printx "\n%s\n" "Warning: The script needs Sun's J2SDK installation kit in directory $j2dir."
    printx "%s\n"   "But this directory is not present. Hence this script will not install"
    printx "%s\n"   "a JAVA-SDK. Please install it later manually. Depending on which version"
    printx "%s\n"   "will be been installed adjust the variable \$J2SDK_ROOT in the file \$HOME/.secondo${platform}rc"
  else
    printx "\n%s\n" "Installing Java SDK ..."
    if win32Host; then
      cd $j2dir
      checkCmd j2sdk*windows*.exe
    else
      cd $sdk
      j2file=$j2dir/j2sdk*.bin
      if [ -n $xterm ]; then
        $xterm -title "JAVA 2 Installation" -e $j2file &
        j2XtermPID=$!
      else
        $j2file
      fi
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


if win32Host; then

  printSep "Starting MinGW Installer ..."
  cd $platformdir/mingw
  checkCmd Min*.exe

  if [ ! -d "$mingwdir" ]; then
     printx  "\n%s\n" " WARNING: Recommended installation directory $mingwdir not found." 
     printx  "%s\n"   "          You may need to configure .secondo.win32rc."
  fi
   
  printSep  "Starting SWI-Prolog Installer ..."
  cd $platformdir/prolog
  checkCmd w32pl*.exe

  if [ ! -d "$prologdir" ]; then
     printx  "\n%s\n" "WARNING: Recommended installation directory $prologdir not found." 
     printx  "%s\n"   "         You may need to configure the .secondo.win32rc."
  fi

  printSep "Silent installation of other tools ..."
  cd $sdk
  uncompressPackage "GNU Tools: bison, etc." "dir" $platformdir/gnu
  uncompressPackage "JPEG-Library" "files" $platformdir/non-gnu/jpeg-*
  uncompressPackage "Flex" $platFormdir/non-gnu/flex-*
  uncompressPackage "CVS-Client" $platformdir/non-gnu/cvs-*

  if [ -n $xterm ]; then
    $xterm -title "Make's messages for building Berkeley-DB" -e tail -f $logfile &
    xtermPID=$!
  fi

  printx "\n\n"
  installPackage "Berkeley-DB" $platformdir/non-gnu/db-* $temp/db-*/build_unix install "--enable-cxx --enable-mingw" ../dist/configure

  finish

fi

if [ ! -d $temp/gcc* ]; then
cd $temp
printSep "Uncompressing 3d-party tools ..."
uncompressFolders "$platformdir/gnu" "$platformdir/non-gnu" "$platformdir/prolog"
echo -e "\n\n"
fi

$xterm -title "Installation Protocol" -e tail -f $logfile & 
xtermPID=$!
sleep 1
if ! isRunning $!; then
  printx "\n%s\n" "Warning: Could not start $xterm in backgound."
  printx "%s\n\n" "Make's output will not be displayed but kept in $logfile."
fi

cd $platformdir
installPackage "GCC with C++ support" $temp/gcc-* c++ "bootstrap install"

# set path for using gcc 3.2 and check version
export PATH=".:$sdk/bin:$PATH"

checkGCC "3.2.3"

printx "\n%s\n" "Compiling packages ..."

cd $platformdir
installPackage "Berkeley-DB" non-gnu/db-* $temp/db-*/build_unix install --enable-cxx ../dist/configure
cd $platformdir
installPackage "Lib curses" non-gnu/ncurses-* $temp/ncurses-* install
cd $platformdir
installPackage "Lib readline" gnu/readline-* $temp/readline-*  install --with-curses
cd $platformdir
installPackage "SWI-Prolog" non-gnu/pl-* $temp/pl-* install
cd $platformdir
installPackage "Lib jpeg" non-gnu/jpeg-* $temp/jpeg-*  "install install-lib" 
cd $platformdir
installPackage "Bison, a parser generator" gnu/bison-* $temp/bison-* install 
cd $platformdir
installPackage "Flex, a scanner generator" gnu/flex-* $temp/flex-* install

finish
