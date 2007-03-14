#!/bin/sh
#
# checksize -s [files]
# check if the file for each given file is 
# smaller than a specified size.
 
declare -i size=$1
declare -i fs=0
shift
for f in $*; do
  fs=$(find $f -printf "%s");
  if let $[$fs > $size]; then
    echo -e "Error: file $f is bigger than $size bytes."
    exit 1
  fi
done

exit 0

