#!/bin/bash

#some tests can be switched off
test_do_mediumfile=0
test_do_massdelete=0

#delete running processes
    killall datanode
    killall indexnode

#directories for running
    root=/home/kingster/masterarbeit/systemtest
    bs=/home/kingster/masterarbeit/code/dfs
    cli=$root/cli
    fileroot=$root/files
    measureroot=$root/measure


#define uris
    urii=dfs-index://localhost:4444
    urid1=dfs-data://localhost:4445
    urid2=dfs-data://localhost:4446

#config
    chunksize=200
    replica=2

#tidy up
    rm -rf $root

#creating directories
    mkdir -p $root/index
    mkdir -p $fileroot
    mkdir -p $measureroot
    for d in {1..2}
    do
    mkdir -p $root/datanode$d/data
    done

#copy binaries stuff
    cp $bs/indexnode/indexnode $root/index
    cp $bs/cli/cli $root
    for d in {1..2}
    do
    cp $bs/datanode/datanode $root/datanode$d
    done

#start data nodes
    $root/datanode1/datanode -p4445 -pw44450 -X &
    $root/datanode2/datanode -p4446 -pw44460 -X &

#start index node
    $root/index/indexnode -p4444 -chunksize$chunksize -r$replica -X &

#wait for servers to be ready
    sleep 1

#register data nodes
    $cli datanode-register-easy $urii localhost 4445
    $cli datanode-register-easy $urii localhost 4446

#create test data
    $cli localfile-create $fileroot/t.txt 1000 ten
    $cli localfile-create $fileroot/single.txt 200 ten
    $cli localfile-create $fileroot/almost.txt 199 ten
    $cli localfile-create $fileroot/bitmore.txt 201 ten

#define some asserts and helper functions
    function assertEquals {
        if [ "$1" == "$2" ]; then
            echo "SUC - $3 - are equals $1"
        else
            echo "ERR - $3 - expected $1 but was $2"
            exit 1
        fi
    }

    function assertDataNodeHash {
        r=$($cli datanode-hash-print $1 | tail -n 1)
        $cli datanode-hash-print $1
        assertEquals $2 $r "data-node-fingerprint"
    }

    function assertCount {
        r=$($cli file-count $urii | tail -n 1)
        if [ "$r" == "$1" ]; then
            echo "SUC - got expected file count $1 from dfs"
        else
            echo "ERR - expected count was $1 but was $r"
            exit 1
        fi
    }

    function assertDataNodeCount {
        r=$($cli datanode-list $urii | tail -n 1)
        if [ "$r" == "$1" ]; then
            echo "SUC - got expected datanode count $1 from dfs"
        else
            echo "ERR - expected datanode count was $1 but was $r"
            exit 1
        fi
    }

    function assertHasFile {
        r=$($cli file-exists $urii $1 | tail -n 1)
        if [ "$r" == "1" ]; then
            echo "SUC - file $1 exists on dfs"
        else
            echo "ERR - file $1 does not exist on dfs"
            exit 1
        fi
    }

    function assertHasNoFile {
        r=$($cli file-exists $urii $1 | tail -n 1)
        if [ "$r" == "0" ]; then
            echo "SUC - file $1 does not exist on dfs"
        else
            echo "ERR - file $1 exists on dfs"
            exit 1
        fi
    }

    function checksum {
        r=($(sha1sum $1))
        echo $r
    }

    function assertChecksums {
        actual=$(checksum "$2")
        echo "got checksum of $2"
        if [ "$1" == "$actual" ]; then
            echo "SUC - checksum same $3"
        else
            echo "ERR - $3 - checksum not equals $1 - $actual"
            exit 1
        fi
    }

    function assertLocalFilesEqual {
        cs0=$(checksum "$1")
        cs1=$(checksum "$2")
        assertEquals $cs0 $cs1 "checksums of $1 and $2"
    }

    function assertTotalSizeRemote {
        totalsizeRemote=$($cli file-totalsize $urii | tail -n 1)
        assertEquals $1 $totalsizeRemote "totalsize of all remote files"
    }

