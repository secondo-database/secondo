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

1 Database Dependent Information

[File ~database.pl~]

The Secondo optimizer module needs database dependent information to
compute the best query plan. In particular information about the
cardinality and the schema of a relation is needed by the optimizer.
Furthermore the spelling of relation and attribute names must be
known to send a Secondo query or command.  Finally the optimizer has 
to be informed, if an index exists for the pair (~relationname~, 
~attributename~). All this information look up is provided by this 
module. There are two assumptions about naming conventions for index
objects and sample relation objects. These are

  * ~relationname~\_~attributename~ for index objects. Note, that the
    first letter of the relationname is written in lower case.

  * ~relationname~\_~sample~ for sample relation objects.

You should avoid naming your objects in this manner. Relation- and
attibute names are written the same way as they are written in the
Secondo database, with the single exception for index objects 
(see above).

1.1 Relation Schemas

1.1.1 Auxiliary Rules

Rule ~extractlist~ finds a complete list for one Secondo object within 
a list of object lists. The result is unified with the second list.

*/
extractList([[First, _]], [First]).
extractList([[First, _] | Rest], [First | Rest2]) :-
  extractList(Rest, Rest2).
/*
Sets all letters of all atoms of the first list into lower case. The 
result is in the second list.

*/
downcase_list([], []).
downcase_list([First1 | Rest1], [First2 | Rest2]) :-
  downcase_atom(First1, First2),
  downcase_list(Rest1, Rest2).

time(Clause) :-
  get_time(Time1),
  (call(Clause) ; true),
  get_time(Time2),
  Time is Time2 - Time1,
  convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  MSs is Minute *60000 + Sec*1000 + MilliSec,
  write('Elapsed Time: '),
  write(MSs),
  write(' ms'),nl.

classifyRel(Rel, small) :-
  card(Rel, Size),
  Size < 1000,
  !.

classifyRel(Rel, middle) :-
  card(Rel, Size),
  Size < 100001,
  Size > 999,
  !.

classifyRel(Rel, large) :-
  card(Rel, Size),
  Size > 100000,
  !.

createSmallRelation(Rel, ObjList) :-  % Rel in lc
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleNameSmall(Rel3, Small),
  member(['OBJECT', Small, _ , [[_ | _]]], ObjList),
  !.

createSmallRelation(Rel, ObjList) :-  % Rel in uc
  spelling(Rel, Rel2),
  not(Rel2 = lc(_)),
  upper(Rel2, URel),
  sampleNameSmall(URel, Small),
  member(['OBJECT', Small, _ , [[_ | _]]], ObjList),
  !.

createSmallRelation(Rel, _)  :-  % Rel in lc
  classifyRel(Rel, small),
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleNameSmall(Rel3, Small),
  concat_atom(['let ', Small, ' = ', Rel3, 
    ' feed consume'], '', QueryAtom),  
  tryCreate(QueryAtom),
  card(Rel3, Card),
  assert(storedCard(Small, Card)),
  downcase_atom(Small, DCSmall),  
  assert(storedSpell(DCSmall, lc(Small))),
  !.

createSmallRelation(Rel, _) :-  % Rel in uc
  classifyRel(Rel, small),
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  sampleNameSmall(URel, Small),
  concat_atom(['let ', Small, ' = ', URel, 
    ' feed consume'], '', QueryAtom),
  tryCreate(QueryAtom),
  card(Rel2, Card),
  lowerfl(Small, LSmall),
  assert(storedCard(LSmall, Card)),
  downcase_atom(Small, DCSmall),
  assert(storedSpell(DCSmall, LSmall)),
  !.

createSmallRelation(Rel, _)  :-  % Rel in lc
  classifyRel(Rel, middle),
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleNameSmall(Rel3, Small),
  concat_atom(['let ', Small, ' = ', Rel3, 
    ' sample[1000, 0.1] consume'], '', QueryAtom), 
  tryCreate(QueryAtom),
  card(Rel3, Card),
  SmallCard is truncate(min(Card, max(1000, Card*0.1))),  
  assert(storedCard(Small, SmallCard)),
  downcase_atom(Small, DCSmall),  
  assert(storedSpell(DCSmall, lc(Small))),
  !.

createSmallRelation(Rel, _)  :-  % Rel in uc
  classifyRel(Rel, middle),
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  sampleNameSmall(URel, Small),
  concat_atom(['let ', Small, ' = ', URel, 
    ' sample[1000, 0.1] consume'], '', QueryAtom), 
  tryCreate(QueryAtom),
  card(Rel2, Card),
  SmallCard is truncate(min(Card, max(1000, Card*0.1))),  
  lowerfl(Small, LSmall),
  assert(storedCard(LSmall, SmallCard)),
  downcase_atom(Small, DCSmall),
  assert(storedSpell(DCSmall, LSmall)),
  !.

createSmallRelation(Rel, _)  :-  % Rel in lc
  classifyRel(Rel, large),
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleNameSmall(Rel3, Small),
  concat_atom(['let ', Small, ' = ', Rel3, 
    ' sample[10000, 0.01] consume'], '', QueryAtom), 
  tryCreate(QueryAtom),
  card(Rel3, Card),
  SmallCard is truncate(min(Card, max(10000, Card*0.01))),  
  assert(storedCard(Small, SmallCard)),
  downcase_atom(Small, DCSmall),  
  assert(storedSpell(DCSmall, lc(Small))),
  !.

createSmallRelation(Rel, _)  :-  % Rel in uc
  classifyRel(Rel, large),
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  sampleNameSmall(URel, Small),
  concat_atom(['let ', Small, ' = ', URel, 
    ' sample[10000, 0.01] consume'], '', QueryAtom), 
  tryCreate(QueryAtom),
  card(Rel2, Card),
  SmallCard is truncate(min(Card, max(10000, Card*0.01))),  
  lowerfl(Small, LSmall),
  assert(storedCard(LSmall, SmallCard)),
  downcase_atom(Small, DCSmall),
  assert(storedSpell(DCSmall, LSmall)),
  !.

