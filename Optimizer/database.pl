/*
1 Database Dependent Information

[File ~database.pl~]


1.1 Relation Schemas

*/
relation(staedte, [sname, bev, plz, vorwahl, kennzeichen]).
relation(plz, [plz, ort]).
relation(ten, [no]).
relation(thousand, [no]).
relation(orte, [kennzeichen, ort, vorwahl, bevt]).

relation(distrikte, [nr, dname, flaeche, bev, bev_maennlich, gebiet, bev20]).
relation(laender, [lname, gebiet]).
relation(staedte2, [sname, ort, bev, plz, vorwahl, kennzeichen]).


/*
1.2 Spelling of Relation and Attribute Names

*/
spelling(staedte:plz, pLZ).
spelling(staedte:sname, sName).
spelling(plz, lc(plz)).
spelling(plz:plz, pLZ).
spelling(ten, lc(ten)).  
spelling(ten:no, lc(no)).
spelling(thousand, lc(thousand)).
spelling(thousand:no, lc(no)).
spelling(orte:bevt, bevT).

spelling(distrikte:dname, dName).
spelling(laender:lname, lName).
spelling(staedte2:sname, sName).
spelling(staedte2:plz, pLZ).


/*

1.3  Cardinalities of Relations

*/
card(staedte, 58).
card(staedte_sample, 58).
card(plz, 41267).
card(plz_sample, 428).
card(ten, 10).
card(ten_sample, 10).
card(thousand, 1000).
card(thousand_sample, 89).
card(orte, 506).
card(orte_sample, 100).

card(distrikte, 569).
card(laender, 9).
card(staedte2, 59).

/*
1.4 Indexes

*/
hasIndex(Rel, attr(_:A, _, _), IndexName) :-
  hasIndex(Rel, attr(A, _, _), IndexName).

hasIndex(rel(Rel, _, _), attr(Attr, _, _), Index) :- index(Rel, Attr, _, Index).



index(plz, ort, btree, plz_Ort).
index(plz, pLZ, btree, plz_PLZ).

index(distrikte, gebiet, rtree, distrikte_Gebiet).


index(_, _, _, _) :- fail.












