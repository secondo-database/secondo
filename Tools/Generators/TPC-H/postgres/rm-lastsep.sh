#/bin/bash!
#
# removes the last separator '|' of a line
# and creates a copy ending with suffix ".pg"

if [ "$1" == "" ]; then
  printf "\n Usage: $0 <filename> removes last | of every line \n\n"
  exit 1
fi

cat "$1" | sed 's#|\(.*\)|#|\1#' > "$1.pg"
