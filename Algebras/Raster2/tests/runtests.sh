#!/bin/bash

cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/../../../bin

for i in ../Algebras/Raster2/tests/*.test
do
	basename $i
	./TestRunner -i $i > $i.log 2> $i.err
	echo -n "        "; grep 'unexpected error' $i.log
	grep -F '*** no ***' $i.log > /dev/null || (echo -n "        "; grep 'caused an error' $i.log)
	echo
done


