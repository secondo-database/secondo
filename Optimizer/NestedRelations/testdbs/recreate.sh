#!/bin/bash

WD=$PWD

cd $SECONDO_BUILD_DIR/bin

SecondoBDB < $WD/createtestdbs.stext

