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

1 Information about Selectivity of Predicates

[File ~statistics.pl~]



1.1 Rules about Commutativity of Predicates

~isCommutativeOP/1~ is defined in file ``operators.pl''.

*/

commute(X = Y, Y = X).
commute(X < Y, Y > X).
commute(X <= Y, Y >= X).
commute(X > Y, Y < X).
commute(X >= Y, Y <= X).
commute(X # Y, Y # X).
commute(Pred1, Pred2) :-
  Pred1 =.. [OP, Arg1, Arg2],
  isCommutativeOP(OP),
  Pred2 =.. [OP, Arg2, Arg1], !.



/*
1.2 Hard-Coded Selectivities of Predicates

(Deprecated sub-section removed by Christian D[ue]ntgen, May-15-2006)

*/


/*

1.3 Determine the Simple Form of Predicates

Simple forms of predicates are stored in predicate ~sel~ or
in predicate ~storedSel~.


----	simple(+Term, +Rel1, +Rel2, -Simple) :-
----

The simple form of a term ~Term~ containing attributes of ~Rel1~ and/or ~Rel2~
is ~Simple~.

*/

simple(attr(Var:Attr, 0, _), rel(Rel, Var, _), _, Rel:Attr) :- !.
simple(attr(Attr, 0, _), rel(Rel, *, _), _, Rel:Attr) :- !.

simple(attr(Var:Attr, 1, _), rel(Rel, Var, _), _, Rel:Attr) :- !.
simple(attr(Attr, 1, _), rel(Rel, *, _), _, Rel:Attr) :- !.

simple(attr(Var:Attr, 2, _), _, rel(Rel, Var, _),  Rel:Attr) :- !.
simple(attr(Attr, 2, _), _, rel(Rel, *, _), Rel:Attr) :- !.

simple(Term, Rel1, Rel2, Simple) :-
  compound(Term),
  functor(Term, T, 1), !,
  arg(1, Term, Arg1),
  simple(Arg1, Rel1, Rel2, Arg1Simple),
  functor(Simple, T, 1),
  arg(1, Simple, Arg1Simple).

simple(Term, Rel1, Rel2, Simple) :-
  compound(Term),
  functor(Term, T, 2), !,
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  simple(Arg1, Rel1, Rel2, Arg1Simple),
  simple(Arg2, Rel1, Rel2, Arg2Simple),
  functor(Simple, T, 2),
  arg(1, Simple, Arg1Simple),
  arg(2, Simple, Arg2Simple).

simple(Term, _, _, Term).

/*
----	simplePred(Pred, Simple) :-
----

The simple form of predicate ~Pred~ is ~Simple~.

*/

simplePred(pr(P, A, B), Simple) :- simple(P, A, B, Simple), !.
simplePred(pr(P, A), Simple) :- simple(P, A, A, Simple), !.
simplePred(X, _) :- throw(sql_ERROR(statistics_simplePred(X, undefined))).

/*

1.4 Retrieving, Storing, and Loading Selectivities

Auxiliary predicates for ~selectivity~.

*/

sampleS(rel(Rel, Var, Case), rel(Rel2, Var, Case)) :-
  atom_concat(Rel, '_sample_s', Rel2).

sampleJ(rel(Rel, Var, Case), rel(Rel2, Var, Case)) :-
  atom_concat(Rel, '_sample_j', Rel2).

sampleNameS(Name, Sample) :-
  atom_concat(Name, '_sample_s', Sample).

sampleNameJ(Name, Sample) :-
  atom_concat(Name, '_sample_j', Sample).

sampleNameSmall(Name, Small) :-
  atom_concat(Name, '_small', Small).

possiblyRename(Rel, Renamed) :-
  Rel = rel(_, *, _),
  !,
  Renamed = feed(Rel).

possiblyRename(Rel, Renamed) :-
  Rel = rel(_, Name, _),
  Renamed = rename(feed(Rel), Name).

dynamicPossiblyRenameJ(Rel, Renamed) :-
  Rel = rel(_, *, _),
  !,
  sampleSizeJoin(JoinSize),
  Renamed = sample(Rel, JoinSize, 0.00001).

dynamicPossiblyRenameJ(Rel, Renamed) :-
  Rel = rel(_, Name, _),
  sampleSizeJoin(JoinSize),
  Renamed = rename(sample(Rel, JoinSize, 0.00001), Name).

dynamicPossiblyRenameS(Rel, Renamed) :-
  Rel = rel(_, *, _),
  !,
  sampleSizeSelection(SelectionSize),
  Renamed = sample(Rel, SelectionSize, 0.00001).

dynamicPossiblyRenameS(Rel, Renamed) :-
  Rel = rel(_, Name, _),
  sampleSizeSelection(SelectionSize),
  Renamed = rename(sample(Rel, SelectionSize, 0.00001), Name).

/*
----  selectivityQuerySelection(+Pred, +Rel, 
                                -QueryTime, -BBoxResCard, -FilterResCard)

           selectivityQueryJoin(+Pred, +Rel1, +Rel2, 
                                -QueryTime, -BBoxResCard, -FilterResCard)
----

The cardinality query for a selection predicate is performed on the selection sample. The cardinality query for a join predicate is performed on the first ~n~ tuples of the selection sample vs. the join sample, where ~n~ is the size of the join sample. It is done in this way in order to have two independent samples for the join, avoiding correlations, especially for equality conditions.

If ~optimizerOption(dynamicSample)~ is defined, dynamic samples are used instead of the \_sample\_j / \_sample\_s resp. \_small relations.

The predicates return the time ~QueryTime~ used for the query, and the cardinality ~FilterResCard~ of the result after applying the predicate.

If ~Pred~ has a predicate operator that performs checking of overlapping minimal bounding boxes, the selectivity query will additionally return the cardinality after the bbox-checking in ~BBoxResCard~, otherwise its return value is set to constant  ~noBBox~. 

*/

selectivityQuerySelection(Pred, Rel, QueryTime, BBoxResCard, FilterResCard) :-
  Pred =.. [OP, Arg1, Arg2],
  isBBoxPredicate(OP),     % spatial predicate with bbox-checking
  BBoxPred =.. [intersects, bbox(Arg1), bbox(Arg2)],
  ( optimizerOption(dynamicSample)
    -> dynamicPossiblyRenameS(Rel, RelQuery)
    ;  ( sampleS(Rel, RelS),
         possiblyRename(RelS, RelQuery)
       )
  ),
  Query = count(filter(count(filter(RelQuery, BBoxPred),1), Pred)),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  dm(selectivity,['\nSelectivity query : ', QueryAtom, '\n']),
  getTime(secondo(QueryAtom, [int, FilterResCard]),QueryTime),
  dm(selectivity,['Elapsed Time: ', QueryTime, ' ms\n']), 
  secondo('list counters',  [[1, BBoxResCard]|_]), !.

selectivityQuerySelection(Pred, Rel, QueryTime, noBBox, ResCard) :-
  Pred =.. [OP|_],
  not(isBBoxPredicate(OP)), % normal predicate
  ( optimizerOption(dynamicSample)
    -> dynamicPossiblyRenameS(Rel, RelQuery)
    ;  ( sampleS(Rel, RelS),
         possiblyRename(RelS, RelQuery)
       )
  ),
  Query = count(filter(RelQuery, Pred)),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  dm(selectivity,['\nSelectivity query : ', QueryAtom, '\n']),
  getTime(secondo(QueryAtom, [int, ResCard]),QueryTime),
  dm(selectivity,['Elapsed Time: ', QueryTime, ' ms\n']), !.

selectivityQueryJoin(Pred, Rel1, Rel2, QueryTime, BBoxResCard, FilterResCard) :-
  Pred =.. [OP|_],
  isBBoxPredicate(OP),     % spatial predicate with bbox-checking
  transformPred(Pred, t, 1, Pred2),
  Pred2 =.. [_, Arg1, Arg2],
  Pred3 =.. [intersects, bbox(Arg1), bbox(Arg2)],
  ( optimizerOption(dynamicSample)
    -> ( dynamicPossiblyRenameJ(Rel1, Rel1Query),
         dynamicPossiblyRenameJ(Rel2, Rel2Query),
         Query = count(filter(counter(loopjoin(Rel1Query, fun([param(t, tuple)], 
                       filter(Rel2Query, Pred3))),1),Pred2) )
       )
    ;  ( sampleS(Rel1, Rel1S),
         sampleJ(Rel2, Rel2S),
         possiblyRename(Rel1S, Rel1Query),
         possiblyRename(Rel2S, Rel2Query),
         Rel2S = rel(BaseName, _, _),
         card(BaseName, JoinSize),
         Query = count(filter(counter(loopjoin(head(Rel1Query, JoinSize), 
                       fun([param(t, tuple)],
                       filter(Rel2Query, Pred3))),1), Pred2) )
       )
  ),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  dm(selectivity,['\nSelectivity query : ', QueryAtom, '\n']),
  getTime(secondo(QueryAtom, [int, FilterResCard]),QueryTime),
  dm(selectivity,['Elapsed Time: ', QueryTime, ' ms\n']),
  secondo('list counters',  [[1, BBoxResCard]|_]), !.

selectivityQueryJoin(Pred, Rel1, Rel2, QueryTime, noBBox, ResCard) :-
  Pred =.. [OP|_],
  not(isBBoxPredicate(OP)), % normal predicate
  transformPred(Pred, t, 1, Pred2),
  ( optimizerOption(dynamicSample)
    -> ( dynamicPossiblyRenameJ(Rel1, Rel1Query),
         dynamicPossiblyRenameJ(Rel2, Rel2Query),
         Query = count(loopsel(Rel1Query, fun([param(t, tuple)], 
                       filter(Rel2Query, Pred2))))
       )
    ;  ( Rel2S = rel(BaseName, _, _),
         sampleS(Rel1, Rel1S),
         sampleJ(Rel2, Rel2S),
         possiblyRename(Rel1S, Rel1Query),
         possiblyRename(Rel2S, Rel2Query),
         card(BaseName, JoinSize),
         Query = count(loopsel(head(Rel1Query, JoinSize), fun([param(t, tuple)],
                       filter(Rel2Query, Pred2))))
       )
   ),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  dm(selectivity,['\nSelectivity query : ', QueryAtom, '\n']),
  getTime(secondo(QueryAtom, [int, ResCard]),QueryTime),
  dm(selectivity,['Elapsed Time: ', QueryTime, ' ms\n']), !.


/*

----	transformPred(Pred, Param, Arg, Pred2) :- 
----

~Pred2~ is ~Pred~ transformed such that the attribute X of relation ~ArgNo~ is written
as ``attribute(Param, attrname(X))''

*/

transformPred(attr(Attr, Arg, Case), Param, Arg, 
  attribute(Param, attrname(attr(Attr, Arg, Case)))) :- !.

transformPred(attr(Attr, Arg, Case), _, _, attr(Attr, Arg, Case)) :- !.

transformPred(Pred, Param, Arg, Pred2) :-
  compound(Pred),
  functor(Pred, T, 1), !,
  arg(1, Pred, Arg1),
  transformPred(Arg1, Param, Arg, Arg1T),
  functor(Pred2, T, 1),
  arg(1, Pred2, Arg1T).

transformPred(Pred, Param, Arg, Pred2) :-
  compound(Pred),
  functor(Pred, T, 2), !,
  arg(1, Pred, Arg1),
  arg(2, Pred, Arg2),
  transformPred(Arg1, Param, Arg, Arg1T),
  transformPred(Arg2, Param, Arg, Arg2T),
  functor(Pred2, T, 2),
  arg(1, Pred2, Arg1T),
  arg(2, Pred2, Arg2T).

transformPred(Pred, _, _, Pred).


% Selectivities must not be 0

nonzero(0, 1) :- !.

nonzero(N, N).

/*
---- getTime(:Goal, -Time)
----

Measures the time used to execute ~Goal~ in milliseconds (ms).

*/

getTime(Goal, TimeMS) :-
  get_time(Time1),
  call(Goal),
  get_time(Time2),
  Time3 is Time2 - Time1,
  convert_time(Time3, _, _, _, _, Minute, Sec, MilliSec),
  TimeMS is Minute *60000 + Sec*1000 + MilliSec, !.


/*

----	selectivity(+P, -Sel) :-
----

The selectivity of predicate ~P~ is ~Sel~.

If ~selectivity~ is called, it first tries to look up
the selectivity via the predicate ~sel~. If no selectivity
is found, a Secondo query is issued, which determines the
selectivity. The retrieved selectitivity is then stored in
predicate ~storedSel~. This ensures that a selectivity has to
be retrieved only once.

Additionally, the time to evaluate a predicate is estimated by
dividing the query time by the number of predicate evaluations.
The result is stored in a table ~storedPET(DB, Pred, PET)~, where
~PET~ means ~Predicate Evaluation Time~.

*/
    
sels(Pred, Sel) :-
  databaseName(DB),
  storedSel(DB, Pred, Sel),
  !.

sels(Pred, Sel) :-
  commute(Pred, Pred2),
  databaseName(DB),
  storedSel(DB, Pred2, Sel).

% handle 'pseudo-joins' (2 times the same argument) as selections
selectivity(pr(Pred, Rel, Rel), Sel) :-
  selectivity(pr(Pred, Rel), Sel), !.

% check if selectivity has already been stored
selectivity(P, Sel) :-
  simplePred(P, PSimple),
  sels(PSimple, Sel), !.

% query for join-selectivity (static samples case)
selectivity(pr(Pred, Rel1, Rel2), Sel) :-
  not(optimizerOption(dynamicSample)),
  Rel1 = rel(BaseName1, _, _),
  sampleNameJ(BaseName1, SampleName1),
  card(SampleName1, SampleCard1),
  Rel2 = rel(BaseName2, _, _),
  sampleNameJ(BaseName2, SampleName2),
  card(SampleName2, SampleCard2),
  selectivityQueryJoin(Pred, Rel1, Rel2, MSs, BBoxResCard, ResCard),
  MSsRes is MSs / (SampleCard1 * SampleCard2),
  nonzero(ResCard, NonzeroResCard), 
  Sel is NonzeroResCard / (SampleCard1 * SampleCard2),	% must not be 0
  dm(selectivity,['Predicate Cost  : ', MSsRes, ' ms\nSelectivity     : ',
                  Sel,'\n']),
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  databaseName(DB),
  assert(storedPET(DB, PSimple, MSsRes)),
  assert(storedSel(DB, PSimple, Sel)), 
  ( ( BBoxResCard = noBBox ; storedBBoxSel(DB, PSimple, _) )
    -> true
    ;  ( nonzero(BBoxResCard, NonzeroBBoxResCard), 
         BBoxSel is NonzeroBBoxResCard / (SampleCard1 * SampleCard2),
         dm(selectivity,['BBox-Selectivity: ',BBoxSel,'\n']),
         assert(storedBBoxSel(DB, PSimple, BBoxSel))
       )
  ),!.

% query for selection-selectivity (static case)
selectivity(pr(Pred, Rel), Sel) :-
  not(optimizerOption(dynamicSample)),
  Rel = rel(BaseName, _, _),
  sampleNameS(BaseName, SampleName),
  card(SampleName, SampleCard),
  selectivityQuerySelection(Pred, Rel, MSs, BBoxResCard, ResCard),
  MSsRes is MSs / SampleCard,
  nonzero(ResCard, NonzeroResCard),
  Sel is NonzeroResCard / SampleCard,		% must not be 0
  dm(selectivity,['Predicate Cost: ', MSsRes, ' ms\nSelectivity : ',
                  Sel,'\n']),
  simplePred(pr(Pred, Rel), PSimple),
  databaseName(DB),
  assert(storedPET(DB, PSimple, MSsRes)),
  assert(storedSel(DB, PSimple, Sel)), 
  ( ( BBoxResCard = noBBox ; storedBBoxSel(DB, PSimple, _) )
    -> true
    ;  ( nonzero(BBoxResCard, NonzeroBBoxResCard), 
         BBoxSel is NonzeroBBoxResCard / SampleCard,
         dm(selectivity,['BBox-Selectivity: ',BBoxSel,'\n']),
         assert(storedBBoxSel(DB, PSimple, BBoxSel))
       )
  ),!.

% query for join-selectivity (dynamic sampling case)
selectivity(pr(Pred, Rel1, Rel2), Sel) :-
  optimizerOption(dynamicSample),
  Rel1 = rel(BaseName1, _, _),
  card(BaseName1, Card1),
  sampleSizeJoin(JoinSize),
  SampleCard1 is min(Card1, max(JoinSize, Card1 * 0.00001)),
  Rel2 = rel(BaseName2, _, _),
  card(BaseName2, Card2),
  SampleCard2 is min(Card2, max(JoinSize, Card2 * 0.00001)),
  selectivityQueryJoin(Pred, Rel1, Rel2, MSs, BBoxResCard, ResCard),
  MSsRes is MSs / (SampleCard1 * SampleCard2),
  nonzero(ResCard, NonzeroResCard),
  Sel is NonzeroResCard / (SampleCard1 * SampleCard2),	% must not be 0
  dm(selectivity,['Predicate Cost: ', MSsRes, ' ms\nSelectivity : ',Sel,'\n']),
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  databaseName(DB),
  assert(storedSel(DB, PSimple, Sel)), 
  ( ( BBoxResCard = noBBox ; storedBBoxSel(DB, PSimple, _) )
    -> true
    ;  ( nonzero(BBoxResCard, NonzeroBBoxResCard), 
         BBoxSel is NonzeroBBoxResCard / (SampleCard1 * SampleCard2),
         dm(selectivity,['BBox-Selectivity: ',BBoxSel,'\n']),
         assert(storedBBoxSel(DB, PSimple, BBoxSel))
       )
  ),!.

% query for selection-selectivity (dynamic sampling case)
selectivity(pr(Pred, Rel), Sel) :-
  optimizerOption(dynamicSample),
  Rel = rel(BaseName, _, _),
  card(BaseName, Card),
  sampleSizeSelection(SelectionSize),
  SampleCard is min(Card, max(SelectionSize, Card * 0.00001)),
  selectivityQuerySelection(Pred, Rel, MSs, BBoxResCard, ResCard),
  MSsRes is MSs / SampleCard,
  nonzero(ResCard, NonzeroResCard),
  Sel is NonzeroResCard / SampleCard,		% must not be 0
  dm(selectivity,['Predicate Cost: ', MSsRes, ' ms\nSelectivity : ',Sel,'\n']),
  simplePred(pr(Pred, Rel), PSimple),
  databaseName(DB),
  assert(storedSel(DB, PSimple, Sel)), 
  ( ( BBoxResCard = noBBox ; storedBBoxSel(DB, PSimple, _) )
    -> true
    ;  ( nonzero(BBoxResCard, NonzeroBBoxResCard), 
         BBoxSel is NonzeroBBoxResCard / SampleCard,
         dm(selectivity,['BBox-Selectivity: ',BBoxSel,'\n']),
         assert(storedBBoxSel(DB, PSimple, BBoxSel))
       )
  ),!.

% handle ERRORs
selectivity(P, _) :- write('Error in optimizer: cannot find selectivity for '),
  simplePred(P, PSimple), write(PSimple), nl, 
  write('Call: selectivity('), write(P), write(',Sel)\n'),
  throw(sql_ERROR(statistics_selectivity(P, undefined))), 
  fail, !.

/*

The selectivities retrieved via Secondo queries can be loaded
(by calling ~readStoredSels~) and stored (by calling
~writeStoredSels~).

*/

readStoredSels :-
  retractall(storedSel(_, _, _)),
  retractall(storedBBoxSel(_, _, _)),
  [storedSels].

/*

The following functions are auxiliary functions for ~writeStoredSels~. Their
purpose is to convert a list of character codes (e.g. [100, 99, 100]) to
an atom (e.g. "dcd"), which makes the stored selectitivities more
readable.

*/

isIntList([]).

isIntList([X | Rest]) :-
  integer(X),
  isIntList(Rest).

charListToAtom(CharList, Atom) :-
  atom_codes(A, CharList),
  concat_atom([' "', A, '"'], Atom).

replaceCharList(InTerm, OutTerm) :-
  isIntList(InTerm),
  !,
  charListToAtom(InTerm, OutTerm).

replaceCharList(InTerm, OutTerm) :-
  compound(InTerm),
  !,
  InTerm =.. TermAsList,
  maplist(replaceCharList, TermAsList, OutTermAsList),
  OutTerm =.. OutTermAsList.

replaceCharList(X, X).

writeStoredSels :-
  open('storedSels.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredSel(FD), _),
  close(FD).

writeStoredSel(Stream) :-
  storedSel(DB, X, Y),
  replaceCharList(X, XReplaced),
  write(Stream, storedSel(DB, XReplaced, Y)),
  write(Stream, '.\n').

writeStoredSel(Stream) :-
  storedBBoxSel(DB, X, Y),
  replaceCharList(X, XReplaced),
  write(Stream, storedBBoxSel(DB, XReplaced, Y)),
  write(Stream, '.\n').

showSel :- 
  storedSel(DB, X, Y),
  write(Y), write('\t\t'), write(DB), write('.'), write(X), nl.

showBBoxSel :- 
  storedBBoxSel(DB, X, Y),
  write(Y), write('\t\t'), write(DB), write('.'), write(X), nl.

showSels :-
  write('Stored selectivities:\n'),
  findall(_, showSel, _),
  write('Stored bbox-selectivities:\n'),
  findall(_, showBBoxSel, _) .

:-
  dynamic(storedSel/3),
  dynamic(storedBBoxSel/3),
  at_halt(writeStoredSels),
  readStoredSels.

readStoredPETs :-
  retractall(storedPET(_, _, _)),
  [storedPETs].  

writeStoredPETs :-
  open('storedPETs.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredPET(FD), _),
  close(FD).

writeStoredPET(Stream) :-  
  storedPET(DB, X, Y),
  replaceCharList(X, XReplaced),
  write(Stream, storedPET(DB, XReplaced, Y)),
  write(Stream, '.\n').

showPETs :-
  write('\nStored predicate costs:\n'),
  write('Cost [ms] \t\t Predicate\n'),
  findall(_, showPET, _).

showPET :-
  storedPET(DB, P, PC),
  replaceCharList(P, PReplaced),
  write(' '), write(PC), write('\t\t'), write(DB), 
  write('.'), write(PReplaced), nl.


:-
  dynamic(storedPET/3),
  at_halt(writeStoredPETs),
  readStoredPETs.

writePETs :-
  findall(_, writePET, _).

writePET :-
  storedPET(DB, X, Y),
  replaceCharList(X, XReplaced),
  write('DB: '),
  write(DB),
  write(', Predicate: '),
  write(XReplaced),
  write(', Cost: '),
  write(Y),
  write(' ms\n').

/*
 ~showDatabase~
 This predicate will inquire all collected statistical data on the 
 opened Secondo database and print it on the screen.

*/

showSingleRelationCard(DB, Rel) :-
  secRelation(Rel, RelExternal),
  lowerfl(RelExternal, RelExternalL),
  storedCard(DB, RelExternalL, Card),
  write('\n\n\tCardinality:   '), write(Card), nl, !.

showSingleRelationCard(_, _) :-
  write('\n\n\tCardinality:   *'), nl, !.

showSingleRelationTuplesize(DB, Rel) :-  
  secRelation(Rel, RelExternal),
  lowerfl(RelExternal, RelExternalL),
  storedTupleSize(DB, RelExternalL, Size),
  write('\tAvg.TupleSize: '), write(Size), nl, !.

showSingleRelationTuplesize(_, _) :-
  write('\tAvg.TupleSize: *'), nl, !.

showSingleIndex(Rel) :-
  databaseName(DB),
  secRelation(Rel, RelExternal),
  lowerfl(RelExternal, RelExternalL),
  storedIndex(DB, RelExternalL, Attr, IndexType, _),
  secAttr(Rel, Attr, AttrS),
  write('\t('), write(AttrS), write(':'), write(IndexType), write(')').

showSingleRelation :-
  databaseName(DB),
  storedRel(DB, Rel, _),
  secRelation(Rel, RelS),
  write('\nRelation '), write(RelS), nl,
  findall(_, showAllAttributes(Rel), _),
  findall(_, showAllIndices(Rel), _),
  showSingleRelationCard(DB, Rel),
  showSingleRelationTuplesize(DB, Rel).

showSingleAttribute(Rel,Attr) :-
  databaseName(DB),
  storedAttrSize(DB, Rel, Attr, Type, CoreTupleSize, InFlobSize, ExtFlobSize),
  secAttr(Rel, Attr, AttrS),
  write('\t'), write(AttrS), write('\t\t'), 
  write(Type), write('\t'),
  write(CoreTupleSize), write('\t'),
  write(InFlobSize),  write('\t'),
  write(ExtFlobSize), nl. 

showAllAttributes(Rel) :-
  write('\tAttr\t\tType\tCoreSz\tIFlobSz\tExtFlobSz\n'),
  findall(_, showSingleAttribute(Rel, _), _).

showAllIndices(Rel) :-
  write('\n\tIndices: \n'),
  findall(_, showSingleIndex(Rel), _).

showDatabase :-
  databaseName(DB),
  write('\nCollected information for database \''), write(DB), write('\':\n'),
  findall(_, showSingleRelation, _),
  write('\n(Type \'showDatabaseSchema.\' to view the complete database schema.)\n').

showDatabase :-
  write('\nNo database open. Use open \'database <name>\' to open an existing database.\n'),
  fail. 


/*
1.5 Examples

Example 22:

*/
example24 :- optimize(
  select *
  from [staedte as s, ten]
  where [
    s:plz < 1000 * no,
    s:plz < 4578]
  ).

/*
Example 23:

*/

example23 :- optimize(
  select *
  from [thousand as th, ten]
  where [
    (th:no mod 10) < 5,
    th:no * 100 < 50000,
    (th:no mod 7) = no]
  ).

/*

1.6 Auxiliary Predicates

Convert internal name representations to valid identifiers in Secondo

*/

secRelation(Internal, External) :- 
  databaseName(DB),
  storedSpell(DB, Internal, lc(External)),!.

secRelation(Internal, ExternalU) :-
  databaseName(DB),
  storedSpell(DB, Internal, External),
  upper(External, ExternalU),!.

secRelation(Internal, Internal) :- !.
  
secAttr(Rel, Internal, External) :- 
  databaseName(DB),
  storedSpell(DB, Rel:Internal, lc(External)),!.

secAttr(Rel, Internal, ExternalU) :-
  databaseName(DB),
  storedSpell(DB, Rel:Internal, External),
  upper(External, ExternalU),!.

secAttr(_, Internal, Internal) :- !.
