#!/bin/sh
#
#
# 16.06.04 T. Behr & M. Spiekermann 
#

cmd="$1"
if [ "x$cmd" == "x" ]; then
  echo -e "\n Usage: $0 <Program> \n   monitors the memory usage of a running program. \n"
  exit 1
fi

date1=$(date "+%Y-%m-%d__%H-%M-%S")
outfile=$1"__"$date1"__memprof.log"
pidcmd="ps -o pid -C"$cmd
pid=$( $pidcmd | tail -n1 )

if [ "$pid" == "$($pidcmd | head -n1)" ]; then
  echo -e "\n Process $cmd is not running! \n"
  exit 2
fi

pscmd="ps -o rss,vsize,pcpu,pmem --pid $pid"

ctr="1"

{ echo -n "Nr";  $pscmd | head -n1;} | tee $outfile 
while [ "" == "" ]; do
  if [ "$($pscmd | head -n1)" == "$($pscmd | tail -n1)" ]; then
    exit 1
  fi 
  { echo -n "$ctr " ; $pscmd | tail -n1; } | tee -a $outfile 
  sleep 1 
  ctr=$(expr $ctr + 1)
done
