#!/bin/bash

set -e

if [[ -z ${KAFKA_HOME} ]];
then
    echo "Error: variable KAFKA_HOME is not set"
    exit 1
else
    echo "KAFKA_HOME is ${KAFKA_HOME}"
fi

if [ -n "$1" ] && [ "$1" = "stop" ]; then
        echo "Shutting down Zookeeper";
        pid=`ps ax | grep -i -w 'org.apache.zookeeper.server' | grep -v grep | awk '{print $1}'`
        if [ -n "$pid" ]
          then
          kill -9 $pid
        else
          echo "Zookeeper was not Running"
        fi
        echo "Shutting down Kafka";
        pid=`ps ax | grep -i -w 'kafka.Kafka' | grep -v grep | awk '{print $1}'`
        if [ -n "$pid" ]
          then
          kill -9 $pid
        else
          echo "Kafka was not Running"
        fi

        exit 0
fi

cd ${KAFKA_HOME}
echo "Starting Zookeeper"
pid=`ps ax | grep -i -w 'org.apache.zookeeper.server' | grep -v grep | awk '{print $1}'`
if [ -n "$pid" ]
then
  echo "Error: Zookeeper is allready running"
  exit 1
fi

nohup bin/zookeeper-server-start.sh config/zookeeper.properties >zookeeper.log 2>&1 &

echo "Starting Kafka"
pid=`ps ax | grep -i -w 'kafka.Kafka' | grep -v grep | awk '{print $1}'`
if [ -n "$pid" ]
  then
  echo "Error: Kafka is allready running"
  exit 1
fi

nohup bin/kafka-server-start.sh config/server.properties >kafka.log 2>&1 &

echo "Zookeper and Kafka startup initialized. Check ${KAFKA_HOME}/zookeeper.log and ${KAFKA_HOME}/kafka.log for sucessful startup"

# https://stackoverflow.com/questions/34512287/how-to-automatically-start-kafka-upon-system-startup-in-ubuntu


