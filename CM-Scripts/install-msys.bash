#!/bin/sh
# install.bash - uncompress files and change MSYS configuration
#
# Oct. 2003, M. Spiekermann - initial version
# May  2004, M. Spiekermann - some improvements

cdpath="$PWD"
instpath="/c"
if [ "$1" = "" ]; then
   printf "\n* Using default installation $instpath \n"
else if [ "$1" = "testmode" ]; then
     printf "\n* Running Test Mode! \n"
     instpath="$HOME/SANDBOX-DRV-C"
     install -d "$instpath/msys/1.0"
     install -d "$instpath/mingw"
   else
     instpath="$1"
   fi
fi

msysdir="$instpath/msys/1.0"
mingwdir="$instpath/mingw"
continue="true"

if [ ! -d "$instpath" ]; then
   printf  "\n ERROR: Directory $instpath not found. \n" 
   continue="false"
fi

if [ ! -d "$msysdir" ]; then
   printf  "\n ERROR: Directory $msysdir not found." 
   printf  "\n        Please install MSYS first.  \n "
   continue="false"
fi

if [ ! -d "$mingwdir" ]; then
   printf  "\n ERROR: Directory $mingwdir not found." 
   printf  "\n        Please install MinGW first.  \n "
   continue="false"
fi

if [ "$continue" = "false" ]; then
   exit
fi   

printf "\n* Installing from " 
printf "\n* '$cdpath' to '$instpath' \n" 
printf "\n* Creating mount points ... \n"


mkdir "$msysdir/secondo-sdk"
mkdir "$msysdir/secondo"
mkdir "$msysdir/mingw"

printf "\n* Uncompressing archives ... "

cd "$instpath"
tar -xzf "$cdpath/secondo.tgz"
tar -xzf "$cdpath/sdk.tgz"
cd "$instpath/secondo-sdk"
tar -xzf "$cdpath/../java/cvs/jcvs-*.tgz"
unzip -q "$cdpath/../bdb/db-4.*.zip"


printf  "\n* Copying configuration files ... \n"
cd "$instpath/secondo"
chmod u+x setvar.bash catvar secondo-bashrc
cp setvar.bash catvar "$instpath/secondo-sdk/bin"
cp secondo-bashrc "$HOME"

cd "$cdpath" 
cp --backup fstab profile /etc

printf  "\n* MSYS Configuration and file extraction has been finished."
printf  "\n* Close all open MSYS windows and open a new one, otherwise"
printf  "\n* the new configuration will not be read. \n"

