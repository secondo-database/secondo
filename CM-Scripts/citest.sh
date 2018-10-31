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

# compare directories or names with
# 

function metaUser {
return 0
#  if [ "$1" = "duentgen" ]; then
#    return 0	  
#  fi
#  if [ "$1" = "spieker" ]; then
#    return 0	  
#  fi
#  if [ "$1" = "behr" ]; then
#    return 0	  
#  fi
#  return 1  
}


function checkPath {

  if [ "$1" == "$2" ]; then
    echo -e "\n ERROR: check in forbidden!"
    echo -e "\n Reason: complex re-design and re-implementation task of FLOBs."
    echo -e "\n Closed file or path: $1 \n"
    exit 1
  fi
}

#echo "Args = $*"
#echo "CVS_USER = $CVS_USER"
argPath=$1
shift
argFiles=$*
cvsh="/home/cvsroot/secondo"

# check special file names for
# unprivileged users
metaUser "$CVS_USER"
if [ $? -ne 0 ]; then

echo -e "\n\n *** Sorry, but committing files to CVS is currently not allowed for user $CVS_USER! *** \n\n"
#checkPath $argPath $cvsh/Algebras/Relation-C++
#checkPath $argPath $cvsh/StorageManager
#
#for f in $argFiles; do
#  checkPath $f FLOB.h
#  checkPath $f FLOBCache.h
#  checkPath $f Attribute.h
#  checkPath $f Tuple.h
#  checkPath $f DBArray.h
#done

fi



 function checkExtension {

    invalid="dep project cproject o a"

    for i in $invalid; do
      if [[ "$1" == *.$i ]]; then
        echo "1"
        return
      fi
    done
    echo "0"
}



prefix="/home/behr/secondo"

pdDir="$prefix/Tools/pd"
scriptDir="$prefix/CM-Scripts"
usingdir="$prefix/Tools/usingcheck"


export PATH="$pdDir:$scriptDir:$usingdir:$PATH:."
export PD_HEADER="$pdDir/pd.header"

files=$(find $PWD -path "*CVS" -prune -o -type f -print)

#textFiles=$(find $PWD -name "*.txt")
#for f in $textFiles; do
#  linecheck $f;
#  rc=$?
#  if [ $rc -ne 0 ]; then
#    exit $rc
#  fi
#done


## for test purposes
echo -e "cvs server: Running pre-commit check for \n"
for f in $files; do
  echo -e "  $f\n"
done

checksize.sh 71200000 $files
rc=$?
if [ $rc -ne 0 ]; then
  exit $rc;
fi

extensionOK="true"
for f in $files; do
   rc=$(checkExtension $f)
   if [ "$rc" != "0" ]; then
     echo "found invalid file extension in file "  $f
     extensionOK="false"
   fi  
done

if [ "$extensionOK" == "false" ]; then
   exit 1;
fi



#checkpd --strong

rc=$?
if [ $rc -ne 0 ]; then
  exit $rc;
fi

checkusing


exit $? 