/*
Creates a sample relation, for determining the selectivity of a relation 
object for a given predicate. The first two rules consider the case, that 
there is a sample relation already available and the last two ones create 
new relations by sending a Secondo ~let~-command.

*/

:- dynamic(set_dynamic_sample/1). 

set_dynamic_sample(off).

sampleSizeJoin(500).

sampleSizeSelection(2000).

thresholdMainMemorySizeSampleJ(2048).

thresholdMainMemorySizeSampleS(2048).

dynamic_sample(X) :-
 var(X),
 set_dynamic_sample(Y),
 atom_concat(Y, '',X),!.

dynamic_sample(X) :-
  X = on,
  retractall(set_dynamic_sample(_)),
  assert(set_dynamic_sample(on)),!.

dynamic_sample(X) :-
  X = off,
  retractall(set_dynamic_sample(_)),
  assert(set_dynamic_sample(off)).
  
hasSampleS(Rel) :-
  getSecondoList(ObjList),
  getSpelledRel(Rel, SpelledRel),
  concat_atom([SpelledRel, '_sample_s'], '', ORel),
  member(['OBJECT', ORel, _ , [[rel | _]]], ObjList).

hasSampleJ(Rel) :-
  getSecondoList(ObjList),
  getSpelledRel(Rel, SpelledRel),
  concat_atom([SpelledRel, '_sample_j'], '', ORel),
  member(['OBJECT', ORel, _ , [[rel | _]]], ObjList).

getSizeSampleJ(Rel, Size) :-
  getSpelledRel(Rel, SpelledRel),
  card(SpelledRel, Card),
  sampleSizeJoin(JoinSize),
  SampleCard is truncate(min(Card, max(JoinSize, Card*0.00001))),
  tuplesize(SpelledRel, TupleSize),
  Size is (SampleCard*TupleSize) / 1024.

getSizeSampleS(Rel, Size) :-
  getSpelledRel(Rel, SpelledRel),
  card(SpelledRel, Card),
  sampleSizeSelection(SelectionSize),
  SampleCard is truncate(min(Card, max(SelectionSize, Card*0.00001))),
  tuplesize(SpelledRel, TupleSize),
  Size is (SampleCard*TupleSize) / 1024.

createSampleJ(Rel) :- %Rel in lc
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleNameJ(Rel3, Sample),
  sampleSizeJoin(JoinSize),
  concat_atom(['let ', Sample, ' = ', Rel3, 
    ' sample[', JoinSize, ', 0.00001] consume'], '', QueryAtom),    
  tryCreate(QueryAtom),
  card(Rel3, Card),
  SampleCard is truncate(min(Card, max(JoinSize, Card*0.00001))),
  assert(storedCard(Sample, SampleCard)),
  downcase_atom(Sample, DCSample),  
  assert(storedSpell(DCSample, lc(Sample))).

createSampleJ(Rel) :- %Rel in uc
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  sampleNameJ(URel, Sample),
  sampleSizeJoin(JoinSize),
  concat_atom(['let ', Sample, ' = ', URel, 
    ' sample[', JoinSize, ', 0.00001] consume'], '', QueryAtom),    
  tryCreate(QueryAtom),
  card(Rel2, Card),
  SampleCard is truncate(min(Card, max(2000, Card*0.00001))),
  lowerfl(Sample, LSample),
  assert(storedCard(LSample, SampleCard)),
  downcase_atom(Sample, DCSample),
  assert(storedSpell(DCSample, LSample)).

createSampleS(Rel) :- %Rel in lc
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleNameS(Rel3, Sample),
  sampleSizeSelection(SelectionSize),
  concat_atom(['let ', Sample, ' = ', Rel3, 
    ' sample[', SelectionSize, ', 0.00001] consume'], '', QueryAtom),    
  tryCreate(QueryAtom),
  card(Rel3, Card),
  SampleCard is truncate(min(Card, max(SelectionSize, Card*0.00001))),
  assert(storedCard(Sample, SampleCard)),
  downcase_atom(Sample, DCSample),  
  assert(storedSpell(DCSample, lc(Sample))).

createSampleS(Rel) :- %Rel in uc
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  sampleNameS(URel, Sample),
  sampleSizeSelection(SelectionSize),
  concat_atom(['let ', Sample, ' = ', URel, 
    ' sample[', SelectionSize, ', 0.00001] consume'], '', QueryAtom),    
  tryCreate(QueryAtom),
  card(Rel2, Card),
  SampleCard is truncate(min(Card, max(2000, Card*0.00001))),
  lowerfl(Sample, LSample),
  assert(storedCard(LSample, SampleCard)),
  downcase_atom(Sample, DCSample),
  assert(storedSpell(DCSample, LSample)).

writeErrorSampleFileJ(Rel, MemorySize) :-
  nl,
  spelling(Rel, Rel2),
  (Rel2 = lc(Rel) -> Rel3 = Rel; upper(Rel,Rel3)),
  write('ERROR: Couldn\'t create join sample file for relation \''),
  write(Rel), write('\'!'),nl,
  write('Sample file needs more than '), write(MemorySize),
  write(' KB in main memory.'),nl,
  write('Please create the file manually, e.g.: '),
  write('let \''), write(Rel3),write('_sample_j = '),
  sampleSizeJoin(JoinSize), 
  write(Rel3),write(' sample['),write(JoinSize),
  write(', 0.00001] consume\'.'),nl,nl.

writeErrorSampleFileS(Rel, MemorySize) :-
  nl,
  spelling(Rel, Rel2),
  (Rel2 = lc(Rel) -> Rel3 = Rel; upper(Rel,Rel3)),
  write('ERROR: Couldn\'t create selection sample file for relation \''),
  write(Rel3), write('\'!'),nl,
  write('Sample file needs more than '), write(MemorySize),
  write(' KB in main memory.'),nl,
  write('Please create the file manually, e.g.: '),
  write('let \''), write(Rel3),write('_sample_s = '),
  sampleSizeSelection(SelectionSize), 
  write(Rel3),write(' sample['),write(SelectionSize),
  write(', 0.00001] consume\'.'),nl,nl.

