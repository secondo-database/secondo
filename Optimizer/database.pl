/*

[File ~database.pl~]

1 Database Dependent Information

The Secondo optimizer module needs database dependent information to
compute the best query plan. In particular information about the
cardinality and the schema of a relation is needed by the optimizer.
Furthermore the spelling of relation and attribute names must be
known to send a Secondo query or command. Remember, that all object
and attribute names are given in lower case, because of the PROLOG
notation. Words starting with a capital letter are interpreted as a
variable in PROLOG. Finally the optimizer has to be informed, if an 
index exists for the pair (relation name, attribute name). All this
information retrievement is provided by this module.

1.1  Information About Cardinalities Of Relations

---- card(Rel, Size) :-
----

The cardinality of relation ~Rel~ is ~Size~.

1.1.1 Retrieving Cardinalities

If ~card~ is called, it tries to look up the cardinality via the 
dynamic predicate ~storedCard~ (automatically stored).
If this fails, a Secondo query is issued, which determines the
cardinality. The retrieved cardinality is then stored in predicate
~storedCard~.

*/

card(Rel, Size) :-
  storedCard(Rel, Size),
  !.

/*
Looking up for default writing of the relation name, e.g. Staedte. A
Secondo query is necessary for the next three rules.

*/

card(Rel, Size) :-
  not(spelling(Rel, _)),
  %spelled(Rel, Rel2, u),
  %Rel = Rel2,
  Query = (count(rel(Rel, _, u))),  
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  secondo(QueryAtom, [int, Size]),
  assert(storedCard(Rel, Size)),
  !.

/*
The first letter of the relation name is written in lower case, e.g. plz 
or cItIEs.

*/

card(Rel, Size) :-
  spelling(Rel, Spelled),
  Spelled = lc(Spelled2),
  Query = (count(rel(Spelled2, _, l))),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  secondo(QueryAtom, [int, Size]),
  assert(storedCard(Rel, Size)),
  !.

/*
Arbitrary writing of the relation name, the first letter is in upper 
case, e.g. PLZ or TEn.

*/

card(Rel, Size) :-
  spelling(Rel, Spelled), 
  Query = (count(rel(Spelled, _,u))),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  secondo(QueryAtom, [int, Size]),
  assert(storedCard(Rel, Size)),
  !.

card(_, _) :- fail.
  
/*
1.1.2 Storing and Loading Cardinalities

At halt, all retrieved information about the cardinality
is stored in a file ~storedCards.pl~. This file is loaded up with the
next call of the optimizer program. This mechanism ensures that a
cardinality has to be retrieved only once.

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
1.2 Information About The Spelling Of Relation And Attribute Names

---- spelling(Rel:Attr, AttrName) :-
     spelling(Rel, RelName) :-
----

The spelling of attribute ~Attr~ of relation ~Rel~ is ~AttrName~.

The spelling of relation ~Rel~ is ~RelName~.

1.2.1 Auxiliary Rules

The Rule ~lowerfl~ sets the first letter of atom ~Upper~ to upper case.
The result is unified with atom ~Lower~.

*/
lowerfl(Upper, Lower) :-
  atom_codes(Upper, [First | Rest]),
  to_lower(First, First2),
  LowerList = [First2 | Rest],
  atom_codes(Lower, LowerList).

/*
The test ~is\_lowerfl~ checks whether the first letter of ~Rel~ is in
lower case.

*/
is_lowerfl(Rel) :-
  atom_chars(Rel, [First | _]),
  downcase_atom(First, LCFirst),
  First = LCFirst.

/*
The ~spelling2~ rules insure that object ~Rel~ is a member of the list
~ObjList~ and then the dynamic predicate ~storedSpell~ is set up to the
respective value. The three possible spellings of relation names have
to be differentiated.

*/
spelling2(Rel, ObjList, _) :-
  upper(Rel, URel),
  member(URel, ObjList),
  assert(storedSpell(Rel, default)),
  !,
  fail.

spelling2(Rel, ObjList, RelName) :-
  member(RelList, ObjList),
  downcase_atom(RelList, Rel),
  is_lowerfl(RelList),
  RelName = lc(RelList),
  assert(storedSpell(Rel, lc(RelList))),
  !.

spelling2(Rel, ObjList, RelName) :-
  member(RelList, ObjList),
  downcase_atom(RelList,Rel),
  lowerfl(RelList, RelName),
  assert(storedSpell(Rel, RelName)),
  !.

