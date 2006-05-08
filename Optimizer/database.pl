/*
//[<] [$<$]
//[>] [$>$]
//[%] [\%]
//[->] [$\rightarrow$]

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
Rule ~extractAttrTypes(Rel, AttrList)~ builds a table of facts 
~storedAttrSize(Database, Rel, Attribute, Type, CoreTupleSize, InFlobSize, 
ExtFlobSize)~ describing the ~Type~ and tuple sizes of all ~Attribute~s 
found in ~AttrList~. To determine ~InFlobSize~, a query should be send to 
Secondo, but that operator is still not implemented.

Operators to determine attribute sizes in Secondo:

  * ~ attrsize ~
   ( stream | rel(tuple X) x ident [->] real ), 
   Return the size of the attribute within a stream or a 
   relation taking into account the FLOBs.

  * ~exattrsize~ -
   ( stream(tuple X) | rel(tuple X) x identifier [->] real ), 
   Return the size of the attribute within a stream or a 
   relation taking into account the small FLOBs.

  * ~rootattrsize~ - 
   ( stream(tuple X) | rel(tuple X) x identifier [->]  int ), 
   Return the size of the attributes root part within a 
   stream or a relation (without small FLOBs).

  * ~tuplesize~ -
   ( stream | rel (tuple x) [->] real ), 
   Return the average size of the tuples within a stream or a 
   relation taking into account the FLOBs.

  * ~exttuplesize~ - 
   ( stream(tuple X) | rel(tuple X) [->] real ), 
   Return the average size of the tuples within a stream 
   or a relation taking into account the small FLOBs.

  * ~roottuplesize~ - 
   ( stream(tuple X) | rel(tuple X) [->]  int ), 
   Return the size of the attributes root part within a 
   stream or a relation (without small FLOBs).

Simple datatypes ~X~, that have no FLOBs and for which ~noFlobType(X)~ 
in file ``operators.pl'' is defined, will be assumed to have no FLOBs.
Instead of querying Secondo for the attribute sizes, the rootsizes stored 
in file ``storedTypeSizes.pl'' for each Secondo datatype are used.

*/

% query Secondo for root attr size of an attribute
getAttrSize(Rel, Attr, AttrSize) :-
  secAttr(Rel, Attr, AttrE),
  secRelation(Rel, RelE),
  concat_atom(['query ', RelE, ' attrsize[ ', AttrE, ' ]'],QueryAtom),
  secondo(QueryAtom, [real, AttrSize]), !.
getAttrSize(Rel, Attr, _) :-
  write('\nERROR: Something\'s wrong in getAttrSize('), write(Rel),
  write(','), write(Attr), write(').'),
  fail, !.

% query Secondo for root attr size of an attribute
getRootAttrSize(Rel, Attr, RootAttrSize) :-
  secAttr(Rel, Attr, AttrE),
  secRelation(Rel, RelE),
  concat_atom(['query ', RelE, ' rootattrsize[ ', AttrE, ' ]'],QueryAtom),
  secondo(QueryAtom, [int, RootAttrSize]), !.
getRootAttrSize(Rel, Attr, _) :-
  write('\nERROR: Something\'s wrong in getRootAttrSize('), write(Rel),
  write(','), write(Attr), write(').'),
  fail, !.

% query Secondo for the extattrsize of an attribute
getExtAttrSize(Rel, Attr, ExtAttrSize) :- 
  secAttr(Rel, Attr, AttrE),
  secRelation(Rel, RelE),
  concat_atom(['query ', RelE, ' extattrsize[ ', AttrE, ' ]'],QueryAtom),
  secondo(QueryAtom, [real, ExtAttrSize]), !.
getExtAttrSize(Rel, Attr, _) :- 
  write('\nERROR: Something\'s wrong in getExtAttrSize('), write(Rel),
  write(','), write(Attr), write(').'),
  fail, !.


% extract attribute types and retrieve attribute detailed sizes
extractAttrTypes(_, []) :- !.

