

tpc3 :- 
  sql 
select
	[ 
          l_orderkey,
%	sum(l_extendedprice * (1 - l_discount)) as revenue,
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
%groupby
%	[ l_orderkey,
%	  o_orderdate,
%	  o_shippriority ]
orderby
	[ 
%          revenue desc,
	  o_orderdate asc 
        ].

%select count(*) from lineitem.

tpc1 :- 
  sql 
select
	[ 
%          count(*) as count_order,
          l_returnflag,
          l_linestatus,
          sum(l_quantity) as sum_qty,
          sum(l_extendedprice) as sum_base_price
%          sum(l_extendedprice * (1 - l_discount)) as sum_disc_price,
%          sum(l_extendedprice * (1 - l_discount) * (1 + l_tax)) as sum_charge,
%          avg(l_quantity) as avg_qty,
%	  avg(l_extendedprice) as avg_price,
%	  avg(l_discount) as avg_disc,
        ]
from
	  lineitem 
where
	  l_shipdate < cmpdate 
groupby [
	  l_returnflag,
	  l_linestatus
        ] 
orderby
	[ 
          l_returnflag asc,
	  l_linestatus asc 
        ].

