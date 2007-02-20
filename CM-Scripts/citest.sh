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

prefix="/home/spieker/secondo"

pdDir="$prefix/Tools/pd"
scriptDir="$prefix/CM-Scripts"

export PATH="$pdDir:$scriptDir:$PATH"
export PD_HEADER="$pdDir/pd.header"

files=$(find $PWD -path "*CVS" -prune -o -type f -print)

textFiles=$(find $PWD -name "*.txt")
for f in $textFiles; do
  linecheck $f;
  rc=$?
  if [ $rc -ne 0 ]; then
    exit $rc
  fi
done


# for test purposes
echo -e "cvs server: Running pre-commit check for \n"
for f in $files; do
  echo -e "  $f\n"
done

checksize.sh 512000 $files
rc=$?
if [ $rc -ne 0 ]; then
  exit $rc;
fi

checkpd --strong

exit $? 
