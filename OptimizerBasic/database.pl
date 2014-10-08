/*
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
/*
Creates a sample relation, for determining the selectivity of a relation 
object for a given predicate. The first two rules consider the case, that 
there is a sample relation already available and the last two ones create 
new relations by sending a Secondo ~let~-command.

*/
createSampleRelation(Rel, ObjList) :-  % Rel in lc
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleName(Rel3, Sample),
  member(['OBJECT', Sample, _ , [[_ | _]]], ObjList),
  !.

createSampleRelation(Rel, ObjList) :-  % Rel in uc
  spelling(Rel, Rel2),
  not(Rel2 = lc(_)),
  upper(Rel2, URel),
  sampleName(URel, Sample),
  member(['OBJECT', Sample, _ , [[_ | _]]], ObjList),
  !.

createSampleRelation(Rel, _)  :-  % Rel in lc
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleName(Rel3, Sample),
  concat_atom(['let ', Sample, ' = ', Rel3, 
    ' sample[1000, 0.0001] consume'], '', QueryAtom),
  secondo(QueryAtom),
  card(Rel3, Card),
  SampleCard is truncate(min(Card, max(1000, Card*0.0001))),
  assert(storedCard(Sample, SampleCard)),
  downcase_atom(Sample, DCSample),
  assert(storedSpell(DCSample, lc(Sample))),
  !.

createSampleRelation(Rel, _) :-  % Rel in uc
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  sampleName(URel, Sample),
  concat_atom(['let ', Sample, ' = ', URel, 
    ' sample[1000, 0.0001] consume'], '', QueryAtom),
  secondo(QueryAtom),
  card(Rel2, Card),
  SampleCard is truncate(min(Card, max(1000, Card*0.0001))),
  lowerfl(Sample, LSample),
  assert(storedCard(LSample, SampleCard)),
  downcase_atom(Sample, DCSample),
  assert(storedSpell(DCSample, LSample)),
  !.
/*
Checks, if an index exists for ~Rel~ and ~Attr~ and stores the 
respective values to the dynamic predicates ~storedIndex/4~ or 
~storedNoIndex/2~.

*/
lookupIndex(Rel, Attr) :-
  not(hasIndex(rel(Rel, _, _), attr(Attr, _, _), _)).

lookupIndex(Rel, Attr) :-
  hasIndex(rel(Rel, _, _), attr(Attr, _, _), _).

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
relation(Rel, AttrList) :-
  storedRel(Rel, AttrList),
  !.

relation(Rel, AttrList) :-
  getSecondoList(ObjList),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [AttrList2]]]]]]], ObjList),
  downcase_atom(ORel, DCRel),
  DCRel = Rel,
  extractList(AttrList2, AttrList3),
  downcase_list(AttrList3, AttrList),
  assert(storedRel(Rel, AttrList)),
  spelling(Rel, _),
  createAttrSpelledAndIndexLookUp(Rel, AttrList3),
  card(Rel, _),
  tuplesize(Rel, _),
  createSampleRelation(Rel, ObjList),
  retract(storedSecondoList(ObjList)).

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
  atom_codes(Upper, [First | Rest]),
  to_lower(First, First2),
  LowerList = [First2 | Rest],
  atom_codes(Lower, LowerList).
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
  storedCard(Rel, Size),
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

---- hasIndex(rel(Rel, _, _),attr(Attr, _, _), IndexName) :-
----

If it exists, the index name for relation ~Rel~ and attribute ~Attr~
is ~IndexName~.

1.4.1 Auxiliary Rule

Checks whether an index exists for ~Rel~ and ~Attr~ in the currently
opened database. Depending on this result the dynamic predicate
~storedIndex/4~ or ~storedNoIndex/2~ is set. 

*/
verifyIndexAndStoreIndex(Rel, Attr, Index) :- % Index exists
  getSecondoList(ObjList),
  member(['OBJECT', Index, _ , [[IndexType | _]]], ObjList),
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
hasIndex(rel(Rel, _, _), attr(_:A, _, _), IndexName) :-
  hasIndex(rel(Rel, _, _), attr(A, _, _), IndexName).
/*
Gets the index name ~Index~ for relation ~Rel~ and attribute ~Attr~
via dynamic predicate ~storedIndex/4~.

*/
hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index) :-
  storedIndex(Rel, Attr, _, Index),
  !.
