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


   (OBJECT CUSTOMER
        ()
        (
            (rel
                (tuple
                    (
                        (C_CUSTKEY int)
                        (C_NAME string)
                        (C_ADDRESS string)
                        (C_NATIONKEY int)
                        (C_PHONE string)
                        (C_ACCTBAL real)
                        (C_MKTSEGMENT string)
                        (C_COMMENT text))))))
    (OBJECT LINEITEM
        ()
        (
            (rel
                (tuple
                    (
                        (L_ORDERKEY int)
                        (L_PARTKEY int)
                        (L_SUPPKEY int)
                        (L_LINENUMBER int)
                        (L_QUANTITY real)
                        (L_EXTENDEDPRICE real)
                        (L_DISCOUNT real)
                        (L_TAX real)
                        (L_RETURNFLAG string)
                        (L_LINESTATUS string)
                        (L_SHIPDATE instant)
                        (L_COMMITDATE instant)
                        (L_RECEIPTDATE instant)
                        (L_SHIPINSTRUCT string)
                        (L_SHIPMODE string)
                        (L_COMMENT string))))))
    (OBJECT NATION
        ()
        (
            (rel
                (tuple
                    (
                        (N_NATIONKEY int)
                        (N_NAME string)
                        (N_REGIONKEY int)
                        (N_COMMENT text))))))
    (OBJECT NATION_sample
        ()
        (
            (rel
                (tuple
                    (
                        (N_NATIONKEY int)
                        (N_NAME string)
                        (N_REGIONKEY int)
                        (N_COMMENT text))))))
    (OBJECT ORDERS
        ()
        (
            (rel
                (tuple
                    (
                        (O_ORDERKEY int)
                        (O_CUSTKEY int)
                        (O_ORDERSTATUS string)
                        (O_TOTALPRICE real)
                        (O_ORDERDATE instant)
                        (O_ORDERPRIORITY string)
                        (O_CLERK string)
                        (O_SHIPPRIORITY int)
                        (O_COMMENT text))))))
    (OBJECT PART
        ()
        (
            (rel
                (tuple
                    (
                        (P_PARTKEY int)
                        (P_NAME text)
                        (P_MFGR string)
                        (P_BRAND string)
                        (P_TYPE string)
                        (P_SIZE int)
                        (P_CONTAINER string)
                        (P_RETAILPRICE real)
                        (P_COMMENT string))))))
    (OBJECT PARTSUPP
        ()
        (
            (rel
                (tuple
                    (
                        (PS_PARTKEY int)
                        (PS_SUPPKEY int)
                        (PS_AVAILQTY int)
                        (PS_SUPPLYCOST real)
                        (PS_COMMENT text))))))
    (OBJECT REGION
        ()
        (
            (rel
                (tuple
                    (
                        (R_REGIONKEY int)
                        (R_NAME string)
                        (R_COMMENT text))))))
    (OBJECT REGION_sample
        ()
        (
            (rel
                (tuple
                    (
                        (R_REGIONKEY int)
                        (R_NAME string)
                        (R_COMMENT text))))))
    (OBJECT SUPPLIER
        ()
        (
            (rel
                (tuple
                    (
                        (S_SUPPKEY int)
                        (S_NAME string)
                        (S_ADDRESS string)
                        (S_NATIONKEY int)
                        (S_PHONE string)
                        (S_ACCTBAL real)
                        (S_COMMENT text))))))
    (OBJECT SUPPLIER_sample
        ()
        (
            (rel
                (tuple
                    (
                        (S_SUPPKEY int)
                        (S_NAME string)
                        (S_ADDRESS string)
                        (S_NATIONKEY int)
                        (S_PHONE string)
                        (S_ACCTBAL real)
                        (S_COMMENT text))))))



