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
  catch(callLookup(Query, _), error_SQL(optimizer_lookupPred1(Term, Term):unknownError), true).
  
  
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

Special handling for operator ~in~ which can be replaced by ~=~ as join operator, when a subquery is transformed 
to its canonical form.

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
  
transform(Query, CanonicalQuery) :-
  fail,
  queryObject(Query, QueryObject),
  write('\nQueryObject: '), write(QueryObject),
  transformNestedAttributes(QueryObject, QueryObject2),
  transformNestedRelations(QueryObject2, QueryObject3),
  transformNestedPredicates(QueryObject3, CanonicalQueryObject),
  queryObject(CanonicalQuery, CanonicalQueryObject).  
	
queryObject(Query, qry(Attrs, Rels, Preds, OrderBy, GroupBy, FirstClause)) :-
  Query =.. [Op, QueryRest, Count],
  (Op = first ; Op = last),
  queryObject(QueryRest, qry(Attrs, Rels, Preds, OrderBy, GroupBy, [])),
  FirstClause =.. [Op, Count].
  
queryObject(Query, qry(Attrs, Rels, Preds, OrderBy, GroupBy, [])) :-
  Query =.. [groupby, QueryRest, GroupBy],
  queryObject(QueryRest, qry(Attrs, Rels, Preds, OrderBy, [], [])).
  
queryObject(Query, qry(Attrs, Rels, Preds, OrderBy, [], [])) :-
  Query =.. [orderby, QueryRest, OrderBy],
  queryObject(QueryRest, qry(Attrs, Rels, Preds, [], [], [])).
  
queryObject(Query, qry(Attrs, Rels, Preds, [], [], [])) :-  
  Query =.. [from, Select, Where],
  Select =.. [select, Attrs],
  Where =.. [where, Rels, Preds].
  
queryObject(Query, qry(Attrs, Rels, [], [], [], [])) :-
  Query =.. [from, Select, Rels],
  Select =.. [select, Attrs].
  
/*

Subqueries in the attribute list have to yield a scalar result. If the nested query is uncorrelated with the outer query,
it can be evaluated independently of it and replaced by its result.

*/
  
transformNestedAttributes([], [], Rels, Rels, Preds, Preds).
  
transformNestedAttributes(Attr, Attr2, Rels, Rels2, Preds, Preds2) :-
  not(is_list(Attr)),
  transformNestedAttribute(Attr, Attr2, Rels, Rels2, Preds, Preds2).
  
transformNestedAttributes([ Attr | Rest ], [ Attr2 | Rest2 ], Rels, Rels2, Preds, Preds2) :-
  transformNestedAttribute(Attr, Attr2, Rels, Rels2, Preds, Preds2),
  transformNestedAttributes(Rest, Rest2, Rels, Rels2, Preds, Preds2).

transformNestedAttributes(qry(Attrs, Rels, Preds, _, _, _), QueryObject2) :-
  transformNestedAttributes1(Attrs, qry(Attrs, Rels, Preds, _, _, _), QueryObject2).
  
transformNestedAttributes1([], QueryObject, QueryObject).
  
transformNestedAttributes1(Attr, QueryObject, QueryObject2) :-
  not(is_list(Attr)),
  transformNestedAttribute(Attr, QueryObject, QueryObject2).
  
transformNestedAttributes1([ Attr | Rest ], QueryObject, QueryObject3) :-
  transformNestedAttribute(Attr, QueryObject, QueryObject2),
  transformNestedAttributes1(Rest, QueryObject2, QueryObject3).

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

Handling of nested queries in the from clause. No transformation is applied at the moment.

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
  transform(Subquery, CanonicalSubquery),
  nestingType(CanonicalSubquery, a),
  optimize(CanonicalSubquery, SecondoQuery, _),
  atom(SecondoQuery),
  atom_concat('query ', SecondoQuery, QueryText),
  secondo(QueryText, [_, Res]),
  not(is_list(Res)),
  SubqueryPred2 =.. [Op, Attr, Res].  
 
