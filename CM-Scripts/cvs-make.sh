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


# include function definitions
# libutil.sh must be in the same directory as this file
source  ${0%/*}/libutil.sh

# check arguments and initialize variables
if [ "$1" == "-h" ]; then

  printf "%s\n" "Usage: $0 [<root-dir>=$HOME] [<checkout-dir>=tmp_secondo_<time>]"
  exit 0
fi


if [ "$1" != "" ]; then
  rootDir=$1
else
  rootDir=$HOME
fi

if [ "$2" != "" ]; then 
  coDir=$2
else
  coDir=tmp_secondo_${date_ymd}_${date_HMS}
fi

# directories
buildDir=${rootDir}/${coDir}
scriptDir=${buildDir}/CM-Scripts

# recognize aliases also in an non interactive shell
shopt -s expand_aliases
source $HOME/.bashrc

## report host status 
printSep "host status"
printf "%s\n" "uptime"
uptime
printf "\n%s\n" "disk free"
df -k
printf "\n%s\n" "memory usage"
free -m


## checkout work copy
printSep "Checking out work copy"
setvar $buildDir
printf "%s\n" "Environment settings"
catvar

printf "cvs user who commited or added files yesterday:\n"

recipients=$( cvs history -c -a -D yesterday | 
              awk '/./ { print $5 }' | sort | uniq | tr "\n" " " )

printf "${recipients}\n"

cd $rootDir
checkCmd "cvs -Q checkout -d $coDir secondo"

## run make
printSep "Compiling SECONDO"

declare -i errors=0
cd $buildDir
checkCmd "make > ../make-all.log 2>&1" 

if let $rc!=0; then

  printf "%s\n" "Problems during build, sending a mail to:"
  printf "%s\n" "$recipients"

mailBody="This is a generated message!  

  Users who committed to CVS yesterday:
  $recipients

  You will find the output of make in the attached file.
  Please fix the problem as soon as possible."

  sendMail "Building SECONDO failed!" "$recipients" "$mailBody" "../make-all.log"
  let errors++ 

fi

## run tests

if let $errors==0; then

printSep "Running automatic tests"
if ! ( ${scriptDir}/run-tests.sh )
then
  
  printf "%s\n" "Problems during test, sending a mail to:"
  printf "%s\n" "$recipients"
  cat ${buildDir}/Tests/Testspecs/*.log > run-tests.log

mailBody="This is a generated message!  

  Users who committed to CVS yesterday:
  $recipients

  You will find the output of run-tests in the attached file.
  Please fix the problem as soon as possible."

  sendMail "Automatic tests failed!" "$recipients" "$mailBody" "./run-tests.log"
  let errors++

fi

fi


## run make clean
printSep "Cleaning SECONDO"
checkCmd "make realclean > ../make-clean.log 2>&1" 

printSep "Check for undeleted files ( *.{o,a,so,dll,class} )"
find . -iregex ".*\.\([oa]\|so\|dll\|class\)"

printf "\n%s\n" "files in SECONDO's /lib and /bin directory:"
find ./lib ! -path "*CVS*" 
find ./bin ! -path "*CVS*"

printf "\n%s\n" "files unkown to CVS:"
cvs -nQ update

## clean up
printSep "Cleaning up"
rm -rf ${buildDir}/${coDir}

exit $errors
