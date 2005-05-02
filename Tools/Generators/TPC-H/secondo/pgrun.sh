#!/bin/bash
#
# Dec. 2004, M. Spiekermann
#
# Open a database and run a query using psql. 

pg="psql -e"
# run query 

time $pg -f"$2" -d"$1"
