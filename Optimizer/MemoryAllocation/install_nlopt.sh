#!/bin/bash

inst() {
	F=nlopt-2.2.4.tar.gz 
	rm $F
	wget http://ab-initio.mit.edu/nlopt/$F || return 1
	tar xzf $F || return 2

	DIR=`echo $F|sed 's/.tar.gz//'`
	cd $DIR || exit 3
	pwd

	./configure && make && sudo make install || exit 4
	cd ..
	rm -rf $DIR # no longer needed
}

inst
RC=$?
echo "RC=$?"
if [ "$RC" != 0 ]; then
	echo "ERROR, please inspect the output for further details."
else
	echo "ok"
fi
exit $RC

# eof

