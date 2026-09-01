#!/bin/bash

echo "================================="
echo "Welcome to the SECONDO installer"
echo "================================="

if [ -f ~/.secondorc ]; then
    echo "Error: Found an old secondo configuration '~/.secondorc'"
    echo "To reconfigure SECONDO, please delete the file first"
    exit 1
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

# Only these are needed at run time. The platform is compiled into the binaries
# and the build-time settings (SWI-Prolog, JDK, Berkeley DB) were resolved when
# the package was built, so nothing has to be detected at login any more.
cat <<-EOF > ~/.secondorc
export SECONDO_WORK_DIR=$workdir
export SECONDO_BUILD_DIR=/opt/secondo
export SECONDO_CONFIG=\$HOME/SecondoConfig.ini
export PATH="\$PATH:\$SECONDO_BUILD_DIR/bin:\$SECONDO_BUILD_DIR/Tools/pd"
EOF

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

