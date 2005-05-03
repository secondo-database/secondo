#!/bin/bash
#
# Dec. 2004, M. Spiekermann
#
# Open a database and run a query using SecondoTTYBDB. 

stty="SecondoTTYBDB"
# run query 

cd ${SECONDO_BUILD_DIR}/bin
time $stty <<< "$1
q;
"

exit $?
