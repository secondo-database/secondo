#!/bin/bash
#
# killtree.sh $1 $* - kill pids $* and all its childs
#
# If $1=dryrun just print out information, don't kill.
# otherwise $1 holds the pid of the root process of the
# recursion. Initially it must be started with $1=0.
#
# June 2005, M. Spiekermann

dryRun=""
if [ "$1" == "dryrun" ]; then
  shift
  dryRun="dryrun"
fi

# first parameter is the root process id
# of the recursion
if [ "$1" == "0" ]; then  
  ownPid=$$
else
  ownPid=$1
fi
shift

childList=""
pidList=$*
declare -i noArgs=$#
while [ $# != 0 ]; do
  if [ "$1" != "$ownPid" ]; then
    tmp_childList=$(ps h -o pid --ppid $1 | tr "\n" " ")
  else
   printf "%s\n" "PID ($ownPid) of killtree.sh excluded!"
  fi
  
  if [ "$tmp_childList" != "" ]; then
    echo "Process $1 has sons."
    childList=$(echo "$tmp_childList" "$childList")
  fi
  shift
done

if [ "$childList" != "" ]; then
  # recursive call to kill the childs
  echo "calling ./killtree.sh $dryRun $ownPid $childList"
  ./killtree.sh $dryRun $ownPid $childList
fi

# kill the given pid list
if [ "$dryRun" == "dryrun" ]; then
  echo "kill -9 $pidList"
else
  kill -9 $pidList
fi
