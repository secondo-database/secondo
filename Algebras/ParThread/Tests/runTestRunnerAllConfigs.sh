#!/bin/bash

runTestRunner.sh ../Algebras/ParThread/Tests/TestConfigs/scfgMaxThreads.ini
runTestRunner.sh ../Algebras/ParThread/Tests/TestConfigs/scfgFourThreads.ini
runTestRunner.sh ../Algebras/ParThread/Tests/TestConfigs/scfgTwoThreads.ini
runTestRunner.sh ../Algebras/ParThread/Tests/TestConfigs/scfgSerial.ini
runTestRunner.sh ../Algebras/ParThread/Tests/TestConfigs/scfgNoDataParallelism.ini
