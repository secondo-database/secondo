#!/bin/bash
#
# Dec. 2004, M. Spiekermann
#
# Open a database and run a query using SecondoTTYBDB. 

stty="SecondoTTYBDB"
# run query 

time $stty <<< "open database $1;
$2;
q;"
