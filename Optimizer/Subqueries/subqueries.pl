/*

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

August 2008, Burkart Poneleit. Initial Version

18.1 Transforming nested queries to their canonical form

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

Some operator definitions

*/
:- op(700, xfx, <>).
%:- op(700, xfx, =+). % outer join operator
:- op(940, xfx, in).
:- op(941, xfx, not).
:- op(799,  fy, left).
:- op(799,  fy, outerjoin).
:- op(799,  fy, right).

/*

---- isQuery(+Term)
----

This predicate is true, if the term is structurally a query formulated
in the sql syntax defined for secondo. Exceptions are ignored, if they
are caused by attribute references to attributes of relations not included
in the FROM-clause. This is needed to correctly handle subqueries with
correlated predicates.

*/

/*
NVK ADDED NR
Not anymore intended for top-level query lookup. See optimizer.pl\#callLookup for further information. This exception thing is really annoying during debugging, the exceptions are raised during the query rewriting phase, because this happens before the lookup phase, so there are no outer variables etc. known. To fix this, i added some more predicates to identify subqueries directly without to call the lookup, but it is still a workaround. This is based on the lookup(Query, \_) predicates within the optimizer.pl.
 
*/
isQuery(Query) :-
  optimizerOption(nestedRelations),
  !,
  simplifiedIsQuery(Query).
% NVK ADDED NR END

isQuery(Query) :-
  \+ optimizerOption(nestedRelations),
  catch(callLookup(Query, _),
  error_SQL(optimizer_lookupPred1(T, T)::unknownIdentifier::_),
  true).

isQuery(Query) :-
  \+ optimizerOption(nestedRelations),
  catch(callLookup(Query, _),
  error_SQL(optimizer_lookupPred(T, T)::malformedExpression::_),
  true).


/*

Predicates to recognize nested queries. It looks for a nested query in
the select-, from- and where clause.

NVK NOTE: This is used nowhere.

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

NVK NOTE: This is used nowhere.

*/

isSubqueryAttr(Attr as _) :-
  isQuery(Attr).

isSubqueryAttr(Attr) :-
  isQuery(Attr).

/*

Predicates to recognize subqueries in predicate lists and predicates.

*/

% case negated predicates
isSubqueryPred(not(Pred)) :-
  isSubqueryPred(Pred).

% case ~not in~
isSubqueryPred(Pred) :-
  Pred =.. [not, Attr, in(Query)],
  isSubqueryPred(Attr in(Query)).

% case simple nested predicate, binary operator
isSubqueryPred(Pred) :-
  compound(Pred),
  Pred =.. [Op, _, Subquery],
  isSubqueryOp(Op),
  isQuery(Subquery).

% case simple nested predicate, unary operator
isSubqueryPred(Pred) :-
  compound(Pred),
  Pred =.. [Op, Subquery],
  isSubqueryOp(Op),
  isQuery(Subquery).

% case quantified subquery
isSubqueryPred(Pred) :-
  compound(Pred),
  Pred =.. [Op, _, QuantifiedPred],
  QuantifiedPred =.. [Quantifier, Subquery],
  isComparisonOp(Op),
  isQuantifier(Quantifier),
  isQuery(Subquery).

% definition of subquery operators
isSubqueryOp(in).
isSubqueryOp(exists).
isSubqueryOp(Op) :-
  isComparisonOp(Op).

% definition of quantifiers
isQuantifier(all).
isQuantifier(any).
/*
NVK NOTE: no preTransformNestedPredicate predicates are defined for this, so far as i can see, this ~some~ quantifier isn't working at all.

*/
%isQuantifier(some).


/*

Special handling for operator ~in~ which can be replaced by ~=~ as join operator,
when a subquery is transformed to its canonical form.

*/

subqueryToJoinOp(in, =).
subqueryToJoinOp(Op, Op).

/*

Quantified predicates are transformed into an equivalent form based on aggregated queries.
The following predicates map the comparison operator to the specific aggregation operator
needed in the transformed query.

*/

anyToAggr(<, max).
anyToAggr(<=, max).
anyToAggr(>, min).
anyToAggr(>=, min).

allToAggr(<, min).
allToAggr(<=, min).
allToAggr(>, max).
allToAggr(>=, max).

/*

Set containment operator ~in~ is translated with a ~set~ and
corresponding ~in~ operator of the collection algebra.

*/

constantType(attr(Var:Attr, _, _), Type) :-
  isAttributeOf(Var:Attr, Rel as Var),
  dcName2internalName(AttrName, Attr),
  attrType(Rel:AttrName, Type).

constantType(attr(Attr,_ , _), Type) :-
  isAttributeOf(Attr, Rel),
  dcName2internalName(AttrName, Attr),
  attrType(Rel:AttrName, Type).

constantType(attr2(Var:Attr, _, _), Type) :-
  isAttributeOf(Var:Attr, Rel as Var),
  dcName2internalName(AttrName, Attr),
  attrType(Rel:AttrName, Type).

constantType(attr2(Attr,_ , _), Type) :-
  isAttributeOf(Attr, Rel),
  dcName2internalName(AttrName, Attr),
  attrType(Rel:AttrName, Type).

constantType([Arg | Rest], Type) :-
  not(Rest = []),
  (constantType(Arg, Type) ; constantType(Rest, Type)).

constantType([Arg], Type) :-
  not(is_list(Arg)),
	constantType(Arg, Type).

constantType(AttrExpr, Type) :-
  not(is_list(AttrExpr)),
  AttrExpr =.. [_ | Args],
  constantType(Args, Type).

clearSubqueryHandling :-
  clear(streamRel), clear(streamName),
  retractall(currentRels(_)),
  retractall(sampleSize(_)), retractall(isJoinPred(_)).

/*

Predicates for Type-A Queries. Evaluate the subquery and return the result.

*/

evaluateSubquery(CanonicalSubquery, Result) :-
  nestingType(CanonicalSubquery, a),
  optimize(CanonicalSubquery, SecondoQuery, _),
  atom(SecondoQuery),
  atom_concat('query ', SecondoQuery, QueryText),
  secondo(QueryText, [_, Res]),
  not(is_list(Res)),
  typeConversion(Res, Result).

/*

Convert the result to a datatype which can be used as a constant in the resulting plan.
Currently only conversion string constants into a list of characters is needed.

*/

typeConversion(Data, Result) :-
  string_concat('"', Temp, Data),
  string_concat(TempRes, '"', Temp),
  string_to_list(TempRes, Result).

typeConversion(Result, Result).

/*
The following syntax for subqueries will recognized by SECONDO:

~in~

----  select <attr-list>
      from <rel-list>
      where attr in (<Subquery>)
----

~not in~

----  select <attr-list>
      from <rel-list>
      where attr not in(<Subquery>)
----

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

18.1.1 Rewriting of Subqueries

---- rewriteQueryForSubqueryProcessing(+QueryIn,-QueryOut)

----

Rewrites a query by transforming subqueries within ~QueryIn~ to their canonical form and unifies rewritten query with ~QueryOut~.

*/

:- dynamic(rewriteCache/2).

% all options deactivated, no subquery handling!
rewriteQueryForSubqueryProcessing(QueryIn, QueryIn) :-
  not(optimizerOption(subqueries)), !.

% only translate subqueries, no unnesting
rewriteQueryForSubqueryProcessing(QueryIn, QueryOut) :-
  not(optimizerOption(subqueryUnnesting)),
  preTransform(QueryIn, QueryOut),
  dm(subqueryUnnesting,['\nREWRITING: Subqueries\n\tIn:  ',
          QueryIn,'\n\tOut: ',
                    QueryOut,'\n\n']),
  clearSubqueryHandling,
  !.

% use query cache to find rewritten query
rewriteQueryForSubqueryProcessing(NestedQuery, CanonicalQuery) :-
    rewriteCache(NestedQuery, CanonicalQuery),
    !,
    dm(subqueryUnnesting,['\nREWRITING: Subqueries\n\tIn:  ',
          NestedQuery,'\n\tOut: ',
                    CanonicalQuery,'\n\n']).

% nothing found in query cache, apply transformations
rewriteQueryForSubqueryProcessing(NestedQuery, CanonicalQuery) :-
    transform(NestedQuery, CanonicalQuery),
    retractall(rewriteCache(NestedQuery, _)),
    assert(rewriteCache(NestedQuery, CanonicalQuery)),
    dm(subqueryUnnesting,['\nREWRITING: Subqueries\n\tIn:  ',
          NestedQuery,'\n\tOut: ',
                    CanonicalQuery,'\n\n']).


/*

Apply Pre-Transformations, like replacing operators ~any~, ~all~ and ~exists~ with their generic transformation

*/


preTransform(Query orderby Order, Query2 orderby Order) :-
  preTransform(Query, Query2).

preTransform(Query groupby Group, Query2 groupby Group) :-
  preTransform(Query, Query2).

preTransform(Query first N, Query2 first N) :-
  preTransform(Query, Query2).

preTransform(Query last N, Query2 last N) :-
  preTransform(Query, Query2).

preTransform(select Attrs from Rels where Preds,
       select CanonicalAttrs
       from CanonicalRels
       where CanonicalPreds) :-
  preTransformNestedPredicates(Attrs, CanonicalAttrs, Rels,
  CanonicalRels, Preds, CanonicalPredList),
  flatten(CanonicalPredList, CanonicalPreds).

preTransform(Query, Query).

preTransformNestedPredicates(Attrs, Attrs, Rels, Rels, [], []).

preTransformNestedPredicates(Attrs, Attrs2,
  Rels, Rels2,
  Pred, Pred2) :-
  not(is_list(Pred)),
  preTransformNestedPredicate(Attrs, Attrs2,
  Rels, Rels2,
  Pred, Pred2).

preTransformNestedPredicates(Attrs, Attrs2, Rels, Rels3,
  [ Pred | Rest ],
  [ Pred2 | Rest2 ]) :-
  preTransformNestedPredicate(Attrs, Attrs2,
  Rels, Rels2,
  Pred, Pred2),
  dc(subqueryUnnesting,
  ( not(Pred = Pred2)
    -> dm(subqueryUnnesting, ['\nPredicate: ',
      Pred,
      '\nTransformedPredicate: ',
      Pred2])
    ; true
  )),
  preTransformNestedPredicates(Attrs, Attrs2,
  Rels2, Rels3,
  Rest, Rest2).

preTransformNestedPredicate(Attrs, Attrs, Rels, Rels, Pred, Pred) :-
  not(isSubqueryPred(Pred)).

/*

The SQL predicates EXISTS, NOT EXISTS, ALL and ANY are transformed to a canonical form, which can be further unnested.

*/

% case operator ~exists~
preTransformNestedPredicate(Attrs, Attrs, Rels, Rels,
  exists(select SubAttr from RelsWhere),
  0 < (select count(SubAttr) from RelsWhere)).

% case operator ~not exists~
preTransformNestedPredicate(Attrs, Attrs, Rels, Rels,
  not(exists(select SubAttr from RelsWhere)),
  0 = (select count(SubAttr) from RelsWhere)).

% case operator ~any~
preTransformNestedPredicate(Attrs, Attrs, Rels, Rels, Pred, NewPred) :-
  Pred =.. [Op, Attr, any(select SubAttr from RelsWhere)],
  anyToAggr(Op, Aggr),
  AggrExpression =.. [Aggr, SubAttr],
  NewPred =.. [Op, Attr, select AggrExpression from RelsWhere].

% special handling for ~= any(Query)~
preTransformNestedPredicate(Attrs, Attrs, Rels, Rels,
  Attr = any(Query), Attr in(Query)).

% special handling for ~<> any(Query)~
preTransformNestedPredicate(Attrs, Attrs, Rels, Rels,
  Attr <> any(Query), Attr not in(Query)).

% case operator ~all~
preTransformNestedPredicate(Attrs, Attrs, Rels, Rels, Pred, NewPred) :-
  Pred =.. [Op, Attr, all(select SubAttr as Alias from RelsWhere)],
  allToAggr(Op, Aggr),
  AggrExpression =.. [Aggr, SubAttr],
  NewPred =.. [Op, Attr, select AggrExpression as Alias from RelsWhere].

% case operator ~all~
preTransformNestedPredicate(Attrs, Attrs, Rels, Rels, Pred, NewPred) :-
  Pred =.. [Op, Attr, all(select SubAttr from RelsWhere)],
  allToAggr(Op, Aggr),
  AggrExpression =.. [Aggr, SubAttr],
  NewPred =.. [Op, Attr, select AggrExpression from RelsWhere].

% default case, no pretransformation
preTransformNestedPredicate(Attrs, Attrs, Rels, Rels, Pred, Pred).


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

transform(select Attrs from Rels where Preds,
      select CanAttrs from CanRels where CanPreds) :-
  transform(select Attrs from Rels, select Attrs2 from Rels2),
  transformNestedPredicates(Attrs2, CanAttrs, Rels2,
  CanRels, Preds, CanPredList),
  flatten(CanPredList, CanPreds).

transform(select Attrs from Rels, select CanAttrs from CanRels) :-
  transformNestedAttributes(Attrs, Attrs2, Rels, Rels2),
  transformNestedRelations(Attrs2, CanAttrs, Rels2, CanRels).

/*

Default handling for queries without subqueries.

*/

transform(Query, Query).

/*

Subqueries in the attribute list have to yield a scalar result. If the nested query is uncorrelated with the outer query,
it can be evaluated independently of it and replaced by its result.

*/

transformNestedAttributes([], [], Rels, Rels).

transformNestedAttributes(Attr, Attr2, Rels, Rels2) :-
  not(is_list(Attr)),
  transformNestedAttribute(Attr, Attr2, Rels, Rels2).

transformNestedAttributes([ Attr | Rest ],
  [ Attr2 | Rest2 ], Rels, Rels3) :-
  transformNestedAttribute(Attr, Attr2, Rels, Rels2),
  transformNestedAttributes(Rest, Rest2, Rels2, Rels3).

/*

Replace a scalar, uncorrelated subquery with its result in the attribute list. As SECONDO does not generate temporary names
for constants in the attribute list, renaming has to occurr.

*/

transformNestedAttribute(Subquery as Variable, Value, Rels, Rels) :-
  nestingType(Subquery, a),
  optimize(Subquery, SecondoQuery, _),
  atom(SecondoQuery),
  atom_concat('query ', SecondoQuery, QueryText),
  secondo(QueryText, [_, Result]),
  not(is_list(Result)),
	Value =.. [as, Result, Variable].
  % atom_concat(Result, ' as ', Temp),
  % atom_concat(Temp, Variable, Value).

transformNestedAttribute(Attr, Attr, Rels, Rels) :-
  not(isQuery(Attr)).


/*

Handling of nested queries in the from clause. Uncorrelated subqueries are transformed
into a temporary relation. The resulting temporary relation is inserted into the
original query in place of the subquery. Transformation for correlated subqueries will fail.

*/

transformNestedRelations(Attrs, Attrs, [], []).

transformNestedRelations(Attrs, Attrs2, Rel, Rel2) :-
  not(is_list(Rel)),
  transformNestedRelation(Attrs, Attrs2, Rel, Rel2).

transformNestedRelations(Attrs, Attrs3,
  [ Rel | Rest ], [ Rel2 | Rest2]) :-
  transformNestedRelation(Attrs, Attrs2, Rel, Rel2),
  transformNestedRelations(Attrs2, Attrs3, Rest, Rest2).


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

transformNestedPredicates(Attrs, Attrs3, Rels, Rels3,
  [ Pred | Rest ], [ Pred2 | Rest2 ]) :-
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, Pred2),
  dc(subqueryUnnesting, ( not(Pred = Pred2)
    -> dm(subqueryUnnesting,
    ['\nPredicate: ', Pred, '\nTransformedPredicate: ', Pred2])
    ; true
   )),
  transformNestedPredicates(Attrs2, Attrs3, Rels2, Rels3, Rest, Rest2).

transformNestedPredicate(Attrs, Attrs, Rels, Rels, Pred, Pred) :-
  not(isSubqueryPred(Pred)).

/*

The SQL predicates EXISTS, NOT EXISTS, ALL and ANY are transformed to a canonical form, which can be further unnested.

*/

% case operator ~exists~
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2,
  exists(select SubAttr from RelsWhere), TransformedQuery) :-
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2,
  0 < (select count(SubAttr) from RelsWhere), TransformedQuery),
  write_canonical(TransformedQuery).

