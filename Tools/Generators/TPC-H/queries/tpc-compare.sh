#!/bin/bash
#
# Dec. 2004, M. Spiekermann

db="$1"
queryDir="$HOME/secondo/Tools/Generators/TPC-H/queries"
soDir="$HOME/secondo-gccopt/bin"
PATH="$PATH:$soDir"
so="ttyrun.sh"
pg="pgrun.sh"

# run TPC queries

for nr in 1 3 5 10 do

  $so $db "@tpc"$nr"_opt"; 
  $pg $db $nr".sql.pg";

done
