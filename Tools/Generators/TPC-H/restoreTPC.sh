# Dec. 2004, M. Spiekermann
#
# This script creates the relations of the TPC-H
# database and imports the data into a Postgres
# database. The data created by dbgen must be located
# in the current directory. 

if [ "$1" == "" ]; then
  printf "\n Usage: $0 <dbname> [<table-dir>]\n\n"
  exit 1
fi

db="$1"
pg_scripts="$PWD/postgres"
tbl_dir="$PWD"
if [ "$2" != "" ]; then
  tbl_dir="$2"
fi

# convert data suitable for the Postgres copy command
for file in $tbl_dir/*.tbl
do
 $pg_scripts/rm-lastsep.sh $file
done

cd $tbl_dir
# create a database
createdb $db

# create relations
psql -e -f"$pg_scripts/create_tbls.sql" -d"$db"

# import data
psql -e -f"$pg_scripts/import_tbls.sql" -d"$db"
