#!/bin/sh
#
# Jan 2005, M. Spiekermann
#
# August 2005, Major changes. A function for calling the
#  tests was added. And each test command is run with a 
#  timeOut. The log files of all failed test are stored
#  in a tar file.


# include function definitions
# libutil.sh must be in the same directory as this file
if ! source ./libutil.sh; then exit 1; fi

printf "\n%s\n" "Running tests in ${buildDir}."

failedTests=""

# runTest $1 $2 $3 [$4]
#
# $1 runDir
# $2 testName
# $3 runCmd 
# $4 waitSeconds 

function runTest() {

  local runDir=$1
  local testName=$2
  local runCmd=$3
  local logFile=$runDir/${testName}.log
  local waitSeconds=240

  if [ "$4" != "" ]; then
    waitSeconds=$4
  fi

  echo -e "\n Running $testName in $runDir"
  cd $runDir
  timeOut $waitSeconds $runCmd > ${logFile} 2>&1

  if ! lastRC; then
    echo -e "\nTest failed with returncode $LU_RC \n"
    failedTests="$failedTests ${logFile#$buildDir/}"
    let error++
  fi

  return $?
} 


#overule configuration file parameter
dbDir="${buildDir}/test-databases-${date_TimeStamp}"
export SECONDO_PARAM_SecondoHome="$dbDir"
export SECONDO_PARAM_RTFlags="SI:NoQueryAnalysis,SI:NoCommandEcho" 

if [ -d $dbDir ]; then

  printf "%s\n" "Warning database directory ${dbDir} exists! Please remove it."
  exit 1

else

  printf "%s\n" "Creating new database directory ${dbDir}."
  mkdir $dbDir

fi

#
# Tests executed by the TestRunner
#

declare -i error=0

testSuites=$(find $buildDir -path "*Tests*.test")
for testName in $testSuites
do 
  runDir=${testName%/*}
  testFile=${testName##*/}
  runTest $runDir $testFile "time TestRunner -i  ${testFile}"
done

#
# Other tests not executed by the TestRunner application
#

runTest ${buildDir}/Optimizer "TestOptimizer" "time TestOptimizer" 300

cd $buildDir
tar -cvzf failedTests.tar.gz $failedTests

#clean up
printf "\n%s\n\n" "Cleaning up ..."
rm -rf $dbDir

if [ $error -gt 0 ]; then
  echo -e "*** Errors: ${error} ***\n"
  echo -e "*** Logfiles: $failedTests \n"
else
  echo -e "*** No Errors! ***\n"
fi

exit $error

