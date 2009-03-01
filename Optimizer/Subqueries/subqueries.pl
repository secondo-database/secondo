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
Depending on its form, subqueries can yield scalar results, single tuple results or table results. 
Subqueries whith a scalar result can be used in the attribute list, the from-clause and the where-clause. 
If the subquery is not correlated with the outer query, i.e. does not reference any attribute of the outer relation
in its predicates, it can be evaluated independently of the outer query. Subqueries in predicates can always be evaluated
by the so called nested-iteration method. The subuqery will be evaluated for every result tuple of the outer query. As
there are always canonical forms of the queries which do not use nesting and yield the same result, 


Queries containing subqueries  will be rewritten by this module to an equivalent canonical form. 


 
Facts about comparison operators

*/

isComparisonOp(=).
isComparisonOp(=<).
isComparisonOp(>=).
isComparisonOp(<).
isComparisonOp(>).
isComparisonOp(<>).

/*

---- isQuery(+Term)
----
This predicate is true, if the term is a query formulated in the sql syntax defined for secondo. 

*/

isQuery(Query) :-
  catch(callLookup(Query, _), error_SQL(optimizer_lookupPred1(Term, Term):unknownIdentifier#_), true).
  
isQuery(Query) :-
  catch(callLookup(Query, _), error_SQL(optimizer_lookupPred(Term, Term):malformedExpression#_), true).
  
  
/*

Predicates to recognize nested queries.

*/

isNestedQuery(Query orderby _) :- isNestedQuery(Query).
isNestedQuery(Query groupby _) :- isNestedQuery(Query).
isNestedQuery(Query first _) :- isNestedQuery(Query).
isNestedQuery(Query last _) :- isNestedQuery(Query).
  
isNestedQuery(select _ from _ where PredList) :-
  makeList(PredList, PredList2),
  not(sublist(isSubqueryPred, PredList2, [])).  
  
isNestedQuery(select AttrList from RelList where _) :-
  isNestedQuery(select AttrList from RelList).
  
isNestedQuery(select AttrList from _) :-
  makeList(AttrList, AttrList2),
  not(sublist(isSubqueryAttr, AttrList2, [])).
  
isNestedQuery(select _ from RelList) :-
  makeList(RelList, RelList2),
  not(sublist(isQuery, RelList2, [])).   
  
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

isSubqueryPred(not(Pred)) :-
  isSubqueryPred(Pred).
  
isSubqueryPred(Pred) :-
  Pred =.. [not, Attr, in(Query)],
  isSubqueryPred(Attr in(Query)).

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
:- op(700, xfx, =+). % outer join operator
:- op(940, xfx, in). % defined with different priority for macros
:- op(941, xfx, not).
:- op(799,  fy, left).
:- op(799,  fy, outerjoin).
:- op(799,  fy, right).

/*

Special handling for operator ~in~ which can be replaced by ~=~ as join operator, when a subquery is transformed 
to its canonical form.

*/

subqueryToJoinOp(in, =).
subqueryToJoinOp(Op, Op).

anyToAggr(<, max).
anyToAggr(<=, max).
anyToAggr(>, min).
anyToAggr(>=, min).

anyToAggr(<, min).
anyToAggr(<=, min).
anyToAggr(>, max).
anyToAggr(>=, max).

/* 

Predicates for Type-A Queries

*/

evaluateSubquery(CanonicalSubquery, Result) :-
  nestingType(CanonicalSubquery, a), 
  optimize(CanonicalSubquery, SecondoQuery, _), 
  atom(SecondoQuery),
  atom_concat('query ', SecondoQuery, QueryText),
  secondo(QueryText, [_, Res]),
  not(is_list(Res)),  
  typeConversion(Res, Result).
  
typeConversion(Data, Result) :-
  string_concat('"', Temp, Data),
  string_concat(TempRes, '"', Temp),
  string_to_list(TempRes, Result).
  
typeConversion(Result, Result).

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
  not(optimizerOption(subqueryUnnesting)), !.
  
rewriteQueryForSubqueryProcessing(NestedQuery, CanonicalQuery) :-
    transform(NestedQuery, CanonicalQuery),
    dm(subqueryUnnesting,['\nREWRITING: Subqueries\n\tIn:  ',NestedQuery,'\n\tOut: ',
                    CanonicalQuery,'\n\n']). 
					
 
  
/*

Nested queries are transformed to their canonical form in the attribute list, the relation list and the predicate list.

*/

transform(Query orderby Order, Query2 orderby Order) :-
  transform(Query, Query2).
  
transform(Query groupby Group, Query2 groupby Group) :-
  transform(Query, Query2).

transform(Query first N, Query2 first N) :-
  transform(Query, Query2).

transform(Query last N, Query2 last N) :-
  transform(Query, Query2).  
  
transform(select Attrs from Rels where Preds, select CanonicalAttrs from CanonicalRels where CanonicalPreds) :-
  transform(select Attrs from Rels, select Attrs2 from Rels2),
  transformNestedPredicates(Attrs2, CanonicalAttrs, Rels2, CanonicalRels, Preds, CanonicalPredList),
  flatten(CanonicalPredList, CanonicalPreds).
  
transform(select Attrs from Rels, select CanonicalAttrs from CanonicalRels) :-
  transformNestedAttributes(Attrs, Attrs2, Rels, Rels2),
  transformNestedRelations(Attrs2, CanonicalAttrs, Rels2, CanonicalRels).  
  
/*

Default handling for queries without subqueries.

*/

transform(Query, Query).

/* :-
  write_canonical(Query),
  not(isNestedQuery(Query)), !, write('No nesting').  */

  
/*

Subqueries in the attribute list have to yield a scalar result. If the nested query is uncorrelated with the outer query,
it can be evaluated independently of it and replaced by its result.

*/
  
transformNestedAttributes([], [], Rels, Rels).
  
transformNestedAttributes(Attr, Attr2, Rels, Rels2) :-
  not(is_list(Attr)),
  transformNestedAttribute(Attr, Attr2, Rels, Rels2).
  
transformNestedAttributes([ Attr | Rest ], [ Attr2 | Rest2 ], Rels, Rels2) :-
  transformNestedAttribute(Attr, Attr2, Rels, Rels2),
  transformNestedAttributes(Rest, Rest2, Rels, Rels2).

/*

Replace a scalar, uncorrelated subquery with its result in the attribute list. As SECONDO does not generate temporary names
for constants in the attribute list, aliasing has to occurr.

*/  
  
transformNestedAttribute(Subquery as Variable, Value, Rels, Rels) :- 
  nestingType(Subquery, a),
  optimize(Subquery, SecondoQuery, _),
  atom(SecondoQuery),
  atom_concat('query ', SecondoQuery, QueryText),
  secondo(QueryText, [_, Result]),
  not(is_list(Result)),
  atom_concat(Result, ' as ', Temp),
  atom_concat(Temp, Variable, Value).  
  
transformNestedAttribute(Attr, Attr, Rels, Rels) :- 
  not(isQuery(Attr)).  
  
  
/*

Handling of nested queries in the from clause. No transformation is applied at the moment.

*/

transformNestedRelations(Attrs, Attrs, [], []).  
  
transformNestedRelations(Attrs, Attrs2, Rel, Rel2) :-
  not(is_list(Rel)),
  transformNestedRelation(Attrs, Attrs2, Rel, Rel2).
  
transformNestedRelations(Attrs, Attrs2, [ Rel | Rest ], [ Rel2 | Rest2]) :-
  transformNestedRelation(Attrs, Attrs2, Rel, Rel2),
  transformNestedRelations(Attrs, Attrs2, Rest, Rest2).
  
 
transformNestedRelation(Attrs, Attrs, Query, NewRel) :-  
  transform(Query, CanonicalQuery),
  isQuery(CanonicalQuery),
  newTempRel(CanonicalQuery, NewRel).
  
transformNestedRelation(Attrs, Attrs, (Query) as _, NewRel) :-
  transform(Query, CanonicalQuery),
  isQuery(CanonicalQuery),
  newTempRel(CanonicalQuery, NewRel).
  
transformNestedRelation(Attrs, Attrs, Rel, Rel) :- 
  not(isQuery(Rel)).  


/*

Subqueries in predicates will be transformed to their canonical equivalents by the following algorithm NEST-G

(1) Transform each type-A predicate to a simple predicate by completely evaluating the subquery. If the subquery itself is
nested, algorithm NEST-G is applied recursively on the subquery. When the nested query has been evaluated, 
it is replaced by its result.
(2) Transform each type-JA predicate to an equivalent type-N or type-J subquery by algorithm NEST-JA/NEST-G. If the subquery
is further nested, algorithm NEST-G will be invoked recursively on the subquery. The subquery is replaced by the transformed 
subquery
(3) Transform each type-D nested subquery to its canonical form by algorithm NEST-D. If either query of the division predicate
is further nested, algorithm NEST-G will be invoked recursively on the subquery. The predicate will be replaced by an appropriate
set of join predicates
(4) Transform the resulting query, which consists only of type-N and type-J subqueries, to an equivalent canonical query by
algorithm NEST-N-J

*/  

transformNestedPredicates(Attrs, Attrs, Rels, Rels, [], []).  
  
transformNestedPredicates(Attrs, Attrs2, Rels, Rels2, Pred, Pred2) :-
  not(is_list(Pred)),
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, Pred2).
  
transformNestedPredicates(Attrs, Attrs2, Rels, Rels3, [ Pred | Rest ], [ Pred2 | Rest2 ]) :-  
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, Pred2),
  dc(subqueryUnnesting, ( not(Pred = Pred2)
						  -> dm(subqueryUnnesting, ['\nPredicate: ', Pred, '\nTransformedPredicate: ', Pred2])
						  ; true
						 )),
  transformNestedPredicates(Attrs, Attrs2, Rels2, Rels3, Rest, Rest2).
  
transformNestedPredicate(Attrs, Attrs, Rels, Rels, Pred, Pred) :- 
  not(isSubqueryPred(Pred)).
  
/* The SQL predicates EXISTS, NOT EXISTS, ALL and ANY are transformed to a canonical form, which can be further unnested */

transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, exists(select SubAttr from RelsWhere), TransformedQuery) :-
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, 0 < (select count(SubAttr) from RelsWhere), TransformedQuery),
  write_canonical(TransformedQuery).
  
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, not(exists(select SubAttr from RelsWhere)), TransformedQuery) :-
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, 0 = (select count(SubAttr) from RelsWhere), TransformedQuery),
  write_canonical(TransformedQuery).
  
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, TransformedQuery) :-
  Pred =.. [Op, Attr, any(select SubAttr from RelsWhere)],
  anyToAggr(Op, Aggr),
  AggrExpression =.. [Aggr, SubAttr],
  NewPred =.. [Op, Attr, select AggrExpression from RelsWhere],
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, NewPred, TransformedQuery),
  write_canonical(TransformedQuery).  
  
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Attr = any(Query), TransformedQuery) :-
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Attr in(Query), TransformedQuery),
  write_canonical(TransformedQuery).
  
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Attr <> any(Query), TransformedQuery) :-
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Attr not in(Query), TransformedQuery),
  write_canonical(TransformedQuery).  
  
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, TransformedQuery) :-
  Pred =.. [Op, Attr, all(select SubAttr from RelsWhere)],
  allToAggr(Op, Aggr),
  AggrExpression =.. [Aggr, SubAttr],
  NewPred =.. [Op, Attr, select AggrExpression from RelsWhere],
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, NewPred, TransformedQuery),
  write_canonical(TransformedQuery).    

  
