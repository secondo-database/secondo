#!/bin/bash


if [ $# -eq 0 ]; then
    echo "Usage $0 <Filename of the stracktrace>"
    exit 1
fi

stacktrace_output=$1

if [ ! -f $stacktrace_output ]; then
   echo "File $stacktrace_output does not exists"
   exit 1
fi

echo -e "\n"
echo "========"
echo " Trying to decode the stacktrace..."
echo "========"
echo -e "\n"

# Get relocation if available 
if [ $(cat $stacktrace_output | grep "Binary relocation:" | wc -l) -gt 0 ]; then
   binary_relocation=$(cat $stacktrace_output | grep "Binary relocation:" | cut -d ":" -f 2)
else
   binary_relocation="0x0"
fi

# Extract and collect addresses
stacktrace=$(cat $stacktrace_output | grep "0x")
for line in $stacktrace; do

   if [ $(echo $line | grep "\[" | wc -l) -eq 0 ]; then
      continue
   fi

   addr=$(echo $line | cut -d "[" -f 2 | sed s/]//g)
   printf -v raddr "0x%X" $(($addr - $binary_relocation))
   lines="$lines $raddr"
done

echo "Executing: "addr2line --demangle=auto -p -fs -e SecondoBDB $lines 
echo -e "\n"
echo -e "\n"

addr2line --demangle=auto -p -fs -e SecondoBDB $lines


