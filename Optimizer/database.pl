/*
1 Database Dependent Information

[File ~database.pl~]

1.1 Rules about Commutativity of Predicates

*/

commute(X = Y, Y = X).
commute(X < Y, Y > X).
commute(X <= Y, Y >= X).
commute(X > Y, Y < X).
commute(X >= Y, Y <= X).
commute(X # Y, Y # X).

/*
1.2 Relation Schemas

*/
relation(staedte, [sname, bev, plz, vorwahl, kennzeichen]).
relation(plz, [plz, ort]).
relation(ten, [no]).
relation(thousand, [no]).
relation(orte, [kennzeichen, ort, vorwahl, bevt]).

spelling(staedte:plz, pLZ).
spelling(staedte:sname, sName).
spelling(plz, lc(plz)).
spelling(plz:plz, pLZ).
spelling(ten, lc(ten)).
spelling(ten:no, lc(no)).
spelling(thousand, lc(thousand)).
spelling(thousand:no, lc(no)).
spelling(orte:bevt, bevT).

hasIndex(Rel, attr(_:A, _, _), IndexName) :-
  hasIndex(Rel, attr(A, _, _), IndexName).

hasIndex(rel(plz, _, _), attr(ort, _, _), plz_Ort).
hasIndex(rel(plz, _, _), attr(pLZ, _, _), plz_PLZ).











