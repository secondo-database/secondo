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

*/

commute(X = Y, Y = X).
commute(X < Y, Y > X).
commute(X <= Y, Y >= X).
commute(X > Y, Y < X).
commute(X >= Y, Y <= X).
commute(X # Y, Y # X).



/*
1.2 Hard-Coded Selectivities of Predicates

*/

sel(plz:ort = staedte:sName, 0.0031).
sel(plz:pLZ = (plz:pLZ)+1, 0.00001644).
sel((plz:pLZ)-1 = plz:pLZ, 0.00001644).
sel(plz:pLZ = (plz:pLZ)*5, 0.0000022).
sel(plz:pLZ = plz:pLZ, 0.000146).
sel(plz:pLZ * 3 = plz:pLZ * 3, 0.000146).
sel(plz:pLZ > 40000, 0.55).
sel(plz:pLZ > 50000, 0.48).
sel(plz:pLZ < 60000, 0.64).
sel((plz:pLZ mod 5) = 0, 0.17).
sel(plz:ort contains "burg", 0.060).
sel(plz:ort starts "M", 0.071).
sel(staedte:bev > 500000, 0.21).
sel(staedte:bev > 300000, 0.26).
sel(staedte:bev < 500000, 0.79).
sel(staedte:kennzeichen starts "F", 0.034).
sel(staedte:kennzeichen starts "W", 0.068).

/*

1.3 Determine the Simple Form of Predicates

Simple forms of predicates are stored in predicate ~sel~ or
in predicate ~storedSel~.


----	simple(Term, Rel1, Rel2, Simple) :-
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
simplePred(X, _) :- throw(X).

/*

1.4 Retrieving, Storing, and Loading Selectivities and Predicate Costs

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

dynamicPossiblyRename(Rel, Renamed) :-
  Rel = rel(_, *, _),
  !,
  Renamed = sample(Rel, 500, 0.00001).

dynamicPossiblyRename(Rel, Renamed) :-
  Rel = rel(_, Name, _),
  Renamed = rename(sample(Rel, 500, 0.00001), Name).

cardQuery(Pred, Rel, Query) :-
  sampleS(Rel, RelS),
  possiblyRename(RelS, RelQuery),
  Query = count(filter(RelQuery, Pred)).

cardQuery(Pred, Rel1, Rel2, Query) :-
  sampleJ(Rel1, Rel1S),
  sampleJ(Rel2, Rel2S),
  possiblyRename(Rel1S, Rel1Query),
  possiblyRename(Rel2S, Rel2Query),
%  transformPred(Pred, t, 1, Pred2),
%  Query = count(loopsel(Rel1Query, fun([param(t, tuple)], filter(Rel2Query, Pred2)))).
  Query = count(symmjoin(Rel1Query, Rel2Query, Pred)).

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

dynamicCardQuery(Pred, Rel, Query) :-
  dynamicPossiblyRename(Rel, RelQuery),
  Query = count(filter(RelQuery, Pred)).

dynamicCardQuery(Pred, Rel1, Rel2, Query) :-
  dynamicPossiblyRename(Rel1, Rel1Query),
  dynamicPossiblyRename(Rel2, Rel2Query),
  Query = count(filter(product(Rel1Query, Rel2Query), Pred)).

/*

----    getPredCostDivisor(Pred, ResultSize, QuerySize, Divisor)
----
is used to determine the divisor within the ~selectivity~ predicates. 
It unifies ~Divisor~ with ~ResultSize~ when ~isBBoxOperator(Pred)~ holds
and ~QuerySize~ otherwise. To avoid division by zero exceptions, ~Divisor~ 
is at least 1.

*/

getPredCostDivisor(Pred, ResultSize, _, Divisor) :-
  compound(Pred),
  functor(Pred, T, _),
  isBBoxOperator(T),
  Divisor is max(ResultSize, 1),
  nl, write('BBox-Predicate: '), write(Pred), nl,
  !.

getPredCostDivisor(Pred, _, QuerySize, Divisor) :- 
  Divisor is max(QuerySize, 1),
  nl, write('Ordinary predicate: '), write(Pred), nl,
  !.

/*
----    cacheRelation(Rel) 
----
ensures the presence of relation ~RelName~ in the system's caches 
by posing a simple query to secondo. The 4 last used Relations are 
deemed resident within the caches. 

*/

:- dynamic(cachedRelation/2).
:- dynamic(cachedRelCounter/1).

incCachedRelCounter :-
  cachedRelCounter(N),
  !,
  N1 is mod((N+1),4),  % here, the second argument to mod is the number of relations resident in memory
  retract(cachedRelCounter(N)),
  assert(cachedRelCounter(N1)),
  !.

incCachedRelCounter :-
  assert(cachedRelCounter(0)),
  !.

retractCacheRelations(N) :-
  retract(cachedRelation( _, N)),
  retractCacheRelations(N).

cacheRelation(Rel) :-
  Rel = rel(RName, _, _),  
  cachedRelation(RName, _),  
%  nl, write('===> Relation '), write(Rel), write(' is still in cache.\n'),
  !.

cacheRelation(Rel) :-
  Query = (count(Rel)),
  Rel = rel(RName, _, _),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  secondo(QueryAtom),
  incCachedRelCounter,
  cachedRelCounter(N),
  not(retractCacheRelations(N)),  
  assert(cachedRelation(RName, N)), 
%  nl, write('===> Cached relation '), write(Rel), write('.\n'),
  !.

cacheRelation(Rel) :-
  nl, write('ERROR in optimizer: cacheRelation('), 
  write(Rel), write(') failed.\n'),
  fail.

/*
---- invalidateRelationCache
----
The predicate should be called between different sql-queries. It invalidates 
the table of ~cachedRelation~s.

*/

invalidateRelationCache :-
 not(retractCacheRelations(_)).

/*
----     calculatePredicateCost(Tq, T0, Ttg, ResCard, Divisor, PredCost) 
----
This predicate gets the query time ~QueryTime~, empty query time ~T0~,
tuple generation cost ~Ttg~, result cardinality ~ResCard~ and a ~Divisor~ and 
unifies ~PedCost~ with the predicate cost for a predicate.

*/

calculatePredicateCost(Tq, T0, Ttg, ResultCard, Divisor, PredCost) :- 
  (Tq - T0 - ResultCard * Ttg) > 100,
  PredCost is max(((Tq - T0 - ResultCard * Ttg) / Divisor), 0.001),
  !.

calculatePredicateCost(_, _, _, _, _, 0.001) :- !. % return base predicate cost


/*
----
  % the first two clauses for sels/2 are needed for using hard coded 
  % selectivities. Since these cause problems with non-existing 
  % predicate cost, they should be omitted.
sels(Pred, Sel) :-
  sel(Pred, Sel),
  !.

sels(Pred, Sel) :-
  commute(Pred, Pred2),
  sel(Pred2, Sel),
  !.

----

*/

sels(Pred, Sel) :-
  databaseName(DB),
  storedSel(DB, Pred, Sel),
  !.

sels(Pred, Sel) :-
  commute(Pred, Pred2),
  databaseName(DB),
  storedSel(DB, Pred2, Sel),
  !.

/*

----	selectivity(P, Sel) :-
----

The selectivity of predicate ~P~ is ~Sel~.

If ~selectivity~ is called, it first tries to look up the 
selectivity via the predicate ~sel~. If no selectivity
is found, a Secondo query is issued, which determines the
selectivity. The retrieved selectitivity is then stored in
predicate ~storedSel~. This ensures that a selectivity has to
be retrieved only once.

*/

% Selectivities must not be 0

selectivity(pr(Pred, Rel, Rel), Sel) :-
  selectivity(pr(Pred, Rel), Sel), 
  !.

selectivity(P, Sel) :-
  simplePred(P, PSimple),
  sels(PSimple, Sel),
  !.

selectivity(pr(Pred, Rel1, Rel2), Sel) :-
  Rel1 = rel(BaseName1, A1, S1),
  sampleNameJ(BaseName1, SampleName1),
  cacheRelation(rel(SampleName1, A1, S1)),
  card(SampleName1, SampleCard1),
  Rel2 = rel(BaseName2, A2, S2),
  sampleNameJ(BaseName2, SampleName2),
  cacheRelation(rel(SampleName2, A2, S2)),
  card(SampleName2, SampleCard2),
  cardQuery(Pred, Rel1, Rel2, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  %write('selectivity query : '), write(QueryAtom),
  get_time(Time1),
  secondo(QueryAtom, [int, ResCard]),
  get_time(Time2),
  Time is Time2 - Time1,
  convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  Tq is Minute*60000 + Sec*1000 + MilliSec,
%  write('Elapsed Time: '), write(Tq), write(' ms'), nl, 
  sampleRuntimesJ(Rel1, Rel2, T0, T100, Ttg),
  TotalCard is (SampleCard1 * SampleCard2),
%  getPredCostDivisor(Pred, ResCard, ProdSize, Divisor),
  Divisor is TotalCard,                % comment out and uncomment previous line to use BBox-Modification
  calculatePredicateCost(Tq, T0, Ttg, ResCard, Divisor, PredCost),
  Sel is max(ResCard,1) / TotalCard,   % must not be 0
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  nl, write('Cost evaluation for join predicate '), write(PSimple), nl,
%  write('  Tq='), write(Tq), nl,
%  write('  T0='), write(T0), nl,
%  write('  Ttg='), write(Ttg), nl,
%  write('  ResCard='), write(ResCard), nl,
%  write('  ProdCard='), write(TotalCard), nl,
%  write('  Divisor='), write(Divisor), nl,
  write('Predicate Cost: '), write(PredCost),write(' ms'), nl,
  write('Selectivity : '), write(Sel), nl,
  databaseName(DB),
  assert(storedPET(DB, PSimple, PredCost, T0, T100, Tq, Ttg, ResCard, TotalCard)),
  assert(storedSel(DB, PSimple, Sel)),
  !.

selectivity(pr(Pred, Rel), Sel) :-
  Rel = rel(BaseName, A, S),
  sampleNameS(BaseName, SampleName),
  cacheRelation(rel(SampleName, A, S)),
  card(SampleName, SampleCard),
  cardQuery(Pred, Rel, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  %write('selectivity query : '), 
  %write(QueryAtom),
  get_time(Time1),
  secondo(QueryAtom, [int, ResCard]),
  get_time(Time2),
  Time is Time2 - Time1,
  convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  Tq is Minute *60000 + Sec*1000 + MilliSec,
%  write('Elapsed Time: '), write(Tq), write(' ms'), nl,
  sampleRuntimesS(Rel, T0),
%  getPredCostDivisor(Pred, ResCard, SampleCard, Divisor),
  Divisor is (SampleCard),                % comment out and uncomment previous line to use BBox-Modification
  calculatePredicateCost(Tq, T0, 0, ResCard, Divisor, PredCost),
  Sel is max(ResCard,1)/ SampleCard,	  % must not be 0
  simplePred(pr(Pred, Rel), PSimple),
  nl, write('Cost evaluation for selection predicate '), write(PSimple), nl,
%  write('  Tq='), write(Tq), nl,
%  write('  ResCard='), write(ResCard), nl,
%  write('  SampleCard='), write(SampleCard), nl,
%  write('  Divisor='), write(Divisor), nl,
  write('Predicate Cost: '), write(PredCost), write(' ms'), nl,
  write('Selectivity : '), write(Sel), nl,
  databaseName(DB),
  assert(storedPET(DB, PSimple, PredCost, T0, *, Tq, *, ResCard, SampleCard)),
  assert(storedSel(DB, PSimple, Sel)),
  !.


/*  
  *Deprecated selectivity clauses using dynamicCardQueries*  
  New calculation of predicate costs not implemented herein!

----

selectivity(pr(Pred, Rel1, Rel2), Sel) :-
  Rel1 = rel(BaseName1, _, _),
  card(BaseName1, Card1),
  SampleCard1 is min(Card1, max(500, Card1 * 0.00001)),
  Rel2 = rel(BaseName2, _, _),
  card(BaseName2, Card2),
  SampleCard2 is min(Card2, max(500, Card2 * 0.00001)),
  dynamicCardQuery(Pred, Rel1, Rel2, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  %write('selectivity query : '),
  %write(QueryAtom),
  get_time(Time1),
  secondo(QueryAtom, [int, ResCard]),
  get_time(Time2),
  Time is Time2 - Time1,
  convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  Tq is Minute *60000 + Sec*1000 + MilliSec,
  write('Elapsed Time: '), write(Tq), write(' ms'), nl, 
%  getPredCostDivisor(Pred, ResCard, (SampleCard1 * SampleCard2), Divisor),
  Divisor is (SampleCard1 * SampleCard2),                % comment out and uncomment previous line to use BBox-Modification
  PredCost is max(Tq,1) / Divisor, 
  Sel is max(ResCard,1) / (SampleCard1 * SampleCard2),	 % must not be 0
  write('Selectivity : '),
  write(Sel),
  nl,
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  databaseName(DB),
  assert(storedPET(DB, PSimple, PredCost, _, _, _, _, _, _)),
  assert(storedSel(DB, PSimple, Sel)),
  nl, write('WARNING: selectivity(pr(Pred, Rel1, Rel2), Sel): deprecated clause1 used!'), nl,
  !.

selectivity(pr(Pred, Rel), Sel) :-
  Rel = rel(BaseName, _, _),
  card(BaseName, Card),
  SampleCard is min(Card, max(2000, Card * 0.00001)),
  dynamicCardQuery(Pred, Rel, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  %write('selectivity query : '), write(QueryAtom),
  get_time(Time1),
  secondo(QueryAtom, [int, ResCard]),
  get_time(Time2),
  Time is Time2 - Time1,
  convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  Tq is Minute *60000 + Sec*1000 + MilliSec,
  write('Elapsed Time: '), write(Tq),write(' ms'), nl,
%  getPredCostDivisor(Pred, ResCard, SampleCard, Divisor),
  Divisor is SampleCard,                % comment out and uncomment previous line to use BBox-Modification
  PredCost is max(Tq,1) / Divisor,
  Sel is max(ResCard,1)/ SampleCard,		% must not be 0
  write('Selectivity : '),
  write(Sel),
  nl,
  simplePred(pr(Pred, Rel), PSimple),
  databaseName(DB),
  assert(storedPET(DB, PSimple, PredCost, _, _, _, _, _, _)),
  assert(storedSel(DB, PSimple, Sel)),
  nl, write('WARNING: selectivity(pr(Pred, Rel1, Rel2), Sel): deprecated clause2 used!'), nl,
  !.
----

*/

selectivity(P, _) :- write('Error in optimizer: cannot find selectivity for '),
  simplePred(P, PSimple), write(PSimple), nl, fail.

/*

The selectivities retrieved via Secondo queries can be loaded
(by calling ~readStoredSels~) and stored (by calling
~writeStoredSels~).

*/

readStoredSels :-
  retractall(storedSel(_, _, _)),
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


/*

The following predicates are used to write/read selectivity and predicate cost 
data to/from disk and to list the data.

*/

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

showStoredSel :- 
  storedSel(DB, X, Y),
  write(DB), write('.'), write(X), write(':\t'),
  write(Y), nl.

showStoredSels :-
  write('Stored selectivities:\n'),
  findall(_, showStoredSel, _).
 
:-
  dynamic(storedSel/3),
  at_halt(writeStoredSels),
  readStoredSels.

readStoredPETs :-
  retractall(storedPET(_, _, _, _, _, _, _, _, _)),
  [storedPETs].  

writeStoredPETs :-
  open('storedPETs.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredPET(FD), _),
  close(FD).

writeStoredPET(Stream) :-  
  storedPET(DB, P, CostP, T0, T100, Tq, Ttg, ResCard, ProdCard),
  replaceCharList(P, PReplaced),
  write(Stream, storedPET(DB, PReplaced, CostP, T0, T100, Tq, Ttg, ResCard, ProdCard)),
  write(Stream, '.\n').

:-
  dynamic(storedPET/9),
  at_halt(writeStoredPETs),
  readStoredPETs.

writePETs :-
  write('\nStored predicate costs:\n'),
  findall(_, writePET, _).

writePET :-
  storedPET(DB, P, PC, T0, T100, Tq, Ttg, ResCard, TotalCard),
  replaceCharList(P, PReplaced),
  write(DB), write('.'),
  write(PReplaced), write(', \tCost: '), write(PC), nl,
  write('\tT0='), write(T0),
  write(', T100='), write(T100),
  write(', Tq='), write(Tq),
  write(', Ttg='), write(Ttg),
  write(', ResCard='), write(ResCard),
  write(', TotalCard='), write(TotalCard),
  nl.

writePETsShort :-
  write('\nSttored predicate costs:\n'),
  write('Cost [ms] \t\t Predicate\n'),
  findall(_, writePETshort, _).

writePETshort :-
  storedPET(DB, P, PC, _, _, _, _, _, _),
  replaceCharList(P, PReplaced),
  write(DB), write('.'), write(PC), write('\t\t'), write(PReplaced), nl.


/*
----    storedSampleRuntimes(DB, R1, R2, T0, T100, Ttg, Mode)
----
This dynamic predicate is used to store reference times for joins on relations ~R1~ and ~R2~
in database ~DB~.
~T0~ is the time used by ~query R1 feed R1 feed symmjoin[FALSE] count~ resp. ~R1 feed 
filter[FALSE] count~, while ~Ttg~ is the time used to construct a single result tuple for a 
join between ~R1~ and ~R2~. ~T100~ is the time used for ~query R1 feed R2 feed 
symmjoin[TRUE] head[100] count~ resp. ~query R1 feed filter[TRUE] head[100] count~.
~Mode~ is used to tell selection-runtimes (s) from join-runtimes (j)

There are predicates to write/read reference times to/from disk and to show it to the user.

*/

readStoredSampleRuntimes :-
  retractall(storedSampleRuntimes(_, _, _, _, _, _, _)),
  [storedSampleRuntimes].  

writeStoredSampleRuntimes :-
  open('storedSampleRuntimes.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredSampleRuntimes(FD), _),
  close(FD).

writeStoredSampleRuntimes(Stream) :-  
  storedSampleRuntimes(DB, R1, R2, T0, T100, Ttg, Mode),
  write(Stream, storedSampleRuntimes(DB, R1, R2, T0, T100, Ttg, Mode)),
  write(Stream, '.\n').

showStoredSampleRuntimes :-
  write('\nStored SampleRuntimes:\n'),
  findall(_, showStoredSampleRuntime, _).

showStoredSampleRuntime :-
  storedSampleRuntimes(DB, R1, _, T0, _, _, s),
  write(DB), write('.'), write('('), write(R1),
  write('):  \tTO='), write(T0), write('ms\n').

showStoredSampleRuntime :-
  storedSampleRuntimes(DB, R1, R2, T0, T100, Ttg, j),
  write(DB), write('.'),
  write('('), write(R1),
  write('  x  '), write(R2),
  write('):  \tTO='), write(T0), write('ms'),
  write(', T100='), write(T100), write('ms'),
  write(', Ttg='), write(Ttg), write('ms\n').

:-
  dynamic(storedSampleRuntimes/7),
  at_halt(writeStoredSampleRuntimes),
  readStoredSampleRuntimes.

/*
---- sampleRuntimesJ(R1, R2, T0, T100, Ttg)
----

This predicate uses three queries on samples to calculate the query 
time ~T0~ for an empty join, the time ~T100~ for a query generating 
100 result tuples, the query time  and the per tuple result generation 
time ~Ttg~ on Relations ~R1~ and ~R2~.

*/

returnTtg(T0, T100, Ttg) :-
%  T100 > T0,
  Ttg is max((T100 - T0) / 100,0.0035),
  !.
%returnTtg( _, _, 0.0035) :- !. % return base cost for tuple generation

sampleRuntimesJ(R1, R2, T0, T100, Ttg) :-
  R1 = rel(BaseName1, _, _),
  sampleNameJ(BaseName1, SampleName1),
  R2 = rel(BaseName2, _, _),
  sampleNameJ(BaseName2, SampleName2),
  databaseName(DB),
  storedSampleRuntimes(DB, SampleName1, SampleName2, T0, T100, Ttg, j),
  !.

sampleRuntimesJ(R1, R2, T0, T100, Ttg) :-
  R1 = rel(BaseName1, _, _),
  sampleNameJ(BaseName1, SampleName1),
  R2 = rel(BaseName2, _, _),
  sampleNameJ(BaseName2, SampleName2),
  databaseName(DB),
  storedSampleRuntimes(DB, SampleName2, SampleName1, T0, T100, Ttg, j),
  !.

sampleRuntimesJ(R1, R2, T0, T100, Ttg) :-
  sampleJ(R1, R1S),
  sampleJ(R2, R2S),
  possiblyRename(R1S, R1Q),
  possiblyRename(R2S, R2Q),
  plan_to_atom(R1Q, R1A),
  plan_to_atom(R2Q, R2A),    
  atom_concat('query ', R1A, Q1),
  atom_concat(Q1, R2A, Q2),
  atom_concat(Q2, ' symmjoin[FALSE] count', T0Query),
  atom_concat(Q2, ' symmjoin[TRUE] head[100] count', T100Query),
  % run a query to get the base runtime T0 for a join
%  nl, write('sampleRuntimesJ/5 T0-Query: '), write(T0Query), nl,
  get_time(TimeA1),
  secondo(T0Query, [int, _]),
  get_time(TimeA2),
  TimeA is TimeA2 - TimeA1,
  convert_time(TimeA, _, _, _, _, MinuteA, SecA, MilliSecA),
  T0 is MinuteA *60000 + SecA*1000 + MilliSecA,
  % run a query to estimate the tuple generation time Ttg
%  nl, write('sampleRuntimesJ/5 T100-Query: '), write(T100Query), nl,
  get_time(TimeB1),
  secondo(T100Query, [int, _]),
  get_time(TimeB2),
  TimeB is TimeB2 - TimeB1,
  convert_time(TimeB, _, _, _, _, MinuteB, SecB, MilliSecB),
  T100 is MinuteB *60000 + SecB*1000 + MilliSecB,
  returnTtg(T0, T100, Ttg),
  R1 = rel(BaseName1, _, _),
  sampleNameJ(BaseName1, SampleName1),
  R2 = rel(BaseName2, _, _),
  sampleNameJ(BaseName2, SampleName2),
  databaseName(DB),
  assert(storedSampleRuntimes(DB, SampleName1, SampleName2, T0, T100, Ttg, j)),
  !.

sampleRuntimesJ(R1, R2, _, _, _, _) :-
  nl, write('ERROR in optimizer: sampleRuntimesJ('), write(R1), write(', '), write(R2), write(', _, _, _, _) failed.\n'),
  fail.

/*
---- sampleRuntimesS(Rel, T0, T100, Ttg)
----
This predicate uses three queries on samples to calculate the query 
time ~T0~ for an empty selection and the per tuple result generation time ~Ttg~ 
on Relation ~Rel~.

*/

sampleRuntimesS(Rel, T0) :-
  Rel = rel(BaseName, _, _),
  sampleNameS(BaseName, SampleName),
  databaseName(DB),
  storedSampleRuntimes(DB, SampleName, _, T0, _, _, s),
  !.

sampleRuntimesS(R1, T0) :-
  sampleS(R1, R1S),
  possiblyRename(R1S, R1Q),
  plan_to_atom(R1Q, R1A),
  atom_concat('query ', R1A, Q2),
  atom_concat(Q2, ' filter[FALSE] count', T0Query),
  % run a query to get the base runtime T0 for a selection
%  nl, write('sampleRuntimesS/4 T0-Query: '), write(T0Query), nl,
  get_time(TimeA1),
  secondo(T0Query, [int, _]),
  get_time(TimeA2),
  TimeA is TimeA2 - TimeA1,
  convert_time(TimeA, _, _, _, _, MinuteA, SecA, MilliSecA),
  T0 is MinuteA *60000 + SecA*1000 + MilliSecA,
  R1 = rel(BaseName1, _, _),
  sampleNameS(BaseName1, SampleName1),
  databaseName(DB),
  assert(storedSampleRuntimes(DB, SampleName1, *, T0, *, *, s)),
  !.

sampleRuntimesS(R, _) :-
  nl, write('ERROR in optimizer: sampleRuntimesS('), write(R), write(', _) failed.\n'),
  fail.

/*
----  ~predicateCost(Pred, PredCost)~
----
unifies ~PredCost~ with the cost for evaluating the  predicate ~Pred~ once. 
Predicate cost data is generated once per predicate, and will be stored on 
disk between sessions in a table  
 ~storedPET(DB, Pred, PredCost, T0, T100, Tq, ResCard, TotalCard)~ 
Additional information is stored to trace the calculation
of ~RedCost~: ~T0~ is the time used by an empty query, ~T100~ is the times consumend
to generate 100 tuples without evaluating ~Pred~, ~Tq~ is a time consumed by evaluating
~Pred~ on sample relation(s), ~ResCard~ is the result cardinality of the sample query,
and ~TotalCard~ is the total number of evaluations of ~Pred~ within the sample query.
 
*/

predicateCost(Pred, PredCost) :-
  simplePred(Pred, SPred),
  databaseName(DB),
  storedPET(DB, SPred, PredCost, _, _, _, _, _, _),
  !.

predicateCost(Pred, PredCost) :-
  simplePred(Pred, SPred),
  commute(SPred, CPred),
  databaseName(DB),
  storedPET(DB, CPred, PredCost, _, _, _, _, _, _),
  !.

predicateCost(Pred, _) :-
  nl, write('ERROR in optimizer: predicateCost('), 
  write(Pred), write(') failed.'), nl,
  fail.

/*
1.5 Examples

Example 22:

*/
example22 :- optimize(
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
 ~showDatabase~
 This predicate will inquire statistical data from the opened Secondo database
 and print it on the screen.

*/

showSingleRelation :-
  card(Rel, Card),
  tuplesize(Rel, Size),
  write('\tRelation: '), write(Rel), nl,
  findall(_, showAllAttributes(Rel), _),
  write('\t\tCardinality:   '), write(Card), nl,
  write('\t\tAvg TupleSize: '), write(Size), nl.

showSingleAttribute(Rel,Attr) :-
  databaseName(DB),
  storedAttrSize(DB, Rel, Attr, Type, CoreTupleSize, InFlobSize),
  write('\t\t'), write(Attr), write('\t'), 
  write(Type), write('\t'),
  write(CoreTupleSize), write('\t'),
  write(InFlobSize), nl. 

showAllAttributes(Rel) :-
  write('\t\tAttr\t Type\t CoreSz\t IFlobSz\t TupleSz\n'),
  findall(_, showSingleAttribute(Rel, _), _).

showDatabase :-
  write('Database schema:\n'),
  findall(_, showSingleRelation, _).

  
