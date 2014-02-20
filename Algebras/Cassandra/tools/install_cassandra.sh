#!/bin/sh
#
# This script will install apache 
# cassandra with all its dependencies. 
# In addition, a basic configuration 
# will be applied to cassandra.
#
# Jan Kristof Nidzwetzki
#
#######################################

cassandra_dir=/opt/cassandra
cassandra_version=2.0.3
cassandra_url=http://ftp-stud.hs-esslingen.de/pub/Mirrors/ftp.apache.org/dist/cassandra/$cassandra_version/apache-cassandra-$cassandra_version-bin.tar.gz
seed_ip=192.168.1.105

# Set language for output processing
LANG=C

# Is cassadra already installed?
if [ -d $cassandra_dir ]; then
   echo "Cassandra is already installed, exiting"
   exit -1
fi

# Install jdk, wget and other required software
apt-get install -y openjdk-7-jdk wget vim

# Install cassandra
mkdir -p $cassandra_dir
cd $cassandra_dir
wget $cassandra_url

rc=$?

if [[ $rc != 0 ]] ; then
    echo "Unable to download cassandra. exiting"
    exit -1
fi

tar zxvf apache-cassandra-$cassandra_version-bin.tar.gz
cd apache-cassandra-$cassandra_version

# Apply a basic configuration
mainip=$(ifconfig eth0 | grep "inet addr" | cut -d ":" -f 2 | cut -d " " -f 1)

if [ ! -f conf/cassandra.yaml ]; then
   echo "Unable to locate cassandra main configuration (cassandra.yaml)"
   exit -1
fi

sed -i "s/listen_address: .*/listen_address: $mainip/" conf/cassandra.yaml
sed -i "s/rpc_address: .*/rpc_address: $mainip/" conf/cassandra.yaml
sed -i "s/seeds: .*/seeds: \"127.0.0.1,$seed_ip\"/" conf/cassandra.yaml

echo "Cassandra is successfully installed..."
echo " "

