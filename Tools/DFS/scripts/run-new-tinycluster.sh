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
    $root/index/indexnode -p4444 &

#wait a bit
    sleep 1

#create some local test data
    $cli localfile-create $fileroot/t100.txt 100 ten
    $cli localfile-create $fileroot/s_a.txt 100 single a
    $cli localfile-create $fileroot/t55.txt 55 ten

#register datanodes to index
    $cli datanode-register-easy $urii localhost 4445
    $cli datanode-register-easy $urii localhost 4446
    $cli datanode-register-easy $urii localhost 4447

#ready, do some simple stuff
    echo "TESTSETUP DONE"

    $cli file-store $urii $fileroot/t100.txt t100
    $cli file-store $urii $fileroot/s_a.txt s_a
    $cli file-store $urii $fileroot/t55.txt t55

    $cli file-load $urii t100 $fileroot/t100.saved
    $cli file-load $urii s_a $fileroot/s_a.saved
    $cli file-load $urii t55 $fileroot/t55.saved

    $cli file-delete-all $urii

#eof
    echo "DONE"
