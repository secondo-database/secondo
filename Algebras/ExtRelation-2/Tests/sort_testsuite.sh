#!/bin/bash

# --------------------------------------------
# configuration

TESTHOME=$SECONDO_BUILD_DIR/Algebras/ExtRelation-2/Tests
TESTLOGDIR=$TESTHOME/logs
TESTSUMMARY=$TESTHOME/logs/testsort_summary.$(date +%d%m%y_%k%M%S).log
TESTEXT=test
DB_DIR=$HOME/secondo-databases/intern

# --------------------------------------------
# Name of test files (*.test extension assumed)

TESTS=( "sort_max_fan_in" 
	"sort_iobuffer" 
	"sort_benchmark_1MB"
	"sort_benchmark_10MB"
	"sort_benchmark_50MB"
	"sort_benchmark_100MB"
	"sort_benchmark_200MB"
	"sort_benchmark_300MB"
	"sort_benchmark_400MB"
	"sort_benchmark_500MB"
	"sort_benchmark_600MB"
	"sort_benchmark_700MB"
	"sort_benchmark_800MB"
	"sort_benchmark_900MB"
	"sort_benchmark_1G"
	"sort_benchmark_5G"
	"sort_benchmark_10G"
	"sort_benchmark_15G"
	"sort_benchmark_20G" )
 
# --------------------------------------------
# Count number of tests

testsLen=${#TESTS[@]}

# --------------------------------------------
# change into test runner bin-directory

cd $SECONDO_BUILD_DIR/bin
 
# --------------------------------------------
# perform all tests

for (( i=0; i<${testsLen}; i++ ));
do
  TESTNAME=${TESTS[$i]}
  TESTLOG=$TESTLOGDIR/$TESTNAME.$(date +%d%m%y_%k%M%S).run
  rm -r $DB_DIR/*
  TestRunner --no-tmp -i $TESTHOME/$TESTNAME.test | tee $TESTLOG
  echo "----------------------------------------" | tee -a $TESTSUMMARY
  echo $TESTNAME.$TESTEXT | tee -a $TESTSUMMARY
  echo "----------------------------------------" | tee -a $TESTSUMMARY
  echo | tee -a $TESTSUMMARY
  grep "Sort-Operation Statistics" -B 1 -A 17 $TESTLOG | tee -a $TESTSUMMARY
  echo | tee -a $TESTSUMMARY
  grep "TEST SUMMARY :" -A 2 $TESTLOG | tee -a $TESTSUMMARY
  echo | tee -a $TESTSUMMARY
done

# --------------------------------------------
# cleanup database directory

rm -r $DB_DIR/*



