#!/bin/sh
#
# 06.02.2006 Create TPC-H tables in a postgres
# database and import the tabel data from files.


db="$1"
rootDir="$2"

if [ "$db" == "" ]; then
  echo "Error: no database specified!"
  exit 1
fi 

if [ "$rootDir" == "" ]; then
  rootDir=$PWD
fi 

echo "db: $db"
echo "rootDir: $rootDir"

for table in "customer" "lineitem" "orders"; do
  echo "COPY $table FROM '$rootDir/s05pp/$table.tbl.pg' WITH DELIMITER AS
'|'" | psql -d $db -e
done

vacuumdb -ef -d $db