#test for upload,count,receive,delete - also mentioned in document
    echo
    echo TEST in document

    $cli file-delete-all $urii
    
	$cli localfile-create $fileroot/a.txt 298 alphabet

    $cli file-store $urii $fileroot/a.txt a
    assertDataNodeHash $urid1 398
    assertDataNodeHash $urid2 398
    assertCount 1
    assertTotalSizeRemote 298
    assertHasFile a

    $cli file-store $urii $fileroot/a.txt b
    assertCount 2
    $cli file-delete $urii b

    assertCount 1
    assertHasFile a
    assertHasNoFile b
    
    $cli file-append-buffer $urii a 123
    assertDataNodeHash $urid1 404
    assertDataNodeHash $urid2 404

    $cli file-load-part-store $urii a 0 2 $fileroot/part1
    assertChecksums "da23614e02469a0d7c7bd1bdab5c9c474b1904dc" $fileroot/part1

    $cli file-load-part-store $urii a 2 2 $fileroot/part15
    assertChecksums "034778198a045c1ed80be271cdd029b76874f6fc" $fileroot/part15

    $cli file-load-part-store $urii a 200 1 $fileroot/part2
    assertChecksums "a0f1490a20d0211c997b44bc357e1972deab8ae3" $fileroot/part2
    
    $cli file-load-part-store $urii a 0 201 $fileroot/part3
    assertChecksums "abd1568ccebe91f75d3f46082b88e55b68c0f8bf" $fileroot/part3

    $cli file-load-part-store $urii a 26 1 $fileroot/part4
    assertChecksums "86f7e437faa5a7fce15d1ddcb9eaeaea377667b8" $fileroot/part4


#simple tests (upload, count, delete) - asserts affect only index node
    echo
    echo TEST simple
    $cli file-delete-all $urii
    $cli file-store $urii $fileroot/t.txt t
    assertHasFile t
    assertHasNoFile s
    assertCount 1
    $cli file-store $urii $fileroot/single.txt s
    assertCount 2
    assertHasFile s
    $cli file-delete $urii none
    assertCount 2
    $cli file-delete $urii t
    assertCount 1
    $cli file-store $urii $fileroot/t.txt t
    assertCount 2
    $cli file-delete $urii t
    assertCount 1
    $cli file-delete $urii t
    assertCount 1
    $cli file-delete $urii s
    assertCount 0
    $cli file-store $urii $fileroot/t.txt t
    assertCount 1
    $cli file-store $urii $fileroot/t.txt t
    assertCount 1

#save some checksums
    cs0=$(checksum $fileroot/t.txt)
    cs1=$(checksum $fileroot/single.txt)
    echo "checksums of testfiles $cs0 $cs1"

#selfcheck
    assertChecksums $cs0 $fileroot/t.txt Selbtest

#simple simple.save-load-checksum tests
    echo
    echo TEST store.load.checksum
    $cli file-store $urii $fileroot/t.txt t0
    $cli file-store $urii $fileroot/t.txt t1
    $cli file-store $urii $fileroot/single.txt s

    $cli file-load $urii t0 $fileroot/t0.saved.txt
    $cli file-load $urii t1 $fileroot/t1.saved.txt
    assertChecksums $cs0 $fileroot/t0.saved.txt "t.txt erste Kopie"
    assertChecksums $cs0 $fileroot/t1.saved.txt "t.txt zweite Kopie"
    $cli file-store $urii $fileroot/t.txt t0
    assertChecksums $cs0 $fileroot/t0.saved.txt "t.txt Ueberschreiben mit sich selbst"
    $cli file-store $urii $fileroot/single.txt t0
    $cli file-load $urii t0 $fileroot/t0.saved.txt
    assertChecksums $cs1 $fileroot/t0.saved.txt "t.txt Ueberschreiben mit anderer"

#test: simple.delete.all
    assertCount 4
    $cli file-delete-all $urii
    assertCount 0

