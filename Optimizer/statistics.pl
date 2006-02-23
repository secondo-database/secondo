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

cardQuery(Pred, Rel, Query) :-
  sampleS(Rel, RelS),
  possiblyRename(RelS, RelQuery),
  Query = count(filter(RelQuery, Pred)).

cardQuery(Pred, Rel1, Rel2, Query) :-
  sampleJ(Rel1, Rel1S),
  sampleJ(Rel2, Rel2S),
  possiblyRename(Rel1S, Rel1Query),
  possiblyRename(Rel2S, Rel2Query),
  transformPred(Pred, t, 1, Pred2),
  Query = count(loopsel(Rel1Query, fun([param(t, tuple)], filter(Rel2Query, Pred2)))).

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


%  Query = count(filter(product(Rel1Query, Rel2Query), Pred)).

dynamicCardQuery(Pred, Rel, Query) :-
  dynamicPossiblyRenameS(Rel, RelQuery),
  Query = count(filter(RelQuery, Pred)).

dynamicCardQuery(Pred, Rel1, Rel2, Query) :-
  dynamicPossiblyRenameJ(Rel1, Rel1Query),
  dynamicPossiblyRenameJ(Rel2, Rel2Query),
  %Query = count(filter(product(Rel1Query, Rel2Query), Pred)).
  transformPred(Pred, t, 1, Pred2),
  Query = count(loopsel(Rel1Query, fun([param(t, tuple)], filter(Rel2Query, Pred2)))).

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
  storedSel(Pred, Sel),
  !.

sels(Pred, Sel) :-
  commute(Pred, Pred2),
  storedSel(Pred2, Sel).

/*

----	selectivity(P, Sel) :-
----

The selectivity of predicate ~P~ is ~Sel~.

If ~selectivity~ is called, it first tries to look up
the selectivity via the predicate ~sel~. If no selectivity
is found, a Secondo query is issued, which determines the
selectivity. The retrieved selectitivity is then stored in
predicate ~storedSel~. This ensures that a selectivity has to
be retrieved only once.

*/

% Selectivities must not be 0

selectivity(pr(Pred, Rel, Rel), Sel) :-
  selectivity(pr(Pred, Rel), Sel), !.

selectivity(P, Sel) :-
  simplePred(P, PSimple),
  sels(PSimple, Sel),
  !.

