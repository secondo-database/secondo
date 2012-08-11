#!/bin/bash

P=http://download.geofabrik.de/osm/europe/germany/
F=nordrhein-westfalen.shp.zip
PF=$P$F
wget "$PF" || return 1

T=$SECONDO_BUILD_DIR/bin/nrw
[ -d $T ] || mkdir $T || return 2

mv $F $T/

cd $T
unzip $F

echo "Please make sure the secondo server proccess is running and press return."read trash

SCMDF=$SECONDO_BUILD_DIR/Optimizer/MemoryAllocation/nrwtest/nrwImportShape.SEC
SecondoCS < $SCMDF

