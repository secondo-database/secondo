#!/bin/bash

secondoDir=../../../

/bin/cp -rf ParallelQueryOptimizer.cpp $secondoDir/QueryProcessor/
/bin/cp -rf ParallelQueryOptimizer.h $secondoDir/include/

/bin/cp -rf makefile.libs $secondoDir
/bin/cp -rf AlgebraList.i.cfg $secondoDir/Algebras/Management
/bin/cp -rf makefile.algebras $secondoDir

/bin/cp -rf QueryProcessor.cpp $secondoDir/QueryProcessor/
/bin/cp -rf QueryProcessor.h $secondoDir/include/



