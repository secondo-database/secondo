#!/bin/sh
# install.bash - create mount points and untar files
#
# 04/10/03 M. Spiekermann

cdpath="$PWD"
instpath="/c"
if [ "$1" = "" ]; then
   printf "\n* Using default installation $instpath \n"
else
   instpath="$1"
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


mkdir "$msysdir/usr"
mkdir "$msysdir/secondo"
mkdir "$msysdir/mingw"


printf  "\n* Changing MSYS configuration ... \n"
cd "$msysdir/etc"
tar -xzf "$cdpath/etc.tgz"

printf "\n* Uncompressing archives ... "

cd "$instpath"
tar -xzf "$cdpath/secondo.tgz"
tar -xzf "$cdpath/usr.tgz"
cd "$instpath/usr/local"
tar -xzf "$cdpath/../bdb/db-4.1.25.tgz"

printf  "\n* MSYS Configuration and file extraction has been finished."
printf  "\n* Close all open MSYS windows and open a new one, otherwise"
printf  "\n* the new configuration will not be read. \n"

