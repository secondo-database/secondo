#/bin/bash!
#
# removes the last | in a line

cat "$1" | sed 's#|\(.*\)|#|\1#' > "$1.pg"
