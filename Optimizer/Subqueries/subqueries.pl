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

18 Transforming nested queries to their canonical form

This file enables the SECONDO optimizer to process subqueries or nested queries formulated in SQL syntax. 
A subquery or nested query is a fully-fledged query contained in another query. 
For example

---- select * 
     from staedte 
     where plz in (select p:plz from plz as p)
---- 	

will yield all entries of relation staedte where plz contains an entry with matching plz. 
Depending on its form, subqueries can yield scalar results, single tuple results or stream of tuple results. 
Subqueries whith a scalar result can be used in the attribute list, the from-clause and the where-clause


Queries containing subqueries  will be rewritten by this module to an equivalent canonical form. 


 
Facts about comparison operators

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
  
  
/*

Predicates to recognize nested queries.

*/
  
isNestedQuery(select AttrList from _ where _) :-
  makeList(AttrList, AttrList2),
  not(sublist(isSubqueryAttr, AttrList2, [])).
  
isNestedQuery(select _ from RelList where _) :-
  makeList(RelList, RelList2),
  not(sublist(isQuery, RelList2, [])).  

isNestedQuery(select _ from _ where PredList) :-
  makeList(PredList, PredList2),
  not(sublist(isSubqueryPred, PredList2, [])).
  
/*

Predicates to recognize subqueries in attribute lists and attributes.

*/
  
isSubqueryAttr(Attr as _) :-
  isQuery(Attr).
  
isSubqueryAttr(Attr) :- 
  isQuery(Attr).

/*

Predicates to recognize subqueries in predicate lists and predicates.

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

/*

special handling for operator ~in~ which can be replaced by ~=~ as join operator

*/

subqueryToJoinOp(in, =).
subqueryToJoinOp(Op, Op).


/*
The following syntax for subqueries will recognized by SECONDO:

~exists~

----    select <attr-list>
        from <rel-list>
        where exists(<Subquery>)
----

~not exists~

----    select <attr-list>
        from <rel-list>
        where not exists(<Subquery>)
----

~$<$compop$>$ any~

----    select <attr-list>
        from <rel-list>
        where <compop> any(<Subquery>)
----

~$<$compop$>$ all~

----    select <attr-list>
        from <rel-list>
        where <compop> all(<Subquery>)
----

~in~ is equivalent to ~$=$ any~

~not in~ is equivalent to ~$<$$>$~ all

18.1 Subqueries in predicates
scalar subquery
single row subquery
single column subquery
table subquery

18.2 subqueries in from clause
table subquery

---- rewriteQueryForSubqueryProcessing(+QueryIn,-QueryOut)
----

Rewrites a query by transforming subqueries within ~QueryIn~ to their canonical form and unifies rewritten query with ~QueryOut~.

*/

rewriteQueryForSubqueryProcessing(QueryIn, QueryIn) :-
  not(optimizerOption(subqueries)), !.
  
rewriteQueryForSubqueryProcessing(NestedQuery, CanonicalQuery) :-
    transform(NestedQuery, CanonicalQuery),
    dm(rewriteQueryForSubqueryProcessing,['\nREWRITING: Subqueries\n\tIn:  ',NestedQuery,'\n\tOut: ',
                    CanonicalQuery,'\n\n']). 
					
/*

Default handling for queries without subqueries.

*/

transform(Query, Query) :-
  not(isNestedQuery(Query)), !.
  
  
/*

Nested queries are transformed to their canonical form in the attribute list, the relation list and the predicate list.

*/
  
transform(select Attrs from Rels where Preds, select CanonicalAttrs from CanonicalRels where CanonicalPreds) :-
  transformNestedAttributes(Rels, Rels2, Attrs, Attrs2, Preds, Preds2),
  transformNestedRelations(Rels2, Rels3, Attrs2, Attrs3, Preds2, Preds3),
  transformNestedPredicates(Attrs3, CanonicalAttrs, Rels3, CanonicalRels, Preds3, CanonicalPreds).
  
  
/*

Subqueries in the attribute list have to yield a scalar result. If the nested query is uncorrelated with the outer query,
it can be evaluated independently of it and replaced by its result.

*/
  
transformNestedAttributes([], [], Rels, Rels, Preds, Preds ).
  
transformNestedAttributes(Attr, Attr2, Rels, Rels2, Preds, Preds2) :-
  not(is_list(Attr)),
  transformNestedAttribute(Attr, Attr2, Rels, Rels2, Preds, Preds2).
  
transformNestedAttributes([ Attr | Rest ], [ Attr2 | Rest2 ], Rels, Rels2, Preds, Preds2) :-
  transformNestedAttribute(Attr, Attr2, Rels, Rels2, Preds, Preds2),
  transformNestedAttributes(Rest, Rest2, Rels, Rels2, Preds, Preds2).

/*

Replace a scalar, uncorrelated subquery with its result in the attribute list. As SECONDO does not generate temporary names
for constants in the attribute list, aliasing has to occurr.

*/  
  
