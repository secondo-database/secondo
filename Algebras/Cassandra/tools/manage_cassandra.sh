#!/bin/bash
#
# This script starts and stops cassandra
# on multiple nodes
#
# Jan Kristof Nidzwetzki
#
#######################################


# Cassandra Nodes
nodes="node1 node2 node3 node4 node5 node6"

# Cassandra dir
cassandradir="/opt/psec/nidzwetzki/cassandra/apache-cassandra-2.0.7"

# Start cassandra
start() {
for node in $nodes; do
   
   echo -n "Starting Cassandra on Node $node " 
   ssh $node "source .secondorc; $cassandradir/bin/cassandra > /dev/null" &
   echo "  [ Done ]"

done

echo -e "\n\n\n\n\n"

echo "Wait for cassandra nodes to become ready...."
sleep 10

# Wait for cassandra nodes
while [ true ]; do
 
 ring=$($cassandradir/bin/nodetool ring)
 $cassandradir/bin/nodetool ring 

 if [ $(echo $ring | grep Down | wc -l) -eq 0 ]; then
    break
 fi
 
 sleep 5;
done
}

# Stop cassandra
stop() {
for node in $nodes; do

   echo -n "Killing Cassandra on node $node"
   ssh $node "ps ux | grep CassandraDaemon | grep -v grep | awk {'print \$2'} | xargs kill 2> /dev/null"
   echo "  [ Done ]"

done
}


case "$1" in 

start)
   start
   ;;
stop)
   stop
   ;;
*)
   echo "Usage $0 {start|stop}"
   ;;
esac

exit 0

