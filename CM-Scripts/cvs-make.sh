# !/bin/bash
#
# cvs-make.sh: 
# 
# This bash-script checks out the last SECONDO sources
# and runs make to compile them.
#
# 03/02/28 M. Spiekermann
# 04/01/27 M. Spiekermann, port from csh to bash 
# 05/01/27 M. Spiekermann, major revision, automatic test runs


# recognize aliases also in an non interactive shell
shopt -s expand_aliases
source $HOME/.bashrc



# check arguments and initialize variables
if [ $# -eq 0 ]
then

  printf "%s\n" "Usage: $0 <build-dir> [<checkout-dir>]"
  exit 1
fi


rootDir=$1
if [ "$2" != "" ]; then 
  coDir=$2
else
  coDir="tmp-secondo-build"
fi
buildDir=${rootDir}/${coDir}
scriptDir=${buildDir}/CM-Scripts

# include function definitions
# libutil.sh must be in the same directory as this file
source  ${0%/*}/libutil.sh

## report host status 
printSep "host status"
uptime
df -k
free -m


## checkout work copy
printSep "Checking out work copy"
setvar $buildDir
catvar

printf "cvs user who commited or added files yesterday:\n"

recipients=$( cvs history -c -a -D yesterday | 
              awk '/./ { print $5 }' | sort | uniq | tr "\n" " " )

printf "${recipients}\n"

cd $rootDir
checkCmd "cvs -Q checkout -d $coDir secondo"

## run make
printSep "Compiling SECONDO"

cd $buildDir
checkCmd "make > ../make-all.log 2>&1" 

if [ "$rc" != "0" ]; then

  printf "%s\n" "Problems during build, sending a mail to:"
  printf "%s\n" "$recipients"

mailBody="This is a generated message!  

  Users who comitted to CVS yesterday:
  $recipients

  You will find the output of make in the attached file.
  Please fix the problem as soon as possible."

  sendMail "Building SECONDO failed!" "$recipients" "$mailBody" "../make-all.log"
  proceed="false"

fi

## run tests

if [ "$proceed" != "false" ]; then

printSep "Running automatic tests"
if ! ( ${scriptDir}/run-tests.sh )
then
  
  printf "%s\n" "Problems during test, sending a mail to:"
  printf "%s\n" "$recipients"
  cat ${buildDir}/Tests/Testspecs/*.log > run-tests.log

mailBody="This is a generated message!  

  Users who comitted to CVS yesterday:
  $recipients

  You will find the output of run-tests in the attached file.
  Please fix the problem as soon as possible."

  sendMail "Automatic tests failed!" "$recipients" "$mailBody" "./run-tests.log"

fi

fi


## run make clean
printSep "Cleaning SECONDO"
checkCmd "make realclean > ../make-clean.log 2>&1" 

printSep "Check for undeleted files ( *.{o,a,so,dll,class} )"
find . -iregex ".*\.\([oa]\|so\|dll\|class\)"

printf "%s\n" "files in SECONDO's /lib and /bin directory"
find ./lib ! -path "*CVS*" 
find ./bin ! -path "*CVS*"

printf "%s\n" "files unkown to CVS"
cvs -nQ update

## clean up
##printSep "Cleaning up"
##cd ..
##echo "rm -rf ${buildDir}/${coDir}"
