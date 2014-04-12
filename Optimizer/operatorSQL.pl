/*
Translation of SQL Queries into Moving Object Operators

*/

:- op(970, fx, get).
:- op(960, xfx, from).
:- op(950, xfx, where).
:- op(950, fx, select).
:- op(980, xfx, =>>).
:- op(800, xfx, atinstant).
:- op(800, xfx, atperiods).
:- op(800, xfx, at).
:- op(970, fx, exists).
:- op(800, xfx, present).
:- op(800, xfx, passes).



% Join select Project =>> Term :-
%   Join =>> r(Name, Term1, Type1),
%   s(Name, Term1, Type1, Project) =>> Term.




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
  member(XType, [point, region]),
  member(YType, [point, line, region]).

_ from [X, Y] where X:v = Y:v =>> r(X, X at Y, moving(XType)) :-
  type(X, moving(XType)),
  type(Y, range(XType)),
  member(XType, [int, real, bool, string]).



exists (get _ from [X, Y] where X:t = Y:t) =>> X present Y :-
  type(X, moving(_)),
  type(Y, YType),
  member(YType, [instant, periods]).

exists (get _ from [X, Y] where X:pos = Y:pos) =>> X passes Y :-
  type(X, moving(XType)),
  type(Y, YType),
  member(XType, [point, region]),
  member(YType, [point, line, region]).

exists (get _ from [X, Y] where X:v = Y:v) =>> X passes Y :-
  type(X, moving(XType)),
  type(Y, range(XType)),
  member(XType, [int, real, bool, string]).



/*
Translation of Projections

*/

s(Name, Term, intime(_), Name) =>> Term.

s(Name, Term, intime(_), Name:t) =>> inst(Term).

s(Name, Term, intime(T), Name:pos) =>> val(Term) :-
  member(T, [point, region]).

s(Name, Term, intime(T), Name:v) =>> val(Term) :-
  member(T, [int, real, bool, string]).


s(Name, Term, moving(_), Name) =>> Term.

s(Name, Term, moving(_), Name:t) =>> deftime(Term).

s(Name, Term, moving(point), Name:pos) =>> trajectory(Term).

s(Name, Term, moving(region), Name:pos) =>> traversed(Term).

s(Name, Term, moving(T), Name:v) =>> rangevalues(Term) :-
  member(T, [int, real, bool, string]).





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















