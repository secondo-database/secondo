tpc(d, No) :- tpcd(No, Query), setupQuery(No), sql(Query), teardownQuery(No).

tpc(d, No, Query) :- tpcd(No, Query).

executeQuery(Benchmark, No) :-
  upcase_atom(Benchmark, BMOut),  
  catch(
  ( tpc(Benchmark, No)
    -> concat_atom(['TPC-', BMOut, ' ', No, ' successful', '\n'], Result)
	;  concat_atom(['TPC-', BMOut, ' ', No, ' failed', '\n'], Result)
   ),
   _, concat_atom(['TPC-', BMOut, ' ', No, ' failed', '\n'], Result)),
   concat_atom([Benchmark, No], Key),
   ( retract(benchmarkResult(Key, _)) ; true ),
   assert(benchmarkResult(Key, Result)).
   
executeQueries(_, []).

executeQueries(Benchmark, [[No, _] | Rest]) :-
  executeQuery(Benchmark, No),
  dc(tpcd, (
  N is No + 1,
  write_list(['Execute next Query (', N, ')? y/n']),
  get_single_char(Answer),
  Answer = 121 )),
  executeQueries(Benchmark, Rest).
  
executeBenchmark(Benchmark) :- 
    retractall(benchmarkResult(_, _)),
	findall([No, Query], tpc(Benchmark, No, Query), Queries),
	executeQueries(Benchmark, Queries),
	findall(Result, benchmarkResult(_, Result), L),
	nl,
    write_list(L).
	
setupQuery(15) :-
    let(revenue, select
		[lsuppkey as supplier_no,
		sum(lextendedprice * (1 - ldiscount)) as total_revenue]
		from
		lineitem
		where
		[lshipdate >= instant("1996-01-01"),
		lshipdate < theInstant(year_of(instant("1996-01-01")), month_of(instant("1996-01-01")) + 3, day_of(instant("1996-01-01")))]
		groupby
		[lsuppkey]).
		
setupQuery(_).
		
teardownQuery(15) :-
  catch(drop_relation(revenue), _, true),
  closeDB,
  updateDB(tpcd),
  open database tpcd.
  
teardownQuery(_).

tpcd(1, select
			[lreturnflag,
			llinestatus,
			sum(lquantity) as sum_qty,
			sum(lextendedprice) as sum_base_price,
			sum(lextendedprice*(1-ldiscount)) as sum_disc_price,
			sum(lextendedprice*(1-ldiscount)*(1+ltax)) as sum_charge,
			avg(lquantity) as avg_qty,
			avg(lextendedprice) as avg_price,
			avg(ldiscount) as avg_disc,
			count(*) as count_order]
			from
			lineitem
			where
			[lshipdate <= instant("1998-09-02")]
			groupby
			[lreturnflag,
			llinestatus]
			orderby
			[lreturnflag,
			llinestatus]).
			
tpcd(2, select
		[sacctbal,
		sname,
		nname,
		ppartkey,
		pmfgr,
		saddress,
		sphone,
		scomment]
		from
		[part,
		supplier,
		partsupp,
		nation,
		region]
		where
		[ppartkey = pspartkey,
		ssuppkey = pssuppkey,
% substitution parameter		
		 psize = 15,
% substitution parameter 
%        ptype like "%BRASS"
		 ptype contains "BRASS",
		 snationkey = nnationkey,
		 nregionkey = rregionkey,
% substitution parameter		
		 rname = "EUROPE",
		 pssupplycost = (
			select
			min(pssupplycost)
			from
			[partsupp, supplier,
			nation, region]
			where
			[ppartkey = pspartkey,
			 ssuppkey = pssuppkey,
			 snationkey = nnationkey,
			 nregionkey = rregionkey,
	% substitution parameter		
			 rname = "EUROPE"]
		)]

		orderby[
		sacctbal desc,
		nname,
		sname,
		ppartkey] first 100).
		
tpcd(3, select
		[lorderkey,
		sum(lextendedprice*(1-ldiscount)) as revenue,
		oorderdate,
		oshippriority]
		from
		[customer,
		orders,
		lineitem]
		where
% substitution parameter		
		[cmktsegment = "BUILDING",
		ccustkey = ocustkey,
		lorderkey = oorderkey,
% substitution parameter		
		oorderdate < instant("1995-03-15"),
% substitution parameter		
		lshipdate > instant("1995-03-15")]
		groupby
		[lorderkey,
		oorderdate,
		oshippriority]
		orderby
		[revenue desc,
		oorderdate] first 10).
		
