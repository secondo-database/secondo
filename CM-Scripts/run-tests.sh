#!/bin/bash
#
# run-tests.sh $1 $2 $3
#
# Options:
# --------
# $1: Mode [ -cs | -tty ] 
# $2: Directory for backup copies of emails and log files
# $3: timeout threshold in seconds for every single test.
#
# Jan 2005, M. Spiekermann
#
# August 2005, M. Spiekermann. Major changes. A function for calling the tests
# was added. And each test command is run with a timeOut. The log files of all
# failed test are stored in a tar file.
#
# Januar 2006, M. Spiekermann. Usage of the ~nice~ command added. 
#
# Sept 2006, M. Spiekermann. New parameter added

# include function definitions
# libutil.sh must be in the same directory as this file

if [ $# -ne 3 ]; then
  printf "\n%s\n" "Error: runTest needs 3 arguments."
  exit 1;
fi

if ! source ./libutil.sh; then exit 1; fi

printf "\n%s\n" "Running tests in ${buildDir}."

if [ "$1" == "-cs" ]; then
  runnerCmd="TestRunnerCS --no-tmp"
else
  runnerCmd="TestRunner --no-tmp"
fi

echo "TestRunner-Cmd: " $(which TestRunner)

failedFileInfoDir="/tmp/run-tests"$$
if [ "$2" != "" ]; then
  failedFileInfoDir="$2"
fi
if [ ! -d $failedFileInfoDir ]; then
  mkdir -p $failedFileInfoDir
fi
failedTests=""

timeOutMax=36000
if [ "$3" != "" ]; then
  timeOutMax=$3
fi

if ! isCmdPresent $runnerCmd; then
  printf "\n%s\n" "Sorry, command $runnerCmd not present."
  exit 1;
fi
runnerCmd="$runnerCmd"


# runTest $1 $2 $3 $4
#
# $1 runDir
# $2 testName
# $3 runCmd 
# $4 waitSeconds 

function runTest() {

  if [ $# -ne 4 ]; then
    printf "\n%s\n" "Error: runTest needs 4 arguments."
    exit 1;
  fi

  local runDir=$1
  local testName=$2
  local runCmd=$3
  local logFile=$runDir/${testName}.log
  local waitSeconds=$4

  echo -e "\n Running $testName in $runDir"
  echo -e "\n $runCmd"
  cd $runDir
  timeOut $waitSeconds $runCmd > ${logFile} 2>&1
  rc=$LU_RC

  if [ $rc -ne 0 ]; then
    echo -e "\nTest failed with returncode $LU_RC \n"
    echo "=============================="
    echo "File: ${logFile}"
    echo "=============================="
    cat ${logFile}
    echo "=============================="
  fi

  return $?
} 


# Setup configuration file and overule some parameter
dbDir="/tmp/$USER/test-databases-${date_TimeStamp}"

export SECONDO_CONFIG="$buildDir/bin/SecondoConfig.ini"
export SECONDO_PARAM_SecondoHome="$dbDir"
export SECONDO_PARAM_RTFlags="SI:NoQueryAnalysis,DEBUG:DemangleStackTrace" 

if [ -d $dbDir ]; then
  printf "%s\n" "Warning database directory ${dbDir} exists! Please remove it."
  exit 1
else
  printf "%s\n" "Creating new database directory ${dbDir}."
  mkdir -p $dbDir
fi


declare -i error=0

#
# Tests executed by the TestRunner
#
# The first test create databases
dbTest="createdb.test"
dbFile="$buildDir/bin/$dbTest" 

testSuites=$(find $buildDir/Tests -wholename "*.test")

#echo -e "$testSuites"
#echo "ldd: "$(ldd $SECONDO_BUILD_DIR/bin/SecondoBDB)

timeOut=136000
echo "*** Executing test suites ***"
for testName in $dbFile $testSuites; do
  runDir=${testName%/*}
  testFile=${testName##*/}
  runTest $runDir $testFile "time $runnerCmd -i  ${testFile}" $timeOut
  timeout=$timeOutMax
done


#
# Algebra tests
#
echo "*** Executing algebra tests ***"
exampleFiles=$(find $buildDir/bin/tmp -wholename "*.examples")
for testName in $exampleFiles; do
  runDir=${testName%/*}
  testFile=${testName##*/}
  runTest $runDir $testFile "cd $buildDir/bin/; time Selftest tmp/${testFile}" $timeOutMax
done


#
# Optimizer tests
#
echo "*** Executing optimizer tests ***"
runTest ${buildDir}/Optimizer "TestOptimizer" "time TestOptimizer" $timeOutMax

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

