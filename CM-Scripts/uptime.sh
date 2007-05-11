#!/bin/sh

while [ "1" == "1" ]; do

  d=$(date "+%m.%d.")
  u=$(uptime)
  echo "$d - $u" >> uptime.log
  sleep 300 
done