tpcd(4, select
		[oorderpriority,
		count(*) as ordercount]
		from orders
		where
% substitution parameter				
		[oorderdate >= instant("1993-07-01"),
% substitution parameter				
		oorderdate < theInstant(year_of(instant("1993-07-01")), month_of(instant("1993-07-01")) + 3, day_of(instant("1993-07-01"))),
		exists(
		select
		*
		from
		lineitem
		where
		[lorderkey = oorderkey,
		lcommitdate < lreceiptdate])
		]
		groupby
		[oorderpriority]
		orderby
		[oorderpriority]).
		
tpcd(5, select
		[nname,
		sum(lextendedprice * (1 - ldiscount)) as revenue]
		from
		[customer,
		orders,
		lineitem,
		supplier,
		nation,
		region]
		where
		[ccustkey = ocustkey,
		oorderkey = lorderkey,
		lsuppkey = ssuppkey,
		cnationkey = snationkey,
		snationkey = nnationkey,
		nregionkey = rregionkey,
% substitution parameter				
		rname = "ASIA",
% substitution parameter				
		oorderdate >= instant("1994-01-01"),
% substitution parameter				
		oorderdate < theInstant(year_of(instant("1994-01-01")) + 1, month_of(instant("1994-01-01")) , day_of(instant("1994-01-01")))]
		groupby
		[nname]
		orderby
		[revenue desc]).
		
tpcd(6, select
		[sum(lextendedprice*ldiscount) as revenue]
		from
		lineitem
		where
% substitution parameter		
		[lshipdate >= instant("1994-01-01"),
% substitution parameter	
		lshipdate < theInstant(year_of(instant("1994-01-01")) + 1, month_of(instant("1994-01-01")) , day_of(instant("1994-01-01"))),
% substitution parameter	
		between(ldiscount, 0.06 - 0.01, 0.06 + 0.01),
% substitution parameter	
		lquantity < 24]
		groupby[]).
		
tpcd(7, select
		[supp_nation,
		cust_nation,
		lyear, sum(volume) as revenue]
		from (
			select
			[n1:nname as supp_nation,
			n2:nname as cust_nation,
			year_of(lshipdate) as lyear,
			lextendedprice * (1 - ldiscount) as volume]
			from
			[supplier,
			lineitem,
			orders,
			customer,
			nation as n1,
			nation as n2]
			where
			[ssuppkey = lsuppkey,
			oorderkey = lorderkey,
			ccustkey = ocustkey,
			snationkey = n1:nnationkey,
			cnationkey = n2:nnationkey,
% substitution parameter			
			(n1:nname = "FRANCE" and n2:nname = "GERMANY")
% substitution parameter			
			or (n1:nname = "GERMANY" and n2:nname = "FRANCE"),
			between(instant2real(lshipdate), instant2real(instant("1995-01-01")), instant2real(instant("1996-12-31")))]
		) as shipping
		groupby
		[supp_nation,
		cust_nation,
		lyear]
		orderby
		[supp_nation,
		cust_nation,
		lyear]).
		
tpcd(8, select
		[oyear,
/*		sum(case
		when nation = "BRAZIL"
		then volume
		else 0
		end) / sum(volume) as mktshare */
		(aggregate((ifthenelse(nation = "BRAZIL", volume, 0.0)), (+), 'real', '[const real value 0.0]') / sum(volume)) as mktshare
		]
		from (
			select
			[year_of(oorderdate) as oyear,
			lextendedprice * (1-ldiscount) as volume,
			n2:nname as nation]
			from
			[part,
			supplier,
			lineitem,
			orders,
			customer,
			nation as  n1,
			nation as n2,
			region]
			where
			[ppartkey = lpartkey,
			ssuppkey = lsuppkey,
			lorderkey = oorderkey,
			ocustkey = ccustkey,
			cnationkey = n1:nnationkey,
			n1:nregionkey = rregionkey,
% substitution parameter			
			rname = "AMERICA",
			snationkey = n2:nnationkey,
			between(instant2real(oorderdate), instant2real(instant("1995-01-01")),  instant2real(instant("1996-12-31"))),
% substitution parameter			
			ptype = "ECONOMY ANODIZED STEEL"]
		) as allnations
		groupby
		[oyear]
		orderby
		[oyear]).
		
