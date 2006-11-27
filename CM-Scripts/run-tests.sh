#!/bin/sh
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
  mkdir $failedFileInfoDir
fi
failedTests=""

timeOutMax=3600
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
    failedTests="$failedTests ${logFile#$buildDir/}"
    let error++
    local testFailed="true"

  fi

  # keep the first date of failure in a file. When the test succeed afterwards
  # the file will be deleted.
  if [ "$failedFileInfoDir" != "" ]; then
    local failedFileInfo=$failedFileInfoDir/"_failed_"$testName
    if [ "$testFailed" == "true" ]; then
      if [ ! -e $failedFileInfo ]; then
         date +"$testName failed since %Y-%m-%d %H:%M / rc=$rc" >> $failedFileInfo
      fi
    else
      rm -f $failedFileInfo
    fi
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



#
# Tests executed by the TestRunner
#

  
# The first test create databases
dbTest="createdb.test"
dbFile="$buildDir/bin/$dbTest" 

declare -i error=0

testSuites=$(find $buildDir -path "*Tests*.test" -o -path "*bin*.test" -a ! -name "*$dbTest")

#echo -e "$testSuites"
#exit 1

#echo "ldd: "$(ldd $SECONDO_BUILD_DIR/bin/SecondoBDB)

timeOut=1800
for testName in $dbFile $testSuites; do
  runDir=${testName%/*}
  testFile=${testName##*/}
  if isCmdPresent "nice"; then
    niceOpt="nice -n 19"
  fi
  runTest $runDir $testFile "$niceOpt time $runnerCmd -i  ${testFile}" $timeOut
  wait $! 
  timeout=$timeOutMax
done


#
# Other tests not executed by the TestRunner application
#

if [ "$1" == "-tty" ]; then
  runTest ${buildDir}/Optimizer "TestOptimizer" "time TestOptimizer" $timeOutMax
fi

if [ "$failedTests" != "" ]; then
  cd $buildDir
  tar -cvzf failedTests.tar.gz $failedTests
fi

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

