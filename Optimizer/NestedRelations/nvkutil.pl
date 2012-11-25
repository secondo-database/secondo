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

% eof

