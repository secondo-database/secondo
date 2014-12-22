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
cassandrapath="/opt/psec/nidzwetzki/cassandra"
cassandradir=$cassandrapath/current
cassandraconfig=$cassandradir/conf/cassandra.yaml

# Cassandra version and download URL
cassandra_version="2.1.2"
cassandra_url="http://artfiles.org/apache.org/cassandra/${cassandra_version}/apache-cassandra-${cassandra_version}-bin.tar.gz"

##
# Configuration
##

# Number of nodes for one physical system
cassandra_vnodes=64

# Path for data
cassandra_data_dir=$cassandrapath/data

# Path for commit logs
cassandra_commitlog_dir=/mnt/diskb/psec2/nidzwetzki/cassandra/commitlog

# Cassandra seed master
cassandra_seed="132.176.69.181"

##
# Other variables
##
done=" \x1b[33;32m[ Done ]\x1b[39;49;00m"
failed=" \x1b[31;31m[ Failed ]\x1b[39;49;00m"

# Start cassandra
start() {

commitlogs=$(ls -l $cassandra_commitlog_dir | wc -l)

if [ $commitlogs -le 1 ]; then
   firststart=1
else
   firststart=0
fi

counter=0
for node in $nodes; do
   
   echo -n "Starting Cassandra on Node $node " 
   ssh $node "source .secondorc; $cassandradir/bin/cassandra > /dev/null" 
   res=$?

   if [ $counter -eq 0 ]; then
      echo -n "(first node) "
      sleep 10
   fi

   if [ $res -ne 0 ]; then
       echo -e $failed 
   else
     if [ $counter -ne 0 ]; then
       if [ $firststart -eq 1 ]; then
          # Wait for node movement to connect 
          while [ true ]; do
             if [ $($cassandradir/bin/nodetool ring | grep Joining | wc -l) -ne 0 ]; then
                break
             fi 
             echo -n "."
          done

          echo -n "-"
          
          # Wait for join
          while [ true ]; do
             if [ $($cassandradir/bin/nodetool ring | grep Joining | wc -l) -eq 0 ]; then
                break
             fi 
             echo -n "."
          done
       fi
     fi
 
     echo -e $done
   fi

   let counter=counter+1
done


echo "All cassandra nodes are ready...."
}

# Stop cassandra
stop() {
for node in $nodes; do

   echo -n "Killing Cassandra on node $node"
   ssh $node "ps ux | grep CassandraDaemon | grep -v grep | awk {'print \$2'} | xargs kill 2> /dev/null"
   echo -e $done 

done
}

# Delete the data and the commit log of cassandra
delete_old_data() {
    
    if [ -d $cassandra_data_dir ]; then
        rm -r $cassandra_data_dir
    fi

    if [ -d $cassandra_commitlog_dir ]; then
        rm -r $cassandra_commitlog_dir
    fi

    mkdir -p $cassandra_data_dir
    mkdir -p $cassandra_commitlog_dir
}

# Download and install cassandra
install_cassandra_local() {
    if [ ! -d $cassandrapath ]; then
       echo "$cassandrapath does not exist, creating"
       mkdir -p $cassandrapath
    fi

    delete_old_data

    cd $cassandrapath
    
    if [ ! -f apache-cassandra-${cassandra_version}-bin.tar.gz ]; then
        wget $cassandra_url
        
        if [ -f current ]; then
           rm current
        fi

        ln -s apache-cassandra-${cassandra_version} current
    else
        rm -r apache-cassandra-${cassandra_version}
    fi
       
    tar zxvf apache-cassandra-${cassandra_version}-bin.tar.gz > /dev/null
    
    ip=$(ifconfig | grep "inet addr" | cut -d ":" -f 2 | awk {'print $1'} | head -1)

    sed -i "s/num_tokens: .*/num_tokens: $cassandra_vnodes/" $cassandraconfig
    sed -i "s/# commitlog_directory/commitlog_directory/" $cassandraconfig
    sed -i "s|commitlog_directory: .*|commitlog_directory: $cassandra_commitlog_dir|" $cassandraconfig
    sed -i "s/# data_file_directories/data_file_directories/" $cassandraconfig
    sed -i "s|#     -.*|     - $cassandra_data_dir|" $cassandraconfig
    sed -i "s/listen_address:.*/listen_address: $ip/" $cassandraconfig
    sed -i "s/rpc_address:.*/rpc_address: $ip/" $cassandraconfig
   
#    if [ $cassandra_seed != $ip ]; then
        sed -i "s|seeds: \"127.0.0.1\"|seeds: \"127.0.0.1,$cassandra_seed\"|" $cassandraconfig
#    fi 
}

# Install cassandra on all nodes
install_cassandra() {
for node in $nodes; do
  
   script=$(readlink -f $0)
 
   echo -n "Install Cassandra on Node $node " 
   ssh $node "source .secondorc; $script install_local /dev/null" 
   echo -e $done
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
install)
   install_cassandra
   ;;
install_local)
   install_cassandra_local
   ;;
init)
   init_cassandra
   ;;
*)
   echo "Usage $0 {start|stop|init|install}"
   ;;
esac

exit 0

