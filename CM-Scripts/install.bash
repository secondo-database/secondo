#!/bin/sh
# install.bash - untar files and run make for
# various software packages
#
# 04/19/05 M. Spiekermann
# 04/22/09 M. Spiekermann, error handling for starting xterm &
# 05/16/02 M. Spiekermann, MSYS-Mingw and Linux script merged into this version
# 05/12/10 M. Spiekermann, Compilation of 3d party packages improved and uninstall function added  
# 05/18/10 M. Spiekermann, Code structured into many new functions. Easier to test and maintain.   

# include function definitions
libFile="./scripts/bin/libutil.sh"
if ! source $libFile; then 
  printf "%s\n" "This script needs routines from the file $libFile"
  exit 1; 
fi

xterm=$(which rxvt)
if [ -z $xterm ]; then
  xterm=$(which xterm)
  if [ -z $xterm ]; then
    showMsg "warn" "No grapichal console like  rxvt or xterm avaliable."
  else
    xterm=""
  fi
fi

function showBashrcMsg {

  printx "\n"
  showMsg "em" "Note: Before compiling SECONDO run \"source \$HOME/.secondorc\" \n\
For convenience you may store the command in the file \"\$HOME/.bashrc\" \n\
in order to execute it automatically at startup of a new shell. \n"
}


