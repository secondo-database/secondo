#!/bin/bash
#
# Dec. 2004, M. Spiekermann
#
# Open a database and run a query using SecondoPL.

pl="SecondoPL -G6m -L8m"
TPC_QUERY_DIR=${SECONDO_BUILD_DIR}/Tools/Generators/TPC-H/queries

# run query 
cd ${SECONDO_BUILD_DIR}/Optimizer

if [ "$1" != "" ]; then

time $pl <<< "[autotest].
['${TPC_QUERY_DIR}/tpcqueries'].
$1"

else

$pl

fi


#return exit code of pl
exit $? 
