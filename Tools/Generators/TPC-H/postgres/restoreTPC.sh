# Dec. 2004, M. Spiekermann
#
# This script creates the relations of the TPC-H
# database and imports the data into a Postgres
# database. The data created by dbgen must be located
# in the current directory. 

if [ "$1" == "" ]; then
  echo -e "\n Usage: $0 <dbname> [<table-dir>]\n\n"
  exit 1
fi

db="$1"
PATH=".:$PATH"

tbl_dir="$PWD"
if [ "$2" != "" ]; then
  tbl_dir="$2"
fi


# convert data suitable for the Postgres copy command
echo -e "\n*** Preparing data files ***\n"
for file in $tbl_dir/*.tbl
do
 if [ ! -e $file.pg ]; then	
   echo -e "Converting $file ...\n"
   rm-lastsep.sh $file
 fi  
done

# create a database
createdb $db

# create relations
psql -e -f"create_tbls_64bit.sql" -d"$db"

# import data
import_tbls.sh $db $tbl_dir