# $1 = package info
# $2 = package file 
# $3 = compilation dir 
# $4 = make targets
# $5 = configure options
# $6 = optional configure cmd
#
function installPackage {

  local conf="./configure"
  if [ "$6" != "" ]; then
    conf=$6
  fi

  printx "%s\n" "Installing package \"$1\" ..."

  local file=${2##*/}
  local instOK="$temp/.INST_OK_PCKG_$file"
  printl "%s\n" "checkFile = $instOK" 
  if [ ! -e $instOK ]; then
    
    # extract files for package
    printx "%s\n" "Uncompressing files ..."
    uncompressFiles $temp $2
    if [ $? -ne 0 ]; then
      return 1
    fi

    # compile package if necessary
    if [ "$3" != "" ]; then
      printx "%s\n" "Compiling package $1 ..."
      cd $3
      checkCmd "$conf --prefix=$sdk $5 --disable-nls $configureFlags"  
      if [ $? -ne 0 ]; then
        return 1;
      fi
      checkCmd "make"
      if [ $? -ne 0 ]; then
        return 1;
      fi
      for target in $4; do
        checkCmd "make $target"
        if [ $? -ne 0 ]; then
          return 1;
        fi
      done
    fi
    checkCmd "touch $instOK"
    showMsg "-> done!"
  else
    showMsg "info" "-> already done!"
  fi

  return 0
}

# $1 package info
# $2 mode = [ dir, files, file ]
# $3 [ $* ] files or dir  
function uncompressPackage {

  if [ $# -le 2 ]; then
    return 1;
  fi

  local pckgInfo=$1
  local mode=$2
  shift
  shift

  printx "%s\n" "Installing package \"$pckgInfo\" ..."
  
  # uncompress all zip files of a given directory
  if [ "$mode" == "dir" ]; then
    local dir="$*"
    local folder=${dir##*/}
    local checkFile=$temp/.INST_OK_DIR_$folder
    if [ ! -e $checkFile ]; then
      uncompressFolders $dir
      if [ $? -eq 0 ]; then
        touch $checkFile
        return 0
      else
        return 1
      fi 
    else
     showMsg "info" "-> already done"
    fi
    return 0
  fi

  # uncompress all given files
  if [ "$mode" == "files" ]; then
    local file=${1##*/}
    local checkFile=$temp/.INST_OK_FILES_$file
    if [ ! -e $checkFile ]; then
      uncompressFiles $sdk $*
      if [ $? -eq 0 ]; then
        touch $checkFile
        return 0
      else
        return 1
      fi 
    else
     showMsg "info" "-> already done!"
    fi
    return 0
  fi

  # uncompress a single file
  if [ "$mode" == "file" ]; then
    local file=${1##*/}
    local checkFile=$temp/.INST_OK_FILE_$file
    if [ ! -e $checkFile ]; then
      uncompressFiles $sdk $*
      if [ $? -eq 0 ]; then
	touch $checkFile
	return 0
      else
	return 1
      fi
    else
      showMsg "info" "-> already done"
    fi
    return 0
  else
    showMsg "err" "uncompressPackge - Unknown mode \"$mode\""
    return 1
  fi

}


function unInstall {

    local dirs="$sdk $temp $build"
    if win32Host; then
      if [ -e $mingwdir ]; then
        dirs="$dirs $mingwdir"
      fi
    fi 
    printx "%s\n" "About to delete the following directories and files:"
    for xdir in $dirs $HOME/.secondo*rc; do
      printx "%s\n" "  $xdir"
    done

    local opt1="Delete"
    local opt2="Abort"
    select choice in "$opt1" "$opt2"; do
      if [ $choice == $opt1 ]; then
        break
      else
        abort
      fi
    done

    for xdir in $dirs $HOME/.secondo*rc; do
      printf "%s\n" "Deleting $xdir ..."
      rm -rf $xdir
      LU_LOG_INIT=""
    done
}

# $1 title
# $* options after xterm -e
function startupXterm {

  local title=$1
  shift
  if [ "$*" == "" ]; then
   return 0
  fi

  if [ -n $xterm ]; then
    $xterm -title "$title" -e tail -f $logfile &
    lastPID=""
    sleep 1
    if ! isRunning $!; then
      showMsg "err" "Could not start \"$xterm -e $*\" in backgound." 
      return 1 
    fi
    lastPID=$!
  else
    showMsg "err" "No graphical console present!" 
    return 1
  fi
  return 0
}


# $1 requested version number
function checkGCC {

  checkCmd "gcc --version >> $logfile"
  checkCmd "gcc --print-search-dirs >> $logfile"

  local gccVersion=$(gcc --version | sed -ne '1 s#[^0-9]*\([\.0-9]*\)#\1#g p')
  if [ "$gccVersion" != "$1" ]; then
    return 1
  fi
  return 0
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

function copyConfigFiles {

  printSep "Copying configuration files ..."
  CheckCmd cp -b $cdpath/scripts/home/.secondo* $HOME
  if win32Host; then
    CheckCmd cp -b $cdpath/scripts/home/.profile $HOME
  fi 
  CheckCmd cp -b $cdpath/scripts/bin/* $sdk/bin

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
      printf "%s\n"   "  -t test mode installing below directory /tmp"
      printf "%s\n"   "The script installs or compiles all 3rd party tools"
      printf "%s\n\n" "needed to compile SECONDO."
      exit 0;;
   
   t) testMode="true"

  esac

done

printf "\n"
showGPL

# Set up variables for important directories
# Do some OS-specific settings
if win32Host; then

  # set up $USER and $HOME
  USER=$LOGNAME 
  if [ "$USER" == "" ]; then
    showMsg "warn" "The variable \$LOGNAME is empty. Trying to create a home directory for user \"nobody\"."
    USER="nobody"
    HOME=/home/$USER
    mkdir -p $HOME
    if [ $? -ne 0 ]; then
      showMsg "err" "Could not create directory \"$HOME\""
      abort
    fi
  fi

  instpath=/c
  if [ "$testMode" == "true" ]; then
    instpath=tmp/installsdk/C
    HOME=$instpath/$msysdir/home/$USER
  fi
  msysdir="$instpath/msys/1.0"
  mingwdir="$instpath/mingw"
  platformdir=$PWD/win32
  encoding="CP1252"
 
else

  if [ "$testMode" == "true" ]; then
    HOME=/tmp/installsdk/$USER
  fi
  instpath=$HOME
  platformdir=$PWD/linux
  encoding="LAT1"

fi

# set variables for important directories
cdpath=$PWD
sdk=$instpath/secondo-sdk
temp=$LU_TMP/installsdk
build=$HOME/secondo
prologdir=$sdk/pl



if [ "$testMode" == "true" ]; then
  showMsg "info" "Running in test mode. \$HOME set to \"$HOME\""
  mkdir -p $HOME
  mkdir -p $instpath
fi

# check if $HOME exists
if [ ! -d $HOME ]; then
  showMsg "err" "You have no home dir! Please create directory \"$HOME\"."
  abort 
fi


# init log file
mkdir -p $temp
logfile="$temp/secondo-install.log"
initLogFile $logfile
if [ $? -ne 0 ]; then
  showMsg "err" "Could not create log file. Giving up!"
  abort
fi
logfile=$LU_LOG

printx "\n%s\n" "*** Installation of the SECONDO DEVELOPMENT TOOLKIT ***" 
printx "\n%s\n" "    Installation source: $cdpath"
printx "%s\n"   "    Target for tools   : $sdk"
printx "%s\n"   "    Target for SECONDO : $build"
printx "%s\n"   "    Temporary directory: $temp"
printx "%s\n\n" "    Recognized platform: $platform"

for xdir in "$sdk" "$prologdir" "$sdk/bin" "$build"; do
  if [ -d $xdir ]; then
    showMsg "warn" "Directory $xdir already exists."
  fi 
done

printx "\n%s\n" "This procedure will install various 3rd party tools on your computer."
printx "%s\n"   "We assume that you have read the installation guide. What would you like to do now?"

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

printx "%s\n" "Starting installation!"
printx "%s\n" "Log information will be written to $logfile"

if [ "$testMode" == "true" ]; then
  mkdir -p $HOME
  if win32Host; then
    mkdir -p $instpath
  fi
fi


# create directories and logfile
#for xdir in "$sdk/bin"; do
#  if [ ! -d $xdir ]; then
#    mkdir -p $xdir
#  fi
#done

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

printSep "Installing SECONDO's Source Files"
if [ ! -d $HOME/secondo ]; then
  printx "%s\n" "Uncompressing source files ..."
  srcfile=$cdpath/secondo-*${encoding}.*
  if [ ! -e $srcfile ]; then
    showMsg "warn" "Can't extract Secondo's sources. Please download them from \n\
\"www.informatik.fernuni-hagen.de/secondo\" and extract the zip or tar.gz archive \n\
into directory \$HOME/secondo."
  else 
    uncompress $srcfile $HOME
  fi
else
  showMsg "info" "-> Source directory is already present!"
fi

printSep "JAVA 2 SDK"
checkFile=$sdk/j2sdk*/LICENSE
if [ ! -e $checkFile ]; then

  j2dir=$cdpath/$platform/j2sdk
  if [ ! -e $j2dir ]; then
    showMsg "warn" "The script needs Sun's J2SDK installation kit in directory \n\
  \"$j2dir\" \n\
But this directory is not present. Hence this script will not install \n\
a JAVA-SDK. Please install it later manually. Depending on which version \n\
will be installed adjust the variable \$J2SDK_ROOT in the file \n\
\$HOME/.secondo.${platform}rc"
  else
    printx "%s\n" "Installing Java SDK ..."
    if win32Host; then
      cd $j2dir
      CheckCmd j2sdk*windows*.exe
    else
      cd $sdk
      j2file=$j2dir/j2sdk*.bin
      if ! startupXterm "JAVA 2 SDK Installation" $j2file; then
       printx "Running $j2file directly"
       checkCmd $j2file
      else
        j2XtermPID=$lastPID
      fi
    fi
  fi

else  
  printx "%s\n" "-> J2SDK seems to be already installed!"
fi


checkFile=$sdk/j2sdk*/LICENSE
if [ -e $cdpath/extras/jcvs/jcvs*.*gz ]; then
  printx "%s\n" "Uncompressing JCVS, a java cvs client ... "
  if [ ! -e $checkFile ]; then
    cd $sdk
    checkCmd tar -xzf $cdpath/extras/jcvs/jcvs*.*gz
    if [ $? -eq 0 ]; then
      touch $checkFile 
    fi
  else
    printx "%s\n" "-> already done!"
  fi
fi

##
## WINDOWS - INSTALLATION
##

if win32Host; then

  printSep "MinGW (GCC 3.2 windows port) Installation"
  cd $platformdir/mingw
  checkFile=$temp/.INST_OK_MINGW
  if [ ! -e $checkFile ]; then 
    checkCmd Min*.exe
  else
    showMsg "info" "-> MinGW seems to be already installed!"
  fi

  if [ ! -d "$mingwdir" ]; then
     showMsg "warn" "Recommended installation directory $mingwdir not found. \ 
You may need to configure .secondo.win32rc."
  else 
     touch $checkFile
     showMsg "-> done!"
  fi
   
  printSep  "SWI-Prolog Installation"
  cd $platformdir/prolog
  checkFile=$temp/.INST_OK_SWIPROLOG
  if [ ! -e $checkFile ]; then
    checkCmd w32pl*.exe
  else
    showMsg "info" "-> SWI-Prolog seems to be already installed!"
  fi

  if [ ! -d "$prologdir" ]; then
     showMsg "Recommended installation directory $prologdir not found. \ 
You may need to configure the .secondo.win32rc."
  else 
     touch $checkFile
     showMsg "-> done!"
  fi

  printSep "Installation of Tools from the gnuwin32 project ..."
  cd $sdk
  uncompressPackage "GNU Tools: bison, etc." "dir"   $platformdir/gnu
  uncompressPackage "JPEG-Library"           "files" $platformdir/non-gnu/jpeg-*
  uncompressPackage "Flex"                   "file"  $platformdir/non-gnu/flex-*
  uncompressPackage "CVS-Client"             "file"  $platformdir/non-gnu/cvs-*

  if ! startupXterm "Messages from make" tail -f $logfile; then
    printx "%s\n" "Messages from make are kept in $logfile"
  else
    xtermPID=$lastPID
  fi

  printSep "Installation of Berkeley-DB"
  installPackage "Berkeley-DB" $platformdir/non-gnu/db-* $temp/db-*/build_unix install "--enable-cxx --enable-mingw" ../dist/configure

  finish

fi

##
## LINUX - INSTALLATION
##

if ! startupXterm "Messages from make" tail -f $logfile; then
  printx "%s\n" "Messages from make are kept in $logfile"
else
  xtermPID=$lastPID
fi


#
# GCC 3.2.3 installation
#
printSep "Installation of GCC 3.2.3"
if checkGCC "3.2.3"; then
  showMsg "info" "Your system's GCC has already version \"3.2.3\" \n\
hence we will not install it again below $sdk"
  export PATH=".:$sdk/bin:$PATH"
else
  # Compile GCC 3.2.3
  gccfiles=$platformdir/gnu/gcc-*
  installPackage "GCC with C++ support" "$gccfiles"  $temp/gcc-* "bootstrap install"
  configureFlags="CFLAGS=-I$sdk/include LDFLAGS=-L$sdk/lib"
  export PATH=".:$sdk/bin:$PATH"
  if ! checkGCC "3.2.3"; then
    showMsg "err" "Something went wrong! gcc --version does not report version 3.2.3" 
    abort
  fi
fi

printSep "Compiling other packages ..."

installPackage "Berkeley-DB"  $platformdir/non-gnu/db-*      $temp/db-*/build_unix install --enable-cxx ../dist/configure
installPackage "Lib curses"   $platformdir/gnu/ncurses-*     $temp/ncurses-*       install
installPackage "Lib readline" $platformdir/gnu/readline-*    $temp/readline-*      install --with-curses
installPackage "SWI-Prolog"   $platformdir/prolog/pl-*       $temp/pl-*            install
installPackage "Lib jpeg"     $platformdir/non-gnu/jpeg*     $temp/jpeg*           "install install-lib" 

installPackage "Bison, a parser generator" $platformdir/gnu/bison-*     $temp/bison-* install 
installPackage "Flex, a scanner generator" $platformdir/non-gnu/flex-*  $temp/flex-* install

finish
