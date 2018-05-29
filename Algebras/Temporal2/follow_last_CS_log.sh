#!/bin/bash

secondo_bin_dir=~/secondo/bin
# sleep 1
latest_cs_pid=$(ps aux --sort=start_time | grep SecondoBDB | grep -v grep | tail -n1 | sed 's/\s\+/ /g' | cut -d' ' -f2)

if [ "$latest_cs_pid" = "" ]; then
  echo no proc found
  exit 1
else
  cs_log_name=${secondo_bin_dir}/server.msg/msg_${latest_cs_pid}.txt
  echo pid=$latest_cs_pid
  echo cs_log_name=$cs_log_name
  touch $cs_log_name
  less +F $cs_log_name
fi
