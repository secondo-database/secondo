#!/bin/bash
#
# Dec. 2004, M. Spiekermann
#
# Open a database and run a query using SecondoPL.

pl="SecondoPL -L4m"
# run query 

cd ${SECONDO_BUILD_DIR}/Optimizer
time $pl <<< "[autotest].
['${PWD}/tpcqueries.pl'].
open 'database $1'.
$2"

#return exit code of pl
exit $? 
