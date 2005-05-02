#!/bin/sh
#
# February 2005, M. Spiekermann

libFile="libutil.sh"
#include libutil.sh if present
buildDir=${SECONDO_BUILD_DIR}
scriptDir="."
if [ -z $buildDir ]; then
  printf "%s\n" "Error: I can't find file ${libUtil}."
  exit 1
else
  scriptDir=${buildDir}/CM-Scripts
fi

source ${scriptDir}/$libFile
if [ $? -ne 0 ]; then exit 1; fi

# default options
dbName="tpc_h"
bdbHome=${HOME}/secondo-databases
tpcNr="1"

declare -i numOfArgs=$#
let numOfArgs++

while [ $# -eq 0 -o $numOfArgs -ne $OPTIND ]; do

  getopts "ihd:r:b:" optKey
  if [ "$optKey" == "?" ]; then
    optKey="h"
  fi

  case $optKey in

   h) showGPL
      printf "\n%s\n" "Usage of ${0##*/}:" 
      printf "%s\n"   "  -h Print this message and exit"
      printf "%s\n"   "  -r<space separated list of query numbers> => e.g. \"1 3 5\""
      printf "%s\n"   "  -b<directory for berkeley-db files> => $HOME/secondo-databases"
      printf "%s\n\n" "  -d<database name> => tpc_h"
      exit 0;;
   
   d) dbName=$OPTARG;;

   b) bdbHome=$OPTARG;;

   r) tpcNr=$OPTARG;; 

  esac

done

if [ -z $buildDir ]; then 

  printf "%s\n" "Error: Variable SECONDO_BUILD_DIR undefined."
  exit 1
fi 

tpcDir=${buildDir}/Tools/Generators/TPC-H

for nr in $tpcNr; do

files="${files}@${tpcDir}/secondo/queries/tpc${nr}_opt;
"
done


export SECONDO_PARAM_SecondoHome=$bdbHome
SecondoTTYBDB <<< "open database $dbName;
${files}
q;"

exit $? 
