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
Rule ~extractAttrSizes(Rel, AttrList)~ builds a table of facts 
~storedAttrSize(Database, Rel, Attribute, Type, CoreTupleSize, InFlobSize)~ describing the 
~Type~ and tuple sizes of all ~Attribute~s found in ~AttrList~. To determine ~InFlobSize~,
a query is send to Secondo.

*/
extractAttrTypes(Rel, [[Attr, Type]]) :-
  noFlobType(Type), % Type is noFlobType: No Secondo query needed
  downcase_atom(Attr, AttrD),
  % determine CoreTupleSize
  secDatatype(Type, CoreTupleSize),
  databaseName(DBName),
  assert(storedAttrSize(DBName, Rel, AttrD, Type, CoreTupleSize, 0)), !.

extractAttrTypes(Rel, [[Attr, Type]]) :-
  downcase_atom(Attr, AttrD),
  % determine CoreTupleSize
  secDatatype(Type, CoreTupleSize),
  % query Secondo for average InFlobSize of Rel:Attr
  InFlobSize is 100,
  databaseName(DBName),
  assert(storedAttrSize(DBName, Rel, AttrD, Type, CoreTupleSize, InFlobSize)), !.

extractAttrTypes(Rel, [[Attr, Type] | Rest]) :-
  noFlobType(Type), % Type is noFlobType: No Secondo query needed
  downcase_atom(Attr, AttrD),
  % determine CoreTupleSize
  secDatatype(Type, CoreTupleSize),
  databaseName(DBName),
  assert(storedAttrSize(DBName, Rel, AttrD, Type, CoreTupleSize, 0)),
  extractAttrTypes(Rel, Rest), !.

extractAttrTypes(Rel, [[Attr, Type] | Rest]) :-
  downcase_atom(Attr, AttrD),
  % determine CoreTupleSize
  secDatatype(Type, CoreTupleSize),
  % query Secondo for average InFlobSize of Rel:Attr
  InFlobSize is 100,
  databaseName(DBName),
  assert(storedAttrSize(DBName, Rel, AttrD, Type, CoreTupleSize, InFlobSize)),
  extractAttrTypes(Rel, Rest), !.

extractAttrTypes(Rel, List) :- 
  write('ERROR in optimizer: extractAttrTypes('), write(Rel), 
  write(', '), write(List), write(') failed.\nl'), 
  fail.

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
  databaseName(DBName),
  assert(storedCard(DBName, Small, Card)),
  downcase_atom(Small, DCSmall),  
  assert(storedSpell(DBName, DCSmall, lc(Small))),
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
  databaseName(DBName),
  assert(storedCard(DBName, LSmall, Card)),
  downcase_atom(Small, DCSmall),
  assert(storedSpell(DBName, DCSmall, LSmall)),
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
  databaseName(DBName),
  SmallCard is truncate(min(Card, max(1000, Card*0.1))),  
  assert(storedCard(DBName, Small, SmallCard)),
  downcase_atom(Small, DCSmall),  
  assert(storedSpell(DBName, DCSmall, lc(Small))),
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
  databaseName(DBName),
  SmallCard is truncate(min(Card, max(1000, Card*0.1))),  
  lowerfl(Small, LSmall),
  assert(storedCard(DBName, LSmall, SmallCard)),
  downcase_atom(Small, DCSmall),
  assert(storedSpell(DBName, DCSmall, LSmall)),
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
  databaseName(DBName),
  SmallCard is truncate(min(Card, max(10000, Card*0.01))),  
  assert(storedCard(DBName, Small, SmallCard)),
  downcase_atom(Small, DCSmall),  
  assert(storedSpell(DBName, DCSmall, lc(Small))),
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
  databaseName(DBName),
  SmallCard is truncate(min(Card, max(10000, Card*0.01))),  
  lowerfl(Small, LSmall),
  assert(storedCard(DBName, LSmall, SmallCard)),
  downcase_atom(Small, DCSmall),
  assert(storedSpell(DBName, DCSmall, LSmall)),
  !.

/*
Creates a sample relation, for determining the selectivity of a relation 
object for a given predicate. The first two rules consider the case, that 
there is a sample relation already available and the last two ones create 
new relations by sending a Secondo ~let~-command.

*/
createSampleRelation(Rel, ObjList) :-  % Rel in lc
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleNameS(Rel3, Sample1),
  member(['OBJECT', Sample1, _ , [[_ | _]]], ObjList),
  sampleNameJ(Rel3, Sample2),
  member(['OBJECT', Sample2, _ , [[_ | _]]], ObjList),
  !.

