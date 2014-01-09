#!/bin/bash

# This script generates a BerlinMOD data set in parallel on a cluster. 
# It can only run after Parallel Secondo has been correctly installed. 
# Besides, it needs following prerequisites: 
#   * Distribute the data files streets, homeRegions and workRegions to the cluster. 
#	* A Hadoop program named GenMOD.jar
# 	* A set of Secondo scripts, include: 
#		- BerlinMOD_DataGenerator_map.SEC		(Generate data on slaves in Map stage)
#		- BerlinMOD_DataGenerator_reduce.SEC	(Generate data on slaves in Reduce stage)
#		- BerlinMOD_DataGenerator_master1.SEC	(Set global parameters on the master database)
#		- BerlinMOD_DataGenerator_master2.SEC	(Collect distributed data on the master at last)
#	* This script must run on the master node of the cluster. 

bin=`dirname "$0"`
bin=`cd "$bin"; pwd`

WARNINFO="Warning !! "
ERRORINFO="ERROR !! "


DBNAME="berlinmod"
SCALEFACTOR=0.01
P_NUMDAYS=0
GALONE=false

# 0. Process the arguments. 
declare -i numOfArgs=$#
let numOfArgs++

while [ $numOfArgs -ne $OPTIND ]; do
  getopts "hd:s:p:l" optKey
  if [ "$optKey" == "?" ]; then
    optKey="h"
  fi
  
  case $optKey in 
    h)
    	echo -en "Usage of ${0##*/}:\n\n"
    	echo -en "	-h Print this message and exit. \n\n"
    	echo -en "	-d Set the created Database name (default) ${DBNAME} \n\n"
    	echo -en "	-s Set the Scale factor of the data set (default) ${SCALEFACTOR} \n\n"
    	echo -en "	-p Set the Period of simulation by days (default) NULL \n\n"
    	echo -en "	-l Generate the data set Lonely on one computer (default) ${GALONE} \n\n "
        exit 0
    ;;
    d)
    	DBNAME="${OPTARG}"
    	if [ -z "${DBNAME}" ]; then
  		  echo -en "${ERRORINFO}The database name cannot be empty. \n\n"
          exit -1
        fi
    ;;
    s)
    	SCALEFACTOR=${OPTARG}
    	CHK=$(echo ${SCALEFACTOR/./} | grep "^[0-9]*$")
    	if [ -z "$CHK" ]; then
    	  echo -en "${ERRORINFO}The input scale factor ${SCALEFACTOR} is invalid.\n\n"
    	  exit -1
    	elif [ "${CHK}" = "${SCALEFACTOR}" ]; then
    	  echo -en "${ERRORINFO}The scale factor ${SCALEFACTOR} should be a real number.\n\n"
    	  exit -1
    	fi
    ;;
    p)
    	P_NUMDAYS=${OPTARG}
    	CHK=$(echo ${P_NUMDAYS/./} | grep "^[0-9]*$")
    	if [ -z "$CHK" ]; then
    	  echo -en "${ERRORINFO}The simulate period ${P_NUMDAYS} is invalid.\n\n"
    	  exit -1
    	elif [ "${CHK}" != "${P_NUMDAYS}" ]; then
    	  echo -en "${ERRORINFO}The simulate period ${P_NUMDAYS} should be an integer.\n\n"
    	  exit -1
    	elif [ ${P_NUMDAYS} -le 0 ]; then
    	  echo -en "${ERRORINFO}The simulate period ${P_NUMDAYS} should at least be one day.\n\n"
    	  exit -1	
    	fi
    ;;
    l)
    	GALONE=true
    ;;
  esac
done

# Check the installation of Parallel SECONDO
if [ ! -d ${PARALLEL_SECONDO_MAINDS}/bin ]; then
  echo -en "${ERRORINFO}The Parallel Secondo is not correctly installed. \n\n"
  exit -1
fi
source ${PARALLEL_SECONDO_MAINDS}/bin/ps-functions


