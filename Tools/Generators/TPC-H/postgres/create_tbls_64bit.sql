-- run in psql as database admin
--
-- This is the 64bit variant which uses 8 byte long
-- integers and float datatypes as SECONDO does.


CREATE TABLE LINEITEM (

  L_ORDERKEY       int8,
  L_PARTKEY        int8, 
  L_SUPPKEY        int8,
  L_LINENUMBER     int8,
  L_QUANTITY       float8,
  L_EXTENDEDPRICE  float8,
  L_DISCOUNT       float8,
  L_TAX            float8,
  L_RETURNFLAG     char(1),
  L_LINESTATUS     char(1),
  L_SHIPDATE       date,
  L_COMMITDATE     date,
  L_RECEIPTDATE    date,
  L_SHIPINSTRUCT   char(25),
  L_SHIPMODE       char(10),
  L_COMMENT        varchar(44)
);


CREATE TABLE CUSTOMER (

  C_CUSTKEY	int8,
  C_NAME	varchar(25),
  C_ADDRESS	varchar(40),
  C_NATIONKEY	int8,
  C_PHONE	char(15),
  C_ACCTBAL	float8,
  C_MKTSEGMENT	char(10),
  C_COMMENT	varchar(117)
);


CREATE TABLE ORDERS (

  O_ORDERKEY		int8,
  O_CUSTKEY		int8,
  O_ORDERSTATUS		char(1),
  O_TOTALPRICE		float8,
  O_ORDERDATE		date,
  O_ORDERPRIORITY	char(15),
  O_CLERK		char(15),
  O_SHIPPRIORITY	int8,
  O_COMMENT		varchar(79)
);

CREATE TABLE SUPPLIER (

  S_SUPPKEY   int8,
  S_NAME      char(25),
  S_ADDRESS   varchar(40),
  S_NATIONKEY int8,
  S_PHONE     char(15),
  S_ACCTBAL   float8,
  S_COMMENT   varchar(101)
);


CREATE TABLE NATION (

  N_NATIONKEY  int8,
  N_NAME       char(25),
  N_REGIONKEY  int8,
  N_COMMENT    varchar(152)
);


CREATE TABLE REGION (

  R_REGIONKEY int8,
  R_NAME      char(25),
  R_COMMENT   varchar(152)
);