/*

The subquery in this predicate does not contain a join predicate that references the relation of the outer query block
and has an aggregation function associated with the column name. As the subquery yields a scalar result, which is independent
of the outer query, the subqeery will be evaluated and replaced by its result.

*/
  
transformNestedPredicate(Attrs, Attrs, Rels, Rels, SubqueryPred, SubqueryPred2) :-
  SubqueryPred =..[Op, Attr, Subquery],
  evaluateSubquery(Subquery, Result), 
  SubqueryPred2 =.. [Op, Attr, Result].  
 
/*

The subquery in this predicate does not contain a join predicate that references the relation of the outer query block
and does not have an aggregation function associated with the column name. Implements algorithm NEST-N-J
(1) Combine the from-clause of subquery and outer query into one from-clause
(2) Build the conjunction of the where-clauses of inner and outer query into one where-clause
(3) Replace the subquery predicate by a corresponding join predicate
(4) Retain the select-clause of the outer query

*/

% transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, not(Pred2)) :-
%  Pred =.. [not, Attr, in(Query)],
%  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Attr in(Query), Pred2).
  
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, Pred2) :-
  Pred =.. [Op, Attr, Subquery],
  transform(Subquery, CanonicalSubquery),
  CanonicalSubquery =.. [from, Select, Where],
  Select =.. [select, SubAttr],   
  Where =.. [where | [ CanonicalRels | CanonicalPreds ]] ,  
  (nestingType(CanonicalSubquery, n) ; nestingType(CanonicalSubquery, j)), 
  dm(subqueryUnnesting, ['\nNEST-N-J:\n']), 
  ( is_list(SubAttr) 
   -> nth1(1, SubAttr, CanonicalAttr)
   ; SubAttr = CanonicalAttr 
  ),   
  restrict(Attrs, Rels, Attrs2),
  makeList(Rels, RelsList),
  makeList(CanonicalRels, CanonicalRelsList),
  append(RelsList, CanonicalRelsList, Rels2),
  subqueryToJoinOp(Op, NewOp),
  JoinPredicate =.. [NewOp, Attr, CanonicalAttr],
  makeList(CanonicalPreds, CanonicalPredsList),
  append([JoinPredicate], CanonicalPredsList, PredList),
  flatten(PredList, Pred2).
  
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, [JoinPredicate]) :-
  Pred =.. [Op, Attr, Subquery],
  transform(Subquery, CanonicalSubquery),
  CanonicalSubquery =.. [from, Select, CanonicalRels],
  Select =.. [select, SubAttr],   
  (nestingType(CanonicalSubquery, n) ; nestingType(CanonicalSubquery, j)), 
  dm(subqueryUnnesting, ['\nNEST-N-J:\n']), 
  ( is_list(SubAttr) 
   -> nth1(1, SubAttr, CanonicalAttr)
   ; SubAttr =.. CanonicalAttr 
  ),   
  restrict(Attrs, Rels, Attrs2),
  makeList(Rels, RelsList),
  makeList(CanonicalRels, CanonicalRelsList),
  append(RelsList, CanonicalRelsList, Rels2),
  subqueryToJoinOp(Op, NewOp),
  JoinPredicate =.. [NewOp, Attr, CanonicalAttr].  
  
/* 

Implements Algorithm NEST-JA2 as designed by Ganski, Wong. 
(1) Project the join column of the outer relation and restrict it with any simple predicates applying to the outer relation
(2) Create a temporary relation, joining the inner relation with the projection of the outer relation. If the aggragate function
is ~COUNT~, the join must be an outer join, and the inner relation must be restricted and projected before the join is performed.
If the aggregate function is ~COUNT([star])~, compute the ~COUNT~ function over the join column. The join predicate must use the same operator
as the join predicate in the original query (except that it must be converted to the corresponding outer operator in case of
~COUNT~), and the join predicate in the original query must be changed to ~=~. In the select clause, select the join column from
the outer table int the join predicate instead of the inner table. The groupby clause will also contain columns from the outer
relation 
(3) Join the outer relation with  the temporary relation, according to the transformed version of the original query

  
refactored version, not yet complete
TODO: 
translation of outerjoin in translation phase of optimizer
extraction of group variable from tempRel3
maybe replace temporary relations with streams

*/

transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, Pred2) :-  
  Pred =.. [Op, Attr, Subquery],
  transform(Subquery, CanonicalSubquery),
  nestingType(CanonicalSubquery, ja),
  dm(subqueryUnnesting, ['\nNEST-JA2']),  
  CanonicalSubquery =.. [from, Select, Where],
  Select =.. [select, AggregatedAttr],
  dm(subqueryDebug, ['\nAggregatedAttr: ', AggregatedAttr]),
  Where =.. [where | [CanonicalRels | [CanonicalPreds]]],
  dm(subqueryDebug, ['\nRels: ', Rels]),
  dm(subqueryDebug, ['\nCanonicalRels: ', CanonicalRels]),
  dm(subqueryDebug, ['\nCanonicalPreds: ', CanonicalPreds]),
  makeList(CanonicalRels, CRList),
  makeList(Rels, RList),
  findall(R, (member(R, RList), not(member(R, CRList))), OuterRels),
  dm(subqueryDebug, ['\nOuterRels: ', OuterRels]),
  joinPred(CanonicalRels, OuterRels, CanonicalPreds, JoinPreds, SimpleInnerPreds, SimpleOuterPreds, OuterJoinAttrs),
  dm(subqueryDebug, ['\nSimpleInnerPreds: ', SimpleInnerPreds]),
  dm(subqueryDebug, ['\nSimpleOuterPreds: ', SimpleOuterPreds]),  
  dm(subqueryDebug, ['\nJoinPreds: ', JoinPreds]),
  dm(subqueryDebug, ['\nOuterJoinAttrs: ', OuterJoinAttrs]),  
  ( tempRel1(OuterJoinAttrs, SimpleOuterPreds, TempRel1) 
  ;   throw(error_SQL(subqueries_transformNestedPredicate:tempRel1Error))),  
  ( tempRel2(CanonicalRels, SimpleInnerPreds, TempRel2)
    ;   throw(error_SQL(subqueries_transformNestedPredicate:tempRel2Error))),
  ( tempRel3(AggregatedAttr, OuterJoinAttrs, TempRel1, TempRel2, JoinPreds, TempRel3, GroupVar, NewJoinAttr1, NewJoinAttr2) 
    ;   throw(error_SQL(subqueries_transformNestedPredicate:tempRel3Error))),
  dm(subqueryDebug, ['\nTempRels successful...']), 
  newTempRel(Var),
  alias(Var, GroupVar, AliasedAttr),
  NewPred =.. [Op, AliasedAttr, Attr],
  dm(subqueryDebug, ['\nNewPred: ', NewPred]),
  aliasInternal(NewJoinAttr1, JoinAttr1),
  NewJoinPred =.. [=, JoinAttr1, Var:NewJoinAttr2],  
  dm(subqueryDebug, ['\nNewJoinPred: ', NewJoinPred]),
  append([NewJoinPred], [NewPred], PredNext),
  dm(subqueryDebug, ['\nPredNext :', PredNext]),
  makeList(Rels, RelsList),
  append(RelsList, [TempRel3 as Var], RelsNext),
  dm(subqueryDebug, ['\nRelsNext :', RelsNext]), 
  transformNestedPredicate(Attrs, Attrs2, RelsNext, Rels2, PredNext, Pred2).  
  
aliasInternal(ExternalAttr, InternalAttr) :-
  sub_atom(ExternalAttr, B, _, _, '_'),
  sub_atom(ExternalAttr, 0, B, _, Attr),
  B1 is B + 1,
  sub_atom(ExternalAttr, B1, _, 0, Internal),
  InternalAttr =.. [(:), Internal, Attr].
  
aliasInternal(Attr, Attr).
  
/* aliasPreds(_, _, [], []).
  
aliasPreds(TempRel3, Var, [ JoinPred | Rest ], [ JoinPred2 | Rest2 ]) :-
  aliasPred(TempRel3, Var, JoinPred, JoinPred2),
  aliasPreds(TempRel3, Var, Rest, Rest2).
  
aliasPred(TempRel3, Var, not(JoinPred), not(JoinPred2)) :-
  aliasPred(TempRel3, Var, JoinPred, JoinPred2).
  
aliasPred(TempRel3, Var, not JoinPred, not JoinPred2) :-
  aliasPred(TempRel3, Var, JoinPred, JoinPred2).  

aliasPred(TempRel3, Var, JoinPred, JoinPred2) :-
  JoinPred =.. [Op, FirstAttr, SecondAttr],
  isAttributeOf(FirstAttr, TempRel3),
  alias(Var, FirstAttr, AliasedAttr),
  JoinPred2 =.. [Op, AliasedAttr, SecondAttr].


aliasPred(TempRel3, Var, JoinPred, JoinPred2) :-
  JoinPred =.. [Op, FirstAttr, SecondAttr],
  isAttributeOf(SecondAttr, TempRel3),
  alias(Var, SecondAttr, AliasedAttr),
  JoinPred2 =.. [Op, FirstAttr, AliasedAttr].  
  
aliasPred(TempRel3, Var, JoinPred, JoinPred2) :-
  JoinPred =.. [Op, FirstAttr, SecondAttr],
  aliasExternal(FirstAttr, Attr),
  isAttributeOf(Attr, TempRel3),
  alias(Var, Attr, AliasedAttr),
  JoinPred2 =.. [Op, AliasedAttr, SecondAttr]. 
  
aliasPred(TempRel3, Var, JoinPred, JoinPred2) :-
  JoinPred =.. [Op, FirstAttr, SecondAttr],
  aliasExternal(SecondAttr, Attr),
  isAttributeOf(Attr, TempRel3),
  alias(Var, Attr, AliasedAttr),
  JoinPred2 =.. [Op, FirstAttr, AliasedAttr]. */
    
  
/*

---- joinPred(+InnerRels, +OuterRels, +PredicateList, -JoinPreds, -InnerPreds, -OuterPreds, -OuterAttrs)
-----

Partition the predicate list in such a way, that ~InnerPreds~ contains all predicates referring to attributes of relations in 
~InnerRels~, ~OuterPreds~ contains all predicates refering to attributes of relations in ~OuterRels~ and ~JoinPreds~ contains
any remaining predicates. 

*/  
  
joinPred(_, _, [], [], [], [], []).
joinPred(InnerRels, OuterRels, [ Pred | Rest ], JoinPreds, InnerPreds, OuterPreds, OuterJoinAttrs) :-
  joinPred(InnerRels, OuterRels, Pred, JoinPred1, InnerPreds1, OuterPreds1, OuterJoinAttrs1),
  joinPred(InnerRels, OuterRels, Rest, JoinPred2, InnerPreds2, OuterPreds2, OuterJoinAttrs2),
  makeList(JoinPred1, JP1List),
  makeList(JoinPred2, JP2List),
  append(JP1List, JP2List, JoinPreds),
  makeList(InnerPreds1, IP1List),
  makeList(InnerPreds2, IP2List),
  append(IP1List, IP2List, InnerPreds),
  makeList(OuterPreds1, OP1List),
  makeList(OuterPreds2, OP2List),
  append(OP1List, OP2List, OuterPreds),
  makeList(OuterJoinAttrs1, OA1List),
  makeList(OuterJoinAttrs2, OA2List),
  append(OA1List, OA2List, OuterJoinAttrs).
  
joinPred(InnerRels, _, Pred, [], InnerPreds, [], []) :-
  not(is_list(Pred)),
  Pred =.. [_ | Args],
  areAttributesOf(Args, InnerRels),
  makeList(Pred, InnerPreds).
  
joinPred(_, OuterRels, Pred, [], [], OuterPreds, []) :-
  not(is_list(Pred)),
  Pred =.. [_ | Args],
  areAttributesOf(Args, OuterRels),
  makeList(Pred, OuterPreds).
  
% joinPred(_, _, Pred, Pred, [], [], []).

joinPred(InnerRels, OuterRels, Pred, [Pred], [], [], OuterAttrs) :-
  makeList(InnerRels, IRelsList),
  append(IRelsList, OuterRels, Rels),
  dm(subqueryDebug, ['\njoinPred:\n\t select * from ', Rels, ' where ', Pred]),
  callLookup(select * from Rels where Pred, _),
  outerAttrs(OuterRels, OuterAttrs),
  dm(subqueryDebug, ['\nOuterAttrs: ', OuterAttrs]).

outerAttrs(OuterRels, OuterAttrs) :-
  findall(Attr, ( 
		member(Rel, OuterRels), 
		usedAttr(rel(Rel, _), 
		attr(Attr2, _, _)), 
		dcName2externalName(Attr, Attr2) 
	), OuterAttrs),
	not(OuterAttrs = []).
  
