#!/bin/bash

cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/../../../bin

script_begin=$(date +%s)
echo -n "script start time:        "
date +%T
echo

for i in ../Algebras/Tile/Tests/*.test
do
  testcases_begin=$(date +%s)
  echo -n "file of testcases:        "
  basename $i
  ./TestRunner -i $i > $i.log 2> $i.errors
  echo -n "testcases status:         "
  grep 'unexpected error' $i.log
  grep -F '*** no ***' $i.log > /dev/null || (echo -n "        "; grep 'caused an error' $i.log)
  testcases_end=$(date +%s)
  echo -n "testcases execution time: "
  echo -n `expr $testcases_end - $testcases_begin`
  echo " seconds"
  echo
done

echo -n "script end time:          "
date +%T
script_end=$(date +%s)
echo -n "script execution time:    "
echo -n `expr $script_end - $script_begin`
echo " seconds"
