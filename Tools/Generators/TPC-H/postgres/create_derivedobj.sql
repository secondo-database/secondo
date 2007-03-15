

-- Build an index and modified version of table
-- LINEITEM

create index orders_o_orderkey on orders(o_orderkey);
create index customer_c_custkey on customer(c_custkey);

create sequence serial start 1;

create table lineitem2
as select
      L_ORDERKEY,
      L_PARTKEY,
      L_SUPPKEY,
      L_LINENUMBER,
      CASE (L_QUANTITY > 15 and L_EXTENDEDPRICE < 15000) WHEN TRUE THEN 1 ELSE 0 END as L_EVALUATEDPREDS,
      L_QUANTITY,
      L_EXTENDEDPRICE,
      L_DISCOUNT,
      L_RETURNFLAG,
      L_LINESTATUS,
      L_SHIPDATE,
      L_COMMITDATE,
      L_RECEIPTDATE,
      L_SHIPINSTRUCT,
      L_SHIPMODE,
      L_COMMENT
      from lineitem;


create table lineitem3
as select
      L_ORDERKEY,
      L_PARTKEY,
      L_SUPPKEY,
      L_LINENUMBER,
      CASE (L_QUANTITY < 6 and L_EXTENDEDPRICE > 9500) WHEN TRUE THEN 1 ELSE 0 END as L_EVALUATEDPREDS,
      L_QUANTITY,
      L_EXTENDEDPRICE,
      L_DISCOUNT,
      L_RETURNFLAG,
      L_LINESTATUS,
      L_SHIPDATE,
      L_COMMITDATE,
      L_RECEIPTDATE,
      L_SHIPINSTRUCT,
      L_SHIPMODE,
      L_COMMENT
      from lineitem;


create table lineitem4
as select
      L_ORDERKEY,
      L_PARTKEY,
      L_SUPPKEY,
      L_LINENUMBER,
      CASE (L_QUANTITY < 6 and L_EXTENDEDPRICE > 9525) WHEN TRUE THEN 1 ELSE 0 END as L_EVALUATEDPREDS,
      L_QUANTITY,
      L_EXTENDEDPRICE,
      L_DISCOUNT,
      L_RETURNFLAG,
      L_LINESTATUS,
      L_SHIPDATE,
      L_COMMITDATE,
      L_RECEIPTDATE,
      L_SHIPINSTRUCT,
      L_SHIPMODE,
      L_COMMENT
      from lineitem;



CREATE TABLE TEN ( T_NO int4);

insert into TEN values (1);
insert into TEN values (2);
insert into TEN values (3);
insert into TEN values (4);
insert into TEN values (5);
insert into TEN values (6);
insert into TEN values (7);
insert into TEN values (8);
insert into TEN values (9);
insert into TEN values (10);


create table THOUSAND as select (A.t_no - 1) * 100 + (B.t_no - 1) * 10 + C.t_no as t_no from ten as A,
ten as B, ten as C;


create table r1000 as select t_no as a, (1000 * random())::int4 as b, (1000 * random())::int4 as c, (1000 * random())::int4 as d from thousand;


create table r_million as select a, b, c, d from r1000, thousand;

create table LINEITEM_BIG as select * from lineitem, ten;
