/*
1 Information about selectivity of predicates, cardinality of relations, etc.

[File ~statistics.pl~]

1.1  Hard-Coded Cardinalities of Relations

*/

card(staedte, 58).
card(plz, 41267).
card(ten, 10).
card(thousand, 1000).

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

possiblyRename(Rel, Renamed) :-
  Rel = rel(_, *, _),
  !,
  Renamed = feed(Rel).

possiblyRename(Rel, Renamed) :-
  Rel = rel(_, Name, _),
  Renamed = rename(feed(Rel), Name).

cardQuery(Pred, Rel, Query) :-
  possiblyRename(Rel, RelQuery),
  Query = count(filter(RelQuery, Pred)).

cardQuery(Pred, Rel1, Rel2, Query) :-
  possiblyRename(Rel1, Rel1Query),
  possiblyRename(Rel2, Rel2Query),
  Query = count(filter(product(Rel1Query, Rel2Query), Pred)).

:- dynamic(storedSel/2).

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
  cardQuery(Pred, Rel1, Rel2, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  % write('selectivity query : '),
  % write(QueryAtom),
  secondo(QueryAtom, [int, ResCard]),
  Rel1 = rel(BaseName1, _, _),
  card(BaseName1, BaseCard1),
  Rel2 = rel(BaseName2, _, _),
  card(BaseName2, BaseCard2),
  Sel is ResCard / (BaseCard1 * BaseCard2),
  % write('selectivity : '),
  % write(Sel),
  nl,
  simplePred(pr(Pred, Rel1, Rel2), PSimple),
  assert(storedSel(PSimple, Sel)),
  !.

selectivity(pr(Pred, Rel), Sel) :-
  cardQuery(Pred, Rel, Query),
  plan_to_atom(Query, QueryAtom1),
  atom_concat('query ', QueryAtom1, QueryAtom),
  % write('selectivity query : '),
  % write(QueryAtom),
  secondo(QueryAtom, [int, ResCard]),
  Rel = rel(BaseName, _, _),
  card(BaseName, BaseCard),
  Sel is ResCard / BaseCard,
  % write('selectivity : '),
  % write(Sel),
  nl,
  simplePred(pr(Pred, Rel), PSimple),
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

writeStoredSels :-
  open('storedSels.pl', write, FD),
  write(FD, '/* Automatically generated file, do not edit by hand. */\n'),
  findall(_, writeStoredSel(FD), _),
  close(FD).

writeStoredSel(Stream) :-
  storedSel(X, Y),
  write(Stream, storedSel(X, Y)),
  write(Stream, '.\n').

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