% case operator ~not exists~
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2,
  not(exists(select SubAttr from RelsWhere)), TransformedQuery) :-
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2,
  0 = (select count(SubAttr) from RelsWhere), TransformedQuery),
  write_canonical(TransformedQuery).

% case operator ~any~
transformNestedPredicate(Attrs, Attrs2,
  Rels, Rels2, Pred, TransformedQuery) :-
  Pred =.. [Op, Attr, any(select SubAttr from RelsWhere)],
  anyToAggr(Op, Aggr),
  AggrExpression =.. [Aggr, SubAttr],
  NewPred =.. [Op, Attr, select AggrExpression from RelsWhere],
  transformNestedPredicate(Attrs, Attrs2,
  Rels, Rels2, NewPred, TransformedQuery),
  write_canonical(TransformedQuery).

% special handling for ~= any(Query)~
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2,
  Attr = any(Query), TransformedQuery) :-
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2,
  Attr in(Query), TransformedQuery),
  write_canonical(TransformedQuery).

% special handling for ~<> any(Query)~
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2,
  Attr <> any(Query), TransformedQuery) :-
  transformNestedPredicate(Attrs, Attrs2, Rels, Rels2,
  Attr not in(Query), TransformedQuery),
  write_canonical(TransformedQuery).

% case operator ~all~
transformNestedPredicate(Attrs, Attrs2,
  Rels, Rels2, Pred, TransformedQuery) :-
  Pred =.. [Op, Attr, all(select SubAttr as Alias from RelsWhere)],
  allToAggr(Op, Aggr),
  AggrExpr =.. [Aggr, SubAttr],
  NewPred =.. [Op, Attr, select AggrExpr as Alias from RelsWhere],
  transformNestedPredicate(Attrs, Attrs2,
  Rels, Rels2, NewPred, TransformedQuery),
  write_canonical(TransformedQuery).

% case operator ~all~
transformNestedPredicate(Attrs, Attrs2,
  Rels, Rels2, Pred, TransformedQuery) :-
  Pred =.. [Op, Attr, all(select SubAttr from RelsWhere)],
  allToAggr(Op, Aggr),
  AggrExpression =.. [Aggr, SubAttr],
  NewPred =.. [Op, Attr, select AggrExpression from RelsWhere],
  transformNestedPredicate(Attrs, Attrs2,
  Rels, Rels2, NewPred, TransformedQuery),
  write_canonical(TransformedQuery).


/*

The subquery in this predicate does not contain a join predicate that references the relation of the outer query block
and has an aggregation function associated with the column name. As the subquery yields a scalar result, which is independent
of the outer query, the subqeery will be evaluated and replaced by its result.

*/

transformNestedPredicate(Attrs, Attrs,
  Rels, Rels, SubqueryPred, SubqueryPred2) :-
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

% case subquery with predicates
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, Pred2) :-
  Pred =.. [Op, Attr, Subquery],
  transform(Subquery, CanSubquery),
  CanSubquery =.. [from, Select, Where],
  Select =.. [select, SubAttr],
  Where =.. [where | [ CanRels | CanPreds ]] ,
  (nestingType(CanSubquery, n) ; nestingType(CanSubquery, j)),
  dm(subqueryUnnesting, ['\nNEST-N-J:\n']),
  ( is_list(SubAttr)
   -> nth1(1, SubAttr, CanAttr)
   ; SubAttr = CanAttr
  ),
  restrict(Attrs, Rels, Attrs2),
	makeList(Rels, RelsList),
	makeList(CanRels, CanRelsList),
	retainSetSemantics(RelsList, Attr, Op, CanAttr,
			CanRelsList, CanPreds, CanRels2, PredList),
  append(RelsList, CanRels2, Rels2),
%  subqueryToJoinOp(Op, NewOp),
%  JoinPredicate =.. [NewOp, Attr, CanAttr],
%  makeList(CanPreds, CanPredsList),
%  append([JoinPredicate], CanPredsList, PredList),
  flatten(PredList, Pred2).

% case subquery without predicates
transformNestedPredicate(Attrs, Attrs2, Rels, Rels2, Pred, JoinPredicate) :-
  Pred =.. [Op, Attr, Subquery],
  transform(Subquery, CanSubquery),
  CanSubquery =.. [from, Select, CanRels],
  Select =.. [select, SubAttr],
  (nestingType(CanSubquery, n) ; nestingType(CanSubquery, j)),
  dm(subqueryUnnesting, ['\nNEST-N-J:\n']),
  ( is_list(SubAttr)
   -> nth1(1, SubAttr, CanAttr)
   ; SubAttr = CanAttr
  ),
  restrict(Attrs, Rels, Attrs2),
  makeList(Rels, RelsList),
  makeList(CanRels, CanRelsList),
	retainSetSemantics(RelsList, Attr, Op,
                     CanAttr, CanRelsList, [], CanRels2, JoinPredicate),
  append(RelsList, CanRels2, Rels2).
%  subqueryToJoinOp(Op, NewOp),
%  JoinPredicate =.. [NewOp, Attr, CanAttr].

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
% find Relations which do not appear in from clause of inner query
  findall(R, (member(R, RList), not(member(R, CRList))), OuterRels),
  dm(subqueryDebug, ['\nOuterRels: ', OuterRels]),
% partition Predicates into simple inner, simple outer and join preds,
  joinPred(CanonicalRels, OuterRels, CanonicalPreds, JoinPreds,
  SimpleInnerPreds, SimpleOuterPreds, OuterJoinAttrs),
  dm(subqueryDebug, ['\nSimpleInnerPreds: ', SimpleInnerPreds]),
  dm(subqueryDebug, ['\nSimpleOuterPreds: ', SimpleOuterPreds]),
  dm(subqueryDebug, ['\nJoinPreds: ', JoinPreds]),
  dm(subqueryDebug, ['\nOuterJoinAttrs: ', OuterJoinAttrs]),
% restrict and project outer relation
  ( tempRel1(OuterJoinAttrs, SimpleOuterPreds, OuterRels, TempRel1)
  ; throw(error_SQL(subqueries_transformNestedPredicate::tRel1Error))),
% restrict and project inner relation
  ( tempRel2(AggregatedAttr, CanonicalRels, OuterRels, SimpleInnerPreds, 
      JoinPreds, TempRel2)
  ; throw(error_SQL(subqueries_transformNestedPredicate::tRel2Error))),
% join tempRel1 and tempRel2, applying aggregation
  ( tempRel3(AggregatedAttr, OuterJoinAttrs, TempRel1, TempRel2,
  JoinPreds, TempRel3, GroupVar, NewJoinAttr1, NewJoinAttr2)
  ; throw(error_SQL(subqueries_transformNestedPredicate::tRel3Error))),
  dm(subqueryDebug, ['\nTempRels successful...']),
% rename attributes, as aliasing in SECONDO and SECONDO optimizer
% is not compatible (Attr_alias vs. alias:Attr)
  newAlias(Var),
  alias(Var, GroupVar, AliasedAttr),
  NewPred =.. [Op, Attr, AliasedAttr],
  dm(subqueryDebug, ['\nNewPred: ', NewPred]),
  aliasInternal(NewJoinAttr1, JoinAttr1),
  aliasInternal(JoinAttr2, NewJoinAttr2),
  NewJoinPred =.. [=, JoinAttr1, Var:JoinAttr2],
  dm(subqueryDebug, ['\nNewJoinPred: ', NewJoinPred]),
  append([NewJoinPred], [NewPred], PredNext),
  dm(subqueryDebug, ['\nPredNext :', PredNext]),
  makeList(Rels, RelsList),
  append(RelsList, [TempRel3 as Var], RelsNext),
  dm(subqueryDebug, ['\nRelsNext :', RelsNext, '\n']),
  projectIfNeeded(Attrs, RelsList, Attrs1),
  transformNestedPredicate(Attrs1, Attrs2,
  RelsNext, Rels2, PredNext, Pred2).

/*

Project the query result to columns of the given relations, if select-clause is [star].

*/


projectIfNeeded1(Rel as Var, AttrList) :-
  !,
  relation(Rel, Attrs),
  findall(Var:A, member(A, Attrs), AttrList).

projectIfNeeded1(Rel, Attrs) :-
  relation(Rel, Attrs).

projectIfNeeded(*, [], []).

projectIfNeeded(*, [Rel | Rels], AttrList) :-
  projectIfNeeded1(Rel, Attr),
  projectIfNeeded(*, Rels, Attrs),
  makeList(Attr, Attr1),
  append(Attr1, Attrs, AttrList).

projectIfNeeded(Attrs, _, Attrs).

/*

Translate between optimizer syntax for renaming (using operator ~:~) and
executable syntax, which uses operator ~\_~ and postfix notation.

*/

aliasInternal(ExternalAttr, Attr) :-
  ground(Attr),
  Attr = Var:InternalAttr,
  concat_atom([InternalAttr, '_', Var], ExternalAttr).

aliasInternal(ExternalAttr, InternalAttr) :-
  ground(ExternalAttr),
  sub_atom(ExternalAttr, B, _, _, '_'),
  sub_atom(ExternalAttr, 0, B, _, Attr),
  B1 is B + 1,
  sub_atom(ExternalAttr, B1, _, 0, Internal),
  InternalAttr =.. [(:), Internal, Attr].

aliasInternal(Attr, Attr).

/*

Retain set semantics for operator ~IN~, otherwise duplicates may appear
in the unnested result. To achieve this, we need to create a temporary
relation, which is a projection and restriction of the inner relations,
containing no duplicates.

*/

% remove duplicates of inner relations, only for operator ~IN~
retainSetSemantics(OuterRels, Attr, in, SelectAttr,
                   InnerRels, InnerPreds, [TempRel], PredList) :-
  joinPred(InnerRels, OuterRels, InnerPreds, JoinPreds,
            SimpleInnerPreds, _, OuterAttrs),
  write('\nJoinPreds: '), write(JoinPreds), nl,
	( ( findall(Alias,
	  ( member(A, OuterAttrs), A =.. [:, Alias, _] ), AliasList),
		setof(A, member(A, AliasList), AL2) )
		; AL2 = [] ),
% convert aliased attributes to internal ~_~ syntax for all
% inner attributes in JoinPreds
  ( ( findall(Pred2, (
		member(Pred, JoinPreds), member(Al, AL2),
		aliasExternal(Pred, Pred2, dummy as Al),
% only consider aliases which are used in JoinPreds
		aliasExternal(Pred, TPred2, dummy as $$), Pred2 \= TPred2 ),
		JoinPreds2 ), setof(JP, member(JP, JoinPreds2), JPList) )
		; JPList = JoinPreds ),
	write('JoinPreds2: '), write(JPList), nl,
	append(InnerRels, OuterRels, Rels),
	( flatten(JoinPreds, [])
		-> callLookup(select * from Rels, _)
		; callLookup(select * from Rels where JoinPreds, _)
	),
	setof(Attrs, outerAttrs(InnerRels, Attrs), L),
	flatten(L, L2),
	( member(SelectAttr, L2)
		-> AttrList = L2
		;	append([SelectAttr], L2, AttrList)
	),
% convert aliased attribute in the new join predicate also
	aliasInternal(CanAttr2, SelectAttr),
  JoinPredicate =.. [=, Attr, CanAttr2],
  append([JoinPredicate], JPList, PredList),
	flatten(SimpleInnerPreds, SIPreds),
%	write('\nSimpleInnerPreds: '), write(SIPreds), nl,
	tempRel1(AttrList, SIPreds, InnerRels, TempRel).
%	write('\nTempRel: '), write(TempRel), nl.




% all other operators
retainSetSemantics(_, _, _, _,
                   Rels, Preds, Rels, Preds).

/*

---- joinPred(+InnerRels, +OuterRels, +PredicateList, -JoinPreds, -InnerPreds, -OuterPreds, -OuterAttrs)
----

Partition the predicate list in such a way, that ~InnerPreds~ contains all predicates referring to attributes of relations in
~InnerRels~, ~OuterPreds~ contains all predicates refering to attributes of relations in ~OuterRels~ and ~JoinPreds~ contains
any remaining predicates.

*/

joinPred(_, _, [], [], [], [], []).
joinPred(InnerRels, OuterRels, [ Pred | Rest ],
  JoinPreds, InnerPreds, OuterPreds, OuterJoinAttrs) :-
  joinPred(InnerRels, OuterRels, Pred,
  JoinPred1, InnerPreds1, OuterPreds1, OuterJoinAttrs1),
  joinPred(InnerRels, OuterRels, Rest,
  JoinPred2, InnerPreds2, OuterPreds2, OuterJoinAttrs2),
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

joinPred(InnerRels, OuterRels, Pred, [], [Pred], [], []) :-
  makeList(InnerRels, IRelsList),
  append(IRelsList, OuterRels, Rels),
  dm(subqueryDebug,
    ['\njoinPred:\n\t select * from ', Rels,
    ' where ', Pred]),
  callLookup(select * from Rels where Pred, _),
  setof(Attrs, outerAttrs(OuterRels, Attrs), L),
	flatten(L, []).

joinPred(InnerRels, OuterRels, Pred, [Pred], [], [], OuterAttrs) :-
  makeList(InnerRels, IRelsList),
  append(IRelsList, OuterRels, Rels),
  dm(subqueryDebug,
    ['\njoinPred:\n\t select * from ', Rels,
    ' where ', Pred]),
  callLookup(select * from Rels where Pred, _),
  setof(Attrs, outerAttrs(OuterRels, Attrs), L),
	flatten(L, OuterAttrs),
  dm(subqueryDebug, ['\nOuterAttrs: ', OuterAttrs]).

/*

Find all query attributes which belong to a Relation in the given query list.

*/

outerAttrs([], []).

outerAttrs([Rel | Rest], OuterAttrs) :-
  outerAttrs1([Rel], RelAttrs),
  outerAttrs(Rest, RestAttrs),
  append(RelAttrs, RestAttrs, OuterAttrs).

outerAttrs1(OuterRels, OuterAttrs) :-
  setof(Attr, (
    member(Rel, OuterRels),
    usedAttr(rel(Rel, *),
    attr(Attr2, _, _)),
    dcName2externalName(Attr, Attr2)
  ), OuterAttrs),
  not(OuterAttrs = []).

outerAttrs1(OuterRels, OuterAttrs) :-
  setof(Var:Attr, (
    member(Rel as Var, OuterRels),
    usedAttr(rel(Rel, Var),
    attr(Attr2, _, _)),
    dcName2externalName(Attr, Attr2)
  ), OuterAttrs),
  not(OuterAttrs = []).

outerAttrs1([_], []).

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

% NVK ADDED NR 
% i don't see where this can happend for nested relations.
isAttributeOf(Var:Attr, Rel as Var) :-
  optimizerOption(nestedRelations),
  isAttributeOf(Var:Attr, Rel).

isAttributeOf(Var:Attr, Rel) :-
  optimizerOption(nestedRelations),
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(predicate, _SRCSQID, Var, RelX, AttrDesc),
  RelX=rel(Rel, _),
  nrSimpleFindAttribute(AttrDC, AttrDesc, _).

isAttributeOf(Attr, Rel) :-
  optimizerOption(nestedRelations),
  atomic(Attr),
  downcase_atom(Attr, AttrDC),
  findAttrLists(predicate, _SRCSQID, *, RelX, AttrDesc),
  RelX=rel(Rel, *),
  nrSimpleFindAttribute(AttrDC, AttrDesc, _).
% NVK ADDED NR END


isAttributeOf(Alias:Attr, Rel as Alias) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED NR
  relation(Rel, List),
  atomic(Attr),
  downcase_atom(Attr, DCAttr),
  member(DCAttr, List).

isAttributeOf(Attr, Rel) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED NR
  atomic(Attr),
  not(is_list(Rel)),
  relation(Rel, List),
  atomic(Attr),
  downcase_atom(Attr, DCAttr),
  member(DCAttr, List).

isAttributeOf(Attr, [ Rel | Rest ]) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED NR
  (( relation(Rel, List),
  atomic(Attr),
  downcase_atom(Attr, DCAttr),
  member(DCAttr, List)) ;
  isAttributeOf(Attr, Rest)).

isAttributeOf(Attr, [ Rel as Alias | Rest ]) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED NR
  (( relation(Rel, List),
  alias(Alias, UnaliasedAttr, Attr),
  atomic(UnaliasedAttr),
  downcase_atom(UnaliasedAttr, DCAttr),
  member(DCAttr, List)) ;
  isAttributeOf(Attr, Rest)).

