#!/bin/bash

#enviroment and config

    action="$1"

    #the root directory where all data and binaries will be copied for the test
    root=/home/kingster/masterarbeit/cluster

    #binary source, where dfs is compiled
    bs=/home/kingster/masterarbeit/code/dfs

    #path to command line interface
    cli=$root/cli

    #location for local test files
    fileroot=$root/files

#data nodes
    urii=dfs-index://localhost:4444
    urid1=dfs-data://localhost:4445
    urid2=dfs-data://localhost:4446
    urid3=dfs-data://localhost:4447

#tidy up
    killall datanode
    killall indexnode
    if [ "$action" == "clean" ]; then
        echo cleaning...
        rm -rf $root
    fi

#creating directories
    mkdir -p $root/index
    mkdir -p $fileroot
    for d in {1..3}
    do
    mkdir -p $root/datanode$d/data
    done

#copy binaries stuff
    cp $bs/indexnode/indexnode $root/index
    cp $bs/cli/cli $root
    for d in {1..3}
    do
    cp $bs/datanode/datanode $root/datanode$d
    done

#start data nodes
    $root/datanode1/datanode -p4445 -pw44450 &
    $root/datanode2/datanode -p4446 -pw44460 &
    $root/datanode2/datanode -p4447 -pw44470 &

#start index node
    $root/index/indexnode -p4444 &

#wait a bit
    sleep 1

#register datanodes to index
    $cli datanode-register-easy $urii localhost 4445
    $cli datanode-register-easy $urii localhost 4446
    $cli datanode-register-easy $urii localhost 4447

#a simple hello world
    if [ "$action" == "helloworld" ]; then
        $cli file-store-buffer $urii helloworld HelloWorld!HowAreYou
    fi