#!/bin/bash

if [ "$1" = "" ]; then
    echo "ERROR: first parameter must be the database name"
	echo "Testserver-DBs: ARNSBERG, GERMANY, NRW"
	exit
fi

export SECONDO_PARAM_RTFlags="DEBUG:DemangleStackTrace,CMSG:Color,SMI:NoTransactions,CTLG:SkipExamples,SI:PrintCmdTimes,SI:ShowCommandTime,SI:NoCommandEcho,QP:OpTree2SVG"
export SECONDO_PARAM_NodeMem=32000
export SECONDO_PARAM_StringMem=8000
export SECONDO_PARAM_TextMem=8000

dbname=$1
basePath=$PWD
queryPath=$PWD/TestQueries/*.sql
configPath=$PWD/TestConfigs/*.ini

secondoBin="$HOME/secondo/bin"

for s in $queryPath
do
    sed -i "s/^open database .*;$/open database ${dbname};/g" ${s}

	sn=$(basename "${s}" ".${s##*.}")
	for c in $configPath
	do
		cn=$(basename "${c}" ".${c##*.}")
		logdir="$basePath/TestResults/${sn}_${cn}"
 
        rm -r ${logdir}
        mkdir ${logdir}
        logname="output.log"

		cd $secondoBin

        runner="SecondoBDB -i $s -c $c"
        $runner 2>&1 | tee "$logdir/$logname"

        cd "$basePath/TestResults"
		mv *.log $logdir
		mv *.gv $logdir
		cd $basePath
    done
done