isAttributeOf(Const, _) :-
  is_list(Const),
  catch(string_to_list(Const, _), _, fail).

isAttributeOf(Const, _) :-
  integer(Const).

neededRels([], Rels, Rels).

neededRels(Preds, Rels, NeededRels) :-
  not(is_list(Preds)),
  lookupPreds([Preds], [Preds2]),
  Preds2 =.. [pr | List],
  findall(R, member(rel(R, *), List), NR),
  findall(R as V, (member(rel(R, V), List), V \= *), NR2),
  setof(Rel, (member(Rel, Rels) ; member(Rel, NR) ; member(Rel, NR2)), 
    NeededRels).

neededRels([Pred | Rest], Rels, NeededRels) :-
  neededRels(Pred, Rels, NR),
  neededRels(Rest, NR, NeededRels).

/*

Create a temporary relation which is a projection of Rels to ~JoinAttr~

*/
% case no predicates to apply
tempRel1(JoinAttrs, [], OuterRels, TempRel1) :-
  !,
	subset(OuterRels, NeededRels),
  areAttributesOf(JoinAttrs, NeededRels),
  dm(subqueryDebug, ['\nJoinAttrs: ', JoinAttrs]),
  dm(subqueryDebug, ['\nNeededRels: ', NeededRels]),
  ground(NeededRels),
  !,
  TemporaryRel1 =.. [from, select distinct JoinAttrs, NeededRels],
  dm(subqueryUnnesting, ['\nTemporaryRel1: ', TemporaryRel1]),
  newTempRel(TemporaryRel1, TempRel1).

% case also apply predicates
tempRel1(JoinAttrs, Preds, OuterRels, TempRel1) :-
  !,
	subset(OuterRels, NeededRels),
  areAttributesOf(JoinAttrs, NeededRels),
  dm(subqueryDebug, ['\nJoinAttrs: ', JoinAttrs]),
  dm(subqueryDebug, ['\nNeededRels: ', NeededRels]),
  neededRels(Preds, NeededRels, NeededRels2),
  ground(NeededRels2),
  !,
  TemporaryRel1 =.. [from, select distinct JoinAttrs, where(NeededRels2, 
    Preds)],
  dm(subqueryUnnesting, ['\nTemporaryRel1: ', TemporaryRel1]),
  newTempRel(TemporaryRel1, TempRel1).

/*

Create a restriction and projection of ~Rels~ by all predicates supplied.

*/
% case no predicates, only projection
tempRel2(AggregatedAttr, Rel, OuterRels, [], JoinPreds, TempRel2) :-
  makeList(Rel, RelList),
  aggregationAttrs(AggregatedAttr, Rel, Attr),
  joinPred(OuterRels, RelList, JoinPreds, _, _, _, Attrs),
  makeList(Attrs, AttrList),
  makeList(Attr, AttrList1),
  ( Attr \= *
    ->   append(AttrList, AttrList1, AttrList2)
    ; AttrList2 = AttrList ),
  setof(A, member(A, AttrList2), AttrList3),
  TemporaryRel2 =.. [from, select AttrList3, Rel],
  dm(subqueryUnnesting, ['\nTemporaryRel2: ', TemporaryRel2]),
  newTempRel(TemporaryRel2, TempRel2).

% case restrict and project
tempRel2(AggregatedAttr, Rels, OuterRels, Preds, JoinPreds, TempRel2) :-
  makeList(Rels, RelList),
  Where =.. [where, RelList, Preds],
  aggregationAttrs(AggregatedAttr, Rels, Attr),
  joinPred(OuterRels, RelList, JoinPreds, _, _, _, Attrs),
  makeList(Attrs, AttrList),
  makeList(Attr, AttrList1),
  ( Attr \= *
    ->   append(AttrList, AttrList1, AttrList2)
    ; AttrList2 = AttrList ),
  setof(A, member(A, AttrList2), AttrList3),
  TemporaryRel2 =.. [from, select AttrList3, Where],
  dm(subqueryUnnesting, ['\nTemporaryRel2: ', TemporaryRel2]),
  newTempRel(TemporaryRel2, TempRel2).

/*

Create temporary relation which replaces the aggregation operation by its
constant result for the corresponding query. If the aggregation operation
is ~count~ the query is constructed with an outerjoin. A simple join operation
is used otherwise.

*/

tempRel3(AggregatedAttr, JoinAttrs, TempRel1, TempRel2,
  JoinPreds, TempRel3, NewColumn, NewJoinAttr1, NewJoinAttr2) :-
  AggregatedAttr =.. [AggrOp, Attr],
  not(AggrOp = count),
  dm(subqueryDebug, ['\nAggrOp: ', AggrOp]),
  dm(subqueryDebug, ['\nAttr: ', Attr]),
  newVariable(NewColumn),
  aliasExternal(Attr, Attr2, TempRel2),
  dm(subqueryDebug, ['\nAttr2: ', Attr2]),
  AggregatedAttr2 =.. [AggrOp, Attr2],
  makeList(AggregatedAttr2 as NewColumn, AggrList),
  dm(subqueryDebug, ['\nJoinAttrs: ', JoinAttrs]),
  findall(JoinAttr2,
    ( member(JoinAttr, JoinAttrs),
    aliasExternal(JoinAttr, JoinAttr2, TempRel2)
  ),
  JoinAttrs2
  ),
  dm(subqueryDebug, ['\nJoinAttrs2: ', JoinAttrs2]),
  append(JoinAttrs2, AggrList, AttrList),
  findall(Pred2,
    ( member(Pred, JoinPreds),
    aliasExternal(Pred, Pred2, TempRel2)
  ),
  JoinPreds2
  ),
  makeList(TempRel2, TR2List),
  append([TempRel1], TR2List, Rels),
  TempRelation3 =.. 
    [groupby, from(select AttrList, where(Rels, JoinPreds2)), JoinAttrs2],
  dm(subqueryDebug, ['\nTemporaryRel3: ', TempRelation3]),
  rewriteQuery(TempRelation3, RQuery),
  callLookup(RQuery, Query2), !,
  queryToPlan(Query2, Plan, _), !,
  dm(subqueryDebug, ['\nPlan: ', Plan]),
  findJoinAttrs(Plan, JoinAttr1, JoinAttr2, _),
  dm(subqueryDebug, ['\nJoinAttr1: ', JoinAttr1, '\nJoinAttr2: ', JoinAttr2]),
  ( isAttributeOf(JoinAttr1, TempRel1)
    -> ( NewJoinAttr1 = JoinAttr1, NewJoinAttr2 = JoinAttr2 )
  ;  ( NewJoinAttr1 = JoinAttr2, NewJoinAttr2 = JoinAttr1 )
  ),
  append([NewJoinAttr2], AggrList, AttrList2),
  dm(subqueryDebug, ['\nAttrList2: ', AttrList2]),
  TemporaryRel3 =.. 
    [groupby, from(select AttrList2, where(Rels, JoinPreds2)), NewJoinAttr2],
  dm(subqueryUnnesting, ['\nTemporaryRel3: ', TemporaryRel3]),
  newTempRel(TemporaryRel3, TempRel3).

% case aggregation function = count, use full outerjoin
tempRel3(AggregatedAttr, JoinAttrs, TempRel1, TempRel2,
  JoinPreds, TempRel3, NewColumn, NewJoinAttr1, NewJoinAttr2) :-
  AggregatedAttr =.. [AggrOp, Attr],
  AggrOp = count,
  dm(subqueryDebug, ['\nAggrOp: ', AggrOp]),
  dm(subqueryDebug, ['\nAttr: ', Attr]),
  newVariable(NewColumn),
  aliasExternal(Attr, Attr2, TempRel2),
  dm(subqueryDebug, ['\nAttr2: ', Attr2]),
  AggregatedAttr2 =.. [AggrOp, Attr2],
  makeList(AggregatedAttr2 as NewColumn, AggrList),
  dm(subqueryDebug, ['\nJoinAttrs: ', JoinAttrs]),
  findall(JoinAttr2,
    ( member(JoinAttr, JoinAttrs),
    aliasExternal(JoinAttr, JoinAttr2, TempRel2)
  ),
  JoinAttrs2
  ),
  dm(subqueryDebug, ['\nJoinAttrs2: ', JoinAttrs2]),
  append(JoinAttrs2, AggrList, AttrList),
  findall(Pred2,
    ( member(Pred, JoinPreds),
    aliasExternal(Pred, Pred2, TempRel2)
  ),
  JoinPreds2
  ),
  makeList(TempRel2, TR2List),
  append([TempRel1], TR2List, Rels),
  TemporaryRel3 =.. 
    [groupby, from(select AttrList, where(Rels, JoinPreds2)), JoinAttrs2],
  dm(subqueryDebug, ['\nTemporaryRel3: ', TemporaryRel3]),
  rewriteQuery(TemporaryRel3, RQuery),
  callLookup(RQuery, Query2), !,
  queryToPlan(Query2, Plan, _), !,
  dm(subqueryDebug, ['\nPlan: ', Plan]),
  findJoinAttrs(Plan, JoinAttr1, JoinAttr2, Op),
  dm(subqueryDebug, ['\nJoinAttr1: ', JoinAttr1, '\nJoinAttr2: ', JoinAttr2]),
  ( isAttributeOf(JoinAttr1, TempRel1)
    -> OuterJoinPred = outerjoin(Op, attrname(JoinAttr1), attrname(JoinAttr2))
  ;  ( OuterJoinPred = outerjoin(Op, attrname(JoinAttr2), attrname(JoinAttr1)),
       retractall(outerjoinCommuted),
       assert(outerjoinCommuted))
  ),
  OuterJoinPred =.. 
    [outerjoin, Op, attrname(NewJoinAttr1), attrname(NewJoinAttr2)],
  dm(subqueryDebug, ['\nOuterJoinPred: ', OuterJoinPred]),
  newQuery,
  lookupRel(TempRel1, IntTempRel1),
  dm(subqueryDebug, ['\nIntTempRel1: ', IntTempRel1]),
  lookupRel(TempRel2, IntTempRel2),
  dm(subqueryDebug, ['\nIntTempRel2: ', IntTempRel2]),
  lookupAttr(JoinAttr1 as NewColumn, _ as IntNewColumn),
  dm(subqueryDebug, ['\nIntNewColumn: ', IntNewColumn]),
  % lookupPred(OuterJoinPred,
  % pr(outerjoin(IntJoinAttr1, IntJoinAttr2), _, _)),
  % dm(subqueryDebug, ['\nIntJoinAttr1: ', IntJoinAttr1]),
  % dm(subqueryDebug, ['\nIntJoinAttr2: ', IntJoinAttr2]),
  lookupPred(OuterJoinPred, pr(IntOuterJoinPred, _, _)),
  IntOuterJoinPred =.. [outerjoin, Op, IntJoinAttr1, IntJoinAttr2],
  makeStream(IntTempRel1, IntStreamRel1),
  plan_to_atom(IntStreamRel1, ExtTempRel1),
  dm(subqueryDebug, ['\nExtTempRel1: ', ExtTempRel1]),
  makeStream(IntTempRel2, IntStreamRel2),
  plan_to_atom(IntStreamRel2, ExtTempRel2),
  dm(subqueryDebug, ['\nExtTempRel2: ', ExtTempRel2]),
  plan_to_atom(IntJoinAttr1, ExtJoinAttr1),
  dm(subqueryDebug, ['\nExtJoinAttr1: ', ExtJoinAttr1]),
  plan_to_atom(IntJoinAttr2, ExtJoinAttr2),
  dm(subqueryDebug, ['\nExtJoinAttr2: ', ExtJoinAttr2]),
  plan_to_atom(attrname(IntNewColumn), ExtNewColumn),
  dm(subqueryDebug, ['\nExtNewColumn: ', ExtNewColumn]),
  plan_to_atom(IntOuterJoinPred, ExtOuterJoinPred),
  dm(subqueryDebug, ['\nExtOuterJoinPred: ', ExtOuterJoinPred]),
  concat_atom([
     ExtTempRel1,
     ExtTempRel2,
%     ' smouterjoin[',ExtJoinAttr1, ',', ExtJoinAttr2, ']',
     ExtOuterJoinPred,
     ' sortby[', ExtJoinAttr1, ' asc]',
     ' groupby[', ExtJoinAttr1, ';', ExtNewColumn, 
        ': group feed filter[not(isempty(.', ExtJoinAttr2, ' ))] count]',
     ' projectextend[', ExtNewColumn, '; ', ExtJoinAttr2 ,
	 ': .', ExtJoinAttr1, ']',
     ' consume'],
  '', TempRelation3),
  dm(subqueryUnnesting, ['\nTempRelation3: ', TempRelation3]),
  ( newTempRel_direct(TempRelation3, TempRel3)
    -> true
  ; throw(error_SQL(subqueries_tempRel3::errorInQuery))).

/*

Get all attributes of a list of attribut expressions.

*/
aggregationAttrs(AggrExpr, Rels, AttrList2) :-
  AggrExpr =.. [_, Term],
  aggregationAttrs1(Term, Rels, AttrList),
  flatten(AttrList, AttrList2).

aggregationAttrs1([], _, []).

aggregationAttrs1(Attr, Rels, Attr) :-
  not(is_list(Attr)),
  isAttributeOf(Attr, Rels),
  not(integer(Attr)).

aggregationAttrs1([ Term | Rest ], Rels, [Attr | AttrList]) :-
  aggregationAttrs1(Term, Rels, Attr),
  aggregationAttrs1(Rest, Rels, AttrList).

aggregationAttrs1(Term, Rels, AttrList) :-
  not(is_list(Term)),
  compound(Term),
  Term =.. [Op | Args],
  Op \= (:),
  aggregationAttrs1(Args, Rels, AttrList).

aggregationAttrs1(Term, _, []) :-
  not(is_list(Term)).

/*

  Ugly hack to find the join Attributes which the optimizer would select on a Join Query with the same predicates. Used only to select
  which attributes will be used in the outerjoin.

*/

% case symmjoin as join operator
findJoinAttrs(Plan, JoinAttr1, JoinAttr2, Op) :-
  Plan =.. [symmjoin | Args], !,
  nth1(3, Args, Pred),
  dm(subqueryDebug, ['\nPred: ', Pred]),
  Pred =.. [Op, attr(Attr1, _, _), attr(Attr2, _, _)],
  ( Attr1 = Var1:AttrName1 ; Attr1 = AttrName1 ),
  ( Attr2 = Var2:AttrName2 ; Attr2 = AttrName2 ),
  dcName2externalName(JoinAttrName1, AttrName1),
  dcName2externalName(JoinAttrName2, AttrName2),
  ( (ground(Var1), JoinAttr1 = Var1:JoinAttrName1)
    ; JoinAttr1 = JoinAttrName1 ),
 ( (ground(Var2), JoinAttr2 = Var2:JoinAttrName2)
    ; JoinAttr2 = JoinAttrName2 ).

% other join operators, assumes that all join operators
% end in 'join'
findJoinAttrs(Plan, JoinAttr1, JoinAttr2, =) :-
  Plan =.. [Op | Args],
  atom_concat(_, 'join', Op), !,
  nth1(3, Args, attrname(attr(Attr1, _, _))),
  nth1(4, Args, attrname(attr(Attr2, _, _))),
  ( Attr1 = Var1:AttrName1 ; Attr1 = AttrName1 ),
  ( Attr2 = Var2:AttrName2 ; Attr2 = AttrName2 ),
  dcName2externalName(JoinAttrName1, AttrName1),
  dcName2externalName(JoinAttrName2, AttrName2),
  ( (ground(Var1), JoinAttr1 = Var1:JoinAttrName1)
    ; JoinAttr1 = JoinAttrName1 ),
 ( (ground(Var2), JoinAttr2 = Var2:JoinAttrName2)
    ; JoinAttr2 = JoinAttrName2 ).

findJoinAttrs(Plan, JoinAttr1, JoinAttr2, Op) :-
  not(is_list(Plan)),
  compound(Plan),
  Plan =.. [_ | Args],
  findJoinAttrs(Args, JoinAttr1, JoinAttr2, Op).

findJoinAttrs([ Arg ], JoinAttr1, JoinAttr2, Op) :-
  findJoinAttrs(Arg, JoinAttr1, JoinAttr2, Op).

