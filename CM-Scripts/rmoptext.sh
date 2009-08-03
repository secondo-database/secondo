#!/bin/sh
#
# 2009 M. Spiekermann, a tiny script for removing optimizer extensions not
# intended for the public version of secondo.


tmpdir=$1

rmoptDir=$SECONDO_BUILD_DIR/Tools/extensionInstaller

rmopt="java RemoveOptimizerExt"

make -C $rmoptDir

cd $rmoptDir
echo $rmopt $(find $SECONDO_BUILD_DIR/$tmpdir/Optimizer -iname "*.pl")
$rmopt $(find $SECONDO_BUILD_DIR/$tmpdir/Optimizer -iname "*.pl")


