#!/bin/bash

cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/../../../bin

for i in ../Algebras/Tile/Tests/*.test
do
  basename $i
  ./TestRunner -i $i > $i.log 2> $i.errors
  echo -n "        "; grep 'unexpected error' $i.log
  grep -F '*** no ***' $i.log > /dev/null || (echo -n "        "; grep 'caused an error' $i.log)
  echo
done