createSampleRelation4(Rel, Size, MemorySize) :-
  not(hasSampleJ(Rel)),
  getSizeSampleJ(Rel, Size),
  thresholdMainMemorySizeSampleJ(MemorySize).

createSampleRelation3(Rel, Size, MemorySize) :-
  not(hasSampleS(Rel)),
  getSizeSampleS(Rel, Size),
  thresholdMainMemorySizeSampleS(MemorySize),
  Size =< MemorySize,
  createSampleS(Rel).

createSampleRelation2(Rel, Size, MemorySize) :-
  hasSampleS(Rel),
  not(hasSampleJ(Rel)),
  getSizeSampleJ(Rel, Size),
  thresholdMainMemorySizeSampleJ(MemorySize).

% Case 1  
createSampleRelation(Rel) :-
  hasSampleS(Rel),
  hasSampleJ(Rel),!.
% Case 2
createSampleRelation(Rel) :-
  createSampleRelation2(Rel, Size, MemorySize),
  Size =< MemorySize,
  createSampleJ(Rel),!.
% Case 3
createSampleRelation(Rel) :-
  createSampleRelation2(Rel, Size, MemorySize),
  Size > MemorySize, 
  writeErrorSampleFileJ(Rel, MemorySize),!,fail.
% Case 4
createSampleRelation(Rel) :-
  hasSampleJ(Rel),
  createSampleRelation3(Rel, _, _),!.
% Case 6
createSampleRelation(Rel) :-
  createSampleRelation4(Rel, Size, MemorySize),
  Size =< MemorySize,
  createSampleRelation3(Rel, _, _),
  createSampleJ(Rel),!.
% Case 7
createSampleRelation(Rel) :-
  createSampleRelation3(Rel, _, _),
  createSampleRelation4(Rel, Size, MemorySize),
  Size > MemorySize,
  writeErrorSampleFileJ(Rel, MemorySize),!,fail.  
% Case 8
createSampleRelation(Rel) :-
  not(hasSampleS(Rel)),
  getSizeSampleS(Rel, Size),
  thresholdMainMemorySizeSampleS(MemorySize),
  Size > MemorySize,
  createSampleRelation4(Rel, Size2, MemorySize2),
  Size2 =< MemorySize2,
  createSampleJ(Rel),
  writeErrorSampleFileS(Rel, MemorySize),!,fail.
% Case 9
createSampleRelation(Rel) :-
  not(hasSampleS(Rel)),
  getSizeSampleS(Rel, Size),
  thresholdMainMemorySizeSampleS(MemorySize),
  Size > MemorySize,
  createSampleRelation4(Rel, Size, MemorySize),
  Size > MemorySize,
  writeErrorSampleFileS(Rel, MemorySize),
  writeErrorSampleFileJ(Rel, MemorySize),!,fail.
% Case 5
createSampleRelation(Rel) :-
  not(hasSampleS(Rel)),
  getSizeSampleS(Rel, Size),
  thresholdMainMemorySizeSampleS(MemorySize),
  Size > MemorySize,
  writeErrorSampleFileS(Rel, MemorySize),
  hasSampleJ(Rel),fail.

createSampleRelationIfNotDynamic(Rel) :-
  set_dynamic_sample(off),
  createSampleRelation(Rel),!.

createSampleRelationIfNotDynamic(_) :-
  set_dynamic_sample(on).

/*createSampleRelation(Rel, ObjList) :-   Rel in lc
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleNameS(Rel3, Sample1),
  member(['OBJECT', Sample1, _ , [[_ | _]]], ObjList),
  sampleNameJ(Rel3, Sample2),
  member(['OBJECT', Sample2, _ , [[_ | _]]], ObjList),
  !.

createSampleRelation(Rel, ObjList) :-   Rel in uc
  spelling(Rel, Rel2),
  not(Rel2 = lc(_)),
  upper(Rel2, URel),
  sampleNameS(URel, Sample1),
  member(['OBJECT', Sample1, _ , [[_ | _]]], ObjList),
  sampleNameJ(URel, Sample2),
  member(['OBJECT', Sample2, _ , [[_ | _]]], ObjList),
  !.

createSampleRelation(Rel, _)  :-   Rel in lc
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleNameS(Rel3, Sample1),
  concat_atom(['let ', Sample1, ' = ', Rel3, 
    ' sample[2000, 0.00001] consume'], '', QueryAtom1),
  sampleNameJ(Rel3, Sample2),
  concat_atom(['let ', Sample2, ' = ', Rel3, 
    ' sample[500, 0.00001] consume'], '', QueryAtom2),    
  tryCreate(QueryAtom1),
  tryCreate(QueryAtom2),
  card(Rel3, Card),
  SampleCard1 is truncate(min(Card, max(2000, Card*0.00001))),
  SampleCard2 is truncate(min(Card, max(500, Card*0.00001))),
  assert(storedCard(Sample1, SampleCard1)),
  assert(storedCard(Sample2, SampleCard2)),
  downcase_atom(Sample1, DCSample1),
  downcase_atom(Sample2, DCSample2),  
  assert(storedSpell(DCSample1, lc(Sample1))),
  assert(storedSpell(DCSample2, lc(Sample2))),
  !.

createSampleRelation(Rel, _) :-   Rel in uc
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  sampleNameS(URel, Sample1),
  concat_atom(['let ', Sample1, ' = ', URel, 
    ' sample[2000, 0.00001] consume'], '', QueryAtom1),
  sampleNameJ(URel, Sample2),
  concat_atom(['let ', Sample2, ' = ', URel, 
    ' sample[500, 0.00001] consume'], '', QueryAtom2),
  tryCreate(QueryAtom1),
  tryCreate(QueryAtom2),
  card(Rel2, Card),
  SampleCard1 is truncate(min(Card, max(2000, Card*0.00001))),
  lowerfl(Sample1, LSample1),
  assert(storedCard(LSample1, SampleCard1)),
  SampleCard2 is truncate(min(Card, max(500, Card*0.00001))),
  lowerfl(Sample2, LSample2),
  assert(storedCard(LSample2, SampleCard2)),
  downcase_atom(Sample1, DCSample1),
  assert(storedSpell(DCSample1, LSample1)),
  downcase_atom(Sample2, DCSample2),
  assert(storedSpell(DCSample2, LSample2)),
  !.*/