outerAttrs(OuterRels, OuterAttrs) :-
  findall(Var:Attr, ( 
		member(Rel as Var, OuterRels), 
		usedAttr(rel(Rel, Var), 
		attr(Attr2, _, _)), 
		dcName2externalName(Attr, Attr2) 
	), OuterAttrs),
	not(OuterAttrs = []).

/*

---- areAttributesOf(AttrList, Rels)
----

Is true if all attributes in ~AttrList~ are attributes of relations in ~Rels~

*/

areAttributesOf([], _).

areAttributesOf(Attr, Rels) :-
  ground(Attr),
  not(is_list(Attr)),
  isAttributeOf(Attr, Rels).

areAttributesOf([ Attr | Rest ], Rels) :-
  isAttributeOf(Attr, Rels),  
  areAttributesOf(Rest, Rels).
  
isAttributeOf(Attr, Rel) :-
  not(is_list(Rel)),
  relation(Rel, List),
  member(Attr, List).
  
isAttributeOf(Alias:Attr, Rel as Alias) :-
  relation(Rel, List),
  member(Attr, List).
  
isAttributeOf(Attr, [ Rel | Rest ]) :-
  (( relation(Rel, List),
  member(Attr, List)) ;
  isAttributeOf(Attr, Rest)).
  
isAttributeOf(Attr, [ Rel as Alias | Rest ]) :-
  (( relation(Rel, List),
  alias(Alias, UnaliasedAttr, Attr),
  member(UnaliasedAttr, List)) ;
  isAttributeOf(Attr, Rest)).
  
isAttributeOf(Const, _) :-
  is_list(Const),
  catch(string_to_list(Const, _), _, fail).
  
isAttributeOf(Const, _) :-
  integer(Const).
  
/*

Create a temporary relation which is a projection of Rels to ~JoinAttr~

*/

tempRel1(JoinAttrs, [], TempRel1) :-
  areAttributesOf(JoinAttrs, NeededRels),  
  dm(subqueryDebug, ['\nJoinAttrs: ', JoinAttrs]),
  dm(subqueryDebug, ['\nNeededRels: ', NeededRels]),
  ground(NeededRels),
  TemporaryRel1 =.. [from, select distinct JoinAttrs, NeededRels],
  dm(subqueryUnnesting, ['\nTemporaryRel1: ', TemporaryRel1]), 
  newTempRel(TemporaryRel1, TempRel1).
  
tempRel1(JoinAttrs, Preds, TempRel1) :-
  areAttributesOf(JoinAttrs, NeededRels),
  dm(subqueryDebug, ['\nJoinAttrs: ', JoinAttrs]),
  dm(subqueryDebug, ['\nNeededRels: ', NeededRels]),
  ground(NeededRels),  
  TemporaryRel1 =.. [from, select distinct JoinAttrs, where(NeededRels, Preds)],
  dm(subqueryUnnesting, ['\nTemporaryRel1: ', TemporaryRel1]), 
  newTempRel(TemporaryRel1, TempRel1).  
  
/*

Create a restriction of ~Rels~ by all predicates supplied.

*/

tempRel2(Rels, Preds, TempRel2) :- 
  Where =.. [where, Rels, Preds],
  TemporaryRel2 =.. [from, select *, Where],
  dm(subqueryUnnesting, ['\nTemporaryRel2: ', TemporaryRel2]),
  newTempRel(TemporaryRel2, TempRel2).
  
tempRel2(Rel, [], TempRel2) :-
  TemporaryRel2 =.. [from, select *, Rel],
  dm(subqueryUnnesting, ['\nTemporaryRel2: ', TemporaryRel2]),
  newTempRel(TemporaryRel2, TempRel2).  
  
tempRel2(Rel, [], Rel).
  
/*

Create temporary relation which replaces the aggregation operation by its constant result for the corresponding query.

*/

tempRel3(AggregatedAttr, JoinAttrs, TempRel1, TempRel2, JoinPreds, TempRel3, NewColumn, NewJoinAttr1, NewJoinAttr2) :-
  AggregatedAttr =.. [AggrOp, Attr],
  not(AggrOp = count),  
  dm(subqueryDebug, ['\nAggrOp: ', AggrOp]),
  dm(subqueryDebug, ['\nAttr: ', Attr]), 
  newVariable(NewColumn),  
  makeList(AggregatedAttr as NewColumn, AggrList),
  dm(subqueryDebug, ['\nJoinAttrs: ', JoinAttrs]),
  findall(JoinAttr2, 
    ( member(JoinAttr, JoinAttrs), 
	  aliasExternal(JoinAttr, JoinAttr2)
	),
	JoinAttrs2
  ),  
  dm(subqueryDebug, ['\nJoinAttrs2: ', JoinAttrs2]),
  append(JoinAttrs2, AggrList, AttrList),
  findall(Pred2, 
    ( member(Pred, JoinPreds),
	  aliasExternal(Pred, Pred2)
	),
	JoinPreds2
  ),  
  TempRelation3 =.. [groupby, from(select AttrList, where([TempRel1, TempRel2], JoinPreds2)), JoinAttrs2],
  dm(subqueryDebug, ['\nTemporaryRel3: ', TempRelation3]), 
  optimize(TempRelation3, Plan, _),  
  dm(subqueryDebug, ['\nPlan: ', Plan]),
  findJoinAttrs(Plan, JoinAttr1, JoinAttr2),
  dm(subqueryDebug, ['\nJoinAttr1: ', JoinAttr1, '\nJoinAttr2: ', JoinAttr2]), 
  ( isAttributeOf(JoinAttr1, TempRel1) 
    -> ( NewJoinAttr1 = JoinAttr1, NewJoinAttr2 = JoinAttr2 )
	;  ( NewJoinAttr1 = JoinAttr2, NewJoinAttr2 = JoinAttr1 )
  ),
  append([NewJoinAttr2], AggrList, AttrList2),
  dm(subqueryDebug, ['\nAttrList2: ', AttrList2]),
  TemporaryRel3 =.. [groupby, from(select AttrList2, where([TempRel1, TempRel2], JoinPreds2)), NewJoinAttr2],
  dm(subqueryUnnesting, ['\nTemporaryRel3: ', TemporaryRel3]),  
  newTempRel(TemporaryRel3, TempRel3).   
  
  
