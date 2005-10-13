#!/bin/bash
#
# Jan 2005, M. Spiekermann. This is a small library
# of functions useful for several shell scripts.
#
# July 2006, M. Spiekermann. Improvements for killing processes.
# Since kill does not kill child processes the functions findChilds
# and killProcess were introduced. Moreover the timeOut function was 
# revised to work without active waiting. 
#

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
declare -i LU_STEPCTR=1
function printSep() {

  printx "\n%s\n" "Step ${LU_STEPCTR}: ${1}"
  printx "%s\n" "---------------------------------------------------------------------------"
  let LU_STEPCTR++
}

# checkCmd $*
# $* command
#
# execute a command. In case of an error display the
# returncode
declare -i LU_RC=0

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
    let LU_RC=$?  # save returncode

    if [ $LU_RC -ne 0 ]; then
      printf "\n Failure! Command {$*} returned with value ${LU_RC} \n"
    fi
  fi
  return $LU_RC
}

# printx
#
# print to screen and into logfile
function printx {

  arg1=$1
  arg2=$2
  shift
  shift
  printf "$arg1" "$arg2"
  if [ "$checkCmd_log" != "" ]; then
    printf "$arg1" "$arg2" >> $checkCmd_log
  fi

}

# findChilds
#
# search recursively for child processes. Result is stored
# in global variable LU_CHILDS

LU_CHILDS=""
function findChilds {

   local nextChilds=$(ps h -o pid --ppid $1 2>/dev/null | cat)
   local nc=""

   LU_CHILDS="$nextChilds $LU_CHILDS"

   for nc in  $nextChilds; do
     findChilds $nc
   done
}


# isRunning $1
#
# PID to check
#
# checks if the given process is still running

function isRunning {

  if [ "$1" != "" ]; then
    ps -p $1 > /dev/null
    return $?
  else
    return 1
  fi

}

# killProcess $1
#
# $1 PID to kill
#
# this function kills the process and its childs

function killProcess {
   echo -e "\n Time-out reached! Killing process $1"
   findChilds $1
   if [ -z $2 ]; then
      sig=-9
   else
      sig=$2
   fi
   echo -e " Childs: $LU_CHILDS\n"
   kill $sig $1 $LU_CHILDS >/dev/null 2>&1
   return 0
}

# killAfterTimeout $1 $2
#
# $1 PID to kill
# $2 seconds to wait
#
# function sending SIGINT to $2 when timeout of $1 is reached.
# Should only be started in background!

function killAfterTimeOut {
  sleep $2
  # check if process is still running
  if isRunning $1; then
    killProcess $1
  fi
  exit 0
}

# timeOut $1 $2 .... 
#
# $1 max seconds to wait
# $2 ... command and args
#
# runs checkCmd and kills the process after timeout 

function timeOut() {

  echo -e "${FUNCNAME}: args = $*\n"

  local seconds=$1
  shift
  
  # start backgound process for command
  # and store its process id
  eval $*&
  local TIMEOUT_PID=$!

  # start function (in backgound) which will 
  # send kill the process which executes the comamnd
  # after timeout
  killAfterTimeOut $TIMEOUT_PID $seconds&
  local FUNC_PID=$!

  echo -e "${FUNCNAME}: command PID=$TIMEOUT_PID, killfunc PID=$FUNC_PID\n"

  # wait for termination of the command 
  wait $TIMEOUT_PID
  local rc=$?
  LU_RC=rc
  
  # if the command finished before timeout
  # kill the sleeping process
  if isRunning $FUNC_PID; then 
    echo -e "${FUNCNAME}: Command finished before time-out!"
    killProcess $FUNC_PID -15 >/dev/null 2>&1
  fi
  return $rc

}

