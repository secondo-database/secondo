-- run in psql as database admin

CREATE TABLE LINEITEM (
  L_ORDERKEY       int4,
  L_PARTKEY        int4, 
  L_SUPPKEY        int4,
  L_LINENUMBER     int4,
  L_QUANTITY       float4,
  L_EXTENDEDPRICE  float4,
  L_DISCOUNT       float4,
  L_TAX            float4,
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
  C_CUSTKEY	int4,
  C_NAME	varchar(25),
  C_ADDRESS	varchar(40),
  C_NATIONKEY	int4,
  C_PHONE	char(15),
  C_ACCTBAL	float4,
  C_MKTSEGMENT	char(10),
  C_COMMENT	varchar(117)
);

CREATE TABLE ORDERS (
  O_ORDERKEY		int4,
  O_CUSTKEY		int4,
  O_ORDERSTATUS		char(1),
  O_TOTALPRICE		float4,
  O_ORDERDATE		date,
  O_ORDERPRIORITY	char(15),
  O_CLERK		char(15),
  O_SHIPPRIORITY	int4,
  O_COMMENT		varchar(79)
);

