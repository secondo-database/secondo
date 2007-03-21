#!/bin/sh
#
# Since 08.06.2006, M. Spiekermann
# 
# A shell script wich updates group membership for
# CVS repository files. This is needed since access
# privileges could only be managed by group memberships

function checkDir 
{
  if [ ! -d $1 ]; then
    echo "Error: $1 is not a directory!"
  fi
  exit 2 
}

##
## parse arguments
##
while [ $# -ne 0 ]; do

  if [ "$1" == "-lockRoot" ]; then
    shift
    lockRoot="$1"
    checkDir "$lockRoot"
    shift
  else if [ "$1" == "-cvsHome" ]; then
    shift
    cvsHome="$1"
    checkDir "$cvsHome"
    shift
  else
    echo "Unknown option ${1}!"
    exit 1
  fi
  fi

done


exit 3

##
## change group memberships
##

chgrp -fR cvs-dipl $lockRoot/secondo

chgrp -fR cvs-dipl $lockRoot/students
chgrp -fR cvs-dipl $cvsHome/students

#chgrp -R cvs-pub $lockRoot/secondo-data
#chgrp -R cvs-pub $cvsHome/secondo-data

