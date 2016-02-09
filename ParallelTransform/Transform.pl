/* 

Nur count

----
sequentiell Eingang
(pquery 
    (count Roads))

parallel Ergebnis
(query 
    (tie 
        (getValue 
            (dloop2 Roads "" 
                (fun 
                    (darray2elem1 DARRAY2ELEM) 
                    (count darray2elem1)))) 
        (fun 
            (first2 ELEMENT) 
            (second3 ELEMENT) 
            (+ first2 second3))))


[pquery,[count,Roads]]
[pquery,[tie,[getValue,[dloop2,Roads,"",[fun,[darray2elem1,DARRAY2ELEM],[count,darray2elem1]]]],[fun,[first2,ELEMENT],[second3,ELEMENT],[+,first2,second3]]]]
----

Count mit Filter

----
sequentiell Eingang
(pquery 
    (count 
        (filter 
            (feed Roads) 
            (fun 
                (streamelem1 STREAMELEM) 
                (contains 
                    (attr streamelem1 Type) 
                    "primary")))))
----
Prolog-Darstellung

----
[pquery,[count,[filter,[feed,roads],[fun,[streamelem1,sTREAMELEM],[contains,[attr,streamelem1,type],"primary"]]]]]
----

parallel Ergebnis

----
(query 
    (tie 
        (getValue 
            (dloop2 Roads "" 
                (fun 
                    (darray2elem1 DARRAY2ELEM) 
                    (count 
                        (filter 
                            (feed darray2elem1) 
                            (fun 
                                (streamelem2 STREAMELEM) 
                                (contains 
                                    (attr streamelem2 Type) 
                                    "primary"))))))) 
        (fun 
            (first3 ELEMENT) 
            (second4 ELEMENT) 
            (+ first3 second4))))
----

Prolog-Darstellung (Eingangs query darf keine grossengeschriebenen Elemente mehr enthalten)

----
[pquery,[count,[filter,[feed,Roads],[fun,[streamelem1,STREAMELEM],[contains,[attr,streamelem1,Type],"primary"]]]]] ##ENDE
[pquery,[tie,[getValue,[dloop2,Roads,"",[fun,[darray2elem1,DARRAY2ELEM],[count,[filter,[feed,darray2elem1],[fun,[streamelem2,STREAMELEM],[contains,[attr,streamelem2,Type],"primary"]]]]]]],[fun,[first3,ELEMENT],[second4,ELEMENT],[+,first3,second4]]]]
----

*/
query_to_parallel(QueryNL, ParallelNL) :-
    write('Query: '), write(QueryNL), nl,
    /* Relations aus Query ausschneiden */
    splitFromRelations(QueryNL, Relations),
    write('Relations: '), writeln(Relations),
    /* ersetzen  der Relations im Query durch elemXX */
    replaceRelationInQuery(Relations, elem, QueryNL, QueryRelsNewNL, 0),
    write('QueryRelsNew: '), write(QueryRelsNewNL), nl,
    /* Trenne Funktion bzw. Value-Expression von query-command */
    QueryRelsNewNL = [QueryCommand | [FunktionNL]],
    write('Funktion: '), write(FunktionNL), nl,
    /* TransformRegel aufrufen inkl. Pruefung auf Tie[]-Notwendigkeit */
    queryToParallelTieDecision(Relations, FunktionNL, ParallelNL),
    write('Ergebnis: '), write(ParallelNL), nl
       .

/*
Entscheidet anhand der zu startenden Funktion welche Fakten fuer die parallele Transformation benutzt werden 

*/
queryToParallelTieDecision(Relations, FunktionNL, Parallel) :-
    /* Fall mit Tie[] Operator, wenn Funktion ein count o. 
     sum an 1. Stelle enthaelt */
    flatten(FunktionNL, FunktionFlat),
    FunktionFlat = [Head | Tail],
    (Head == count; Head == sum),
    secondoPrologParallelQueryTie(Relations, FunktionNL, Parallel)
    .

queryToParallelTieDecision(Relations, FunktionNL, Parallel) :-
    /* Fall mit Tie[] Operator, wenn Funktion ein min o. 
     max an 1. Stelle enthaelt */
    flatten(FunktionNL, FunktionFlat),
    FunktionFlat = [Head | Tail],
    (Head == min; Head == max),
    (Head == min -> 
        secondoPrologParallelQueryMin(Relations, FunktionNL, Parallel)    
    ; Head == max ->
        secondoPrologParallelQueryMax(Relations, FunktionNL, Parallel)
    )
    .

queryToParallelTieDecision(Relations, FunktionNL, Parallel) :-
    /* Fall mit Tie[] Operator, wenn Funktion ein avg an 1. Stelle enthaelt. 
    Laeuft aktuell noch falsch, muss auf Summe(Summe(attribut)) dividiert durch
    Summe(count(attribut))
    */
    flatten(FunktionNL, FunktionFlat),
    FunktionFlat = [Head | Tail],
    Head == avg,
    secondoPrologParallelQueryAvg(Relations, FunktionNL, Parallel)
    .

queryToParallelTieDecision(Relations, FunktionNL, Parallel) :-
    /* Fall ohne Tie[] Operator */
    flatten(FunktionNL, FunktionFlat),
    FunktionFlat = [Head | Tail],
    Head \= count, Head \= avg, Head \= max, Head \= min, Head \= sum,
    secondoPrologParallelQuery(Relations, FunktionNL, Parallel)
    .

