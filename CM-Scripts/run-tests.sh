#!/bin/sh
#
# Jan 2005, M. Spiekermann
#


# include function definitions
# libutil.sh must be in the same directory as this file
source  ${0%/*}/libutil.sh

buildDir=$1
if [ "$1" == "" ]; then
  buildDir=$SECONDO_BUILD_DIR
fi

inputDir="${buildDir}/Tests/Testspecs"
testSuites=$(find ${inputDir} -name "*.test" -printf "%f ")

printf "\n%s\n" "Running tests in ${buildDir}."

cd ${buildDir}/bin
#overule configuration file parameter

dbDir="${buildDir}/test-databases-${date_TimeStamp}"
export SECONDO_PARAM_SecondoHome="$dbDir"

if [ -d $dbDir ]; then

  printf "%s\n" "Warning database directory ${dbDir} exists! Please remove it."
  exit 1

else

  printf "%s\n" "Creating new database directory ${dbDir}."
  mkdir $dbDir

fi


for testName in $testSuites
do 
  file="${inputDir}/${testName}"
  printf "\n%s\n" "Running ${testName} ..."
  checkCmd "time TestRunner -i  ${file} > ${file}.log 2>&1"
done

#clean up
printf "\n%s\n\n" "Cleaning up ..."
rm -rf $dbDir

