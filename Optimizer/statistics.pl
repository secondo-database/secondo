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

1.4 Retrieving, Storing, and Loading Selectivities and PETs

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
  Query = count(filter(counter(filter(RelQuery, BBoxPred),1), Pred)),
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

----	
selectivity(+P, -Sel)
selectivity(+P, -Sel, -CalcPET, -ExpPET)
getPET(+P, -CalpPET, -ExpPET)
----

The selectivity of predicate ~P~ is ~Sel~. The analytic predicate cost function reports the evaluation of the predicate to take ~CalcPET~ milliseconds of time. During the actual query, the evaluation took ~ExpPET~ milliseconds of time for a single evaluation. 

If ~selectivity~ is called, it first tries to look up
the selectivity via the predicate ~sel~. If no selectivity
is found, a Secondo query is issued, which determines the
selectivity. The retrieved selectitivity is then stored in
predicate ~storedSel~. This ensures that a selectivity has to
be retrieved only once.

Additionally, the time to evaluate a predicate is estimated by
dividing the query time by the number of predicate evaluations.
The result is stored in a table ~storedPET(DB, Pred, CalcPET, ExpPET)~, where
~PET~ means ~Predicate Evaluation Time~.

*/

sels(Pred, Sel, CalcPET, ExpPET) :-
  databaseName(DB),
  storedSel(DB, Pred, Sel),
  storedPET(DB, Pred, CalcPET, ExpPET), !.

sels(Pred, Sel, CalcPET, ExpPET) :-
  commute(Pred, Pred2),
  databaseName(DB),
  storedSel(DB, Pred2, Sel), 
  storedPET(DB, Pred2, CalcPET, ExpPET), !.


% Wrapper for standard optimizer
selectivity(Pred, Sel) :-
  selectivity(Pred, Sel, _, _).

selectivity(P, _) :- write('Error in optimizer: cannot find selectivity for '),
  simplePred(P, PSimple), write(PSimple), nl, 
  write('Call: selectivity('), write(P), write(',Sel)\n'),
  throw(sql_ERROR(statistics_selectivity(P, undefined))), 
  fail, !.


% handle 'pseudo-joins' (2 times the same argument) as selections
selectivity(pr(Pred, Rel, Rel), Sel, CalcPET, ExpPET) :-
  selectivity(pr(Pred, Rel), Sel, CalcPET, ExpPET), !.

% check if selectivity has already been stored
selectivity(P, Sel, CalcPET, ExpPET) :-
  simplePred(P, PSimple),
  sels(PSimple, Sel, CalcPET, ExpPET), !.

% query for join-selectivity (static samples case)
selectivity(pr(Pred, Rel1, Rel2), Sel, CalcPET, ExpPET) :-
  not(optimizerOption(dynamicSample)),
  Rel1 = rel(BaseName1, _, _),
  sampleNameJ(BaseName1, SampleName1),
  card(SampleName1, SampleCard1),
  Rel2 = rel(BaseName2, _, _),
  sampleNameJ(BaseName2, SampleName2),
  card(SampleName2, SampleCard2),
  selectivityQueryJoin(Pred, Rel1, Rel2, MSs, BBoxResCard, ResCard),
  nonzero(ResCard, NonzeroResCard), 
  Sel is NonzeroResCard / (SampleCard1 * SampleCard2),	% must not be 0
  tupleSizeSplit(BaseName1,TupleSize1),
  tupleSizeSplit(BaseName2,TupleSize2),
  calcExpPET(MSs, SampleCard1, TupleSize1, 
                  SampleCard2, TupleSize2, NonzeroResCard, MSsRes), % correct PET
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  predCost(PSimple, CalcPET), % calculated PET
  ExpPET is MSsRes / max(SampleCard1 * SampleCard2, 1),
  dm(selectivity,['Predicate Cost  : (', CalcPET, '/', ExpPET, 
                  ') ms\nSelectivity     : ', Sel,'\n']),
  databaseName(DB),
  assert(storedPET(DB, PSimple, CalcPET, ExpPET)),
  assert(storedSel(DB, PSimple, Sel)), 
  ( ( BBoxResCard = noBBox ; storedBBoxSel(DB, PSimple, _) )
    -> true
    ;  ( nonzero(BBoxResCard, NonzeroBBoxResCard), 
         BBoxSel is NonzeroBBoxResCard / (SampleCard1 * SampleCard2),
         dm(selectivity,['BBox-Selectivity: ',BBoxSel,'\n']),
         assert(storedBBoxSel(DB, PSimple, BBoxSel))
       )
  ),!.

