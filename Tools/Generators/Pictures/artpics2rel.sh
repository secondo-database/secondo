#!/bin/bash
#
# March 2005, Markus Spiekermann
#
# This script creates a relation object for a collection
# of JPEG pictures. From the path and file names additonal
# information about artists and titles is extraced.
# The data must be organized in a directory structure
# like source/artist/subdir1/.../subdirN/filename.jpg
# The base data was downloaded with wget from
#
# www.ibiblio.org
# www.artchive.com
#
# using the commands
#
# wget -r -nc --random-wait -A"*.jpg" -I /artchive www.artchive.com/ftp_site.htm
# wget -r -nc -np -w2 --random-wait -Q2000m -A"*.jpg" http://www.ibiblio.org/wm/paint/auth/index.html
#
#
# Usage of the shell script:
#
# 1) change to the directory above source
# 2) find * -name "*.jpg" | artpics2rel.sh > objectfile


declare -i id=0
declare -i wrn=0

objdeclaration=" 
(OBJECT paintings
    ()
    (rel
        (tuple
            (
                (id int)
                (artist string)
                (title string)
                (pic picture))))
    ("

# declare object
printf "%s\n" "$objdeclaration"
dateStr=$(date +"%Y-%m-%d-%T")

# assemble tuples in a loop
read inputFile 
while [ "$inputFile" != "" ]; 
do
  # file name
  baseName=${inputFile##*/}
  file=${inputFile#./}
  # artist is the second directory name in path
  dir1=${file%%/*}
  dir2=${file#$dir1/}
  artist=${dir2%%/*}
  
  title=${file#*/}

  # extract keyword for title from subdir1/.../subdirN/filename.jpg
  # remove _, -, ., /, jpg and $artist name
  for xchar in "$artist" "_" "-" "." "/" "jpg" "  " "  "; do
    title=${title//${xchar}/ }
  done
  title=${title## }

  # remove duplicate keywords
  title2=""
  w1=" "
  wx=$title
  while [ "$w1" != "" ]; do
    w1=${wx%% *}
    wx=${wx#$w1 }
    wx=${wx//$w1/}
    title2="$title2$w1 "
  done
  ## remove trailing blanks
  title2=${title2% }
  title2=${title2% }

  # remove special chars in artist
  for xchar in "_" "-"; do
    artist=${artist//${xchar}/ }
  done 

  # check length
  if [ ${#title2} -gt 48 ]; then
    let wrn++;
    errors="# ${file} -- ${title2}\n"
  fi

  printf "%s\n" "( ${id} \"${artist}\" \"${title2}\"" 
  printf "%s\n" "  ( \"${baseName}\""
  printf "%s\n" "    \"${dateStr}\""
  printf "%s\n" "    \"gemaelde\"" 
  printf "%s\n" "     TRUE"
  printf "%s\n" "     <file>${file}</file---> ) )"
  
  let id++
  read inputFile
done

# close list of tuples
printf "%s\n" ") () )"
logFile=artpics2rel.log

printf "%s\n" "# $wrn titles greater than 48 chars." > $logFile
printf "%s\n" "$errors" > $logFile

exit $?
