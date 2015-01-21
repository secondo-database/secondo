#!/bin/bash
#
# This script starts and stops a 
# local DSECONDO installation. This Script
# can also manage all DSECONDO worker nodes.
#
# Jan Kristof Nidzwetzki
#
#######################################

# Set language to default
LANG=C

# port for secondo (the port range $port - $port+$instances-1 will be used)
port=12234

# secondo worker instances (2 cores for the kernel and cassandra 
# and the remaning cores for DSECONDO)
instances=$(($(cat /proc/cpuinfo | grep processor | wc -l) - 2))

# Keyspace to use
keyspace="keyspace_r3"

# Cassandra Nodes
#nodes="node1 node2 node4 node5 node6"
nodes="node1 node2 node6"

# Variables
screensessionServer="dsecondo-server"
screensessionExecutor="dsecondo-executor"

# Scriptname and Path
pushd `dirname $0` > /dev/null
scriptpath=`pwd`
scriptname=$(basename $0)
popd > /dev/null

# Get local IP
function getIp {
   /sbin/ifconfig | grep "inet addr" | grep -v "127.0.0.1" | cut -d ":" -f 2 | awk {'print $1'} | head -1
}

# Get ID for Screen
function getScreenId {
   ids=$(screen -ls | grep $1 | cut -f1 -d'.' | tr -d '\t')
   
   if [ $(screen -ls | grep $1 | cut -f1 -d'.' | wc -l) -eq 1 ]; then
     echo -n $ids
   else   
     echo -n "-1"
   fi
}

# Exec a command in screen session
function execCommandsInScreen {
  session=$1
  shift 
  
  screenid=$(getScreenId "$session")
  
  while [ "$1" != "" ]; do 
    COMMAND=$1
    
    # Line borrowed from psecondo tools
    screen -S $screenid -p 0 -X stuff "${COMMAND}$(printf \\r)"
    shift
  done
}

# Start local dsecondo instance
start_local() {
   echo "Starting DSECONDO instance"

   if [ ! -f $SECONDO_CONFIG ]; then
      echo "Unable to locate SECONDO config" 1>&2
      exit -1;
   fi 
   
   # Check if screen is already running
   sInstances=$(screen -ls | egrep "($screensessionServer|$screensessionExecutor)" | wc -l)
  
   if [ $sInstances -ne 0 ]; then
       echo "[Error] Screen is already running... " 1>&2
       exit -1
   fi
 
   ports="" 
   instance=0
   while [ $instance -le $((instances-1)) ]; do

     instance_port=$((port+instance))
     ports+=":${instance_port}"

     # Change SECONDO Server Port in configuration
     configuration=${SECONDO_CONFIG}_${instance}
     cp ${SECONDO_CONFIG} $configuration
     sed -i "s/^SecondoPort=.*/SecondoPort=$instance_port/" $configuration
     sed -i "s/^SecondoHome=\(.*\)/SecondoHome=\1_$instance/" $configuration
 #    sed -i "s/^RTFlags +=.*Server:BinaryTransfer/#RTFlags +=.*Server:BinaryTransfer/" $configuration
     secondoHome=$(grep ^SecondoHome $configuration | cut -d "=" -f 2) 
     mkdir -p $secondoHome

     # Start screen in deamon mode 
     screenId=${screensessionServer}_${instance}
     screen -dmS $screenId
     execCommandsInScreen $screenId "cd $SECONDO_BUILD_DIR/bin" "export SECONDO_CONFIG=${configuration}" "./SecondoMonitor -s "
     
     instance=$((instance+1))
   done
     
   sleep 2

   # remove trailing ':' in ports list
   ports=$(echo $ports | sed s/^://g)
   
   screen -dmS $screensessionExecutor
   localIp=$(getIp "")
   #execCommandsInScreen $screensessionExecutor "cd $SECONDO_BUILD_DIR/Algebras/Cassandra/tools/queryexecutor/" "./Queryexecutor -i $localIp -k $keyspace -s 127.0.0.1 -p $ports | tee /tmp/qe.log"
   execCommandsInScreen $screensessionExecutor "cd $SECONDO_BUILD_DIR/Algebras/Cassandra/tools/queryexecutor/" "./Queryexecutor -i $localIp -k $keyspace -s 127.0.0.1 -p $ports"
}

# Stop local dsecondo instance
stop_local() {
   echo "Stopping DSECONDO instance"

   # Kill the screen executor session
   screenid=$(getScreenId "$screensessionExecutor")
   
   if [ "$screenid" != "-1" ]; then
      echo $screenid
      kill $screenid 
   fi
   
   # Stop secondo instances
   instance=0
   while [ $instance -le $((instances-1)) ]; do

      screenId=${screensessionServer}_${instance}
     
      # Line borrowed from psecondo tools
      screenpid=$(getScreenId "$screenId")
      if [ "$screenpid" != "-1" ]; then
         execCommandsInScreen $screenpid "quit" "y" "exit"
      fi
      instance=$((instance+1))
   done
    
   sleep 5
     
   # kill remaining secondo instances 
   instance=0
   while [ $instance -le $((instances-1)) ]; do

      screenId=${screensessionServer}_${instance}
    
      # Kill the screen executor session
      screenpid=$(getScreenId "$screenId")
   
      if [ "$screenpid" != "-1" ]; then
         echo $screenpid
         kill $screenpid 
      fi
     
      instance=$((instance+1))
   done
}

# Start all descondo instances
start() {

   for node in $nodes; do
      echo -n "Starting DSECONDO on Node $node " 
         ssh $node "source .secondorc; $scriptpath/$scriptname start_local > /dev/null"
         #ssh $node "source .secondorc; bash -x $scriptpath/$scriptname start_local > /dev/null"
      echo "  [ Done ]"
   done
}

# Stop all desecondo instances
stop() {
   for node in $nodes; do
   
      echo -n "Stopping DSECONDO on Node $node " 
         ssh $node "source .secondorc; $scriptpath/$scriptname stop_local > /dev/null"
         #ssh $node "source .secondorc; bash -x $scriptpath/$scriptname stop_local > /dev/null"
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
start_local)
   start_local
   ;;
stop_local)
   stop_local
   ;;
*)
   echo "Usage $0 {start|stop}"
   ;;
esac

exit 0