tpcd(9, select
		[nation,
		oyear,
		sum(amount) as sumprofit]
		from (
			select
			[nname as nation,
			year_of(oorderdate) as oyear,
			lextendedprice * (1 - ldiscount) - pssupplycost * lquantity as amount]
			from
			[part,
			supplier,
			lineitem,
			partsupp,
			orders,
			nation]
			where
			[ssuppkey = lsuppkey,
			pssuppkey = lsuppkey,
			pspartkey = lpartkey,
			ppartkey = lpartkey,
			oorderkey = lorderkey,
			snationkey = nnationkey,
% substitution parameter			
			pname contains "green"]
		) as profit
		groupby
		[nation,
		oyear]
		orderby
		[nation,
		oyear desc]).
		
		
tpcd(10, select
		[ccustkey,
		cname,
		sum(lextendedprice * (1 - ldiscount)) as revenue,
		cacctbal,
		nname,
		caddress,
		cphone,
		ccomment]
		from
		[customer,
		orders,
		lineitem,
		nation]
		where
		[ccustkey = ocustkey,
		lorderkey = oorderkey,
		oorderdate >= instant("1993-10-01"),
		oorderdate < theInstant(year_of(instant("1993-10-01")), month_of(instant("1993-10-01")) + 3, day_of(instant("1993-10-01"))),
		lreturnflag = "R",
		cnationkey = nnationkey]
		groupby
		[ccustkey,
		cname,
		cacctbal,
		cphone,
		nname,
		caddress,
		ccomment]
		orderby
		[revenue desc] first 20).
		
tpcd(11, select
		[pspartkey,
		(sum(pssupplycost * psavailqty)) as value]
		from
		[partsupp,
		supplier,
		nation]
		where
		[pssuppkey = ssuppkey,
		snationkey = nnationkey,
% substitution parameter		
		nname = "GERMANY"]
		groupby
		[pspartkey] 
/*		having
		[sum(pssupplycost * psavailqty) > (
				select
	% substitution parameter				
				[sum(pssupplycost * psavailqty * 0.0001)]
				from
				[partsupp,
				supplier,
				nation]
				where
				[pssuppkey = ssuppkey,
				snationkey = nnationkey,
				nname = "GERMANY"]
			)
		]*/
		orderby
		[value desc]).
		
tpcd(12, select
		[lshipmode,
/*		
		sum(case
		when oorderpriority ="1-URGENT"
		or oorderpriority ="2-HIGH"
		then 1
		else 0
		end) as highlinecount,
		sum(case
		when oorderpriority <> "1-URGENT"
		and oorderpriority <> "2-HIGH"
		then 1
		else 0
		end) as lowlinecount
*/		
		aggregate((ifthenelse(oorderpriority = "1-URGENT" or oorderpriority = "2-HIGH", 1, 0)), (+), 'int', '[const int value 0]') as highlinecount,
		aggregate((ifthenelse(not(oorderpriority = "1-URGENT") and not(oorderpriority = "2-HIGH"), 1, 0)), (+), 'int', '[const int value 0]') as lowlinecount		
		]
		from
		[orders,
		lineitem]
		where
		[oorderkey = lorderkey,
% substitution parameter
		lshipmode in ("MAIL", "SHIP"),
%        lshipmode = "MAIL" or lshipmode = "SHIP",
		lcommitdate < lreceiptdate,
		lshipdate < lcommitdate,
% substitution parameter		
		lreceiptdate >= instant("1994-01-01"),
% substitution parameter		
		lreceiptdate < theInstant(year_of(instant("1994-01-01")) + 1, month_of(instant("1994-01-01")), day_of(instant("1994-01-01")))]
		groupby
		[lshipmode]
		orderby
		[lshipmode]).
		

tpcd(13, _).		
/* 
tpcd(13, select
		[ccount, count(*) as custdist]
		from (
			select
			[ccustkey,
			count(oorderkey)]
			from
			customer left outer join orders on
			[ccustkey = ocustkey
			and ocomment not like "%[WORD1]%[WORD2]%"]
			groupby
			[ccustkey]
		)as corders (ccustkey, ccount)
		groupby
		[ccount]
		orderby
		[custdist desc,
		ccount desc]). 
*/
		
