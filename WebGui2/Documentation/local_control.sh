#!/bin/bash

#####################################################################################
# This loop launches a module in the to be run as loop in the background
#####################################################################################

#define the unique regexp to identify your module in ps
DIR=`pwd`
WORKER=`basename $DIR`
PROGGREP="$DIR/SecondoMonitor"

#set a display name for your module
PROGNAME=SecondoMonitor

#This scripts name
CTRLNAME=$DIR/${0##*/}

#List of people to mail when restarted ..
#Actual email address will be inserted by configpushps, from vars.sh.
MAILTO=russk.irina@gmail.com

# The configuration should be done in vars.sh
CORE_DUMP_ANALYSIS=false

#runs the program
runprog() {
    /home/russkaya/secondo/bin/SecondoMonitor -s
}

send_email() {
    if [ "$MAILTO" != "" ] ; then
        hn=`hostname`;
        printf "$PROGNAME restarted on $hn : [ $1 ]  @ `date` \n Control name: $CTRLNAME \n" | mail -s "$PROGNAME restarted on $hn" $MAILTO
		printf "$PROGNAME restarted on $hn : [ $1 ]  @ `date` \n Control name: $CTRLNAME \n" > local_control.log
    fi    
}

#kills the control loop
killcontrol() {
    pid=`ps -ef | grep -e "$CTRLNAME start_loop" | grep -v -e "grep" -e " $$ " | awk  '{ print $2}'`
    if [ "$pid" != '' ]; then
	k=`kill -9 ${pid}`
    fi
}

#kills the program
killprog() {
    pid=`ps -ef | grep -e "$PROGGREP" | grep -v -e "grep" | awk  '{ print $2}'`
    if [ "$pid" != '' ]; then
	k=`kill ${pid}`
    fi
	
	sleep 3s
    pid=`ps -ef | grep -e "$PROGGREP" | grep -v -e "grep" | awk  '{ print $2}'`
    if [ "$pid" != '' ]; then
	k=`kill -9 ${pid}`
    fi	
}

printstatus() {
    ps -ef | grep -e "$PROGGREP" -e "$CTRLNAME start_loop" | grep -v -e "grep" -e " $$ " 
}

is_prog_running() {
    pid=`ps -ef | grep -e "$PROGGREP" | grep -v -e "grep" | awk  '{ print $2}'`
    if [ "$pid" != '' ]; then
        return 0
    fi    
    return 1	
}

is_control_running() {
    pid=`ps -ef | grep -e "$CTRLNAME start_loop" | grep -v -e "grep" -e " $$ " | awk  '{ print $2}'`
    if [ "$pid" != '' ]; then
        return 0 # return the opposite
    fi
    return 1
}

# Enable core dumps
enable_core_dump() {
	ulimit -c unlimited
}

extract_back_trace(){
	
    hn=`hostname`;
	printf  "$PROGNAME restarted on $hn : @ `date` \n Control name: $CTRLNAME \n" > backtrace.txt 
	printf  "Backtrace from core dump: \n\n" >> backtrace.txt 

	gdb ${PROGNAME} core --batch --quiet -ex "thread apply all bt full" -ex "quit" >> backtrace.txt 
    rm core
    if [ "$MAILTO" != "" ] ; then
    	cat backtrace.txt | mail -s "$PROGNAME on $hn crash backtrace" $MAILTO
    fi    
}

usage() {
    echo "------------------------------------------------------"
    echo "Usage: $CTRLNAME [launch|start|restart|stop|status] "
    echo "  ----> launch : deprecated, use start instead"
    echo "  ----> start  : starts the loop in the background"
    echo "  ----> restart: restarts the $PROGNAME"
    echo "  ----> stop   : stops both the $PROGNAME & the loop"
    echo "  ----> status : prints the status of the loop & $PROGNAME"
    echo "------------------------------------------------------"
}

if [ "$1" == "launch" ] || [ "$1" == "start" ] ; then
 #start this loop in the background ..
 killcontrol
 nohup $CTRLNAME start_loop > /dev/null 2>&1 &

elif [ "$1" == "start_loop" ] ; then
    i="0" 

	if [ -n "${CORE_DUMP_ANALYSIS}" ] && "${CORE_DUMP_ANALYSIS}" 
	then 
		enable_core_dump
	fi

    while [ 1 ]
    do
      #Allow only 100 restarts
      if ! (is_prog_running) then
          if  [ $i -lt 100 ]; then
              echo "Starting $PROGNAME : $i @ `date`" >> lc.out
              runprog $i
              if [ "$i" != "0" ]; then
                  	send_email $i

					if [ -n "${CORE_DUMP_ANALYSIS}" ] && "${CORE_DUMP_ANALYSIS}" 
					then 
						extract_back_trace
					fi
              fi
              i=$[$i+1]
              sleep 20s
          else
              sleep 30s
          fi
      else
          sleep 15s
      fi      
    done

elif [ "$1" == "stop" ] ; then        
    killcontrol
    killprog
elif [ "$1" == "restart" ] ; then 
    killprog
    if ! (is_control_running) ; then
        nohup $CTRLNAME start_loop > /dev/null 2>&1 &
    fi
elif [ "$1" == "" ] || [ "$1" == "status" ] ; then
    printstatus
else
    usage
fi
