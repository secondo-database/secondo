/*

----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

August 2008, Burkart Poneleit. Initial Version

This file contains testqueries to verify subquery functionality. Long running 
queries are commented out.

*/

myDebug(Command) :-
  leash(-all),
  trace,
  Command,
  leash(+all).
  
/*

Testqueries over database berlinmod, to test subquery functionality

*/

berlinmod(7, select [pp:pos as pos, v1:licence as licence]
from [datascar as v1, querypoints as pp]
where [v1:trip passes pp:pos, v1:type = "passenger",
  inst(initial(v1:trip at pp:pos)) <= (all
  (select inst(initial(v2:trip at pp:pos))
   from [datascar as v2]
   where [v2:trip passes pp:pos, v2:type = "passenger"]))
]).

/*

Testqueries from the papers "On Optimizing an SQL-like Nested Query" by Won Kim and
"Optimization of nested SQL queries revisited" by Ganski/Wong. These papers are
the base of the unnesting algorithms implemented in Secondo.

*/

ganski(1, select sname from s where sno in 
  (select sno from sp where no = 'P2')).
ganski(2, select sno from sp where pno = 
  (select max(pno) from p)).
ganski(3, select sno from sp where pno in 
  (select pno from p where weight > 50)).
ganski(4, select sname from s where sno in 
  (select sno from sp where [qty > 100, sporigin = scity])).
ganski(5, select pname from p where pno = 
  (select max(pno) from sp where sporigin = pcity)).
kiessling(2, select pnum from parts where qoh = 
  (select count(shipdate) 
    from supply where [supply:pnum = parts:pnum, shipdate < 19800101])).
kiessling(5, select pnum from parts where qoh = 
  (select max(quan) 
    from supply where [supply:pnum < parts:pnum, shipdate < 19800101])).
kim(4, select sno from sp where pno = 
  (select max(pno) from p where price > 25)).
kim(5, select sno from sp where pno in 
  (select pno from p where price > 25)).
kim(6, select sno from sp where pno in 
  (select pno from pj where [sp:sno = pj:jno, jloc = 'NEW YORK'])).
kim(7, select sno from sp where pno = 
  (select max(pno) from pj where [pj:jno = sp:jno, jloc = 'NEW YORK'])).
kim(10, select sname from s 
  where sno not in(select sno from sp where [pno = 'P1'])).

/*

Testqueries over relation of database opt

*/

% test program courtesy of Prof. Gueting
testSQ(N) :- N > 47.

testSQ(N) :-
  testQuery(N, X),
  nl, nl, nl, nl, nl,
  write('============================================================'), nl,
  write('Query '), write(N), nl,
  write(X),
  nl,
  sql X,
  M is N + 1,
  testSQ(M). 

testQuery(1, select * from orte as o where o:ort in 
  (select ort from plz where plz < 10000) first 10).
testQuery(2, select * from orte as o where o:ort in 
  (select ort from plz where plz > o:bevt) first 10).
testQuery(3, select * from [orte as o, staedte as s] 
  where [o:ort = s:sname, 
    o:ort in (select p:ort from plz as p where p:ort = s:sname)] first 10).

% These two Queries take a rather long time to run, as they cannot be unnested.
% testQuery(4, select * from orte as o 
%  where o:ort not in(select ort from plz where plz > 10000) first 10).
% testQuery(5, select * from orte as o 
%  where o:ort not in(select ort from plz where plz < o:bevt) first 10).
% testQuery(6, select * from [orte as o, staedte as s] 
%  where [o:ort = s:sname, 
%  o:ort not in(select p:ort from plz as p where p:ort = s:sname)] first 10).
% dummy queries for test program
testQuery(4, select count(*) from orte).
testQuery(5, select count(*) from orte).
testQuery(6, select count(*) from orte).

testQuery(7, select * from orte as o 
  where [exists(select ort from plz where plz < o:bevt)] first 10).
testQuery(8, select * from [orte as o, staedte as s] 
  where [o:ort = s:sname, exists(select p:ort from plz as p 
    where [p:ort = s:sname, s:bev > o:bevt])] first 10).
testQuery(9, select * from orte as o 
  where [not(exists(select ort from plz where plz < o:bevt))] first 10).
testQuery(10, select * from [orte as o, staedte as s] 
  where [o:ort = s:sname, 
  not(exists(select p:ort from plz as p 
    where [p:ort = s:sname, s:bev > o:bevt]))] first 10).
testQuery(11, select * from orte as o 
  where o:bevt <= all(select bev from staedte where plz > 8699) first 10).
testQuery(12, select * from orte as o 
  where o:bevt <= all(select bev from staedte where o:ort > sname) first 10).
testQuery(13, select * from [orte as o, plz as p] 
  where o:bevt <= all(select bev from staedte 
    where [o:ort > sname, p:ort = sname]) first 10).
