#!/bin/bash

if [ "$1" = "" ]; then
    echo "ERROR: first parameter must be the database name"
	echo "Testserver-DBs: ARNSBERG, GERMANY, NRW"
	exit
fi
if [ "$2" = "" ]; then
    echo "ERROR: second parameter must be the testgroup."
    echo "This is used to replace some tablenames in queries"
	echo "e.g TCA, TCB, TCB"
	exit
fi

export SECONDO_PARAM_RTFlags="DEBUG:DemangleStackTrace,CMSG:Color,SMI:NoTransactions,CTLG:SkipExamples,SI:PrintCmdTimes,SI:ShowCommandTime,SI:NoCommandEcho,QP:OpTree2SVG"
export SECONDO_PARAM_NodeMem=32000
export SECONDO_PARAM_StringMem=8000
export SECONDO_PARAM_TextMem=8000


dbname=$1
testgroup=$2
basePath=$PWD
queryPath=$PWD/TestQueries/*.sql
configPath=$PWD/TestConfigs/*.ini

secondoBin="$HOME/secondo/bin"

rm "$basePath/TestResults/runtimes.log"

#iterate all scripts in scriptpath
for s in $queryPath
do
    #iterate all configs in configpath
	sn=$(basename "${s}" ".${s##*.}")
	for c in $configPath
	do
		cn=$(basename "${c}" ".${c##*.}")
        
		#execute each query multiple times
		for n in 1 2 3
		do
            #copy the config and sql script to working directory
			logdir="$basePath/TestResults/${testgroup}_${sn}_${cn}_${n}"
		    rm -r ${logdir}
		    mkdir ${logdir}

            sl="$logdir/${sn}.sql"
            cl="$logdir/${cn}.ini"         
            cp $s $sl
            cp $c $cl

            #replace test specific parts of the sql statement
			sed -i "s/^open database .*;$/open database ${dbname};/g" ${sl}
			sed -i "s/Osm_Ways/${testgroup}_Ways/g" ${sl}
			sed -i "s/Osm_Nodes/${testgroup}_Nodes/g" ${sl}

		    logname="output.log"

			cd $secondoBin

            export SECONDO_PARAM_RTFlags="SMI:NoTransactions,DEBUG:DemangleStackTrace,CMSG:Color,CTLG:SkipExamples,SI:PrintCmdTimes,SI:ShowCommandTime,SI:NoCommandEcho,QP:OpTree2SVG,QP:ProgDisable"

		    runner="SecondoBDB -i $sl -c $cl"
		    $runner 2>&1 | tee "$logdir/$logname"
        done

        cd "$basePath/TestResults"
		mv *.log $logdir
		mv *.gv $logdir
		cd $basePath

        #python analyzeLogFiles.py "$logdir/$logname" "$basePath/TestResults/runtimes.log"
    done
done