% query for selection-selectivity (static samples case)
selectivity(pr(Pred, Rel), Sel, CalcPET, ExpPET) :-
  not(optimizerOption(dynamicSample)),
  Rel = rel(BaseName, _, _),
  sampleNameS(BaseName, SampleName),
  card(SampleName, SampleCard),
  selectivityQuerySelection(Pred, Rel, MSs, BBoxResCard, ResCard),
  nonzero(ResCard, NonzeroResCard),
  Sel is NonzeroResCard / SampleCard,		% must not be 0
  tupleSizeSplit(BaseName,TupleSize),
  calcExpPET(MSs, SampleCard, TupleSize, MSsRes), % correct PET
  simplePred(pr(Pred, Rel), PSimple),
  predCost(PSimple,CalcPET), % calculated PET
  ExpPET is MSsRes / max(SampleCard,1),
  dm(selectivity,['Predicate Cost  : (', CalcPET, '/', ExpPET, 
                  ') ms\nSelectivity     : ', Sel,'\n']),
  databaseName(DB),
  assert(storedPET(DB, PSimple, CalcPET, ExpPET)),
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
selectivity(pr(Pred, Rel1, Rel2), Sel, CalcPET, ExpPET) :-
  optimizerOption(dynamicSample),
  Rel1 = rel(BaseName1, _, _),
  card(BaseName1, Card1),
  sampleSizeJoin(JoinSize),
  SampleCard1 is min(Card1, max(JoinSize, Card1 * 0.00001)),
  Rel2 = rel(BaseName2, _, _),
  card(BaseName2, Card2),
  SampleCard2 is min(Card2, max(JoinSize, Card2 * 0.00001)),
  selectivityQueryJoin(Pred, Rel1, Rel2, MSs, BBoxResCard, ResCard),
  nonzero(ResCard, NonzeroResCard),
  Sel is NonzeroResCard / (SampleCard1 * SampleCard2),	% must not be 0
  tupleSizeSplit(BaseName1,TupleSize1),
  tupleSizeSplit(BaseName2,TupleSize2),
  calcExpPET(MSs, SampleCard1, TupleSize1, 
                  SampleCard2, TupleSize2, NonzeroResCard, MSsRes), % correct PET
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  predCost(PSimple,CalcPET), % calculated PET
  ExpPET is MSsRes / max(SampleCard1 * SampleCard2,1),
  dm(selectivity,['Predicate Cost  : (', CalcPET, '/', ExpPET, 
                  ') ms\nSelectivity     : ', Sel,'\n']),

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
selectivity(pr(Pred, Rel), Sel, CalcPET, ExpPET) :-
  optimizerOption(dynamicSample),
  Rel = rel(BaseName, _, _),
  card(BaseName, Card),
  sampleSizeSelection(SelectionSize),
  SampleCard is min(Card, max(SelectionSize, Card * 0.00001)),
  selectivityQuerySelection(Pred, Rel, MSs, BBoxResCard, ResCard),
  nonzero(ResCard, NonzeroResCard),
  Sel is NonzeroResCard / SampleCard,		% must not be 0
  tupleSizeSplit(BaseName,TupleSize),
  calcExpPET(MSs, SampleCard, TupleSize, MSsRes), % correct PET
  simplePred(pr(Pred, Rel), PSimple),
  predCost(PSimple,CalcPET), % calculated PET
  ExpPET is MSsRes / max(SampleCard,1),
  dm(selectivity,['Predicate Cost  : (', CalcPET, '/', ExpPET, 
                  ') ms\nSelectivity     : ', Sel,'\n']),
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
selectivity(P, _, _, _) :- write('Error in optimizer: cannot find selectivity for '),
  simplePred(P, PSimple), write(PSimple), nl, 
  write('Call: selectivity('), write(P), write(', _, _, _)\n'),
  throw(sql_ERROR(statistics_selectivity(P, undefined, undefined, undefined))), 
  fail, !.


