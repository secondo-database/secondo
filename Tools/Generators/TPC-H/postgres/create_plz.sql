-- run in psql as database admin

CREATE TABLE PLZ (
  P_PLZ  int4, 
  P_ORT  varchar(48)
);

CREATE TABLE PLZ10 (
  P10_PLZ  int4, 
  P10_ORT  varchar(48),
  P10_NR   int4
);

\copy PLZ FROM '/home-local/pgadmin/datasets/plz.tbl.pg' WITH DELIMITER AS '|'
\copy PLZ10 FROM '/home-local/pgadmin/datasets/plz10.tbl.pg' WITH DELIMITER AS '|'
