# !/bin/bash
#
# cvs-make.sh: 
# 
# This bash-script checks out the last SECONDO sources
# and runs make to compile them.
#
# 03/02/28 M. Spiekermann
# 04/01/27 M. Spiekermann, port from csh to bash 

# recognize aliases also in an non interactive shell
shopt -s expand_aliases
source $HOME/.bashrc

secondoRoot=$HOME/SecBase

# check argument
if [ $# -eq 0 ]
then

  echo "Usage: $0 dir-name"
  exit

else

  buildDir=$secondoRoot/$1
fi


## report file systems sizes
echo ""
echo "File systems :"
echo "--"
df -k

echo ""
echo "- Step 1: Checking out work copy -"
echo "----------------------------------"
echo ""

## checkout work copy
setvar $buildDir
catvar
recipients=$(cvs history -c -a -D yesterday | awk '/./ { print $5 }' | sort | uniq)

echo ""
echo "cvs user who commited or added files yesterday:"
echo "--"
echo "$recipients"


cd $secondoRoot
cvs -Q checkout -d $1 secondo
cd $buildDir

echo ""
echo "Step 2: Compiling SECONDO"
echo "--"
echo ""

## run make
if !( make > ../make-all.log 2>&1 ) 
then

  echo ""
  echo "Problems during build, sending a mail to:"
  echo "--"
  echo "$recipients"

  mail -s"Building secondo on zeppelin failed!" -a ../make-all.log spieker <<- EOFM
	This is a generated message! 
	
	Users who comitted yesterday: 
	$recipients
	
	You will find the output of make in the attached file.
	Please fix the problem as soon as possible.
EOFM
  exit

fi

echo ""
echo "Step 3: Cleaning SECONDO -"
echo "--"
echo ""
## run make clean
if !( make clean > ../make-clean.log 2>&1 ) 
then
  echo ""
  echo "Problems with make clean!"
  exit
fi

echo ""
echo "Step 4: Check for undeleted files -"
echo "--"
echo ""
find . -name "*.o"
find . -name "*.so"
find . -name "*.a"
find . -name "*.dll"
find . -name "*.class"
find ./lib ! -path "*CVS*" 
find ./bin ! -path "*CVS*"

## clean up
cd ..
rm -rf $buildDir
