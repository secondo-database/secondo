/*
$Header$
@author Nikolai van Kempen

My personal utility/testing predicates. I have some doubts whether they are of much use for anyone else. Thaty why I put them here.

*/

reset :-
  reload,
  closeDB,
  updateDB(optext),
  odb.

odb :-
  open('database optext').

obt :-
  open('database berlintest').

nrw2 :-
  open('database nrw2').

nrw3 :-
  open('database nrw3').

reload :-
  ['NestedRelations/test'],
  ['NestedRelations/nr'],
  ['NestedRelations/init'],
  ['NestedRelations/util'],
  ['NestedRelations/tutil'],
  ['MemoryAllocation/ma.pl'],
  ['MemoryAllocation/test.pl'],
  ['MemoryAllocation/ma_improvedcosts.pl'],
  %['Subqueries/subqueries'], % reloaded already then by optimizer
  [database],
  [operators],
  [statistics],
  [optimizer],
	!.

% Simply switch the enviroment
nr :-
  delOption(memoryAllocation),
  setOption(nestedRelations).

ma :-
  delOption(nestedRelations),
  setOption(memoryAllocation).

ic :-
  delOption(memoryAllocation),
  setOption(improvedcosts).

nomanr :-
  delOption(nestedRelations),
  delOption(memoryAllocation).

manr :-
  setOption(nestedRelations),
  setOption(memoryAllocation).

% Some stuff used within my colloquium

nrdemo :- 
	nr,
	open('database nrdemo').

bdemo :-
	cdb,
	open('database berlintest').

demo(1, select * from orte).
demo(2, select [o:ort, (select * from plz where ort=o:ort) as newsubrel] from orte as o).
demo(3, select * from orteh).
demo(4, select [o:bevth, (select ort from o:subrel) as newsubrel] from orteh as o).


demo(5, select *
from   orteh as o
where  [exists(select ort from   o:subrel where  ort starts "B")]).

demo(6, select [o:bevth, (select ort from   o:subrel
                  where  ort starts "B") as newsubrel]
from   orteh as o
where  [exists(   select ort from   o:subrel
                  where  ort starts "B")]).



demo(7, select * from orteh unnest(subrel)).
demo(org8, select * from orte nest(kennzeichen, subrel)).
demo(8, select * from plz nest(ort, subrel) where ort starts "A").
demo(9, select * from trains).
demo(10, select * from trains unnest(trip)).


demo(11, select *
from   (
         select [bevt div 100 as bevth,
                bevt, ort, kennzeichen, vorwahl]
         from   orte
       ) nest(bevth, subRel)).


mademo :-
	cdb,fail.
mademo :-
	ma,
	debugLevel(ma),
	nrw3,
  maCreateTestSels.

demo(ma,  select * from [roads300k as r1, roads400k as r2, roads500k as r3, buildings300k as r4, buildings400k as r5, buildings500k as r6] where[r1:no=r2:no,r2:no=r3:no,r3:no=r4:no,r4:no=r5:no, r5:no=r6:no]).
	


% eof