/*
~downcase\_list~ converts the letters of all atoms of the first list
to lower case.

*/ 
downcase_list([], []).
downcase_list([First1 | Rest1], [First2 | Rest2]) :-
  downcase_atom(First1, First2),
  downcase_list(Rest1, Rest2).

/*
~nextto~ succeeds when ~Y~ follows ~X~ in a list.

*/
nextto(X, Y, [First1, First2 | Rest], RestList) :-
  X = First1,
  Y = First2,
  RestList = Rest.
nextto(X, Y, [_ | Rest],RestList) :-
  nextto(X, Y, Rest,RestList).

/*
~getElem~ issues the Secondo command ~list objects~ and unifies 
~Elem~ with the real spelling of ~Attr~. Furthermore ~Attr~ must
be an attribute of ~Rel~.

*/
getElem(Rel, Attr, Elem) :-
  secondo('list objects', ObjList),
  flatten(ObjList, ObjList2),
  downcase_list(ObjList2,ObjList3),
  nth1(Index1, ObjList3, object),
  nth1(Index2, ObjList3, Rel),
  Index2 is Index1 + 1,
  nextto(object, Rel, ObjList3, RestList),
  nth1(Index3, RestList, Attr),
  Index4 is Index2 + Index3,
  nth1(Index4, ObjList2, Elem),
  assert(elem_is(Rel, Attr, Elem)),
  !.

/* 
1.2.2 Spelling Of Attribute Names

The first two ~spelling~ rules try to get the spelling of the
attribute name via the dynamic predicate ~storedSpell~.

*/
spelling(Rel:Attr, _) :-
  storedSpell(Rel:Attr, default),
  !,
  fail.

spelling(Rel:Attr, AttrName) :-
  storedSpell(Rel:Attr, AttrName),
  !.

/*
The next three ~spelling~ rules retrieve the real spelling of
~Attr~ by sending a Secondo command. Again the three possible
different spellings of attribute names have to be differentiated.

*/
spelling(Rel:Attr, _) :-
  getElem(Rel, Attr, Elem),
  upper(Attr, UAttr),
  UAttr = Elem,
  assert(storedSpell(Rel:Attr, default)),
  !,
  fail.

spelling(Rel:Attr, AttrName) :-
  elem_is(Rel, Attr, Elem),
  is_lowerfl(Elem),
  AttrName = lc(Elem),
  assert(storedSpell(Rel:Attr, lc(Elem))),
  !.

spelling(Rel:Attr, AttrName) :-
  elem_is(Rel, Attr, Elem),
  lowerfl(Elem, AttrName),
  assert(storedSpell(Rel:Attr, AttrName)),
  !.

spelling(_:_, _) :-
  !,
  fail. 

/* 
1.2.3 Spelling Of Relation Names

The first two ~spelling~ rules try to retrieve the spelling
of relation name ~Rel~ by the predicate ~storedSpell~. The
last rule issues a Secondo command for this purpose.

*/
spelling(Rel, _) :-
  storedSpell(Rel, default),
  !,
  fail.

spelling(Rel, RelName) :-
  storedSpell(Rel, RelName),
  !.

spelling(Rel, RelName) :-
  secondo('list objects', ObjList),
  flatten(ObjList, ObjList2),
  spelling2(Rel, ObjList2, RelName),
  !.

/*
1.2.4 Loading and Storing Relation And Attribute Names

This is quite similiar to the mechanism used in the section about
the cardinalities of relations.

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
  dynamic(elem_is/3),
  at_halt(writeStoredSpells),
  readStoredSpells.

/*
1.3 Relation Schemas

---- relation(Rel, AttrList) :-
----

The schema for relation ~Rel~ is ~AttrList~.

1.3.1 Auxiliary Rule

Rule and fact ~extractlist~ finds a list for one object
within a list of object lists. The result is unified with
the second list.

*/
extractList([[First, _]], [First]).
extractList([[First, _] | Rest], [First | Rest2]) :-
  extractList(Rest, Rest2).

/*
1.3.2 Looking Up For Relation Schemas

The schema of the relation ~Rel~, namely a list of the attribute names,
is binded to the variable ~AttrList~ (PROLOG list).

*/
relation(Rel, AttrList) :-
  storedRel(Rel, AttrList),
  !.

