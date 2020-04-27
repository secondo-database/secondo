#!/bin/bash

# install TBB for multithreaded query processing

sudo apt-get -y install libtbb-dev
sudo apt-get -y install libtbb2
sudo apt-get -y install libtbb-doc
sudo apt-get -y install libtbb2-dbg


#support environmental variables for TBB
echo 'export TBB_LIB_DIR=/usr/lib/x86_64-linux-gnu/' >>$HOME/.secondorc
echo 'export TBB_INCLUDE_DIR=/usr/include/tbb/' >>$HOME/.secondorc
echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TBB_LIB_DIR' >>$HOME/.secondorc