tempRel3(AggregatedAttr, JoinAttrs, TempRel1, TempRel2, JoinPreds, TempRel3, NewColumn, NewJoinAttr1, NewJoinAttr2) :-
  AggregatedAttr =.. [AggrOp, Attr],
  AggrOp = count,  
  dm(subqueryDebug, ['\nAggrOp: ', AggrOp]),
  dm(subqueryDebug, ['\nAttr: ', Attr]), 
  newVariable(NewColumn),  
  makeList(AggregatedAttr as NewColumn, AggrList),
  dm(subqueryDebug, ['\nJoinAttrs: ', JoinAttrs]),
  findall(JoinAttr2, 
    ( member(JoinAttr, JoinAttrs), 
	  aliasExternal(JoinAttr, JoinAttr2)
	),
	JoinAttrs2
  ),  
  dm(subqueryDebug, ['\nJoinAttrs2: ', JoinAttrs2]),
  append(JoinAttrs2, AggrList, AttrList),
  findall(Pred2, 
    ( member(Pred, JoinPreds),
	  aliasExternal(Pred, Pred2)
	),
	JoinPreds2
  ),  
  TemporaryRel3 =.. [groupby, from(select AttrList, where([TempRel1, TempRel2], JoinPreds2)), JoinAttrs2],
  dm(subqueryDebug, ['\nTemporaryRel3: ', TemporaryRel3]), 
  optimize(TemporaryRel3, Plan, _),  
  dm(subqueryDebug, ['\nPlan: ', Plan]),
  findJoinAttrs(Plan, JoinAttr1, JoinAttr2),
  dm(subqueryDebug, ['\nJoinAttr1: ', JoinAttr1, '\nJoinAttr2: ', JoinAttr2]), 
  ( isAttributeOf(JoinAttr1, TempRel1) 
    -> OuterJoinPred = outerjoin(JoinAttr1, JoinAttr2)
	;  OuterJoinPred = outerjoin(JoinAttr2, JoinAttr1)
  ),
  OuterJoinPred =.. [outerjoin, NewJoinAttr1, NewJoinAttr2],
  dm(subqueryDebug, ['\nOuterJoinPred: ', OuterJoinPred]),  
  newQuery,
  lookupRel(TempRel1, IntTempRel1),
  dm(subqueryDebug, ['\nIntTempRel1: ', IntTempRel1]),
  lookupRel(TempRel2, IntTempRel2),
  dm(subqueryDebug, ['\nIntTempRel2: ', IntTempRel2]),
  lookupAttr(JoinAttr1 as NewColumn, _ as IntNewColumn),
  dm(subqueryDebug, ['\nIntNewColumn: ', IntNewColumn]),  
  lookupPred(OuterJoinPred, pr(outerjoin(IntJoinAttr1, IntJoinAttr2), _, _)),
  dm(subqueryDebug, ['\nIntJoinAttr1: ', IntJoinAttr1]),
  dm(subqueryDebug, ['\nIntJoinAttr2: ', IntJoinAttr2]),
  plan_to_atom(IntTempRel1, ExtTempRel1),
  dm(subqueryDebug, ['\nExtTempRel1: ', ExtTempRel1]),
  plan_to_atom(IntTempRel2, ExtTempRel2),
  dm(subqueryDebug, ['\nExtTempRel2: ', ExtTempRel2]),
  plan_to_atom(attrname(IntJoinAttr1), ExtJoinAttr1),
  dm(subqueryDebug, ['\nExtJoinAttr1: ', ExtJoinAttr1]),  
  plan_to_atom(attrname(IntJoinAttr2), ExtJoinAttr2),
  dm(subqueryDebug, ['\nExtJoinAttr2: ', ExtJoinAttr2]),
  plan_to_atom(attrname(IntNewColumn), ExtNewColumn),
  dm(subqueryDebug, ['\nExtNewColumn: ', ExtNewColumn]),  
  newTempRel(TempRel3),
  dm(subqueryDebug, ['\nTempRel3: ', TempRel3]),
  concat_atom([TempRel3, ' = ', 
               ExtTempRel1, ' feed ', 
               ExtTempRel2, ' feed ',
               ' smouterjoin[',ExtJoinAttr1, ',', ExtJoinAttr2, ']',			   
               ' sortby[', ExtJoinAttr2, ' asc]', 
               ' groupby[', ExtJoinAttr2, ';', ExtNewColumn, ': group count]',	
               ' project[', ExtJoinAttr2, ',', ExtNewColumn, ']',			   
               ' consume'], 
  '', TempRelation3),  
  dm(subqueryUnnesting, ['\nTempRelation3: ', TempRelation3]),
  ( let(TempRelation3) 
    -> true
	; throw(error_SQL(subqueries_tempRel3:errorInQuery))). 
	
/* 

	Ugly hack to find the join Attributes which the optimizer would select on a Join Query with the same predicates. Used only to select
	which attributes will be used in the outerjoin.
	
*/
	
findJoinAttrs(Atom, JoinAttr1, JoinAttr2) :-
  sub_atom(Atom, B, L, A, 'join['), 
  B1 is B + L,
  sub_atom(Atom, B1, A, 0, Sub),
  sub_atom(Sub, B2, _, _, ']'),
  sub_atom(Sub, 0, B2, _, Sub2),
  concat_atom(['[', Sub2, ']'], Sub3),
  term_to_atom(Term, Sub3),
  Term = [Attr1 | [ Attr2 | _ ]],
  dcName2externalName(JoinAttr1, Attr1),
  dcName2externalName(JoinAttr2, Attr2).	
 
aliasExternal(Var:Attr, AliasedAttr) :-
  concat_atom([Attr, '_', Var], '', AliasedAttr).
	
aliasExternal(Attr, Attr) :-
  atomic(Attr).
  
aliasExternal(Pred, AliasedPred) :-
  compound(Pred),
  Pred =.. [Op, Attr1, Attr2],
  not(Op = (:)),
  aliasExternal(Attr1, AliasedAttr1),
  aliasExternal(Attr2, AliasedAttr2),
  AliasedPred =.. [Op, AliasedAttr1, AliasedAttr2].
  
aliasExternal(Pred, AliasedPred) :-
  compound(Pred),
  Pred =.. [UnaryOp, Pred2],
  aliasExternal(Pred2, AliasedPred2),
  AliasedPred =..[UnaryOp, AliasedPred2].  
  
/*

---- outerJoinExtend(+Rel, -ExtendAtom)
----

This predicate is true iff ExtendAtom unifies with an extend-Expression in SECONDO-Syntax which gives constant ~undefined~ for
every attribute defined in relation Rel.

*/
  
outerJoinExtend(Rel, ExtendAtom) :-
  ground(Rel),
%  dm(subqueryUnnesting, ['\nRel: ', Rel]),
  restrict(*, Rel, _),
%  dm(subqueryUnnesting, ['\nAttrList: ', AttrList]),
  secondoCatalogInfo(Rel, _, _, Type),
  Type = [[rel, [tuple, TypeList]]],
%  dm(subqueryUnnesting, ['\nTypeList: ', TypeList]),
  maplist(undefine, TypeList, ExtendAttrList),
%  dm(subqueryUnnesting, ['\nExtendAttrList: ', ExtendAttrList]),
  concat_atom(ExtendAttrList, ', ', CommaSeparatedAttrList),
%  dm(subqueryUnnesting, ['\nCommaSeparatedAttrList: ', CommaSeparatedAttrList]),
  concat_atom(['extend[', CommaSeparatedAttrList,']'], '', ExtendAtom).
%  dm(subqueryUnnesting, ['\nExtendAtom: ', ExtendAtom]).
  
  

/*

---- undefine(+[AttrName, type], -UndefinedAttr)
----

UndefinedAttr is a SECONDO-Attribute definition with constant value ~undefined~ for the corresponding SECONDO type ~type~

*/

undefine([AttrName, int], UndefinedAttr) :-
  concat_atom([AttrName, ': real2int(1/0)'], '', UndefinedAttr).
  
undefine([AttrName, int], UndefinedAttr) :-
  concat_atom([AttrName, ': 1/0'], '', UndefinedAttr).

undefine([AttrName, string], UndefinedAttr) :-
  concat_atom([AttrName, ': num2string(real2int(1/0))'], '', UndefinedAttr).  
  
undefine([AttrName, bool], UndefinedAttr) :-
  concat_atom([AttrName, ': real2int(int2bool(1/0))'], '', UndefinedAttr).

undefine([AttrName, point], UndefinedAttr) :-
  concat_atom([AttrName, ': makepoint(real2int(1/0), 1)'], '', UndefinedAttr).  

/* 

Restrict StarQueries to only select attributes of the given relations

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

---- alias(?Alias, ?Attr, ?Alias:Attr)
----

Helper predicate to alias/un-alias attribute names.

*/

alias(_, *, *).

alias(_, Const, Const) :-
  integer(Const).

alias(Alias, Attr, Alias:Attr).


/*
----  temporaryRelation(TempRelName)
----

Store all temporary Relations created by the evaluation of a subquery for deletion at halt.

---- temporaryRelation(TempRelName, RelQuery, Cost)
----
Store temporary relation for creation if the optimizer decides on an execution plan involving a temporary Relation.

*/

% :- at_halt(deleteTempRels).
% :- at_halt(dropTempRels).

:- dynamic temporaryRelation/1,
   temporaryRelation/2.
   
/*

Register deletion of created temporary relations.

*/

:- dynamic(relDefined/1).

newTempRel(Var) :-
  relDefined(N),
  !,
  N1 is N + 1,
  retract(relDefined(N)),
  assert(relDefined(N1)),
  atom_concat('txxrel', N1, Var).

newTempRel(Var) :-
  assert(relDefined(1)),
%  dropTempRels,
  Var = 'txxrel1'.
  
newTempRel(Query, TempRelName) :-  
  dm(subqueries, ['\nTempRelName: ', TempRelName, '\n']),
  ( temporaryRelation(TempRelName, Query)
    ; ( newTempRel(TempRelName),
	    assert(temporaryRelation(TempRelName, Query)), 
        createTempRel(TempRelName)
	   )
  ).