findJoinAttrs([ Arg | Rest ], JoinAttr1, JoinAttr2, Op) :-
  ( findJoinAttrs(Arg, JoinAttr1, JoinAttr2, Op) ;
    ( not(Rest = []),
    findJoinAttrs(Rest, JoinAttr1, JoinAttr2, Op) )).

/*

Translate aliased expression from optimizer to kernel syntax.

*/
aliasExternal(Expr, Expr, TempRel2) :-
  is_list(TempRel2).

aliasExternal(Var:Attr, Var:Attr, _ as Var) :-
  !.

aliasExternal(Var:Attr, AliasedAttr, T) :-
  not(is_list(T)),
  concat_atom([Attr, '_', Var], '', AliasedAttr).

aliasExternal(Attr, Attr, _) :-
  atomic(Attr).

aliasExternal(Pred, AliasedPred, T) :-
  compound(Pred),
  Pred =.. [Op, Attr1, Attr2],
  not(Op = (:)),
  aliasExternal(Attr1, AliasedAttr1, T),
  aliasExternal(Attr2, AliasedAttr2, T),
  AliasedPred =.. [Op, AliasedAttr1, AliasedAttr2].

aliasExternal(Pred, AliasedPred, T) :-
  compound(Pred),
  Pred =.. [UnaryOp, Pred2],
  aliasExternal(Pred2, AliasedPred2, T),
  AliasedPred =..[UnaryOp, AliasedPred2].

aliasExternal(Expr, AliasedExpr, T) :-
  compound(Expr),
  Expr =.. [Op, Arg1, Arg2, Arg3],
  not(Op = (:)),
  aliasExternal(Arg1, AliasedArg1, T),
  aliasExternal(Arg2, AliasedArg2, T),
  aliasExternal(Arg3, AliasedArg3, T),
  AliasedExpr =.. [Op, AliasedArg1, AliasedArg2, AliasedArg3].

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

Utitility predicates to define unique names with a defined prefix.

*/
:- dynamic(relDefined/1).
:- dynamic(uvarDefined/2).

newUniqueVar(Name, Var) :-
  ground(Name),
  uvarDefined(Name, N),
  !,
  N1 is N + 1,
  retract(uvarDefined(Name, N)),
  assert(uvarDefined(Name, N1)),
  atom_concat(Name, N1, Var).

newUniqueVar(Name, Var) :-
  ground(Name),
  assert(uvarDefined(Name, 1)),
  atom_concat(Name, '1', Var).

% unique temporary relation name
newTempRel(Var) :-
  !,
  newUniqueVar('trelxx', Var).

% uniquey alias name for renaming
newAlias(Var) :-
	\+ optimizerOption(nestedRelations), % NVK ADDED NR
  !,
  newUniqueVar('alias', Var).

/*
NVK ADDED NR
The current alias is stored here to have access to this value within
the plan\_to\_atom phase.

*/
newAlias(Var) :-
	optimizerOption(nestedRelations),
  !,
	getSubqueryCurrent(SQID), 
	aliasBySQID(SQID, Var).
% NVK ADDED NR END

/*

----  newTempRel(+Query, -TempRelName)
----

Store all temporary Relations created by the evaluation of a subquery for reuse in subsequent invocations.

---- temporaryRelation(TempRelName, RelQuery, Cost)
----
Store temporary relation for creation if the optimizer decides on an execution plan involving a temporary Relation.

*/


:- dynamic temporaryRelation/1,
   temporaryRelation/2.

newTempRel(Query, TempRelName) :-
  ( temporaryRelation(TempRelName, Query)
    ; ( newTempRel(TempRelName),
      assert(temporaryRelation(TempRelName, Query)),
        createTempRel(TempRelName)
     )
  ),
	dm(temprels, ['\nTempRelName: ', TempRelName, '\n']).

% case executable syntax for temprel supplied
newTempRel_direct(Plan, TempRelName) :-
  ( temporaryRelation(TempRelName, Plan)
     ; (  newTempRel(TempRelName),
        concat_atom([TempRelName, ' = ', Plan], Query),
      dm(subqueryDebug, ['\nTempRelDirect Plan: ', Query]),
      let(Query),
      assert(temporaryRelation(TempRelName, Plan))
    )
  ),
	dm(temprels, ['\nTempRelName_direct: ', TempRelName, '\n']).

createTempRel(TempRelName) :-
  ground(TempRelName),
  temporaryRelation(TempRelName, RelQuery),
  dm(temprels, ['\nRelQuery: ', RelQuery]),
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
  dm(temprels, ['\nDeleting temporary Relation ', Rel, ' ...']),
  catch(drop_relation(Rel), Ex, true),
  dm(temprels, [Ex]),
  deleteTempRels(Rest).

/*

Remove all temporary relations in the open database. Will always be called
on opening a database to provide a clean execution environment.

*/

dropTempRels :-
  retractall(temporaryRelation(_)),
  retractall(temporaryRelation(_, _)),
  retractall(rewriteCache(_, _)),
  findall(Rel, ( databaseName(DB),
                 storedRel(DB, Rel, _),
         atom_concat('trelxx', _, Rel),
         not(atom_concat(_, '_sample_s', Rel)),
         not(atom_concat(_, '_sample_j', Rel))
        ), RelList),
  not(RelList = []),
  write_list(['\nINFO:\tRemoving leftover temporary relations...']), nl,
  catch(deleteTempRels(RelList), _, true).
%  updateCatalog.

dropTempRels.

/*

type-A nesting, uncorrelated subquery with scalar result.

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
  throw(error_SQL(subqueries_nestingType::unknownNesting)).

/*

18.1.2 Lookup of subquery predicates

*/

lookupSubquery(all(Query), all(Query2)) :-
  lookupSubquery(Query, Query2).

lookupSubquery(any(Query), any(Query2)) :-
  lookupSubquery(Query, Query2).

lookupSubquery(select Attrs from Rels where Preds,
        select Attrs2 from Rels2List where Preds2List) :-
	nrAssignAlias, % NVK ADDED NR
  lookupRelsDblCheck(Rels, Rels2),
  !,
  lookupAttrs(Attrs, Attrs2),
  lookupPreds(Preds, Preds2),
  makeList(Rels2, Rels2List),
  makeList(Preds2, Preds2List),
  (optimizerOption(entropy)
    -> registerSelfJoins(Preds2List, 1); true).
          % needed for entropy optimizer

lookupSubquery(select Attrs from Rels,
        select Attrs2 from Rels2) :-
	nrAssignAlias, % NVK ADDED NR
  lookupRelsDblCheck(Rels, Rels2), !,
  lookupAttrs(Attrs, Attrs2).

lookupSubquery(Query orderby Attrs, Query2 orderby Attrs3) :-
  lookupSubquery(Query, Query2),
  makeList(Attrs, Attrs2),
  lookupAttrs(Attrs2, Attrs3).

lookupSubquery(Query groupby Attrs, Query2 groupby Attrs3) :-
  lookupSubquery(Query, Query2),
  makeList(Attrs, Attrs2),
  lookupAttrs(Attrs2, Attrs3).

lookupSubquery(Query first N, Query2 first N) :-
  lookupSubquery(Query, Query2).

lookupSubquery(Query last N, Query2 last N) :-
  lookupSubquery(Query, Query2).


/*

Lookup relation, special error message for subqueries, if relation names
are not unique.

*/
lookupRelsDblCheck([], []).

lookupRelsDblCheck([R | Rs], [R2 | R2s]) :-
  lookupRelDblCheck(R, R2),
  lookupRelsDblCheck(Rs, R2s).

lookupRelsDblCheck(Rel, Rel2) :-
  not(is_list(Rel)),
  lookupRelDblCheck(Rel, Rel2).

lookupRelDblCheck(Rel as Var, rel(RelDC, Var)) :-
  lookupRel(Rel as Var, rel(RelDC, Var)).

lookupRelDblCheck(Rel, rel(RelDC, *)) :-
  not(queryRel(Rel, _)),
  lookupRel(Rel, rel(RelDC, *)).

lookupRelDblCheck(Rel, rel(RelDC, *)) :-
  queryRel(Rel, rel(RelDC, *)),
  term_to_atom(Rel, RelA),
  concat_atom(['Ambiguous use of relation ',RelA,
         ' in outer and inner query block.'],'',ErrMsg),
  write_list(['\nERROR:\t',ErrMsg]),
  throw(
    error_SQL(subqueries_lookupRelDblCheck(Rel)::malformedExpression::ErrMsg)).

:- multifile(lookupRelNoDblCheck/2).

lookupRelNoDblCheck(Rel as Var, rel(RelDC, Var)) :-
  atomic(Rel),       %% changed code FIXME
  atomic(Var),       %% changed code FIXME
  dcName2externalName(RelDC,Rel),
  relation(RelDC, _), !,          %% changed code FIXME
  ( variable(Var, _)
    ;  assert(variable(Var, rel(RelDC, Var)))
  ).

:- dynamic(currentAttrs/1).
:- dynamic(currentRels/1).
:- dynamic(currentVariables/1).
:- dynamic(isJoinPred/1).
:- dynamic(selectivityQuery/1).
:- dynamic(selectivityRels/1).
:- dynamic(sampleSize/1).
:- dynamic(maxSampleCard/1).
:- dynamic(maxSelCard/1).
:- dynamic(outerjoinCommuted/0).


subquerySelectivity(Pred, [Rel]) :-
  subquerySelectivity(pr(Pred, Rel)).

subquerySelectivity(Pred, [Rel1, Rel2]) :-
  subquerySelectivity(pr(Pred, Rel1, Rel2)).

relsAfter(Rels, AttrRelsAfter, RelsAfter) :-
  findall(Rel, ( member(Rel, Rels), not(member(Rel, AttrRelsAfter))), L1),
  setof(R, member(R, L1), L), append(AttrRelsAfter, L, RelsAfter).

relsAfter(Rels, [], RelsAfter) :-
  setof(R, member(R, Rels), RelsAfter).

relsAfter(Rels, RelsAfter, RelsAfter) :-
  findall(Rel, ( member(Rel, Rels), not(member(Rel, RelsAfter))), []).

relsAfter(_, _, _) :-
  currentRels(QueryRels),
  currentVariables(QueryVariables),
%  write('\nQueryRels: '), write(QueryRels), nl,
  findall(A, variable(A, _), _),
%  write('\nVariables: '), write(L1), nl,
  findall([R, rel(R, Var)],
  ( queryRel(R, rel(R, Var)),
    not(member([R, rel(R, Var)], QueryRels)
  ),
  retractall(queryRel(R, rel(R, Var)))),
  _),
  findall(V,
  ( variable(V, _),
    not(member(V, QueryVariables)),
    retractall(variable(V, _))
  ),
  _),
%  write('\nRetractRels: '), write(L),
%  retractall(currentRels(_)),
  fail.

% lookup for outerjoin expression
lookupSubqueryPred(outerjoin(Op, A1, A2), outerjoin(Op, Attr1, Attr2), 
    RelsBefore, RelsAfter) :-
  lookupPred1(A1, Attr1, RelsBefore, Attr1RelsAfter),
  lookupPred1(A2, Attr2, Attr1RelsAfter, RelsAfter),
  !.

% case operator ~not in~
lookupSubqueryPred(Pred, Pred2, RelsBefore, RelsAfter) :-
  isSubqueryPred1(Pred),
  dm(subqueryDebug, ['\nlookupSubqueryPred 1\nPred: ', Pred]),
  Pred =.. [not, Attr, in(Query)],
  dm(subqueryDebug,
    ['\nAttr: ', Attr,
    '\nQuery: ', Query]),
  lookupPred1(Attr, Attr2, RelsBefore, AttrRelsAfter),
  dm(subqueryDebug, ['\nAttr2: ', Attr2,
    '\nAttrRelsAfter: ', AttrRelsAfter]),
  findall(A, queryAttr(A), QueryAttrs),
  assert(currentAttrs(QueryAttrs)),
  findall([R, RR], queryRel(R, RR), QueryRels),
  assert(currentRels(QueryRels)),
  findall(V, variable(V, _), Variables),
  assert(currentVariables(Variables)),
  enterSubquery(predicate), 	% NVK ADDED
  lookupSubquery(Query, Query2),
  getSubqueryCurrent(SQID),		% NVK ADDED
  leaveSubquery, 			% NVK ADDED
  dm(subqueryDebug, ['\nQuery2: ', Query2]),
  correlationRels(Query2, Rels),
  dm(subqueryDebug, ['\nRels: ', Rels]),
  relsAfter(Rels, AttrRelsAfter, RelsAfter), !,
  dm(subqueryDebug, ['\nRelsAfter: ', RelsAfter]),
  Query3 = subquery(SQID, Query2, RelsAfter), % NVK MODIFIED
  Pred2 =.. [not, Attr2, in(Query3)],
  retractall(currentAttrs(_)),
  retractall(currentRels(_)),
  retractall(currentVariables(_)),
  dm(subqueryDebug, ['\nPred2: ', Pred2]),
  subquerySelectivity(Pred2, RelsAfter).

% default case, can fail for selection RelsAfter = [_G12345]
lookupSubqueryPred(Pred, Pred2, RelsBefore, RelsAfter) :-
  isSubqueryPred1(Pred),
  dm(subqueryDebug, ['\nlookupSubqueryPred 1\nPred: ', Pred]),
  Pred =.. [Op, Attr, Query],
  dm(subqueryDebug,
    ['\nOp: ', Op,
    '\nAttr: ', Attr,
    '\nQuery: ', Query,
    '\nRelsAfter: ', RelsAfter]),
  lookupPred1(Attr, Attr2, RelsBefore, AttrRelsAfter),
  dm(subqueryDebug, ['\nAttr2: ', Attr2,
           '\nAttrRelsAfter: ', AttrRelsAfter]),
  findall(A, queryAttr(A), QueryAttrs),
  assert(currentAttrs(QueryAttrs)),
  findall([R, RR], queryRel(R, RR), QueryRels),
  assert(currentRels(QueryRels)),
  findall(V, variable(V, _), Variables),
  assert(currentVariables(Variables)),
  enterSubquery(predicate), 	% NVK ADDED
  lookupSubquery(Query, Query2),
  getSubqueryCurrent(SQID),		% NVK ADDED
  leaveSubquery, 			% NVK ADDED
  dm(subqueryDebug, ['\nQuery2: ', Query2]),
  correlationRels(Query2, Rels),
  dm(subqueryDebug, ['\nRels: ', Rels]),
  relsAfter(Rels, AttrRelsAfter, RelsAfter), !,
  dm(subqueryDebug, ['\nRelsAfter: ', RelsAfter]),
  Query3 = subquery(SQID, Query2, RelsAfter), % NVK MODIFIED
  Pred2 =.. [Op, Attr2, Query3],
  retractall(currentAttrs(_)),
  retractall(currentRels(_)),
  retractall(currentVariables(_)),
  dm(subqueryDebug, ['\nPred2: ', Pred2]),
  subquerySelectivity(Pred2, RelsAfter).

% remove side effects, if previous predicate failed
lookupSubqueryPred(_, _, _, _) :-
  currentAttrs(QueryAttrs),
  findall(A,
  ( queryAttr(A),
    not(member(A, QueryAttrs)),
    retract(queryAttr(A))
  ),
  _),
  retractall(currentAttrs(_)),
  fail.

% case negated predicate
lookupSubqueryPred(not(Pred), not(Pred2), RelsBefore, RelsAfter) :-
  isSubqueryPred1(not(Pred)),
  dm(subqueryDebug, ['\nlookupSubqueryPred 2\nPred: ', not(Pred)]),
  Pred =.. [Op, Query],
  dm(subqueryDebug,
    ['\nOp: ', Op,
    '\nQuery: ', Query]),
  enterSubquery(predicate),   % NVK ADDED
  lookupSubquery(Query, Query2),
  getSubqueryCurrent(SQID),   % NVK ADDED
  leaveSubquery,              % NVK ADDED
  dm(subqueryDebug, ['\nQuery2: ', Query2]),
  correlationRels(Query2, Rels),
  append(Rels, RelsBefore, RelsAfter),
  Query3 = subquery(SQID, Query2, RelsAfter), % NVK MODIFIED
  Pred2 =.. [Op, Query3],
  dm(subqueryDebug, ['\nPred2: ', not(Pred2)]),
  subquerySelectivity(not(Pred2), RelsAfter).

