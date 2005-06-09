#!/bin/bash
#
# Jan 2005, M. Spiekermann. This is a small library
# of functions useful for several shell scripts.

if [ -z "$BASH" ]; then
  printf "%s\n" "Error: You need a bash shell to run this script!"
  exit 1
fi

if [ "$OSTYPE" == "msys" ]; then
   prefix=/c
   platform="win32"
else 
   prefix=$HOME
   platform="linux"
fi


# recognize aliases also in an non interactive shell
shopt -s expand_aliases

# getTimeStamp
function getTimeStamp() {

date_TimeStamp=$(date "+%y%m%d-%H%M%S")
date_ymd=${date_TimeStamp%-*}
date_HMS=${date_TimeStamp#*-}

}

# printSep $1
#
# $1 message
#
# print a separator with a number and a message
declare -i stepCtr=1
function printSep() {

  printf "\n%s\n" "Step ${stepCtr}: ${1}"
  printf "%s\n" "---------------------------------------------------------------------------"
  let stepCtr++
}

# checkCmd $*
# $* command
#
# execute a command. In case of an error display the
# returncode
declare -i rc=0



function checkCmd() {

  printf "%s\n" "cmd: $*"
  if [ "$testMode" != "true" ]; then
    # call command using eval 
    if [ -z $checkCmd_log ]; then
      eval "$*"  
    else
      printf "%s\n" "-------------------------------------------------" >> $checkCmd_log
      printf "%s\n" "msg for: $*" >> $checkCmd_log
      printf "%s\n" "-------------------------------------------------" >> $checkCmd_log
      eval "{ $*; } >> $checkCmd_log 2>&1"
    fi
    let rc=$?  # save returncode

    if [ $rc -ne 0 ]; then
      printf "\n Failure! Command {$*} returned with value ${rc} \n"
    fi
  fi
  return $rc
}

# findrec
#
# search recursively for child processes

CHILDS=""
function findrec {
   nextchilds=$(ps h -o pid --ppid $1 2>/dev/null | cat)
   CHILDS=$(echo "$nextchilds" "$CHILDS" )
   for nc in  $nextchilds; do
     findrec $nc
   done
}


# terminateAfter $1 $2 $3 pid
#
# $1 max time to wait
# $2 time interval for alive check
# $3 pid of the process to wait

function terminateAfter() {

typeset -i i=0
typeset -i maxTime=$1
typeset -i sleepTime=$2
psCmd="ps --no-heading"
printf "%s\n" "Timeout for PID = ${pid} in ${maxTime} seconds! (Live check every $2 seconds)."

while $psCmd $pid >/dev/null
do
  sleep $sleepTime 
  i=$i+$sleepTime
  if [ $i -ge $maxTime ]
  then
    if $psCmd $pid >/dev/null
    then
      printf "%s\n" "Program is running longer than ${maxTime} seconds! - Killing $pid"
      CHILDS=""
      findrec $pid
      echo "Killing child pids" $CHILDS
      kill -9 $CHILDS
      kill -9 $pid
    fi
    break
  fi
done

}

# timeOut $1 $2 $3
#
# $1 max time to wait
# $2 time interval for alive check
# $3 command
#
# runs checkCmd and kills the process after timeout 

function timeOut() {

checkCmd $3 &
pid=$!

terminateAfter $1 $2 $pid
wait $pid

let rc=$?
return $rc

}


# sendMail $1 $2 $3 [$4]
#
# $1 subject
# $2 recipients
# $3 body
# $4 attached file
#
# Sends a mail (with a given attachment) to the list of
# recipients.
sendMail_Deliver="true"
function sendMail() {

  if [ "${4}" != "" ]; then
    attachment="-a ${4}"
  fi

  if [ "$sendMail_Deliver" == "true" ]; then
  mail -s"$1" ${attachment} "$2" <<-EOFM
$3
EOFM

  else
    printf "%s\n" "Test Mode: Not sending mails !!!"
    printf "%s\n" "Mail Command:"
    printf "%s\n" "mail -s \"$1\" $attchment \"$2\""

  fi

}

#showGPL
#
function showGPL() {

  printf "%s\n"   "Copyright (C) 2004, University in Hagen,"
  printf "%s\n"   "Department of Computer Science,"
  printf "%s\n\n" "Database Systems for New Applications."
  printf "%s\n"   "This is free software; see the source for copying conditions."
  printf "%s\n"   "There is NO warranty; not even for MERCHANTABILITY or FITNESS"
  printf "%s\n"   "FOR A PARTICULAR PURPOSE."
}

#uncompressFolders
#
# $1 list of directories
#
# For each direcory all *.gz files are assumed to be a tar archive and
# all *.zip files a zip archive

function uncompressFolders() {

for folder in $*; do
  zipFiles=$(find $folder -maxdepth 1 -name "*.zip")
  gzFiles=$(find $folder -maxdepth 1 -name "*.*gz")
  for file in $zipFiles; do
    printf "\n  processing $file ..."
    if { ! unzip -q -o $file; }; then
      exit 21 
    fi
  done
  for file in $gzFiles; do
    printf "\n  processing $file ..."
    if { ! tar -xzf $file; }; then
      exit 22 
    fi
  done
done

}

# mapStr
#
# $1 file
# $2 name1
# $3 separator
#
# reads file $1 which contains a list of "name1 name2" entries
# and returns name2 if "$1"=0"name1". The parameter name1 should 
# be unique otherwise the first occurence will be used.
function mapStr() {

  sep=$3
  if [ "$sep" == "" ]; then
    sep=" "
  fi

  mapStr_line=$(grep $2 $1) 
  mapStr_name1=${mapStr_line%%${sep}*}
  if [ "$mapStr_name1" == "$2" ]; then
    #cut off name1 
    mapStr_name2=${mapStr_line#*${sep}}
    # remove trailing blanks
    mapStr_name2=${mapStr_name2%% *} 
  else
    mapStr_name2=""
  fi 
} 

# define some environment variables
TEMP="/tmp"
if [ ! -d $TEMP ]; then
  printf "%s\n" "creating directory ${TEMP}"
fi 

buildDir=${SECONDO_BUILD_DIR}
scriptDir=${buildDir}/CM-Scripts
binDir=${buildDir}/bin
optDir=${buildDir}/Optimizer


PATH="${PATH}:${binDir}:${optDir}:${scriptDir}"
LD_LIBRARY_PATH="/lib:${LD_LIBRARY_PATH}"

#initialize date_ variables
getTimeStamp

####################################################################################
#
# Test functions
#
####################################################################################

if [ "$1" == "test" ]; then  

for msg in "hallo" "dies" "ist" "ein" "test"
do
  printSep $msg
done 

checkCmd "echo 'hallo' > test.txt 2>&1"
checkCmd "dfhsjhdfg > test.txt 2>&1"

timeOut "6" "1" "sleep 8"
timeOut "6" "3" "sleep 8"
timeOut "7" "2" "sleep 6"
timeOut "7" "8" "sleep 6"

timeOut "5" "1" "echo 'test'; sleep 3; [ 1 == 2 ]"
echo "rc = $rc"
timeOut "5" "1" "echo 'test'; sleep 3; [ 1 == 1 ]"
echo "rc = $rc"
timeOut "5" "1" "echo 'test'; sleep 6; [ 1 == 2 ]"
echo "rc = $rc"
timeOut "5" "1" "echo 'test'; sleep 6; [ 1 == 1 ]"
echo "rc = $rc"


sendMail_Deliver="false"
XmailBody="This is a generated message!  

  Users who comitted to CVS yesterday:
  $recipients

  You will find the output of make in the attached file.
  Please fix the problem as soon as possible."

sendMail "Test Mail!" "spieker root" "$XmailBody" "test.txt"

fi

if [ "$1" == "mapTest" ]; then

   cat $2
   mapStr "$2" "$3" "$4"
   printf "%s\n" "\"$3\" -> \"$mapStr_name2\""
fi

if [ "$1" == "tty" ]; then

runTTYBDB "list algebras;
q;"

fi