/*

The subquery in this predicate does not contain a join predicate that references the relation of the outer query block
and does not have an aggregation function associated with the column name. Implements algorithm NEST-N-J
(1) Combine the from-clause of subquery and outer query into one from-clause
(2) Build the conjunction of the where-clauses of inner and outer query into one where-clause
(3) Replace the subquery predicate by a corresponding join predicate
(4) Retain the select-clause of the outer query

*/
  
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, Pred2) :-
  Pred =.. [Op, Attr, Subquery],
  transform(Subquery, CanonicalSubquery),
  (nestingType(CanonicalSubquery, n) ; nestingType(CanonicalSubquery, j)),
  write('\nNEST-N-J'),
  CanonicalSubquery =.. [from, Select, Where],
  Select =.. [select, CanonicalAttr],    
  Where =.. [where | [ CanonicalRels | CanonicalPreds ]],
  restrict(Attrs, Rels, Attrs2),
  makeList(Rels, RelsList),
  makeList(CanonicalRels, CanonicalRelsList),
  append(RelsList, CanonicalRelsList, Rels2),
  subqueryToJoinOp(Op, NewOp),
  JoinPredicate =.. [NewOp, Attr, CanonicalAttr],
  append([JoinPredicate], CanonicalPreds, Pred2).
  
/*

This predicate handles correlated predicates with aggregation function associated with their single column attrlist.
Implements Algorithm NEST-JA (which does not work correctly for some queries)

TODO: Rewrite to Algorithm NEST-JA2 as proposed by Ganski, Wong
Problems: Secondo does not support outer joins, which are needed for that algorithm
(1) Generate temporary relation from subquery such that each Cn+1 column value of the temporary relation is a constant obtained by 
applying the aggregation function on the column
(2) Transform the subquery by changing references to the aggregation column to this new column in the temporary relation. The
resulting query is of type-J, since the aggregation function has been replaced.

*/
  
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, Pred2) :-
  Pred =.. [Op, Attr, Subquery],
  transform(Subquery, CanonicalSubquery),
  nestingType(CanonicalSubquery, ja),
  write('\nNEST-JA'),
  CanonicalSubquery =.. [from, Select, Where],
  Select =.. [select, AggregatedAttr],
  write('\nAggregatedAttr: '), write(AggregatedAttr),
  AggregatedAttr =.. [AggrOp | [CanonicalAttr]],
  write('\nCanonicalAttr: '), write(CanonicalAttr),
  Where =.. [where | [CanonicalRels | CanonicalPreds]],
  write('\nCanonicalRels: '), write(CanonicalRels),
  write('\nCanonicalPreds: '), write(CanonicalPreds),
  restrict(*, CanonicalRels, CanonicalAttrList),
  partition(=(CanonicalAttr), CanonicalAttrList, _, GroupAttrs),
  write('\nGroupAttrs: '), write(GroupAttrs),
  newVariable(NewColumn),  
  write('\nNewColumn: '), write(NewColumn),
  flatten([GroupAttrs | AggregatedAttr as NewColumn], ProjectionAttrs),
  ( GroupAttrs = []
      -> TemporaryRel =.. [from, select AggregatedAttr as NewColumn, CanonicalRels]
	  ;  TemporaryRel =.. [groupby, from(select ProjectionAttrs, CanonicalRels), GroupAttrs] ),
  write('\nTemporaryRel: '), write(TemporaryRel), 
  optimize(TemporaryRel, RelQuery, Cost),
  newVariable(TempRel),
  assert(temporaryRelation(TempRel, RelQuery, Cost)),
  TempPred =.. [Op, Attr, from(select NewColumn, where(TempRel, CanonicalPreds))],
  write('\nPred2: '), write(TempPred),
%  createTempRel(TempRel),
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, TempPred, Pred2).  
  
