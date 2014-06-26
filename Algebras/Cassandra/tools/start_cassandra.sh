#!/bin/bash
#
# This script starts cassandra
# on multiple nodes
#
# Jan Kristof Nidzwetzki
#
#######################################


# Cassandra Nodes
nodes="node1 node2 node3 node4 node5 node6"

# Cassandra binary
cassandradir="/opt/psec/nidzwetzki/cassandra/apache-cassandra-2.0.7"


for node in $nodes; do
   
   echo -n "Starting Cassandra on Node $node " 
   ssh $node "source .secondorc; $cassandradir/bin/cassandra > /dev/null" &
   echo "  [ Done ]"

done

echo -e "\n\n\n\n\n"

echo "Wait for cassandra nodes to become ready...."
sleep 5

while [ true ]; do
 
 $cassandradir/bin/nodetool ring

 ring=$($cassandradir/bin/nodetool ring)

 if [ $(echo $ring | grep Down | wc -l) -eq 0 ]; then
    break
 fi
 
 sleep 5;
done
