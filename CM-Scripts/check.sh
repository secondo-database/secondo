#!/bin/sh
#
# check.sh $1
#
#   $1 fileName
#
# check the syntax of a bash shell script and report the difference between the
# defined and used variables. This is a simple test which can help to avoid
# using misspelled variables.
# 
# Definition of variables are recognized by the regex pattern
# "[${alpha}0-9_]+=" and used variables by the pattern [${alpha}0-9{_]. That's
# simple, but not exact in any case. However, sometimes more variables are
# recognized, e.g. x='$y' will report the usage of variable y, or 
#
#   for name in $list do ...
#
# will report a undefined variable when $name is used later. 
#
#
# History:
# --------
# August 2005, M. Spiekermann: Initial version

bash -n $1
if [ "$?" != "0" ]; then
  echo -e "\n There are syntax errors! \n"
  exit 1
fi

defVarFile=".defVar"
usedVarFile=".usedVar"

#extract defined variables
pat1="[0-9:]*"
alpha="a-zA-Z"
pat2="[${alpha}0-9_]"

defVar=$(grep -no "$pat2\+=" $1)

echo -e "$defVar"  | sed -e 's#'$pat1'\('$pat2'\+\)=.*#\1#g;' | sort | uniq > $defVarFile

#extract used variables
pat2="[${alpha}0-9{_]" 

usedVar=$(grep -no "\$$pat2\+" $1) 

echo -e "$usedVar" | sed -e 's#'$pat1'$\('$pat2'\+\)#\1#g; s#{##g; s#}##g;' | sort | uniq > $usedVarFile

varDiff=$(diff $defVarFile $usedVarFile)

echo -e "\nVariables, which are used but not defined:"
echo -e "------------------------------------------"
echo -e "$varDiff" | grep "> " | sed -e 's#> #$#'


echo -e "\nVariables, which are defined but not used:"
echo -e "------------------------------------------"
echo -e "$varDiff" | grep "< " | sed -e 's#< #$#'

exit $?