/* 

Implements Algorithm NEST-JA2 as designed by Ganski, Wong. 
(1) Project the join column of the outer relation and restrict it with any simple predicates applying to the outer relation
(2) Create a temporary relation, joining the inner relation with the projection of the outer relation. If the aggragate function
is ~COUNT~, the join must be an outer join, and the inner relation must be restricted and projected before the join is performed.
If the aggregate function is ~COUNT(*)~, compute the ~COUNT~ function over the join column. The join predicate must use the same operator
as the join predicate in the original query (except that it must be converted to the corresponding outer operator in case of
~COUNT~), and the join predicate in the original query must be changed to ~=~. In the select clause, select the join column from
the outer table int the join predicate instead of the inner table. The groupby clause will also contain columns from the outer
relation 
(3) Join the outer relation with  the temporary relation, according to the transformed version of the original query

As SECONDO does not implement outer joins we have to simulate them. Left outer join can be simulated by a union of an inner join to <NewJoinCol> and the following query:
query <Rel1> feed extend[<NewJoinCol>: <correctly typed undef>] filter [fun(var1: TUPLE) <Rel2> feed filter [not(.<JoinAttr> = attr(var1, <JoinAttr>))] count = <Rel2> feed count] consume
Only join columns which can be transformed to value "UNDEFINED" are supported.
Simple transformations from (real 0/1) exist for the following types:
real, int, bool, string and point.

*/
  
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, Pred2) :-
  Pred =.. [Op, Attr, Subquery],
  transform(Subquery, CanonicalSubquery),
  nestingType(CanonicalSubquery, ja),
  write('\nNEST-JA2'),  
  CanonicalSubquery =.. [from, Select, Where],
  Select =.. [select, AggregatedAttr],
  write('\nAggregatedAttr: '), write(AggregatedAttr),
  AggregatedAttr =.. [AggrOp | [CanonAttr]],
  write('\nCanonAttr: '), write(CanonAttr),
  Where =.. [where | [CanonicalRels | CanonicalPreds]],
  write('\nCanonicalRels: '), write(CanonicalRels),
  write('\nCanonicalPreds: '), write(CanonicalPreds),
  restrict(*, CanonicalRels, AggregatedAttrList),
  partition(=(CanonAttr), AggregatedAttrList, _, GroupAttrs),
  write('\nGroupAttrs: '), write(GroupAttrs),
  newVariable(NewColumn),  
  write('\nNewColumn: '), write(NewColumn),
  flatten([GroupAttrs | AggregatedAttr as NewColumn], ProjectionAttrs),
  ( GroupAttrs = []
      -> TemporaryRel =.. [from, select AggregatedAttr as NewColumn, CanonicalRels]
	  ;  TemporaryRel =.. [groupby, from(select ProjectionAttrs, CanonicalRels), GroupAttrs] ),
  write('\nTemporaryRel: '), write(TemporaryRel), 
  optimize(TemporaryRel, RelQuery, Cost),
  newVariable(TempRel),
  assert(temporaryRelation(TempRel, RelQuery, Cost)),
  TempPred =.. [Op, Attr, from(select NewColumn, where(TempRel, CanonicalPreds))],
  write('\nPred2: '), write(TempPred),
%  createTempRel(TempRel),
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, TempPred, Pred2).   
  

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

---- alias(?Alias, ?Attr, ?Alias:Attr).
----

Helper predicate to alias/un-alias attribute names.

*/

alias(Alias, Attr, Alias:Attr).


/*
----  temporaryRelation(TempRelName)

----

Store all temporary Relations created by the evaluation of a subquery for deletion at halt.

---- temporaryRelation(TempRelName, RelQuery, Cost)

----
Store temporary relation for creation if the optimizer decides on an execution plan involving a temporary Relation.

*/

:- dynamic temporaryRelation/1,
   temporaryRelation/3.
   
/*

Register deletion of created temporary relations.

*/
   
:- retractall(temporaryRelation(_)),   
   at_halt(deleteTempRels).
   
createTempRel(TempRelName) :-
  ground(TempRelName),
  temporaryRelation(TempRelName, RelQuery, _),
  atom(RelQuery),  
  concat_atom(['derive ', TempRelName, ' = ', RelQuery], '', RelQuery2),  
  secondo(RelQuery2),
  retractall(temporaryRelation(TempRelName, _, _)),
  assert(temporaryRelation(TempRelName)). 

deleteTempRels :-
  findall(Rel, temporaryRelation(Rel), L),
  deleteTempRels(L),
  retractall(temporaryRelation(_)).
  
deleteTempRels([]).

deleteTempRels([ Rel | Rest ]) :-
  write('\nDeleting temporary Relation '), write(Rel), write(' ...'),
  concat_atom(['delete ', Rel], '', DeleteQuery),
  catch(secondo(DeleteQuery), sql_ERROR(_), true),
  deleteTempRels(Rest).
  
  
/*

type-A nesting, uncorrelated subquery with scalar result

*/

nestingType(Subquery, a) :-
  aggrQuery(Subquery, _, _, _),
  catch(callLookup(Subquery, _), error_SQL(optimizer_lookupPred1(Term, Term):unknownError), fail),
  write('\nnesting Type-A'),
  !.
  
/*

type-N nesting, uncorrelated subquery with row result

*/

nestingType(Subquery, n) :-
  not(aggrQuery(Subquery , _, _, _)),
  catch(callLookup(Subquery, _), error_SQL(optimizer_lookupPred1(Term, Term):unknownError), fail), 
  write('\nnesting Type-N'),
  !.
  
  
/*

type-J nesting, correlated subquery without aggregation function, row result

*/

nestingType(Subquery, j) :-
  not(aggrQuery(Subquery, _, _, _)), 
  write('\nnesting Type-J').
  
  
/*

type-JA nesting, correlated subquery with aggregation function, scalar result.

*/

nestingType(Subquery, ja) :-
  aggrQuery(Subquery, _, _, _),
  write('\nnesting Type-JA').
  
  
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