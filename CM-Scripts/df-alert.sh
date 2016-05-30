#!/bin/sh
#
# May 2007, M. Spiekermann
#
# Send an email alert if disk capacity runs out 
#


# Include function definitions of libutil.sh. It must be in the same direcory
# as this script file or in the search path. 

baseDir=$HOME/${0%/*}
if [ -s $baseDir/libutil.sh ]; 
then
  if ! source $baseDir/libutil.sh; then exit 1; fi
else
  if ! source libutil.sh; then exit 1; fi
fi

if [ $? -ne  0 ]; then 
  printf "%s\n" "This script needs routines from the file $libFile"
  exit 1; 
fi

if [ $# != 2 ]; then
  echo "Error: please specify device and threshold value, e.g. /dev/sda1 90"
  exit 1
fi

device="$1"
declare -i treshold=$2


function dfUsage()
{
  local device="$1"
  local -i usage=$(df -H /dev/sda1 | sed -ne '2s#\(.*\) \([0-9]*\)%\(.*\)#\2#p')

  return $usage
}

dfUsage $device 
declare -i usage=$?

if (( $usage > $treshold )); then

LU_SENDMAIL_FROM="Simone.Jandt@fernuni-hagen.de"
LU_SENDMAIL="true"
XmailBody="This is a generated message!  
 
  Host      : $HOST
  Device    : $device 
  Fill rate : ${usage}%
"

  sendMail "disk alert! on $HOST" "fabio.valdes@fernuni-hagen.de" "$XmailBody"
fi
