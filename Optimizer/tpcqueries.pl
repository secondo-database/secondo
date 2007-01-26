%
% November 2004, M. Spiekermann
%
% Some TPC-H queries in Secondo SQL syntax notated
% as prolog facts.


tpcQuery(10, select
      [
	c_custkey,
	c_name,
	sum(l_extendedprice * (1 - l_discount)) as revenue,
	c_acctbal,
	n_name,
	c_address,
	c_phone,
	c_comment
      ]
from
      [
	customer,
	orders,
	lineitem,
	nation
      ]
where
      [
	c_custkey = o_custkey,
	l_orderkey = o_orderkey,
	not(o_orderdate < cmpdate10_1),
	o_orderdate < cmpdate10_2,
	l_returnflag = "R",
	c_nationkey = n_nationkey
      ]
groupby
      [
	c_custkey,
	c_name,
	c_acctbal,
	c_phone,
	n_name,
	c_address,
	c_comment
       ]
orderby [ revenue desc]
first 20 
).

tpcQuery(5, select
       [
	n_name,
	sum(l_extendedprice * (1 - l_discount)) as revenue
       ]
from
       [
        customer,
	orders,
	lineitem,
	supplier,
	nation,
	region
       ]
where
       [
	c_custkey = o_custkey,
	l_orderkey = o_orderkey,
	l_suppkey = s_suppkey,
	c_nationkey = s_nationkey,
	s_nationkey = n_nationkey,
	n_regionkey = r_regionkey,
        r_name = "ASIA", 
	not(o_orderdate < cmpdate5_1),
	o_orderdate < cmpdate5_2 
       ]
groupby [ n_name ]
orderby [ revenue desc ]
).


tpcQuery(3, select
	[ 
          l_orderkey,
          sum(l_extendedprice * (1 - l_discount)) as revenue,
	  o_orderdate,
	  o_shippriority 
        ]
from
	[ 
          customer,
	  orders,
	  lineitem 
        ]
where
	[
          c_mktsegment = "BUILDING", 
          c_custkey = o_custkey,
	  l_orderkey = o_orderkey 
        ]
groupby
	[ 
          l_orderkey,
	  o_orderdate,
	  o_shippriority 
        ]
orderby
	[ 
          revenue desc,
	  o_orderdate asc 
        ]
first 10
).

tpcQuery(1, select
	[ 
          count(*) as count_order,
          l_returnflag,
          l_linestatus,
          sum(l_quantity) as sum_qty,
          sum(l_extendedprice) as sum_base_price,
          sum(l_extendedprice * (1 - l_discount)) as sum_disc_price,
          sum(l_extendedprice * (1 - l_discount) * (1 + l_tax)) as sum_charge,
          avg(l_quantity) as avg_qty,
	  avg(l_extendedprice) as avg_price,
	  avg(l_discount) as avg_disc
        ]
from
	  lineitem 
where
l_shipdate < theInstant(1998,9,2)
groupby [
	  l_returnflag,
	  l_linestatus
        ] 
orderby
	[ 
          l_returnflag asc,
	  l_linestatus asc 
        ]
).

tpc(No) :- tpcQuery(No, X), sql(X).
tpcAfterLookup(No) :- tpcQuery(No, X), callLookup(X,Y), !, write(Y).

% a variant of TPC-3 which includes some correlated predicates
tpcCorrelated(1, select
	[ 
          c_nationkey,
          count(*) as sumX
        ]
from
	[ 
          customer,
	  orders,
	  lineitem 
        ]
where
	[
          l_receiptdate < (l_shipdate + create_duration(30.0)),
          l_commitdate < (l_shipdate + create_duration(30.0)),
          l_commitdate < (l_receiptdate + create_duration(30.0)),
          l_receiptdate > (theInstant(1996,1,1) + create_duration(30.0)),
          l_commitdate > (theInstant(1996,1,1) + create_duration(30.0)),
          l_shipdate >  (theInstant(1996,1,1) + create_duration(30.0)),
          l_quantity > 25, 
          c_custkey = o_custkey,
	  l_orderkey = o_orderkey 
        ]
groupby [
	  c_nationkey 
        ] 

).

tpcCorrel(N) :-
  tpcCorrelated(N, T), sql(T). 

