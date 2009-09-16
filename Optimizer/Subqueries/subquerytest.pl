myDebug(Command) :-
  leash(-all),
  trace,
  Command,
  leash(+all).

testSubqueries :-
  ( testSubqueries1 -> true ; write('\n\t\t\t\t\t\tTest 1 failed')),
  ( testSubqueries2 -> true ; write('\n\t\t\t\t\t\tTest 2 failed')),
  ( testSubqueries3 -> true ; write('\n\t\t\t\t\t\tTest 3 failed')),
  ( testSubqueries4 -> true ; write('\n\t\t\t\t\t\tTest 4 failed')).

testSubqueries1 :-
  optimize(select * from staedte as s where s:sname = "Hagen", A, _),
   B = 'Staedte  feed {s}  filter[(.SName_s = "Hagen")] {',
   sub_atom(A, 0, _, _, B).

testSubqueries2 :-
  optimize(select * from staedte where plz = (select max(plz) from plz), A, _),
  B = 'Staedte  feed  filter[(.PLZ = 99998)] {',
  sub_atom(A, 0, _, _, B).

testSubqueries3 :-
  optimize(select * from plz as p where p:ort in (select sname from staedte), A, _),
  write('\nA: '), write(A),
  B = 'Staedte  feed project[SName]  loopjoin[plz_Ort_btree plz  exactmatch[.SName] {',
  sub_atom(A, 0, _, _, B),
  C = ' project[Ort, PLZ] {p} ] project[PLZ_p, Ort_p]  consume ',
  sub_atom(A, _, _, _, C).

testSubqueries4 :-
  transform(select * from staedte where sname in (select ort from plz where plz > 5000), A),
  term_to_atom(A, C),
  B = 'select[sname, bev, plz, vorwahl, kennzeichen]from[staedte, plz]where[sname=ort, plz>5000]',
  sub_atom(C, 0, _, _, B).

testSubqueries5 :-
  transform(select * from staedte where plz in (select max(plz) from plz where ort = sname), Res),
  write('\nRes: '), write(Res).

testSubqueries6 :-
  sql(select pnum from parts where qoh = (select count(s:shipdate) from supply as s where [pnum = s:pnum, s:shipdate < 19800101])).

testSubqueries7 :-
  transformNestedPredicate(pnum, _, parts, _, qoh= (select count(s:shipdate)from supply as s where[pnum=s:pnum, s:shipdate<19800101]), _).


ttt :-
secondo('query SEC2CACHEINFO feed count', A),
write(A), nl,
secondo('query SEC2COMMANDS feed count', B),
write(B), nl,
secondo('query SEC2COUNTERS feed count', C),
write(C), nl,
secondo('query SEC2FILEINFO feed count', D),
write(D), nl,
secondo('query SEC2OPERATORINFO feed count', E),
write(E), nl,
secondo('query SEC2OPERATORUSAGE feed count', F),
write(F), nl,
secondo('query SEC2PJOIN feed count', G),
write(G), nl,
secondo('query SEC2PJOINCOST feed count', H),
write(H), nl,
secondo('query SEC2TYPEINFO feed count', I),
write(I), nl.

berlinmod(7, select [pp:pos as pos, v1:licence as licence]
from [datascar as v1, querypoints as pp]
where [v1:trip passes pp:pos, v1:type = "passenger",
  inst(initial(v1:trip at pp:pos)) <= (all
  (select inst(initial(v2:trip at pp:pos))
   from [datascar as v2]
   where [v2:trip passes pp:pos, v2:type = "passenger"]))
]).

ganski(1, select sname from s where sno in (select sno from sp where no = 'P2')).
ganski(2, select sno from sp where pno = (select max(pno) from p)).
ganski(3, select sno from sp where pno in (select pno from p where weight > 50)).
ganski(4, select sname from s where sno in (select sno from sp where [qty > 100, sporigin = scity])).
ganski(5, select pname from p where pno = (select max(pno) from sp where sporigin = pcity)).
kiessling(2, select pnum from parts where qoh = (select count(shipdate) from supply where [supply:pnum = parts:pnum, shipdate < 19800101])).
kiessling(5, select pnum from parts where qoh = (select max(quan) from supply where [supply:pnum < parts:pnum, shipdate < 19800101])).
kim(4, select sno from sp where pno = (select max(pno) from p where price > 25)).
kim(5, select sno from sp where pno in (select pno from p where price > 25)).
kim(6, select sno from sp where pno in (select pno from pj where [sp:sno = pj:jno, jloc = 'NEW YORK'])).
kim(7, select sno from sp where pno = (select max(pno) from pj where [pj:jno = sp:jno, jloc = 'NEW YORK'])).
kim(10, select sname from s where sno not in(select sno from sp where [pno = 'P1'])).

