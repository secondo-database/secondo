-- November 2004, M. Spiekermann
--
-- Postgres Runtime Environment:
--------------------------------
-- To create a runtime environment for Postgres a database cluster
-- must be created. Its location can be defined in the environment
-- variable PGDATA. The command
--
--   initdb [-D <dir>]
--
-- creates the following files:
--
--	spieker@sopra:~/pg-databases> du -sk *
--	4       PG_VERSION
--	7138    base
--	128     global
--	8       pg_clog
--	4       pg_hba.conf
--	4       pg_ident.conf
--	16400   pg_xlog
--	8       postgresql.conf
--	spieker@sopra:~/pg-databases> du -sk .
--	23694   .
--
-- The database server (postmaster) can be started and stopped
-- by
--
--   pg_ctl start -l <logfile>
--   pg_ctl stop
--
-- Connecting to a database:
----------------------------
-- The command line client is called psql and you can connect to a default
-- database called template1:
--
--   psql template1
--
-- This client connection can be used to run SQL scripts in order to
-- create new databases, run queries, etc. 
-- 
-- Documentation:
--
-- Comprehensive documentation (User Guide, Admin Guide, Progrmmer Guide) can 
-- be found (if it is installed) in
-- 
--   /usr/share/doc/packages/postgresql/html/index.html
--
-- Databases can be created or destroyed with
--
--   CREATE DATABASE <name>
--   DROP DATABASE <name> 


-- We load every relation in a single transaction

\copy customer FROM 'customer.tbl.pg' WITH DELIMITER AS '|'

\copy lineitem FROM 'lineitem.tbl.pg' WITH DELIMITER AS '|'

\copy orders FROM 'orders.tbl.pg' WITH DELIMITER AS '|'

\copy part FROM 'part.tbl.pg' WITH DELIMITER AS '|'

\copy supplier FROM 'supplier.tbl.pg' WITH DELIMITER AS '|'

\copy partsupp FROM 'partsupp.tbl.pg' WITH DELIMITER AS '|'

\copy nation FROM 'nation.tbl.pg' WITH DELIMITER AS '|'

\copy region FROM 'region.tbl.pg' WITH DELIMITER AS '|'