newTempRel_direct(Plan, TempRelName) :-
  newTempRel(TempRelName),
  dm(subqueries, ['\nTempRelName: ', TempRelName,  '\n']),
  concat_atom([TempRelName, ' = ', Plan], '', Query),
  let(Query),
  assert(temporaryRelation(TempRelName)).
   
createTempRel(TempRelName) :-
  ground(TempRelName),
  temporaryRelation(TempRelName, RelQuery),
  dm(subqueries, ['\nRelQuery: ', RelQuery]),
  let(TempRelName, RelQuery),
  retractall(temporaryRelation(TempRelName, _, _)),
  assert(temporaryRelation(TempRelName)). 

deleteTempRels :-
  findall(Rel, temporaryRelation(Rel), L),
  catch(deleteTempRels(L), _, true),
  retractall(temporaryRelation(_)).
  
deleteTempRels([]) :- 
  retractall(relDefined(_)), 
  retractall(temporaryRelation(_)),
  retractall(temporaryRelation(_, _)).

deleteTempRels([ Rel | Rest ]) :-
  dm(subqueries, ['\nDeleting temporary Relation ', Rel, ' ...']),
  catch(drop_relation(Rel), Ex, true),
  dm(subqueryUnnesting, [Ex]),
  deleteTempRels(Rest).
  
dropTempRels :-
  findall(Rel, ( databaseName(DB),
                 storedRel(DB, Rel, _), 
				 atom_concat('txxrel', _, Rel)
				 
				), RelList),
  not(RelList = []),
  catch(deleteTempRels(RelList), _, true),
  databaseName(DB),
  closeDB,
  updateDB(DB),
  open database DB.
  
/*

type-A nesting, uncorrelated subquery with scalar result

*/

nestingType(Subquery, a) :-
  aggrQuery(Subquery, _, _, _),
  catch(callLookup(Subquery, _), _, fail),
  dm(subqueryUnnesting, ['\nnesting Type-A']),
  !.

nestingType(Subquery, a) :-
  userDefAggrQuery(Subquery, _, _, _, _),
  catch(callLookup(Subquery, _), _, fail),
  dm(subqueryUnnesting, ['\nnesting Type-A']),
  !.
  
/*

type-JA nesting, correlated subquery with aggregation function, scalar result.

*/

nestingType(Subquery, ja) :-
  aggrQuery(Subquery, _, _, _),
  dm(subqueryUnnesting, ['\nnesting Type-JA']),
  !.

nestingType(Subquery, ja) :-
  userDefAggrQuery(Subquery, _, _, _, _),
  dm(subqueryUnnesting, ['\nnesting Type-JA']),
  !.  
  
/*

type-N nesting, uncorrelated subquery with row result

*/

nestingType(Subquery, n) :-
  not(userDefAggrQuery(Subquery , _, _, _, _)),
  not(aggrQuery(Subquery, _, _, _)),
  catch(callLookup(Subquery, _), _, fail), 
  dm(subqueryUnnesting, ['\nnesting Type-N']),
  !.  
  
/*

type-J nesting, correlated subquery without aggregation function, row result

*/

nestingType(Subquery, j) :-
  not(userDefAggrQuery(Subquery, _, _, _, _)), 
  not(aggrQuery(Subquery, _, _, _)),
  dm(subqueryUnnesting, ['\nnesting Type-J']),
  !.
  
nestingType(_, unknown) :-
  throw(error_SQL(subqueries_nestingType:unknownNesting)). 
  
/*:- multifile (=>)/2.

join(arg(N), arg(M), pr(OuterJoinPred, _, _)) => smouterjoin(Arg1S, Arg2S, attrname(X), attrname(Y)) :-
  write('\ntranslate: ', OuterJoinPred),
  arg(N) => Arg1S,
  write('\nargN: ', N),
  arg(M) => Arg2S,
  write('\nargM: ', M),
  OuterJoinPred =.. [outerjoin, X, Y],
  write('\nX: ', X),
  write('\nY: ', Y). 

:- multifile cost/4.
  
cost(smouterjoin(_, _, _, _), Sel, 1, 0).*/

%:- multifile lookup/2.

lookup1(select Attrs from Rels where Preds,
        select Attrs2 from Rels3List where Preds2List) :-   
  lookupRels(Rels, Rels2),
  lookupAttrs(Attrs, Attrs2),
  lookupSubquery(Preds, Preds2, SubqueryRels),
  makeList(Rels2, Rels2List),
  makeList(SubqueryRels, SubqueryRelsList),
  union(Rels2List, SubqueryRelsList, Rels3List),
  makeList(Preds2, Preds2List),
  (optimizerOption(entropy)
    -> registerSelfJoins(Preds2List, 1); true).
					% needed for entropy optimizer
					
lookupSubquery(Pred, Pred2, SubqueryRel) :-
  not(is_list(Pred)),
  isSubqueryPred1(Pred),
  lookupSubqueryPred(Pred, Pred2, SubqueryRel).					
					
lookupSubquery([Pred | Rest], [Pred2 | RestList], [SubqueryRel | SubqueryRels2]) :-
  isSubqueryPred1(Pred),
  lookupSubqueryPred(Pred, Pred2, SubqueryRel),
  lookupSubquery(Rest, RestList, SubqueryRels2).
  
  
% quantified predicate 
lookupSubqueryPred(SubqueryPred, pr(SubqueryPred2, RelsAfter, Rels2), Rels2) :-  
  dm(subqueryDebug, ['\nQuantifiedPred']),
  dm(subqueryDebug, ['SubqueryPred: ', SubqueryPred]),
  SubqueryPred =.. [Op, Attrs, SubqueryTemp],
  dm(subqueryDebug, ['Op: ', Op]),
  dm(subqueryDebug, ['Attrs: ', Attrs]),
  dm(subqueryDebug, ['SubqueryTemp: ', SubqueryTemp]),
  SubqueryTemp =.. [Quantifier, Subquery],
  dm(subqueryDebug, ['Quantifier: ', Quantifier]),
  dm(subqueryDebug, ['Subquery: ', Subquery]),
  lookup(Subquery, Subquery2),
  Subquery2 =.. [from, _, Rels2],
  dm(subqueryDebug, ['Rels2: ', Rels2]),
  Rels2 =.. [rel, _, _, _],
  lookupPred1(Attrs, Attrs2, [], [RelsAfter]),
  Subquery3 =.. [Quantifier, Subquery2],
  SubqueryPred2 =.. [Op, Attrs2, Subquery3].  

   
lookupSubqueryPred(SubqueryPred, pr(SubqueryPred2, RelsAfter, Rels2), Rels2) :-
  dm(subqueryDebug, ['SimplePred']),
  dm(subqueryDebug, ['SubqueryPredA: ', SubqueryPred]),
  SubqueryPred =.. [Op, Attrs, Subquery],
  dm(subqueryDebug, ['Op: ', Op]),
  dm(subqueryDebug, ['Attrs: ', Attrs]),  
  dm(subqueryDebug, ['Subquery: ', Subquery]),
  lookup(Subquery, Subquery2),
  dm(subqueryDebug, ['Subquery2: ', Subquery2]),
  Subquery2 =.. [from, _, Rels2],
  dm(subqueryDebug, ['Rels2: ', Rels2]),
  Rels2 =.. [rel, _, _],
  lookupPred1(Attrs, Attrs2, [], [RelsAfter]),
  SubqueryPred2 =.. [Op, Attrs2, Subquery2].
  
lookupSubqueryPred(SubqueryPred, pr(SubqueryPred2, RelsAfter, Rels2), Rels2) :-
  dm(subqueryDebug, ['SubqueryPredB: ', SubqueryPred]),
  SubqueryPred =.. [Op, Attrs, Subquery],
  dm(subqueryDebug, ['Op: ', Op]),
  dm(subqueryDebug, ['Attrs: ', Attrs]),  
  dm(subqueryDebug, ['Subquery: ', Subquery]),
  lookup(Subquery, Subquery2),
  dm(subqueryDebug, ['Subquery2: ', Subquery2]),
  Subquery2 =.. [from, _, Where],
  dm(subqueryDebug, ['Where: ', Where]), 
  Where =.. [where, [Rels2], _],
  dm(subqueryDebug, ['Rels2: ', Rels2]),
  Rels2 =.. [rel, _, _, _],
  lookupPred1(Attrs, Attrs2, [], [RelsAfter]),
  dm(subqueryDebug, ['Attrs2: ', Attrs2]),
  SubqueryPred2 =.. [Op, Attrs2, Subquery2].