#test: many small files
    echo
    echo TEST many files
    totalsize=0
    filecount=10
    for i in {1..10}
    do
        echo "handle file $i of $filecount"
        m=$(($i%4))
        size=$chunksize
        if [ $m == 1 ]; then
            size=$(($chunksize*10))
        elif [ $m == 2 ]; then
            size=$(($chunksize-1))
        elif [ $m == 3 ]; then
            size=$(($chunksize*4+11))
        fi
        totalsize=$((totalsize+$size))

        echo $size

        file=$fileroot/file$i.txt
        fileid=file$i
        $cli localfile-create $file $size ten
        $cli file-store $urii $file $fileid
    done
    assertCount $filecount
    totalsizeRemote=$($cli file-totalsize $urii | tail -n 1)
    assertEquals $totalsize $totalsizeRemote "total file size check"

#test: some files with category
    echo
    echo TEST categories
    #$cli file-delete-all $urii
    $cli file-store $urii $fileroot/single.txt d0 cat0
    $cli file-store $urii $fileroot/single.txt d1 cat0
    $cli file-store $urii $fileroot/single.txt d2 cat1
    $cli file-store $urii $fileroot/single.txt d3 cat1
    $cli file-store $urii $fileroot/single.txt d3 cat1
    $cli file-store $urii $fileroot/single.txt d4 cat1
    assertCount 15
    $cli file-delete-all-category $urii catnone
    assertCount 15
    $cli file-delete-all-category $urii cat0
    assertCount 13


#test: medium file
    if [ "$test_do_mediumfile" -eq "1" ]
    then
        echo
        echo TEST medium file with changed chunksize
        $cli file-delete-all $urii
        newChunkSize=$((32*1024*1024))
        $cli change-setting $urii chunksize $newChunkSize
        $cli localfile-create $fileroot/bigger.dat $(($newChunkSize*3)) ten
        $cli file-store $urii $fileroot/bigger.dat big1
        totalsizeRemote=$($cli file-totalsize $urii | tail -n 1)
        assertEquals 100663296 $totalsizeRemote
        $cli change-setting $urii chunksize 200
        assertCount 1
    fi

#test: restart indexnode (state must be restored)
    #killall indexnode
    #$root/index/indexnode -p4444 -chunksize$chunksize &
    #sleep 1
    #assertCount 13

#test: delete all test
    echo
    echo TEST delete.all
    $cli file-delete-all $urii
    assertCount 0
    assertTotalSizeRemote 0
    assertDataNodeHash $urid1 0
    assertDataNodeHash $urid2 0

    file=$fileroot/a.txt
    $cli localfile-create $file 201 ten
    $cli file-store $urii $file a
    assertCount 1
    assertDataNodeHash $urid1 204
    assertDataNodeHash $urid2 204

    file=$fileroot/a.txt
    $cli localfile-create $file 100 ten
    $cli file-store $urii $file b
    assertCount 2
    assertDataNodeHash $urid1 505
    assertDataNodeHash $urid2 505

    $cli file-delete-all $urii
    assertCount 0
    assertTotalSizeRemote 0
    assertDataNodeHash $urid1 0
    assertDataNodeHash $urid2 0

#test: delete single file test
    echo
    echo TEST delete.single AND datanode.hash
    $cli file-delete-all $urii
    assertCount 0
    assertTotalSizeRemote 0
    assertDataNodeHash $urid1 0
    assertDataNodeHash $urid2 0

    $cli localfile-create $file 199 ten
    $cli file-store $urii $file a
    $cli localfile-create $file 200 ten
    $cli file-store $urii $file b
    $cli localfile-create $file 401 ten
    $cli file-store $urii $file c
    assertCount 3
    assertDataNodeHash $urid1 2009
    assertDataNodeHash $urid2 2009

    $cli file-delete $urii a
    assertCount 2
    assertDataNodeHash $urid1 1208
    assertDataNodeHash $urid2 1208

    $cli file-delete $urii a
    assertCount 2
    assertDataNodeHash $urid1 1208
    assertDataNodeHash $urid2 1208

    $cli file-delete $urii c
    assertCount 1
    assertDataNodeHash $urid1 201
    assertDataNodeHash $urid2 201

    downloaded=$fileroot/b.saved.txt
    $cli localfile-create $file 200 ten
    $cli file-load $urii b $downloaded

    assertLocalFilesEqual $file $downloaded

    $cli file-delete $urii b
    assertCount 0
    assertTotalSizeRemote 0
    assertDataNodeHash $urid1 0
    assertDataNodeHash $urid2 0

    $cli file-store $urii $file b
    assertCount 1
    assertDataNodeHash $urid1 201
    assertDataNodeHash $urid2 201

