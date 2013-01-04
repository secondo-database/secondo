#!/bin/bash

WD=$PWD

cp nrdemo $SECONDO_BUILD_DIR/bin/
cd $SECONDO_BUILD_DIR/bin
F=$WD/createtestdbs.stext

if [ "$1" != "quit" ]; then
	echo "************************"
	echo "Warning: The following databases will be deleted and restored:"
	cat "$F"|grep "restore database"|sed 's/restore database//g'|sed 's/from.*//g'
	echo "Press return to continue or ctrl+c to abort."
	read trash
fi

SecondoBDB < "$F"