extractAttrTypes(Rel, [[Attr, Type] | Rest]) :-
  ( noFlobType(Type) 
    *-> ( secDatatype(Type, CoreAttrSize),
          InFlobSize   is 0,
          ExtFlobSize  is 0
        )
      ; (
          getAttrSize(Rel, Attr, AttrSize),
          getExtAttrSize(Rel, Attr, ExtAttrSize),
          getRootAttrSize(Rel, Attr, RootAttrSize),
          CoreAttrSize is RootAttrSize,
          InFlobSize   is ExtAttrSize - RootAttrSize,
          ExtFlobSize  is AttrSize - ExtAttrSize
        )
  ),
  databaseName(DBName),
  downcase_atom(Attr, AttrD),
  assert(storedAttrSize(DBName, Rel, AttrD, Type, CoreAttrSize, InFlobSize,
    ExtFlobSize)), !, 
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

/*
Create small relations for use with the entropy optimizer. Relations are classified into three groups called ~small~, ~middle~, and ~large~ by the ~classify~ predicate. For each group, sample sizes can be set differently. Currently ~small~ sizes are determined as follows:

  * small = less than 1000 tuples: full size

  * middle = between 1000 and 100000 tuples: 10 [%], but at least 1000 tuples

  * large = more than 100000 tuples, 1 [%], but at least 10000 tuples

This schema is chosen to have sample sizes grow monotonically.

In addition, an attribute is added for uniquely identifying tuples in the small relations with name ~xxxID[<]relname[>]~ (~[<]relname[>]~ starting with a lower case letter). This is needed for selfjoin correction in the entropy optimizer.

*/

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
  buildSmallRelation(Rel3, Rel3, 0, _),
  !.

createSmallRelation(Rel, _) :-  % Rel in uc
  classifyRel(Rel, small),
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  buildSmallRelation(Rel2, URel, 0, _),
  !.

createSmallRelation(Rel, _)  :-  % Rel in lc
  classifyRel(Rel, middle),
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  buildSmallRelation(Rel3, Rel3, 1000, 0.1),
  !.

createSmallRelation(Rel, _)  :-  % Rel in uc
  classifyRel(Rel, middle),
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  buildSmallRelation(Rel2, URel, 1000, 0.1),
  !.

createSmallRelation(Rel, _)  :-  % Rel in lc
  classifyRel(Rel, large),
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  buildSmallRelation(Rel3, Rel3, 10000, 0.01),
  !.

createSmallRelation(Rel, _)  :-  % Rel in uc
  classifyRel(Rel, large),
  spelling(Rel, Rel2),
  upper(Rel2, URel),

  buildSmallRelation(Rel2, URel, 10000, 0.01),
  !.


/*
----	buildSmallRelation(+RelLC, +RelName, +MinSize, +Percent) :-
----

Build a small relation where ~RelLC~ is the relation name with a lower case first letter, ~RelName~ is the Secondo relation name, ~MinSize~ is the desired minimal size, and ~Percent~ is the desired minimal percentage of tuples for use in the sample operator. Add a unique identifier attribute ~xxxID[<]relname[>]~ by numbering tuples sequentially. If ~MinSize~ = 0, no sampling is needed.

*/
buildSmallRelation(RelLC, RelName, 0, _) :-
  sampleNameSmall(RelName, Small),
  atom_concat('xxxID', RelLC, IDAttr),
  concat_atom(['let ', Small, ' = ', RelName, 
    ' feed extend[', IDAttr, ': seqnext()] consume'], '',
    QueryAtom), 
  tryCreate(QueryAtom),
  card(RelLC, SmallCard),
  lowerfl(Small, LSmall),
  databaseName(DB),
  assert(storedCard(DB, LSmall, SmallCard)),
  downcase_atom(Small, DCSmall),
  assert(storedSpell(DB, DCSmall, LSmall)),
  !.

buildSmallRelation(RelLC, RelName, MinSize, Percent) :-
  sampleNameSmall(RelName, Small),
  atom_concat('xxxID', RelLC, IDAttr),
  concat_atom(['let ', Small, ' = ', RelName, 
    ' sample[', MinSize, ', ', Percent, '] extend[', IDAttr, 
    ': seqnext()] consume'], '',
    QueryAtom), 
  tryCreate(QueryAtom),
  card(RelLC, Card),
  SmallCard is truncate(min(Card, max(MinSize, Card * Percent))),
  lowerfl(Small, LSmall),
  databaseName(DB),
  assert(storedCard(DB, LSmall, SmallCard)),
  downcase_atom(Small, DCSmall),
  assert(storedSpell(DB, DCSmall, LSmall)).


