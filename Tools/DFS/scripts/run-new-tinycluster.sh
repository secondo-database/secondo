#!/bin/bash

#enviroment and config

    #the root directory where all data and binaries will be copied for the test
    root=/home/kingster/masterarbeit/tinytest

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
    #urid4=dfs-data://storage.kingster.de:4445

#tidy up
    killall datanode
    killall indexnode
    rm -rf $root

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
    $root/index/indexnode -p4444 -r3 &

#wait a bit
    sleep 1

#register datanodes to index
    $cli datanode-register-easy $urii localhost 4445
    $cli datanode-register-easy $urii localhost 4446
    $cli datanode-register-easy $urii localhost 4447

#ready, do some simple stuff
    echo "TESTSETUP DONE"

    $cli localfile-create $fileroot/ten.txt 4999127 255
    $cli file-store $urii $fileroot/ten.txt ten

    $cli file-load $urii ten $fileroot/ten.txt.saved

    $cli quit-cluster $urii

#eof
    echo "DONE"
