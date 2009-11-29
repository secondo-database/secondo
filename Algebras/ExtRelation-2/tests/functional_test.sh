#!/bin/bash

# --------------------------------------------
# configuration

TESTHOME=$SECONDO_BUILD_DIR/Algebras/ExtRelation-2/tests
TESTLOGDIR=$TESTHOME/logs
TESTSUMMARY=$TESTHOME/logs/functional_tests_summary.$(date +%d%m%y_%k%M%S).log
TESTEXT=test
DB_DIR=$HOME/secondo-databases/intern

# --------------------------------------------
# Name of test files (*.test extension assumed)

TESTS=( "tuplefile_functional1" 
	"tuplefile_functional2" 
	"tuplebuffer_functional1" 
	"tuplebuffer_functional2" 
	"sort_functional1" 
	"sort_functional2" 
	"sortmergejoin_functional1" 
	"sortmergejoin_functional2" 
	"gracehashjoin_functional1"
	"gracehashjoin_functional2"
	"hybridhashjoin_functional1"
	"hybridhashjoin_functional2" )
 
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
grep "TEST SUMMARY :" -A 2 $TESTLOG | tee -a $TESTSUMMARY
  echo | tee -a $TESTSUMMARY
done

# --------------------------------------------
# cleanup database directory

rm -r $DB_DIR/*



