#!/bin/bash

set -e

if [[ -z ${KAFKA_HOME} ]];
then
    echo "Error: variable KAFKA_HOME is not set"
    exit 1
fi
echo "KAFKA_HOME is ${KAFKA_HOME}"
cd ${KAFKA_HOME}

# Init variables
scriptName=`basename "$0"`
zookeeper_pid=`ps ax | grep -i -w 'org.apache.zookeeper.server' | grep -v grep | awk '{print $1}'`
kafka_servers_pids=`ps ax | grep -i -w 'kafka.Kafka' | grep -v grep | awk '{print $1}'`

print_usage () {
  echo "Usage:"
  echo "${scriptName} start"
  echo "${scriptName} stop"
  echo "${scriptName} status"
}


print_status () {
  echo "Status:"
  echo "zookeeper is running as pid ${zookeeper_pid}"


  while read pid ; do
    echo "kafka server is running as pid ${pid}"
  done <<< ${kafka_servers_pids}

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
  nohup bin/zookeeper-server-start.sh config/zookeeper.properties >zookeeper.log 2>&1 &

  echo "Starting Kafka"
  nohup bin/kafka-server-start.sh config/server-1.properties >kafka-1.log 2>&1 &
  nohup bin/kafka-server-start.sh config/server-2.properties >kafka-2.log 2>&1 &

  echo "Zookeper and Kafka startup initialized. Check ${KAFKA_HOME}/zookeeper.log and ${KAFKA_HOME}/kafka-x.log for sucessful startup"
  print_status
}

stop_cluster () {
        echo "Shutting down Zookeeper";
        if [ -n "${zookeeper_pid}" ]
          then
            echo "killing ${zookeeper_pid}"
            kill -9 "${zookeeper_pid}"
        else
          echo "Zookeeper was not Running"
        fi

        echo "Shutting down Kafka servers";
        if [ -n "${kafka_servers_pids}" ]
          then
            while read pid ; do
              echo "Killing kafka server with pid ${pid}"
              kill -9 "${pid}"
            done <<< ${kafka_servers_pids}
        else
          echo "Kafka was not Running"
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
  status)
    print_status
    ;;
  *)
    print_usage
esac

exit 0

# https://stackoverflow.com/questions/34512287/how-to-automatically-start-kafka-upon-system-startup-in-ubuntu


