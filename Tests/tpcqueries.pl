% This file is part of SECONDO.
%
% Copyright (C) 2004, University in Hagen, Department of Computer Science, 
% Database Systems for New Applications.
%
% SECONDO is free software%  you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation%  either version 2 of the License, or
% (at your option) any later version.
%
% SECONDO is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY%  without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with SECONDO%  if not, write to the Free Software
% Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
%
%
% November 2004, M. Spiekermann
%
% Some TPC-H queries in Secondo SQL syntax


tpc5 :-
  sql
select
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
groupby [ n_name asc ]
orderby [ revenue desc ].


tpc3 :- 
  sql 
select
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
        ].

%select count(*) from lineitem.

tpc1 :- 
  sql 
select
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
	  l_shipdate < cmpdate1 
groupby [
	  l_returnflag,
	  l_linestatus
        ] 
orderby
	[ 
          l_returnflag asc,
	  l_linestatus asc 
        ].


