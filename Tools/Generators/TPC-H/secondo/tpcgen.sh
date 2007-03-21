#!/bin/sh
#
# February 2005, M. Spiekermann

printf "%s\n" "Starting tpcgen ..."

#include libutil.sh if present
buildDir=${SECONDO_BUILD_DIR}
scriptDir=${buildDir}/CM-Scripts
libFile="$scriptDir/libutil.sh"

if [ ! -e $libFile ]; then 
  printf "%s\n" "Error: I can't find file ${libFile}."
  exit 1
fi

source $libFile

# default options
dbName="tpc_h"
scaleFactor="0.01"
bdbHome=${HOME}/secondo-databases

declare -i numOfArgs=$#
let numOfArgs++

while [ $# -eq 0 -o $numOfArgs -ne $OPTIND ]; do

  getopts "ihd:s:b:" optKey
  if [ "$optKey" == "?" ]; then
    optKey="h"
  fi

  case $optKey in

   h) showGPL
      printf "\n%s\n" "Usage of ${0##*/}:" 
      printf "%s\n"   "  -h Print this message and exit"
      printf "%s\n"   "  -i generate and import data into SECONDO"
      printf "%s\n"   "  -s<volume factor given as real number> => 0.01"
      printf "%s\n"   "  -b<directory for berkeley-db files> => $HOME/secondo-databases"
      printf "%s\n\n" "  -d<database name> => tpc_h"
      exit 0;;
   
   s) scaleFactor=$OPTARG;;

   d) dbName=$OPTARG;;

   b) bdbHome=$OPTARG

  esac

done

if [ -z $buildDir ]; then 

  printf "%s\n" "Error: Variable SECONDO_BUILD_DIR undefined."
  exit 1
fi 



printf "\n%s\n" "Creating database ${dbName} with a scale factor ${scaleFactor}!"

tpcDir=${buildDir}/Tools/Generators/TPC-H
tempDir=/tmp/$USER/TPC/tmp_tpcgen_${date_ymd}_${date_HMS}

PATH="$PATH:."

# Generate data
printSep "Generating TPC-H data"
cd $tpcDir
if [ ! -e dbgen ]; then
assert make
fi

checkCmd "dbgen -f -s $scaleFactor" 
if [ $? -ne 0 ]; then
  exit 1
else
  assert mkdir -p $tempDir
  printf "%s\n" "Moving files into ${tempdir}"
  assert mv *_tbl $tempDir
fi


# restore TPC-H Benchmark tables created by dbgen
printSep "Restore database $dbName"
assert cd $tempDir


export SECONDO_PARAM_SecondoHome=$bdbHome
#export SECONDO_PARAM_RTFlags="SMI:NoTransactions,DEBUG:DemangleStackTrace,CMSG:Color"

ncmd=$(which nice)
niceOpt="nice -n19"
if [ $? -ne 0 ]; then
  niceOpt=""    
fi

$niceOpt SecondoTTYNT <<< "create database ${dbName};
open database ${dbName};
@${tpcDir}/secondo/restore_objs
q;"

if [ $? -eq 0 ]; then
 rm -rf $tempDir
fi