testQuery(1, select * from orte as o where o:ort in (select ort from plz where plz < 10000) first 10).
testQuery(2, select * from orte as o where o:ort in (select ort from plz where plz > o:bevt) first 10).
testQuery(3, select * from [orte as o, staedte as s] where [o:ort = s:sname, o:ort in (select p:ort from plz as p where p:ort = s:sname)] first 10).
testQuery(4, select * from orte as o where o:ort not in(select ort from plz where plz > 10000) first 10).
testQuery(5, select * from orte as o where o:ort not in(select ort from plz where plz < o:bevt) first 10).
testQuery(6, select * from [orte as o, staedte as s] where [o:ort = s:sname, o:ort not in(select p:ort from plz as p where p:ort = s:sname)] first 10).
testQuery(7, select * from orte as o where [exists(select ort from plz where plz < o:bevt)] first 10).
testQuery(8, select * from [orte as o, staedte as s] where [o:ort = s:sname, exists(select p:ort from plz as p where [p:ort = s:sname, s:bev > o:bevt])] first 10).
testQuery(9, select * from orte as o where [not(exists(select ort from plz where plz < o:bevt))] first 10).
testQuery(10, select * from [orte as o, staedte as s] where [o:ort = s:sname, not(exists(select p:ort from plz as p where [p:ort = s:sname, s:bev > o:bevt]))] first 10).
testQuery(11, select * from orte as o where o:bevt <= all(select bev from staedte where plz > 8699) first 10).
testQuery(12, select * from orte as o where o:bevt <= all(select bev from staedte where o:ort > sname) first 10).
testQuery(13, select * from [orte as o, plz as p] where o:bevt <= all(select bev from staedte where [o:ort > sname, p:ort = sname]) first 10).
testQuery(14, select * from orte as o where o:bevt < all(select bev from staedte where plz > 8699) first 10).
testQuery(15, select * from orte as o where o:bevt < all(select bev from staedte where o:ort > sname) first 10).
testQuery(16, select * from [orte as o, plz as p] where o:bevt < all(select bev from staedte where [o:ort > sname, p:ort = sname]) first 10).
testQuery(17, select * from orte as o where o:bevt >= all(select bev from staedte where plz > 8699) first 10).
testQuery(18, select * from orte as o where o:bevt >= all(select bev from staedte where o:ort > sname) first 10).
testQuery(19, select * from [orte as o, plz as p] where o:bevt >= all(select bev from staedte where [o:ort > sname, p:ort = sname]) first 10).
testQuery(20, select * from orte as o where o:bevt > all(select bev from staedte where plz > 8699) first 10).
testQuery(21, select * from orte as o where o:bevt > all(select bev from staedte where o:ort > sname) first 10).
testQuery(22, select * from [orte as o, plz as p] where o:bevt > all(select bev from staedte where [o:ort > sname, p:ort = sname]) first 10).
testQuery(23, select * from orte as o where o:bevt <= any(select bev from staedte where plz > 8699) first 10).
testQuery(24, select * from orte as o where o:bevt <= any(select bev from staedte where o:ort > sname) first 10).
testQuery(25, select * from [orte as o, plz as p] where o:bevt <= any(select bev from staedte where [o:ort > sname, p:ort = sname]) first 10).
testQuery(26, select * from orte as o where o:bevt < any(select bev from staedte where plz > 8699) first 10).
testQuery(27, select * from orte as o where o:bevt < any(select bev from staedte where o:ort > sname) first 10).
testQuery(28, select * from [orte as o, plz as p] where o:bevt < any(select bev from staedte where [o:ort > sname, p:ort = sname]) first 10).
testQuery(29, select * from orte as o where o:bevt = any(select bev from staedte where plz > 8699) first 10).
testQuery(30, select * from orte as o where o:bevt = any(select bev from staedte where o:ort > sname) first 10).
testQuery(31, select * from [orte as o, plz as p] where o:bevt = any(select bev from staedte where [o:ort > sname, p:ort = sname]) first 10).
testQuery(32, select * from orte as o where o:bevt >= any(select bev from staedte where plz > 8699) first 10).
testQuery(33, select * from orte as o where o:bevt >= any(select bev from staedte where o:ort > sname) first 10).
testQuery(34, select * from [orte as o, plz as p] where o:bevt >= any(select bev from staedte where [o:ort > sname, p:ort = sname]) first 10).
testQuery(35, select * from orte as o where o:bevt > any(select bev from staedte where plz > 8699) first 10).
testQuery(36, select * from orte as o where o:bevt > any(select bev from staedte where o:ort > sname) first 10).
testQuery(37, select * from [orte as o, plz as p] where o:bevt = (select max(bev*1000) from staedte where [o:ort > sname, p:ort = sname]) first 10).
testQuery(38, select * from orte as o where o:bevt = (select max(bev/1000) from staedte where plz > 8699) first 10).
testQuery(39, select * from orte as o where o:bevt = (select max(bev/1000) from staedte where o:ort > sname) first 10).
testQuery(40, select * from [orte as o, plz as p] where o:bevt = (select max(bev/1000) from staedte where [o:ort > sname, p:ort = sname]) first 10).
testQuery(41, select * from orte as o where o:bevt = (select min(bev/1000) from staedte where plz > 8699) first 10).
testQuery(42, select * from orte as o where o:bevt = (select min(bev/1000) from staedte where o:ort > sname) first 10).
testQuery(43, select * from [orte as o, plz as p] where o:bevt = (select min(bev/1000) from staedte where [o:ort > sname, p:ort = sname]) first 10).
testQuery(44, select * from orte as o where o:bevt = (select avg(bev/1000) from staedte where plz > 8699) first 10).
testQuery(45, select * from orte as o where o:bevt = (select avg(bev/1000) from staedte where o:ort > sname) first 10).
testQuery(46, select * from [orte as o, plz as p] where o:bevt = (select avg(bev/1000) from staedte where [o:ort > sname, p:ort = sname]) first 10).
testQuery(47, select * from plz as p1 where p1:plz = (select max(plz) from plz where ort = p1:ort) first 10).