tpcd(14, select
/*		[100.00 * sum(case
		when ptype like "PROMO%"
		then lextendedprice*(1-ldiscount)
		else 0
		end) / sum(lextendedprice * (1 - ldiscount)) as promo_revenue]
*/
		[(aggregate(((ifthenelse(ptype starts "PROMO", lextendedprice*(1-ldiscount), 0)) * 100.0), (+), 'real', '[const real value 0.0]') / sum(lextendedprice * (1 - ldiscount))) as promo_revenue]
		from
		[lineitem,
		part]
		where
		[lpartkey = ppartkey,
		lshipdate >= instant("1995-09-01"),
		lshipdate < theInstant(year_of(instant("1995-09-01")), month_of(instant("1995-09-01")) + 1, day_of(instant("1995-09-01")))]
		groupby[]).
		
tpcd(15, % Query) :-
/*  create view revenue[STREAM_ID] (supplier_no, total_revenue) as
select
l_suppkey,
sum(l_extendedprice * (1 - l_discount))
from
lineitem
where
l_shipdate >= date "[DATE]"
and l_shipdate < date "[DATE]" + interval "3" month
group by
l_suppkey;*/
select
		[ssuppkey,
		sname,
		saddress,
		sphone,
		total_revenue]
		from
		[supplier,
		revenue/*[STREAMID]*/]
		where
		[ssuppkey = supplier_no,
		total_revenue = (
			select
			max(total_revenue)
			from
			revenue/*[STREAMID]*/
		)]
		orderby
		[ssuppkey])
/* drop view revenue[STREAM_ID]; */.

tpcd(16, select
		[pbrand,
		ptype,
		psize,
		count(distinct pssuppkey) as suppliercnt]
		from
		[partsupp,
		part]
		where
		[ppartkey = pspartkey,
% substitution parameter		
		not(pbrand = "Brand#45"),
% substitution parameter		
		not(ptype starts "MEDIUM POLISHED"),
% substitution parameters		
		psize in (49, 14, 23, 45, 19, 3, 36, 9),
		pssuppkey not in(
			select
			ssuppkey
			from
			supplier
			where
%			scomment like "%Customer%Complaints%"
            [scomment contains "Customer",
			scomment contains "Complaints"]
		)]
		groupby
		[pbrand,
		ptype,
		psize]
		orderby
		[suppliercnt desc,
		pbrand,
		ptype,
		psize]).
		
tpcd(17, select
		[sum(lextendedprice / 7.0) as avg_yearly]		
		from
		[lineitem,
		part]
		where
		[ppartkey = lpartkey,
		pbrand = "Brand#23",
		pcontainer = "MED BOX",
		lquantity < (
			select
			avg(0.2 * lquantity)
			from
			[lineitem]
			where
			[lpartkey = ppartkey]
		)]
		groupby
		[]).		
		
tpcd(18, select
		[cname,
		ccustkey,
		oorderkey,
		oorderdate,
		ototalprice,
		sum(lquantity) as quant]
		from
		[customer,
		orders,
		lineitem]
		where
		[oorderkey in (
			select
			lorderkey
			from
			lineitem
			/* groupby
			[lorderkey having
% substitution parameter			
			sum(lquantity) > 300] */
		),
		ccustkey = ocustkey,
		oorderkey = lorderkey]
		groupby
		[cname,
		ccustkey,
		oorderkey,
		oorderdate,
		ototalprice]
		orderby
		[ototalprice desc,
		oorderdate]).
		
