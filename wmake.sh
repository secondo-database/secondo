# !/bin/bash
#
# wmake.sh: This script sets up the environment to use
#           wine on linux to build most parts of the source
#           code using the mingw tool (compiler, linker, etc.)
#           However, some parts have to be created with the native
#           linux versions.
#
# M. Spiekermann 27.01.2004


WINEROOT=$HOME/Windows-Wine-Root

# Setting the WINEROOT. This is the directory
# which wine uses for "C:". Under this directory copy
# mingw, prolog (only headers and libraries are needed).
# Below changes of /.wine/config are documented. You don't
# need a windows installation the dlls of the DLLOverides
# section are sufficient.
#
# [Drive C]
# "Path" = "${HOME}/Windows-Wine-Root"
# "Type" = "hd"
# "Label" = "fake_windows"
# "Filesystem" = "win95"
# 
# [wine]
# "Windows" = "C:\\windows-dll"
# "System" = "C:\\windows-dll"
# "Temp" = "C:\\windows-dll"
# "Path" = "C:\\windows-dll;X:\\;Y:\\;C:\\MinGW\\bin;C:\\usr\\local\\bin;C:\\usr\\local\\lib"
#
# [DllOverrides]
# "msvcrt" = "native,builtin"
# "crtdll" = "native,builtin"
# "msi" = "native,builtin"

shopt -s expand_aliases

source $HOME/.bashrc
setvar $WINEROOT/secondo

cd $WINEROOT/secondo
make config

# A generated parser and awk will not run
# inside wine therfore we have to recode
# some files and compile or run them on linux
specParserDir=$WINEROOT/secondo/Tools/Parser/SpecParser

if !( test -e $specParserDir/Parser.linux )
then
	cd $specParserDir
	cd recode cp1252..lat1 *.cpp *.l *.y
	make clean
	make
	mv Parser Parser.linux
	make clean

	cd $WINEROOT/secondo/Algebras
	recode cp1252..lat1 *.awk
fi



# Now we can compile using wine -- gcc .....
cd $WINEROOT/secondo

export J2SDK_ROOT="$WINEROOT/j2sdk1.4.2"
export BISON_SIMPLE="$WINEROOT/usr/local/share/bison/bison.simple"
export SWI_HOME_DIR="$WINEROOT/pl"
make SECONDO_PLATFORM=win32 WINE=true TTY
make SECONDO_PLATFORM=win32 WINE=true optimizer

