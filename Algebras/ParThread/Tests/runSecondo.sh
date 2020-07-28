#!/bin/bash

if [ "$1" = "" ]; then
    echo "ERROR: first parameter must be the path to the configuration file"
	echo "e.g realtive to bin directory: ../Algebras/ParThread/Tests/TestConfigs/scfgMaxThreads.ini"
	exit
fi

configPath=$1
secondoBin="$HOME/secondo/bin"

cd "$secondoBin"

export SECONDO_PARAM_RTFlags="SMI:NoTransactions,DEBUG:DemangleStackTrace,CMSG:Color,CTLG:SkipExamples,SI:PrintCmdTimes,SI:ShowCommandTime,SI:NoCommandEcho,QP:OpTree2SVG,QP:ProgDisable"

SecondoBDB -c "$configPath"
