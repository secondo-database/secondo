##
# Function library for DSECONDO
##

##
# variables
##
done=" \x1b[33;32m[ Done ]\x1b[39;49;00m"
failed=" \x1b[31;31m[ Failed ]\x1b[39;49;00m"

# Max pending tasks
max_pending=3

##
# functions
##

# Execute command parallel on multiple nodes
function execute_parallel() {
   command=$1
   task=$2
   nodes=$3
   max_pending=$4

   # Number of pending starts
   pending=0

   for node in $nodes; do
      echo "$task on Node $node "
      ssh $node "$command" &
      
      pending=$((pending + 1)) 

      if [ $pending -ge $max_pending ]; then
         echo -n "Waiting for pending commands to finish..."  
         wait
         pending=0
         echo -e " $done"
       fi
   done

   if [ $pending -gt 0 ]; then
      echo -n "Waiting for pending commands to finish..."  
      wait
      echo -e " $done"
   fi
}

# Get local IP
function getIp {
   echo $(/sbin/ifconfig | grep "inet addr" | grep -v "127.0.0.1" | cut -d ":" -f 2 | awk {'print $1'} | head -1)
}


