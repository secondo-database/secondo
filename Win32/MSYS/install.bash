#This file is part of SECONDO.
#
#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.
#
#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.
#
#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
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


mkdir "$msysdir/secondo-sdk"
mkdir "$msysdir/secondo"
mkdir "$msysdir/mingw"

printf "\n* Uncompressing archives ... "

cd "$instpath"
tar -xzf "$cdpath/secondo.tgz"
tar -xzf "$cdpath/sdk.tgz"
cd "$instpath/secondo-sdk"
tar -xzf "$cdpath/../java/cvs/jcvs-522.tgz"
tar -xzf "$cdpath/../bdb/db-4.1.25.tgz"
printf "\n* Note: The errors above are harmless :-) "


printf  "\n* Copying configuration files ... \n"
cd "$instpath/secondo"
chmod u+x setvar.bash catvar secondo-bashrc
cp setvar.bash catvar "$instpath/secondo-sdk/bin"
cp secondo-bashrc "$HOME"

cd "$cdpath" 
cp --backup fstab profile "$msysdir/etc"

printf  "\n* MSYS Configuration and file extraction has been finished."
printf  "\n* Close all open MSYS windows and open a new one, otherwise"
printf  "\n* the new configuration will not be read. \n"

