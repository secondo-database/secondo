# winsetup.sh - uncompress files and change MSYS configuration
#
# Oct. 2003, M. Spiekermann - initial version
# May  2004, M. Spiekermann - some improvements


# the mount point must exists before installing mingw
mkdir -p $msysdir/mingw
printSep "Starting MinGW Installer ..."
cd $platformdir/mingw
Min*.exe

if [ ! -d "$mingwdir" ]; then
   printf  "\n%s\n" " WARNING: Directory $mingwdir not found." 
   printf  "%s\n"   "          You may need to configure etc/fstab."
fi
 
printSep  "Starting SWI-Prolog Installer ..."
cd $platformdir/prolog
w32pl*.exe

prologdir=$sdk/pl
if [ ! -d "$prologdir" ]; then
   printf  "\n%s\n" "WARNING: Directory $prologdir not found." 
   printf  "%s\n"   "         You need to configure the .secondo*rc files."
fi


printSep "Installing unzip ..."

cd $sdk/bin
$platformdir/non-gnu/unzip/unz550xN.exe > /dev/null 
export PATH="$sdk/bin:$sdk/lib:$PATH"
unzip -q -o "$platformdir/non-gnu/unzip/zip23xN.zip"

printSep "\n%s\n" "* Uncompressing 3d-party tools ..."

cd $sdk
uncompressFolders "$platformdir/gnu" "$platformdir/non-gnu"

printSep "\n* Compiling Berkeley-DB ... \n\n"

export PATH=/c/mingw/bin:$PATH
rxvt -sl 5000 -title "Berkeley-DB Compilation" -e tail -f $logfile &
cd $temp/db-*/build_unix && ../dist/configure --prefix=$sdk --enable-cxx --enable-mingw >> $logfile 2>&1
make > $logfile 2>&1 && make install >> $logfile 2>&1

printf  "\n* MSYS and MinGW Configuration ... \n"
make SECONDO_SDK=$sdk -f makefile.cm update-environment

printf "%s\n" "* MSYS Configuration and file extraction has been finished."
printf "%s\n" "* Close all open MSYS windows and open a new one, otherwise"
printf "%s\n" "* the new configuration will not be read. Proceed with the "
printf "%s\n" "* Installation Guide."
printf "\n"