regression(1) :-
    ( cdb ; true ),
	odb(opt),
	clear(streamName),
	clear(streamRel),
	retractall(selectivityQuery(_)),
	streamRel(rel(orte, *)),
	streamRel(rel(staedte, s)),
	streamName(txx1),
	transformCorrelatedPreds([rel(plz, p)], txx1, [pr(attr(p:ort, 1, u)=attr(s:sName, 2, u), rel(plz, p), rel(staedte, s)), pr(attr(s:bev, 1, u)>attr(bevT, 2, u), rel(staedte, s), rel(orte, *))], Result),
	Result= [pr(attr(p:ort, 1, u)=attr(s:sName, 2, u), rel(plz, p), rel(staedte, s)), pr(attr(s:bev, 1, u)>attribute(txx1, attrname(attr(bevT, 2, u))), rel(staedte, s), rel(orte, *))],
	cdb.

regression(2) :-
    ( cdb ; true ),
	odb(opt),
	clear(streamName),
	clear(streamRel),
	retractall(selectivityQuery(_)),
	streamRel(rel(orte, *)),
	streamRel(rel(staedte, s)),
	streamName(txxrel35),
	transformCorrelatedPreds([rel(plz, p)], txxrel35, [pr(attr(s:bev, 1, u)>attr(bevT, 2,u), rel(staedte, s), rel(orte, *))], Result),
	Result = [pr(attribute(txxrel35, attrname(attr(s:bev, 1, u)))>attribute(txxrel35, attrname(attr(bevT, 2, u))), rel(staedte, s), rel(orte, *))],
	cdb.

regression(3) :-
    ( cdb ; true ),
	odb(tpcd),
	clear(streamName),
	clear(streamRel),
	retractall(selectivityQuery(_)),
	streamRel(rel(partsupp, *)),
	streamRel(rel(part, *)),
	streamName(txx1),
	transformAttributeExpr(attr(psSUPPLYCOST, 1, l), txx1, 1, Result, []),
	Result = attribute(txx1, attrname(attr(psSUPPLYCOST, 1, l))),
	cdb.