#test: mass add and delete test

    if [ "$test_do_massdelete" -eq "1" ]
    then

        echo TEST advanced.many delete and add and check data nodes
        $cli file-delete-all $urii
        for i in {1..1000}
        do
            file=$fileroot/a.txt
            $cli localfile-create $file 401 ten
            name="datei$i"
            echo $name
            $cli file-store $urii $file $name
        done
        assertCount 1000
        for i in {200..499}
        do
            name="datei$i"
            $cli file-delete $urii $name
        done
        for i in {1..250}
        do
            name="datei$i"
            $cli file-delete $urii $name
        done
        for i in {500..1000}
        do
            name="datei$i"
            $cli file-delete $urii $name
        done
        assertCount 0
        assertDataNodeHash $urid1 0
        assertDataNodeHash $urid2 0
    fi

#test: append test
    $cli file-delete-all $urii
    assertCount 0
    file=$fileroot/a.txt
    tmpfile=$fileroot/appendix.txt

    $cli localfile-create $file 500 ten
    $cli file-store $urii $file a
    assertTotalSizeRemote 500
    assertCount 1
    assertDataNodeHash $urid1 903
    assertDataNodeHash $urid2 903

    $cli localfile-create $tmpfile 100 ten
    $cli file-append-localfile $urii a $tmpfile
    assertTotalSizeRemote 600
    assertCount 1
    assertDataNodeHash $urid1 1203
    assertDataNodeHash $urid2 1203

    $cli file-append-localfile $urii a $tmpfile
    assertTotalSizeRemote 700
    assertCount 1
    assertDataNodeHash $urid1 1604
    assertDataNodeHash $urid2 1604

    $cli localfile-create $tmpfile 99 ten
    $cli file-append-localfile $urii a $tmpfile
    assertTotalSizeRemote 799
    assertDataNodeHash $urid1 2000
    assertDataNodeHash $urid2 2000

    $cli localfile-create $tmpfile 2 single x
    $cli file-append-localfile $urii a $tmpfile
    assertTotalSizeRemote 801
    assertDataNodeHash $urid1 2010
    assertDataNodeHash $urid2 2010

    $cli localfile-create $tmpfile 599 single y
    $cli file-append-localfile $urii a $tmpfile
    assertTotalSizeRemote 1400
    assertDataNodeHash $urid1 5607
    assertDataNodeHash $urid2 5607

    $cli file-load $urii a $fileroot/a.saved

#test for big file
	echo
	echo TEST big file
	bigfile=$fileroot/big.iso
	$cli localfile-create $bigfile 5000000000 ten
	$cli change-setting $urii chunksize 1048576
	$cli file-store $urii ~/big.iso

#test for loosing one data node
    echo
    echo TEST loose one node

    $cli file-delete-all $urii
	$cli localfile-create $fileroot/a.txt 2000 single 1
	$cli file-store $urii $fileroot/a.txt a

	$cli datanode-stop $urid1
	$cli datanode-list $urii

	$cli file-load $urii a $fileroot/a.loose
	$cli file-load $urii a $fileroot/a.loose
	$cli file-load $urii a $fileroot/a.loose
	$cli file-load $urii a $fileroot/a.loose
	$cli file-load $urii a $fileroot/a.loose

	assertDataNodeCount 1

echo "all tests done"
echo "ALLSUC"
