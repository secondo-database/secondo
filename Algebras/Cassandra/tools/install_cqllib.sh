#!/bin/sh
#
# This script will download and build the 
# DataStax C++ Driver for Apache Cassandra
#
# It will create two .deb packages 
# 
# You can install them as follows: 
#
# dpkg -i cassandra0_0.5-1_amd64.deb 
# dpkg -i cassandra0-dev_0.5-1_amd64.deb
#
# Jan Kristof Nidzwetzki
#
#######################################

# Install build dependencies
apt-get install -y git cmake libboost-system-dev libboost-thread-dev libboost-date-time-dev libboost-program-options-dev libboost-test1.49-dev fakeroot dpkg-dev

# Make tmp dir
dir=$(mktemp -d)
cd $dir

# Download and build driver
git clone https://github.com/datastax/cpp-driver.git
cd cpp-driver

# Build the driver
# on RPM based distributions, you can build
# the driver with: 
# cmake . && make && make cql_demo && make cql_test && make test && make install

dpkg-buildpackage -rfakeroot

echo "Build ready"
