# winsetup.sh - uncompress files and change MSYS configuration
#
# Oct. 2003, M. Spiekermann - initial version
# May  2004, M. Spiekermann - some improvements


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


if [ "$continue" = "false" ]; then
   exit 1
fi   


if [ -d "$mingwdir" ]; then
   printf "\n Warning: Directory $mingwdir already exists. \n"
fi 

if [ -d "$sdk" ]; then
   printf "\n Warning: Directory $sdk already exists. \n"
fi

if [ -d "$sdk/pl" ]; then
   printf "\n Warning: Directory $sdk/pl already exists. \n"
fi


printf "\n* Installing the SECONDO DEVELOPMENT TOOLKIT from " 
printf "\n* '$cdpath' to '$instpath' " 
printf "\n* "
printf "\n* Starting in 5 seconds. Press CRTL-C to abort! \n"
sleep 5

install -d "$sdk"

# the mount point must exists before installing mingw
install -d "$msysdir/mingw"
printf "\n* Starting MinGW Installer ... \n"
cd "$cdpath/mingw"
Min*.exe

if [ ! -d "$mingwdir" ]; then
   printf  "\n ERROR: Directory $mingwdir not found." 
   printf  "\n        Please install MinGW into directory $mingwdir.  \n "
   exit 2
fi
 
printf  "\n* Starting SWI-Prolog Installer ... \n"
cd "$cdpath/prolog"
w32pl*.exe

prologdir="$sdk/pl"
if [ ! -d "$prologdir" ]; then
   printf  "\n ERROR: Directory $prologdir not found." 
   printf  "\n        Please install SWI-Prolog into directory $prologdir.  \n "
   exit 6 
fi


javadir="$sdk/j2sdk1.4.2"
if [ ! -d "$javadir" ]; then
   printf  "\n ERROR: Directory $javadir not found." 
   printf  "\n        Please install JAVA 2 into directory $javadir.  \n "
   exit 7 
fi
printf "\n* Installing unzip ... \n"

install -d "$sdk/bin"
cd "$sdk/bin"
"$cdpath/non-gnu/unzip/unz550xN.exe" > /dev/null 
export PATH="$sdk/bin:$sdk/lib:$PATH"
unzip -q -o "$cdpath/non-gnu/unzip/zip23xN.zip"

cd "$sdk"
printf "\n* Uncompressing 3d-party tools ... \n"

for folder in $cdpath/gnu $cdpath/non-gnu $cdpath/../java/cvs; do
  zipFiles=$(find $folder -maxdepth 1 -name "*.zip")
  gzFiles=$(find $folder -maxdepth 1 -name "*.*gz")
  for file in $zipFiles; do
    printf "\n  processing $file ..."
    if { ! unzip -q -o $file; }; then
      exit 3 
    fi
  done
  for file in $gzFiles; do
    printf "\n  processing $file ..."
    if { ! tar -xzf $file; }; then
      exit 4 
    fi
  done
done

cd "$HOME"
printf "\n\n* Uncompressing SECONDO source files ... \n"
if { ! tar -xzf "$cdpath/secondo-win32.tgz"; }; then
  exit 5
fi

printf "\n* Compiling Berkeley-DB ... \n\n"
export SECONDO_SDK="$sdk"
export PATH="/c/mingw/bin:$PATH"
cd $HOME/secondo/Win32
logfile="$HOME/secondo-install.log"
touch $logfile
rxvt -sl 5000 -title "Berkeley-DB Compilation" -e tail -f $logfile &
cd $temp/db-*/build_unix && ../dist/configure --prefix=$sdk --enable-cxx --enable-mingw >> $logfile 2>&1
make > $logfile 2>&1 && make install >> $logfile 2>&1

printf  "\n* MSYS and MinGW Configuration ... \n"
make SECONDO_SDK=$sdk -f makefile.cm update-environment

printf  "\n* MSYS Configuration and file extraction has been finished."
printf  "\n* Close all open MSYS windows and open a new one, otherwise"
printf  "\n* the new configuration will not be read. Proceed with the "
printf  "\n* Installation Guide. \n"

