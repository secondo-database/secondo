#!/bin/bash
#
# Jan 2005, M. Spiekermann
#

buildDir=$1
if [ "$1" == "" ]; then
  buildDir=$SECONDO_BUILD_DIR
fi



#testSuites=$(find ${buildDir}/Tests -name "*test" -printf "%f ")
testSuites="binfiletest networktest polygontest relalgtest btreetest ftexttest oldrelalgtest rangetest"


cd ${buildDir}/bin
for testName in $testSuites
do 
  echo -e "\nRunning test ${testName} ... \n"
  if !( time TestRunner -i ../Tests/${testName} > ../Tests/${testName}.log 2>&1 )
  then
    echo -e "  Problems with test ${testName}. $? error(s) detected. \n"
  fi
done

