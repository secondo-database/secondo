#!/bin/bash
#
# Dec. 2004, M. Spiekermann
#
# Open a database and run a query using SecondoPL.

pl="SecondoPL -L4m"
# run query 

time $pl <<< "[tpcqueries].
open 'database $1'.
$2.
halt."
