#!/bin/bash

set -e

# Init variables
scriptName=`basename "$0"`
scriptBaseDir=`dirname "$0"`
scriptDir="$(dirname $(readlink -f $0))"

#echo "pwd: `pwd`"
#echo "\$0: $0"
#echo "basename: `basename $0`"
#echo "dirname: `dirname $0`"
#echo "dirname/readlink: $(dirname $(readlink -f $0))"

if [[ -z ${KAFKA_HOME} ]];
then
  KAFKA_HOME=${HOME}/kafka/kafka_dist
fi

if ! [[ -d "$KAFKA_HOME" ]]
then
    echo "Error: Kafka installation can not be found"
    echo "KAFKA_HOME is $KAFKA_HOME"
    exit 1
fi

echo "KAFKA_HOME is ${KAFKA_HOME}"
cd ${KAFKA_HOME}


zookeeper_pid=`ps ax | grep -i -w 'org.apache.zookeeper.server' | grep -v grep | awk '{print $1}'`
kafka_servers_pids=`ps ax | grep -i -w 'kafka.Kafka' | grep -v grep | awk '{print $1}'`
start_kafka_loops_pids=`ps ax | grep -i -w 'start_kafka_loop.sh' | grep -v grep | awk '{print $1}'`

print_usage () {
  echo "Usage:"
  echo "${scriptName} start"
  echo "${scriptName} stop"
  echo "${scriptName} stophard"
  echo "${scriptName} status"
}


print_status () {
  echo "Status:"
  echo "zookeeper is running as pid: ${zookeeper_pid}"


  while read pid ; do
    echo "kafka server is running as pid: ${pid}"
  done <<< ${kafka_servers_pids}

  while read pid ; do
    echo "loop script is running as pid: ${pid}"
  done <<< ${start_kafka_loops_pids}

  echo "For more information run: ps ax | grep <pid>"
}

start_cluster () {
  if [ -n "${zookeeper_pid}" ]
  then
    echo "Error: Zookeeper is allready running"
    exit 1
  fi

  if [ -n "${kafka_servers_pids}" ]
    then
    echo "Error: Kafka is allready running"
    exit 1
  fi

  echo "Starting Zookeeper"
#  nohup ${scriptDir}/start_kafka_loop.sh "./bin/zookeeper-server-start.sh config/zookeeper.properties" "zookeeper.log" &
#  nohup bin/zookeeper-server-start.sh config/zookeeper.properties >zookeeper.log 2>&1 &
  nohup bin/zookeeper-server-start.sh config/zookeeper.properties >zookeeper.log 2>&1 &

  sleep 5

  echo "Starting Kafka"
#  nohup ${scriptDir}/start_kafka_loop.sh "./bin/kafka-server-start.sh config/server-1.properties" "kafka-1.log" &
#  nohup ${scriptDir}/start_kafka_loop.sh "./bin/kafka-server-start.sh config/server-2.properties" "kafka-2.log" &
#  nohup bin/kafka-server-start.sh config/server-1.properties >kafka-1.log 2>&1 &
#  nohup bin/kafka-server-start.sh config/server-2.properties >kafka-2.log 2>&1 &
  nohup bin/kafka-server-start.sh config/server.properties >kafka.log 2>&1 &
  echo "Zookeper and Kafka startup initialized. Check nohup.out, startClusterLogs/zookeeper.log and startClusterLogs/kafka-x.log in ${KAFKA_HOME} for sucessful startup"
}

stop_cluster () {

        kill_op=$1

        echo "Shutting down loop scripts";
        if [ -n "${start_kafka_loops_pids}" ]
          then
            while read pid ; do
              echo "Killing loop script with pid ${pid}"
              kill ${kill_op} "${pid}"
            done <<< ${start_kafka_loops_pids}
            sleep 1
        else
          echo "Loop scripts were not Running"
        fi

        echo "Shutting down Kafka servers";
        if [ -n "${kafka_servers_pids}" ]
          then
            while read pid ; do
              echo "Killing kafka server with pid ${pid}"
              kill ${kill_op} "${pid}"
            done <<< ${kafka_servers_pids}
            sleep 10
        else
          echo "Kafka was not Running"
        fi

        echo "Shutting down Zookeeper";
        if [ -n "${zookeeper_pid}" ]
          then
            echo "killing Zookeeper ${zookeeper_pid}"
            kill ${kill_op} "${zookeeper_pid}"
        else
          echo "Zookeeper was not Running"
        fi

        exit 0
}

# https://stackoverflow.com/questions/34512287/how-to-automatically-start-kafka-upon-system-startup-in-ubuntu
case "$1" in
  start)
    start_cluster
    ;;
  stop)
    stop_cluster
    ;;
  stophard)
    stop_cluster "-9"
    ;;
  status)
    print_status
    ;;
  *)
    print_usage
esac

exit 0

# https://stackoverflow.com/questions/34512287/how-to-automatically-start-kafka-upon-system-startup-in-ubuntu