# lastRC 
#
# returns $LU_RC
function lastRC {
  return $LU_RC
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
LU_SENDMAIL="true"
function sendMail() {

  if [ "$1" == "" ]; then
    echo -e "${FUNCNAME}: Error, no recipients in argument list!\n"
    return 1
  fi

  if [ "${4}" != "" ]; then
    local attachment="-a ${4}"
  fi

  if [ "$LU_SENDMAIL" == "true" ]; then

  # send mail
  mail -s"$1" ${attachment} "$2" <<-EOFM
$3
EOFM
  
  # print warning
  else
    printf "%s\n" "Test Mode: Not sending mails !!!"
    printf "%s\n" "Mail command:"
    printf "%s\n" "  mail -s \"$1\" $attchment \"$2\""
    printf "%s\n" "Mail body: $3"

  fi
  return 0
}

# showGPL
#
# Prints out the GPL disclaimer.

function showGPL() {

  printf "%s\n"   "Copyright (C) 2004, University in Hagen,"
  printf "%s\n"   "Department of Computer Science,"
  printf "%s\n\n" "Database Systems for New Applications."
  printf "%s\n"   "This is free software; see the source for copying conditions."
  printf "%s\n"   "There is NO warranty; not even for MERCHANTABILITY or FITNESS"
  printf "%s\n"   "FOR A PARTICULAR PURPOSE."
}

# uncompressFolders
#
# $1 list of directories
#
# For each direcory all *.gz files are assumed to be a tar archive and
# all *.zip files a zip archive

function uncompressFolders() {

for folder in $*; do
  local zipFiles=$(find $folder -maxdepth 1 -name "*.zip")
  local gzFiles=$(find $folder -maxdepth 1 -name "*.*gz")
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

  local sep=$3
  if [ "$sep" == "" ]; then
    sep=" "
  fi

  local line=$(grep $2 $1) 
  local name1=${line%%${sep}*}
  local name2=""

  if [ "$name1" == "$2" ]; then
    #cut off name1 
    name2=${line#*${sep}}
    # remove trailing blanks
    name2=${name2%% *} 
  else
    name2=""
  fi
 
  LU_MAPSTR=$name2
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

LU_TRACE=0;

if [ "$1" == "msgs" ]; then  

for msg in "hallo" "dies" "ist" "ein" "test"
do
  printSep $msg
done 

checkCmd "echo 'hallo' > test.txt 2>&1"
rc=$?
lastRC
x=$?
echo "rc = $rc, lastRC=$x"

checkCmd "dfhsjhdfg > test.txt 2>&1"
rc=$?
lastRC
x=$?
echo "rc = $rc, lastRC=$x"

fi

if [ "$1" == "timeOut" ]; then  

printSep "Command is running longer than timeout"
timeOut 2 sleep 4
printSep "Command finishs before timeout"
timeOut 4 sleep 2

printSep "Checking return codes"
timeOut 5 "sleep 3; [ 1 == 2 ]"
echo "LU_RC, rc = $LU_RC, $?"
timeOut 5 "sleep 3; [ 1 == 1 ]"
echo "LU_RC, rc = $LU_RC, $?"
timeOut 5 "sleep 6; [ 1 == 2 ]"
echo "LU_RC, rc = $LU_RC, $?"
timeOut 5 "sleep 6; [ 1 == 1 ]"
echo "LU_RC, rc = $LU_RC, $?"

fi

if [ "$1" == "killProcess" ]; then  
  killProcess $2 $3
fi

if [ "$1" == "findChilds" ]; then  
  findChilds $2
  echo $LU_CHILDS
fi

if [ "$1" == "sendMail" ]; then  

LU_SENDMAIL="false"
XmailBody="This is a generated message!  

  Users who comitted to CVS yesterday:
  $recipients

  You will find the output of make in the attached file.
  Please fix the problem as soon as possible."

sendMail "Test Mail!" "spieker root" "$XmailBody" "test.txt"

fi

if [ "$1" == "mapStr" ]; then

   cat $2
   mapStr "$2" "$3" "$4"
   echo $name1 $name2
   printf "%s\n" "\"$3\" -> \"$LU_MAPSTR\""
   
fi
