#!/bin/sh
#
# This shell script creates a regular expression
# as a disjunction of all algebra names. For these Algebras
# the configuration parameters are extracted from the file AlgebraList.i.cfg
# and prefixed with an "ALGEBRA_INCLUDE("

regexp="(,$1,"
shift
while [ $# -ne 0 ];
do
   regexp=$regexp"|,$1,"	
   shift
done

regexp=$regexp")"
awk "BEGIN {FS=\"(\"}; /"$regexp"/{print \"ALGEBRA_INCLUDE(\"\$2}; " AlgebraList.i.cfg
