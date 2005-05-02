#!/bin/sh
#
# Dec. 2004, M. Spiekermann
#
# Set variables for Postgres and Secondo
#

soDir="$SECONDO_BUILD_DIR"

export PGDATA="$HOME/pg-databases"
export PATH="$PATH:$soDir/bin:$soDir/Optimizer:$soDir/CM-Scripts"
export LD_LIBRARY_PATH="/lib:$LD_LIBRARY_PATH"
