#!/bin/sh
#
# Jan 2005, M. Spiekermann
#


# include function definitions
# libutil.sh must be in the same directory as this file
if ! source ./libutil.sh; then exit 1; fi

startedTests="$buildDir/started-tests"
passedTests="$buildDir/passed-tests"
rm -f $startedTests
rm -f $passedTests

inputDir="${buildDir}/Tests/Testspecs"
testSuites=$(find $buildDir -path "*Tests*.test" -printf "%P\n")

printf "\n%s\n" "Running tests in ${buildDir}."

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

declare -i error=0
for testName in $testSuites
do 
  baseDir=${testName%/*}
  logFile="${testName}.log"
  printf "\n%s\n" "Running ${testName} in directory $baseDir"
  cd $baseDir
  printf "%s\n" "==================================================================="  > $logFile
  printf "%s\n" "===================================================================\n"  > $logFile
  echo "$logFile" >> $startedTests
  checkCmd "time TestRunner -i  ${testName} > ${logFile} 2>&1"
  if lastRC; then
    echo "$logFile" >> $passedTests
  else
    let error++
  fi
done

printf "\n%s\n\n" "Running optimizer test ..."
cd ${buildDir}/Optimizer
echo "Optimizer/optimizer.test.log" >> $startedTests
checkCmd "time TestOptimizer"
if lastRC; then
  echo "Optimizer/optimizer.test.log" >> $passedTests
else
  let error++
fi

#clean up
printf "\n%s\n\n" "Cleaning up ..."
rm -rf $dbDir

exit $error