createSampleRelation(Rel, ObjList) :-  % Rel in uc
  spelling(Rel, Rel2),
  not(Rel2 = lc(_)),
  upper(Rel2, URel),
  sampleNameS(URel, Sample1),
  member(['OBJECT', Sample1, _ , [[_ | _]]], ObjList),
  sampleNameJ(URel, Sample2),
  member(['OBJECT', Sample2, _ , [[_ | _]]], ObjList),
  !.

createSampleRelation(Rel, _)  :-  % Rel in lc
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
  databaseName(DBName),
  SampleCard1 is truncate(min(Card, max(2000, Card*0.00001))),
  SampleCard2 is truncate(min(Card, max(500, Card*0.00001))),
  assert(storedCard(DBName, Sample1, SampleCard1)),
  assert(storedCard(DBName, Sample2, SampleCard2)),
  downcase_atom(Sample1, DCSample1),
  downcase_atom(Sample2, DCSample2),  
  assert(storedSpell(DBName, DCSample1, lc(Sample1))),
  assert(storedSpell(DBName, DCSample2, lc(Sample2))),
  !.

createSampleRelation(Rel, _) :-  % Rel in uc
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
  databaseName(DBName),
  SampleCard1 is truncate(min(Card, max(2000, Card*0.00001))),
  lowerfl(Sample1, LSample1),
  assert(storedCard(DBName, LSample1, SampleCard1)),
  SampleCard2 is truncate(min(Card, max(500, Card*0.00001))),
  lowerfl(Sample2, LSample2),
  assert(storedCard(DBName, LSample2, SampleCard2)),
  downcase_atom(DBName, Sample1, DCSample1),
  assert(storedSpell(DBName, DCSample1, LSample1)),
  downcase_atom(Sample2, DCSample2),
  assert(storedSpell(DBName, DCSample2, LSample2)),
  !.
/*
Checks, if an index exists for ~Rel~ and ~Attr~ and stores the 
respective values to the dynamic predicates ~storedIndex/5~ or 
~storedNoIndex/3~.

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

checkForIndex(_, []).

checkForIndex(Rel, [First|Rest]) :-
  updateIndex(Rel, First),
  checkForIndex(Rel, Rest).

relation(Rel, AttrList) :-
  databaseName(DBName),
  storedRel(DBName, Rel, AttrList),
  %checkForIndex(Rel, AttrList),
  !.

relation(Rel, AttrList) :-
  getSecondoList(ObjList),
  databaseName(DBName),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [AttrList2]]]]]]], ObjList),
  downcase_atom(ORel, DCRel),
  DCRel = Rel,
  extractList(AttrList2, AttrList3),
  downcase_list(AttrList3, AttrList),
  assert(storedRel(DBName, Rel, AttrList)),
  spelling(Rel, _),
  createSmallRelation(Rel, ObjList),
  createAttrSpelledAndIndexLookUp(Rel, AttrList3),
  card(Rel, _),
  tuplesize(Rel, _),
  createSampleRelation(Rel, ObjList),
  extractAttrTypes(Rel, AttrList2).
  %retract(storedSecondoList(ObjList)).

/*
1.1.3 Storing And Loading Relation Schemas

*/
readStoredRels :-
  retractall(storedRel(_, _, _)),
  [storedRels].  

