#!/bin/sh
#
# July 2008, M. Spiekermann

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
dbName="tpcd"
scaleFactor="0.01"
zipfFactor=""
bdbHome=${HOME}/secondo-databases

declare -i numOfArgs=$#
let numOfArgs++

while [ $# -eq 0 -o $numOfArgs -ne $OPTIND ]; do

  getopts "ihd:s:b:z:" optKey
  if [ "$optKey" == "?" ]; then
    optKey="h"
  fi

  case $optKey in

   h) showGPL
      printf "\n%s\n" "Usage of ${0##*/}:" 
      printf "%s\n"   "  -h Print this message and exit"
      printf "%s\n"   "  -i generate and import data into SECONDO"
      printf "%s\n"   "  -s<volume factor given as real number> => 0.01"
      printf "%s\n"   "  -z<zipf factor (skewed data)> unused by default"
      printf "%s\n"   "  -b<directory for berkeley-db files> => $HOME/secondo-databases"
      printf "%s\n\n" "  -d<database name> => $dbName"
      exit 0;;
   
   s) scaleFactor=$OPTARG;;

   z) zipfFactor=$OPTARG;;

   d) dbName=$OPTARG;;

   b) bdbHome=$OPTARG

  esac

done

if [ -z $buildDir ]; then 

  printf "%s\n" "Error: Variable SECONDO_BUILD_DIR undefined."
  exit 1
fi 


#check algebra modules
./listAlgebras.sh >& list.out 


function checkAlg {
  cat list.out | grep -e "$1"
  if [ $? -ne 0 ]; then
    echo -e "\n Error: $1 needs to be present! \n"
    exit 1  
  fi
}

checkAlg "ImExAlgebra"
checkAlg "TopOpsAlgebra"
checkAlg "TopRelAlgebra"


printf "\n%s\n" "Creating database ${dbName} with a scale factor ${scaleFactor}!"

tpcDir=${buildDir}/Tools/Generators/TPC-D

PATH="$PATH:."


# Generate data
printSep "Generating TPC-D data"
cd $tpcDir
if [ ! -e dbgen ]; then
assert make
fi

dbgenOpt=" -f -s $scaleFactor"
if [ ! $zipfFactor == "" ]; then
  dbgenOpt="$dbgenOpt -z $zipfFactor"
fi

checkCmd "dbgen $dbgenOpt" 
if [ $? -ne 0 ]; then
  exit 1
fi


# restore TPC-H Benchmark tables created by dbgen
printSep "Restore database $dbName"

export SECONDO_PARAM_SecondoHome=$bdbHome
#export SECONDO_PARAM_RTFlags="SMI:NoTransactions,DEBUG:DemangleStackTrace,CMSG:Color"

niceOpt="nice -n19"
ncmd=$(which nice)
if [ $? -ne 0 ]; then
  niceOpt=""    
fi


f="cmds.sec"

echo "create database ${dbName};" > $f
echo "open database ${dbName};" >> $f
echo "@tpcgen.sec" >> $f

$niceOpt SecondoTTYNT -i $f

if [ $? -eq 0 ]; then
 rm -rf *.tbl
fi