SUBDBNAME="ps${DBNAME}"
if [ ${#SUBDBNAME} -gt 16 ]; then
  echo -en "${ERRORINFO}The database name ${DBNAME} is too long. \n\n"
  exit -1
fi

tcmd=$(which ps-startTTYCS)
if [ $? -ne 0 ]; then
  echo -en "${ERRORINFO}The Parallel Secondo is not correctly installed. \n\n"
  exit -1
fi

if [ ! -f "${bin}/BerlinMOD_DataGenerator_master1.SEC" ] \
 || [ ! -f "${bin}/BerlinMOD_DataGenerator_master2.SEC" ] \
 || [ ! -f "${bin}/BerlinMOD_DataGenerator_map.SEC" ] \
 || [ ! -f "${bin}/BerlinMOD_DataGenerator_reduce.SEC" ] \
 || [ ! -f "${bin}/GenMOD.jar" ]; then
  echo -en "${ERRORINFO}Cannot find SECONDO scripts and the hadoop program. \n\n"
  exit -1
fi

CONFFILE=${PARALLEL_SECONDO_MAINDS}/${PARALLEL_SECONDO_MINI_NAME}/bin/SecondoConfig.ini
if [ ! -f $CONFFILE ]; then
  echo -en "${ERRORINFO}The configuration of the Parallel Secondo is wrong. \n\n"
  exit -1
fi
LFNAME="/tmp/SECMON_$(get_secPort $CONFFILE)"

if ${GALONE} ; then
  if [ -f ${LFNAME} ]; then
    echo -en "${ERRORINFO}Stop the running monitors first if you like generate data on a single computer. \n\n"
    exit -1
  fi
  
  ps-startTTY -s 1 <<< "delete database ${DBNAME};
create database ${DBNAME};
open database ${DBNAME};
let SCALEFACTOR = ${SCALEFACTOR};
close database;
q;"
  
  if [ ${P_NUMDAYS} -gt 0 ]; then
    ps-startTTY -s 1 <<< "open database ${DBNAME};
let P_NUMDAYS = ${P_NUMDAYS};
close database;
q;"
  fi
  
  ps-startTTY -s 1 <<< "open database ${DBNAME};
@@${bin}/BerlinMOD_DataGenerator_map.SEC
@@${bin}/BerlinMOD_DataGenerator_reduce.SEC
close database;
q;"
 
  exit 0
fi

if [ ! -f ${LFNAME} ]; then
  echo -en "${ERRORINFO}The monitors are not started. \n\n"
  exit -1
fi

ps-startTTYCS -s 1 <<< "delete database ${DBNAME};
create database ${DBNAME};
open database ${DBNAME};
let SCALEFACTOR = ${SCALEFACTOR};
close database;
q;"

if [ ${P_NUMDAYS} -gt 0 ]; then
  ps-startTTYCS -s 1 <<< "open database ${DBNAME};
let P_NUMDAYS = ${P_NUMDAYS};
close database;
q;"
fi

ps-startTTYCS -s 1 <<< "open database ${DBNAME};
@@${bin}/BerlinMOD_DataGenerator_master1.SEC
close database;
q;"

if [ $? -ne 0 ];then
  echo -en "${ERROR}Initialization on the master DS fails.\n\n"
  exit -1
fi

hadoop jar ${bin}/GenMOD.jar GenMOD ${SUBDBNAME} ${SCALEFACTOR} ${P_NUMDAYS} ${bin}/BerlinMOD_DataGenerator_map.SEC ${bin}/BerlinMOD_DataGenerator_reduce.SEC

if [ $? -ne 0 ];then
  echo -en "${ERROR}The Hadoop program fails.\n\n"
  exit -1
fi

RESULTFILE=$bin/.genResult
hadoop dfs -cat OUTPUT-MOD/p* > ${RESULTFILE}
RESULT="("
while read ROW DS COLUMN; do
  RESULT="${RESULT}(${ROW} ${DS} ${COLUMN} '') "
done < ${RESULTFILE}
RESULT="${RESULT})"

ps-startTTYCS -s 1 <<< "open database ${DBNAME};
let locRel = [const rel(tuple([Row:int, DS:int, Column:int, Path:text])) value ${RESULT} ];
@@${bin}/BerlinMOD_DataGenerator_master2.SEC
close database;
q;"
