%
% November 2004, M. Spiekermann
%
% Some TPC-H queries in Secondo SQL syntax notated
% as prolog facts.


tpcQuery(10, select
      [
        ccustkey,
        cname,
        sum(lextendedprice * (1 - ldiscount)) as revenue,
        cacctbal,
        nname,
        caddress,
        cphone,
        ccomment
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
        ccustkey = ocustkey,
        lorderkey = oorderkey,
        not(oorderdate < theInstant(1993,10,1)),
        oorderdate < theInstant(1994,1,1),
        lreturnflag = "R",
        cnationkey = nnationkey
      ]
groupby
      [
        ccustkey,
        cname,
        cacctbal,
        cphone,
        nname,
        caddress,
        ccomment
      ]
orderby [ revenue desc]
first 20 
).

tpcQuery(5, select
       [
        nname,
        sum(lextendedprice * (1 - ldiscount)) as revenue
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
        ccustkey = ocustkey,
        lorderkey = oorderkey,
        lsuppkey = ssuppkey,
        cnationkey = snationkey,
        snationkey = nnationkey,
        nregionkey = rregionkey,
        rname = "ASIA", 
        not(oorderdate < theInstant(1994,1,1)),
        oorderdate < theInstant(1995,1,1) 
       ]
groupby [ nname ]
orderby [ revenue desc ]
).


tpcQuery(3, select
  [ 
    lorderkey,
    sum(lextendedprice * (1 - ldiscount)) as revenue,
    oorderdate,
    oshippriority 
  ]
from
	[ 
    customer,
    orders,
    lineitem
  ]
where
	[
    cmktsegment = "BUILDING", 
    ccustkey = ocustkey,
    lorderkey = oorderkey 
  ]
groupby
  [ 
    lorderkey,
    oorderdate,
    oshippriority 
  ]
orderby
  [ 
    revenue desc,
    oorderdate asc 
  ]
first 10
).

tpcQuery(1, select
	[ 
          count(*) as count_order,
          lreturnflag,
          llinestatus,
          sum(lquantity) as sum_qty,
          sum(lextendedprice) as sum_base_price,
          sum(lextendedprice * (1 - ldiscount)) as sum_disc_price,
          sum(lextendedprice * (1 - ldiscount) * (1 + ltax)) as sum_charge,
          avg(lquantity) as avg_qty,
	  avg(lextendedprice) as avg_price,
	  avg(ldiscount) as avg_disc
        ]
from
	  lineitem 
where
lshipdate < theInstant(1998,9,2)
groupby [
          lreturnflag,
          llinestatus
        ] 
orderby
	[ 
    lreturnflag asc,
    llinestatus asc 
  ]
).

tpcQuery(simple1, select
	[ 
      count(*) as count_order,
      lreturnflag,
      llinestatus,
      sum(lquantity) as sum_qty,
	    avg(ldiscount) as avg_disc
  ]
from
	  lineitem 
where
    lshipdate < theInstant(1998,9,2)
groupby [
          lreturnflag,
          llinestatus
        ] 
).


tpc(No) :- tpcQuery(No, X), sql(X).
tpcAfterLookup(No) :- tpcQuery(No, X), callLookup(X,Y), !, write(Y).

% a variant of TPC-3 which includes some correlated predicates
tpcCorrelated(1, select
	[ 
          cnationkey,
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
    lreceiptdate < (lshipdate + create_duration(30.0)),
    lcommitdate < (lshipdate + create_duration(30.0)),
    lcommitdate < (lreceiptdate + create_duration(30.0)),
    lreceiptdate > (theInstant(1996,1,1) + create_duration(30.0)),
    lcommitdate > (theInstant(1996,1,1) + create_duration(30.0)),
    lshipdate >  (theInstant(1996,1,1) + create_duration(30.0)),
    lquantity > 25, 
    ccustkey = ocustkey,
    lorderkey = oorderkey 
  ]
groupby [
          cnationkey 
        ] 

).

tpcCorrel(N) :-
  tpcCorrelated(N, T), sql(T). 

