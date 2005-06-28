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




baseDir=$HOME/${0%/*}
# include function definitions
# libutil.sh must be in the search PATH 
if [ -s $baseDir/libutil.sh ]; 
then
  if ! source $baseDir/libutil.sh; then exit 1; fi
else
  if ! source libutil.sh; then exit 1; fi
fi

#default options
rootDir=$HOME
coDir=tmp_secondo_${date_ymd}_${date_HMS}
cvsDir=/${CVSROOT#*/}
coTag="HEAD"
coModule="secondo"
sendMail_Deliver="true"

declare -i numOfArgs=$#
let numOfArgs++

while [ $# -eq 0 -o $numOfArgs -ne $OPTIND ]; do

  getopts "hnr:c:t:" optKey
  if [ "$optKey" == "?" ]; then
    optKey="h"
  fi

  case $optKey in

   h) showGPL
      printf "\n%s\n" "Usage of ${0##*/}:" 
      printf "%s\n"   "  -r<root-dir> => Mandatory argument"
      printf "%s\n"   "Options:"
      printf "%s\n"   "  -h print this message and exit."
      printf "%s\n"   "  -n send no mails. Just print the message to stdout."
      printf "%s\n"   "  -t<version-tag> => \"${coTag}\" "
      printf "%s\n"   "  -m<cvs-module> => \"${coModule}\" "
      printf "%s\n\n" "  -c<checkout-dir> => \"tmp_secondo_<date>\""
      printf "%s\n"   "The script checks out a local copy of <cvs-module> into the directoy"
      printf "%s\n"   "into <root-dir>/<checkout-dir> and runs make, various tests, etc."
      printf "%s\n\n" "In case of a failure an email will be sent to the CVS users."
      exit 0;;
   
   r) rootDir=$OPTARG;;

   c) coDir=$OPTARG;;

   t) coTag=$OPTARG;;

   n) sendMail_Deliver="false";;

  esac

done

# set up environment for secondo
source ~/.bashrc # this is needed when started by cron
if ! source ~/.secondorc ""; then exit 1; fi

# derive some other important directories
cbuildDir=${rootDir}/${coDir}
scriptDir=${cbuildDir}/CM-Scripts

printSep "Startup parameters ($(date))"

printf "%s\n" "Script = ${PWD}/$0"
printf "%s\n" "rootDir = $rootDir"
printf "%s\n" "cvsDir = $cvsDir"
printf "%s\n" "coDir = $coDir"
printf "%s\n" "coTag = $coTag"
printf "%s\n" "coModule = $coModule"
printf "%s\n" "sendMail_Deliver = $sendMail_Deliver"


## check if files in the module secondo were changed
## since the last run
lastChanges=$HOME/.last_changes_secondo
lastDate=$(find $lastChanges -printf "%AY-%Am-%Ad %AH:%AM\n")
cvs history -xMAR -a -D$lastDate -p secondo > $lastChanges
hasChanged=$(cat $lastChanges | sed -ne'2p')

if [ "$hasChanged" != "" ]; then
 
printf "last changes:\n"
cat $lastChanges

cvshist_result=$( cat $lastChanges | 
                  awk '/./ { print $5 }' | sort | uniq | tr "\n" " " )

printf "%s\n" "cvs user who commited or added files yesterday:"
printf "%s\n" "$cvshist_result"

recipients=""
for userName in $cvshist_result; do

  mapStr "${cvsDir}/CVSROOT/users" "$userName" ":"
  recipients="$recipients $mapStr_name2"

done

else

  printf "\nNo changes since ${lastDate}! \n\n"
  exit 1;

fi

exit



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
setvar $cbuildDir
printf "%s\n" "Environment settings"
catvar


printSep "Alias definitions"
alias



cd $rootDir
checkCmd "cvs -Q checkout -d $coDir secondo"

## run make
printSep "Compiling SECONDO"

declare -i errors=0
cd $cbuildDir
printf "\n%s" "$PWD"
checkCmd "make > ../make-all.log 2>&1" 

if let $rc!=0; then

  printf "%s\n" "Problems during build, sending a mail to:"
  printf "%s\n" "$recipients"

mailBody="This is a generated message!  

  Users who committed to CVS yesterday:
  $cvshist_result

  You will find the output of make in the attached file.
  Please fix the problem as soon as possible."

  sendMail "Building SECONDO failed!" "$recipients" "$mailBody" "../make-all.log"
  let errors++ 

fi

## run tests

if let $errors==0; then

printSep "Running automatic tests"
timeOut "300" "60" "${scriptDir}/run-tests.sh" 

if let $rc!=0; then
  
  printf "%s\n" "Problems during test, sending a mail to:"
  printf "%s\n" "$recipients"
  cat ${cbuildDir}/Tests/Testspecs/*.log > run-tests.log

mailBody="This is a generated message!  

  Users who committed to CVS yesterday:
  $cvshist_result

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
rm -rf ${cbuildDir}

if [ $[errors] != 0 ]; then

  printf "There were $errors errors!"

else

  # set label
  cvs tag -d LAST_STABLE 
  cvs tag LAST_STABLE

fi

exit $errors