writeStoredRels :-
  open('storedRels.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredRel(FD), _),
  close(FD).

writeStoredRel(Stream) :-
  storedRel(N, X, Y),
  write(Stream, storedRel(N, X, Y)),
  write(Stream, '.\n').

showStoredRel :-
  storedRel(N, X, Y),
  write(N), write('.'), write(X), write(':\t'), write(Y), nl.

showStoredRels :- 
  nl, write('Stored relation schemas:\n'),
  findall(_, showStoredRel, _).

:-
  dynamic(storedRel/3),
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

~Spelled~ is available via the dynamic predicate ~storedSpell/3~.

*/
spelling(Rel:Attr, Spelled) :-
  databaseName(DBName),
  storedSpell(DBName, Rel:Attr, Spelled),
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
  databaseName(DBName),
  assert(storedSpell(DBName, Rel:Attr, lc(OAttr))),
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
  databaseName(DBName),
  assert(storedSpell(DBName, Rel:Attr, Spelled)),
  !.

spelling(_:_, _) :- !, fail.
/*
1.2.3 Spelling Of Relation Names

---- spelling(Rel, Spelled) :-
----

The spelling of relation ~Rel~ is ~Spelled~.

~Spelled~ is available via the dynamic predicate ~storedSpell/3~.

*/  
spelling(Rel, Spelled) :-
  databaseName(DBName),
  storedSpell(DBName, Rel, Spelled),
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
  databaseName(DBName),
  assert(storedSpell(DBName, Rel, lc(ORel))),
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
  databaseName(DBName),
  assert(storedSpell(DBName, Rel, Spelled)),
  !.
/*
1.2.4 Storing And Loading Of Spelling

*/  
readStoredSpells :-
  retractall(storedSpell(_, _, _)),
  [storedSpells]. 

writeStoredSpells :-
  open('storedSpells.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredSpell(FD), _),
  close(FD).

writeStoredSpell(Stream) :-
  storedSpell(N, X, Y),
  write(Stream, storedSpell(N, X, Y)),
  write(Stream, '.\n').

:-
  dynamic(storedSpell/3),
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
dynamic predicate ~storedCard/3~ (automatically stored).
If this fails, a Secondo query is issued, which determines the
cardinality. This cardinality is then stored in local memory.

*/
card(Rel, Size) :-
  databaseName(DBName),
  storedCard(DBName, Rel, Size),
  !.
/*
First letter of ~Rel~ is written in lower case.

*/
card(Rel, Size) :-
  databaseName(DBName),
  spelled(Rel, Rel2, l),
  Query = (count(rel(Rel2, _, l))),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  secondo(QueryAtom, [int, Size]),
  assert(storedCard(DBName, Rel2, Size)),
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
  databaseName(DBName),
  assert(storedCard(DBName, Rel2, Size)),
  !.

card(_, _) :- fail.
/*
1.3.2 Storing And Loading Cardinalities

*/
readStoredCards :-
  retractall(storedCard(_, _, _)),
  [storedCards].  

writeStoredCards :-
  open('storedCards.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredCard(FD), _),
  close(FD).

writeStoredCard(Stream) :-
  storedCard(N, X, Y),
  write(Stream, storedCard(N, X, Y)),
  write(Stream, '.\n').

:-
  dynamic(storedCard/3),
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
~storedIndex/5~ or ~storedNoIndex/3~ is set. 

*/
verifyIndexAndStoreIndex(Rel, Attr, Index, IndexType) :- % Index exists
  getSecondoList(ObjList),
  member(['OBJECT', Index, _ , [[IndexType | _]]], ObjList),
    %write(Index),nl,
    %write(IndexType),nl, 
  databaseName(DBName),
  assert(storedIndex(DBName, Rel, Attr, IndexType, Index)),
  !.

verifyIndexAndStoreNoIndex(Rel, Attr) :-      % No index
  downcase_atom(Rel, DCRel),
  downcase_atom(Attr, DCAttr),
  relation(DCRel, List),
  member(DCAttr, List),
  databaseName(DBName),
  assert(storedNoIndex(DBName, Rel, Attr)).
/*
1.4.2 Look up Index

The first rule simply reduces an attribute of the form e.g. p:ort just 
to its attribute name e.g. ort.

*/
hasIndex(rel(Rel, _, _), attr(_:A, _, _), IndexName, _) :-
  hasIndex(rel(Rel, _, _), attr(A, _, _), IndexName, _).
/*
Gets the index name ~Index~ for relation ~Rel~ and attribute ~Attr~
via dynamic predicate ~storedIndex/5~.

*/
hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index, Type) :-
  databaseName(DBName),
  storedIndex(DBName, Rel, Attr, Type, Index),
  !.
/*
If there is information stored in local memory, that there is no index
for relation ~Rel~ and attribute ~Attr~ then this rule fails.

*/
hasIndex(rel(Rel, _, _), attr(Attr, _, _), _, _) :-
  databaseName(DBName),
  storedNoIndex(DBName, Rel, Attr),
  !,
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
  !.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), _, _) :-     %attr in lc
                                                        %fails
  not(Attr = _:_),                                   
  spelled(Rel:Attr, attr(_, 0, l)),
  verifyIndexAndStoreNoIndex(Rel, Attr),
  !, fail.

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
  !.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), _, _) :-     %attr in uc
  not(Attr = _:_),                                      %fails
  spelled(Rel:Attr, attr(_, 0, u)),
  verifyIndexAndStoreNoIndex(Rel, Attr),
  !, fail.

