#!/bin/bash

source $HOME/.secondorc $HOME/secondo

cd $SECONDO_BUILD_DIR
if [ -f ./Algebras/CompiledExpressions/origSecondoKernel4CEA.tgz ] ; then
  echo "There is already a backup of the original files."
  echo "Please check if a change is necessary."
else
  echo "Save a copy from orginal Secondo Code Files..."
  tar -cvzf ./Algebras/CompiledExpressions/origSecondoKernel4CEA.tgz include/QueryProcessor.h include/AlgebraManager.h Algebras/Management/AlgebraManager.cpp QueryProcessor/SecondoInterfaceTTY.cpp QueryProcessor/QueryProcessor.cpp makefile

  echo "Replace the orginal Secondo Code Files..."
  tar -xvzf ./Algebras/CompiledExpressions/ReplaceSecondoKernel4CEA.tgz
fi
