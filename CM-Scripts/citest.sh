#!/bin/bash
#
# This script does a pre-commit check. Currently the script
# checkpd is invoked to check if all "*.cpp" and "*.h" files
# are written in correct PD syntax and could be processed by 
# latex. The file must be invoked in the CVSROOT/commitinfo
# configuration file.
#
# July 2004, M. Spiekermann

prefix="/home/spieker/cvs-snapshot/Tools/pd"

export PATH="$prefix:$PATH"
export PD_HEADER="$prefix/pd.header"

# for test purposes
#dirfiles=$(ls -l $PWD)
#printf "$dirfiles \n"

checkpd

exit $? 