/*
1.4.3 Storing And Loading About Existing Indexes

Storing and reading of  the two dynamic predicates ~storedIndex/5~ and 
~storedNoIndex/3~ in the file ~storedIndexes~.

*/
readStoredIndexes :-
  retractall(storedIndex(_, _, _, _, _)),
  retractall(storedNoIndex(_, _, _)),
  [storedIndexes].  

writeStoredIndexes :-
  open('storedIndexes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredIndex(FD), _),
  findall(_, writeStoredNoIndex(FD), _),
  close(FD).

writeStoredIndex(Stream) :-
  storedIndex(D, U, V, W, X),
  write(Stream, storedIndex(D, U, V, W, X)),
  write(Stream, '.\n').

writeStoredNoIndex(Stream) :-
  storedNoIndex(D, U, V),
  write(Stream, storedNoIndex(D, U, V)),
  write(Stream, '.\n').

:-
  dynamic(storedIndex/5),
  dynamic(storedNoIndex/3),
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
updateIndex(Rel, Attr) :- % add index on small relation
  spelled(Rel, SRel, _),  
  spelled(Rel:Attr, attr(Attr2, _, _)),
  databaseName(DBName),
  storedNoIndex(DBName, SRel, Attr2),
  retract(storedNoIndex(DBName, SRel, Attr2)),  
  hasIndex(rel(SRel, _, _),attr(Attr2, _, _), _, _).

updateIndex(Rel, Attr) :- % delete index on small relation
  spelled(Rel, SRel, _),
  spelled(Rel:Attr, attr(Attr2, _, _)),
  databaseName(DBName),
  storedIndex(DBName, SRel, Attr2, _, Index),
  %concat_atom([X,Y], '_', Index),
  retract(storedIndex(DBName, SRel, Attr2, _, Index)), 
  assert(storedNoIndex(DBName, SRel, Attr2)),
  concat_atom(['delete ', Index, '_small'], '', QueryAtom),
  secondo(QueryAtom).

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
  databaseName(DB),
  storedSel(DB, Term, _),
  arg(1, Term, Arg1),
  getRelAttrName(Rel, Arg1),
  retract(storedSel(DB, Term, _)),
  retractSels(Rel).

retractSels(Rel) :-
  databaseName(DB),
  storedSel(DB, Term, _),
  arg(2, Term, Arg2),
  getRelAttrName(Rel, Arg2),
  retract(storedSel(DB, Term, _)),
  retractSels(Rel).

retractSels(_).

retractPETs(Rel) :-
  databaseName(DB),
  storedPET(DB, Term, _, _, _, _, _, _, _),
  arg(1, Term, Arg1),
  getRelAttrName(Rel, Arg1),
  retract(storedPET(DB, Term, _, _, _, _, _, _, _)),
  retractPETs(Rel).

retractPETs(Rel) :-
  databaseName(DB),
  storedPET(DB, Term, _, _, _, _, _, _, _),
  arg(2, Term, Arg2),
  getRelAttrName(Rel, Arg2),
  retract(storedPET(DB, Term, _, _, _, _, _, _, _)),
  retractPETs(Rel).

retractPETs(_).
  
updateRel(Rel) :- % rel in lc
  databaseName(DB),
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
  retractall(storedCard(DB, Rel, _)),
  concat_atom([Rel,'_small'], '', Small2),
  retractall(storedCard(DB, Small2, _)),
  retractall(storedTupleSize(DB, Rel, _)),
  retractall(storedCard(DB, LSample1, _)),
  retractall(storedCard(DB, LSample2, _)),
  retractall(storedSpell(DB, Rel, _)),
  retractall(storedSpell(DB, Rel:_, _)),
  retractall(storedSpell(DB, DCSample1, _)),
  retractall(storedSpell(DB, DCSample2, _)),  
  retractall(storedSpell(DB, Small2, _)),
  retractall(storedSampleRuntimes(DB, DCSample2, _, _, _, _)),
  retractall(storedSampleRuntimes(DB, _, DCSample2, _, _, _)),
  retractSels(Rel),
  retractPETs(Rel),
  retractall(storedRel(DB, Rel, _)),
  retractall(storedIndex(DB, Rel, _, _, _)),
  retractall(storedNoIndex(DB, Rel, _)),
  retractall(storedAttrSize(DB, Rel, _, _, _, _)),!.

updateRel(Rel) :- % rel in uc
  databaseName(DB),
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
  retractall(storedCard(DB, Rel2, _)),
  concat_atom([Rel,'_small'], '', Small2),
  retractall(storedCard(DB, Small2, _)),
  retractall(storedTupleSize(DB, Rel2, _)),
  retractall(storedCard(DB, LSample1, _)),
  retractall(storedCard(DB, LSample2, _)),
  retractall(storedSpell(DB, Rel, _)),
  retractall(storedSpell(DB, Rel:_, _)),
  retractall(storedSpell(DB, DCSample1, _)),
  retractall(storedSpell(DB, DCSample2, _)),
  retractall(storedSpell(DB, Small2, _)),
  retractall(storedSampleRuntimes(DB, DCSample2, _, _, _, _)),
  retractall(storedSampleRuntimes(DB, _, DCSample2, _, _, _)),
  retractSels(Rel2),
  retractPETs(Rel2),
  retractall(storedRel(DB, Rel, _)),
  retractall(storedIndex(DB, Rel2, _, _, _)),
  retractall(storedNoIndex(DB, Rel2, _)),
  retractall(storedAttrSize(DB, Rel, _, _, _, _)),!.

/*
1.6 Tuple and Attribute Sizes

Tuplesizes are used within the operator cost functions of the 
optimizer to estimate the amount of tuples that can be handled
in-memory. As during a query tuplesizes of the streams may change 
due to join, project, extend and remove operations, information
on the size of datatype instances is used to keep track of the
actual tuplesizes. Secondo automatically generates a table with
the coretuple sizes, which is imported from a file. Additionally,
Secondo is queried for the average size of inline-flobs of any
attribute encountered in the relation schemas.

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
  databaseName(DB),
  storedTupleSize(DB, Rel, Size),
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
  databaseName(DB),
  assert(storedTupleSize(DB, Rel2, Size)),
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
  databaseName(DB),
  assert(storedTupleSize(DB, Rel2, Size)),
  !.

tuplesize(_, _) :- fail.
/*
1.6.2 Storing And Loading Average Overall  Tuple Sizes

*/
readStoredTupleSizes :-
  retractall(storedTupleSize(_, _, _)),
  [storedTupleSizes].  

writeStoredTupleSizes :-
  open('storedTupleSizes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredTupleSize(FD), _),
  close(FD).

writeStoredTupleSize(Stream) :-
  storedTupleSize(DB, X, Y),
  write(Stream, storedTupleSize(DB, X, Y)),
  write(Stream, '.\n').

:-
  dynamic(storedTupleSize/3),
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

/*
1.6.3 Loading Datatype Core Tuple Sizes

This data does not need to be stored, as it will be generated by
Secondo at startup.

*/
readStoredTypeSizes :-
  retractall(secDatatype(_, _)),
  [storedTypeSizes].  

showStoredTypeSize :-
  secDatatype(X, Y),
  write(X), write(': '), write(Y), write(' byte\n').

showStoredTypeSizes :-
  findall(_, showStoredTypeSize, _).

:-
  dynamic(secDatatype/2),
  readStoredTypeSizes.

/*
1.6.4 Showing, Loading and Storing Attribute Sizes

*/

readStoredAttrSizes :-
  retractall(storedAttrSize(_, _, _, _, _, _)),
  [storedAttrSizes].  

writeStoredAttrSizes :-
  open('storedAttrSizes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredAttrSize(FD), _),
  close(FD).

writeStoredAttrSize(Stream) :-
  storedAttrSize(Database, Rel, Attr, Type, CoreSize, InFlobSize),
  write(Stream, storedAttrSize(Database, Rel, Attr, Type, CoreSize, InFlobSize)),
  write(Stream, '.\n').

showStoredAttrSize :-
  storedAttrSize(Database, Rel, Attr, Type, CoreSize, InFlobSize),
  write(Database), write('.'), write(Rel), write('.'), write(Attr), write(': \t'), write(Type), 
  write(' ('), write(CoreSize),write('/'), write(InFlobSize), write(')\n').

showStoredAttrSizes :-
  write('Stored attribute sizes\nRel.Attr: Type (CoreTupleSize/Avg.InlineFlobSize) [byte]:\n'),
  findall(_, showStoredAttrSize, _).

:-
  dynamic(storedAttrSize/6),
  at_halt(writeStoredAttrSizes),
  readStoredAttrSizes.

/*
2. Machine Dependent Information

[File ~database.pl~]

The optimizer needs to know some details on the machine, SECONDO runs on.

2.1 Inquire machine speed constants

The machine speed has strong impact on the cost of executing database operations.
All constants used in costs estimation are with regard to a reference system.
Actual cost are calculated by applying factors for the actuaql system's CPU- and
FS-speed. These factors are measured once by sending queries to SECONDO and stored
on disk for later use. 

*/

/*
~machineSpeedFactor(CPU,FS)~ unifies ~CPU~ with the current machine's CPU speed and 
~FS~ with its FS speed with regard to some reference system. The predicate can be used 
to read the machine's relative speed and scale costs within the optimizer's cost functions.

Calling predicate ~toggleSpeed~ will toggle between the actual (stored) cost factors and 
uniform cost factors of 1.0 (e.g. for testing of cost functions).

*/

machineSpeedFactor(1,1) :- 
  uniformMachineSpeedFactor,
  !.

machineSpeedFactor(CPU, FS) :-
  storedMachineSpeedFactor(CPU, FS),
  !.

machineSpeedFactor(CPU, FS) :-
  setMachineSpeedFactor,
  storedMachineSpeedFactor(CPU, FS),
  !.
 
/* 

2.1.1 Auxiliary predicates to ~machineSpeedFactor/2~

*/

referenceSpeed(2489.74, 39696.67).        % (CPUtime, FStime) determine the times needed by the reference system

toggleSpeed :-
 uniformMachineSpeedFactor,
 retract(uniformMachineSpeedFactor),
 write('\nNow using actual machineSpeedFactor:'),
 machineSpeedFactor,
 !.

toggleSpeed :-
 not(uniformMachineSpeedFactor),
 assert(uniformMachineSpeedFactor),
 write('\nNow using uniform machineSpeedFactor:'),
 machineSpeedFactor,
 !.
 
/*
~queryTime(Query, TimeMS)~ unifies ~TimeMS~ with the time in ms that it takes SECONDO to run ~Query~

*/

queryTime(Query, TimeMS) :- 
  atom_concat('query ', Query, SecQuery),
  get_time(Time1),
  secondo(SecQuery, _),   
  get_time(Time2),
  Time is (Time2 - Time1),
  convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  TimeMS is (Minute * 60000) + (Sec*1000) + MilliSec,
  !.

 
/*

The predicate ~fibonacci(N, M)~ recursively calculates the ~N~th Fibonacci number ~M~. This 
extremely expensive function is used to measure CPU speed.

~determineCostCPU(Time)~ unifies ~Time~ with the average time in ms for evaluating ~fibonacci(31, X)~
trice.

*/

fibonacci(1, 1) :- !.
fibonacci(2, 1) :- !.
fibonacci(N, M) :-
  N1 is N - 1,
  N2 is N - 2,
  fibonacci(N1, M1),
  fibonacci(N2, M2),
  M is M1 + M2,
  !.

determineCostCPU(Cost) :-
  get_time(Time1),
  fibonacci(31, _),
  get_time(Time2),
  Time is Time2 - Time1,
  convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  Cost is Minute *60000 + Sec*1000 + MilliSec,
  !. 
 
ensureDatabaseOptOpen :- 
  isDatabaseOpen,
  secondo('close database',_),
  open('database opt'),
  !.

ensureDatabaseOptOpen :- 
  notIsDatabaseOpen,
  open('database opt'),
  !.

ensureDatabaseOptOpen :- 
  nl, write('ERROR in optimizer: ensureDatabaseOptOpen/0 failed.'), 
  nl, write('      Optimizer requires database opt to be properly installed.'), nl,
  fail.

/*
~setMachineSpeedFactor/0~ is run to initialize ~storedMachineSpeedFactor/2~.
~updateMachineSpeedFactor/0~ can be invoked to update the stored speed factors.
~machineSpeedFactor/0~ will print the stored speed factors.

*/

setMachineSpeedFactor :-
  storedMachineSpeedFactor( _, _),
  !.

setMachineSpeedFactor :-
  not(storedMachineSpeedFactor( _, _)),
  referenceSpeed(CPUref, FSref),
  determineCostCPU(C1),
  determineCostCPU(C2),
  determineCostCPU(C3),
  CPUtime is (C1 + C2 + C3) / 3,
  % CAUTION: The opt database will be opened and closed!
  ensureDatabaseOptOpen,
  FSquery = 'plz feed {p1} filter[.PLZ_p1 < 15000] plz feed {p2} filter[.PLZ_p2 < 15000] symmjoin[.Ort_p1 = ..Ort_p2] count',
  queryTime(FSquery, _),          % Execute a query with high FS cost  (large relation, large tuplesize, trivial predicate)
  queryTime(FSquery, FStime),     %   Do this twice and used second result to avoid errors caused by buffering effects
  FactorCPU is CPUtime / CPUref,  % Factors > 1 means the host is slower than the reference system
  FactorFS  is FStime  / FSref,      
  assert(storedMachineSpeedFactor(FactorCPU, FactorFS)), % store in tuple machineSpeedFactor(FactorCPU, FactorFS)
  secondo('close database'),
  !.

setMachineSpeedFactor :- 
  nl, write('ERROR in optimizer:setMachineSpeedFactor/0 failed'), nl,
  fail.

updateMachineSpeedFactor :-
  retract(storedMachineSpeedFactor( _, _)),
  setMachineSpeedFactor,
  !.

machineSpeedFactor :-
  machineSpeedFactor(CPU, FS),
  storedMachineSpeedFactor(CPU2, FS2),
  nl, write('Machine speed factors:'),
  nl, write(' Factor\tStored \tUsed \n'),
  write(' CPU=\t'), write(CPU2), write('\t'), write(CPU), nl,
  write(' FS=\t'), write(FS2), write('\t'), write(FS), nl,
  write('(Type \'toggleSpeed\' to change between actual and uniform machineSpeedFactors.)'), nl,
  !.


/*
2.1.2  Storing and Loading Machine Speed Data

Data stored in ~MachineSpeedFactor(X,Y)~ can be written to and read from disk using ~writeMachineSpeedFactor/0~ and
~readMachineSpeedFactor/0~. At the optimizer's startup, the file ~xstoredMachineSpeedFactor.pl~ will be consulted to
retrieve a saved ~machineSpeedFactor~.

*/

readMachineSpeedFactor :-
  retractall(storedMachineSpeedFactor(_, _)),
  [xstoredMachineSpeedFactor].

writeMachineSpeedFactor :-
  open('xstoredMachineSpeedFactor.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  storedMachineSpeedFactor(X, Y),
  write(FD, storedMachineSpeedFactor(X, Y)),
  write(FD, '.\n'),
  close(FD).

:-  dynamic(uniformMachineSpeedFactor/0).
:-  dynamic(storedMachineSpeedFactor/2).
:-  at_halt(writeMachineSpeedFactor).
:-  readMachineSpeedFactor.
:-  setMachineSpeedFactor.


/*
Some preticates for testing

*/


wqt(Query) :- % write QueryTime for testing
  atom_concat('query ', Query, SecQuery),
  get_time(Time1),
  secondo(SecQuery, _),   
  get_time(Time2),
  Time is (Time2 - Time1),
  convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  TimeMS is (Minute * 60000) + (Sec*1000) + MilliSec,
  nl, write(TimeMS), write(' ms'), nl,
  !.

mqt2(Query, 1, Result) :-         % auxiliary predicate to mqt/2
  atom_concat('query ', Query, SecQuery),
  get_time(Time1),
  secondo(SecQuery, _),   
  get_time(Time2),
  Time is (Time2 - Time1),
  convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  Result is (Minute * 60000) + (Sec*1000) + MilliSec,
  !.

mqt2(Query, N, Result) :- % auxiliary predicate to mqt/2
  N2 is N - 1,
  mqt2(Query, N2, Result2),
  atom_concat('query ', Query, SecQuery),
  get_time(Time1),
  secondo(SecQuery, _),   
  get_time(Time2),
  Time is (Time2 - Time1),
  convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  Result3 is (Minute * 60000) + (Sec*1000) + MilliSec,
  Result is min(Result2, Result3),
  !.

mqt(Query, N) :-      % write minimal execution time of N executions of Query (for testing)
  mqt2(Query, N, Result),
  nl, write(Result), write(' ms'), nl,
  !.