% default case for join predicates
lookupSubqueryPred(Pred, Pred2, RelsBefore, RelsAfter) :-
  isSubqueryPred1(Pred),
  dm(subqueryDebug, ['\nlookupSubqueryPred 2\nPred: ', Pred]),
  Pred =.. [Op, Query],
  dm(subqueryDebug,
    ['\nOp: ', Op,
    '\nQuery: ', Query]),
  enterSubquery(predicate),   % NVK ADDED
  lookupSubquery(Query, Query2),
  getSubqueryCurrent(SQID),   % NVK ADDED
  leaveSubquery,              % NVK ADDED
  dm(subqueryDebug, ['\nQuery2: ', Query2]),
  correlationRels(Query2, Rels),
  append(Rels, RelsBefore, RelsAfter),
  Query3 = subquery(SQID, Query2, RelsAfter),
  Pred2 =.. [Op, Query3],
  dm(subqueryDebug, ['\nPred2: ', Pred2]),
  subquerySelectivity(Pred2, RelsAfter).

containsSubqueryPred(Pred) :-
  not(is_list(Pred)),
  isSubqueryPred1(Pred).

containsSubqueryPred([Pred | Rest]) :-
  isSubqueryPred1(Pred) ;
  containsSubqueryPred(Rest).

isSubqueryPred1(not(Pred)) :-
  isSubqueryPred1(Pred).

isSubqueryPred1(Pred) :-
  compound(Pred),
  Pred =.. [Op, _, Subquery],
  isSubqueryOp(Op),
  Subquery =.. [from, _, _].

isSubqueryPred1(Pred) :-
  compound(Pred),
  Pred =.. [not, _, in(Subquery)],
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

isSubqueryPred1(Pred) :-
  compound(Pred),
  Pred =.. [Op, _, Subquery],
  isSubqueryOp(Op),
  isSubquery(Subquery).

isSubqueryPred1(Pred) :-
  compound(Pred),
  Pred =.. [not, _, in(Subquery)],
  isSubquery(Subquery).

isSubqueryPred1(Pred) :-
  compound(Pred),
  Pred =.. [Op, Subquery],
  isSubqueryOp(Op),
  isSubquery(Subquery).

isSubqueryPred1(Pred) :-
  compound(Pred),
  Pred =.. [Op, _, QuantifiedPred],
  QuantifiedPred =.. [Quantifier, Subquery],
  isComparisonOp(Op),
  isQuantifier(Quantifier),
  isSubquery(Subquery).

isSubqeryPred1(exists(Subquery)) :-
  isSubquery(Subquery).

isSubquery(subquery(_, _, _)). % NVK MODIFIED NR

/*

Find all relations referenced in predicates of ~Query~ which are not part
of the from-clause.

*/

correlationRels(Query, OuterRels) :-
  \+ optimizerOption(nestedRelations), % NVK ADDED NR
  removeCorrelatedPreds(Query, _, Preds),
  usedRels(Preds, Rels),
  Query =.. [from, _, Where],
  Where =.. [where, InnerRels, _],
  findall(Rel, ( member(Rel, Rels), not(member(Rel, InnerRels)) ), OuterRels),
  dm(subqueryDebug, ['\ncorrelationRels_OuterRels: ', OuterRels]),
	!. % NVK ADDED NR

correlationRels(select _ from InnerRels, InnerRels) :-
  \+ optimizerOption(nestedRelations),
  not(InnerRels =.. [where | _ ]),
	!. % NVK ADDED NR

/*
NVK ADDED NR
Replaced to reflect that a subquery can now be a correlated predicates if the 
from clause contains a attribute from the outer query. Otherwiese queries like test query no. 9 would not be possible with the current pog optimizer.

*/
correlationRels(Query, OuterRels3) :-
  optimizerOption(nestedRelations),
  Query =.. [from, _, Where],
  ( (
      Where =.. [where, InnerRels, _],
      removeCorrelatedPreds(Query, _, Preds)
    ) ->
    (
      usedRels(Preds, Rels),
      findall(Rel, (member(Rel, Rels), not(member(Rel, InnerRels))), OuterRels)
    )
  ;
    % no sql predicates
    (
      OuterRels=[],
      makeList(Where, InnerRels)
    )
  ),

  findall(ParentQueryRel, (
      member(Rel2, InnerRels),
      Rel2=rel(T, _),
      %T=..[arel|_],
      T=..[irrel,arel|_],
      toParentQueryRel(Rel2, ParentQueryRel)
    ), OuterRels2),
  append(OuterRels, OuterRels2, OuterRels3),
  dm(subqueryDebug, ['\ncorrelationRels_OuterRels 1: ', OuterRels3]),
	!.
% NVK ADDED NR END

correlationRels(all(Query), OuterRels) :-
  correlationRels(Query, OuterRels).

correlationRels(any(Query), OuterRels) :-
  correlationRels(Query, OuterRels).

/*
NVK ADDED NR
Note that this arel attribute can only come from the direct surrounded query
because otherwise the optimization strategy can't handle this case.
The error that will occur then is that the pog creation fails.

*/
toParentQueryRel(Rel, ParentQueryRel) :-
  Rel=rel(T, _),
  T=irrel(_, _, _, _, _, _, TypeSpec),
	TypeSpec=arel(RelVar:_, _, _, _, _), 
  findBinding(RelVar, variable(RelVar, ParentQueryRel), _),
	!.


toParentQueryRel(Rel, ParentQueryRel) :-
  Rel=rel(T, _),
  T=irrel(_, _, _, _, _, _, TypeSpec),
	TypeSpec=arel((*):AttrDC, _, _, _, _), 

  findAttrLists(attribute, _, *, Rel2, AttrDesc),
  nrSimpleFindAttribute(AttrDC, AttrDesc, _),
	Rel2=ParentQueryRel, !.

toParentQueryRel(Rel, ParentQueryRel) :-
	!,
  throw(
    error_Internal(subqueries_toParentQueryRel(Rel, ParentQueryRel)::failed)).
% NVK ADDED NR END

/*

Collect the relations used in the given predicate list.

*/

usedRels([], []).
usedRels([ pr(_, Rel) | Rest ], [ Rel | RelRest]) :-
  usedRels(Rest, RelRest).

usedRels([ pr(_, Rel1, Rel2) | Rest ], [ Rel1 | [ Rel2 | RelRest ]]) :-
  usedRels(Rest, RelRest).

/*

---- removeCorrelatedPreds(+Query1, -Query2, -Preds)
----

Strip ~Query1~ of all correlated predicates, i.e. all predicates referencing
relations not contained in the from-clause. ~Query2~ is the stripped query,
~Preds~ is the list of predicates removed from ~Query1~.

*/

removeCorrelatedPreds(Query groupby Attrs, Query2 groupby Attrs, Pred) :-
  removeCorrelatedPreds(Query, Query2, Pred).

removeCorrelatedPreds(Query orderby Attrs, Query2 orderby Attrs, Pred) :-
  removeCorrelatedPreds(Query, Query2, Pred).

removeCorrelatedPreds(Query first N, Query2 first N, Pred) :-
  removeCorrelatedPreds(Query, Query2, Pred).

removeCorrelatedPreds(Query last N, Query2 last N, Pred) :-
  removeCorrelatedPreds(Query, Query2, Pred).


removeCorrelatedPreds(select Attrs from Rels where [], 
    select Attrs from Rels, []) :-
  ground(Attrs),
  ground(Rels).

removeCorrelatedPreds(Select from Rels where [ Pred | Rest ], Query2, 
    CorrelatedPreds) :-
  removeCorrelatedPred(Rels, Pred, Pred2, CorrelatedPred),
  removeCorrelatedPreds(Select from Rels where Rest, Query, CorrelatedRest),
  CorrelatedPreds1 = [ CorrelatedPred | CorrelatedRest ],
  flatten(CorrelatedPreds1, CorrelatedPreds),
  appendPred(Query, Pred2, Query2).

% NVK ADDED NR
removeCorrelatedPreds(select Attrs from Rels, select Attrs from Rels, []).
% NVK ADDED NR END

removeCorrelatedPred(Rels, pr(P, Rel), pr(P, Rel), []) :-
  member(Rel, Rels).

removeCorrelatedPred(Rels, pr(P, Rel1, Rel2), pr(P, Rel1, Rel2), []) :-
  member(Rel1, Rels),
  member(Rel2, Rels).

removeCorrelatedPred(_, Pred, [], Pred).

/*
---- appendQuery(+Query, +PredList, -Query2)
----

Append predicates to the predicate list of ~Query~.

*/

appendPred(Query, [], Query).
appendPred(Select from Rels where Preds, Pred, Select from Rels where Preds2) :-
  makeList(Pred, PredList),
  makeList(Preds, PredsList),
  append(PredList, PredsList, Preds2).

appendPred(Select from Rels, Pred, Select from Rels where [Pred]).

/*

Add to predicates to an execution plan. This predicate is used to reappend
correlated predicates after the translation of the stripped query to an
execution plan.

*/

addCorrelatedPreds(Plan, Plan, []).

addCorrelatedPreds(Plan, PlanOut, [ Pred | Rest ]) :-
  addCorrelatedPred(Plan, Plan2, Pred),
  addCorrelatedPreds(Plan2, PlanOut, Rest).

addCorrelatedPred(simpleAggrNoGroupby(Op, Plan, Attr),
  simpleAggrNoGroupby(Op, filter(Plan, P), Attr), pr(P, _, _)).

addCorrelatedPred(consume(project(Stream, Attrs)),
  consume(project(filter(Stream, P), Attrs)), pr(P, _, _)).
% NVK ADDED NR
addCorrelatedPred(aconsume(project(Stream, Attrs)),
  aconsume(project(filter(Stream, P), Attrs)), pr(P, _, _)).

addCorrelatedPred(aconsume(Plan),
  aconsume(filter(Plan, P)), pr(P, _, _)).
% NVK ADDED NR END


addCorrelatedPred(consume(Plan),
  consume(filter(Plan, P)), pr(P, _, _)).
addCorrelatedPred(count(project(Stream, Attrs)),
  count(project(filter(Stream, P), Attrs)), pr(P, _, _)).

addCorrelatedPred(count(Plan),
  count(filter(Plan, P)), pr(P, _, _)).

addCorrelatedPred(Plan,
  filter(Plan, P), pr(P, _, _)).

/*

Finds the correct renaming for a relation in an execution plan.

*/

isNameOf(Param, Rel) :-
  stack(streamRel, SR),
  stack(streamName, SN),
  length(SR, N),
  length(SN, N),
  peak(streamName, Param),
  peak(streamRel, Rel).

isNameOf(txx1, Rel) :-
  peak(streamName, txx1),
  peak(streamRel, 2, Rel).

isNameOf(Param, Rel) :-
  Param \= txx1,
  peak(streamName, Param),
  peak(streamRel, Rel).

isNameOf(Param, Rel) :-
  stack(streamRel, SR),
  stack(streamName, SN),
  length(SR, N),
  length(SN, N1),
  N1 < N,
  peak(streamName, Param),
  peak(streamRel, 2, Rel).

isNameOf(Param, Rel) :-
  peak(streamName, 2, Param),
  peak(streamRel, 2, Rel).

/*

Apply renaming to the correlated predicates, so that it matches the renaming of the
base relation.

*/

transformCorrelatedPreds(_, _, [], []).

transformCorrelatedPreds(Rels, Param, [pr(P, Rel1, Rel2)], [Pred3]) :-
  not(member(Rel1, Rels)),
  not(member(Rel2, Rels)),
  isNameOf(Param, Rel1),
  isNameOf(Param, Rel2),
  transformPred(pr(P, Rel1, Rel2), Param, 1, Pred2),
  transformPred(Pred2, Param, 2, Pred3).

transformCorrelatedPreds(Rels, Param, [pr(P, Rel1, Rel2)], [Pred2]) :-
  not(member(Rel1, Rels)),
  isNameOf(Param, Rel1),
  transformPred(pr(P, Rel1, Rel2), Param, 1, Pred2).

transformCorrelatedPreds(Rels, Param, [pr(P, Rel1, Rel2)], [Pred2]) :-
  not(member(Rel2, Rels)),
  isNameOf(Param, Rel2),
  transformPred(pr(P, Rel1, Rel2), Param, 2, Pred2).

transformCorrelatedPreds(_, _, [pr(P, Rel1, Rel2)], [pr(P, Rel1, Rel2)]).


transformCorrelatedPreds(Rels, Param, [pr(P, Rel1, Rel2)], [_]) :-
  write('Rels: '), write(Rels), nl,
  write('Param: '), write(Param), nl,
  write('P: '), write(P), nl,
  write('Rel1: '), write(Rel1), nl,
  write('Rel2: '), write(Rel2), nl, !,
  ( firstStream(A) ; true ),
  write('firstStream: '), write(A), nl,
  throw(error_Internal(subqueries_transformCorrelatedPreds::notImplemented::_)).

transformCorrelatedPreds(Rels, Param, [Pred | Rest], [Pred2 | Rest2]) :-
  transformCorrelatedPreds(Rels, Param, [Pred], [Pred2]),
  transformCorrelatedPreds(Rels, Param, Rest, Rest2).

/*

Apply the correct renaming to an attribute expression, so that it matches
the renaming of the base relations.

*/

% If there is a name ~Param~ for relation Rel, Rel is not contained in
% list Rels and Attr is attribute of Rel, then rename Attr with ~Param~
% case aliased attribute, Rel is first stream
transformAttrExpr(attr(Var:Attr, Arg, Case), Param, _,
    attribute(Param, attrname(attr(Var:Attr, Arg, Case))), Rels) :-
  peak(streamRel, rel(Rel, Var)),
  isNameOf(Param, rel(Rel, Var)),
  not(member(rel(Rel, Var), Rels)),
  findAttribute(Attr, rel(Rel, Var)).

% If there is a name ~Param~ for relation Rel, Rel is not contained in
% list Rels and Attr is attribute of Rel, then rename Attr with ~Param~
% case aliased attribute, Rel is second stream
transformAttrExpr(attr(Var:Attr, Arg, Case), Param, _,
    attribute(Param, attrname(attr(Var:Attr, Arg, Case))), Rels) :-
  peak(streamRel, 2, rel(Rel, Var)),
  isNameOf(Param, rel(Rel, Var)),
  not(member(rel(Rel, Var), Rels)),
  findAttribute(Attr, rel(Rel, Var)).

% If there is a name ~Param~ for relation Rel, Rel is not contained in
% list Rels and Attr is attribute of Rel, then rename Attr with ~Param~
% case Rel is first stream
transformAttrExpr(attr(Attr, Arg, Case), Param, _,
    attribute(Param, attrname(attr(Attr, Arg, Case))), Rels) :-
  peak(streamRel, rel(Rel, Var)),
  isNameOf(Param, rel(Rel, Var)),
  not(member(rel(Rel, Var), Rels)),
  findAttribute(Attr, rel(Rel, Var)).

% If there is a name ~Param~ for relation Rel, Rel is not contained in
% list Rels and Attr is attribute of Rel, then rename Attr with ~Param~
% case Rel is second stream
transformAttrExpr(attr(Attr, Arg, Case), Param, _,
    attribute(Param, attrname(attr(Attr, Arg, Case))), Rels) :-
  peak(streamRel, 2, rel(Rel, Var)),
  isNameOf(Param, rel(Rel, Var)),
  not(member(rel(Rel, Var), Rels)),
  findAttribute(Attr, rel(Rel, Var)).

transformAttrExpr(attribute(Param, attrname(attr(Attr, Arg, Case))),
  _, _, attribute(Param, attrname(attr(Attr, Arg, Case))), _) :- !.

transformAttrExpr([], _, _, [], _).
transformAttrExpr([Arg1|Args1], Param, Arg, [Arg1T|Args1T], Rels) :-
  transformAttrExpr(Arg1, Param, Arg, Arg1T, Rels),
  transformAttrExpr(Args1, Param, Arg, Args1T, Rels).

transformAttrExpr(Pred, Param, Arg, Pred2, Rels) :-
  compound(Pred),
  not(is_list(Pred)),
  Pred =.. [T|Args],
  transformAttrExpr(Args, Param, Arg, Args2, Rels),
  Pred2 =.. [T|Args2].

transformAttrExpr(Pred, _, _, Pred, _).

