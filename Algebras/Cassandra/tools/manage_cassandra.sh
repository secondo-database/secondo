#!/bin/bash
#
# This script starts and stops cassandra
# on multiple nodes. The script can also
# create the default keyspaces.
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
sleep 5

# Wait for cassandra nodes
while [ true ]; do
 
 if [ $($cassandradir/bin/nodetool ring | grep Down | wc -l) -eq 0 ]; then
    if [ $($cassandradir/bin/nodetool ring | grep Joining | wc -l) -eq 0 ]; then
       break
    fi
 fi
 
 $cassandradir/bin/nodetool ring 
 
 sleep 1;
done

echo "All cassandra nodes are ready...."
}

# Stop cassandra
stop() {
for node in $nodes; do

   echo -n "Killing Cassandra on node $node"
   ssh $node "ps ux | grep CassandraDaemon | grep -v grep | awk {'print \$2'} | xargs kill 2> /dev/null"
   echo "  [ Done ]"

done
}

# Init cassandra keyspaces
init_cassandra() {
	tmpfile=$(mktemp)
	ip=$(ifconfig | grep "inet addr" | cut -d ":" -f 2 | awk {'print $1'} | head -1)

cat << EOF > $tmpfile
CREATE KEYSPACE keyspace_r1 WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 1};
CREATE KEYSPACE keyspace_r2 WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 2};
CREATE KEYSPACE keyspace_r3 WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 3};
CREATE KEYSPACE keyspace_r4 WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 4};
CREATE KEYSPACE keyspace_r5 WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 5};
CREATE KEYSPACE keyspace_r6 WITH replication = {'class': 'SimpleStrategy', 'replication_factor' : 6};
EOF

	$cassandradir/bin/cqlsh $ip < $tmpfile

	rm $tmpfile
}

case "$1" in 

start)
   start
   ;;
stop)
   stop
   ;;
init)
   init_cassandra
   ;;
*)
   echo "Usage $0 {start|stop|init}"
   ;;
esac

exit 0