/*
Checks, if an index exists for ~Rel~ and ~Attr~ and stores the 
respective values to the dynamic predicates ~storedIndex/4~ or 
~storedNoIndex/2~.

*/
lookupIndex(Rel, Attr) :-
  not( hasIndex(rel(Rel, _, _), attr(Attr, _, _), _, _) ).

lookupIndex(Rel, Attr) :-
  hasIndex(rel(Rel, _, _), attr(Attr, _, _), _, _).

/*
Gets the spelling of each attribute name of a relation and stores 
the result to ~storedSpells~. The index checking for every attribute
over the given relation ~Rel~ is also called.

*/
createAttrSpelledAndIndexLookUp(_, []).
createAttrSpelledAndIndexLookUp(Rel, [ First | Rest ]) :-
  downcase_atom(First, DCFirst),
  spelling(Rel:DCFirst, _),
  spelled(Rel, SRel, _),
  lowerfl(First, LFirst),
  lookupIndex(SRel, LFirst),
  createAttrSpelledAndIndexLookUp(Rel, Rest).
/*
1.1.2 Look Up The Relation Schema

---- relation(Rel, AttrList) :-
----

The schema for relation ~Rel~ is ~AttrList~. If this predicate
is called, we also look up for the spelling of ~Rel~ and all
elements of ~AttList~. Index look up and creating sample relations
are executed furthermore by this rule.

*/

%checkForIndex(_, []).

%checkForIndex(Rel, [First|Rest]) :-
%updateIndex(Rel, First),
%checkForIndex(Rel, Rest).

trycreateSmallRelation(Rel, ObjList) :- 
  usingVersion(entropy),
  createSmallRelation(Rel, ObjList),!.

trycreateSmallRelation(_, _) :- 
  usingVersion(standard).

relation(Rel, AttrList) :-
  storedRel(Rel, AttrList),!.

relation(Rel, AttrList) :-
  set_dynamic_sample(on),
  getSecondoList(ObjList),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [AttrList2]]]]]]], ObjList),
  downcase_atom(ORel, DCRel),
  DCRel = Rel,
  extractList(AttrList2, AttrList3),
  downcase_list(AttrList3, AttrList),
  createSampleRelationIfNotDynamic(Rel),
  trycreateSmallRelation(Rel, ObjList),!.

relation(Rel, AttrList) :-
  getSecondoList(ObjList),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [AttrList2]]]]]]], ObjList),
  downcase_atom(ORel, DCRel),
  DCRel = Rel,
  extractList(AttrList2, AttrList3),
  downcase_list(AttrList3, AttrList),
  spelling(Rel, _),
  card(Rel, _),
  tuplesize(Rel, _),
  createSampleRelationIfNotDynamic(Rel),
  trycreateSmallRelation(Rel, ObjList),
  assert(storedRel(Rel, AttrList)),
  createAttrSpelledAndIndexLookUp(Rel, AttrList3).

  %retract(storedSecondoList(ObjList)).

/*
1.1.3 Storing And Loading Relation Schemas

*/
readStoredRels :-
  retractall(storedRel(_, _)),
  [storedRels].  

writeStoredRels :-
  open('storedRels.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredRel(FD), _),
  close(FD).

writeStoredRel(Stream) :-
  storedRel(X, Y),
  write(Stream, storedRel(X, Y)),
  write(Stream, '.\n').

:-
  dynamic(storedRel/2),
  at_halt(writeStoredRels),
  readStoredRels.
/*
1.2 Spelling of Relation and Attribute Names

Due to the facts, that PROLOG interprets words beginning with a capital 
letter as varibales and that Secondo allows arbitrary writing of
relation and attribute names, we have to find a convention. So, for
Secondo names beginning with a small letter, the PROLOG notation will be
lc(name), which means, leaver the first letter as it is. If the first
letter of a Secondo name is written in upper case, then it is set to lower
case. E.G.

The PROLOG notation for ~pLz~ is ~lc(pLz)~ and for ~EMPLOYEE~ it'll be ~eMPLOYEE~. 

1.2.1 Auxiliary Rules

Checks, if the first letter of ~Rel~ is written in lower case

*/
is_lowerfl(Rel) :-
  atom_chars(Rel, [First | _]),
  downcase_atom(First, LCFirst),
  First = LCFirst.
/*
Sets the first letter of ~Upper~ to lower case. Result is ~Lower~.

*/  
lowerfl(Upper, Lower) :-
  atom_chars(Upper, [First | Rest]),
  char_type(First2, to_lower(First)),
  append([First2], Rest, LowerList),
  atom_chars(Lower, LowerList).
  %atom_codes(Upper, [First | Rest]),
  %to_lower(First, First2),
  %LowerList = [First2 | Rest],
  %atom_codes(Lower, LowerList).
/*
Returns a list of Secondo objects, if available in the knowledge
base, otherwise a Secondo command is issued to get the list. The
second rule ensures in addition, that the object list is stored 
into local memory by the dynamic predicate ~storedSecondoList/1~.

*/
getSecondoList(ObjList) :-
  storedSecondoList(ObjList),
  !.

getSecondoList(ObjList) :-
  secondo('list objects',[_, [_, [_ | ObjList]]]), 
  assert(storedSecondoList(ObjList)),
  !.
/*
1.2.2 Spelling Of Attribute Names

---- spelling(Rel:Attr, Spelled) :-
----

The spelling of attribute ~Attr~ of relation ~Rel~ is ~Spelled~.

~Spelled~ is available via the dynamic predicate ~storedSpell/2~.

*/
spelling(Rel:Attr, Spelled) :-
  storedSpell(Rel:Attr, Spelled),
  !.
/*
Returns the spelling of attribute name ~Attr~, if the first letter of
the attribute name is written in lower case. ~Spelled~ returns a term
lc(attrnanme).

*/ 
spelling(Rel:Attr, Spelled) :-
  getSecondoList(ObjList),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [AttrList]]]]]]], ObjList),
  downcase_atom(ORel, Rel),
  member([OAttr, _], AttrList),
  downcase_atom(OAttr, Attr),
  is_lowerfl(OAttr),
  Spelled = lc(OAttr),
  assert(storedSpell(Rel:Attr, lc(OAttr))),
  !.