findAttribute(Attr, rel(Rel, *)) :-
  isAttributeOf(Attr, Rel).

findAttribute(Attr, rel(Rel, Var)) :-
  isAttributeOf(Attr, Rel as Var).

% renaming for selectivity queries, handling of operator  ~not in~.
subqueryTransformPred(Pred, T, Arg, Pred3) :-
  Pred =.. [not, Attr, InExpr],
  InExpr =.. [in, Query],
  Pred1 =.. [in, Attr, Query],
  subqueryTransformPred(Pred1, T, Arg, Pred2),
  Pred2 =.. [in, Attr2, Query2],
  InExpr2 =.. [in, Query2],
  Pred3 =.. [not, Attr2, InExpr2].

% renaming for selectivity queries
subqueryTransformPred(Pred, T, Arg, Pred2) :-
  Pred =.. [Op, Attr, Query],
  isSubqueryOp(Op),
  extractQuery(Query, Query1),
  Query =.. [subquery, SQID, _, OuterRels], % NVK MODIFIED NR
  Query1 =.. [from, Select, Where],
  Where =.. [where, Rels, Preds],
  transformAttrExpr(Select, T, Arg, Select2, Rels),
  transformAttrExpr(Attr, T, Arg, Attr2, []),
  removeCorrelatedPreds(Query1, _, CorrelatedPreds),
  transformCorrelatedPreds(Rels, txx1, CorrelatedPreds, Preds2),
  dm(subqueryDebug,
    ['\nsubqueryTransformPred\n\tCorrelatedPreds: ', CorrelatedPreds,
     '\n\tPreds2: ', Preds2]),
  dm(subqueryDebug, ['\nPreds: ', Preds]),
  append(CorrelatedPreds, SimplePreds, Preds),
  dm(subqueryDebug, ['\nSimplePreds: ', SimplePreds]),
  append(Preds2, SimplePreds, PredList),
  dm(subqueryDebug, ['\nPredList: ', PredList]),
  Where2 =.. [where, Rels, PredList],
  Query2 =.. [from, Select2, Where2],
  Pred2 =.. [Op, Attr2, subquery(SQID, Query2, OuterRels)], % NVK MODIFIED NR
  clearQuery(Query).

subqueryTransformPred(attribute(Param, attrname(attr(Attr, Arg, Case))),
  _, Arg, attribute(Param, attrname(attr(Attr, Arg, Case)))) :- !.

transformPreds([], _, _, []).
transformPreds([Pred], Param, Arg, [Pred2]) :-
  transformPred(Pred, Param, Arg, Pred2).
transformPreds([ Pred | Rest ], Param, Arg, [ Pred2 | Rest2 ] ) :-
  not(is_list(Pred)),
  not(Pred = [[]]),
  transformPred(Pred, Param, Arg, Pred2),
  transformPreds(Rest, Param, Arg, Rest2).

simpleSubqueryPred(pr(P, A, B), Simple) :-
  simple1(P, A, B, Simple).

/*

Replacement of predicate ~simple(+P, +Rel1, +Rel2, -SimplePredicate)~ of file
~statistics.pl~ which handles subquery predicates correctly.

*/

simple1(attr(Var:Attr, 0, _), _, _, Rel:Attr) :-
  usedAttr(rel(Rel, Var), attr(Var:Attr, 0, _)),
  !.
simple1(attr(Attr, 0, _), _, _, Rel:Attr) :-
  usedAttr(rel(Rel, *), attr(Attr, 0, _)),
  !.

simple1(attr(Var:Attr, 1, _), _, _, Rel:Attr) :-
  usedAttr(rel(Rel, Var), attr(Var:Attr, 1, _)),
  !.

simple1(attr(Attr, 1, _), _, _, Rel:Attr) :-
  usedAttr(rel(Rel, *), attr(Attr, 1, _)),
  !.

simple1(attr(Var:Attr, 2, _), _, _,  Rel:Attr) :-
  usedAttr(rel(Rel, Var), attr(Var:Attr, 2, _)),
  !.

simple1(attr(Attr, 2, _), _, _, Rel:Attr) :-
  usedAttr(rel(Rel, *), attr(Attr, 2, _)),
  !.

simple1(dbobject(X),_,_,dbobject(X)) :- !.

simple1([], _, _, []) :- !.
simple1([A|Rest], Rel1, Rel2, [Asimple|RestSimple]) :-
  simple1(A,Rel1,Rel2,Asimple),
  simple1(Rest,Rel1,Rel2,RestSimple),
  !.
simple1(Term, Rel1, Rel2, Simple) :-
  compound(Term),
  Term =.. [Op|Args],
  simple1(Args, Rel1, Rel2, ArgsSimple),
  Simple =..[Op|ArgsSimple],
  !.

simple1(Term, _, _, Term).

/*

---- sampleSQ(+Rel, -SampleRel)
----

Returns the corresponding sample relation for selectivity queries for a
subquery predicate. At the moment the selection sample is used.

*/

sampleSQ([], []).

sampleSQ(rel(Rel, Var), rel(Rel2, Var)) :-
  not(sampleS(_, rel(Rel, Var))),
  not(sampleJ(_, rel(Rel, Var))),
  ensureSampleSexists(Rel),
  sampleS(rel(Rel, Var), rel(Rel2, Var)).

sampleSQ([ Rel | Rest ], [ Rel2 | Rest2 ]) :-
  sampleSQ(Rel, Rel2),
  sampleSQ(Rest, Rest2).

sampleSQ(Rel, Rel).

:- assert(maxSelCard(250000)).
:- assert(maxSampleCard(500)).

:- dynamic(currentLevel/1).

:- assert(currentLevel(0)).

ascendLevel :-
  currentLevel(L),
  L1 is L - 1,
  retract(currentLevel(_)),
  assert(currentLevel(L1)).

descendLevel :-
  currentLevel(L),
  L1 is L + 1,
  retract(currentLevel(_)),
  assert(currentLevel(L1)).

/*

Stack handling for outer relations and their names. This information
is needed  in the translation phase to apply the correct renaming to
attributes.

*/

streamName(Var) :-
  push(streamName, Var).

clearStreamName :-
  pop(streamName, _).

clearStreamName :-
  concat_atom(['Pop empty', ' stack streamName'], ErrMsg),
  throw(error_SQL(subqueries_streamName::malformedExpression::ErrMsg)).

clearStreamName(Var) :-
  streamName(Var, _),
  retract(streamName(Var, _)).

clearStreamName(_).

clearStreamNames :-
  retractall(streamName(_, _)).

streamRel(Rel) :-
  push(streamRel, Rel).

clearStreamRel :-
  pop(streamRel, _).

clearStreamRel.

clearStreamRel(Rel) :-
  peak(streamRel, Rel),
  !,
  pop(streamRel, Rel).

clearStreamRel(_).

clearStreamRel(Rel1, Rel2) :-
  peak(streamRel, Rel2),
  peak(streamRel, 2, Rel1),
  !,
  pop(streamRel, Rel2),
  pop(streamRel, Rel1).

clearStreamRel(_, _).

/*

Apply renaming to a subquery in a selectivity query.

*/

/*
Transforming Selection Predicates

---- transformQuery(+Rel, +Pred, +QueryIn, -QueryOut)
----

*/

% no transformation, if subquery handling is deactivated
transformQuery(_, _, Query, Query) :-
  not(optimizerOption(subqueries)).

% flag query as selectivity query
transformQuery(_, Pred, Query, Query2) :-
  optimizerOption(subqueries),
  assert(selectivityQuery(Pred)),
  isSubqueryPred1(Pred),
%  streamRel(Rel),
  transformPlan(Query, Query2).

% no transformation for simple predicates
transformQuery(_, _, Query, Query).

% report error
transformQuery(_, _, Q, _) :-
  not(optimizerOption(subqueries)),
  throw(error_Internal(subqueries_transformQuery(Q)::notImplemented::_)).

/*
Transforming Join Predicates

---- transformQuery(+Rel1, +Rel2, +Pred, +QueryIn, JoinSize, -QueryOut)
----

*/

% case loopsel for nested selection predicates
transformQuery(_, _, _, count(loopsel(Query, Fun)), JoinSize,
    count(loopsel(head(Query, JoinSize), Fun))) :-
  not(optimizerOption(subqueries)).

transformQuery(_, _, _, count(timeout(loopsel(Query, Fun), T)), JoinSize,
    count(timeout(loopsel(head(Query, JoinSize), Fun), T))) :-
  not(optimizerOption(subqueries)).

% case loopjoin for nested join predicates
transformQuery(_, _, _, count(filter(counter(loopjoin(Q, Fun), C), P)),
    JoinSize,
    count(filter(counter(loopjoin(head(Q, JoinSize), Fun), C), P))) :-
  not(optimizerOption(subqueries)).

transformQuery(_,_,_,count(timeout(filter(counter(loopjoin(Q, Fun), C), P), T)),
    JoinSize,
    count(timeout(filter(counter(loopjoin(head(Q,JoinSize),Fun), C), P), T))) :-
  not(optimizerOption(subqueries)).

% XRIS: case symmjoin for nested non-bbox selection predicates
transformQuery(_, _, _,
    count(symmjoin(Query1,Query2,Pred)), JoinSize,
    count(symmjoin(head(Query1,JoinSize),Query2,Pred))) :-
  not(optimizerOption(subqueries)).

transformQuery(_, _, _,
    count(timeout(symmjoin(Query1,Query2,Pred),T)), JoinSize,
    count(timeout(symmjoin(head(Query1,JoinSize),Query2,Pred),T))) :-
  not(optimizerOption(subqueries)).

% XRIS: case symmjoin for nested bbox selection predicates
transformQuery(_, _, _,
    count(filter(counter(
      symmjoin(Query1,Query2,P1),C1),P2)), JoinSize,
    count(filter(counter(
      symmjoin(head(Query1,JoinSize),Query2,P1),C1),P2))) :-
  not(optimizerOption(subqueries)).

transformQuery(_, _, _,
    count(timeout(filter(counter(
      symmjoin(Query1,Query2,P1),C1),P2),T)), JoinSize,
    count(timeout(filter(counter(
      symmjoin(head(Query1,JoinSize),Query2,P1),C1),P2),T))) :-
  not(optimizerOption(subqueries)).

% error: No more cases for not(optimizerOption(subqueries))
transformQuery(_, _, _, Q, JS, _) :-
  not(optimizerOption(subqueries)),
  throw(error_Internal(subqueries_transformQuery(Q, JS)::notImplemented::_)).

% case loopsel for nested selection predicates
transformQuery(_, _, Pred, count(loopsel(Query, Fun)), JoinSize,
    count(loopsel(head(Query, JoinSize), Fun))) :-
  not(isSubqueryPred1(Pred)).

transformQuery(_, _, Pred, count(timeout(loopsel(Query, Fun),T)), JoinSize,
    count(timeout(loopsel(head(Query, JoinSize), Fun),T))) :-
  not(isSubqueryPred1(Pred)).

% XRIS: case symmjoin for nested non-bbox selection predicates
transformQuery(_, _, Pred, count(symmjoin(Q1,Q2,P)), JoinSize,
    count(symmjoin(head(Q1,JoinSize),Q2,P))) :-
  not(isSubqueryPred1(Pred)).

transformQuery(_, _, Pred, count(timeout(symmjoin(Q1,Q2,P),TO)), JoinSize,
    count(timeout(symmjoin(head(Q1,JoinSize),Q2,P),TO))) :-
  not(isSubqueryPred1(Pred)).

% XRIS: case symmjoin for nested bbox selection predicates
transformQuery(_, _, Pred,
    count(filter(counter(symmjoin(Q1,Q2,P2),C1),P1)), JoinSize,
    count(filter(counter(
      symmjoin(head(Q1,JoinSize),Q2,P2),C1),P1))) :-
  not(isSubqueryPred1(Pred)).

transformQuery(_, _, Pred,
    count(timeout(filter(counter(symmjoin(Q1,Q2,P2),C1),P1),TO)), JoinSize,
    count(timeout(filter(counter(
      symmjoin(head(Q1,JoinSize),Q2,P2),C1),P1),TO))) :-
  not(isSubqueryPred1(Pred)).

% case loopjoin for nested join predicates
transformQuery(_, _, Pred, count(filter(counter(loopjoin(Q, Fun), C), P)),
    JoinSize,
    count(filter(counter(loopjoin(head(Q, JoinSize), Fun), C), P))) :-
  not(isSubqueryPred1(Pred)).

transformQuery(_,_,Pred,count(timeout(filter(counter(loopjoin(Q, Fun),C),P),T)),
    JoinSize,
    count(timeout(filter(counter(loopjoin(head(Q, JoinSize), Fun), C), P),T))):-
  not(isSubqueryPred1(Pred)).

% report error
transformQuery(_, _, _, Q, JS, _) :-
  not(optimizerOption(subqueries)),
  throw(error_Internal(subqueries_transformQuery(Q, JS)::notImplemented::_)).

% return JoinSize, which is calculated depending on the
% count of relations used in this query
transformQuery(_, _, Pred, Query, JoinSize, Query2) :-
  optimizerOption(subqueries),
  maxSampleCard(C),
  retractall(maxSampleCard(_)),
	NewSampleCard is min(JoinSize, C),
  assert(maxSampleCard(NewSampleCard)),
  transformPlan(Query, Query2),
	sampleSize(S),
	retractall(realJoinSize(Pred, _)),
	assert(realJoinSize(Pred, S)).

:- dynamic(realJoinSize/2).

clearSelectivityQuery(_, Pred) :-
  !,
  retractall(selectivityQuery(Pred)).

clearSelectivityQuery(_, _, Pred) :-
  !,
  retractall(selectivityQuery(Pred)).

/*

Predicates to count the number of relations which are part of a selectivity query.
The resulting number is used to calculate the size used in sampling.

*/

selectivityRel :-
  selectivityRels(N),
  retractall(selectivityRels(_)),
  N1 is N + 1,
  assert(selectivityRels(N1)).

selectivityRel :-
  retractall(selectivityRels(_)),
  assert(selectivityRels(1)).

/*

Apply the sample sizes calculated to the selectivity query. Maximum cardinality
for a selectivity query is ~maxSelCard(selCard)~, maximum cardinality for a sample
relation is ~maxSampleCard(sampleCard)~. The cardinality used for sampling is
calculated as $\sqrt[N]{selCard}$, where N is the number of relations used in the
selectivity query.

*/

transformPlan(Plan, Plan) :-
  not(selectivityQuery(_)).

transformPlan(Plan, Plan2) :-
  sampleSize(C),
  maxSampleCard(C1),
  C < C1,
  transformPlan1(Plan, Plan2, C),
  retractall(sampleSize(_)).

% calculate sampleSize
transformPlan(Plan, Plan2) :-
  retractall(selectivityRels(_)),
  transformPlan1(Plan, Plan2, C),
  selectivityRels(N),
  maxSelCard(Max),
  maxSampleCard(SampleSize),
  A is 1 / N,
  B is Max ** A,
  C is min(SampleSize, floor(B)),
  retractall(sampleSize(_)),
  assert(sampleSize(C)).

transformPlan1([], [], _).

transformPlan1(pr(P, R1, R2), pr(P, R1, R2), _).
transformPlan1(pr(P, R1), pr(P, R1), _).

% Relation found, case feed, apply ~head~ operator
transformPlan1(feed(Rel), head(feed(Rel2), C), C) :-
  sampleSQ(Rel, Rel2),
  selectivityRel.

% Relation found, case feedproject, apply ~head~ operator
transformPlan1(feedproject(Rel, Attrs),
    head(feedproject(Rel2, Attrs), C), C) :-
  sampleSQ(Rel, Rel2),
  selectivityRel.

% Relation found, only count relation
transformPlan1(rel(Rel, Var), rel(Rel, Var), _) :-
  selectivityRel.

% NVK ADDED NR
% This is the case for arel access where there is no rel-term anymore available.
transformPlan1(afeed(Rel), head(afeed(Rel), C), C) :-
  selectivityRel.
% NVK ADDED NR END

transformPlan1([ Plan | Rest ], [Plan2 | Rest2], C) :-
  transformPlan1(Plan, Plan2, C),
  transformPlan1(Rest, Rest2, C).

transformPlan1(Plan, Plan2, C) :-
  compound(Plan),
  not(is_list(Plan)),
  Plan =.. [Op | Args],
  transformPlan1(Args, Args2, C),
  Plan2 =.. [Op | Args2].