/*
----	createSmall(+Rel, +Size) :-
----

Create small relation manually, for non-standard databases.

*/

createSmall(Rel, Size)  :-  % Rel in lc
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  buildSmallRelation(Rel3, Rel3, Size, 0.000001),
  !.

createSmall(Rel, Size)  :-  % Rel in uc
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  buildSmallRelation(Rel2, URel, Size, 0.000001),
  !.



/*
Creates a sample relation, for determining the selectivity of a relation 
object for a given predicate. The first two rules consider the case, that 
there is a sample relation already available and the last two ones create 
new relations by sending a Secondo ~let~-command.

*/

sampleSizeJoin(500).

sampleSizeSelection(2000).

thresholdMainMemorySizeSampleJ(2048).

thresholdMainMemorySizeSampleS(2048).

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
  createSample(Sample, Rel3, JoinSize, SampleCard),
  databaseName(DB),
  assert(storedCard(DB, Sample, SampleCard)),
  downcase_atom(Sample, DCSample),  
  assert(storedSpell(DB, DCSample, lc(Sample))).

createSampleJ(Rel) :- %Rel in uc
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  sampleNameJ(URel, Sample),
  sampleSizeJoin(JoinSize),
  createSample(Sample, URel, JoinSize, SampleCard),
  lowerfl(Sample, LSample),
  databaseName(DB),
  assert(storedCard(DB, LSample, SampleCard)),
  downcase_atom(Sample, DCSample),
  assert(storedSpell(DB, DCSample, LSample)).

createSampleS(Rel) :- %Rel in lc
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleNameS(Rel3, Sample),
  sampleSizeSelection(SelectionSize),
  createSample(Sample, Rel3, SelectionSize, SampleCard),
  databaseName(DB),
  assert(storedCard(DB, Sample, SampleCard)),
  downcase_atom(Sample, DCSample),  
  assert(storedSpell(DB, DCSample, lc(Sample))).

createSampleS(Rel) :- %Rel in uc
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  sampleNameS(URel, Sample),
  sampleSizeSelection(SelectionSize),
  createSample(Sample, URel, SelectionSize, SampleCard),
  lowerfl(Sample, LSample),
  databaseName(DB),
  assert(storedCard(DB, LSample, SampleCard)),
  downcase_atom(Sample, DCSample),
  assert(storedSpell(DB, DCSample, LSample)).



/*

----	createSample(+Sample, +Rel, +SampleSize, -SampleCard)
----

Create a random order sample ~Sample~ for relation ~Rel~ with desired sample size ~SampleSize~. The actual sample size is returned as ~SampleCard~.

*/

createSample(Sample, Rel, SampleSize, SampleCard) :-
  concat_atom(['let ', Sample, ' = ', Rel, 
    ' sample[', SampleSize, ', 0.00001] 
      extend[xxxNo: randint(20000)] sortby[xxxNo asc] remove[xxxNo]
      consume'], '', QueryAtom),    
  tryCreate(QueryAtom),
  card(Rel, Card),
  SampleCard is truncate(min(Card, max(SampleSize, Card*0.00001))).




/*

----	createSamples(Rel, SelectionSize, JoinSize) :-
----

Create samples for ~Rel~ manually, speciying the size of selection and join samples.

*/


createSamples(Rel, SelectionSize, JoinSize) :-
  sampleNameS(Rel, SampleS),
  createSample(SampleS, Rel, SelectionSize, _),
  sampleNameJ(Rel, SampleJ),
  createSample(SampleJ, Rel, JoinSize, _).



writeErrorSampleFileJ(_, _).

writeErrorSampleFileS(Rel, MemorySize) :-
  nl,
  spelling(Rel, Rel2),
  (Rel2 = lc(Rel) -> Rel3 = Rel; upper(Rel,Rel3)),
  write('ERROR: Couldn\'t create sample files for relation \''),
  write(Rel3), write('\'!'),nl,
  write('Sample file needs more than '), write(MemorySize),
  write(' KB in main memory.'),nl,
  write('Please create the file manually, e.g.: '),
  write('createSamples(\''), write(Rel3), write('\', 100, 50)'), nl,
  write('where 100 and 50 are the desired sample sizes for the selection'), nl,
  write('and join samples, respectively.'), nl, nl.


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
  not(optimizerOption(dynamicSample)),
  createSampleRelation(Rel),!.

createSampleRelationIfNotDynamic(_) :-
  optimizerOption(dynamicSample), !.

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

tryCreateSmallRelation(Rel, ObjList) :- 
  optimizerOption(entropy),
  createSmallRelation(Rel, ObjList),!.

tryCreateSmallRelation(_, _) :- 
  not(optimizerOption(entropy)).

relation(Rel, AttrList) :-
  databaseName(DB),
  storedRel(DB, Rel, AttrList), !.

relation(Rel, AttrList) :-
  optimizerOption(dynamicSample),
  getSecondoList(ObjList),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [AttrList2]]]]]]], ObjList),
  downcase_atom(ORel, DCRel),
  DCRel = Rel,
  extractList(AttrList2, AttrList3),
  downcase_list(AttrList3, AttrList),
  createSampleRelationIfNotDynamic(Rel),
  tryCreateSmallRelation(Rel, ObjList),!.

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
  tryCreateSmallRelation(Rel, ObjList),
  extractAttrTypes(Rel, AttrList2),  
  databaseName(DB),
  assert(storedRel(DB, Rel, AttrList)),
  createAttrSpelledAndIndexLookUp(Rel, AttrList3).
