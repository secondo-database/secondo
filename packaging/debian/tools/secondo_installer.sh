#!/bin/bash

echo "================================="
echo "Welcome to the SECONDO installer"
echo "================================="

if [ -f ~/.secondorc ]; then
    echo "Error: Found an old secondo configuration '~/.secondorc'"
    echo "To reconfigure SECONDO, please delete the file first"
    exit -1
fi

databasedir=~/secondo-databases
echo -n "Specify the directory for your databases [$databasedir]: "
read databasedir2

if [ -n "$databasedir2" ]; then
    databasedir=$databasedir2
fi

if [ ! -d $databasedir ]; then
	mkdir -p $databasedir
fi

workdir=~/.secondo
echo -n "Specify the SECONDO working directory [$workdir]: "
read workdir2

if [ -n "$workdir2" ]; then
    workdir=$workdir2
fi

if [ ! -d $workdir ]; then
	mkdir -p $workdir
	mkdir -p $workdir/sgui
	mkdir -p $workdir/optimizer

	cp /opt/secondo/bin/javagui/GBS.cfg $workdir/sgui
	cp /opt/secondo/bin/javagui/gui.cfg $workdir/sgui
fi

cp /opt/secondo/etc/SecondoConfig.ini ~/
sed -i "s|SecondoHome=.*|SecondoHome=$databasedir|" ~/SecondoConfig.ini

# Determine plattform
architecture=$(getconf LONG_BIT)

if [ $architecture -eq 64 ]; then
cat <<-EOF > ~/.secondorc
export SECONDO_WORK_DIR=$workdir
export SECONDO_BUILD_DIR=/opt/secondo
export SECONDO_PLATFORM=linux64
export SECONDO_CONFIG=~/SecondoConfig.ini
export SWI_HOME_DIR=/usr/lib/swi-prolog
export PL_LIB_DIR=\$SWI_HOME_DIR/lib/amd64/
export PL_DLL_DIR=\$SWI_HOME_DIR/lib/amd64/
export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:\$SWI_HOME_DIR/lib:\$PL_LIB_DIR
export PATH=\$PATH:\$SECONDO_BUILD_DIR/bin
export JPL_DLL=\$SWI_HOME_DIR/\$PL_LIB_DIR/libjpl.so
export JPL_JAR=\$SWI_HOME_DIR/lib/jpl.jar
EOF
else 
cat <<-EOF > ~/.secondorc
export SECONDO_WORK_DIR=$workdir
export SECONDO_BUILD_DIR=/opt/secondo
export SECONDO_PLATFORM=linux
export SECONDO_CONFIG=~/SecondoConfig.ini
export SWI_HOME_DIR=/usr/lib/swipl-6.6.5
export PL_LIB_DIR=\$SWI_HOME_DIR/lib/i686-linux/
export PL_DLL_DIR=\$SWI_HOME_DIR/lib/i686-linux/
export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:\$SWI_HOME_DIR/lib:\$PL_LIB_DIR
export PATH=\$PATH:\$SECONDO_BUILD_DIR/bin
export JPL_DLL=\$SWI_HOME_DIR/\$PL_LIB_DIR/libjpl.so
export JPL_JAR=\$SWI_HOME_DIR/lib/jpl.jar
EOF
fi

if [ $(grep secondorc ~/.bashrc | wc -l) -eq 0 ]; then
   sed -i '1s|^|source ~/.secondorc\n|' ~/.bashrc
fi

echo ""
echo "Installation complete."
echo ""
echo "Your databases will be stored in $databasedir"
echo ""
echo "The configuration of SECONDO will be stored in $workdir"
echo ""
echo "Please re-login or execute 'source ~/.secondorc'"
echo ""
echo "Afterwards, you can start SECONDO by executing 'SecondoTTYBDB'"
echo ""
echo ""

