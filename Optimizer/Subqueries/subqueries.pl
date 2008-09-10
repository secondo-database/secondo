/*
//paragraph [10] title: [{\Large \bf ]  [}]
//characters [1] formula:       [$]     [$]
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[**] [$**$]
//[star] [$*$]
//[->] [$\rightarrow$]
//[toc] [\tableofcontents]
//[=>] [\verb+=>+]
//[:Section Translation] [\label{sec:translation}]
//[Section Translation] [Section~\ref{sec:translation}]
//[:Section 4.1.1] [\label{sec:4.1.1}]
//[Section 4.1.1] [Section~\ref{sec:4.1.1}]
//[Figure pog1] [Figure~\ref{fig:pog1.eps}]
//[Figure pog2] [Figure~\ref{fig:pog2.eps}]
//[newpage] [\newpage]

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

August 2008, Burkart Poneleit. Initial Version

18 Unnesting nested queries

This file enables the SECONDO optimizer to process subqueries or nested queries formulated in SQL syntax. 
A subquery or nested query is a fully-fledged query contained in another query. 
For example
---- select * 
     from staedte 
     where plz in (select p:plz from plz as p)
---- 	
will yield all entries of relation staedte where plz contains an entry with matching plz. 
Depending on its form, subqueries can yield scalar results, single tuple results or stream of tuple results.

Queries containing subqueries  will be rewritten by this module to an equivalent unnested form. 


 
facts about comparison operators

*/

isComparisonOp(=).
isComparisonOp(<=).
isComparisonOp(>=).
isComparisonOp(<).
isComparisonOp(>).
isComparisonOp(<>).

/*
---- isQuery(+Term)
----
This predicate is true, if the term is a query formulated in the sql syntax defined for secondo

*/

isQuery(Query) :-
  callLookup(Query, _).
  
isNestedQuery(select AttrList from _ where _) :-
  makeList(AttrList, AttrList2),
  not(sublist(isSubqueryAttr, AttrList2, [])).
  
isNestedQuery(select _ from RelList where _) :-
  makeList(RelList, RelList2),
  not(sublist(isQuery, RelList2, [])).  

isNestedQuery(select _ from _ where PredList) :-
  makeList(PredList, PredList2),
  not(sublist(isSubqueryPred, PredList2, [])).
  
isSubqueryAttr(Attr as _) :-
  isQuery(Attr).
  
isSubqueryAttr(Attr) :- isQuery(Attr).

/*

Predicates to recognize subqueries in predicate lists and predicates

*/

isSubqueryPred(Pred) :- 
  compound(Pred),
  Pred =.. [Op, _, Subquery],
  isSubqueryOp(Op),
  isQuery(Subquery).
  
isSubqueryPred(Pred) :- 
  compound(Pred),
  Pred =.. [Op, Subquery],
  isSubqueryOp(Op),
  isQuery(Subquery).  
  
isSubqueryPred(Pred) :-
  compound(Pred),
  Pred =.. [Op, _, QuantifiedPred],
  QuantifiedPred =.. [Quantifier, Subquery],
  isComparisonOp(Op),
  isQuantifier(Quantifier),
  isQuery(Subquery).
  
isSubqueryOp(in).
isSubqueryOp(exists).
isSubqueryOp(Op) :-
  isComparisonOp(Op).

isQuantifier(all).
isQuantifier(any).
isQuantifier(some).

:- op(700, xfx, <>).
:- op(940, xfx, in).

subqueryToJoinOp(in, =).
subqueryToJoinOp(Op, Op).


/*
The following syntax for subqueries is recognized by SECONDO:

exists

----    select <attr-list>
        from <rel-list>
        where exists(<Subquery>)
----

not exists

----    select <attr-list>
        from <rel-list>
        where not exists(<Subquery>)
----

<compop> any

----    select <attr-list>
        from <rel-list>
        where <compop> any(<Subquery>)
----

<compop> all

----    select <attr-list>
        from <rel-list>
        where <compop> all(<Subquery>)
----

in = equivalent to = any

not in = equivalent to <> all

18.1 Subqueries in predicates
scalar subquery
single row subquery
single column subquery
table subquery

18.2 subqueries in from clause
table subquery

---- rewriteQueryForSubqeryProcessing(+QueryIn,-QueryOut)
----

Rewrites a query by unnesting subqueries within ~QueryIn~ and unifies rewritten query with ~QueryOut~.

*/

rewriteQueryForSubqueryProcessing(QueryIn, QueryIn) :-
  not(optimizerOption(subqueries)), !.
  
rewriteQueryForSubqueryProcessing(QueryIn, QueryOut) :-
    unnestSubqueries(QueryIn, QueryOut),
    dm(rewriteQueryForSubqueryProcessing,['\nREWRITING: Subqueries\n\tIn:  ',QueryIn,'\n\tOut: ',
                    QueryOut,'\n\n']). 
					
/*

Default handling.

*/

unnestSubqueries(Query, Query) :-
  not(isNestedQuery(Query)), !.
  
  
/*

Handling of nested queries in predicate list. 

*/
  
unnestSubqueries(select Attrs from Rels where Preds, select UnnestedAttrs from UnnestedRels where UnnestedPreds) :-
  unnestAttributes(Rels, Rels2, Attrs, Attrs2, Preds, Preds2),
  unnestRelations(Rels2, Rels3, Attrs2, Attrs3, Preds2, Preds3),
  unnestPredicates(Preds3, UnnestedPreds, Attrs3, UnnestedAttrs, Rels3, UnnestedRels).
  
unnestAttributes(Preds, Preds, [], [], Rels, Rels).
  
