/*
1 Database Dependent Information

[File ~database.pl~]


1.1 Relation Schemas

*/
extractList([[First, _]], [First]).
extractList([[First, _] | Rest], [First | Rest2]) :-
  extractList(Rest, Rest2).

downcase_list([], []).
downcase_list([First1 | Rest1], [First2 | Rest2]) :-
  downcase_atom(First1, First2),
  downcase_list(Rest1, Rest2).

/*
relation(staedte, [sname, bev, plz, vorwahl, kennzeichen]).
relation(plz, [plz, ort]).
relation(ten, [no]).
relation(thousand, [no]).
relation(orte, [kennzeichen, ort, vorwahl, bevt]).
relation(test1, [attr1, attr2, attr3]).
relation(test2, [attr1, attr2, attr3]).
relation(test3, [attr1, attr2, attr3]).
relation(test4, [attr1, attr2, attr3]).
*/

createSampleRelation(Rel, ObjList) :-
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleName(Rel3, Sample),
  member(['OBJECT', Sample, _ , [[_ | _]]], ObjList),
  !.

createSampleRelation(Rel, ObjList) :-
  spelling(Rel, Rel2),
  not(Rel2 = lc(_)),
  upper(Rel2, URel),
  sampleName(URel, Sample),
  member(['OBJECT', Sample, _ , [[_ | _]]], ObjList),
  !.

createSampleRelation(Rel, _) :-
  spelling(Rel, Rel2),
  Rel2 = lc(Rel3),
  sampleName(Rel3, Sample),
  concat_atom(['let ', Sample, ' = ', Rel3, 
    ' sample[100, 0.01] consume'], '', QueryAtom),
  secondo(QueryAtom),
  card(Rel3, Card),
  SampleCard is truncate(min(Card, max(100, Card*0.01))),
  assert(storedCard(Sample, SampleCard)),
  downcase_atom(Sample, DCSample),
  assert(storedSpell(DCSample, lc(Sample))),
  !.

createSampleRelation(Rel, _) :-
  spelling(Rel, Rel2),
  upper(Rel2, URel),
  sampleName(URel, Sample),
  concat_atom(['let ', Sample, ' = ', URel, 
    ' sample[100, 0.01] consume'], '', QueryAtom),
  secondo(QueryAtom),
  card(Rel2, Card),
  SampleCard is truncate(min(Card, max(100, Card*0.01))),
  lowerfl(Sample, LSample),
  assert(storedCard(LSample, SampleCard)),
  downcase_atom(Sample, DCSample),
  assert(storedSpell(DCSample, LSample)),
  !.

lookupIndex(Rel, Attr) :-
  not(hasIndex(rel(Rel, _, _), attr(Attr, _, _), _)).

lookupIndex(Rel, Attr) :-
  hasIndex(rel(Rel, _, _), attr(Attr, _, _), _).

createAttrSpelledAndIndexLookUp(_, []).
createAttrSpelledAndIndexLookUp(Rel, [ First | Rest ]) :-
  %downcase_atom(Rel, DCRel),
  downcase_atom(First, DCFirst),
  spelling(Rel:DCFirst, _),
  %lowerfl(Rel, LRel),
  spelled(Rel, SRel, _),
  lowerfl(First, LFirst),
  lookupIndex(SRel, LFirst),
  createAttrSpelledAndIndexLookUp(Rel, Rest).

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
  createSampleRelation(Rel, ObjList),
  retract(storedSecondoList(ObjList)).

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
1.2 Spelling of Relation and Attribute Names

*/

is_lowerfl(Rel) :-
  atom_chars(Rel, [First | _]),
  downcase_atom(First, LCFirst),
  First = LCFirst.
  
lowerfl(Upper, Lower) :-
  atom_codes(Upper, [First | Rest]),
  to_lower(First, First2),
  LowerList = [First2 | Rest],
  atom_codes(Lower, LowerList).
  
getSecondoList(ObjList) :-
  storedSecondoList(ObjList),
  !.

getSecondoList(ObjList) :-
  secondo('list objects',[_, [_, [_ | ObjList]]]), 
  assert(storedSecondoList(ObjList)),
  !.
 
/* 
spelling(staedte:plz, pLZ).
spelling(staedte:sname, sName).
spelling(plz, lc(plz)).
spelling(plz:plz, pLZ).
spelling(ten, lc(ten)).
spelling(ten:no, lc(no)).
spelling(thousand, lc(thousand)).
spelling(thousand:no, lc(no)).
spelling(orte:bevt, bevT).
spelling(test1, test1).
spelling(test2, lc(test2)).
spelling(test3, lc(tEST3)).
spelling(test4, tEst4).
spelling(test1:attr1, attr1).
spelling(test1:attr2, lc(attr2)).
spelling(test1:attr3, lc(aTTR3)).
spelling(test1:attr4, aTtr4).
spelling(test2:attr1, attr1).
spelling(test2:attr2, lc(attr2)).
spelling(test2:attr3, lc(aTTR3)).
spelling(test2:attr4, aTtr4).
spelling(test3:attr1, attr1).
spelling(test3:attr2, lc(attr2)).
spelling(test3:attr3, lc(aTTR3)).
spelling(test3:attr4, aTtr4).
spelling(test4:attr1, attr1).
spelling(test4:attr2, lc(attr2)).
spelling(test4:attr3, lc(aTTR3)).
spelling(test4:attr4, aTtr4).
*/

