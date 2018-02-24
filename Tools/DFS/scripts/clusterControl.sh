#!/bin/bash
#<nodetyp> <sshhost> <sshuser> <binpath> [port] [datapath] [webport]

echo DFS remote control
echo Stephan Scheide
echo
echo "clusterControl <action> <file> [Options]"
echo
echo "<action> is start"
echo "<file> is the configuration file for this script"

filename="$2"
tmpfile="/tmp/clusterControl.tmp"

function executeForRemote {
    user="$1@$2"
    ssh $user
}

function executeBackgroundProcessOnRemoteHost {
    user="$1@$2"
    cmd="$3"
    ssh -f $user "bash -c 'nohup $cmd 2>&1 &'"
}

function stopNode {
    if [ "$1" == "index" ] || [ "$1" == "data" ]; then
        host="$2"
        user="$3"
        appname="$1node"
        cmd="killall $appname"
        executeBackgroundProcessOnRemoteHost $user $host "$cmd"
    fi
}

function startNode {
    if [ "$1" == "index" ] || [ "$1" == "data" ]; then
        host="$2"
        user="$3"
        binpath="$4"
        port="$5"
        datapath="$6"
        webport="$7"

        params=""
        if [ -n "$port" ]; then
            params="$params -p$port"
        fi

        if [ -n "$webport" ]; then
            params="$params -pw$webport"
        fi

        if [ -n "$datapath" ]; then
            params="$params -d$datapath"
        fi

        cmd="$binpath $params"
        echo "starting $1 node using this command <$cmd> remotely at $3@$2"
        executeBackgroundProcessOnRemoteHost $user $host "$cmd"
    fi
}

action=$1
echo $action
while read line
do
    if [ "$action" == "start" ]; then
	echo "starting node $line"
    	startNode $line
    fi

    if [ "$action" == "stop" ]; then
    	stopNode $line
    fi

done <"$filename"