/*
Returns the spelling of attribute name ~Attr~, if the first letter
of the attribute name is written in upper case. ~Spelled~ returns just
the attribute name with the first letter written in lower case.

*/
spelling(Rel:Attr, Spelled) :-
  getSecondoList(ObjList),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [AttrList]]]]]]], ObjList),
  downcase_atom(ORel, Rel),
  member([OAttr, _], AttrList),
  downcase_atom(OAttr, Attr),
  lowerfl(OAttr, Spelled),
  assert(storedSpell(Rel:Attr, Spelled)),
  !.

spelling(_:_, _) :- !, fail.
/*
1.2.3 Spelling Of Relation Names

---- spelling(Rel, Spelled) :-
----

The spelling of relation ~Rel~ is ~Spelled~.

~Spelled~ is available via the dynamic predicate ~storedSpell/2~.

*/  
spelling(Rel, Spelled) :-
  storedSpell(Rel, Spelled),
  !.
/*
Returns the spelling of relation name ~Rel~, if the first letter of
the relation name is written in lower case. ~Spelled~ returns a term
lc(relationname).

*/
spelling(Rel, Spelled) :-
  getSecondoList(ObjList),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [_]]]]]]], ObjList),
  downcase_atom(ORel, Rel),
  is_lowerfl(ORel),
  Spelled = lc(ORel),
  assert(storedSpell(Rel, lc(ORel))),
  !.
/*
Returns the spelling of relation name ~Rel~, if the first letter
of the relation name is written in upper case. ~Spelled~ returns just
the relation name with the first letter written in lower case.

*/  
spelling(Rel, Spelled) :-
  getSecondoList(ObjList),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [_]]]]]]], ObjList),
  downcase_atom(ORel, Rel),
  lowerfl(ORel, Spelled),
  assert(storedSpell(Rel, Spelled)),
  !.
/*
1.2.4 Storing And Loading Of Spelling

*/  
readStoredSpells :-
  retractall(storedSpell(_, _)),
  [storedSpells]. 

