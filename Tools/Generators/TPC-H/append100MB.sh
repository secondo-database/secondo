#!/bin/bash
#
# Append 100MB to file $1

data32="12345678901234567890123456789012"
data128="$data32$data32$data32$data32"
data512="$data128$data128$data128$data128"
data1k="$data512$data512"

of="$1";



for n in 1 2 3 4 5 6 7 8 9 10; do 
for m in 1 2 3 4 5 6 7 8 9 10; do

# create 1 MB
for i in 1 2 3 4 5 6 7 8 9 10; do 
for j in 1 2 3 4 5 6 7 8 9 10; do
for k in 1 2 3 4 5 6 7 8 9 10; do
  echo -e "$data1k\n" >> $of
done
done
done

done
done