% access stored PETs by simplified predicate term
getPET(P, CalcPET, ExpPET) :-
  databaseName(DB),
  simplePred(P,PSimple),
  storedPET(DB, PSimple, CalcPET, ExpPET), !.

getPET(P, _, _) :- write('Error in optimizer: cannot find PETs for '),
  simplePred(P, PSimple), write(PSimple), nl, 
  write('Call: getPET('), write(P), write(', _, _)\n'),
  throw(sql_ERROR(statistics_getPET(P, undefined, undefined))), 
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
  write(Stream, storedSel(DB, XReplaced, Y)), write(Stream, '.\n').

writeStoredSel(Stream) :-
  storedBBoxSel(DB, X, Y),
  replaceCharList(X, XReplaced),
  write(Stream, storedBBoxSel(DB, XReplaced, Y)), write(Stream, '.\n').

showSel :- 
  storedSel(DB, X, Y),
  replaceCharList(X, XRepl),
  format('  ~p~16|~p.~p~n',[Y,DB,XRepl]).
%  write(Y), write('\t\t'), write(DB), write('.'), write(X), nl.

showBBoxSel :- 
  storedBBoxSel(DB, X, Y),
  replaceCharList(X, XRepl),
  format('  ~p~16|~p.~p~n',[Y,DB,XRepl]).
%  write(Y), write('\t\t'), write(DB), write('.'), write(X), nl.

showSels :-
  write('\nStored selectivities:\n'),
  findall(_, showSel, _),
  write('\nStored bbox-selectivities:\n'),
  findall(_, showBBoxSel, _) .

:-
  dynamic(storedSel/3),
  dynamic(storedBBoxSel/3),
  at_halt(writeStoredSels),
  readStoredSels.

readStoredPETs :-
  retractall(storedPET(_, _, _, _)),
  [storedPETs].  

