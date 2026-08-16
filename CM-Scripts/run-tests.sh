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
    let error++
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
export SECONDO_PARAM_RTFlags="SI:NoQueryAnalysis" 

if [ -d $dbDir ]; then
  printf "%s\n" "Warning database directory ${dbDir} exists! Please remove it."
  exit 1
else
  printf "%s\n" "Creating new database directory ${dbDir}."
  mkdir -p $dbDir
fi


declare -i error=0

#
# Tests executed by the TestRunner (skippable via SECONDO_SKIP_TESTRUNNER_TESTS).
# These drive a kernel linked into TestRunner and are, apart from createdb.test
# which sets up the databases the rest of this section shares, single threaded.
# The sections below each create their own databases, so skipping this one does
# not disturb them.
#
if [ "${SECONDO_SKIP_TESTRUNNER_TESTS:-}" == "true" ]; then
  echo "*** Skipping test suites (SECONDO_SKIP_TESTRUNNER_TESTS=true) ***"
else
  # The first test create databases
  dbTest="createdb.test"
  dbFile="$buildDir/bin/$dbTest"

  testSuites=$(find $buildDir/Tests -wholename "*.test")

  #echo -e "$testSuites"
  #echo "ldd: "$(ldd $SECONDO_BUILD_DIR/bin/SecondoBDB)

  echo "*** Executing test suites ***"
  for testName in $dbFile $testSuites; do
    runDir=${testName%/*}
    testFile=${testName##*/}
    runTest $runDir $testFile "time $runnerCmd -i  ${testFile}" $timeOutMax
  done
fi


#
# Algebra tests (skippable via SECONDO_SKIP_ALGEBRA_TESTS). Selftest runs each
# operator's example queries in one process, so like the section above this is
# breadth over the operator set rather than concurrency coverage.
#
if [ "${SECONDO_SKIP_ALGEBRA_TESTS:-}" == "true" ]; then
  echo "*** Skipping algebra tests (SECONDO_SKIP_ALGEBRA_TESTS=true) ***"
else
  echo "*** Executing algebra tests ***"
  exampleFiles=$(find $buildDir/bin/tmp -wholename "*.examples")
  for testName in $exampleFiles; do
    runDir=${testName%/*}
    testFile=${testName##*/}
    runTest $runDir $testFile "cd $buildDir/bin/; time Selftest tmp/${testFile}" $timeOutMax
  done
fi


#
# Optimizer tests (skippable via SECONDO_SKIP_OPTIMIZER_TESTS, e.g. when the
# optimizer was not built). TestOptimizer drives the optimizer through the
# embedded Prolog interface.
#
if [ "${SECONDO_SKIP_OPTIMIZER_TESTS:-}" == "true" ]; then
  echo "*** Skipping optimizer tests (SECONDO_SKIP_OPTIMIZER_TESTS=true) ***"
else
  echo "*** Executing optimizer tests ***"
  runTest ${buildDir}/Optimizer "TestOptimizer" "time TestOptimizer" $timeOutMax
  # Guards against the SecondoPL autoloading regression (must_be/2 spam at
  # optimizer startup); see Optimizer/TestSecondoPLStartup.
  runTest ${buildDir}/Optimizer "TestSecondoPLStartup" \
          "time TestSecondoPLStartup" $timeOutMax
fi

#
# Client/server tests (skippable via SECONDO_SKIP_CS_TESTS). The .test suites
# above run against a kernel linked into TestRunner; this one starts a real
# SecondoMonitor on a private port and drives parallel SecondoCS clients
# against it, covering the process tree that serves network clients.
# ClientServer/ is not on PATH (libutil.sh adds bin/, Optimizer/ and
# CM-Scripts/ only), so invoke via ./ -- runTest cds into the directory first.
#
if [ "${SECONDO_SKIP_CS_TESTS:-}" == "true" ]; then
  echo "*** Skipping client/server tests (SECONDO_SKIP_CS_TESTS=true) ***"
else
  echo "*** Executing client/server tests ***"
  runTest ${buildDir}/ClientServer "TestClientServer" "time ./TestClientServer" $timeOutMax
  # TestClientServer asserts the values that come back. This one reads the
  # bytes: it walks the <SecondoResult> record framing in a server trace, which
  # is the only check that a chunk header agreeing with a chunk reader does not
  # simply mean both are wrong in the same way.
  runTest ${buildDir}/ClientServer "CheckResultFraming" \
          "./CheckResultFraming" $timeOutMax
fi

#
# Distributed2 tests (skippable via SECONDO_SKIP_DISTRIBUTED2_TESTS). The
# .test suites above run against a kernel linked into TestRunner, and no
# Distributed2 operator does anything without workers -- which is why the
# algebra's coverage was until now a Distributed2.examples file whose entries
# are mostly marked as known bugs. TestDistributed2 starts real monitors as
# workers on private ports and drives a .test suite against them.
#
# Linux only: gate on uname rather than on libutil.sh's $platform, which
# compares $OSTYPE against the literal "mac_osx" while macOS reports darwin*,
# and so calls macOS Linux.
#
if [ "$(uname -s)" != "Linux" ]; then
  echo "*** Skipping Distributed2 tests (not supported on $(uname -s)) ***"
elif [ "${SECONDO_SKIP_DISTRIBUTED2_TESTS:-}" == "true" ]; then
  echo "*** Skipping Distributed2 tests (SECONDO_SKIP_DISTRIBUTED2_TESTS=true) ***"
else
  echo "*** Executing Distributed2 tests ***"
  runTest ${buildDir}/Algebras/Distributed2 "TestDistributed2" \
          "time ./TestDistributed2" $timeOutMax
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

