/*
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

sample(rel(Rel, Var, Case), rel(Rel2, Var, Case)) :-
  atom_concat(Rel, '_sample', Rel2).

sampleName(Name, Sample) :-
  atom_concat(Name, '_sample', Sample).

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
  Renamed = sample(Rel, 100, 0.01).

dynamicPossiblyRename(Rel, Renamed) :-
  Rel = rel(_, Name, _),
  Renamed = rename(sample(Rel, 100, 0.01), Name).

cardQuery(Pred, Rel, Query) :-
  sample(Rel, RelS),
  possiblyRename(RelS, RelQuery),
  Query = count(filter(RelQuery, Pred)).

cardQuery(Pred, Rel1, Rel2, Query) :-
  sample(Rel1, Rel1S),
  sample(Rel2, Rel2S),
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
  dynamicPossiblyRename(Rel, RelQuery),
  Query = count(filter(RelQuery, Pred)).

dynamicCardQuery(Pred, Rel1, Rel2, Query) :-
  dynamicPossiblyRename(Rel1, Rel1Query),
  dynamicPossiblyRename(Rel2, Rel2Query),
  Query = count(filter(product(Rel1Query, Rel2Query), Pred)).

createSampleRelation(Rel) :-
  not(spelling(Rel, _)),
  upper(Rel, URel),
  sampleName(URel, Sample),
  concat_atom(['let ', Sample, ' = ', URel, 
    ' sample[100, 0.01] consume'], '', QueryAtom),
  secondo(QueryAtom), !.

createSampleRelation(Rel) :-
  spelling(Rel, Spelled),
  Spelled = lc(Spelled2),
  sampleName(Spelled2, Sample),
  concat_atom(['let ', Sample, ' = ', Spelled2, 
    ' sample[100, 0.01] consume'], '', QueryAtom),
  secondo(QueryAtom), !.

createSampleRelation(Rel) :-
  upper(Rel, URel),
  sampleName(URel, Sample),
  concat_atom(['let ', Sample, ' = ', URel, 
    ' sample[100, 0.01] consume'], '', QueryAtom),
  secondo(QueryAtom), !.

sels(Pred, Sel) :-
  sel(Pred, Sel),
  !.

sels(Pred, Sel) :-
  commute(Pred, Pred2),
  sel(Pred2, Sel),
  !.

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


selectivity(P, Sel) :-
  simplePred(P, PSimple),
  sels(PSimple, Sel),
  !.

selectivity(pr(Pred, Rel1, Rel2), Sel) :-
  Rel1 = rel(BaseName1, _, _),
  sampleName(BaseName1, SampleName1),
  card(SampleName1, SampleCard1),
  Rel2 = rel(BaseName2, _, _),
  sampleName(BaseName2, SampleName2),
  card(SampleName2, SampleCard2),
  cardQuery(Pred, Rel1, Rel2, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  %write('selectivity query : '),
  %write(QueryAtom),
  secondo(QueryAtom, [int, ResCard]),
  Sel is (ResCard + 1) / (SampleCard1 * SampleCard2),	% must not be 0
  write('selectivity : '),
  write(Sel),
  nl,
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  assert(storedSel(PSimple, Sel)),
  !.

  selectivity(pr(Pred, Rel1, Rel2), Sel) :-
  Rel1 = rel(BaseName1, _, _),
  sampleName(BaseName1, SampleName1),
  card(SampleName1, SampleCard1),
  Rel2 = rel(BaseName2, _, _),
  card(BaseName2, Card2),
  SampleCard2 is truncate(min(Card2, max(100, Card2 * 0.01))),
  sampleName(BaseName2, SampleName),
  assert(storedCard(SampleName, SampleCard2)),
  dynamicCardQuery(Pred, Rel1, Rel2, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  %write('selectivity query : '),
  %write(QueryAtom),
  secondo(QueryAtom, [int, ResCard]),
  Sel is (ResCard + 1) / (SampleCard1 * SampleCard2),	% must not be 0
  write('selectivity : '),
  write(Sel),
  nl,
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  createSampleRelation(BaseName2),
  assert(storedSel(PSimple, Sel)),
  !.

  selectivity(pr(Pred, Rel1, Rel2), Sel) :-
  Rel2 = rel(BaseName2, _, _),
  sampleName(BaseName2, SampleName2),
  card(SampleName2, SampleCard2),
  Rel1 = rel(BaseName1, _, _),
  card(BaseName1, Card1),
  SampleCard1 is truncate(min(Card1, max(100, Card1 * 0.01))),
  sampleName(BaseName1, SampleName),
  assert(storedCard(SampleName, SampleCard1)),
  dynamicCardQuery(Pred, Rel1, Rel2, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  %write('selectivity query : '),
  %write(QueryAtom),
  secondo(QueryAtom, [int, ResCard]),
  Sel is (ResCard + 1) / (SampleCard1 * SampleCard2),	% must not be 0
  write('selectivity : '),
  write(Sel),
  nl,
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  createSampleRelation(BaseName1),
  assert(storedSel(PSimple, Sel)),
  !. 

selectivity(pr(Pred, Rel), Sel) :-
  Rel = rel(BaseName, _, _),
  sampleName(BaseName, SampleName),
  card(SampleName, SampleCard),
  cardQuery(Pred, Rel, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  %write('selectivity query : '),
  %write(QueryAtom),
  secondo(QueryAtom, [int, ResCard]),
  Sel is (ResCard + 1)/ SampleCard,		% must not be 0
  write('selectivity : '),
  write(Sel),
  nl,
  simplePred(pr(Pred, Rel), PSimple),
  assert(storedSel(PSimple, Sel)),
  !.

selectivity(pr(Pred, Rel1, Rel2), Sel) :-
  Rel1 = rel(BaseName1, _, _),
  card(BaseName1, Card1),
  SampleCard1 is truncate(min(Card1, max(100, Card1 * 0.01))),
  sampleName(BaseName1, SampleName1),
  assert(storedCard(SampleName1, SampleCard1)),
  Rel2 = rel(BaseName2, _, _),
  card(BaseName2, Card2),
  SampleCard2 is truncate(min(Card2, max(100, Card2 * 0.01))),
  sampleName(BaseName2, SampleName2),
  assert(storedCard(SampleName2, SampleCard2)),
  dynamicCardQuery(Pred, Rel1, Rel2, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  %write('selectivity query : '),
  %write(QueryAtom),
  secondo(QueryAtom, [int, ResCard]),
  Sel is (ResCard + 1) / (SampleCard1 * SampleCard2),	% must not be 0
  write('selectivity : '),
  write(Sel),
  nl,
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  createSampleRelation(BaseName1),
  createSampleRelation(BaseName2),
  assert(storedSel(PSimple, Sel)),
  !.

selectivity(pr(Pred, Rel), Sel) :-
  Rel = rel(BaseName, _, _),
  card(BaseName, Card),
  SampleCard is truncate( min(Card, max(100, Card * 0.01))),
  sampleName(BaseName, SampleName),
  assert(storedCard(SampleName, SampleCard)),
  dynamicCardQuery(Pred, Rel, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  write('selectivity query : '),
  write(QueryAtom),
  secondo(QueryAtom, [int, ResCard]),
  Sel is (ResCard + 1)/ SampleCard,		% must not be 0
  write('selectivity : '),
  write(Sel),
  nl,
  simplePred(pr(Pred, Rel), PSimple),
  createSampleRelation(BaseName),
  assert(storedSel(PSimple, Sel)),
  !.

selectivity(P, _) :- write('Error in optimizer: cannot find selectivity for '),
  simplePred(P, PSimple), write(PSimple), nl, fail.

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