%  retractall(storedSecondoList(ObjList)).

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
  storedRel(DB, X, Y),
  write(Stream, storedRel(DB, X, Y)),
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
1.1.4 Printing the complete Database Schema

By now, only stored meta information is handled by the optimizer and printed
when using the show- or write- predicates.

In contrast, ~showDatabaseSchema/0~ will print the schemas of all, not only
the actually used relations.

*/

showRelationAttrs([]).
showRelationAttrs([[AttrD, Type] | Rest]) :- 
  % prints a list of [attributes,datatype]-pairs
  write(' '), write(AttrD), write(':'), write(Type), write(' '),
  showRelationAttrs(Rest), !.

showRelationSchemas([]).         
  % filters all relation opbjects from the database schema
showRelationSchemas([Obj | ObjList]) :-
  Obj = ['OBJECT',Rel,_ | [[[_ | [[_ | [AttrList2]]]]]]],
  write('  '), write(Rel), write('  ['),
  showRelationAttrs(AttrList2),
  write(']\n'),
  showRelationSchemas(ObjList),
  !.
showRelationSchemas([_ | ObjList]) :-
  showRelationSchemas(ObjList),
  !.

showDatabaseSchema :-
  databaseName(DB),
  getSecondoList(ObjList),
  write('\nAll relation-schemas of database \''), write(DB), write('\':\n'),
  showRelationSchemas(ObjList),
  nl,
  write('(Type \'showDatabase.\' to see meta data 
         collected by the optimizer.)\n'),
  !. 

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
  databaseName(DB),
  storedSpell(DB, Rel:Attr, Spelled),
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
  databaseName(DB),
  assert(storedSpell(DB, Rel:Attr, lc(OAttr))),
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
  databaseName(DB),
  assert(storedSpell(DB, Rel:Attr, Spelled)),
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
  databaseName(DB),
  storedSpell(DB, Rel, Spelled),
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
  databaseName(DB),
  assert(storedSpell(DB, Rel, lc(ORel))),
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
  databaseName(DB),
  assert(storedSpell(DB, Rel, Spelled)),
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
  storedSpell(DB, X, Y),
  write(Stream, storedSpell(DB, X, Y)),
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
  spelled(Rel, Rel2, _),
  databaseName(DB),
  storedCard(DB, Rel2, Size),
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
  databaseName(DB),
  assert(storedCard(DB, Rel2, Size)),
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
  databaseName(DB),
  assert(storedCard(DB, Rel2, Size)),
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
  storedCard(DB, X, Y),
  write(Stream, storedCard(DB, X, Y)),
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
verifyIndexAndStoreIndex(Rel, Attr, Index, LogicalIndexType) :- % Index exists
  dm(index,['\n-->verifyIndexAndStoreIndex(',Rel, ',',Attr, ',',Index, ',',LogicalIndexType, ')\n']),
  getSecondoList(ObjList),
  member(['OBJECT', Index, _ , [[PhysicalIndexType | _]]], ObjList),
  (   ( indexType(LogicalIndexType),
        LogicalIndexType = PhysicalIndexType
      )
    ; ( optimizerOption(rtreeIndexRules), 
        member([LogicalIndexType, PhysicalIndexType],
               [[object_time,rtree], [object_space,rtree], [object_d3,rtree3],
%               [group10_time,rtree],[group10_space,rtree],[group10_d3,rtree3],
                [unit_time,rtree],   [unit_space,rtree],   [unit_d3,rtree3]
               ])
      )
  ),
  databaseName(DB),
  assert(storedIndex(DB, Rel, Attr, LogicalIndexType, Index)),
  dm(index,['\n<--verifyIndexAndStoreIndex(',Rel, ',',Attr, ',',Index, ',',LogicalIndexType, ')\n']),
  !.