writeStoredSpells :-
  open('storedSpells.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredSpell(FD), _),
  close(FD).

writeStoredSpell(Stream) :-
  storedSpell(X, Y),
  write(Stream, storedSpell(X, Y)),
  write(Stream, '.\n').

:-
  dynamic(storedSpell/2),
  dynamic(storedSecondoList/1),
  dynamic(elem_is/3),
  at_halt(writeStoredSpells),
  readStoredSpells.
/*
1.3  Cardinalities of Relations

---- card(Rel, Size) :-
----

The cardinality of relation ~Rel~ is ~Size~.

1.3.1 Get Cardinalities

If ~card~ is called, it tries to look up the cardinality via the 
dynamic predicate ~storedCard/2~ (automatically stored).
If this fails, a Secondo query is issued, which determines the
cardinality. This cardinality is then stored in local memory.

*/
card(Rel, Size) :-
  spelled(Rel, Rel2, _),
  storedCard(Rel2, Size),
  !.
/*
First letter of ~Rel~ is written in lower case.

*/
card(Rel, Size) :-
  spelled(Rel, Rel2, l),
  Query = (count(rel(Rel2, _, l))),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  secondo(QueryAtom, [int, Size]),
  assert(storedCard(Rel2, Size)),
  !.
/*
First letter of ~Rel~ is written in upper case.

*/
card(Rel, Size) :-
  spelled(Rel, Rel2, u),
  Query = (count(rel(Rel2, _, u))),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  secondo(QueryAtom, [int, Size]),
  assert(storedCard(Rel2, Size)),
  !.

card(_, _) :- fail.
/*
1.3.2 Storing And Loading Cardinalities

*/
readStoredCards :-
  retractall(storedCard(_, _)),
  [storedCards].  

writeStoredCards :-
  open('storedCards.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredCard(FD), _),
  close(FD).

writeStoredCard(Stream) :-
  storedCard(X, Y),
  write(Stream, storedCard(X, Y)),
  write(Stream, '.\n').

:-
  dynamic(storedCard/2),
  at_halt(writeStoredCards),
  readStoredCards.
/*
1.4 Looking Up For Existing Indexes

---- hasIndex(rel(Rel, _, _),attr(Attr, _, _), IndexName, IndexType) :-
----

If it exists, the index name for relation ~Rel~ and attribute ~Attr~
is ~IndexName~. The type of the index is ~IndexType~.

1.4.1 Auxiliary Rule

Checks whether an index exists for ~Rel~ and ~Attr~ in the currently
opened database. Depending on this result the dynamic predicate
~storedIndex/4~ or ~storedNoIndex/2~ is set. 

*/
verifyIndexAndStoreIndex(Rel, Attr, Index, IndexType) :- % Index exists
  getSecondoList(ObjList),
  member(['OBJECT', Index, _ , [[IndexType | _]]], ObjList),
    %write(Index),nl,
    %write(IndexType),nl, 
  assert(storedIndex(Rel, Attr, IndexType, Index)),
  !.

verifyIndexAndStoreNoIndex(Rel, Attr) :-      % No index
  downcase_atom(Rel, DCRel),
  downcase_atom(Attr, DCAttr),
  relation(DCRel, List),
  member(DCAttr, List),
  assert(storedNoIndex(Rel, Attr)).
/*
1.4.2 Look up Index

The first rule simply reduces an attribute of the form e.g. p:ort just 
to its attribute name e.g. ort.

*/

hasIndex(rel(Rel, _, _), attr(_:A, _, _), IndexName, Type) :-
  hasIndex(rel(Rel, _, _), attr(A, _, _), IndexName, Type),
  write('\nhasIndex 1\n').
/*

Gets the index name ~Index~ for relation ~Rel~ and attribute ~Attr~
via dynamic predicate ~storedIndex/4~.

*/

hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index, Type) :-
  storedIndex(Rel, Attr, Type, Index),
  !,
  write('\nhasIndex 2\n').

/*
If there is information stored in local memory, that there is no index
for relation ~Rel~ and attribute ~Attr~ then this rule fails.

*/

hasIndex(rel(Rel, _, _), attr(Attr, _, _), _, _) :-
  storedNoIndex(Rel, Attr),
  !,
  write('\nhasIndex 3\n'),
  fail.
/*
We have to differentiate the next rules, if the first letter of attribute 
name ~Attr~ is written in lower or in upper case and if there is an
index available for relation ~Rel~ and attribute ~Attr~.

*/
hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index, IndexType) :- %attr in lc
						                %rel in lc  						     
  not(Attr = _:_),                                              %succeeds
  spelled(Rel:Attr, attr(Attr2, 0, l)),              
  spelled(Rel, _, l),
  atom_concat(Rel, '_', Index1),
  atom_concat(Index1, Attr2, Index),
  verifyIndexAndStoreIndex(Rel, Attr, Index, IndexType),
  concat_atom(['let ', Index, '_small', ' = ', Rel, 
  '_small create', IndexType, ' [', Attr, ']'], '', QueryAtom),
  tryCreate(QueryAtom),    
  %write(QueryAtom),nl,
  write('\nhasIndex 4\n'),
  !.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index, IndexType) :- %attr in lc
                                                                %rel in uc
  not(Attr = _:_),                                              %succeeds
  spelled(Rel:Attr, attr(Attr2, 0, l)),  
  spelling(Rel, Spelled),
  Rel = Spelled,
  upper(Rel, URel),
  atom_concat(Rel, '_', Index1),
  atom_concat(Index1, Attr2, Index),
  verifyIndexAndStoreIndex(Rel, Attr, Index, IndexType),
  concat_atom(['let ', Index, '_small', ' = ', URel, 
  '_small create', IndexType, ' [', Attr, ']'], '', QueryAtom),
  tryCreate(QueryAtom),    
  %write(QueryAtom),nl,
  write('\nhasIndex 5\n'),
  !.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), _, _) :-     %attr in lc
                                                        %fails
  not(Attr = _:_),                                   
  spelled(Rel:Attr, attr(_, 0, l)),
  verifyIndexAndStoreNoIndex(Rel, Attr),
  !,
  write('\nhasIndex 6\n'), 
  fail.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index, IndexType) :- %attr in uc
                                            	                %rel in lc
  not(Attr = _:_),                                              %succeeds
  spelled(Rel:Attr, attr(Attr2, 0, u)),             
  spelled(Rel, _, l),
  upper(Attr2, SpelledAttr),
  atom_concat(Rel, '_', Index1),
  atom_concat(Index1, SpelledAttr, Index),
  verifyIndexAndStoreIndex(Rel, Attr, Index, IndexType),
  concat_atom(['let ', Index, '_small', ' = ', Rel, 
  '_small create', IndexType, ' [', SpelledAttr, ']'], '', QueryAtom),
  tryCreate(QueryAtom),    
  %write(QueryAtom),nl,
  write('\nhasIndex \7n'),
  !.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index, IndexType) :- %attr in uc
                                                                %rel in uc
  not(Attr = _:_),                                              %succeeds
  spelled(Rel:Attr, attr(Attr2, 0, u)),              
  spelling(Rel, Spelled),
  Rel = Spelled,
  upper(Rel, URel),
  upper(Attr2, SpelledAttr),
  atom_concat(Rel, '_', Index1),
  atom_concat(Index1, SpelledAttr, Index),
  verifyIndexAndStoreIndex(Rel, Attr, Index, IndexType),
  concat_atom(['let ', Index, '_small', ' = ', URel, 
  '_small create', IndexType, ' [', SpelledAttr, ']'], '', QueryAtom),
  tryCreate(QueryAtom),    
  %write(QueryAtom),nl,
  write('\nhasIndex 8\n'),
  !.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), _, _) :-
                                                 	%attr in uc
  not(Attr = _:_),                                      %fails
  spelled(Rel:Attr, attr(_, 0, u)),
  verifyIndexAndStoreNoIndex(Rel, Attr),
  !,
  write('\nhasIndex 9\n'), 
  fail.

/*
1.4.3 Storing And Loading About Existing Indexes

Storing and reading of  the two dynamic predicates ~storedIndex/4~ and 
~storedNoIndex/2~ in the file ~storedIndexes~.

*/
readStoredIndexes :-
  retractall(storedIndex(_, _, _, _)),
  retractall(storedNoIndex(_, _)),
  [storedIndexes].  

