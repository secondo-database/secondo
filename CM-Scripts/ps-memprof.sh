#!/bin/sh
#
# 16.06.04 T. Behr & M. Spiekermann 
#

cmd="$1"
if [ "x$cmd" == "x" ]; then
  echo -e "\n Usage: $0 <process-name> [<logfile>] [time-interval] "
  echo -e "\n monitors the memory usage of a running program. \n"
  exit 1
fi

# define log file name
if [ "$2" == "" ]; then
  date1=$(date "+%Y-%m-%d__%H-%M-%S")
  outfile=$1"__"$date1"__memprof.log"
else
  outfile="$2"
fi

# define time interval in seconds
timeInterval="1"
if [ "$3" != "" ]; then
  timeInterval="$3"
fi

pidcmd="ps -o pid -C"$cmd
pid=$( $pidcmd | tail -n1 )

if [ "$pid" == "$($pidcmd | head -n1)" ]; then
  echo -e "\n Process $cmd is not running! \n"
  exit 2
fi

pscmd="ps -o rss,vsize,pcpu,pmem --pid $pid"

ctr="1"

{ echo -n "Nr";  $pscmd | head -n1;} > $outfile 
while [ "" == "" ]; do
  if [ "$($pscmd | head -n1)" == "$($pscmd | tail -n1)" ]; then
    exit 1
  fi 
  { echo -n "$ctr " ; $pscmd | tail -n1; } >> $outfile 
  sleep $timeInterval 
  ctr=$(expr $ctr + 1)
done