tpcd(19, select
		[(sum(lextendedprice * (1 - ldiscount) )) as revenue]
		from
		[lineitem,
		part]
		where
		[((
			(
				(
					(	
						(
							(
								(
									(
										(ppartkey = lpartkey)
% substitution parameter										
										and (pbrand = "Brand#12")
									)									
									and (pcontainer in ( "SM CASE", "SM BOX", "SM PACK", "SM PKG"))
%									and (((pcontainer = "SM CASE" or pcontainer =  "SM BOX") or pcontainer =  "SM PACK") or pcontainer = "SM PKG")
								)
% substitution parameter								
								and (lquantity >= 1)
							) 
% substitution parameter							
							and (lquantity <= 1 + 10)
						)
						and (between(psize, 1, 5))
					)
					and (lshipmode in ("AIR", "AIR REG"))
%					and (lshipmode = "AIR" or lshipmode =  "AIR REG")
				)
			and (lshipinstruct = "DELIVER IN PERSON")
			)
		)
		or
		(
			(
				(
					(	
						(
							(
								(
									(
										(ppartkey = lpartkey)
% substitution parameter										
										and (pbrand = "Brand#23")
									)
									and (pcontainer in ("MED BAG", "MED BOX", "MED PKG", "MED PACK"))
%									and (((pcontainer = "MED BAG" or pcontainer = "MED BOX") or pcontainer = "MED PKG") or pcontainer = "MED PACK")
								)
% substitution parameter								
								and (lquantity >= 10)
							) 
% substitution parameter							
							and (lquantity <= 10 + 10)
						)
						and (between(psize, 1, 10))
					)
					and (lshipmode in ("AIR", "AIR REG"))
%					and (lshipmode = "AIR" or lshipmode = "AIR REG")
				)
			and (lshipinstruct = "DELIVER IN PERSON")
			)		
		))
		or
		(
			(
				(
					(	
						(
							(
								(
									(
										(ppartkey = lpartkey)
% substitution parameter										
										and (pbrand = "Brand#34")
									)
									and (pcontainer in ( "LG CASE", "LG BOX", "LG PACK", "LG PKG"))
%									and (((pcontainer = "LG CASE" or pcontainer = "LG BOX") or pcontainer = "LG PACK") or pcontainer = "LG PKG")
								)
% substitution parameter								
								and (lquantity >= 20)
							) 
% substitution parameter							
							and (lquantity <= 20 + 10)
						)
						and (between(psize, 1, 15))
					)
					and (lshipmode in ("AIR", "AIR REG"))
%					and (lshipmode = "AIR" or lshipmode = "AIR REG")
				)
			and (lshipinstruct = "DELIVER IN PERSON")
			)			
		)]
		groupby[]).	

tpcd(20, select
		[sname,
		saddress]
		from
		[supplier, nation]
		where
		[ssuppkey in (
			select
			[pssuppkey]
			from
			[partsupp]
			where
			[pspartkey in (
				select
				ppartkey
				from
				part
				where
%				pname like ’[COLOR]%’
% substitution parameter
				tostring(pname) starts "forest"
			),
			psavailqty > (
				select
				sum(lquantity * 0.5)
				from
				lineitem
				where
				[lpartkey = pspartkey,
				lsuppkey = pssuppkey,
% substitution parameter				
				lshipdate >= instant("1994-01-01"),
% substitution parameter				
				lshipdate < theInstant(year_of(instant("1994-01-01")) + 1, month_of(instant("1994-01-01")), day_of(instant("1994-01-01")))]
			)]
		),
		snationkey = nnationkey,
% substitution parameter		
		nname = "CANADA"]
		orderby
		[sname]).		
		

tpcd(21, select
		[sname,
		count(*) as numwait]
		from
		[supplier,
		lineitem as l1,
		orders,
		nation]
		where
		[ssuppkey = l1:lsuppkey,
		oorderkey = l1:lorderkey,
		oorderstatus = "F",
		l1:lreceiptdate > l1:lcommitdate,
		exists(
			select
			*
			from
			lineitem as l2
			where
			[l2:lorderkey = l1:lorderkey,
			not(l2:lsuppkey = l1:lsuppkey)]
		),
		not(exists(
			select
			*
			from
			lineitem as l3
			where
			[l3:lorderkey = l1:lorderkey,
			not(l3:lsuppkey = l1:lsuppkey),
			l3:lreceiptdate > l3:lcommitdate]
		)),
		snationkey = nnationkey,
% substitution parameter		
		nname = "SAUDI ARABIA"]
		groupby
		[sname]
		orderby
		[numwait desc,
		sname] 
		first 100).
		
tpcd(22, select
		[cntrycode,
		count(*) as numcust,
		sum(cacctbal) as totacctbal]
		from (
			select
			[substr(cphone, 1, 2) as cntrycode,
			cacctbal]
			from
			customer
			where
			[substr(cphone, 1, 2) in
% substitution parameters			
			("13","35","31","23","29","30","18"),
			cacctbal > (
				select
				avg(cacctbal)
				from
				customer
				where
				[cacctbal > 0.00,
				substr(cphone, 1, 2) in
% substitution parameter				
				("13","35","31","23","29","30","18")]
			),
			not( exists(
				select
				*
				from
				orders
				where
				ocustkey = ccustkey
			))]
		) as custsale
		groupby
		[cntrycode]
		orderby
		[cntrycode]).