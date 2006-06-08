#!/bin/sh
#
# March 2006, M. Spiekermann
#
# makes a list of all files except those
# specified in $1 and writes them to $2

if [ $# != 2 ]; then
  echo "Usage: $0 input-file output-file"
  exit 1
else
  infile=$1
  outfile=$2
fi

exec 3< $infile
exec 4> $outfile
i=1
section=0

cmd="find secondo "
sedcmd="cat secondo/makefile.algebras.temp"

while read <&3 line; do
  if [ ${#line} -gt 0 ]; then
    if [ `expr index "$line" '#'` -ne 1 ]; then
      if [ $section -eq 1 ]; then

        algebra=`echo $line | awk -F" " ' { print $1 }'`
        algebradir=`echo $line | awk -F" " ' { print $2 }'`

        echo "Excluding algebra: $algebra (dir: $algebradir)"
        if [ -d "secondo/Algebras/$algebradir" ]; then
          cmd=$cmd" -wholename \"secondo/Algebras/$algebradir\" -prune -o"
        fi

        sedcmd=$sedcmd" | sed -e \"/ALGEBRAS.*\+\=.*$algebra/d\""
        sedcmd=$sedcmd" | sed -e \"/ALGEBRA_DIRS.*\+\=.*$algebradir/d\""

      elif [ $section -eq 2 ]; then

        if [ -d "secondo/$line" ]; then
          echo "Excluding directory: $line"
          cmd=$cmd" -wholename \"secondo/$line\" -prune -o"
          entered=1
        fi

      elif [ $section -eq 3 ]; then

        if [ -f "secondo/$line" ]; then
          echo "Excluding file: $line"
          cmd=$cmd" -wholename \"secondo/$line\" -prune -o"
          entered=1
        fi

      elif [ $section -eq 4 ]; then

        echo "Excluding files with pattern: *$line"
        cmd=$cmd" -name \"*$line\" -prune -o"
        entered=1

      fi
     
    else

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

cmd=$cmd" ! -type d -print"
sedcmd=$sedcmd" | sed -e \"/^$/N;/\n$/D\""
echo "sedcmd: $sedcmd"
echo "cmd: $cmd"
eval $cmd > $2
cp secondo/makefile.algebras.sample secondo/makefile.algebras.temp
eval $sedcmd > secondo/makefile.algebras.sample
rm secondo/makefile.algebras.temp

exit 0

