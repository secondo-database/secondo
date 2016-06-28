#!/bin/bash
#
#
# June 2009, M. spiekermann
#
# A tool for extracting version numbers from a programs version
# message, e.g. getversion.sh major felx -V will return the major version
# number of flex.


# $1 part
# $2 command 
# $3 flag
function getVersion {

  #echo "args: $1 $2 $3"
 # local version1=$($2 $3| sed -nr '1s#.* ([0-9]+)[.]([0-9]+)[.]?([0-9]+)?.*#\1x\2y\3#p')


  local version2=$($2 $3 | sed "s/[^0-9.]*//"  | sed "s/\./x/" | sed "s/\./y/" | sed "s/ .*//g"   ) 

  local version3=$(echo $version2 | sed "s/ .*//g")

  local -i n1=$[${version3%x*}]
  local rest=${version3#*x}
  #showValue rest
  local -i n2=$[${rest%y*}]
  local -i n3=$[${rest#*y}]
  
  if [ "$1" == "major" ]; then
    echo "$n1"
  fi; 
  
  if [ "$1" == "minor" ]; then
    echo "$n2"
  fi; 
 
  if [ "$1" == "subminor" ]; then
    echo "$n3"
  fi; 
}

getVersion $*
