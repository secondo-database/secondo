# winsetup.sh - uncompress files and change MSYS configuration
#
# Oct. 2003, M. Spiekermann - initial version
# May  2004, M. Spiekermann - some improvements

if [ ! -d "$msysdir" ]; then
   printf  "\n ERROR: Directory $msysdir not found." 
   printf  "\n        Please install MSYS first.  \n "
   exit 1; 
fi

for xdir in "$mingwdir" "$sdk" "$sdk/pl"; do
   
  if [ -d $xdir ]; then
    printf "\n Warning: Directory $xdir already exists. \n"
  fi
done

mkdir -p "$sdk"

# the mount point must exists before installing mingw
mkdir -p "$msysdir/mingw"
printf "\n* Starting MinGW Installer ... \n"
cd "$platformdir/mingw"
Min*.exe

if [ ! -d "$mingwdir" ]; then
   printf  "\n ERROR: Directory $mingwdir not found." 
   printf  "\n        Please install MinGW into directory $mingwdir.  \n "
   exit 2
fi
 
printf  "\n* Starting SWI-Prolog Installer ... \n"
cd "$platformdir/prolog"
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

mkdir -p "$sdk/bin"
cd "$sdk/bin"
"$platformdir/non-gnu/unzip/unz550xN.exe" > /dev/null 
export PATH="$sdk/bin:$sdk/lib:$PATH"
unzip -q -o "$platformdir/non-gnu/unzip/zip23xN.zip"

cd $sdk
printf "\n* Uncompressing 3d-party tools ... \n"

for folder in $platformdir/gnu $platformdir/non-gnu $cdpath/java/cvs; do
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
if { ! tar -xzf "$platformdir/secondo-win32.tgz"; }; then
  exit 5
fi

export PATH="/c/mingw/bin:$PATH"
cd $HOME/secondo/Win32
printf "\n* Compiling Berkeley-DB ... \n\n"
rxvt -sl 5000 -title "Berkeley-DB Compilation" -e tail -f $logfile &
cd $temp/db-*/build_unix && ../dist/configure --prefix=$sdk --enable-cxx --enable-mingw >> $logfile 2>&1
make > $logfile 2>&1 && make install >> $logfile 2>&1

printf  "\n* MSYS and MinGW Configuration ... \n"
make SECONDO_SDK=$sdk -f makefile.cm update-environment

printf  "%s\n" "* MSYS Configuration and file extraction has been finished."
printf  "%s\n" "* Close all open MSYS windows and open a new one, otherwise"
printf  "%s\n" "* the new configuration will not be read. Proceed with the "
printf  "%s\n" "* Installation Guide. \n"

