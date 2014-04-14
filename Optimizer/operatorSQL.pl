/*
Translation of SQL Queries into Moving Object Operators

*/

:- op(970, fx, get).
:- op(980, xfx, =>>).
:- op(970, fx, exists).


	/*
:- op(960, xfx, from).
:- op(950, xfx, where).
:- op(950, fx, select).
:- op(930, xfy,  as).
:- op(800, xfx, atinstant).
:- op(800, xfx, atperiods).
:- op(800, xfx, at).

:- op(800, xfx, present).
:- op(800, xfx, passes).
	*/




get Project from Join =>> Term :-
  _ from Join =>> r(Name, Term1, Type1),
  s(Name, Term1, Type1, Project) =>> Term.



/*
Translation of Joins

For the moment, we allow to consider from the result of the join only the projection to the first argument, hence projections can later be applied only to that part of the join.

*/

_ from [X, Y] where X:t = Y:t =>> r(X, X atinstant Y, intime(T)) :-
  type(X, moving(T)),
  type(Y, instant).

_ from [X, Y] where X:t = Y:t =>> r(X, X atperiods Y, moving(T)) :-
  type(X, moving(T)),
  type(Y, periods).

_ from [X, Y] where X:pos = Y:pos =>> r(X, X at Y, moving(XType)) :-
  type(X, moving(XType)),
  type(Y, YType),
  memberchk(XType, [point, region]),
  memberchk(YType, [point, line, region]).

_ from [X, Y] where X:v = Y:v =>> r(X, X at Y, moving(XType)) :-
  type(X, moving(XType)),
  type(Y, range(XType)),
  memberchk(XType, [int, real, bool, string]).



_ from X =>> r(X, X, XType) :- 
  type(X, XType).



exists (get _ from [X, Y] where X:t = Y:t) =>> X present Y :-
  type(X, moving(_)),
  type(Y, YType),
  memberchk(YType, [instant, periods]).

exists (get _ from [X, Y] where X:pos = Y:pos) =>> X passes Y :-
  type(X, moving(XType)),
  type(Y, YType),
  memberchk(XType, [point, region]),
  memberchk(YType, [point, line, region]).

exists (get _ from [X, Y] where X:v = Y:v) =>> X passes Y :-
  type(X, moving(XType)),
  type(Y, range(XType)),
  memberchk(XType, [int, real, bool, string]).



/*
Translation of Projections

*/

s(_, Term, _, *) =>> Term.

s(Name, Term, _, Name) =>> Term.



s(Name, Term, intime(_), Name:t) =>> inst(Term).

s(Name, Term, intime(T), Name:pos) =>> val(Term) :-
  memberchk(T, [point, region]).

s(Name, Term, intime(T), Name:v) =>> val(Term) :-
  memberchk(T, [int, real, bool, string]).



s(Name, Term, moving(_), Name:t) =>> deftime(Term).

s(Name, Term, moving(point), Name:pos) =>> trajectory(Term).

s(Name, Term, moving(region), Name:pos) =>> traversed(Term).

s(Name, Term, moving(T), Name:v) =>> rangevalues(Term) :-
  memberchk(T, [int, real, bool, string]).





type(mi, moving(int)).
type(mr, moving(real)).
type(mb, moving(bool)).
type(ms, moving(string)).

type(mp, moving(point)).
type(mre, moving(region)).

type(i, instant).
type(pe, periods).

type(p, point).
type(l, line).
type(re, region).

type(ira, range(int)).



type(train7, moving(point)).
type(mehringdamm, point).
type(tiergarten, region).
type(six30, instant).
type(trip, moving(point)).


/*
Rewriting SQL queries.

Example:

select val(trip atinstant six30) as atsix30 
from trains 
where trip passes mehringdamm

can be written as

select (get trip:pos from [trip, six30] where trip:t = six30:t) as atsix30
from trains
where exists(get trip from [trip, mehringdamm] where trip:pos = mehringdamm:pos)


*/

moSQL(select Attrs from Rels where Preds, 
  	select Attrs4 from Rels where Preds4) :-
  makeList(Attrs, Attrs2),
  moAttrs(Attrs2, Attrs3),
  makeNoList(Attrs3, Attrs4),
  makeList(Preds, Preds2),
  moPreds(Preds2, Preds3),
  makeNoList(Preds3, Preds4),
  !.

moSQL(X, X).



moAttrs([], []).

moAttrs([Attr | Attrs], [Attr2 | Attrs2]) :-
  moAttr(Attr, Attr2),
  moAttrs(Attrs, Attrs2).

moAttr(Term as Name, Term2 as Name) :-
  moTerm(Term, Term2),
  !.

moAttr(X, X).



moTerm(X, Y) :-
  X =>> Y,		% Translation
  !.

moTerm(X, X).



moPreds([], []).

moPreds([Pred | Preds], [Pred2 | Preds2]) :-
  moPred(Pred, Pred2),
  moPreds(Preds, Preds2).


moPred(exists(X), Y) :-
  exists(X) =>> Y,	% Translation
  !.

moPred(X, X).




% makeList defined elsewhere

% makeList(L, L) :- is_list(L).

% makeList(L, [L]) :- not(is_list(L)).


makeNoList([X], X) :- 
  !.

makeNoList(L, L).











