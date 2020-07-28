#!/bin/bash 

files="*.gv"
regex="^(partialTree_ContextEntity_[0-9]*_[0-9]*).gv$"
for f in $files    # unquoted in order to allow the glob to expand
do
   if [[ $f =~ $regex ]]
    then
        name=${BASH_REMATCH[1]}
        echo "Converting $name"
        dot -Tpng "${name}.gv" -o "${name}.png" 
    fi
done

 
