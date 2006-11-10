#!/bin/sh
#
# Jan 2005, M. Spiekermann. This is a small library
# of functions useful for several shell scripts.
#
# July 2005, M. Spiekermann. Improvements for killing processes.
# Since kill does not kill child processes the functions findChilds
# and killProcess were introduced. Moreover the timeOut function was 
# revised to work without active waiting. 
#
# Jan 2006, M. Spiekermann. Function sendMail revised. Mails and theirs attachment
# will now be backed up in a configurable directory. Moreover a new function 
# ~isCmdPresent~ was added. 
#
# Sept 2006, M. Spiekermann. Function ~checkVersion~ introduced. The tests are
# relocated into a new file called libutil-test.sh.


# recognize aliases also in a non interactive shell
shopt -s expand_aliases

# global variables used to store results 
LU_LOG=""
LU_LOG_INIT=""

LU_VARVALUE=""

LU_TMP=""
LU_USERTMP=""

LU_CHILDS=""

LU_MAPSTR=""
LU_xPID=""

declare -i LU_STEPCTR=1
declare -i LU_RC=0
declare -i LU_ERRORS=0

# global constants which influence the 
# behavior of some functions
LU_TESTMODE=""
LU_SENDMAIL="true"

LU_RULER="--------------------------------------------------------------------"

# colors
LU_normal="\033[0m"
LU_red="\033[31m"
LU_green="\033[32m"
LU_blue="\033[34m"

###################################################################
###
###  Start of function definitions
###
###################################################################

