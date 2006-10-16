#!/bin/sh
#
# Sept. 2006, M. Spiekermann
#
# Test script for the libutil.sh functions
#

# include function definitions
libFile="./libutil.sh"
if ! source $libFile; then 
  printf "%s\n" "This script needs routines from the file $libFile"
  exit 1; 
fi


function showTests()
{
  local i=0
  for name in ${testName[*]}; do
    local info=${testInfo[$i]}
    echo -e "$name : $info"
    let i++
  done
}

testName[0]="msgs"
testInfo[0]="Tests the printSep function"
testName[1]="timeOut"
testName[2]="killProcess"
testName[3]="findChilds"
testName[4]="isRunning"
testName[5]="sendMail"
testName[6]="mapStr" 
testName[7]="uncompress"
testName[8]="uncompressFolders"
testName[9]="uncompressFiles"
testName[10]="varValue"
testName[11]="initLogFile"
testName[12]="startupXterm"
testName[13]="createDB"
testName[14]="isCmdPresent"
testName[15]="checkVersion"
testName[16]="createDir"
testName[17]="showValue"


# check test name 
declare -i i=0
testStr=""
for name in ${testName[*]}; do
  if [ "$1" == "$name" ]; then
    testStr=${testName[$i]}
  fi
  let i++
done

if [ "$testStr" == "" ]; then
  echo -e "\nError: Test \"$testStr\" unknown! Possible tests are:\n"
  showTests
  exit 1
fi



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

if [ "$1" == "isRunning" ]; then
  if isRunning $2; then
    echo "Yes"
  else
    echo "No"
  fi
  exit $?
fi

if [ "$1" == "sendMail" ]; then  

LU_SENDMAIL="$2"
XmailBody="This is a generated message!  

  Users who comitted to CVS yesterday:

  You will find the output of make in the attached file.
  Please fix the problem as soon as possible."

sendMail "Test Mail!" "spieker root" "$XmailBody" "$3" "$4"

fi

if [ "$1" == "mapStr" ]; then

   cat $2
   mapStr "$2" "$3" "$4"
   echo $name1 $name2
   printf "%s\n" "\"$3\" -> \"$LU_MAPSTR\""
   
fi

if [ "$1" == "uncompress" ]; then
  uncompress $2 $3
fi

if [ "$1" == "uncompressFolders" ]; then
  shift
  xdir="/tmp/libutil-tests"
  assert rm -rf $xdir
  assert mkdir $xdir
  assert cd $xdir
  if [ $? -ne 0 ]; then
    exit $?
  fi
  uncompressFolders $*
fi

if [ "$1" == "uncompressFiles" ]; then
  shift
  xdir="/tmp/libutil-tests"
  assert rm -rf $xdir
  assert mkdir $xdir
  assert cd $xdir
  if [ $? -ne 0 ]; then
    exit $?
  fi
  uncompressFiles $*
fi

if [ "$1" == "varValue" ]; then
  varValue $2
  echo -e "\n rc=$?"
  echo -e "\n <$LU_VARVALUE> \n"
  exit $?
fi

if [ "$1" == "initLogFile" ]; then
  initLogFile
  echo -e "\n rc=$?"
  exit $?
fi

if [ "$1" == "startupXterm" ]; then
  title="$2"
  shift 2
  startupXterm "$title" $*
fi

if [ "$1" == "createDB" ]; then
  db="$2"
  file="$3"
  createSecondoDatabase "$db" "$file" 
fi

if [ "$1" == "isCmdPresent" ]; then

  for cmd in "nice" "xkjfhd"; do
    isCmdPresent "$cmd"
    echo -e "$cmd: $?" 
  done
fi

# $1: version number
function returnVersion {
  echo "sdkfjh jhdf "$1" (sdlfkj) lsdkfj"
}


if [ "$1" == "checkVersion" ]; then

  flex --version
  if checkVersion "flex --version" "1.1"; then
    echo "version is higher or equal than 1.1"
  fi
  if ! checkVersion "flex --version" "9.1"; then
    echo "version is smaller than 9.1"
  fi
  if ! checkVersion "gcc --version" "4.02"; then
    echo "version is smaller than 4.02"
  fi
  if ! checkVersion "gcc --version" "3.01"; then
    echo "version is higher or equal 3.01"
  fi

  for v in "1.01" "0.07" "2.1" "3.5.5" "4.02.1"; do
    for v2 in "1.01" "0.07" "2.1" "3.5.6" "4.0"; do
    #returnVersion "$v" "$v2"
    if ! checkVersion "returnVersion $v" "$v2"; then
      echo "Version $v < $v2!"
    else  
      echo "Version $v >= $v2!"
    fi
   done
  done  
  
  if ! checkVersion "returnVersion 4.1.1" "gcc-core-4.1.1.tar.gz"; then
    echo "Error: gcc 4.1.1 expected !"
  else
    echo "gcc 4.1.1 as expected"
  fi
    
  
fi

if [ "$1" == "createDir" ]; then
 createDir $2
fi

if [ "$1" == "showValue" ]; then
 xyz="hallo"
 abc="test"
 showValue xyz
 showValue abc 
fi
