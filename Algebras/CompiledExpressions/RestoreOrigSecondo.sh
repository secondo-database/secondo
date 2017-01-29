#!/bin/bash

source $HOME/.secondorc $HOME/secondo

cd $SECONDO_BUILD_DIR
if [ -f ./Algebras/CompiledExpressions/origSecondoKernel4CEA.tgz ] ; then
  echo "Replace the orginal Secondo Code Files..."
  tar -xvzf ./Algebras/CompiledExpressions/origSecondoKernel4CEA.tgz
  rm ./Algebras/CompiledExpressions/origSecondoKernel4CEA.tgz
fi
