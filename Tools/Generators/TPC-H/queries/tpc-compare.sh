#!/bin/bash
#
# Dec. 2004, M. Spiekermann

if [ "$1" == "" ]; then
  echo -e "\n  Usage: $0 <database> <query numbers>\n"
  exit 1
fi

db="$1"

so="ttyrun.sh"
sosrv="SecondoTTYBDB"

pg="pgrun.sh"
pgsrv="postmaster"

plsrv="SecondoPL"
pl="plrun.sh"

prof="ps-memprof.sh"


source tpcconfig.sh

# run TPC queries
for nr in $2; do

#  $prof $sosrv $sosrv"."$nr".prof.log" 1 & 
#  $rmcache; $so $db "@tpc"$nr"_opt"
#  $prof $pgsrv $pgsrv"."$nr".prof.log" 1 & 
#  $rmcache; $pg $db $nr".sql.pg"
   cat XLoad >/dev/null; 
   $prof $plsrv $plsrv"."$nr".prof.log" 1 & 
   $pl $db "tpc"$nr

done
