# winsetup.sh - uncompress files and change MSYS configuration
#
# Oct. 2003, M. Spiekermann - initial version
# May. 2004, M. Spiekermann - some improvements
# Feb. 2005, M. Spiekermann - Linux and Windows installation scripts merged into a
#            single version called installsdk

printSep "Starting MinGW Installer ..."
cd $platformdir/mingw
checkCmd Min*.exe

if [ ! -d "$mingwdir" ]; then
   printf  "\n%s\n" " WARNING: Default installation directory $mingwdir not found." 
   printf  "%s\n"   "          You may need to configure .secondo.win32rc."
fi
 
printSep  "Starting SWI-Prolog Installer ..."
cd $platformdir/prolog
checkCmd w32pl*.exe

prologdir=$sdk/pl
if [ ! -d "$prologdir" ]; then
   printf  "\n%s\n" "WARNING: Default installation directory $prologdir not found." 
   printf  "%s\n"   "         You may need to configure the .secondo.win32rc."
fi


printSep "Uncompressing 3d-party tools ..."
cd $sdk
uncompressFolders "$platformdir/gnu" "$platformdir/non-gnu"

printf "\n"
printSep "Compiling Berkeley-DB ..."
$xterm -title "Berkeley-DB Compilation" -e tail -f $logfile &
xtermPID=$!
cd $sdk/db-*/build_unix
checkCmd "../dist/configure --prefix=$sdk --enable-cxx --enable-mingw"
checkCmd "make && make install"

copyConfigFiles

printf "%s\n" "* MSYS Configuration and file extraction has been finished."
printf "%s\n" "* Close all open MSYS windows and open a new one, otherwise"
printf "%s\n" "* the new configuration will not be read."
printf "\n"