transformPlan1(Plan, Plan, _).

getSubqueryTypeTree(subquery(_, Query, _), % NVK MODIFIED
    OuterRels1, [subquery, TypeTree, DCType]) :-
  newVariable(T),
  subquery_to_plan(Query, simpleAggrNoGroupby(Op, Plan, Attr), T),
  Plan2 =.. [Op, Plan, Attr],
  getTypeTree(Plan2, OuterRels1, TypeTree),
  TypeTree = [_, _, DCType],
  !.

subqueryPredRel(_).

/*

Assert information about the position of outer relations.

*/
joinRel([], _).

joinRel(feed(Rel), 1) :-
%  retract(streamRel(_)),
  assert(streamRel(Rel, 1)).

joinRel(feed(Rel), 2) :-
  assert(streamRel(Rel, 2)).

joinRel(feedproject(Rel, _), 1) :-
%  retractall(streamRel(_)),
  assert(streamRel(Rel, 1)).

joinRel(feedproject(Rel, _), 2) :-
  assert(streamRel(Rel, 2)).

joinRel([ Stream | Rest ], Arg) :-
  joinRel(Stream, Arg),
  joinRel(Rest, Arg).

joinRel(Stream, Arg) :-
  compound(Stream),
  not(is_list(Stream)),
  Stream =.. [_ | Args],
  joinRel(Args, Arg).

joinRel(_ ,_).

joinRels(_, _).

joinRels(Arg1, Arg2) :-
  joinRel(Arg1, 1),
  joinRel(Arg2, 2).

/*

Extract a stream plan and its schema from an execution plan.

*/
extractStream(consume(project(StreamPlan, QueryAttr)),
  StreamPlan, QueryAttr).

extractStream(consume(StreamPlan), StreamPlan, QueryAttr) :-
  extractQueryAttr(StreamPlan, QueryAttr).

% NVK ADDED NR
% May still fail for tables or arel's with only one column.
extractStream(aconsume(project(StreamPlan, QueryAttr)), StreamPlan, QueryAttr).
extractStream(aconsume(StreamPlan), StreamPlan, QueryAttr) :-
  extractQueryAttr(StreamPlan, QueryAttr).
% NVK ADDED NR END

extractQueryAttr(project(_, QueryAttr), QueryAttr).

extractQueryAttr(Stream, QueryAttr) :-
  Stream =.. [filter, Stream2, _],
  extractQueryAttr(Stream2, QueryAttr).

extractQueryAttr(_, _) :- !, fail.

/*
18.1.3 Translation of nested queries

Translate a subquery into an execution plan. Includes handling of
correlated predicates.

*/

% case subquery of selection predicate
subquery_to_plan(Query, Plan3, T) :-
  ground(Query),
  Query =.. [from, _, Where],
  Where =.. [where, Rels, _],
  removeCorrelatedPreds(Query, Query1, Preds), !,
  ground(Query1),
  dm(subqueryDebug, ['\nQuery1: ', Query1,
                     '\nCorrelatedPreds: ', Preds]),
  Query1 =.. [from, Select1, Where1],
  transformAttrExpr(Select1, T, 2, Select2, []),
  Query2 =..[from, Select2, Where1],
  descendLevel,
  %putOptimizerState, % NVK ADDED
  queryToPlan(Query2, Plan, _), !,
  %restoreOptimizerState, % NVK ADDED
  ascendLevel,
  dm(subqueryDebug, ['\nPlan: ', Plan]),
  transformCorrelatedPreds(Rels, T, Preds, Preds2),
  addCorrelatedPreds(Plan, Plan2, Preds2),
  dm(subqueryDebug, ['\nPlan2: ', Plan2]),
  transformPlan(Plan2, Plan3),
  dm(subqueryDebug, ['\nPlan3: ', Plan3]).

subquery_to_plan(Query, Plan2, _) :-
  ground(Query),
  descendLevel,
  queryToPlan(Query, Plan, _), !,
  ascendLevel,
  ensure(transformPlan(Plan, Plan2)),
  dm(subqueryDebug, ['\nPlan2: ', Plan2]).

% case subquery of join predicate, apply both names
subquery_to_plan(Query, Plan3, T1, T2) :-
  ground(Query),
  Query =.. [from, _, Where],
  Where =.. [where, Rels, _],
  removeCorrelatedPreds(Query, Query1, Preds), !,
  ground(Query1),
  dm(subqueryDebug, ['\nQuery1: ', Query1,
                     '\nCorrelatedPreds: ', Preds]),
  Query1 =.. [from, Select1, Where1],
  transformAttrExpr(Select1, T1, 2, Select2, []),
  transformAttrExpr(Select2, T2, 1, Select3, []),
  Query2 =..[from, Select3, Where1],
  descendLevel,
  queryToPlan(Query2, Plan, _), !,
  ascendLevel,
  dm(subqueryDebug, ['\nPlan: ', Plan]),
  transformCorrelatedPreds(Rels, T1, Preds, Preds2),
  transformCorrelatedPreds(Rels, T2, Preds2, Preds3),
  addCorrelatedPreds(Plan, Plan2, Preds3),
  dm(subqueryDebug, ['\nPlan2: ', Plan2]),
  transformPlan(Plan2, Plan3),
  dm(subqueryDebug, ['\nPlan3: ', Plan3]).

/*

18.1.4 Translation of a subquery into executable syntax.

*/

subquery_to_atom(Query, QueryAtom) :-
  ground(Query),
  subquery_to_plan(Query, Plan),
  plan_to_atom(Plan, QueryAtom),
  dm(subqueryDebug, ['\nQueryAtom: ', QueryAtom]).

/*

Expressions over attributes

*/

% case atomic value
subquery_expr_to_plan(Expr, _, Expr) :-
  atomic(Expr).

% case attribute value, applies renaming
subquery_expr_to_plan(attr(A, Arg, Case), _,
    attribute(T, attrname(attr(A, Arg, Case)))) :-
  peak(streamRel, rel(Rel, _)),
  isAttributeOf(A, Rel),
  peak(streamName, T).

subquery_expr_to_plan(attr(A, Arg, Case), _,
    attribute(T, attrname(attr(A, Arg, Case)))) :-
  peak(streamRel, 2, rel(Rel, _)),
  isAttributeOf(A, Rel),
  peak(streamName, 2, T).

subquery_expr_to_plan(attr(A, Arg, Case), _,
    attribute(T, attrname(attr(A, Arg, Case)))) :-
  peak(streamRel, 2, rel(Rel, _)),
  isAttributeOf(A, Rel),
  not(peak(streamName, 2, _)),
  peak(streamName, T).

% general translation of attribute expression
subquery_expr_to_plan(Expr, Param, Expr2) :-
  transformAttrExpr(Expr, Param, 1, Expr1, []),
  transformAttrExpr(Expr1, Param, 2, Expr2, []).

subquery_expr_to_plan(A, B, C) :-
  concat_atom(['Not Implemented'], Msg),
  throw(error_Internal(subqueries_Expr(A, B, C)::notImplemented::Msg)).

% NVK ADDED NR
subqueryContainsWhere(subquery(_, Query, _)) :-
  Query =.. [from, _, Where],
  Where =.. [where, _, _].
% NVK ADDED NR END

/*

Extract query from subquery and put used relations on stack.

*/
extractQuery(subquery(_, Query, [Rel]), Query) :- % NVK MODIFIED NR
  !,
  streamRel(Rel).

extractQuery(subquery(_, Query, [Rel1, Rel2]), Query) :- % NVK MODIFIED NR
  !,
  streamRel(Rel1),
  streamRel(Rel2).

clearQuery(subquery(_, _, [Rel])) :- % NVK MODIFIED NR
  !,
  clearStreamRel(Rel).

clearQuery(subquery(_, _, [Rel1, Rel2])) :- % NVK MODIFIED NR
  !,
  clearStreamRel(Rel1, Rel2).

/*

Translate a list of values into kernel syntax for lists.

*/

secondo_list_to_atom(CommaList, Result) :-
  CommaList =.. [(,), X, Xs],
  plan_to_atom(X, XAtom),
  secondo_list_to_atom((Xs), XsAtom),
  concat_atom([XAtom, ' ', XsAtom], '', Result),
  !.

secondo_list_to_atom(X, Result) :-
  plan_to_atom(X, Result),
  !.

secondo_list_to_atom([], '') :- !.

% floating point constants
subquery_plan_to_atom(Data, Result) :-
  float(Data),
	format(atom(Result), '~10f~n', Data).

% in expression with constant list
subquery_plan_to_atom(X, Result) :-
  X =.. [in, Attr, ValueList],
  ValueList =.. [(,) | _],
  constantType(Attr, Type),
  plan_to_atom(Attr, AttrAtom),
  secondo_list_to_atom(ValueList, VLAtom),
  concat_atom([AttrAtom, ' in [const set(', Type, ') value (', VLAtom, ')]'], 
    Result),
  !.

/*

Translation of outerjoin expressions, only binary operators with
attributes are currently considered.

*/

% outerjoin, equi-join case
subquery_plan_to_atom(outerjoin(=, JoinAttr1, JoinAttr2), Result) :-
  plan_to_atom(JoinAttr1, AAtom1),
  plan_to_atom(JoinAttr2, AAtom2),
  concat_atom([' smouterjoin[', AAtom1, ',', AAtom2, ']'], Result),
  !.

% outerjoin, general case
subquery_plan_to_atom(outerjoin(Op, attrname(JoinAttr1), attrname(JoinAttr2)), 
    Result) :-
  ( outerjoinCommuted
    -> Pred =.. [Op, JoinAttr2, JoinAttr1]
    ; Pred =.. [Op, JoinAttr1, JoinAttr2] ),
  retractall(outerjoinCommuted),
  consider_Arg2(Pred, Pred2),    % transform second arg/3 to arg2/3
  plan_to_atom(Pred2, PAtom),
  concat_atom([' symmouterjoin[', PAtom, ']'], Result),
  !.

/*

The translation of nested predicates uses a mapping function
to implement nested-iteration semantics. Parameter(s) to this
function are automaticall renamed. Special handling for subqueries
using more than one outer relation is necessary. Such predicates
are a form of general join predicate an need access to both streams.

*/

% case nested predicate is join predicate
subquery_plan_to_atom(Pred, Result) :-
  ground(Pred),
  isJoinPred(Pred),
  dm(subqueryDebug, ['\nisJoinPred!\n']),
  Pred =.. [Op, Attr, Query],
  extractQuery(Query, Query1),
  isSubqueryOp(Op),
  Query1 =.. [from | _],
  dm(subqueryDebug, ['\n',
           '\nisJoinPred\n\n\t']),
  dc(subqueryDebug, write_canonical(Pred)),
  dm(subqueryDebug, ['\nOp: ', Op,
           '\nAttr: ', Attr,
           '\nQuery: ', Query1]),
  newTempRel(T1),
  streamName(T1),
  newTempRel(T2),
  streamName(T2),
  Query=subquery(SQID, _, _), % NVK ADDED
  restoreSQID(SQID), 				% NVK ADDED
  subquery_to_plan(Query1, QueryPlan, T1, T2),
  transformAttrExpr(Attr, T1, 1, Attr3, []),
  transformAttrExpr(Attr3, T2, 2, Attr4, []),
  ResultPlan =.. [Op, Attr4, QueryPlan],
  dm(subqueryDebug, ['\nQueryPlan: ', QueryPlan]),
  plan_to_atom(fun([param(T1, tuple), param(T2, tuple2)], ResultPlan), Result),
  setSQIDToPrevious(SQID), % NVK ADDED
  clearStreamName,
  clearStreamName,
  clearQuery(Query).

% case join predicate with not in expression
subquery_plan_to_atom(Pred, Result) :-
  ground(Pred),
  isJoinPred(Pred),
  dm(subqueryDebug, ['\nisJoinPred!\n']),
  Pred =.. [not, Attr, InExpr],
  InExpr =.. [in, Query],
  extractQuery(Query, Query1),
  Query1 =.. [from | _],
  dm(subqueryDebug, ['\n',
           '\nisJoinPred\n\n\t']),
  dc(subqueryDebug, write_canonical(Pred)),
  dm(subqueryDebug, ['\nAttr: ', Attr,
           '\nQuery: ', Query1]),
  newTempRel(T1),
  streamName(T1),
%  assert(firstStream(T1)),
  newTempRel(T2),
  streamName(T2),
  Query=subquery(SQID, _, _), % NVK ADDED
  restoreSQID(SQID), 				% NVK ADDED
  subquery_to_plan(Query1, QueryPlan, T1, T2),
  extractStream(QueryPlan, StreamPlan, QueryAttr),
  transformAttrExpr(Attr, T1, 1, Attr3, []),
  transformAttrExpr(Attr3, T2, 2, Attr4, []),
  ResultPlan =.. 
    [in, Attr4, collect_set(projecttransformstream(StreamPlan, QueryAttr))],
  dm(subqueryDebug, ['\nQueryPlan: ', QueryPlan]),
  plan_to_atom(fun([param(T1, tuple), param(T2, tuple2)], not(ResultPlan)), 
    Result),
  setSQIDToPrevious(SQID), % NVK ADDED
  clearStreamName,
  clearStreamName,
  clearQuery(Query).

% case simple nested predicate with not in
subquery_plan_to_atom(Pred, Result) :-
  ground(Pred),
  Pred =.. [not, Attr, InExpr],
  InExpr =.. [in, Query],
  Pred2 =.. [in, Attr, Query],
  extractQuery(Query, Query1),
  Query1 =.. [from | _],
  dm(subqueryDebug, ['\n\n\n\n',
           '\nsubquery_plan_to_atom\n\n\t']),
  dc(subqueryDebug, write_canonical(Pred2)),
  dm(subqueryDebug, ['\nAttr: ', Attr,
           '\nQuery: ', Query1, '\n\n\n\n']),
%  newTempRel(T),
  Query=subquery(SQID, _, _), % NVK ADDED
  restoreSQID(SQID), 				% NVK ADDED
  newAlias(T),
  streamName(T),
%  subquery_to_plan(Query, consume(project(StreamPlan, QueryAttr)), T),
  subquery_to_plan(Query1, QueryPlan, T),
  extractStream(QueryPlan, StreamPlan, QueryAttr),
  dm(subqueryDebug, ['\nStreamPlan: ', StreamPlan]),
  subquery_expr_to_plan(Attr, T, Attr2),
  ResultPlan =.. 
    [in, Attr2, collect_set(projecttransformstream(StreamPlan, QueryAttr))],
  plan_to_atom(fun([param(T, tuple)], not(ResultPlan)), Result),
  setSQIDToPrevious(SQID), % NVK ADDED
  clearStreamName,
  clearQuery(Query).

% case simple nested predicate with in
subquery_plan_to_atom(Pred, Result) :-
  ground(Pred),
  Pred =.. [in, Attr, Query],
  extractQuery(Query, Query1),
  Query1 =.. [from | _],
  dm(subqueryDebug, ['\n\n\n\n',
           '\nsubquery_plan_to_atom\n\n\t']),
  dc(subqueryDebug, write_canonical(Pred)),
  dm(subqueryDebug, ['\nAttr: ', Attr,
           '\nQuery: ', Query, '\n\n\n\n']),
%  newTempRel(T),
  Query=subquery(SQID, _, _), % NVK ADDED
  restoreSQID(SQID), 				% NVK ADDED
  newAlias(T),
  streamName(T),
%  assert(firstStream(T)),
  subquery_to_plan(Query1, QueryPlan, T),
  % consume(project(StreamPlan, QueryAttr))
  extractStream(QueryPlan, StreamPlan, QueryAttr),
  dm(subqueryDebug, ['\nStreamPlan: ', StreamPlan]),
  subquery_expr_to_plan(Attr, T, Attr2),
  ResultPlan =.. 
    [in, Attr2, collect_set(projecttransformstream(StreamPlan, QueryAttr))],
  plan_to_atom(fun([param(T, tuple)], ResultPlan), Result),
  setSQIDToPrevious(SQID), % NVK ADDED
  clearStreamName,
  clearQuery(Query).

