#!/bin/bash
#
# This script does a pre-commit check. Currently the script
# checkpd is invoked to check if all "*.cpp" and "*.h" files
# are written in correct PD syntax and could be processed by 
# latex. The file must be invoked in the CVSROOT/commitinfo
# configuration file.
#
# July 2004, M. Spiekermann
#
# cvs runs this script in /tmp/cvs-serv$PID below this directory
# a subdirectory CVS will be created. All files passed by the cvs client
# for a commit are copied (creating subdirectories if necessary). 

echo "********************"
echo "* Server Migration *"
echo "* updates are not  *"
echo "* allowed!         *"
echo "********************"

exit 1 