/*
OuterJoinQuery:
temprel1  feed  temprel2  feed  sortmergejoin[pnum , pnum_s] {0.333, 71.4444}  temprel1 feed  extend[pnum_s: real2int(1/0), quan_s: real2int(1/0), shipdate_
s: real2int(1/0)]filter[fun(outerjoin1: TUPLE) temprel2 feed filter[not(.pnum_s = attr(outerjoin1, pnum_s))] count = temprel2 feed count] sort rdup mergeunion temprel2  fee
d  extend[pnum: real2int(1/0)] filter[fun(outerjoin2: TUPLE) temprel1 feed filter[not(.pnum = attr(outerjoin2, pnum_s))] count = temprel1 feed count] project[pnum, pnum_s,
quan_s, shipdate_s]  sort rdup mergeunion sortby[pnum asc] groupby[pnum; Var3: group feed filter[not(isempty(.shipdate_s))]  count ]project[pnum, Var3] consume

/ *

query <Rel1> feed extend[<NewJoinCol>: <correctly typed undef>] filter [fun(var1: TUPLE) <Rel2> feed filter [not(.<JoinAttr> = attr(var1, <JoinAttr>))] count = <Rel2> feed count] consume

select[n1:nname as supp_nation, n2:nname as cust_nation, year_of(lshipdate)as lyear, lextendedprice* (1-ldiscount)as volume]from[supplier, lineitem, orders, customer, nation as n1, nation as n2]where[ssuppkey=lsuppkey, oorderkey=lorderkey, ccustkey=ocustkey, snationkey=n1:nnationkey, cnationkey=n2:nnationkey, (n1:nname=[70, 82, 65, 78, 67, 69]and n2:nname=[71, 69, 82, 77, 65, 78, 89])or (n1:nname=[71, 69, 82, 77, 65, 78, 89]and n2:nname=[70, 82, 65, 78, 67, 69]), between(instant2real(lshipdate), instant2real(instant([49, 57, 57, 53, 45, 48, 49, 45, 48, 49])), instant2real(instant([49, 57, 57, 54, 45, 49, 50, 45, 51, 49])))]
*/




 /*
 trace(lookupPred, +all), trace(lookupPreds, +all), tpcd(2, Q), callLookup(Q, Q2).
lookupPred(pssupplycost= (select min(pssupplycost)from[partsupp, supplier, nation, region]where[ppartkey=pspartkey, ssuppkey=pssuppkey, snationkey=nnationkey,nregionkey=rregionkey, rname=[69, 85, 82, 79, 80, 69]]), _G3590)

lookupPred1(select min(pssupplycost)from[partsupp, supplier, nation, region]where[ppartkey=pspartkey, ssuppkey=pssuppkey, snationkey=nnationkey, nregionkey=rregionkey, rname=[69, 85, 82, 79, 80, 69]], _G2037, [rel(partsupp, *)], _L300)


 getTime(
	(
		pog([rel(part, *), rel(supplier, *), rel(partsupp, *), rel(nation, *), rel(region, *)],
		   [
			pr(attr(pPARTKEY, 1, l)=attr(psPARTKEY, 2, l), rel(part, *), rel(partsupp, *)),
		    pr(attr(sSUPPKEY, 1, l)=attr(psSUPPKEY, 2, l), rel(supplier, *), rel(partsupp, *)),
			pr(attr(pSIZE, 1, l)=15, rel(part, *)),
			pr(attr(pTYPE, 1, l)contains[66, 82, 65, 83, 83], rel(part, *)),
			pr(attr(sNATIONKEY, 1, l)=attr(nNATIONKEY, 2, l), rel(supplier, *), rel(nation, *)),
			pr(attr(nREGIONKEY, 1, l)=attr(rREGIONKEY, 2, l),rel(nation, *), rel(region, *)),
			pr(attr(rNAME, 1, l)=[69, 85, 82, 79, 80, 69], rel(region, *)),
			pr(attr(psSUPPLYCOST, 1, l)= (
				select min(attr(psSUPPLYCOST, 0, l))
					from[rel(partsupp, *), rel(supplier, *), rel(nation, *), rel(region, *)]
					where[
						  pr(attr(pPARTKEY, 1, l)=attr(psPARTKEY, 2, l), rel(part, *), rel(partsupp, *)),
					      pr(attr(sSUPPKEY, 1, l)=attr(psSUPPKEY, 2, l), rel(supplier, *), rel(partsupp, *)),
						  pr(attr(sNATIONKEY, 1, l)=attr(nNATIONKEY, 2, l), rel(supplier, *), rel(nation, *)),
						  pr(attr(nREGIONKEY, 1, l)=attr(rREGIONKEY, 2, l), rel(nation, *), rel(region, *)),
						  pr(attr(rNAME, 1, l)=[69, 85, 82, 79, 80, 69], rel(region, *))]),
			[rel(partsupp, *), rel(partsupp, *), rel(supplier, *), rel(nation, *), rel(region, *)])
			], Nodes, Edges),
		assignCosts,
		bestPlan(Plan, Cost),
		!
	),
	Result)

[debug] 16 ?- myDebug(pog([rel(supplier, *), rel(partsupp, *), rel(nation, *), rel(region, *)],
   [
pr(attr(psSUPPLYCOST, 1, l)= (
select min(attr(psSUPPLYCOST, 0, l))
from[rel(partsupp, *), rel(supplier, *), rel(nation, *), rel(region, *)]
where[
  pr(attr(pPARTKEY, 1, l)=attr(psPARTKEY, 2, l), rel(part, *), rel(partsupp, *)),
      pr(attr(sSUPPKEY, 1, l)=attr(psSUPPKEY, 2, l), rel(supplier, *), rel(partsupp, *)),
  pr(attr(sNATIONKEY, 1, l)=attr(nNATIONKEY, 2, l), rel(supplier, *), rel(nation, *)),
  pr(attr(nREGIONKEY, 1, l)=attr(rREGIONKEY, 2, l), rel(nation, *), rel(region, *)),
  pr(attr(rNAME, 1, l)=[69, 85, 82, 79, 80, 69], rel(region, *))]),
[rel(partsupp, *), rel(partsupp, *), rel(supplier, *), rel(nation, *), rel(region, *)])
], Nodes, Edges)).

query plz feed {p} Orte feed {o}
symmjoin[fun(t1:TUPLE, t2:TUPLE2) attr(t1,PLZ_p) =
  plz feed filter[.Ort contains "x"] filter[.PLZ > 50000]
    filter[.Ort = attr(t2, Ort_o)]
    max[PLZ]
]
count


myDebug(sql select [pp:pos as pos, v1:licence as licence]
from [datascar as v1, querypoints as pp]
where [v1:trip passes pp:pos, v1:type = "passenger",
  inst(initial(v1:trip at pp:pos)) <= (all
  (select inst(initial(v2:trip at pp:pos))
   from [datascar as v2]
   where [v2:trip passes pp:pos, v2:type = "passenger"]))
])

select[pp:pos as pos, v1:licence as licence]
from [datascar as v1, querypoints as pp, datascar as v2]
where [v1:trip passes pp:pos,
v1:type=[112, 97, 115, 115, 101, 110, 103, 101, 114],
inst(initial(v1:trip at pp:pos))<= (min(inst(initial(v2:trip at pp:pos)))),
v2:trip passes pp:pos,
v2:type=[112, 97, 115, 115, 101, 110, 103, 101, 114]]

sql select [pp:pos as pos, v1:licence as licence]
from [datascar as v1, querypoints as pp]
where [v1:trip passes pp:pos, v1:type = "passenger",
  inst(initial(v1:trip at pp:pos)) <= (all
  (select inst(initial(trip at pp:pos)) as firsttime
   from [datascar]
   where [trip passes pp:pos, type = "passenger"]))
]
select[pp:pos as pos, v1:licence as licence]
from[datascar as v1, querypoints as pp, txxrel5 as txxrel6]
where[v1:trip passes pp:pos, v1:type=[112, 97, 115, 115, 101, 110, 103, 101, 114], pp:pos=txxrel6:trip_v2, inst(initial(v1:trip at pp:pos))<=txxrel6:var2]


selectivity(pr(attr(psSUPPLYCOST, 1, l)= (select min(attr(psSUPPLYCOST, 0, l))from[rel(partsupp, *), rel(supplier, *), rel(nation, *), rel(region, *)]where[pr(attr(pPARTKEY, 1, l)=attr(psPARTKEY, 2, l), rel(part, *), rel(partsupp, *)), pr(attr(sSUPPKEY, 1, l)=attr(psSUPPKEY, 2, l), rel(supplier, *), rel(partsupp, *)), pr(attr(sNATIONKEY, 1, l)=attr(nNATIONKEY, 2, l), rel(supplier, *), rel(nation, *)), pr(attr(nREGIONKEY, 1, l)=attr(rREGIONKEY, 2, l), rel(nation, *), rel(region, *)), pr(attr(rNAME,1, l)=[69, 85, 82, 79, 80, 69], rel(region, *))]), rel(supplier, *), [rel(nation, *), rel(region, *), rel(partsupp, *)]), _G4414, _L183, _L184)

select[attr(oORDERPRIORITY, 0, l), count(*)as attr(ordercount, 0, u)]
from [rel(orders, *)]
where [
 pr(attr(oORDERDATE, 1, l)>=instant([49, 57, 57, 51, 45, 48, 55, 45,48, 49]), rel(orders, *)),
 pr(attr(oORDERDATE, 1, l)<theInstant(year_of(instant([49, 57, 57, 51, 45, 48, 55, 45, 48, 49])), month_of(instant([49, 57, 57, 51, 45, 48, 55, 45, 48, 49]))+3, day_of(instant([49, 57, 57, 51, 45, 48, 55, 45, 48, 49]))), rel(orders, *)),
 pr(
  exists(
   select*
   from[rel(lineitem, *)]
   where[
    pr(attr(lORDERKEY, 1, l)=attr(oORDERKEY, 2, l), rel(lineitem, *), rel(orders, *)),
	pr(attr(lCOMMITDATE, 1, l)<attr(lRECEIPTDATE, 1, l), rel(lineitem, *))
   ]
  ),
  rel(lineitem, *))
]
groupby [attr(oORDERPRIORITY, 0, l)]
orderby [attr(oORDERPRIORITY, 0, l)]

Q = select[oorderpriority, count(*)as ordercount] from orders where [oorderdate>=instant([49, 57, 57, 51, 45, 48, 55, 45, 48, 49]), oorderdate<theInstant(year_of(instant([4
9, 57, 57, 51, 45, 48, 55, 45, 48, 49])), month_of(instant([49, 57, 57, 51, 45, 48, 55, 45, 48, 49]))+3, day_of(instant([49, 57, 57, 51, 45, 48, 55, 45, 48, 49]))), exists(
select*from lineitem where[lorderkey=oorderkey, lcommitdate<lreceiptdate])] groupby [oorderpriority] orderby [oorderpriority],
Q2 = select[attr(oORDERPRIORITY, 0, l), count(*)as attr(ordercount, 0, u)] from [rel(orders, *)] where [pr(attr(oORDERDATE, 1, l)>=instant([49, 57, 57, 51, 45, 48, 55, 45,
48, 49]), rel(orders, *)), pr(attr(oORDERDATE, 1, l)<theInstant(year_of(instant([49, 57, 57, 51, 45, 48, 55, 45, 48, 49])), month_of(instant([49, 57, 57, 51, 45, 48, 55, 45
, 48, 49]))+3, day_of(instant([49, 57, 57, 51, 45, 48, 55, 45, 48, 49]))), rel(orders, *)), pr(exists(select*from[rel(lineitem, *)]where[pr(attr(lORDERKEY, 1, l)=attr(oORDE
RKEY, 2, l), rel(lineitem, *), rel(orders, *)), pr(attr(lCOMMITDATE, 1, l)<attr(lRECEIPTDATE, 1, l), rel(lineitem, *))]), rel(orders, *))] groupby [attr(oORDERPRIORITY, 0,
l)] orderby [attr(oORDERPRIORITY, 0, l)],
P = consume(sortby(project(groupby(sortby(predinfo(filter(feed(rel(lineitem, *)), attr(lCOMMITDATE, 1, l)<attr(lRECEIPTDATE, 1, l)), 0.614885, 0.0665), [attrname(attr(oORDE
RPRIORITY, 0, l))asc]), [attrname(attr(oORDERPRIORITY, 0, l))], [field(attr(ordercount, 0, u), count(feed(group)))]), [attrname(attr(oORDERPRIORITY, 0, l)), attrname(attr(o
rdercount, 0, u))]), [attrname(attr(oORDERPRIORITY, 0, l))asc])),
C = 24002.0
*/

%findall(No, (tpcd(No, Q), not(skipQuery(d, No))), L), findall(N, ( member(N, L), tpcd(N, Q), sql(Q) ), L2), findall(N, (member(N, L), not(member(N, L2))), L3).
%findall(No, ( testQuery(No, Q), No > 7, No < 47), L), findall(N, ( member(N, L), testQuery(N, Q), sql(Q) ), L2), findall(N, (member(N, L), not(member(N, L2))), L3).

