-- Nov. 2004 M. Spiekermann
--
-- A SQL script to create the TPC-H database.
-- run in psql as database admin!
--
-- All char(x) attributes are set to char(48) since
-- this corresponds to SECONDOS datatype string.
--
-- All comments are varchar(x) attributes which correspond
-- to datatype text.
--
-- The int and real data type of SECONDO have a size of 12 bytes
-- hence int8 and float8 are used in Postgres.


CREATE TABLE LINEITEM (
  L_ORDERKEY       int8,
  L_PARTKEY        int8, 
  L_SUPPKEY        int8,
  L_LINENUMBER     int8,
  L_QUANTITY       float8,
  L_EXTENDEDPRICE  float8,
  L_DISCOUNT       float8,
  L_TAX            float8,
  L_RETURNFLAG     char(48),
  L_LINESTATUS     char(48),
  L_SHIPDATE       date,
  L_COMMITDATE     date,
  L_RECEIPTDATE    date,
  L_SHIPINSTRUCT   char(48),
  L_SHIPMODE       char(48),
  L_COMMENT        varchar(44)
);

CREATE TABLE CUSTOMER (
  C_CUSTKEY	int8,
  C_NAME	char(48),
  C_ADDRESS	char(48),
  C_NATIONKEY	int8,
  C_PHONE	char(48),
  C_ACCTBAL	float8,
  C_MKTSEGMENT	char(48),
  C_COMMENT	varchar(117)
);

CREATE TABLE ORDERS (
  O_ORDERKEY       int8,
  O_CUSTKEY        int8,
  O_ORDERSTATUS	   char(48),
  O_TOTALPRICE     float8,
  O_ORDERDATE	   date,
  O_ORDERPRIORITY  char(48),
  O_CLERK	   char(48),
  O_SHIPPRIORITY   int8,
  O_COMMENT        varchar(79)
);


CREATE TABLE NATION (
  N_NATIONKEY  int8,
  N_NAME       char(48),
  N_REGIONKEY  int8,
  N_COMMENT    varchar(152) 
);


CREATE TABLE PART (
  P_PARTKEY      int8,
  P_NAME         varchar(55),
  P_MFGR         char(48),
  P_BRAND        char(48),
  P_TYPE         char(48),
  P_SIZE         int8,
  P_CONTAINER    char(48),
  P_RETAILPRICE  float(4),
  P_COMMENT      char(48)
);

CREATE TABLE PARTSUPP (
  PS_PARTKEY     int8,
  PS_SUPPKEY     int8,
  PS_AVAILQTY    int8,
  PS_SUPPLYCOST  float(4),
  PS_COMMENT     varchar(199) 
);

CREATE TABLE REGION (
  R_REGIONKEY  int8,
  R_NAME       char(48),
  R_COMMENT    varchar(152) 
);

CREATE TABLE SUPPLIER (
  S_SUPPKEY    int8,
  S_NAME       char(48),
  S_ADDRESS    char(48),
  S_NATIONKEY  int8,
  S_PHONE      char(48),
  S_ACCTBAL    float(4),
  S_COMMENT    varchar(101) 
);


