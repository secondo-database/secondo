#!/bin/sh
#
# August 2006, M. Spiekermann
#
# Reads in the standard input and removes all lines which
# are equal to an existing directory name

while read <&0 line; do
  if [ ${#line} -gt 0 ]; then
    if [ -d $line ]; then
      echo "Removing directory $line" >& 2
    else
      echo $line
    fi
  fi
done

exit 0