%  dm(subqueryDebug, ['SubqueryPred2: ', SubqueryPred2]).   

% 
lookupSubqueryPred(SubqueryPred, pr(SubqueryPred2, Rels), Rels) :-
  SubqueryPred =.. [Op, Subquery],
  lookup(Subquery, Subquery2),
  Subquery2 =.. [from, _, Rels],
  SubqueryPred2 =.. [Op, Subquery2]. 
  
  
containsSubqueryPred(Pred) :-
  not(is_list(Pred)),
  isSubqueryPred1(Pred).
  
containsSubqueryPred([Pred | Rest]) :-
  isSubqueryPred1(Pred) ; 
  containsSubqueryPred(Rest).   

isSubqueryPred1(Pred) :- 
  compound(Pred),
  Pred =.. [Op, _, Subquery],
  isSubqueryOp(Op),
  Subquery =.. [from, _, _].
  
isSubqueryPred1(Pred) :- 
  compound(Pred),
  Pred =.. [Op, Subquery],
  isSubqueryOp(Op),
  Subquery =.. [from, _, _].  
  
isSubqueryPred1(Pred) :-
  compound(Pred),
  Pred =.. [Op, _, QuantifiedPred],
  QuantifiedPred =.. [Quantifier, Subquery],
  isComparisonOp(Op),
  isQuantifier(Quantifier),
  Subquery =.. [from, _, _].
  
/* isSubqueryOp(in).
isSubqueryOp(exists).
isSubqueryOp(Op) :-
  isComparisonOp(Op). 

isQuantifier(all).
isQuantifier(any).
isQuantifier(some). */

:- op(700, xfx, <>).

/* isComparisonOp(=).
isComparisonOp(<=).
isComparisonOp(=>).
isComparisonOp(<).
isComparisonOp(>).
isComparisonOp(<>). */


%:- multifile plan_to_atom/2.

subquery_to_atom(Pred, Result) :-
   dm(subqueryDebug, ['subquery_to_atom']),
   dm(subqueryDebug, ['Pred: ', Pred]),
   Pred =.. [in, Attrs, Subquery],
    dm(subqueryDebug, ['Attrs: ', Attrs]), 
    dm(subqueryDebug, ['Subquery: ', Subquery]),
   Subquery =.. [from, Select, _],
    dm(subqueryDebug, ['Select: ', Select]),
   Select =.. [select, Arg],
    dm(subqueryDebug, ['Arg: ', Arg]),
   Expr =.. [=, Arg, Attrs],
    dm(subqueryDebug, ['Expr: ', Expr]),
   plan_to_atom(Expr, Result),
    write(Result),
   !.  
   
subquery_to_atom(Pred, Result) :-
 %  dm(subqueryDebug, ['subquery_to_atom']),
   Pred =.. [Op, Attrs, QuantifiedSubquery],
 %  dm(subqueryDebug, ['Attrs: ', Attrs]), 
   QuantifiedSubquery =.. [_, Subquery],
 %  dm(subqueryDebug, ['Subquery: ', Subquery]),
   Subquery =.. [from, Select, _],
 %  dm(subqueryDebug, ['Select: ', Select]),
   Select =.. [select, Arg],
 %  dm(subqueryDebug, ['Arg: ', Arg]),
   Expr =.. [Op, Arg, Attrs],
 %  dm(subqueryDebug, ['Expr: ', Expr]),
   plan_to_atom(Expr, Result),
 %  write(Result),
   !.     
   
/*:- multifile newEdge/4.

newEdge(pr(P, R1, R2), PNo, Node, Edge) :-
  isSubqueryPred1(P),
  findRels(R1, R2, Node, Source, Arg1, Arg2),
  Target is Source + PNo,
  nodeNo(Arg1, Arg1No),
  nodeNo(Arg2, Arg2No),
  Result is Arg1No + Arg2No + PNo,
  Edge = edge(Source, Target, subquery(Arg1, Arg2, pr(P, R1, R2)), Result,
    Node, PNo).	   */
  

:- multifile (=>)/2.

% simple in predicate
subquery(Arg1, _, pr(Pred, _, _)) => remove(filter(product(Arg1S, feed(Rel)), Expr), AttrList2) :-
  isSubqueryPred1(Pred), 
%  dm(subqueryDebug, ['translateA: ', Pred]),
  Arg1 => Arg1S,
%  dm(subqueryDebug, ['Arg1: ', Arg1]),
  Pred =.. [in, Attrs, Subquery],
%  dm(subqueryDebug, ['Attrs: ', Attrs]),
  Subquery =.. [from, Select, Rel],
  not(is_list(Rel)),
%  dm(subqueryDebug, ['Rel: ', Rel]),
  Rel =.. [rel, Name, _, _],   
%  write(Name),
%  write(Var),
  relation(Name, AttrList),
%  write(AttrList),
%  write(AttrList2),
  myFun(AttrList, AttrList2, *),
  Select =.. [select, Arg],
  Expr =.. [=, Attrs, Arg].

% simple in predicate
subquery(Arg1, _, pr(Pred, _, _)) => remove(filter(product(Arg1S, feed(Rel)), Expr), AttrList2) :-
  isSubqueryPred1(Pred), 
%  dm(subqueryDebug, ['translateA: ', Pred]),
  Arg1 => Arg1S,
%  dm(subqueryDebug, ['Arg1: ', Arg1]),
  Pred =.. [in, Attrs, Subquery],
%  dm(subqueryDebug, ['Attrs: ', Attrs]),
  Subquery =.. [from, Select, Rel],
  not(is_list(Rel)),
%  dm(subqueryDebug, ['Rel: ', Rel]),
  Rel =.. [rel, Name, Var, _],
%  write(Name),
%  write(Var),
  relation(Name, AttrList),
%  write(AttrList),
  myFun(AttrList, AttrList2, Var),
%  write(AttrList2),
  Select =.. [select, Arg],
  Expr =.. [=, Attrs, Arg].
  
myFun([], [], _).

myFun([Attr | Rest], AttrList2, Var) :-
  lookupAttr(Var:Attr, Attr2),
  myFun(Rest, Rest2, Var),
  flatten([attrname(Attr2), Rest2], AttrList2).
  
% in predicate with predicate
subquery(Arg1, _, pr(Pred, _, _)) => filter(product(Arg1S, filter(feed(Rel), SubPred)), Expr) :-
  isSubqueryPred1(Pred), 
%  dm(subqueryDebug, ['translateB: ', Pred]),
  Arg1 => Arg1S,
%  dm(subqueryDebug, ['Arg1: ', Arg1]),
  Pred =.. [in, Attrs, Subquery],
%  dm(subqueryDebug, ['Attrs: ', Attrs]),
%  dm(subqueryDebug, ['Subquery: ', Subquery]),
  Subquery =.. [from, Select, Where],
%  dm(subqueryDebug, ['Select: ', Select]),
%  dm(subqueryDebug, ['Where: ', Where]),
  Where =.. [where, [Rel], [pr(SubPred, _)]],
%  dm(subqueryDebug, ['SubPred: ', SubPred]),
%  dm(subqueryDebug, ['Rel: ', Rel]),
%  SubPred => SubPred2,
  Select =.. [select, Arg],
  Expr =.. [=, Arg, Attrs]. 
  
  
subquery(Arg1, _, pr(Pred, _, _)) => remove(filter(extend(product(Arg1S, feed(Rels)), [newattr(attrname(attr(subext, 1, l)), Expr)]), attr(subext, 1, l) = true), attrname(attr(subext, 1, l)))  :-
  isSubqueryPred1(Pred),
%  dm(subqueryDebug, ['translateC: ', Pred]),  
%  write(Pred),
  Arg1 => Arg1S,
  Pred =.. [Op, Attrs, QuantifiedSubquery],
