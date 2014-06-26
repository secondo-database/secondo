#!/bin/bash
#
# This script stops cassandra
# on multiple nodes
#
# Jan Kristof Nidzwetzki
#
#######################################

# Cassandra Nodes
nodes="node1 node2 node3 node4 node5 node6"

# Cassandra binary
cassandrabin="/opt/cassandra/apache-cassandra-2.0.7/bin/cassandra"


for node in $nodes; do

   ssh $node "ps ux | grep cassandra | grep -v grep | grep -v stop_cassandra | awk {'print \$2'} | xargs kill"

done