writeStoredPETs :-
  open('storedPETs.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredPET(FD), _),
  close(FD).

writeStoredPET(Stream) :-  
  storedPET(DB, X, Y, Z),

  replaceCharList(X, XReplaced),
  write(Stream, storedPET(DB, XReplaced, Y, Z)), write(Stream, '.\n').

showPETs :-
  write('\nStored predicate costs:\n'),
  format('  ~p~18|~p~34|~p~n',['Calculated [ms]', 'Measured [ms]','Predicate']),
  findall(_, showPET, _).

showPET :-
  storedPET(DB, P, Calc, Exp),
  replaceCharList(P, PRepl),
  format('  ~p~18|~p~34|~p.~p~n',[Calc, Exp, DB, PRepl]).

:-
  dynamic(storedPET/4),
  at_halt(writeStoredPETs),
  readStoredPETs.

writePETs :-
  findall(_, writePET, _).

writePET :-
  storedPET(DB, X, Y, Z),
  replaceCharList(X, XReplaced),
  write('DB: '), write(DB),
  write(', Predicate: '), write(XReplaced),
  write(', Cost: '), write(Y), write('/'), write(Z), write(' ms\n').


/*
1.5 Printing Metadata on Database 

---- showDatabase
----

This predicate will inquire all collected statistical data on the 
opened Secondo database and print it on the screen.

*/

showSingleOrdering(DB, Rel) :-
  secRelation(Rel, RelExternal),
  findall( X, 
           ( storedOrder(DB,RelExternal,X1),
             translateOrderingInfo(Rel, X1,X)
           ),
           OrderingAttrs
         ),
  write('\n\n\tOrdering:  '),
  write(OrderingAttrs), nl, !.

translateOrderingInfo(_, none,none) :- !.
translateOrderingInfo(_, shuffled,shuffled) :- !.
translateOrderingInfo(Rel, Attr, AttrS) :-
  secAttr(Rel, Attr, AttrS).

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
  tupleSizeSplit(Rel, Size2),
  write('\tAvg.TupleSize: '), write(Size), write(' = '), write(Size2), nl, !.

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
  showSingleOrdering(DB, Rel),
  showSingleRelationCard(DB, Rel),
  showSingleRelationTuplesize(DB, Rel).

showSingleAttribute(Rel,Attr) :-
  databaseName(DB),
  storedAttrSize(DB, Rel, Attr, Type, CoreTupleSize, InFlobSize, ExtFlobSize),
  secAttr(Rel, Attr, AttrS),
  format('\t~p~35|~p~49|~p~60|~p~69|~p~n',[AttrS,Type,CoreTupleSize,InFlobSize,ExtFlobSize]).

showAllAttributes(Rel) :-
  format('\t~p~35|~p~49|~p~60|~p~69|~p~n',['AttributeName','Type','CoreSz','IFlobSz','ExtFlobSz']),
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
1.5 Determining System Speed and Calibration Factors

To achieve good cost estimations, the used cost factors for operators need to be calibrated. 

*/

/*
The cost factors for the cost functions are read from a file:

*/

readStoredOperatorTF :-
  retractall(tempOperatorTF(_)),
  retractall(storedOperatorTF(_)),
  [sysDependentOperatorTF].


/*
The cost factors for the cost functions are stored into a file;

*/

writeStoredOperatorTF :-
  open('sysDependentOperatorTF.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredOperatorTF(FD), _),
  close(FD).

writeStoredOperatorTF(Stream) :-
  storedOperatorTF(Op, A, B),
  % replaceCharList(X, XReplaced),
  write(Stream, storedOperatorTF(Op, A, B)),
  write(Stream, '.\n').

/* 
The Original cost factors for operators are:

*/
setOriginalOperatorTFs :-
  assert(storedOperatorTF(consume, 0.00000147, 0.0003)),
  assert(storedOperatorTF(loopjoin, 0.000000095, 0.000014)),
  assert(storedOperatorTF(filter, 0, 0)),
  assert(storedOperatorTF(string, notype, 0.00000053)),
  assert(storedOperatorTF(int, notype, 0.00000051)),
  assert(storedOperatorTF(real, notype, 0.00000052)),
  assert(storedOperatorTF(feed, 0.000000735, 0)),
  assert(storedOperatorTF(producta, 0.00000781, 0.00453249)),
  assert(storedOperatorTF(productb, 0.000049807, 0.008334)),
  assert(storedOperatorTF(productc, 0.434782, 0)),
  assert(storedOperatorTF(hashtemp, 0.000000789, 0.0002168)),
  assert(storedOperatorTF(hashcachecal, 0.1, 61.2)),
  assert(storedOperatorTF(hashtab, 0.000000766, 0)),
  assert(storedOperatorTF(extend, 0.000015, 0)),
  assert(storedOperatorTF(intsort, 0.000000083, 0.00001086)),
  assert(storedOperatorTF(extsort, 0.000000218, 0.000125193)),
  assert(storedOperatorTF(concatenationjoin, 0.000000095, 0.000014)),
  assert(storedOperatorTF(concatenationflob, 0.00000171, 0)),
  assert(storedOperatorTF(spatialjoinx, 0.000000216, 0.00000000004)),
  assert(storedOperatorTF(spatialjoiny, 0.00000024,  0.000000000045)),
  assert(storedOperatorTF(remove, 0.000015, 0)),
  assert(storedOperatorTF(rename, 0.00001, 0)),
  assert(storedOperatorTF(project, 0, 0)),
  assert(storedOperatorTF(count, 0, 0)),
  assert(storedOperatorTF(orderby, 0, 0)),
  assert(storedOperatorTF(groupby, 0, 0)),
  assert(storedOperatorTF(exactmatchfun, 0.000002265, 0.000298)),
  assert(storedOperatorTF(exactmatch, 0.000002265, 0.000298)),
  assert(storedOperatorTF(leftrange, 0.000002265, 0.000298)),
  assert(storedOperatorTF(windowintersects, 0.000000972, 0)),
  assert(storedOperatorTF(insidelr, 0.016712944, 0)).

:- dynamic(storedOperatorTF/3),
  at_halt(writeStoredOperatorTF),
  readStoredOperatorTF.

:- ( not(storedOperatorTF(_,_,_))
     -> setOriginalOperatorTFs
     ;  true
   ).


/*
Estimate the speed the stytem by posing a specific query, calculating the used time and comparing it with the value of the appropriate cost function. From this, calculate a new calibration factor.

*/

tfCPU(TestRel) :-
  concat_atom(['query ', TestRel, ' feed count'],'',Query),
  getTime(secondo(Query, [_, _]),Time),
  downcase_atom(TestRel, DCTestRel),
  card(DCTestRel, Card),
  tupleSizeSplit(DCTestRel, sizeTerm(Core, InFlob, _)),
  TLExt is Core + InFlob,
  testRelVolume(Card, TLExt),
  storedOperatorTF(feed, A, B),
  Cost is (TLExt * A + B) * Card,
  CFnew is Time / (Cost * 1000),
  vCostFactorCPU(V),
  nl, write(' The current CPU calibration factor: '), write(V),
  nl, write(' The suitable factor: '), write(CFnew),
  setCostFactor(tempCostFactor(vCostFactorCPU,CFnew)).

testRelVolume(Size, TupleSizeExt) :-
  Volume is Size * TupleSizeExt,
  Volume > 3500000,
  !.

testRelVolume(_, _) :-
  nl,
  write('Test relation should have a size of at least 3,5 MB (without Flobs).'),
  fail.


/*
Create a new clause for the temporary storage of the new calibration factor.

*/

setCostFactor(tempCostFactor(Op, CF)) :- 
  retract(tempCostFactor(Op, _)),
  assert(tempCostFactor(Op, CF)), !.

setCostFactor(tempCostFactor(Op, CF)) :- 
  assert(tempCostFactor(Op, CF)).

/*
Replaces the old calibration factor with a new one.

*/

updateCostFactorCPU :-
  tempCostFactor(vCostFactorCPU, CF),
  retract(vCostFactorCPU(_)),
  assert(vCostFactorCPU(CF)),
  write('vCostFactorCPU: '), write(CF),
  write(' update').

updateCostFactorCPU(CF) :-
  retract(vCostFactorCPU(_)),
  assert(vCostFactorCPU(CF)),
  write('vCostFactorCPU: '), write(CF),
  write(' update').

/*
The calibration factor and the conversion factor for the cost functions
are read from a file ``costFactors.pl:

*/

readStoredCostFactors :-
  retractall(wCostFactor(_)),
  retractall(vCostFactorCPU(_)),
  retractall(tempCostFactor(_,_)),
  [sysDependentCostFactors].

writeStoredCostFactors :-
  open('sysDependentCostFactors.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredCostFactors(FD), _),
  close(FD).

/*
The conversion factor for the cost functions is written back into the file.

*/

writeStoredCostFactors(Stream) :-
  wCostFactor(X),
  % replaceCharList(X, XReplaced),
  write(Stream, wCostFactor(X)),
  write(Stream, '.\n').

/*
The calibration factor of the cost functions is written back into the file.

*/

writeStoredCostFactors(Stream) :-
  vCostFactorCPU(X),
  % replaceCharList(X, XReplaced),
  write(Stream, vCostFactorCPU(X)),
  write(Stream, '.\n').

setOriginalCalibrationFactors :-
  assert(wCostFactor(1000000)),
  assert(vCostFactorCPU(1)).


:-
  dynamic(costFactors/1),
  at_halt(writeStoredCostFactors),
  readStoredCostFactors.

:- ( (not(wCostFactor(_)) ; not(vCostFactorCPU(_)))
     -> (   retractall(wCostFactor(_)),
            retractall(vCostFactorCPU(_)),
            retractall(tempCostFactor(_,_)),
            setOriginalCalibrationFactors
        )
     ;  true
   ).

/* 
1.6 Estimating PETs using an Analytical Model

---- predCost(+Pred, -PredCost)
     predCost(+Predicate, -PredCost, -ArgTypeX, -ArgSize, +predArg(PA))
----

Calculation of the costs of a predicate. To get the total costs of a predicate,
the costs of its sub-terms are assessed first. The estimated costs are based
on the attribute sizes and the attribute types.

If the type of a term cannot be dertermined, a failure value (e.g. ~undefined~) should be returned. The clauses should be modified to handle the case where a recursive call yields an undefined result.

*/

predCost(Pred, PredCost) :-
  predCost(Pred, PredCost, _, _, predArg(1)).

predCost(Pred, PredCost, predArg(N)) :-
  predCost(Pred, PredCost, _, _, predArg(N)).

predCost(X = Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  equalTCnew(ArgTypeX,  ArgTypeY, OperatorTC), % should somehow depend on ArgSize
  PredCost is PredCostX + PredCostY + OperatorTC. 

predCost(X < Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  smallerTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + OperatorTC.

predCost(X > Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  biggerTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + OperatorTC.

predCost(X <= Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  equalsmallerTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + OperatorTC.

predCost(X >= Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  equalbiggerTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + OperatorTC.

predCost(X inside Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  insideTCnew(ArgTypeX, ArgTypeY, OperatorTC, S),
  sTTS(ArgSizeX,ArgSizeXT),
  sTTS(ArgSizeY,ArgSizeYT),
  PredCost is PredCostX + PredCostY 
            + (ArgSizeXT ** S) * (ArgSizeYT * OperatorTC ) / 100000.

predCost(X adjacent Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  adjacentTCnew(ArgTypeX,  ArgTypeY, OperatorTC, S),
  sTTS(ArgSizeX,ArgSizeXT),
  sTTS(ArgSizeY,ArgSizeYT),
  PredCost is PredCostX + PredCostY
              + (ArgSizeXT * OperatorTC) * (ArgSizeYT ** S) / 100000.

predCost(X intersects Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  intersectsTCnew(ArgTypeX,  ArgTypeY, OperatorTC, S),
  sTTS(ArgSizeX,ArgSizeXT),
  sTTS(ArgSizeY,ArgSizeYT),
  PredCost is PredCostX + PredCostY
              + (ArgSizeXT * OperatorTC)*(ArgSizeYT ** S) / 100000.

predCost(X contains Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  containsTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + OperatorTC.

predCost(X + Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX, ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  plusTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + PA * OperatorTC.

predCost(X - Y, PredCost, ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX,  ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY, ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  minusTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + PA * OperatorTC.

predCost(X * Y, PredCost,  ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX,  ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY,  ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  timesTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + PA * OperatorTC.

predCost(X / Y, PredCost,  ArgTypeX, ArgSize, predArg(PA)) :- !,
  predCost(X, PredCostX,  ArgTypeX, ArgSizeX, predArg(PA)),
  predCost(Y, PredCostY,  ArgTypeY, ArgSizeY, predArg(PA)),
  biggerArg(ArgSizeX, ArgSizeY, ArgSize),
  overTCnew(ArgTypeX,  ArgTypeY, OperatorTC),
  PredCost is PredCostX + PredCostY + PA * OperatorTC.

predCost(Rel:Attr, 0,  ArgType, ArgSize, _) :- !,
  downcase_atom(Attr, DCAttr),
  downcase_atom(Rel, DCRel),
  attrType(DCRel:DCAttr, ArgType),
  attrSize(DCRel:DCAttr, ArgSize), !.


predCost(_, 0, notype, 1, _) :- !.

biggerArg(sizeTerm(X1,X2,X3), sizeTerm(Y1,Y2,Y3), sizeTerm(X1,X2,X3)) :- 
  X1+X2+X3 >= Y1+Y2+Y3, !.

biggerArg(_, Arg2, Arg2).

/*
~sTTS~ meams sizeTerm to Total Size.

Maps a sizeTerm to a single value, equal to the sum of the term's components.

*/
sTTS(sizeTerm(X,Y,Z),T) :-
  T is X + Y + Z.

sTTS(T,T).


/*
Different operators can occur within a predicate. Cost factors for certain predicates and different combinations of argument types are stored in the following Prolog facts:

*/

plusTCnew(_, _, 0.0000005).
minusTCnew(_, _, 0.0000005).
smallerTCnew(_, _, 0.0000005).
biggerTCnew(_, _, 0.0000005).
timesTCnew(_, _, 0.0000005).
overTCnew(_, _, 0.0000005).
modTCnew(_, _, 0.0000005).
divTCnew(_, _, 0.0000005).
equalTCnew(_, _, 0.0000005).
equalbiggerTCnew(_, _, 0.0000005).
equalsmallerTCnew(_, _, 0.0000005).
insideTCnew(point, line, 0.05086, 0).
insideTCnew(point, region, 0.04457463, 0).
insideTCnew(line, region, 0.046230259, 0).
insideTCnew(region, region, 0.00004444, 1).
insideTCnew(_, _, 0.046230259, 0).
adjacentTCnew(line, region, 0.000004702, 1).
adjacentTCnew(region, line, 0.000004702, 1).
adjacentTCnew(region, region, 0.046612378, 0).
adjacentTCnew(line, line, 0.051482479, 0).
adjacentTCnew(_, _, 0.000040983, 0).
intersectsTCnew(line, line, 0.041509433, 0).
intersectsTCnew(line, region, 0.000046277, 0).
intersectsTCnew(region, line, 0.000046277, 0).
intersectsTCnew(region, region, 0.046612328, 0).
intersectsTCnew(_, _, 0.000046277, 0).
containsTCnew(_, _, 0.00002).



/*
1.7 Correcting Measured PETs

When ~optimizerOption(nawracosts))~ is defined, PETs will be corrected:

---- 
calcExpPET(+MSs, +Card, +TupleSize, -Time)
calcExpPET(+MSs, +Card1, +TupleSize1, 
                 +Card2, +TupleSize2, +ResCard, -Time)
----
Calculation of experimental net-PETs (predicate evaluation times):

Reduce the measured time ~MSs~ when determinating the selectivity of selection-predicates 
by the estimated costs of operator feed.

Reduce the measured time when determining the selectivity of join-predicates
by the estimated costs of the operators feed and loopjoin.

Other arguments: ~Card~, ~Card1~, ~Card2~ are the cardinality of the input streams; ~TupleSize~, ~TupleSize1~, ~TupleSize2~ are the tuple sizes of the input streams, ~ResCard~ is the cardinality of the join.

~Time~ is the sesulting net-PET in milliseconds.

*/


calcExpPET(MSs, Card, TupleSize, Time) :-
  optimizerOption(nawracosts),
  storedOperatorTF(feed, OpA, OpB),
  wCostFactor(W),
  TupleSize = sizeTerm(Core, InFlob, _),
  Cost is  (OpA * (Core + InFlob) + OpB) * Card,
  costtotal(Cost, CostTotal),
  TimeOp is MSs - (CostTotal / W) * 1000,
  notnegativ(TimeOp, Time), !.

calcExpPET(MSs, _, _, MSs) :-
  not(optimizerOption(nawracosts)), !.

calcExpPET(MSs, Card1, TupleSize1, Card2, TupleSize2, ResCard, Time) :-
  optimizerOption(nawracosts),
  storedOperatorTF(feed, OpA, OpB),
  storedOperatorTF(loopjoin, A, B),
  storedOperatorTF(concatenationflob, A2, _),
  TupleSize1 = sizeTerm(Core1, InFlob1, ExtFlob1),
  TupleSize2 = sizeTerm(Core2, InFlob2, ExtFlob2),
  FlobSize is ExtFlob1 + ExtFlob2,
  ExtSize1 is Core1 + InFlob1,
  ExtSize2 is Core2 + InFlob2,
  wCostFactor(W),
  Cost is  (OpA * ExtSize1 + OpB) * Card1 + (OpA * ExtSize2 + OpB) * Card2
          + (((ExtSize1 + ExtSize2) * A + B) + (FlobSize * A2)) * ResCard,
  costtotal(Cost, CostTotal),
  TimeOp is MSs - (CostTotal / W) * 1000,
  notnegativ(TimeOp, Time), !.

calcExpPET(MSs, _, _, _, _, _, MSs) :-
  not(optimizerOption(nawracosts)), !.

/*
1.8 Examples

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

1.9 Auxiliary Predicates

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