%  dm(subqueryDebug, ['Op: ', Op]),
%  dm(subqueryDebug, ['Attrs: ', Attrs]),
%  dm(subqueryDebug, ['QuantifiedSubquery: ', QuantifiedSubquery]),
  QuantifiedSubquery =.. [_, Subquery],
%  dm(subqueryDebug, ['Quantifier: ', Quantifier]),
%  dm(subqueryDebug, ['Subquery: ', Subquery]),
  Subquery =.. [from, Select, Rels],
%  dm(subqueryDebug, ['Select: ', Select]),
%  dm(subqueryDebug, ['Rels: ', Rels]),
  Select =.. [select, Arg],
%  dm(subqueryDebug, ['Arg: ', Arg]),
%  dm(subqueryDebug, ['Op: ', Op]),
  Expr =.. [Op, Arg, Attrs].
%  dm(subqueryDebug, ['Expr: ', Expr]),  

  
/*  
subquery(Arg1, _, pr(Pred, _, _)) => loopsel(feed(Rels), fun([param(T1, tuple)], filter(Arg1S, Expr))) :-
  fail,
  isSubqueryPred(Pred), !,
  Arg1 => Arg1S,
  Pred =.. [in, Attrs, Subquery],
  Subquery =.. [from, Select, Rels],
  Select =.. [select, Arg],
  Expr =.. [=, attribute(T1, attrname(Arg)), Attrs],
  newVariable(T1).    
  
subquery(Arg1, _, pr(Pred, _, _)) => loopsel(feed(Rels), fun([param(T1, tuple)], filter(Arg1S, Expr))) :-
  isSubqueryPred(Pred), !,
%  write(Pred),
  Arg1 => Arg1S,
  Pred =.. [Op, Attrs, QuantifiedSubquery],
  Op => Op2,
  QuantifiedSubquery =.. [Quantifier, Subquery],
  Subquery =.. [from, Select, Rels],
  Select =.. [select, Arg],
  Expr =.. [Op2, attribute(T1, attrname(Arg)), Attrs],
  newVariable(T1).
*/  


/*
join(arg(N), arg(M), pr(OuterJoinPred, _, _)) => mergeunion(
												mergeunion(
													sort(product(
															mergediff(
																rdup(sort(remove(
																	hashjoin(Arg1S, Arg2S, 
																			 attrname(Attr1), attrname(Attr2), 99997),
																	Arg1Attrs
																))),
																sort(Arg2S)
															),
															rename(feed(rel(Arg1UNDEF, Var1)), Var1)
													)),
													sort(product(
															rename(feed(rel(Arg2UNDEF, Var2)), Var2),
															mergediff(
																rdup(sort(remove(
																	hashjoin(Arg1S, Arg2S, 
																			 attrname(Attr1), attrname(Attr2), 99997),
																	Arg2Attrs
																))),
																sort(Arg1S)
															)														
													))
												),
												sort(hashjoin(Arg1S, Arg2S, Attr1, Attr2, 99997))) :-
																
																		
  OuterJoinPred =.. [outerjoin, Pred],
  write('\nPred: ', Pred),
  Pred =.. [=, X, Y],
  write('\nX: ', X),
  write('\nY: ', Y),
  arg(N) => Arg1S,
  write('\nN: ', N),
  arg(M) => Arg2S,
  write('\nM: ', M),
  isOfFirst(Attr1, X, Y),
  write('\nAttr1: ', Attr1),
  isOfSecond(Attr2, X, Y),
  write('\nAttr2: ', Attr2),
  argument(N, Rel1),
  write('\nRel1: ', Rel1),
  argument(M, Rel2),
  write('\nRel2: ', Rel2),
  Rel1 = rel(RelName1, Var1),
  write('\nRelName1: ', RelName1),
  write('\nVar1: ', Var1),
  Rel2 = rel(RelName2, Var2),
  write('\nRelName2: ', RelName2),
  write('\nVar2: ', Var2),  
  concat_atom([RelName1, 'undef'], '', Arg1UNDEF),
  write('\nArg1UNDEF: ', Arg1UNDEF),  
  concat_atom([RelName2, 'undef'], '', Arg2UNDEF),
  write('\nArg2UNDEF: ', Arg2UNDEF),
  usedAttrList(Rel1, Arg1Attrs), % todo Attributliste korrekt erzeugen auch bei Kostenfunktion berücksichtigen
  write('\nArg1Atttrs: ', Arg1Attrs),
  usedAttrList(Rel2, Arg2Attrs),
  write('\nArg2Attrs: ', Arg2Attrs).
 
/ * 
cost(mergeunion(Stream1, Stream2), 0, Size, Cost) :-
  cost(Stream1, _, Size1, Cost1),
  cost(Stream2, _, Size2, Cost2),
  Size is Size1 + Size2,
  Cost is Cost1 + Cost2.
  
cost(mergediff(Stream1, Stream2), 0, Size, Cost) :-
  cost(Stream1, _, Size1, Cost1),
  cost(Stream2, _, Size2, Cost2),
  Size is Size1 + Size2,
  Cost is Cost1 + Cost2.* /
  
cost(mergeunion(mergeunion(sort(product(mergediff(rdup(sort(remove(hashjoin(rename(feed(rel(Rel1, Var1)), Var1), rename(feed(rel(Rel2, Var2)), Var2), attrname(attr(Var1:Attr1,1, u)), attrname(attr(Var2:Attr2, 2, u)), 99997), [attrname(attr(ort, 0, u))]))),sort(rename(feed(rel(Rel2, Var2)), Var2))), rename(feed(rel(Rel1undef, Var1)), Var1))), sort(product(rename(feed(rel(Rel2undef, Var2)), Var2), mergediff(rdup(sort(remove(hashjoin(rename(feed(rel(Rel1, Var1)), Var1), rename(feed(rel(Rel2, Var2)), Var2), attrname(attr(Var1:Attr1,1,u)), attrname(attr(Var2:Attr2, 2,u)), 99997), [attrname(attr(ort, 0, u))]))), sort(rename(feed(rel(Rel1, Var1)),Var1)))))), sort(hashjoin(rename(feed(rel(Rel1, Var1)), Var1), rename(feed(rel(Rel2, Var2)), Var2), attr(Var1:Attr1,1, u), attr(Var2:Attr2, 2, u),99997))), 4e-06, Size, 0).  */
  
  
/*

for Debugging purposes

*/

partition(Pred, List, Included, Excluded) :-
	partition_(List, Pred, Included, Excluded).

partition_([], _, [], []).
partition_([H|T], Pred, Incl, Excl) :-
	(   call(Pred, H)
	->  Incl = [H|I],
	    partition_(T, Pred, I, Excl)
	;   Excl = [H|E],
	    partition_(T, Pred, Incl, E)
	).

:- [subquerytest].
:- [tpcdqueries].

/*
isNestedQuery(select[supp_nation, cust_nation, lyear, sum(volume)as revenue]
from (
select[n1:nname as supp_nation, n2:nname as cust_nation, year_of(lshipdate)as lyear, lextendedprice* (1-ldiscount)as volume]
from[supplier, lineitem, orders, customer, nation as n1, nation as n2]
where[ssuppkey=lsuppkey, oorderkey=lorderkey, ccustkey=ocustkey, snationkey=n1:nnationkey, cnationkey=n2:nnationkey, (n1:nname=[70, 82, 65, 78, 67, 69]and n2:nname=[71, 69, 82, 77, 65, 78, 89])or (n1:nname=[71, 69, 82, 77, 65, 78, 89]and n2:nname=[70, 82, 65, 78, 67, 69]), between(instant2real(lshipdate), instant2real(instant([49, 57, 57, 53, 45, 48, 49, 45, 48, 49])), instant2real(instant([49,57, 57, 54, 45, 49, 50, 45, 51, 49])))])
as shipping groupby[supp_nation, cust_nation, lyear]orderby[supp_nation, cust_nation, lyear])

transform(select[sum(lextendedprice/7.0)as avg_yearly]from[lineitem, part]where[ppartkey=lpartkey, pbrand=[66, 114, 97, 110, 100, 35, 50, 51], pcontainer=[77,69, 68, 32, 66, 79, 88], lquantity< (select[0.2*avg(lquantity)]from[lineitem]where[lpartkey=ppartkey])], A)
*/