/* 
Fuegt gefundene Relationen in eine Liste 

*/
splitFromRelations(Query, Relations) :-
    writeln('Start Split'),
    flatten(Query, QueryList),
    writeln('Flatten'),
    findRelations(QueryList, [], Relations)
    .

/*
Relationen anhand drei bestimmter Regeln in Funktion identifizieren
   1. hinter feed befindet sich gekennzeichnete <Relation>
   2. Wenn bereits gefundene <Relation> und noch feed vorhanden dann wieder hinter feed
   3. Wenn keine <Relation> gefunden und kein feed dann ist letztes ELement eine <Relation> 


*/
findRelations([],_,_).


/* 
feed existiert 

*/
findRelations(QList, TmpRels, Relations) :-
    member(feed, QList),
    nth0(FeedIdx, QList, feed),
    TempIdx is FeedIdx+1,
    nth0(TempIdx, QList, Relation),
    atom_chars(Relation, RelChars),
    write('QList: '), writeln(QList),
    write('Rel Chars1: '), writeln(RelChars),
    (member('<', RelChars) ->  
        append(TmpRels,[Relation],NewTmpRels),
        selectchk(feed, QList, RestList),
        /* write('Treffer < 1: '), writeln(RestList),
        write('NewTmpRels 1: '), writeln(NewTmpRels),*/
        findRelations(RestList, NewTmpRels, Relations)
    ;     selectchk(feed, QList, RestList),
        /* write('kein Treffer < 1: '), writeln(RestList),
        write('NewTmpRels 1: '), writeln(TmpRels),*/
        findRelations(RestList, TmpRels, Relations)
    )
    .

 /* feed existiert nicht und TmpRels belegt */
findRelations(QList, TmpRels, Relations) :-
    \+member(feed, QList),
    \+length(TmpRels, 0),
    Relations = TmpRels
    .

 /* feed existiert nicht und TmpRels unbelegt */
findRelations(QList, TmpRels, Relations) :-
    \+member(feed, QList),
    length(TmpRels, 0),
    last(QList, Relation),
    atom_chars(Relation, RelChars),
    write('Rel Chars2: '), writeln(RelChars),
    member('<', RelChars), 
    append([],[Relation],Relations)
    .

 /* Ersetzt die Liste von Relationen im Query durch einen 
    Platzhalter mit gezaehltem Index*/
replaceRelationInQuery([], elem, Query, _, _).

replaceRelationInQuery([Head | Tail], elem, Query, QueryNew, Idx) :-
    Tail = [],
    plus(11, Idx, IdxNew),
    atom_number(IdxAtom, IdxNew),
    atom_concat(elem, IdxAtom, ReplaceValue),
    /* write('Query End: '), writeln(Query),
    write('ReplValue End: '), writeln(ReplaceValue), */
    nb_setval('hit', 0),
    replaceAtom(Head, ReplaceValue, Query, TMPQuery),
    QueryNew = TMPQuery
    .

replaceRelationInQuery([Head | Tail], elem, Query, QueryNew, Idx) :-
    plus(11, Idx, IdxNew),
    atom_number(IdxAtom, IdxNew),
    atom_concat(elem, IdxAtom, ReplaceValue),
    /* write('ReplValue : '), writeln(ReplaceValue),*/
    nb_setval('hit', 0),
    replaceAtom(Head, ReplaceValue, Query, TMPQuery),
    replaceRelationInQuery(Tail, elem, TMPQuery, QueryNew, IdxNew)
    .

remove_at(X,[X|Xs],1,Xs).
remove_at(X,[Y|Xs],K,[Y|Ys]) :- K > 1, 
   K1 is K - 1, remove_at(X,Xs,K1,Ys).

/* 

subs(Suchwert, Ersetzungswert, InitialListeNL, ErsetzungsListeNL). 
Ermoeglicht das ersetzen 'eines' Treffers. alle anderen 
werden wieder in Ergebnis-NestedList gepackt 
alternativ: 

*/
replaceAtom(_, _, [], []).
replaceAtom(X, Y, [H1|T1], [H2|T2]) :-
    (H1 == X ->
    nb_getval('hit', Hit),
    plus(1, Hit, HitNew),
    nb_setval('hit', HitNew),
    (HitNew =< 1 ->
            H2 = Y,
        replaceAtom(X, Y, T1, T2)
    ;     H1 = H2,
        replaceAtom(X, Y, T1, T2)
    )
    ; is_list(H1) ->
        replaceAtom(X, Y, H1, H2),
        replaceAtom(X, Y, T1, T2)
    ;
        H1 = H2,
        replaceAtom(X, Y, T1, T2)
    ).
/* 

Ersetzt alle Treffer mit uebergebenem Wert und gibt NestedList zurueck

----
replaceAtomAll(_, _, [], []).
replaceAtomAll(X, Y, [X|T1], [Y|T2]) :- 
    replaceAtomAll(X, Y, T1, T2), !
    .
replaceAtomAll(X, Y, [H|T1], [H|T2]) :- 
    \+ is_list(H), 
    replaceAtomAll(X, Y, T1, T2), !
    .
replaceAtomAll(X, Y, [H1|T1], [H2|T2]) :- 
    replaceAtomAll(X, Y, H1, H2), 
    replaceAtomAll(X, Y, T1, T2)
    .
----

*/


print_all([]).
print_all([X|Rest]) :- 
    write(X), nl, 
    print_all(Rest).

list_to_downcase([]).
list_to_downcase([X|Rest]) :-
    write(X),
    downcase_atom(X, LowerX),
    write(' -> '),
    write(LowerX), nl,
    flatten(Rest, FlRest),
    list_to_downcase(FlRest)    
    .