writeStoredIndexes :-
  open('storedIndexes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredIndex(FD), _),
  findall(_, writeStoredNoIndex(FD), _),
  close(FD).

writeStoredIndex(Stream) :-
  storedIndex(U, V, W, X),
  write(Stream, storedIndex(U, V, W, X)),
  write(Stream, '.\n').

writeStoredNoIndex(Stream) :-
  storedNoIndex(U, V),
  write(Stream, storedNoIndex(U, V)),
  write(Stream, '.\n').

:-
  dynamic(storedIndex/4),
  dynamic(storedNoIndex/2),
  at_halt(writeStoredIndexes),
  readStoredIndexes.
/*
1.5 Update Indexes And Relations

The next two predicates provide an update about known indexes and 
an update for informations about relations, which are stored in local 
memory.

1.5.1 Update Indexes

---- updateIndex(Rel, Attr) :-
----

The knowledge about an existing index for ~Rel~ and ~Attr~ in local memory 
is updated, if an index has been added or an index has been deleted. Note,
that all letters of ~Rel~ and ~Attr~ must be written in lower case.

*/
updateIndex :-
  retract(storeupdateIndex(0)),
  assert(storeupdateIndex(1)),
  secondo('list objects',[_, [_, [_ | ObjList]]]),
  retract(storedSecondoList(_)),
  assert(storedSecondoList(ObjList)),
  checkForAddedIndices(ObjList),
  checkForRemovedIndices(ObjList),
  retract(storeupdateIndex(1)),
  assert(storeupdateIndex(0)).

/*updateIndex(Rel, Attr) :- % add index on small relation
  spelled(Rel, SRel, _),  
  spelled(Rel:Attr, attr(Attr2, _, _)),
  storedNoIndex(SRel, Attr2),
  retract(storedNoIndex(SRel, Attr2)),  
  hasIndex(rel(SRel, _, _),attr(Attr2, _, _), _, _).

updateIndex(Rel, Attr) :- % delete index on small relation
  spelled(Rel, SRel, _),
  spelled(Rel:Attr, attr(Attr2, _, _)),
  storedIndex(SRel, Attr2, _, Index),
  %concat_atom([X,Y], '_', Index),
  retract(storedIndex(SRel, Attr2, _, Index)), 
  assert(storedNoIndex(SRel, Attr2)),
  concat_atom(['delete ', Index, '_small'], '', QueryAtom),
  secondo(QueryAtom).*/

/*
1.5.2 Update Relations

---- updateRel(Rel) :-
----

All information stored in local memory about relation ~Rel~ will
be deleted. The next query, issued on relation ~Rel~, will update
all needed information in local memory about ~Rel~. Note, that all
letters of relation ~Rel~ must be written in lower case.
 
*/
getRelAttrName(Rel, Arg) :-
  Arg = Rel:_.

getRelAttrName(Rel, Term) :-
  functor(Term, _, 1),
  arg(1, Term, Arg1),
  getRelAttrName(Rel, Arg1).

getRelAttrName(Rel, Term) :-
  functor(Term, _, 2),
  arg(1, Term, Arg1),
  getRelAttrName(Rel, Arg1).

getRelAttrName(Rel, Term) :-
  functor(Term, _, 2),
  arg(2, Term, Arg2),
  getRelAttrName(Rel, Arg2).

retractSels(Rel) :-
  storedSel(Term, _),
  arg(1, Term, Arg1),
  getRelAttrName(Rel, Arg1),
  retract(storedSel(Term, _)),
  retractSels(Rel).

retractSels(Rel) :-
  storedSel(Term, _),
  arg(2, Term, Arg2),
  getRelAttrName(Rel, Arg2),
  retract(storedSel(Term, _)),
  retractSels(Rel).

retractSels(_).

retractPETs(Rel) :-
  storedPET(Term, _),
  arg(1, Term, Arg1),
  getRelAttrName(Rel, Arg1),
  retract(storedPET(Term, _)),
  retractPETs(Rel).

retractPETs(Rel) :-
  storedPET(Term, _),
  arg(2, Term, Arg2),
  getRelAttrName(Rel, Arg2),
  retract(storedPET(Term, _)),
  retractPETs(Rel).

retractPETs(_).

getSpelledRel(Rel, SpelledRel) :-
  spelling(Rel, Spelled),
  Spelled = lc(SpelledRel),!.

getSpelledRel(Rel, SpelledRel) :-
  spelling(Rel, Spelled),
  upper(Spelled, SpelledRel),!.

getSpelledRel(_, _) :-
  write('ERROR: Relation Name Not Known!'),!,fail.

tryDeleteFile(Name, ObjList) :-
  member(['OBJECT', Name, _ , [[rel | _]]], ObjList),
  concat_atom(['delete ', Name], '', QueryAtom),
  secondo(QueryAtom).

tryDeleteFile(Name, ObjList) :- 
  not(member(['OBJECT', Name, _ , [[rel | _]]], ObjList)).

deleteSampleAndSmallFiles(SpelledRel, ObjList) :-
  sampleNameS(SpelledRel, SampleS),
  tryDeleteFile(SampleS, ObjList),
  sampleNameJ(SpelledRel, SampleJ),
  tryDeleteFile(SampleJ, ObjList),
  sampleNameSmall(SpelledRel, Small),
  tryDeleteFile(Small, ObjList).

retractStoredInformation(SpelledRel) :-
  sampleNameS(SpelledRel, SampleS),
  sampleNameJ(SpelledRel, SampleJ),
  lowerfl(SpelledRel,LFSpelledRel),
  downcase_atom(SpelledRel, DCSpelledRel),  
  lowerfl(SampleS, LFSampleS),
  downcase_atom(SampleS, DCSampleS),
  lowerfl(SampleJ, LFSampleJ),
  downcase_atom(SampleJ, DCSampleJ),
  concat_atom([LFSpelledRel,'_small'], '', Small),
  downcase_atom(Small, DCSmall),
  retractall(storedCard(LFSpelledRel, _)),
  retractall(storedCard(LFSampleS, _)),
  retractall(storedCard(LFSampleJ, _)),
  retractall(storedCard(Small, _)),
  retractall(storedTupleSize(LFSpelledRel, _)),
  retractall(storedSpell(DCSpelledRel, _)),
  retractall(storedSpell(DCSpelledRel:_, _)),
  retractall(storedSpell(DCSampleS, _)),
  retractall(storedSpell(DCSampleJ, _)),  
  retractall(storedSpell(DCSmall, _)),
  retractSels(Rel),
  retractPETs(Rel),
  retractall(storedRel(DCSpelledRel, _)).
  %retractall(storedIndex(LFSpelledRel, _, _, _)),
  %retractall(storedNoIndex(LFSpelledRel, _)).
  
updateRel2(_, SpelledRel, ObjList) :-
  member(['OBJECT', SpelledRel, _ , [[rel | _]]], ObjList),
  retractStoredInformation(SpelledRel),!.

updateRel2(_, SpelledRel, ObjList) :-
  not(member(['OBJECT', SpelledRel, _ , [[rel | _]]], ObjList)),
  deleteSampleAndSmallFiles(SpelledRel, ObjList),
  retractStoredInformation(SpelledRel).
    	
updateRel(Rel) :-
  retract(storeupdateRel(0)),
  assert(storeupdateRel(1)),
  secondo('list objects',[_, [_, [_ | ObjList]]]),
  retract(storedSecondoList(_)),
  assert(storedSecondoList(ObjList)),
  getSpelledRel(Rel, SpelledRel),
  updateRel2(Rel,SpelledRel, ObjList),
  retract(storeupdateRel(1)),
  assert(storeupdateRel(0)).  
  


  

/*updateRel(Rel) :- % rel in lc
  spelling(Rel, Spelled),
  Spelled = lc(Rel),
  sampleNameS(Rel, Sample1),
  concat_atom(['delete ', Sample1], '', QueryAtom1),
  tryDelete(QueryAtom1),
  sampleNameJ(Rel, Sample2),
  concat_atom(['delete ', Sample2], '', QueryAtom2),
  tryDelete(QueryAtom2),
  sampleNameSmall(Rel, Small),
  concat_atom(['delete ', Small], '',  QueryAtom3),
  tryDelete(QueryAtom3),
  retract(storedSecondoList(_)),
  getSecondoList(_),
  lowerfl(Sample1, LSample1),
  downcase_atom(Sample1, DCSample1),
  lowerfl(Sample2, LSample2),
  downcase_atom(Sample2, DCSample2),
  retractall(storedCard(Rel, _)),
  concat_atom([Rel,'_small'], '', Small2),
  retractall(storedCard(Small2, _)),
  retractall(storedTupleSize(Rel, _)),
  retractall(storedCard(LSample1, _)),
  retractall(storedCard(LSample2, _)),
  retractall(storedSpell(Rel, _)),
  retractall(storedSpell(Rel:_, _)),
  retractall(storedSpell(DCSample1, _)),
  retractall(storedSpell(DCSample2, _)),  
  retractall(storedSpell(Small2, _)),
  retractSels(Rel),
  retractPETs(Rel),
  retractall(storedRel(Rel, _)),
  retractall(storedIndex(Rel, _, _, _)),
  retractall(storedNoIndex(Rel, _)),!.*/

/*updateRel(Rel) :- % rel in uc
  spelled(Rel, Rel2, u),
  upper(Rel2, URel),
  sampleNameS(URel, Sample1),
  concat_atom(['delete ', Sample1], '', QueryAtom1),
  tryDelete(QueryAtom1),
  sampleNameJ(URel, Sample2),
  concat_atom(['delete ', Sample2], '', QueryAtom2),
  tryDelete(QueryAtom2),
  sampleNameSmall(URel, Small),
  concat_atom(['delete ', Small], '', QueryAtom3),
  tryDelete(QueryAtom3),
  retract(storedSecondoList(_)),
  getSecondoList(_),
  lowerfl(Sample1, LSample1),
  downcase_atom(Sample1, DCSample1),
  lowerfl(Sample2, LSample2),
  downcase_atom(Sample2, DCSample2),
  retractall(storedCard(Rel2, _)),
  concat_atom([Rel,'_small'], '', Small2),
  retractall(storedCard(Small2, _)),
  retractall(storedTupleSize(Rel2, _)),
  retractall(storedCard(LSample1, _)),
  retractall(storedCard(LSample2, _)),
  retractall(storedSpell(Rel, _)),
  retractall(storedSpell(Rel:_, _)),
  retractall(storedSpell(DCSample1, _)),
  retractall(storedSpell(DCSample2, _)),
  retractall(storedSpell(Small2, _)),
  retractSels(Rel2),
  retractPETs(Rel2),
  retractall(storedRel(Rel, _)),
  retractall(storedIndex(Rel2, _, _, _)),
  retractall(storedNoIndex(Rel2, _)).*/
/*
1.6 Average Size Of A Tuple

---- tuplesize(Rel, Size) :-

----

The average size of a tuple in Bytes of relation ~Rel~ 
is ~Size~.

1.6.1 Get The Tuple Size

Succeed or failure of this predicate is quite similar to
predicate ~card/2~, see section about cardinalities of
relations.

*/
tuplesize(Rel, Size) :-
  spelled(Rel, Rel2, _),
  storedTupleSize(Rel2, Size),
  !.
/*
First letter of ~Rel~ is written in lower case.

*/
tuplesize(Rel, Size) :-
  spelled(Rel, Rel2, l),
  Query = (tuplesize(rel(Rel2, _, l))),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  secondo(QueryAtom, [real, Size]),
  assert(storedTupleSize(Rel2, Size)),
  !.
/*
First letter of ~Rel~ is written in upper case.

*/
tuplesize(Rel, Size) :-
  spelled(Rel, Rel2, u),
  Query = (tuplesize(rel(Rel2, _, u))),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  secondo(QueryAtom, [real, Size]),
  assert(storedTupleSize(Rel2, Size)),
  !.

tuplesize(_, _) :- fail.
/*
1.6.2 Storing And Loading Tuple Sizes

*/
readStoredTupleSizes :-
  retractall(storedTupleSize(_, _)),
  [storedTupleSizes].  

writeStoredTupleSizes :-
  open('storedTupleSizes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredTupleSize(FD), _),
  close(FD).

writeStoredTupleSize(Stream) :-
  storedTupleSize(X, Y),
  write(Stream, storedTupleSize(X, Y)),
  write(Stream, '.\n').

:-
  dynamic(storedTupleSize/2),
  at_halt(writeStoredTupleSizes),
  readStoredTupleSizes.

% try to create/delete samples and ignore error codes.

tryCreate(QueryAtom) :- 
  secondo(QueryAtom), !.
  
tryCreate(_) :-
  write('Using existing object!' ), nl.

tryDelete(QueryAtom) :- 
  secondo(QueryAtom), !.
  
tryDelete(_).











