#!/bin/bash

set -e

cd
current_dir=`pwd`

if [[ -d "kafka" ]]
then
    echo "Folder ${current_dir}/kafka exists on your filesystem. Installation can not be continued."
    exit 1
fi

mkdir kafka
cd kafka

# https://www.apache.org/dist/kafka/2.1.1/kafka_2.11-2.1.1.tgz
# https://www.apache.org/dist/kafka/2.4.0/kafka_2.12-2.4.0.tgz
# http://apache.mirror.digionline.de/kafka/2.4.0/kafka_2.12-2.4.0.tgz
wget http://apache.mirror.digionline.de/kafka/2.4.0/kafka_2.12-2.4.0.tgz

mkdir kafka_dist
tar -xvzf kafka_2.12-2.4.0.tgz --directory kafka_dist --strip 1

