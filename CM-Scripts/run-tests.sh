#!/bin/sh
#
# Jan 2005, M. Spiekermann
#


# include function definitions
# libutil.sh must be in the same directory as this file
if ! source libutil.sh; then exit 1; fi

inputDir="${buildDir}/Tests/Testspecs"
testSuites=$(find ${inputDir} -name "*.test")

printf "\n%s\n" "Running tests in ${buildDir}."

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


declare -i error=0
for testName in $testSuites
do 
  baseDir=${testName%/*}
  file=${testName##*/}
  logFile="${file}.log"
  printf "\n%s\n" "Running ${file} in directory $baseDir"
  cd $baseDir
  printf "%s\n" "==================================================================="  > $logFile
  printf "%s\n" "===================================================================\n"  > $logFile
  checkCmd "time TestRunner -i  ${file} > ${logFile} 2>&1"
  if [ $rc -ne 0 ]; then
    let error++
  fi
done

#clean up
printf "\n%s\n\n" "Cleaning up ..."
rm -rf $dbDir

exit $error