verifyIndexAndStoreNoIndex(Rel, Attr) :-      % No index
  dm(index,['\n-->verifyIndexAndStoreNoIndex(',Rel, ',',Attr, ',',Index, ',',IndexType, ')\n']),
  downcase_atom(Rel, DCRel),
  downcase_atom(Attr, DCAttr),
  relation(DCRel, List),
  member(DCAttr, List),
  databaseName(DB),
  assert(storedNoIndex(DB, Rel, Attr)),
  dm(index,['\n<--verifyIndexAndStoreNoIndex(',Rel, ',',Attr, ',',Index, ',',IndexType, ')\n']).

/*
1.4.2 Look up Index

The first rule simply reduces an attribute of the form e.g. p:ort just 
to its attribute name e.g. ort.

---- hasIndex(+Rel, +Attr, -IndexName, ?IndexType)
----
 
There is an index named ~IndexName~ of type ~IndexType~ on relation 
~Rel~ with key ~Attr~ in the opened database.

*/

hasIndex(rel(Rel, _, _), attr(_:A, _, _), IndexName, Type) :-
  hasIndex(rel(Rel, _, _), attr(A, _, _), IndexName, Type).
/*

Gets the index name ~Index~ for relation ~Rel~ and attribute ~Attr~
via dynamic predicate ~storedIndex/5~.

*/

hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index, Type) :-
  databaseName(DB),
  storedIndex(DB, Rel, Attr, Type, Index),
  !.

/*
If there is information stored in local memory, that there is no index
for relation ~Rel~ and attribute ~Attr~ then this rule fails.

*/

hasIndex(rel(Rel, _, _), attr(Attr, _, _), _, _) :-
  databaseName(DB),
  storedNoIndex(DB, Rel, Attr),
  !,
  fail.

/*
We have to differentiate the next rules, if the first letter of attribute 
name ~Attr~ is written in lower or in upper case and if there is an
index available for relation ~Rel~ and attribute ~Attr~.

*/

% cases: Attr in lc, Rel in lc or uc succeeds (index found)
hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index, IndexType) :-
  not(Attr = _:_), 
  spelled(Rel:Attr, attr(Attr2, 0, l)),              
  ( ( spelled(Rel, _, l), URel = Rel ) % Rel in lc
    *-> true
    ; ( % Rel in uc
        spelling(Rel, Spelled),
        Rel = Spelled,
        upper(Rel, URel)
      )
  ),
  atom_concat(Rel, '_', Index1),
  atom_concat(Index1, Attr2, Index),
  verifyIndexAndStoreIndex(Rel, Attr, Index, IndexType),
  getSmallIndexCreateQuery(none, none, IndexType, URel, Attr, Index, QueryAtom),
  tryCreate(QueryAtom),
  !.

%attr in lc fails (no index)
hasIndex(rel(Rel, _, _), attr(Attr, _, _), _, _) :-  
  not(Attr = _:_),                                   
  spelled(Rel:Attr, attr(_, 0, l)),
  verifyIndexAndStoreNoIndex(Rel, Attr),
  !,
  fail.


