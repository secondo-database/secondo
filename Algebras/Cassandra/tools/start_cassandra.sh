#!/bin/bash
#
# This script starts cassandra
# on multiple nodes
#
# Jan Kristof Nidzwetzki
#
#######################################

# Cassandra Nodes
nodes="node1 node2 node3 node4 node5"

# Cassandra binary
cassandrabin="/opt/cassandra/apache-cassandra-2.0.5/bin/cassandra"


for node in $nodes; do

   ssh root@$node killall -9 java
   sleep 1
   ssh root@$node $cassandrabin &

done
