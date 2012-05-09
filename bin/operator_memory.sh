#!/bin/bash
#
# This script examines the memory / time
# relationship for a given query. The resuls are 
# exported as a CSV file named "OperatorMemory.csv"
#
# Also a gnuplot script is written, called 
# "OperatorMemory.plot". So you can create a 
# gif / latex plot of your data by executing:
# 
# gnuplot OperatorMemory.plot
#
# If you like to include these plot in latex
# you can do it as follow:
#
########################
#\documentclass{article}
#
#\usepackage{graphics}
#\usepackage{nopageno}
#\usepackage{txfonts}
#\usepackage[usenames]{color}
#
#\title{My Sample \LaTeX{} Document}
#\author{Max Mustermann}
#
#\begin{document}
#\maketitle
#\begin{center}
#\input{OperatorMemory}
#\end{center}
#
#\end{document} 
#######################
#
# Use the placeholder _MEMORY_ in your query to specify
# the position for the memory keyword. 
#
# Example:
#
# "query plz feed fifty feed head[20] product sortby[PLZ asc] _MEMORY_ count"
# 
#
# Mai 2012 - Jan Kristof Nidzwetzki
#
#####################################################################

# process parameter
query=$1

# memory values
memory="16 32 48 64 80 96 128 160 192 224 256 288 320 352 384"

# filename for gnuplot
gnuplot="OperatorMemory.plot"

# check parameter
if [ $# -ne 1 ]; then
   echo "Usage $0 <QUERY>"
   echo ""
   echo "Use _MEMORY_ as placeholder for the memory parameter"
   echo ""
   echo "Example $0 \"query plz feed sortby[PLZ] _MEMORY_ count\""
   exit -1
fi

# remove _ chars. For Latex export
gunuplottitle=$(echo $1 | sed s/_//g);

# Write gnuplot info
cat <<-EOF > $gnuplot
set terminal gif
set output 'OperatorMemory.gif'
set autoscale
set pointsize 2.5
set xrange [0: 400]
set grid x y

set xlabel "Memory (MB)"
set ylabel "Time (Sec)"
set title "$gunuplottitle"

EOF

echo -n "set xtics (0" >> $gnuplot

# write xtics labels
for m in $memory; do
echo -n ", $m" >> $gnuplot
done

echo ")" >> $gnuplot

cat <<-EOF >> $gnuplot

plot "< cat OperatorMemory.csv | sed -e 's/,/  /g'" every ::1 using 1:2 pt 7 notitle

set terminal epslatex
set out 'OperatorMemory.tex'
replot

## Uncomment the lines below if your gnuplot supports pdf output
#set terminal pdf
#set out 'OperatorMemory.pdf'
#replot

EOF

# create a temp file for our secondo queries
file=$(mktemp)

cat <<-EOF > $file
open database berlintest

delete MemoryData 

let MemoryData = [const rel(tuple([
  Memory: int,
  ExecutionTime: real
]))
value ()] 

EOF

# First run - get a warm state of the system
echo $query | sed "s/_MEMORY_/{memory 512}/g">> $file
echo "" >> $file

for m in $memory; do
  # delete old variables
   echo "delete x1;" >> $file
   echo "" >> $file
  
  # execute query 3 times to get a warm state of the operator
  for i in 1 2 3; do
      echo $query | sed "s/_MEMORY_/{memory $m}/g">> $file
      echo "" >> $file
   done

   # write execution time
   echo "let x1 = SEC2COMMANDS feed tail[3] avg[ElapsedTime]" >> $file
   echo "" >> $file
   echo "query MemoryData inserttuple[$m, x1] consume" >> $file
   echo "" >> $file
done

echo "query MemoryData" >> $file
echo "" >> $file
echo "query MemoryData feed csvexport['OperatorMemory.csv', FALSE, TRUE] count" >> $file

cat $file 

# execute statements
./SecondoBDB -i $file

# remove old statement file
rm $file