% cases: Attr in uc, Rel in lc or uc succeeds (index found)
hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index, IndexType) :-
  not(Attr = _:_), 
  spelled(Rel:Attr, attr(Attr2, 0, u)),             
  ( ( spelled(Rel, _, l), URel = Rel ) % Rel in lc
    *-> true
    ; ( % Rel in uc
        spelling(Rel, Spelled),
        Rel = Spelled,
        upper(Rel, URel)
      )
  ),
  upper(Attr2, SpelledAttr),
  atom_concat(Rel, '_', Index1),
  atom_concat(Index1, SpelledAttr, Index),
  verifyIndexAndStoreIndex(Rel, Attr, Index, IndexType),
  getSmallIndexCreateQuery(none, none, IndexType, URel, Attr, Index, QueryAtom),
  tryCreate(QueryAtom),
  !.


%attr in uc fails (no index)
hasIndex(rel(Rel, _, _), attr(Attr, _, _), _, _) :- 
  not(Attr = _:_),
  spelled(Rel:Attr, attr(_, 0, u)),
  verifyIndexAndStoreNoIndex(Rel, Attr),
  !,
  fail.

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
  storedIndex(DB, U, V, W, X),
  write(Stream, storedIndex(DB, U, V, W, X)),
  write(Stream, '.\n').

writeStoredNoIndex(Stream) :-
  storedNoIndex(DB, U, V),
  write(Stream, storedNoIndex(DB, U, V)),
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
  storedPET(DB, Term, _),
  arg(1, Term, Arg1),
  getRelAttrName(Rel, Arg1),
  retract(storedPET(DB, Term, _)),
  retractPETs(Rel).

retractPETs(Rel) :-
  databaseName(DB),
  storedPET(DB, Term, _),
  arg(2, Term, Arg2),
  getRelAttrName(Rel, Arg2),
  retract(storedPET(DB, Term, _)),
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
  databaseName(DB),
  retractall(storedCard(DB, LFSpelledRel, _)),
  retractall(storedCard(DB, LFSampleS, _)),
  retractall(storedCard(DB, LFSampleJ, _)),
  retractall(storedCard(DB, Small, _)),
  retractall(storedTupleSize(DB, LFSpelledRel, _)),
  retractall(storedSpell(DB, DCSpelledRel, _)),
  retractall(storedSpell(DB, DCSpelledRel:_, _)),
  retractall(storedSpell(DB, DCSampleS, _)),
  retractall(storedSpell(DB, DCSampleJ, _)),  
  retractall(storedSpell(DB, DCSmall, _)),
  retractSels(Rel),
  retractPETs(Rel),
  retractall(storedRel(DB, DCSpelledRel, _)),
  retractall(storedAttrSize(DB, DCSpelledRel, _, _, _, _, _)).
  %retractall(storedIndex(DB, LFSpelledRel, _, _, _)),
  %retractall(storedNoIndex(DB, LFSpelledRel, _)).
  
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
  databaseName(DB),
  storedTupleSize(DB, Rel2, Size),
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
1.6.2 Storing And Loading Tuple Sizes

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
  retractall(storedAttrSize(_, _, _, _, _, _, _)),
  [storedAttrSizes].  

writeStoredAttrSizes :-
  open('storedAttrSizes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredAttrSize(FD), _),
  close(FD).

writeStoredAttrSize(Stream) :-
  storedAttrSize(Database, Rel, Attr, Type, CoreSize, InFlobSize, ExtFlobSize),
  write(Stream, storedAttrSize(Database, Rel, Attr, Type, CoreSize, 
                               InFlobSize, ExtFlobSize)),
  write(Stream, '.\n').

showStoredAttrSize :-
  storedAttrSize(Database, Rel, Attr, Type, CoreSize, InFlobSize, ExtFlobSize),
  write(Database), write('.'), write(Rel), write('.'), 
  write(Attr), write(': \t'), write(Type), 
  write(' ('), write(CoreSize), write('/'), 
  write(InFlobSize), write('/'), write(ExtFlobSize), write(')\n').

showStoredAttrSizes :-
  write('Stored attribute sizes\nRel.Attr: Type '),
  write('(CoreTupleSize/Avg.InlineFlobSize/Avg.ExtFlobSize) [byte]:\n'),
  findall(_, showStoredAttrSize, _).

:-
  dynamic(storedAttrSize/7),
  at_halt(writeStoredAttrSizes),
  readStoredAttrSizes.




