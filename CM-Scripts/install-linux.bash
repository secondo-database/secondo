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

printf "\n* Installing from " 
printf "\n* '$cdpath' to '$instpath' \n" 

printf "\n* Installing Java SDK ... this needs some user interaction"
printf" \n* all other tools will be compiled and installed without questions"

cd "sdk"
"$cdpath/java/j2sdk*.bin"

printf "\n* Uncompressing archives ... "

cd "$temp"
tar -xzf "$cdpath/gcc-core*.tgz"
tar -xzf "$cdpath/gcc-g++*.tgz"
tar -xzf "$cdpath/../bdb/db-*.tgz"
tar -xzf "$cdpath/prolog/pl-*.tgz"
tar -xzf "$cdpath/prolog/readline-*.tgz"

cd "$HOME"
tar -xzf "$cdpath/secondo.tgz"

cd "$sdk"
tar -xzf "$cdpath/../java/cvs/jcvs*.tgz"

printf "\n* Compiling GCC ... this will take the most time ... \n"
cd "$temp/gcc-*"
./configure --prefix=$sdk
"make bootstrap"
"make install"
export PATH=".:$sdk:$PATH"

printf "\n* Compiling Berkeley-DB ... \n"
cd "$temp/db-*"
./configure --prefix=$sdk --enable-cxx
make
"make install"

printf "\n* Compiling SWI-Prolog ... \n"
cd "$temp/readline-*"
./configure --prefix=$sdk
make
"make install"
cd "$temp/db-*"
./configure --prefix=$sdk
make
"make install"


printf  "\n* Copying configuration files ... \n"
cd "$instpath/secondo"
chmod u+x setvar.bash catvar secondo-bashrc
cp setvar.bash catvar "$instpath/secondo-sdk/bin"
cp secondo-bashrc "$HOME"

printf  "\n* Proceed with the installation guide ... \n"