spelling(Rel:Attr, Spelled) :-
  storedSpell(Rel:Attr, Spelled),
  !.

spelling(Rel:Attr, Spelled) :-
  getSecondoList(ObjList),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [AttrList]]]]]]], ObjList),
  downcase_atom(ORel, Rel),
  member([OAttr, _], AttrList),
  downcase_atom(OAttr, Attr),
  is_lowerfl(OAttr),
  Spelled = lc(OAttr),
  assert(storedSpell(Rel:Attr, lc(OAttr))),
  %retract(storedSecondoList(ObjList)),
  !.

spelling(Rel:Attr, Spelled) :-
  getSecondoList(ObjList),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [AttrList]]]]]]], ObjList),
  downcase_atom(ORel, Rel),
  member([OAttr, _], AttrList),
  downcase_atom(OAttr, Attr),
  lowerfl(OAttr, Spelled),
  assert(storedSpell(Rel:Attr, Spelled)),
  %retract(storedSecondoList(ObjList)),
  !.

spelling(_:_, _) :- !, fail.  

spelling(Rel, Spelled) :-
  storedSpell(Rel, Spelled),
  !.

spelling(Rel, Spelled) :-
  getSecondoList(ObjList),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [_]]]]]]], ObjList),
  downcase_atom(ORel, Rel),
  is_lowerfl(ORel),
  Spelled = lc(ORel),
  assert(storedSpell(Rel, lc(ORel))),
  %retract(storedSecondoList(ObjList)),
  !.
  
spelling(Rel, Spelled) :-
  getSecondoList(ObjList),
  member(['OBJECT',ORel,_ | [[[_ | [[_ | [_]]]]]]], ObjList),
  downcase_atom(ORel, Rel),
  lowerfl(ORel, Spelled),
  assert(storedSpell(Rel, Spelled)),
  %retract(storedSecondoList(ObjList)),
  !.
  
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

*/

/*
card(staedte, 58).
card(staedte_sample, 58).
card(plz, 41267).
card(plz_sample, 428).
card(ten, 10).
card(ten_sample, 10).
card(thousand, 1000).
card(thousand_sample, 89).
card(orte, 506).
card(orte_sample, 100).
card(test1, 2).
card(test2, 2).
card(tEST3, 2).
card(tEst4, 2).
card(test1_sample, 2).
card(test2_sample, 2).
card(tEST3_sample, 2).
card(tEST4_sample, 2).
*/

card(Rel, Size) :-
  storedCard(Rel, Size),
  !.
  
card(Rel, Size) :-
  %downcase_atom(Rel, DCRel),
  spelled(Rel, Rel2, l),
  Query = (count(rel(Rel2, _, l))),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  secondo(QueryAtom, [int, Size]),
  assert(storedCard(Rel2, Size)),
  !.

card(Rel, Size) :-
  spelled(Rel, Rel2, u),
  Query = (count(rel(Rel2, _, u))),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  secondo(QueryAtom, [int, Size]),
  assert(storedCard(Rel2, Size)),
  !.

card(_, _) :- fail.

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
1.4.2 Looking Up For Existing Indexes

The first rule simply reduces a renamed attribute of the form e.g.
p:ort just to its attribute name e.g. ort.

*/

verifyIndexAndStoreIndex(Rel, Attr, Index) :-
  getSecondoList(ObjList),
  member(['OBJECT', Index, _ , [[IndexType | _]]], ObjList),
  assert(storedIndex(Rel, Attr, IndexType, Index)),
  !.

verifyIndexAndStoreNoIndex(Rel, Attr) :-
  downcase_atom(Rel, DCRel),
  downcase_atom(Attr, DCAttr),
  relation(DCRel, List),
  member(DCAttr, List),
  assert(storedNoIndex(Rel, Attr)).

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
  dynamic(storedIndex/4),
  dynamic(storedNoIndex/2),
  at_halt(writeStoredIndexes),
  readStoredIndexes.
/*
1.4 Update Indexes And Relations

1.4.1 Update Indexes

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
1.4.1 Update Relations

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
  retractall(storedCard(LSample, _)),
  retractall(storedSpell(Rel, _)),
  retractall(storedSpell(Rel:_, _)),
  retractall(storedSpell(DCSample, _)),  
  retractSels(Rel2),
  retractall(storedRel(Rel, _)),
  retractall(storedIndex(Rel2, _, _, _)),
  retractall(storedNoIndex(Rel2, _)).














