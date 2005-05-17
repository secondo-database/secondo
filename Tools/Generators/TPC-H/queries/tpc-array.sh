#!/bin/bash
#
# Dec. 2004, M. Spiekermann

if [ "$1" == "" ]; then
  echo -e "\n  Usage: $0 <database[s]> [<SecondoHome>]\n"
  exit 1
fi

if [ "$2" != "" ]; then
  export SECONDO_PARAM_SecondoHome="$2"
fi

so="ttyrun.sh"
logDir="array_run"
refFile="tpc_join_static"

mkdir $logDir 

source tpcconfig.sh

# run TPC queries

# reference runtimes without arrays


# runtimes with arrays
for db in $1; do

  printf "%s\n" "Running ${dbDir}/${refFile} ..."
  dbDir=${logDir}/${db}
  mkdir $dbDir
  time $so $db "@$refFile" > ${dbDir}/${refFile}.log 2>&1
  mv *.csv $dbDir 

for file in "tpc_join_array" "tpc_join_adaptive"; do
for nr in 25 50 100 200; do

  nrDir=${dbDir}/${nr}/${file}
  printf "%s\n" "Running ${nrDir} ..."
  mkdir -p $nrDir
  cmdFile=${file}_${nr}
  sed "s#LINEITEM_25#LINEITEM_$nr#g" $file > $cmdFile   
  time $so $db "@$cmdFile" > ${nrDir}/${cmdFile}.log 2>&1
  mv *.csv $nrDir 

done
done
done
