#!/bin/sh
# install.bash - untar files and run make for
# various software packages
#
# 04/19/05 M. Spiekermann
# 04/22/09 M. Spiekermann, error handling for starting xterm &
# 05/16/02 M. Spiekermann, MSYS-Mingw and Linux script merged into this version

# include function definitions
# libutil.sh must be in the search PATH 
if ! source libutil.sh; then 
  printf "%s\n" "This script needs routines from libutil.sh"
  exit 1; 
fi

#default options
installJava="false"
testMode="false"

declare -i numOfArgs=$#
let numOfArgs++

while [ $numOfArgs -ne $OPTIND ]; do

  getopts "hjat" optKey
  if [ "$optKey" == "?" ]; then
    optKey="h"
  fi

  case $optKey in

   h) showGPL
      printf "\n%s\n" "Usage of ${0##*/}:" 
      printf "%s\n"   "  -h print this message and exit."
      printf "%s\n"   "  -t test mode"
      printf "%s\n"   "  -j Install Java SDK"
      printf "%s\n"   "The script installs or compiles all 3rd party tools except JAVA 2 SDK"
      printf "%s\n\n" "needed to compile SECONDO."
      exit 0;;
   
   j) installJava="true";;

   t) testMode="true"

  esac

done

printf "\n"
showGPL

# Do some OS-specific settings
if [ $platform != "linux" ]; then

  # set up $USER and $HOME
  if [ "$LOGNAME" == "" ]; then
    LOGNAME="nobody"
    USER=$LOGNAME 
  fi
  HOME=/home/$USER
  if [ ! -d $HOME ]; then
    printf "\n%s\n" "You have no user name! Creating directory \"$HOME\"."
    mkdir -p $HOME
  fi

  instpath=/c
  if [ "$testMode" == "true" ]; then
    HOME=$msysDir/home/$USER
    instpath=$HOME/tmp_drv_c
  fi
  platformdir=$PWD/windows
  msysdir="$instpath/msys/1.0"
  mingwdir="$instpath/mingw"

else

  instpath=$HOME
  if [ "$testMode" == "true" ]; then
    HOME=$HOME/tmp_home_dir
    instpath=$HOME
  fi
  platformdir=$PWD/linux

fi

if [ "$testMode" == "true" ]; then

  if [ "$platform" == "linux" ]; then
    mkdir -p $HOME
  else
    msysDir=$instpath/msys/1.0
    mkdir -p $msysdir
    mkdir -p $HOME 
    mkdir -p $msysdir/etc
    mkdir -p $sdk
    mkdir -p $build
  fi
  printf "\n%s\n" "* Running in test mode. \$HOME set to \"$HOME\" "
fi


# set variables for important directories
cdpath=$PWD
sdk=$instpath/secondo-sdk
temp=$HOME/temp-build
build=$HOME/secondo

printf "\n%s\n" "* Installation of the SECONDO DEVELOPMENT TOOLKIT" 
printf "\n%s\n" "  Installation source: $cdpath"
printf "%s\n"   "  Target for tools   : $sdk"
printf "%s\n"   "  Target for SECONDO : $build"
printf "%s\n"   "  Temporary directory: $temp"

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

if [ "$installJava" == "true" ]; then 

  printf "\n%s\n" "* Installing Java SDK ..."
  if [ "$platform" == "linux" ]; then

    mkdir -p $sdk
    mkdir -p $temp

    cp $platformdir/java/j2sdk*.bin $temp
    cd $temp && chmod u+x j2sdk*.bin
    cd $sdk
    if { ! xterm -T "JAVA 2 Installer" -e $temp/j2sdk*.bin & }; then 
       printf "\n%s\n" "Error: Could not start xterm, maybe you are not allowed to connect to the xhost!n"
    fi

  else

    cd $platformdir/java
    j2sdk*windows*.exe

  fi  

fi

mkdir -p $temp
logfile="$temp/secondo-install.log"
touch $logfile

cd $HOME
printf "\n\n* Uncompressing SECONDO source files ... \n"
if { ! tar -xzf "$platformdir/secondo-${platform}.tgz"; }; then
  exit 3
fi

if [ "$platform" != "linux" ]; then

  # run windows specific installation commands and exit
  source ./winsetup.sh
  exit 
fi

cd "$temp"
printf "\n* Uncompressing 3d-party tools ... \n"

uncompressFolders "$platformdir/gnu" "$platformdir/non-gnu" "$platformdir/prolog"

cd $sdk
if { ! tar -xzf $cdpath/java/cvs/jcvs*.tgz; }; then
  exit 4
fi

if { ! xterm -T "Installation Protocol" -e "tail -f $logfile" & }; then
  exit 5
fi

printf "\n* Compiling GCC ... this will take the most time ... \n"
cd $temp/gcc-* && ./configure --prefix=$sdk >> $logfile 2>&1
if { ! make bootstrap >> $logfile 2>&1; }; then
  printf "\n Error: Compiling GCC failed! \n"
  exit 6
fi
make install >> $logfile 2>&1
export PATH=".:$sdk/bin:$PATH"
printf "\n <PATH: $PATH> \n" >> $logfile
gcc --version >> $logfile

printf "\n* Compiling Berkeley-DB ... \n"
cd $temp/db-*/build_unix && ../dist/configure --prefix=$sdk --enable-cxx >> $logfile 2>&1
make >> $logfile 2>&1 && make install >> $logfile 2>&1

printf "\n* Compiling libncurses ... \n"
cd $temp/ncurses-* && ./configure --prefix=$sdk >> $logfile 2>&1
make >> $logfile 2>&1 && make install >> $logfile 2>&1 

printf "\n* Compiling SWI-Prolog ... \n"
cd $temp/readline-* && ./configure --prefix=$sdk --with-curses >> $logfile 2>&1
make >> $logfile 2>&1 && make install >> $logfile 2>&1
cd $temp/pl-* && ./configure --prefix=$sdk >> $logfile 2>&1
make >> $logfile 2>&1 && make install >> $logfile 2>&1

printf "\n* Compiling flex and bison, the scanner and parser generators \n"
cd $temp/flex-* && ./configure --prefix=$sdk >> $logfile 2>&1
make >> $logfile 2>&1 && make install >> $logfile 2>&1
cd $temp/bison-* && ./configure --prefix=$sdk >> $logfile 2>&1
make >> $logfile 2>&1 && make install >> $logfile 2>&1

printf "\n* Compiling JPEG library ... \n"
cd $temp/jpeg-* && ./configure --prefix=$sdk >> $logfile 2>&1
make >> $logfile 2>&1 && make install >> $logfile 2>&1 && make install-lib >> $logfile 2>&1

printf "\n* Compiling the make tool ... \n"
cd $temp/make-* && ./configure --prefix=$sdk >> $logfile 2>&1
make >> $logfile 2>&1 && make install >> $logfile 2>&1 

printf  "\n* Copying configuration files ... \n"
make SECONDO_SDK=$sdk -f makefile.cm update-environment

printf  "\n\n* Proceed with the installation guide ... \n\n"

exit