relation(Rel, AttrList) :-
  secondo('list objects',[_, [_, [_ | ObjList]]]),
  member(['OBJECT',OrigRel,_ | [[[_ | [[_ | [RestList]]]]]]], ObjList),
  downcase_atom(OrigRel, DCRel),
  DCRel = Rel,
  extractList(RestList, AttrList2),
  downcase_list(AttrList2, AttrList),
  assert(storedRel(Rel, AttrList)).

/*
1.3.3 Storing And Loading Relation Schemas

This is again the same procedure as it is described in the respective 
sections above.

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
1.4 Indexes

---- hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index) :-
----

If succeeds, ~Index~ is the name of an index of relation ~Rel~ and
attribute ~Attr~, otherwise fails.

1.4.1 Auxiliary Rules

Looks up if an index exists for ~Rel~ and ~Attr~. If an index exists the
dynamic predicate ~storedIndex~ is set up properly with the index type
~IndexType~ e.g. btree and the index name ~Index~. If there isn't any
existing index, the dynamic predicate ~storedNoIndex~ is initialized 
respectively (second rule).

*/
retrieveIndex(Rel, Attr, Index) :-
  secondo('list objects',[_, [_, [_ | ObjList]]]),
  member(['OBJECT', Index, _ , [[IndexType | _]]], ObjList),
  assert(storedIndex(Rel, Attr, IndexType, Index)),
  !.

retrieveIndex(Rel, Attr, _) :-
  relation(Rel, List),
  member(Attr, List),
  assert(storedNoIndex(Rel, Attr)),
  !,
  fail.

/*
1.4.2 Looking Up For Existing Indexes

The first rule simply reduces a renamed attribute of the form e.g.
p:ort just to its attribute name e.g. ort.

*/

hasIndex(rel(Rel, _, _), attr(_:A, _, _), IndexName) :-
  hasIndex(rel(Rel, _, _), attr(A, _, _), IndexName).

/*
There is an index available via dynamic predicate ~storedIndex~.

*/

hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index) :-
  storedIndex(Rel, Attr, _, Index),
  !.

/*
No Index for relation ~Rel~ and attribute ~Attr~. The rule fails.

*/
hasIndex(rel(Rel, _, _), attr(Attr, _, _), _) :-
  storedNoIndex(Rel, Attr),
  !,
  fail.

/*
We have to differentiate the next rules by the four possible combinations
of spelling relation name ~Rel~ and attribute name ~Attr~, so that
~retrieveIndex~ is able to find the index object within the list of
objects.

*/
hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index) :- %both written in lc
  not(Attr = _:_),
  spelled(Rel, Rel2, l),
  spelled(Rel:Attr, attr(Attr2, 0, l)),
  atom_concat(Rel2, '_', Index1),
  atom_concat(Index1, Attr2, Index),
  retrieveIndex(Rel, Attr, Index),
  !.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index) :- %rel in lc attr in uc
  not(Attr = _:_),
  spelled(Rel, Rel2, l),
  spelled(Rel:Attr, attr(Attr2, 0, u)),
  not(Attr2 = lc(_)),
  upper(Attr2, SpelledAttr),
  atom_concat(Rel2, '_', Index1),
  atom_concat(Index1, SpelledAttr, Index),
  retrieveIndex(Rel, Attr, Index),
  !.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index) :- %rel in uc attr in lc
  not(Attr = _:_),
  spelled(Rel, Rel2, u),
  spelled(Rel:Attr, attr(Attr2, 0, l)),
  not(Rel2 = lc(_)),
  %upper(Rel2, SpelledRel),
  atom_concat(Rel2, '_', Index1),
  atom_concat(Index1, Attr2, Index),
  retrieveIndex(Rel, Attr, Index),
  !.

hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index) :- % both written in uc
  not(Attr = _:_),
  spelled(Rel, Rel2, u),
  spelled(Rel:Attr, attr(Attr2, 0, u)),
  not(Rel2 = lc(_)),
  not(Attr2 = lc(_)),
  %upper(Rel2, SpelledRel),
  upper(Attr2, SpelledAttr),
  atom_concat(Rel2, '_', Index1),
  atom_concat(Index1, SpelledAttr, Index),
  retrieveIndex(Rel, Attr, Index),
  !.

/*
1.4.3 Storing And Loading About Existing Indexes

Here we provide storing and reading of  the two dynamic predicates 
~storedIndex~ and ~storedNoIndex~ in the file ~storedIndexes~.

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
  dynamic(storedIndex/2),
  dynamic(storedNoIndex/2),
  at_halt(writeStoredIndexes),
  readStoredIndexes.













