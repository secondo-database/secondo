#!/bin/bash
#
# Dec. 2004, M. Spiekermann

if [ "$1" == "" ]; then
  echo -e "\n  Usage: $0 <database> <query numbers>\n";
  exit 1;
fi

db="$1"
queryDir="$HOME/secondo/Tools/Generators/TPC-H/queries"
soDir="../../../../"
PATH="$PATH:$soDir/bin:$soDir/Optimizer:$soDir/CM-Scripts"
so="ttyrun.sh"
sosrv="SecondoTTYBDB"
pg="pgrun.sh"
pgsrv="postmaster"
prof="ps-memprof.sh"

# run TPC queries

for nr in $2; do
#for nr in 5; do

  cat XLoad > /dev/null
#  $prof $sosrv $sosrv"."$nr".prof.log" 1 & 
  $so $db "@tpc"$nr"_opt"
  cat XLoad > /dev/null
#  $prof $pgsrv $pgsrv"."$nr".prof.log" 1 & 
  $pg $db $nr".sql.pg"

done
