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
  transformNestedPredicate(pnum, Attrs2, parts, Rels2, qoh= (select count(s:shipdate)from supply as s where[pnum=s:pnum, s:shipdate<19800101]), Preds2).
  
 
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




 