/*
If there is information stored in local memory, that there is no index
for relation ~Rel~ and attribute ~Attr~ then this rule fails.

*/
hasIndex(rel(Rel, _, _), attr(Attr, _, _), _) :-
  storedNoIndex(Rel, Attr),
  !,
  fail.
/*
We have to differentiate the next rules, if the first letter of attribute 
name ~Attr~ is written in lower or in upper case and if there is an
index available for relation ~Rel~ and attribute ~Attr~.

*/
hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index) :- %attr in lc
  not(Attr = _:_),                                   %succeeds
  spelled(Rel:Attr, attr(Attr2, 0, l)),
  atom_concat(Rel, '_', Index1),
  atom_concat(Index1, Attr2, Index),
  verifyIndexAndStoreIndex(Rel, Attr, Index),
  !.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), _) :-     %attr in lc
  not(Attr = _:_),                                   %fails
  spelled(Rel:Attr, attr(_, 0, l)),
  verifyIndexAndStoreNoIndex(Rel, Attr),
  !, fail.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index) :- %attr in uc
  not(Attr = _:_),                                   %succeeds
  spelled(Rel:Attr, attr(Attr2, 0, u)),
  upper(Attr2, SpelledAttr),
  atom_concat(Rel, '_', Index1),
  atom_concat(Index1, SpelledAttr, Index),
  verifyIndexAndStoreIndex(Rel, Attr, Index),
  !.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), _) :-     %attr in uc
  not(Attr = _:_),                                   %fails
  spelled(Rel:Attr, attr(_, 0, u)),
  verifyIndexAndStoreNoIndex(Rel, Attr),
  !, fail.

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
updateIndex2(Rel, Attr) :-
  spelled(Rel, SRel, _),
  spelled(Rel:Attr, attr(Attr2, _, _)),
  storedNoIndex(SRel, Attr2),
  retract(storedNoIndex(SRel, Attr2)),
  hasIndex(rel(SRel, _, _),attr(Attr2, _, _), _).

updateIndex2(Rel, Attr) :-
  spelled(Rel, SRel, _),
  spelled(Rel:Attr, attr(Attr2, _, _)),
  storedIndex(SRel, Attr2, _, Index),
  retract(storedIndex(SRel, Attr2, _, Index)),
  not(hasIndex(rel(SRel, _, _),attr(Attr2, _, _), Index)).

updateIndex(Rel, Attr) :-
  getSecondoList(ObjList),
  updateIndex2(Rel, Attr),
  retract(storedSecondoList(ObjList)), 
  !.

updateIndex(Rel, Attr) :-
  getSecondoList(ObjList),
  not(updateIndex2(Rel, Attr)),
  retract(storedSecondoList(ObjList)), !, fail.
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

updateRel(Rel) :-
  spelled(Rel, Rel2, l),
  sampleName(Rel2, Sample),
  concat_atom(['delete ', Sample], '', QueryAtom),
  secondo(QueryAtom),
  lowerfl(Sample, LSample),
  downcase_atom(Sample, DCSample),
  retractall(storedCard(Rel2, _)),
  retractall(storedTupleSize(Rel2, _)),
  retractall(storedCard(LSample, _)),
  retractall(storedSpell(Rel, _)),
  retractall(storedSpell(Rel:_, _)),
  retractall(storedSpell(DCSample, _)), 
  retractSels(Rel2),
  retractall(storedRel(Rel, _)),
  retractall(storedIndex(Rel2, _, _, _)),
  retractall(storedNoIndex(Rel2, _)),!.

updateRel(Rel) :-
  spelled(Rel, Rel2, u),
  upper(Rel2, URel),
  sampleName(URel, Sample),
  concat_atom(['delete ', Sample], '', QueryAtom),
  secondo(QueryAtom),
  lowerfl(Sample, LSample),
  downcase_atom(Sample, DCSample),
  retractall(storedCard(Rel2, _)),
  retractall(storedTupleSize(Rel2, _)),
  retractall(storedCard(LSample, _)),
  retractall(storedSpell(Rel, _)),
  retractall(storedSpell(Rel:_, _)),
  retractall(storedSpell(DCSample, _)),  
  retractSels(Rel2),
  retractall(storedRel(Rel, _)),
  retractall(storedIndex(Rel2, _, _, _)),
  retractall(storedNoIndex(Rel2, _)).
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
  storedTupleSize(Rel, Size),
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
  write(QueryAtom1),
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













