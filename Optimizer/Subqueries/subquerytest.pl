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
 