/*
Translation of SQL Queries into Moving Object Operators

Parts overlapping with standard optimizer. Only needed when running operatorSQL independently.

*/



:- op(960, xfx, from).
:- op(950, xfx, where).
:- op(950, fx, select).
:- op(930, xfy,  as).
:- op(800, xfx, atinstant).
:- op(800, xfx, atperiods).
:- op(800, xfx, at).

:- op(800, xfx, present).
:- op(800, xfx, passes).




makeList(L, L) :- is_list(L).

makeList(L, [L]) :- not(is_list(L)).











