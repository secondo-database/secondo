#!/bin/sh
# install.bash <path> - uncompress files and change MSYS configuration
#
# Oct. 2003, M. Spiekermann - initial version
# May  2004, M. Spiekermann - some improvements

cdpath="$PWD"
instpath="/c"
if [ "$1" = "" ]; then
   printf "\n* Using default installation $instpath \n"
else if [ "$1" = "testmode" ]; then
     printf "\n* Running Test Mode! \n"
     #create directory structure
     instpath="$HOME/SANDBOX-DRV-C"
     HOME="$instpath/msys/1.0/home/dummy"
     install -d "$instpath/msys/1.0/home/dummy"
     install -d "$instpath/msys/1.0"
     install -d "$instpath/msys/1.0/etc"
     install -d "$instpath/mingw"
     install -d "$instpath/secondo-sdk"
     install -d "$instpath/secondo"
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
printf "\n\n* Creating mount points ... \n"


install -d "$msysdir/secondo-sdk"
install -d "$msysdir/secondo"
install -d "$msysdir/mingw"

printf "\n\n* Installing unzip ..."

install -d "$instpath/secondo-sdk/bin"
cd "$instpath/secondo-sdk/bin"
"$cdpath/non-gnu/unzip/unz550xN.exe > /dev/null"
export PATH="$instpath/secondo-sdk/bin:$instpath/secondo-sdk/lib:$PATH"
unzip -q -o "$cdpath/non-gnu/unzip/zip23xN.zip"

cd "$instpath/secondo-sdk"
printf "\n\n* Uncompressing archives ... \n"

for folder in $cdpath/gnu $cdpath/non-gnu $cdpath/../java/cvs; do
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
printf "\n\n  Uncompressing SECONDO source files ... \n"
if { ! tar -xzf "$cdpath/secondo.tgz"; }; then
  exit 3
fi

printf  "\n\n* Copying configuration files ... \n"
cd "$HOME/secondo/CM-Scripts"
cp --backup setvar.bash catvar.sh "$instpath/secondo-sdk/bin"
cp --backup .secondorc .bashrc-sample "$HOME"
cp --backup .bashrc-sample "$HOME/.bashrc"
cd "$instpath/secondo-sdk/bin"
chmod u+x setvar.bash catvar.sh 
cd "$HOME"
chmod u+x .secondorc .bashrc
cd "$HOME/secondo/Win32/MSYS"
cp --backup fstab profile "$msysdir/etc"

cd "$cdpath/prolog"
w32pl5010.exe

cd "$cdpath/java"
j2sdk-1_4_2-windows-i586.exe

printf  "\n* MSYS Configuration and file extraction has been finished."
printf  "\n* Close all open MSYS windows and open a new one, otherwise"
printf  "\n* the new configuration will not be read. Proceed with the "
printf  "\n* Installation Guide. \n"

