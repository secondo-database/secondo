#!/bin/sh
#
# 06.02.2006 Create TPC-H tables in a postgres
# database and import the tabel data from files.


db="$1"
dataDir="$2"

if [ "$db" == "" ]; then
  echo "Error: no database specified!"
  exit 1
fi 

if [ "$dataDir" == "" ]; then
  dataDir=$PWD
fi 

# import data
echo -e "\n *** Importing data from $dataDir into database $db ... *** \n"

for table in $dataDir/*.tbl.pg; do
  basename=${table##*/}
  rel=${basename%.tbl.pg}
  echo "COPY $rel FROM '$table' WITH DELIMITER AS
'|'" | psql -d $db -e
done

vacuumdb -ef -d $db

exit $?