selectivity(pr(Pred, Rel1, Rel2), Sel) :-
  set_dynamic_sample(off),
  Rel1 = rel(BaseName1, _, _),
  sampleNameJ(BaseName1, SampleName1),
  card(SampleName1, SampleCard1),
  Rel2 = rel(BaseName2, _, _),
  sampleNameJ(BaseName2, SampleName2),
  card(SampleName2, SampleCard2),
  cardQuery(Pred, Rel1, Rel2, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  %write('selectivity query : '),
  %write(QueryAtom),
  get_time(Time1),
  secondo(QueryAtom, [int, ResCard]),
  get_time(Time2),
  Time is Time2 - Time1,
  convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  MSs is Minute *60000 + Sec*1000 + MilliSec,
  write('Elapsed Time: '),
  write(MSs),
  write(' ms'),nl, 
  MSsRes is MSs / (SampleCard1 * SampleCard2), 
  Sel is (ResCard + 1) / (SampleCard1 * SampleCard2),	% must not be 0
  write('Predicate Cost: '),
  write(MSsRes),
  write(' ms'),nl,
  write('Selectivity : '),
  write(Sel),
  nl,
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  assert(storedPET(PSimple, MSsRes)),
  assert(storedSel(PSimple, Sel)),
  !.

selectivity(pr(Pred, Rel), Sel) :-
  set_dynamic_sample(off),
  Rel = rel(BaseName, _, _),
  sampleNameS(BaseName, SampleName),
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
  MSs is Minute *60000 + Sec*1000 + MilliSec,
  write('Elapsed Time: '),
  write(MSs),
  write(' ms'),nl,
  MSsRes is MSs / SampleCard,
  Sel is (ResCard + 1)/ SampleCard,		% must not be 0
  write('Predicate Cost: '),
  write(MSsRes),
  write(' ms'),nl,
  write('Selectivity : '),
  write(Sel),
  nl,
  simplePred(pr(Pred, Rel), PSimple),
  assert(storedPET(PSimple, MSsRes)),
  assert(storedSel(PSimple, Sel)),
  !.

selectivity(pr(Pred, Rel1, Rel2), Sel) :-
  set_dynamic_sample(on),
  Rel1 = rel(BaseName1, _, _),
  card(BaseName1, Card1),
  sampleSizeJoin(JoinSize),
  SampleCard1 is min(Card1, max(JoinSize, Card1 * 0.00001)),
  Rel2 = rel(BaseName2, _, _),
  card(BaseName2, Card2),
  SampleCard2 is min(Card2, max(JoinSize, Card2 * 0.00001)),
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
  MSs is Minute *60000 + Sec*1000 + MilliSec,
  write('Elapsed Time: '),
  write(MSs),
  write(' ms'),nl,
  MSsRes is MSs / (SampleCard1 * SampleCard2),
  Sel is (ResCard + 1) / (SampleCard1 * SampleCard2),	% must not be 0
  write('Predicate Cost: '),
  write(MSsRes),
  write(' ms'),nl,
  write('Selectivity : '),
  write(Sel),
  nl,
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  assert(storedSel(PSimple, Sel)),
  !.

selectivity(pr(Pred, Rel), Sel) :-
  set_dynamic_sample(on),
  Rel = rel(BaseName, _, _),
  card(BaseName, Card),
  sampleSizeSelection(SelectionSize),
  SampleCard is min(Card, max(SelectionSize, Card * 0.00001)),
  dynamicCardQuery(Pred, Rel, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  %write('selectivity query : '),
  %write(QueryAtom),
  get_time(Time1),
  secondo(QueryAtom, [int, ResCard]),
  get_time(Time2),
  Time is Time2 - Time1,
  convert_time(Time, _, _, _, _, Minute, Sec, MilliSec),
  MSs is Minute *60000 + Sec*1000 + MilliSec,
  write('Elapsed Time: '),
  write(MSs),
  write(' ms'),nl,
  MSsRes is MSs / SampleCard,
  Sel is (ResCard + 1)/ SampleCard,		% must not be 0
  write('Predicate Cost: '),
  write(MSsRes),
  write(' ms'),nl,
  write('Selectivity : '),
  write(Sel),
  nl,
  simplePred(pr(Pred, Rel), PSimple),
  assert(storedSel(PSimple, Sel)),
  !.

selectivity(P, _) :- write('Error in optimizer: cannot find selectivity for '),
  simplePred(P, PSimple), write(PSimple), nl, 
  write('Call: selectivity('), write(P), write(',Sel)\n'),
  fail.

/*

The selectivities retrieved via Secondo queries can be loaded
(by calling ~readStoredSels~) and stored (by calling
~writeStoredSels~).

*/

readStoredSels :-
  retractall(storedSel(_, _)),
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
  storedSel(X, Y),
  replaceCharList(X, XReplaced),
  write(Stream, storedSel(XReplaced, Y)),
  write(Stream, '.\n').

:-
  dynamic(storedSel/2),
  at_halt(writeStoredSels),
  readStoredSels.

readStoredPETs :-
  retractall(storedPET(_, _)),
  [storedPETs].  

writeStoredPETs :-
  open('storedPETs.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredPET(FD), _),
  close(FD).

writeStoredPET(Stream) :-  
  storedPET(X, Y),
  replaceCharList(X, XReplaced),
  write(Stream, storedPET(XReplaced, Y)),
  write(Stream, '.\n').

:-
  dynamic(storedPET/2),
  at_halt(writeStoredPETs),
  readStoredPETs.

writePETs :-
  findall(_, writePET, _).

writePET :-
  storedPET(X, Y),
  replaceCharList(X, XReplaced),
  write('Predicate: '),
  write(XReplaced),
  write(', Cost: '),
  write(Y),
  write(' ms\n').

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
