#!/bin/sh
#
# April 2006, M. Spiekermann
#
# Pretty print file diffs

f1=$1
f2=$2

diffOptions="-w --minimal --left-column --suppress-common-lines"
a2psOptions="-1r -L66"
head="$f1 vs. $f2"

diff $diffOptions  $f1 $f2 | a2ps --center-title="$head" -o diff1.ps
diff $diffOptions -y --width 180 $f1 $f2 | a2ps $a2psOptions --center-title="$head" -o diff2.ps