% case simple nested predicate, no aggregation
subquery_plan_to_atom(Pred, Result) :-
  ground(Pred),
  Pred =.. [Op, Attr, Query],
  isSubqueryOp(Op),
  extractQuery(Query, Query1),
  Query1 =.. [from | _],
  dm(subqueryDebug, ['\n',
           '=================',
           '=================',
           '=================',
           '=================',
           '\nsubquery_plan_to_atom\n\n\t']),
  dc(subqueryDebug, write_canonical(Pred)),
  dm(subqueryDebug, ['\nOp: ', Op,
           '\nAttr: ', Attr,
           '\nQuery: ', Query1]),
  Query=subquery(SQID, _, _), % NVK ADDED
  restoreSQID(SQID), 				% NVK ADDED
  newAlias(T),
  streamName(T),
  subquery_to_plan(Query1, QueryPlan, T),
  subquery_expr_to_plan(Attr, T, Attr2),
  dm(subqueryDebug, ['\nAttr2: ', Attr2]),
  ResultPlan =.. [Op, Attr2, QueryPlan],
  plan_to_atom(fun([param(T, tuple)], ResultPlan), Result),
  setSQIDToPrevious(SQID), % NVK ADDED
  clearStreamName,
  clearQuery(Query).

% case aggregated atttribute expression
subquery_plan_to_atom(AttrExpr, Result) :-
  ground(AttrExpr),
  AttrExpr =.. [Op, Attr],
  isAggregationOP(Op),
  dm(subqueryDebug, ['\nAttr: ', Attr]),
  Attr =.. [attribute, _, attrname(attr(Attr2, _, _))],
  dm(subqueryDebug, ['\nAttr: ', Attr,
             '\nAttr2: ', Attr2]),
  concat_atom([Op, '[', Attr2, ']'], Result).

% case nested predicate with exists
subquery_plan_to_atom(Expr, Result) :-
  ground(Expr),
  Expr =.. [exists, Query],
  % NVK ADDED NR: Pretest with no side effects (unlike extractQuery).
  subqueryContainsWhere(Query),
  % NVK ADDED NR END
  extractQuery(Query, Query1),
  Query1 =.. [from, _, Where],
  Where =.. [where | _],
  removeCorrelatedPreds(Query1, Query2, Preds), !,
  ground(Query2),
  dm(subqueryDebug, ['\nQuery2: ', Query2,
                     '\nCorrelatedPreds: ', Preds]),
  Query=subquery(SQID, _, _), % NVK ADDED
  restoreSQID(SQID), 				% NVK ADDED
  newAlias(T),
  streamName(T),
  subquery_to_plan(Query2, Plan, T),
  dm(subqueryDebug, ['\nPlan: ', Plan]),
  transformPreds(Preds, T, 2, Preds2),
  dm(subqueryDebug, ['\nPreds2: ', Preds2]), !,
  % NVK MODIFIED NR
  %addCorrelatedPreds(Plan, consume(Query3), Preds2),
  addCorrelatedPreds(Plan, Plan2, Preds2),
  Plan2=..[COp, Query3],
  member(COp, [consume, aconsume]),
  % NVK MODIFIED NR END
  dm(subqueryDebug, ['\nsubquery_plan_to_atom_Query2: ', Query3]),
  plan_to_atom(fun([param(T, tuple)], =(count(head(Query3, 1)), 1)), Result),
  setSQIDToPrevious(SQID), % NVK ADDED NR
  dm(subqueryDebug, ['\nsubquery_plan_to_atom_QueryAtom: ', Result]),
  clearStreamName,
  clearQuery(Query).

/*
NVK ADDED NR
case nested predicate with exists
Case as above, but without a where clause.

Note that this is currentliy only usefull if in the from clause a arel relation appears due to the optimization limitations.

Imlemented exemplarily for the exists operators. This subquery\_plan\_to\_atom thing should be generalized in some way...

*/
subquery_plan_to_atom(Expr, Result) :-
	optimizerOption(nestedRelations),
  ground(Expr),
  Expr =.. [exists, Query],
  extractQuery(Query, Query1),
  Query1 =.. [from, _, Where],
  \+ Where =.. [where | _],
  Query1=Query2,
  ground(Query2),
  dm(subqueryDebug, ['\nQuery2: ', Query2]),
  Query=subquery(SQID, _, _), 
  restoreSQID(SQID), 
  newAlias(T),
  streamName(T),
  subquery_to_plan(Query2, Plan, T),
  dm(subqueryDebug, ['\nPlan: ', Plan]),
  Plan=Plan2,
  Plan2=..[COp, Query3],
  member(COp, [consume, aconsume]),
  dm(subqueryDebug, ['\nsubquery_plan_to_atom_Query2: ', Query3]),
  plan_to_atom(fun([param(T, tuple)], =(count(head(Query3, 1)), 1)), Result),
  dm(subqueryDebug, ['\nsubquery_plan_to_atom_QueryAtom: ', Result]),
  setSQIDToPrevious(SQID),
  clearStreamName,
  clearQuery(Query).
% NVK ADDED NR END


% case nested predicate with not exists
subquery_plan_to_atom(not(Expr), Result) :-
  ground(Expr),
  Expr =.. [exists, Query],
  extractQuery(Query, Query1),
  Query1 =.. [from, _, Where],
  Where =.. [where | _],
  removeCorrelatedPreds(Query1, Query2, Preds), !,
  ground(Query2),
  dm(subqueryDebug, ['\nQuery2: ', Query2,
                     '\nCorrelatedPreds: ', Preds]),
  Query=subquery(SQID, _, _), % NVK ADDED
  restoreSQID(SQID), % NVK ADDED
  newAlias(T),
  streamName(T),
  subquery_to_plan(Query2, Plan, T),
  dm(subqueryDebug, ['\nPlan: ', Plan]),
  transformPreds(Preds, T, 2, Preds2),
  dm(subqueryDebug, ['\nPreds2: ', Preds2]), !,
  % NVK MODIFIED NR
  %addCorrelatedPreds(Plan, consume(Query3), Preds2),
  addCorrelatedPreds(Plan, Plan2, Preds2),
  Plan2=..[COp, Query3],
  member(COp, [consume, aconsume]),
  % NVK MODIFIED NR END
  dm(subqueryDebug, ['\nsubquery_plan_to_atom_Query2: ', Query3]),
  plan_to_atom(fun([param(T, tuple)], not(=(count(head(Query3, 1)), 1))), 
    Result),
  setSQIDToPrevious(SQID), % NVK ADDED NR
  dm(subqueryDebug, ['\nsubquery_plan_to_atom_Result: ', Result]),
  clearStreamName,
  clearQuery(Query).

% flag symmjoin with nested predicate as join predicate
subquery_plan_to_atom(symmjoin(Arg1, Arg2, Pred), Result) :-
  isSubqueryPred1(Pred),
  not(isJoinPred(Pred)),
  dm(subqueryDebug, ['\nsymmjoin: ', Pred]),
  assert(isJoinPred(Pred)),
  joinRels(Arg1, Arg2),
  plan_to_atom(symmjoin(Arg1, Arg2, Pred), Result),
  dm(subqueryDebug, ['n\symmjoin succeeded']),
  retractall(isJoinPred(Pred)).

% case rightrange with nested predicate
subquery_plan_to_atom(rightrange(Arg1, Arg2, Query), Result) :-
  extractQuery(Query, Query1),
  Query1 =.. [from | _],
  dm(subqueryDebug, ['\nrightrange: ', Query]),
  Query=subquery(SQID, _, _), % NVK ADDED
  restoreSQID(SQID), % NVK ADDED
  subquery_to_plan(Query1, Plan, _),
  plan_to_atom(rightrange(Arg1, Arg2, Plan), Result),
  setSQIDToPrevious(SQID). % NVK ADDED NR

% case leftrange with nested predicate
subquery_plan_to_atom(leftrange(Arg1, Arg2, Query), Result) :-
  extractQuery(Query, Query1),
  Query1 =.. [from | _],
  dm(subqueryDebug, ['\nleftrange: ', Query]),
  Query=subquery(SQID, _, _), % NVK ADDED
  restoreSQID(SQID), % NVK ADDED
  subquery_to_plan(Query1, Plan, _),
  plan_to_atom(rightrange(Arg1, Arg2, Plan), Result),
  setSQIDToPrevious(SQID). % NVK ADDED NR

/*
NVK ADDED NR
The above Prolog predicates are for query predicates, this
is for subqueries within the attribute list. It is added in same way as the
upper predicates.

*/
subquery_plan_to_atom(SQuery, Result) :-
  SQuery=subquery(SQID, Query, Rels),
  ground(Query),
  removeCorrelatedPreds(Query, Query2, Preds), !,
  restoreSQID(SQID),
  newAlias(T),
  streamName(T),
  subquery_to_plan(Query2, QueryPlan, T),

  transformPreds(Preds, T, 2, Preds2),
  dm(subqueryDebug, ['\nPreds2: ', Preds2]), !,
  addCorrelatedPreds(QueryPlan, Plan2, Preds2),

  plan_to_atom(fun([param(T, tuple)], Plan2), Result),
  setSQIDToPrevious(SQID),
  clearStreamName,
  (Rels \= [] ->
    clearQuery(subquery(SQID, Query, Rels))
  ;
    true
  ).

subquery_plan_to_atom(SQuery, Result) :-
  SQuery=..[subquery|_],
  throw(error_Internal(subquery_subquery_plan_to_atom(SQuery, Result)::failed)).
% NVK ADDED NR END


/*

Translation of the predicates of a subquery. The following predicates
are needed, to translate the correlated predicates of a nested query.

*/
% case standard predicate, no special handling necessary.
subquerypred_to_atom(Pred, Result) :-
  ground(Pred),
  plan_to_atom(Pred, Result),
  dm(subqueryDebug, ['\nNormalPred: ', Pred,
                     '\nNormalResult: ', Result]).

subquerypred_to_atom([], []).

% case selection predicate
subquerypred_to_atom(pr(P, _), Result) :-
  subquerypred_to_atom(P, Result).

% case join predicate
subquerypred_to_atom(pr(P, _, _), Result) :-
  subquerypred_to_atom(P, Result).

% case list of predicates
subquerypred_to_atom([ Pred | Rest ], Atom) :-
  ground(Pred),
  ground(Rest),
  dm(subqueryDebug, ['\nSubqueryPred: ', Pred]),
  subquerypred_to_atom(Pred, PredAtom),
  dm(subqueryDebug, ['\nPredAtom: ', PredAtom]),
  subquerypred_to_atom(Rest, RestAtom),
  dm(subqueryDebug, ['\nRestAtom: ', RestAtom]),
  ( RestAtom = [], PredAtom = Atom )
    ; concat_atom([PredAtom, RestAtom], Atom).

% correlated predicate
subquerypred_to_atom(Pred, PredAtom) :-
  ground(Pred),
  not(is_list(Pred)),
  Pred =.. [ Op, Attr1, Attr2 ],
  Pred2 =.. [Op, attribute(var1, attrname(Attr1)), Attr2],
  plan_to_atom(Pred2, Pred2Atom),
  concat_atom(['filter[', Pred2Atom, ']'], PredAtom).

subquerySelectivity([]).

subquerySelectivity(Pred) :-
  not(is_list(Pred)),
  selectivity(Pred, _).

subquerySelectivity([ Pred | Rest ]) :-
  subquerySelectivity(Pred),
  subquerySelectivity(Rest).

subquerySelectivity(pr(Pred, Rel1, Rel2), Sel, CalcPET, ExpPET) :-
  isSubqueryPred1(Pred),
  selectivity(pr(Pred, Rel1, Rel2), Sel, CalcPET, ExpPET),
  retractall(firstStream(_)).

subquerySelectivity(pr(Pred, Rel), Sel, CalcPET, ExpPET) :-
  isSubqueryPred1(Pred),
  selectivity(pr(Pred, Rel), Sel, CalcPET, ExpPET),
  retractall(firstStream(_)).

/*

Translation of nested queries in the from-clause if there are no
predicates in the outer query.

*/

subqueryToStream(Query orderby OrderAttrs, Stream4, Cost) :-
  subqueryToStream1(Query, OrderAttrs, Stream4, Cost).

subqueryToStream(Query, Stream4, Cost) :-
  subqueryToStream1(Query, [], Stream4, Cost).

% case simple query no groupby, no predicates
subqueryToStream1(Select from (Subquery) as _, Attrs, Stream4, Cost) :-
  translate1(Subquery, SubStream, SubSelect, Update, Cost),
  dm(subqueryDebug, ['\nSubStream: ', SubStream,
                     '\nSubSelect: ', SubSelect]),
  selectClause(SubSelect, Extend, Project, Rdup),
  dm(subqueryDebug, ['\nExtend: ', Extend,
                     '\nProject: ', Project,
           '\nRdup: ', Rdup]),
  finish2(SubStream, Extend, Project, Rdup, [], Stream2),
  dm(subqueryDebug, ['\nStream2: ', Stream2]),
  selectClause(Select, Extend2, Project2, Rdup2),
  dm(subqueryDebug, ['\nExtend2: ', Extend2,
                     '\nProject2: ', Project2,
           '\nRdup2: ', Rdup2]),
  finish2(Stream2, Extend2, Project2, Rdup2, Attrs, Stream3),
  dm(subqueryDebug, ['\nStream3: ', Stream3]),
  finishUpdate(Update, Stream3, Stream4),
  dm(subqueryDebug, ['\nStream4: ', Stream4]),
  !.

% case simple query with groupby
subqueryToStream1(SelectClause from (Subquery) as _ groupby GroupAttrs, Attrs,
                  Stream5,
          Cost) :-
  translate1(Subquery, SubStream, SubSelect, Update, Cost),
  dm(subqueryDebug, ['\nSubStream: ', SubStream,
                     '\nSubSelect: ', SubSelect]),
  selectClause(SubSelect, Extend, Project, Rdup),
  dm(subqueryDebug, ['\nExtend: ', Extend,
                     '\nProject: ', Project,
           '\nRdup: ', Rdup]),
  finish2(SubStream, Extend, Project, Rdup, [], Stream2),
  dm(subqueryDebug, ['\nStream2: ', Stream2]),
  makeList(GroupAttrs, Attrs2),
  attrnames(Attrs2, AttrNamesGroup),
  attrnamesSort(Attrs2, AttrNamesSort),
  SelectClause = (select Select),
  dm(subqueryDebug, ['\nSelect: ', Select]),
  makeList(Select, SelAttrs),
  dm(subqueryDebug, ['\nSelAttrs: ', SelAttrs]),
  %write('SelAttrs:'), write(SelAttrs), nl,
  translateFields(SelAttrs, Attrs2, Fields, Select2,_,_),
  Stream3 = groupby(sortby(Stream2, AttrNamesSort), AttrNamesGroup, Fields),
  selectClause(select Select2, Extend2, Project2, Rdup2),
  dm(subqueryDebug, ['\nExtend2: ', Extend2,
                     '\nProject2: ', Project2,
           '\nRdup2: ', Rdup2]),
  finish2(Stream3, Extend2, Project2, Rdup2, Attrs, Stream4),
  dm(subqueryDebug, ['\nStream4: ', Stream4]),
  finishUpdate(Update, Stream4, Stream5),
%  throw(error_SQL(subqueries_transformNestedPredicate::tRel1Error)),
  dm(subqueryDebug, ['\nStream5: ', Stream5]),
  !.

/*

Utility Functions

*/

:- dynamic(stack/2).

push(StackName, Var) :-
  ground(Var),
  stack(StackName, L),
  retractall(stack(StackName, _)),
  assert(stack(StackName, [ Var | L])).

push(StackName, Var) :-
  ground(Var),
  not(stack(StackName, _)),
  assert(stack(StackName, [ Var ])).

push(_, _) :-
  throw(error_SQL(subqueries_push::unknownError::_)).

pop(StackName, Var) :-
  stack(StackName, [ Var | L ]),
  retractall(stack(StackName, _)),
  assert(stack(StackName, L)).

peak(StackName, Var) :-
  stack(StackName, [ Var | _ ]).

peak(StackName, Depth, Var) :-
  stack(StackName, L),
  nth1(Depth, L, Var).

clear(StackName) :-
  retractall(stack(StackName, _)).

printStack(StackName) :-
  stack(StackName, L),
  nl, write('Stack '), write(StackName), nl,
  write_list(L), nl.

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

subset([], []).

subset([_|Xs], Ys) :-
  ground(Xs),
	subset(Xs, Ys).

subset([X|Xs], [X|Ys]) :-
  subset(Xs, Ys).

:- [subquerytest].
:- [tpcdqueries].
