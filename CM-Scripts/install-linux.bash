#!/bin/sh
# install-linux.bash - untar files and run make for
# various software packages
#
# 04/19/05 M. Spiekermann

cdpath="$PWD"
instpath="$HOME"

mkdir $HOME/secondo-sdk
mkdir $HOME/temp-build

temp=$HOME/temp-build
sdk=$HOME/secondo-sdk

printf "\n* Installing the SECONDO DEVELOPMENT TOOLKIT from " 
printf "\n* '$cdpath' to '$instpath' \n" 

printf "\n* Installing Java SDK ... this needs some user interaction"
printf" \n* all other tools will be compiled and installed silently"

cp $cdpath/../java/j2sdk*.bin $temp
cd $temp
chmod u+x j2sdk*.bin
cd $sdk
xterm -T "JAVA 2 Installer" -e $temp/j2sdk*.bin &

cd "$temp"
printf "\n* Uncompressing 3d-party tools ... \n"

for folder in $cdpath/gnu $cdpath/non-gnu $cdpath/prolog; do
  zipFiles=$(find $folder -maxdepth 1 -name "*.zip")
  gzFiles=$(find $folder -maxdepth 1 -name "*.*gz")
  for file in $zipFiles; do
    printf "\n  processing $file ..."
    if { ! unzip -q -o $file; }; then
      exit 1
    fi
  done
  for file in $gzFiles; do
    printf "\n  processing $file ..."
    if { ! tar -xzf $file; }; then
      exit 2 
    fi
  done
done

cd "$HOME"
printf "\n\n* Uncompressing SECONDO source files ... \n"
if { ! tar -xzf "$cdpath/secondo.tgz"; }; then
  exit 3
fi

cd "$sdk"
if { ! tar -xzf $cdpath/../java/cvs/jcvs*.tgz; }; then
  exit 4
fi

logfile="$temp/secondo-install.log"
touch $logfile
xterm -T "Installation Protocol" -e "tail -f $temp/$logfile" &

printf "\n* Compiling GCC ... this will take the most time ... \n"
cd $temp/gcc-* && ./configure --prefix=$sdk >> $logfile
make bootstrap >> $logfile && make install >> $logfile
export PATH=".:$sdk/bin:$PATH"
printf "\n <PATH: $PATH> \n"
gcc --version

printf "\n* Compiling Berkeley-DB ... \n"
cd $temp/db-*/build-unix && ./configure --prefix=$sdk --enable-cxx >> $logfile
make >> $logfile && make install >> $logfile

printf "\n* Compiling SWI-Prolog ... \n"
cd $temp/readline-* && ./configure --prefix=$sdk >> $logfile
make >> $logfile && make install >> $logfile
cd $temp/pl-* && ./configure --prefix=$sdk >> $logfile
make >> $logfile && make install >> $logfile

printf  "\n* Copying configuration files ... \n"
cd "$HOME/secondo/CM-Scripts"
cp --backup setvar.bash catvar.sh "$instpath/secondo-sdk/bin"
cp --backup .secondorc .bashrc-sample "$HOME"
cd "$instpath/secondo-sdk/bin"
chmod u+x setvar.bash catvar.sh 
cd "$HOME"
chmod u+x .secondorc .bashrc

printf  "\n\n* Proceed with the installation guide ... \n"
