#!/bin/sh
#
# March 2006, M. Spiekermann
#
# makes a list of all files except those
# specified in $1 and writes them to $2

if [ $# -ne 2 ]; then
 echo -e "ERROR: input and output file expected!"
 exit 1
fi

elements=$(sed -ne 's/\(^[^#].*\)/\1/p' $1)

cmd="find ! -type d"

for elem in $elements; do

  if [ -d $elem ]; then
    echo "Excluding directory: $elem"
    cmd=$cmd" ! -path \"*/$elem/*\""
  else
    echo "Excluding file(s): $elem"
    cmd=$cmd" ! -name \"$elem\""
  fi 
done

echo "cmd:<$cmd>"
eval $cmd > $2

exit 0