# getTimeStamp
function getTimeStamp {

  date_TimeStamp=$(date "+%y%m%d-%H%M%S")
  date_ymd=${date_TimeStamp%-*}
  date_HMS=${date_TimeStamp#*-}
}

# show the value of a given variable name
#
# $1 variable name

function showValue
{
  local var=$1
  echo "$var = <"$(eval echo '$'$var)">"
}

# print to log-file
#
function printl {
  if [ -n "$LU_LOG_INIT" ]; then
    printf "$1" "$2" >> $LU_LOG
  fi
}

# print to screen and into log-file if $LU_LOG is nonzero
#
function printx {

  printf "$1" "$2"
  printl "$1" "$2"
}

# 
#
function printlr {
  printl "%s\n" $LU_RULER
}

function printxr {
  printx "%s\n" $LU_RULER
}

# $1 variable name
function varValue {

  LU_VARVALUE=""
  if [ $# -eq 0 ]; then
    return 1
  fi
  
  LU_VARVALUE="\"$(env | grep ^$1=)\""
  return 0
}


# $1 mode = [err, warn, info]
# $2 msg
function showMsg {

  local col=$LU_normal
  local msg=""

  if [ $# == 2 ]; then
    if [ "$1" == "err" ]; then
      col=$LU_red
      msg="ERROR: "      
    fi
    if [ "$1" == "warn" ]; then
      col=$LU_blue
      msg="WARNING: "
    fi  
    if [ "$1" == "info" ]; then
      col=$LU_green
    fi
    if [ "$1" == "em" ]; then
      col=$LU_blue
    fi  
    shift
  fi

  echo -e "${col}${msg}${1}${LU_normal}\n" 
  if [ -n "$LU_LOG_INIT" ]; then
    echo -e "${1}\n" >> $LU_LOG 
  fi
}

# check if we are running in a bash
if [ -z "$BASH" ]; then
  showMsg "ERROR: You need a bash shell to run this script!"
  exit 1
fi

# assert - check if a command was successful
#
# $* cmd
function assert {

  if ! $*; then
    showMsg "err" "Assertion failed!"
    printx "%s\n" "pwd: $PWD" 
    printx "%s\n" "cmd: $*"
    exit 1
  fi
  return 0 
}


# check if /tmp is present and writable and create a user specific
# subdir variables LU_TMP and LU_USERTMP are defined afterwards
#
function createTempDir {

  LU_TMP=/tmp
  if [ ! -w /tmp ]; then
    showMsg "warn" "Directory \"/tmp\" not present or not writable!" 
    if [ ! -w "$HOME" ]; then
      varValue HOME
      showMsg "err" "Directory $LU_VARVALUE not present or not writeable!" 
      exit 1
    fi
    LU_TMP=$HOME/0tmp$!
    if ! mkdir -p $LU_TMP; then
      showMsg "err" "Could not create directory \"$LU_TMP\""
    fi
  fi

  LU_USERTMP=$LU_TMP/shlog-$USER
  if [ ! -d "$LU_USERTMP" ]; then
    if ! mkdir -p $LU_USERTMP; then
     showMsg "err" "Could not create directory \"$LU_USERTMP\""
     exit 1
    fi
  fi

  return 0
}

# create a given directory if it does not exist
#
# $1 dir

function createDir {

 local dir=$1

 if [ ! -d $dir ]; then
   assert mkdir -p $dir
 fi 

}


# write startup information into logfile
# $1 writable log-file
#
function initLogFile {

  if [ -n "$1" ]; then
    LU_LOG=$1
  else
    createTempDir
    LU_LOG=$LU_USERTMP/sh_$$.log
  fi

  if [ -n "$LU_LOG" ]; then
    checkCmd touch $LU_LOG
    if [ $? -ne 0 ]; then
      varValue LU_LOG
      showMsg "err" "Can't touch log-file $LU_VARVALUE!"
      exit 1
    fi
  else
    showMsg "err" "No log-file defined!"
    exit 1
  fi 
  LU_LOG_INIT="true" 

  # set native language support to US English in order to
  # avoid exotic messages in th log file
  export LANG="en_US"

  printl "%s\n" "############################################"
  printl "%s\n" "# Log of $(date)"
  printl "%s\n" "############################################"
  printl "%s\n" "Environment settings:"
  env 2>&1 >> $LU_LOG
  printl "%s\n" "############################################"
}


if [ "$OSTYPE" == "msys" ]; then
   prefix=/c
   platform="win32"
elif [ "$OSTYPE" == "mac_osx" ]; then 
   prefix=$HOME
   platform="mac_osx"
else
   # assuming linux
   prefix=$HOME
   platform="linux"
fi


function win32Host {

  if [ "$OSTYPE" == "msys" ]; then
    return 0
  fi 
  return 1
}



# printSep $1
#
# $1 message
#
# print a separator with a number and a message
function printSep {

  printx "\n%s\n" "Step ${LU_STEPCTR}: ${1}"
  printx "%s\n" "$LU_RULER" 
  let LU_STEPCTR++
}

# checkCmd $*
# $* command
#
# execute a command. In case of an error display the
# return code

function checkCmd {

  printlr
  printl "%s\n" "pwd: $PWD"
  printl "%s\n" "cmd: $*"
  if [ "$LU_TESTMODE" != "true" ]; then
    # call command using eval 
    if [ -z "$LU_LOG_INIT" ]; then
      eval "$*"  
    else
      eval "{ $*; } >> $LU_LOG 2>&1"
    fi
    LU_RC=$?  # save return code
    if [ $LU_RC -ne 0 ]; then
      showMsg "err" "Command {$*} returned with value ${LU_RC}"
      let LU_ERRORS+=1  
    fi  
  fi
  printlr
  return $LU_RC
}


# findChilds
#
# search recursively for child processes. Result is stored
# in global variable LU_CHILDS

LU_TAB=$(echo -e "\t")
function findChilds {

   if [ "$OSTYPE" == "msys" ]; then
     # the msys sed implementation has problems with \t.
     # the next variable holds a TAB value
     #ps -f | sed -ne "s#\([^ $t]*\)[ $t]\+\([0-9]\+\)[ $t]\+$1[ $t].*#\2#p" | cat
     local nextChilds=$(ps -f | sed -ne "s#\([^ $LU_TAB]*\)[ $LU_TAB]\+\([0-9]\+\)[ $LU_TAB]\+$1[ $LU_TAB].*#\2#p" | cat)
   else
     local nextChilds=$(ps h -o pid --ppid $1 2>/dev/null | cat)
   fi
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
    if [ "$platform" != "linux" ]; then
      ps -f | sed -ne 's#\([^ \t]*\)[ \t]*\([0-9]\+\).*#_\2_#p' | grep "_$1_" > /dev/null
      rc=$?
    else
      ps -p $1 > /dev/null
      rc=$?
    fi 
    return $rc
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
   printl "\n%s\n" " Killing process $1"
   findChilds $1
   if [ -z $2 ]; then
      sig=-9
   else
      sig=$2
   fi
   printl "%s\n" " Killing child processes: $LU_CHILDS"
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
    printx "\n%s\n" "Timeout for process $1 reached!"
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

# isCmdPresent $1
#
# $1 command name
#
# checks if a given command is known

function isCmdPresent()
{
  type -t $1 >& /dev/null 
  return $?
}


# sendMail $1 $2 $3 [$4 $5]
#
# $1 subject
# $2 recipients
# $3 body
# $4 backup dir
# $5 attached file
#
# Sends a mail with a given attachment (only 1 file) to the 
# list of recipients. A copy of the mail text and its attachments 
# will be kept in a backup directory. 

function sendMail() {

  # check mandatory arguments
  if [ $# -lt 3 ]; then
    echo -e "\n ${FUNCNAME}: Error, less than 3 arguments!\n"
    return 1
  fi

  if [ "$2" == "" ]; then
    echo -e "\n ${FUNCNAME}: Error, 2nd argument is empty!\n"
    return 1
  fi
 
  # check optional arguments 
  if [ "$4" != "" ]; then
    local backupDir="$4"
  fi
  if [ "$5" != "" ]; then
    local attachOpt="-a $5"
    local attachFile="$5"
  fi


  # send mail
  if [ "$LU_SENDMAIL" == "true" ]; then

    printf "%s\n"   "${FUNCNAME}:  mail -s \"$1\" $attachOpt \"$2\" <body>"
    mail -s"$1" ${attachOpt} "$2" <<-EOFM
$3
EOFM
 
    if [ "$backupDir" != "" ]; then
     
       if [ ! -d $backupDir ]; then
	 assert mkdir -p $backupDir      
       fi	       
	    
       # redirect stdout   
       exec 6>&1
       exec >> "$backupDir/Mails.txt"

       echo -e "Subject    : $1"
       echo -e "Recipients : $2"
       if [ "$attachFile" != "" ]; then
         echo -e "Attachments: $attachFile"
         cp $attachFile $backupDir 
       fi
       echo -e "$3"
       echo -e "------------------------------"

       # restore stdout   
       exec 1>&6 6>&-
    fi

  else
    printf "%s\n"   "${FUNCNAME}: Test mode!"
    printf "%s\n"   "----------------------"
    printf "%s\n"   "  Command   : mail -s \"$1\" $attachOpt \"$2\""
    printf "%s\n"   "  backup Dir: $4"
    printf "%s\n"   "  Mail body : $3"
    printf "%s\n\n" "----------------------"
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
# For each directory all *.gz files are assumed to be a tar archive and
# all *.zip files a zip archive

function uncompressFolders {

  local err=""

  for folder in $*; do
    local files=$(find $folder -maxdepth 1 -iname "*.zip" -or -iname "*.*gz")
    for file in $files; do
      uncompress $file
      if [ $? -ne 0 ]; then
        err="true"
      fi
    done
  done

  if [ -n "$err" ]; then
    return 1

else
    return 0
  fi
}

# $1 extraction dir
# $2, .. $n files
function uncompressFiles {

  local dir=$1
  local err=""

  # change dir
  if ! cd $1; then
    return $?
  fi
  
  shift
  # check if files are present
  if [ -z "$*" ]; then
    return 1;
  fi

  # uncompress files
  for file in $*; do
    uncompress $file
    if [ $? -ne 0 ]; then
      err="true"
    fi
  done

  if [ -n "$err" ]; then
    return 1
  else
    return 0
  fi
}

# $1 file
# $2 target dir
function uncompress {

  local storedPWD=$PWD
  local rc=0
  local run=""

  if [ -z $1 ]; then
    return 0
  fi

  if [ -n "$2" ]; then
    if [ ! -d $2 ]; then
      showMsg "err" "uncompress: Directory $2 does not exist!"
      return 1;
    else
      assert cd $2
    fi
  fi

  local suffix=${1##*.}
  if [ "$suffix" == "gz" -o "$suffix" == "GZ" -o "$suffix" == "tgz" -o "$suffix" == "TGZ" ]; then
    checkCmd "tar -xzf $1"
    rc=$?
    run="true"
  fi

  if [ "$suffix" == "bz2" -o "$suffix" == "BZ2" ]; then
    checkCmd "tar -xjf $1"
    rc=$?
    run="true"
  fi

  if [ "$suffix" == "zip" -o "$suffix" == "ZIP" ]; then
    checkCmd "unzip -q -o $1"
    rc=$? 
    run="true"
  fi

  if [ -n "$run" ]; then
    assert cd $storedPWD
    return $rc
  fi

  assert cd $storedPWD
  showMsg "warn" "uncompress: Don't know how to handle suffix \"$suffix\"."
  return 1;
}

# mapStr
#
# $1 file
# $2 name1
# $3 separator
#
# reads file $1 which contains a list of "name1 name2" entries
# and returns name2 if "$1"=0"name1". The parameter name1 should 
# be unique otherwise the first occurrence will be used.

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

# $1 title
# $* options after xterm -e
function startupXterm {

  local title=$1
  shift
  if [ "$*" == "" ]; then
   return 0
  fi

  if [ -n "$LU_xterm" ]; then

	  ## in some situations xhost access control must be disabled	  
    xhost + > /dev/null 2>&1	  
    $LU_xterm -title "$title" -e $* &
    LU_xPID=""
    sleep 1
    xhost - > /dev/null 2>&1	  
    if ! isRunning $!; then
      showMsg "err" "Could not start \"$LU_xterm -e $*\" in backgound." 
      return 1 
    fi
    LU_xPID=$!
  else
    showMsg "info" "No graphical console present! Child window will not be started." 
    return 1
  fi
  return 0
}


# $1 command 
# $2 version given as "x.y" 
function checkVersion {

  local version1=$($1 | sed -nr '1s#.* ([0-9]+)[.]([0-9]+)[.]?([0-9]+)?.*#\1x\2y\3#p')
  local version2=$(echo "$2" | sed -nr '1s#[^0-9]*([0-9]+)[.]([0-9]+)[.]?([0-9]+)?.*#\1x\2y\3#p')
  #echo "$version1 >= $version2 ?"

  local -i n1=$[${version1%x*}]
  local rest=${version1#*x}
  #showValue rest
  local -i n2=$[${rest%y*}]
  local -i n3=$[${rest#*y}]
  
  local -i k1=$[${version2%x*}]
  local rest=${version2#*x}
  #showValue rest
  local -i k2=$[${rest%y*}]
  local -i k3=$[${rest#*y}]
  
  #echo "$n1 $n2 $n3 >= $k1 $k2 $k3 ?"
  LU_Version1="${n1}.${n2}.${n3}" 
  LU_Version2="${k1}.${k2}.${k3}" 
 
  if  let $[$n1 > $k1]; then
    return 0
  fi
  if  let $[$n1 < $k1]; then
    return 1
  fi
  

  # major number is equal, compare 2nd level
  if  let $[$n2 > $k2]; then
    return 0
  fi
  if  let $[$n2 < $k2]; then
    return 1
  fi


  # 2nd level is equal, compare 3rd level
  if  let $[$n3 >= $k3]; then
    return 0
  fi
  return 1
}



# createSecondoDatabase $1 $2
#
# $1 database name
# $2 database file

function createSecondoDatabase() {
  
  local db=$1
  local file=$2

  cd ${buildDir}/bin
  local cmd1="create database ${db};"
  local cmd2="restore database ${db} from '"${file}"';"
  SecondoTTYBDB < "$cmd1; $cmd2; q;"


  
  local rc=$?
 
  if [ $rc -ne 0 ]; then 
    printf "\n%s\n" "Warning could not restore database ${db}! Some subsequent tests may fail."
  fi
  return $rc 
}



###################################################################
###
###  End of function definitions
###
###################################################################

# define some environment variables
createTempDir
TEMP=$LU_TMP

# check if a graphical console is present
type -p rxvt > /dev/null 2>&1
if [ $? -ne 0 ]; then
  type -p xterm > /dev/null 2>&1
  if [ $? -ne 0 ]; then
    showMsg "warn" "No graphical console like  rxvt or xterm available."
    LU_xterm="" 
  else
    LU_xterm="xterm"
  fi
else
  LU_xterm="rxvt"
fi


# some important directories in SECONDO's source tree 
buildDir=${SECONDO_BUILD_DIR}
scriptDir=${buildDir}/CM-Scripts
binDir=${buildDir}/bin
optDir=${buildDir}/Optimizer

# extend PATH variables for using SECONDO inside shell scripts
PATH="${PATH}:${binDir}:${optDir}:${scriptDir}"

#initialize date_ variables
getTimeStamp

