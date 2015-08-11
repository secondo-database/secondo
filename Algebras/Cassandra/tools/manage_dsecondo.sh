#!/bin/bash
#
# This script is used to install, upgrade,
# start and stop the query processing
# nodes (QPNs) of a DSECONDO installation.
#
# Jan Kristof Nidzwetzki
#
#######################################

# Scriptname and Path
pushd `dirname $0` > /dev/null
scriptpath=`pwd`
scriptname=$(basename $0)
popd > /dev/null

# Include functions
. $scriptpath/functions.sh

# Set language to default
LANG=C

# port for secondo (the port range $port - $port+$instances-1 will be used)
port=12234

# secondo worker instances (2 cores for the kernel and cassandra 
# and the remaning cores for DSECONDO)
instances=$(($(cat /proc/cpuinfo | grep processor | wc -l) - 3))

# Keyspace to use
keyspace="keyspace_r3"

# QPN Nodes
nodes="${DSECONDO_QPN}"

if [ -z "$nodes" ]; then
   echo "Your environment variable \$(DSECONDO_QPN) is empty. Please check your .secondorc"
   exit -1
fi

# Overwrite nodes by cli argument
if [ $# -eq 2 ]; then
   nodes=$2
fi

if [ -z "$DSECONDO_QPN_DIR" ]; then
   echo "Your environment variable \${DSECONDO_QPN_DIR} is empty. Please check your .secondorc"
   exit -1
fi

if [ -z "$DSECONDO_DIR" ]; then
   echo "Your environment variable \${DSECONDO_DIR} is empty. Please check your .secondorc"
   exit -1
fi

# Variables
screensessionServer="dsecondo-server"
screensessionExecutor="dsecondo-executor"

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
      exit -1
   fi

   if [ $(grep CassandraHost $SECONDO_CONFIG | wc -l ) -ne 1 ]; then
      echo "CassandraHost is not set in your SECONDO config, exiting" 1>&2
      exit -1
   fi 
   
   # Check if screen is already running
   sInstances=$(screen -ls | egrep "($screensessionServer|$screensessionExecutor)" | wc -l)
  
   if [ $sInstances -ne 0 ]; then
       echo "[Error] Screen is already running... " 1>&2
       exit -1
   fi
 
   # local ip
   ip=$(getIp)
  
   # username
   username=$(whoami)
 
   ports="" 
   instance=0
   while [ $instance -le $((instances-1)) ]; do

     instance_port=$((port+instance))
     ports+=":${instance_port}"

     # Change SECONDO Server Port in configuration
     configuration=/tmp/SecondoConfig_${username}_${instance}.ini
     cp ${SECONDO_CONFIG} $configuration
     sed -i "s/^SecondoPort=.*/SecondoPort=$instance_port/" $configuration
     sed -i "s/^SecondoHome=\(.*\)/SecondoHome=\1_$instance/" $configuration
     sed -i "s/^CassandraHost=.*/CassandraHost=$ip/" $configuration
     
     secondoHome=$(grep ^SecondoHome $configuration | cut -d "=" -f 2) 
     mkdir -p $secondoHome

     # Start screen in deamon mode 
     screenId=${screensessionServer}_${instance}
     screen -dmS $screenId
     execCommandsInScreen $screenId "cd $SECONDO_BUILD_DIR/bin; ./SecondoMonitor -s -c ${configuration}"
     
     instance=$((instance+1))
   done
     
   sleep 5

   # remove trailing ':' in ports list
   ports=$(echo $ports | sed s/^://g)
   
   screen -dmS $screensessionExecutor
   localIp=$(getIp "")
   #execCommandsInScreen $screensessionExecutor "cd $SECONDO_BUILD_DIR/Algebras/Cassandra/tools/queryexecutor/" "./Queryexecutor -i $localIp -k $keyspace -s 127.0.0.1 -p $ports | tee /tmp/qe.log"
   execCommandsInScreen $screensessionExecutor "cd $SECONDO_BUILD_DIR/Algebras/Cassandra/tools/queryexecutor/" "while [ true ]; do ./Queryexecutor -i $localIp -k $keyspace -s 127.0.0.1 -p $ports; sleep 5; done"
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
   #execute_parallel "bash -x $scriptpath/$scriptname start_local > /dev/null" "Starting DSECONDO" "$nodes" $max_pending
   execute_parallel "source .secondorc; export SECONDO_BUILD_DIR=$DSECONDO_QPN_DIR/secondo; export SECONDO_CONFIG=$SECONDO_BUILD_DIR/bin/SecondoConfig.ini; \$SECONDO_BUILD_DIR/Algebras/Cassandra/tools/$scriptname start_local > /dev/null" "Starting DSECONDO" "$nodes" $max_pending
}

# Stop all desecondo instances
stop() {
   #execute_parallel "bash -x $scriptpath/$scriptname stop_local > /dev/null" "Stopping DSECONDO" "$nodes" $max_pending
   execute_parallel "source .secondorc; export SECONDO_BUILD_DIR=$DSECONDO_QPN_DIR/secondo; export SECONDO_CONFIG=$SECONDO_BUILD_DIR/bin/SecondoConfig.ini; \$SECONDO_BUILD_DIR/Algebras/Cassandra/tools/$scriptname stop_local > /dev/null" "Stopping DSECONDO" "$nodes" $max_pending
}

# Install SECONDO on all QPNs
install() {
   
   execute_parallel "mkdir -p $DSECONDO_QPN_DIR" "Creating DSECONDO directory" "$nodes" $max_pending
   
   for node in $nodes; do
      echo "Copy .secondorc to Node $node"
      scp ~/.secondorc ${node}:
   done
   
   execute_parallel "if [ $(cat ~/.bashrc | grep .secondorc | wc -l) -eq 0 ]; then echo 'source ~/.secondorc' >> ~/.bashrc; fi" "Adding SECONDO environment variables" "$nodes" $max_pending
   
   # Upgrade DSECONDO Build on all QPNs
   upgrade
}

# Upgrade the SECONDO installation and all drivers on all QPNs
upgrade() {
   for node in $nodes; do
      echo "Upgrading drivers on Node $node"
      rsync --progress -l -H -p -o -g -D -t -r  -e "ssh" ${DSECONDO_DIR}/driver/ $node:${DSECONDO_QPN_DIR}/driver > /dev/null
      echo "Upgrading QPN-SECONDO on Node $node"
      rsync --progress -l -H -p -o -g -D -t -r --files-from=${DSECONDO_DIR}/secondo/Algebras/Cassandra/tools/qpnfiles -e "ssh" ${DSECONDO_DIR}/secondo/ $node:${DSECONDO_QPN_DIR}/secondo > /dev/null
   done
}

# Download and install the CPP driver and all dependencies
install_driver() {
   
   # Install LIBUV
   if [ -d ${DSECONDO_DIR}/driver/libuv ]; then
      echo "LibUV is already installed";
   else
      mkdir -p ${DSECONDO_DIR}/driver
      cd ${DSECONDO_DIR}/driver
      git clone https://github.com/libuv/libuv.git 
      cd libuv
      sh autogen.sh
      ./configure
      make
      cd ..
   fi
   
   # Install the CPP Driver
   if [ -d ${DSECONDO_DIR}/driver/cpp-driver ]; then
      echo "cpp-driver is already installed";
   else
      mkdir -p ${DSECONDO_DIR}/driver
      cd ${DSECONDO_DIR}/driver
      git clone https://github.com/datastax/cpp-driver.git 
      cd cpp-driver
      wget https://raw.githubusercontent.com/jnidzwetzki/misc/master/cpp-driver/0001-Added-single-node-loadbalancing.patch 
      git am < 0001-Added-single-node-loadbalancing.patch  
      export LIBUV_ROOT_DIR=${DSECONDO_DIR}/driver/libuv
      cmake .
      make
      cd ..
   fi
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
install_driver)
   install_driver
   ;;
install)
   install
   ;;
upgrade)
   upgrade
   ;;
*)
   echo "Usage $0 {start | stop | install | upgrade | install_driver}"
   ;;
esac

exit 0

