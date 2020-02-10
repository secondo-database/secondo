#!/bin/bash

echo "Dir:"
pwd

mkdir -p startClusterLogs

logFile="startClusterLogs/$2"

backup_log_file () {
  # Make a backup if previous log file exists
  if [ -f $logFile ]; then
    DATE_WITH_TIME=$(date "+%Y%m%d-%H%M%S")
    mv $logFile ${logFile}_${DATE_WITH_TIME}.bk
  fi
}

backup_log_file

CMD="$1 >$logFile 2>&1"
echo "Starting: $CMD"

i=0
until eval "$CMD"; do
  ((i++))
  echo "Server satrted with $CMD crashed($i time) with exit code $?.  Respawning.." >&2
  if [[ "$i" == '100' ]]; then
    echo "Reached maximal number of restarts for $CMD"
    break
  fi

  backup_log_file
  sleep 5
done

echo "Exiting loop script"