#!/bin/sh
#
# March 2006, M. Spiekermann
#
# Sept 2006, M. Spiekermann. Sed commands changed and automatic modification
# of AlgebraList.i.cfg implemented. 
#
# makes a list of all files except those
# specified in file $1 and writes them to $2

if let $[$# < 2]; then
  echo "Usage: $0 input-file output-file [prefix=secondo]"
  exit 1
else
  infile=$1
  outfile=$2
fi

prefix="secondo"
if [ $# == 3 ]; then
  prefix=$3
fi

exec 3< $infile
exec 4> $outfile
i=1
section=0

cmd="find $prefix "
algnames=""
dirnames=""

while read <&3 line; do
  if [ ${#line} -gt 0 ]; then
    if [ `expr index "$line" '#'` -ne 1 ]; then
      if [ $section -eq 1 ]; then

        # handle entries of the algebra section  
        algebra=`echo $line | awk -F" " ' { print $1 }'`
        algebradir=`echo $line | awk -F" " ' { print $2 }'`

        echo "Excluding algebra: $algebra (dir: $algebradir)"
        if [ -d "$prefix/Algebras/$algebradir" ]; then
          cmd=$cmd" -regex \"$prefix/Algebras/$algebradir\" -prune -o "
        fi

        if [ "$algnames" == "" ]; then
          algnames=$algebra 
        else
          algnames=$algnames"|$algebra"
        fi
        if [ "$dirnames" == "" ]; then
          dirnames=$algebradir 
        else
          dirnames=$dirnames"|$algebradir"
        fi

      elif [ $section -eq 2 ]; then

        # handle entries of the directories section  
        if [ -d "$prefix/$line" ]; then
          echo "Excluding directory: $line"
          cmd=$cmd" -regex \"$prefix/$line\" -prune -o "
          entered=1
        fi

      elif [ $section -eq 3 ]; then

        # handle entries of the files section  
        if [ -f "$prefix/$line" ]; then
          echo "Excluding file: $line"
          cmd=$cmd"! -regex \"$prefix/$line\" "
          entered=1
        fi

      elif [ $section -eq 4 ]; then

        # handle entries of the pattern section  
        echo "Excluding files with pattern: *$line"
        cmd=$cmd"! -name \"*$line\" "
        entered=1

      fi
     
    else
      # look for a section header
      if [ `expr match "$line" '# Algebras'` -gt 1 ]; then
        section=1
      elif [ `expr match "$line" '# Directories'` -gt 1 ]; then
        section=2
      elif [ `expr match "$line" '# Files by Name'` -gt 1 ]; then
        section=3
      elif [ `expr match "$line" '# Files by Extension'` -gt 1 ]; then
        section=4
      fi

    fi
  fi
done

# remove non-public algebra modules from makefile.algebras
cmd=$cmd" ! -type d -print"

file1="$prefix/makefile.algebras.sample"
file2="$prefix/Algebras/Management/AlgebraList.i.cfg"
sed1="cat $file1 | sed -re '/ALGEBRAS.*[+:]*=.*($algnames)/d' | sed -re '/ALGEBRA_DIRS.*[+:]*=.*($dirnames)/d'"
sed2="cat $file2 | sed -re '/ALGEBRA_INCLUDE.*($algnames)/d'"


echo "sed1: $sed1"
echo "sed2: $sed2"
echo "cmd: $cmd"

eval $cmd > $2
eval $sed1 > .tmp_algebras
mv .tmp_algebras $file1
eval $sed2 > .tmp_alglist
mv .tmp_alglist $file2

exit 0

