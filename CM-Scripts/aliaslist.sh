# list of command aliases
#
# Feb 2005, M. Spiekermann


# function newAlias
#
# $1 alias name
# $2 command name
#
# Adds path information before the command name $2
function newAlias() {

if [ -z $SECONDO_BUILD_DIR ]; then
  alias ${1}="$2"
else
  alias ${1}="${SECONDO_BUILD_DIR}/${cmdDir}/$2"
fi

}
###########################################################
##
## Start of configurable part
##
## Please add new shell scripts here. $scriptDir defines
## a path relative to $SECONDO_BUILD_DIR.
##
###########################################################

cmdDir=CM-Scripts
newAlias backup backup.sh
newAlias runtests run-tests.sh
newAlias cvstest cvs-make.sh

cmdDir=Tools/Generators/TPC-H/secondo
newAlias tpcgen tpcgen.sh
newAlias tpcrun tpcrun.sh

cmdDir=bin
newAlias SecondoTTY SecondoTTYBDB
newAlias SecondoMonitor "SecondoMonitorBDB -s"


# define some useful aliases
alias setvar="source ${SECONDO_SDK}/bin/setvar.bash" 
alias catvar="${SECONDO_SDK}/bin/catvar.sh" 
alias jcvs='java -jar ${SECONDO_SDK}/jCVS-5.2.2/jars/jcvsii.jar'
alias cvs-info='cvs -nq update | grep "^[A-Z]"'
alias cvs-mod='cvs -nq update | grep "^[M]"'


