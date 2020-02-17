#!/bin/bash

set -e

cd
current_dir=`pwd`

if [[ -d "kafka" ]]
then
    echo "ERROR: Folder ${current_dir}/kafka exists on your filesystem. Installation can not be continued."
    exit 1
fi

mkdir kafka
cd kafka

wget http://apache.mirror.digionline.de/kafka/2.2.0/kafka_2.12-2.2.0.tgz
mkdir kafka_dist
tar -xvzf kafka_2.12-2.2.0.tgz --directory kafka_dist --strip 1

#wget http://apache.mirror.digionline.de/kafka/2.4.0/kafka_2.12-2.4.0.tgz
#mkdir kafka_dist
#tar -xvzf kafka_2.12-2.4.0.tgz --directory kafka_dist --strip 1