transformNestedAttribute(Subquery as Variable, Value, Rels, Rels, Preds, Preds) :- 
  nestingType(Subquery, a),
  optimize(Subquery, SecondoQuery, _),
  atom(SecondoQuery),
  atom_concat('query ', SecondoQuery, QueryText),
  secondo(QueryText, [_, Result]),
  not(is_list(Result)),
  atom_concat(Result, ' as ', Temp),
  atom_concat(Temp, Variable, Value).  
  
transformNestedAttribute(Attr, Attr, Rels, Rels, Preds, Preds) :- 
  not(isQuery(Attr)).  
  
  
/*



*/

transformNestedRelations(Attrs, Attrs, [], [], Preds, Preds).  
  
transformNestedRelations(Attrs, Attrs2, Rel, Rel2, Preds, Preds2) :-
  not(is_list(Rel)),
  transformNestedRelation(Attrs, Attrs2, Rel, Rel2, Preds, Preds2).
  
transformNestedRelations(Attrs, Attrs2, [ Rel | Rest ], [ Rel2 | Rest2], Preds, Preds2) :-
  transformNestedRelation(Attrs, Attrs2, Rel, Rel2, Preds, Preds2),
  transformNestedRelations(Attrs, Attrs2, Rest, Rest2, Preds, Preds2).
  
transformNestedRelation(Attrs, Attrs, Rel, Rel, Preds, Preds) :- 
  not(isQuery(Rel)).


/*

Subqueries in predicates will be transformed to their canonical equivalents by algorithm NEST-G, as described in 

*/  

transformNestedPredicates(Attrs, Attrs, Rels, Rels, [], []).  
  
transformNestedPredicates(Attrs, Attrs2, Rels, Rels2, Pred, Pred2) :-
  not(is_list(Pred)),
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, Pred2).
  
transformNestedPredicates(Attrs, Attrs2, Rels, Rels2, [ Pred | Rest ], [ Pred2 | Rest2 ]) :-
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, Pred2),
  transformNestedPredicates(Attrs, Attrs2, Rels, Rels2, Rest, Rest2).
  
transformNestedPredicate(Attrs, Attrs, Rels, Rels, Pred, Pred) :- 
  not(isSubqueryPred(Pred)).
  
/*

The subquery in this predicate does not contain a join predicate that references the relation of the outer query block
and has an aggregation function associated with the column name. As the subquery yields a scalar result, which is independent
of the outer query, the subquery will be evaluated and replaced by its result.

*/
  
transformNestedPredicate(Attrs, Attrs, Rels, Rels, SubqueryPred, SubqueryPred2) :-
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
and does not have an aggregation function associated with the column name. The predicate will be replaced by a disjunction
of predicates generated from the subquery result.

*/
  
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, SubqueryPred, SubqueryPredList) :-
  SubqueryPred =.. [Op, Attr, Subquery],
  nestingType(Subquery, n),
  subqueryToJoinOp(Op, JoinOp),
  Subquery =.. [from, Select, Where],
  restrict(Attrs, Rels, Attrs2),
  Select =.. [select | SubAttrs],
  makeList(SubAttrs, SubAttrList),  
  makePredList(Attr, JoinOp, SubAttrList, SubqueryPred2),
  makeList(Rels, RelsList),
  (Where =.. [where, SubRels, SubPreds] 
    -> ( append([SubRels], RelsList, Rels2),
	     append([SubPreds], [SubqueryPred2], SubqueryPredList) )
	; ( append([Where], RelsList, Rels2),
        SubqueryPredList = SubqueryPred2 )).
  
makePredList(Attr, Op, [ Result | Rest ], SubqueryPred2) :-
  makePredList(Attr, Op, Rest, SubqueryPred),
  Term =.. [Op, Attr, Result],
  ( SubqueryPred = [] 
    -> SubqueryPred2 = Term
	; SubqueryPred2 =.. [or, SubqueryPred, Term] ).
  
makePredList(_, _, [], []).

/* 

Restrict ~StarQueries~ to only select attributes of the given relations

*/

restrict(*, Rels, AttrList) :-
  not(is_list(Rels)),  
  relation(Rels, AttrList).
  
restrict(*, Rel as Alias, AttrList2) :-
  restrict(*, Rel, AttrList),
  maplist(alias(Alias), AttrList, AttrList2).
  
restrict(*, [ Rel | Rest ], AttrList) :-
  restrict(*, Rel, Attrs),
  restrict(*, Rest, Attrs2),
  append(Attrs, Attrs2, AttrList).
  
restrict(*, [], []).
  
restrict(Attrs, _, Attrs).

/*

---- alias(?Alias, ?Attr, ?Alias:Attr).
----

Helper predicate to alias/un-alias attribute names.

*/

alias(Alias, Attr, Alias:Attr).
  
  
/*


type-A nesting, uncorrelated subquery with scalar result

*/

nestingType(Subquery, a) :-
  aggrQuery(Subquery, _, _, _),
  callLookup(Subquery, _),
  !.
  
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