testQuery(14, select * from orte as o 
  where o:bevt < all(select bev from staedte where plz > 8699) first 10).
testQuery(15, select * from orte as o 
  where o:bevt < all(select bev from staedte where o:ort > sname) first 10).
testQuery(16, select * from [orte as o, plz as p] 
  where o:bevt < all(select bev from staedte 
    where [o:ort > sname, p:ort = sname]) first 10).
testQuery(17, select * from orte as o 
  where o:bevt >= all(select bev from staedte where plz > 8699) first 10).
testQuery(18, select * from orte as o 
  where o:bevt >= all(select bev from staedte where o:ort > sname) first 10).
testQuery(19, select * from [orte as o, plz as p] 
  where o:bevt >= all(select bev from staedte
  where [o:ort > sname, p:ort = sname]) first 10).
testQuery(20, select * from orte as o 
  where o:bevt > all(select bev from staedte where plz > 8699) first 10).
testQuery(21, select * from orte as o 
  where o:bevt > all(select bev from staedte where o:ort > sname) first 10).
testQuery(22, select * from [orte as o, plz as p] 
  where o:bevt > all(select bev from staedte
  where [o:ort > sname, p:ort = sname]) first 10).
testQuery(23, select * from orte as o 
  where o:bevt <= any(select bev from staedte where plz > 8699) first 10).
testQuery(24, select * from orte as o 
  where o:bevt <= any(select bev from staedte where o:ort > sname) first 10).
testQuery(25, select * from [orte as o, plz as p] 
  where o:bevt <= any(select bev from staedte
  where [o:ort > sname, p:ort = sname]) first 10).
testQuery(26, select * from orte as o 
  where o:bevt < any(select bev from staedte where plz > 8699) first 10).
testQuery(27, select * from orte as o 
    where o:bevt < any(select bev from staedte where o:ort > sname) first 10).
testQuery(28, select * from [orte as o, plz as p] 
  where o:bevt < any(select bev from staedte
  where [o:ort > sname, p:ort = sname]) first 10).
testQuery(29, select * from orte as o 
  where o:bevt = any(select bev from staedte where plz > 8699) first 10).
testQuery(30, select * from orte as o 
  where o:bevt = any(select bev from staedte where o:ort > sname) first 10).
testQuery(31, select * from [orte as o, plz as p] 
  where o:bevt = any(select bev from staedte
    where [o:ort > sname, p:ort = sname]) first 10).
testQuery(32, select * from orte as o 
  where o:bevt >= any(select bev from staedte where plz > 8699) first 10).
testQuery(33, select * from orte as o 
  where o:bevt >= any(select bev from staedte where o:ort > sname) first 10).
testQuery(34, select * from [orte as o, plz as p] 
  where o:bevt >= any(select bev from staedte
    where [o:ort > sname, p:ort = sname]) first 10).
testQuery(35, select * from orte as o 
  where o:bevt > any(select bev from staedte where plz > 8699) first 10).
testQuery(36, select * from orte as o 
  where o:bevt > any(select bev from staedte where o:ort > sname) first 10).
testQuery(37, select * from [orte as o, plz as p] 
  where o:bevt = (select max(bev*1000) from staedte
    where [o:ort > sname, p:ort = sname]) first 10).
testQuery(38, select * from orte as o 
  where o:bevt = (select max(bev/1000) from staedte where plz > 8699) first 10).
testQuery(39, select * from orte as o 
  where o:bevt = (select max(bev/1000) from staedte where o:ort > sname) first 10).
testQuery(40, select * from [orte as o, plz as p] 
  where o:bevt = (select max(bev/1000) from staedte
    where [o:ort > sname, p:ort = sname]) first 10).
testQuery(41, select * from orte as o 
  where o:bevt = (select min(bev/1000) from staedte where plz > 8699) first 10).
testQuery(42, select * from orte as o 
  where o:bevt = (select min(bev/1000) from staedte where o:ort > sname) first 10).
testQuery(43, select * from [orte as o, plz as p] 
  where o:bevt = (select min(bev/1000) from staedte
    where [o:ort > sname, p:ort = sname]) first 10).
testQuery(44, select * from orte as o 
  where o:bevt = (select avg(bev/1000) from staedte where plz > 8699) first 10).
testQuery(45, select * from orte as o 
  where o:bevt = (select avg(bev/1000) from staedte 
    where o:ort > sname) first 10).
testQuery(46, select * from [orte as o, plz as p] 
  where o:bevt = (select avg(bev/1000) from staedte
  where [o:ort > sname, p:ort = sname]) first 10).
testQuery(47, select * from plz as p1 
  where p1:plz = (select max(plz) from plz 
    where ort = p1:ort) first 10).