unnestAttributes(Preds, Preds2, Attr, Attr2, Rels, Rels2) :-
  not(is_list(Attr)),
  unnestAttribute(Preds, Preds2, Attr, Attr2, Rels, Rels2).
  
unnestAttributes(Preds, Preds2, [ Attr | Rest ], [ Attr2 | Rest2 ], Rels, Rels2) :-
  unnestAttribute(Preds, Preds2, Attr, Attr2, Rels, Rels2),
  unnestAttributes(Preds, Preds2, Rest, Rest2, Rels, Rels2).

unnestAttribute(Preds, Preds, Subquery as Variable, Value, Rels, Rels) :- 
  aggrQuery(Subquery, _, _, _),
  optimize(Subquery, SecondoQuery, _),
  atom(SecondoQuery),
  atom_concat('query ', SecondoQuery, QueryText),
  secondo(QueryText, [_, Result]),
  not(is_list(Rest)),
  atom_concat(Result, ' as ', Temp),
  atom_concat(Temp, Variable, Value).  
  
unnestAttribute(Preds, Preds, Attr, Attr, Rels, Rels) :- 
  not(isQuery(Attr)).  
  
unnestRelations(Preds, Preds, Attrs, Attrs, [], []).  
  
unnestRelations(Preds, Preds2, Attrs, Attrs2, Rel, Rel2) :-
  not(is_list(Rel)),
  unnestRelation(Preds, Preds2, Attrs, Attrs2, Rel, Rel2).
  
unnestRelations(Preds, Preds2, Attrs, Attrs2, [ Rel | Rest ], [ Rel2 | Rest2]) :-
  unnestRelation(Preds, Preds2, Attrs, Attrs2, Rels, Rels2),
  unnestRelations(Rests, Rests2, Attrs, Attrs2, Rest, Rest2).
  
unnestRelation(Preds, Preds, Attrs, Attrs, Rel, Rel) :- 
  not(isQuery(Rel)). 

unnestPredicates([], [], Attrs, Attrs, Rels, Rels).  
  
unnestPredicates(Pred, Pred2, Attrs, Attrs2, Rels, Rels2) :-
  not(is_list(Pred)),
  unnestPredicate(Pred, Pred2, Attrs, Attrs2, Rels, Rels2).
  
unnestPredicates([ Pred | Rest ], [ Pred2 | Rest2 ], Attrs, Attrs2, Rels, Rels2) :-
  unnestPredicate(Pred, Pred2, Attrs, Attrs2, Rels, Rels2),
  unnestPredicates(Rest, Rest2, Attrs, Attrs2, Rels, Rels2).
  
unnestPredicate(Pred, Pred, Attrs, Attrs, Rels, Rels) :- 
  not(isSubqueryPred(Pred)).
  
/*

The subquery in this predicate does not contain a join predicate that references the relation of the outer query block
and has an aggregation function associated with the column name. As the subquery yields a scalar result, which is independent
of the outer query, the subquery will be evaluated and replaced by its result.

*/
  
unnestPredicate(SubqueryPred, SubqueryPred2, Attrs, Attrs, Rels, Rels) :-
  SubqueryPred =..[Op, Attr, Subquery],
  nestingType(Subquery, a),
  optimize(Subquery, SecondoQuery, _),
  atom(SecondoQuery),
  atom_concat('query ', SecondoQuery, QueryText),
  secondo(QueryText, [_, Res]),
  not(is_list(Res)),
  SubqueryPred2 =.. [Op, Attr, Res].  
 
/*

The subquery in this predicate does not contain a join predicate that references the relation of the outer query block
and does not have an aggregation function associated with the column name. 

*/
  
unnestPredicate(SubqueryPred, SubqueryPred2, Attrs, Attrs2, Rels, Rels2) :-
  SubqueryPred =.. [Op, Attr, Subquery],
  nestingType(Subquery, n),
  subqueryToJoinOp(Op, JoinOp),
  Subquery =.. [from, Select, Where],
  constrict(Attrs, Rels, Attrs2),
  Select =.. [select | SubAttrs],
  makeList(Rels, RelsList),
  (Where =.. [where, SubRels, Preds] 
    -> (append([SubRels], RelsList, Rels2))
	; append([Where], RelsList, Rels2)), 
  makeList(SubAttrs, SubAttrList),
  makePredList(Attr, JoinOp, SubAttrList, SubqueryPred2).
  
makePredList(Attr, Op, [ Result | Rest ], SubqueryPred3) :-
  makePredList(Attr, Op, Rest, SubqueryPred),
  Term =.. [Op, Attr, Result],
  write(Term), nl,
  append([Term], SubqueryPred, SubqueryPred2),
  flatten(SubqueryPred2, SubqueryPred3).
  
makePredList(_, _, [], []).

constrict(*, Rels, Attrs2) :-
  relation(Rels, Attrs2).
  
constrict(Attrs, Rels, Attrs).
  
  
/*

type-A nesting, uncorrelated subquery with scalar result

*/

nestingType(Subquery, a) :-
  aggrQuery(Subquery, _, _, _),
  callLookup(Subquery, _).
  
/*

type-N nesting, uncorrelated subquery with row result

*/

nestingType(Subquery, n) :-
  not(aggrQuery(Subquery , _, _, _)),
  callLookup(Subquery, _),
  !.
  
  
/*

type-J nesting, correlated subquery without aggregation function, row result

*/

nestingType(Subquery, j) :-
  not(aggrQuery(Subquery , _, _, _)).
  
  
/*

type-JA nesting, correlated subquery with aggregation function, scalar result.

*/

nestingType(Subquery, ja) :-
  aggrQuery(Subquery , _, _, _).
  
  
/*

for Debugging purposes

*/

:- [subquerytest].