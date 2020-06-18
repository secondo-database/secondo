#!/bin/bash
#
# March 2011, Jiamin Lu

printf "%s\n" "Starting DBLPgen ..."

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
dbName="dblp"
bdbHome=${HOME}/secondo-databases
fileName="dblp.xml"

declare -i numOfArgs=$#
let numOfArgs++

while [ $# -eq 0 -o $numOfArgs -ne $OPTIND ]; do

  getopts "hf:d:b:" optKey
  if [ "$optKey" == "?" ]; then
    optKey="h"
  fi

  case $optKey in

   h) showGPL
      printf "\n%s\n" "Usage of ${0##*/}:" 
      printf "%s\n"   "  -h Print this message and exit"
      printf "%s\n"   "  -f<xml file contains dblp data> => $fileName"
      printf "%s\n"   "  -b<directory for berkeley-db files> => $HOME/secondo-databases"
      printf "%s\n\n" "  -d<database name> => $dbName"
      exit 0;;

   f) fileName=$OPTARG;;
   
   d) dbName=$OPTARG;;

   b) bdbHome=$OPTARG
 
  esac

done

if [ -z $buildDir ]; then 

  printf "%s\n" "Error: Variable SECONDO_BUILD_DIR undefined."
  exit 1
fi 

printf "\n%s\n" "Creating database ${dbName}!"

dblpDir=${buildDir}/Tools/Converter/Dblp2Secondo
tempDir=/tmp/$USER/Dblp/tmp_Dblpgen_${date_ymd}_${date_HMS}

PATH="$PATH:."

# Generate data
printSep "Generating TPC-H data"
cd $dblpDir
if [ ! -e dbgen ]; then
assert make
fi

checkCmd "dblp2secondo $fileName" 
if [ $? -ne 0 ]; then
  exit 1
else
  assert mkdir -p $tempDir
  printf "%s\n" "Moving files into ${tempdir}"
  assert mv Document $tempDir
  assert mv Author $tempDir
  assert mv Authordoc $tempDir
  assert mv Keyword $tempDir
fi

# restore DBLP tables created by dblp2secondo
printSep "Restore database $dbName"
assert cd $tempDir

export SECONDO_PARAM_SecondoHome=$bdbHome

ncmd=$(which nice)
niceOpt="nice -n19"
if [ $? -ne 0 ]; then
  niceOpt=""    
fi

$niceOpt SecondoTTYNT <<< "create database ${dbName};
open database ${dbName};
@${dblpDir}/restore_objs
q;"

if [ $? -eq 0 ]; then
 rm -rf $tempDir
fi



