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

1 Constants for Operators

[File ~operators.pl~]

1.1 Constants for Certain Operators

The constants for the operators have been determined by experiments. For
those experiments, time measurement code was added to some relational operators
(hashjoin, product, and sortmergejoin). This permitted determining how much
CPU time is needed for sorting tuples, how much CPU time is needed for hashing etc.
If one wants to experiment by oneself, the time meeasurement support in the
relational algebra can be easily switched on by uncommenting a line in the
relational algebra (and thus defining a symbol ~MEASURE\_OPERATORS~).

The experiments considered only a rather small set of queries. Although the
constants below are quite accurate e.g. for examples 14 to
21 in the optimizer, they may be inappropriate for other queries.

*/

feedTC(0.4).
consumeTC(1.0).
filterTC(1.68).
productTC(1.26, 0.4).
leftrangeTC(10).
loopjoinTC(1.0).
exactmatchTC(10.0).
hashjoinTC(1.5, 0.65).
sortmergejoinTC(0.3, 0.73). % first also used for sort(), second for mergejoin()
symmjoinTC(1.4, 0.7).
extendTC(1.5).
removeTC(0.6).
projectTC(0.71).
renameTC(0.1).
windowintersectsTC(0.1).
spatialjoinTC(10.0, 0.7).

/*
1.2 General Constants

A fact with the value of the buffer size in bytes. This is the amount of mainmemory, operators are allowed to use to keep their internal data (e.g. hashtables, tuple buffers etc.).

*/

bufferSize(16384000).


/*

2 Properties Of Certain Operators

Several operators, like geometric predicates who use bounding boxes, have properties, that
require them to be handled differently in some ways.

*/

% Predicates which can use bboxes:
isBBoxPredicate(intersects).
isBBoxPredicate(intersects_new).
isBBoxPredicate(p_intersects).
isBBoxPredicate(inside).       % also on moving x moving -> movingbool
isBBoxPredicate(insideold).
isBBoxPredicate(adjacent).
isBBoxPredicate(attached).
isBBoxPredicate(overlaps).
isBBoxPredicate(onborder).
isBBoxPredicate(ininterior).

% other operators using bboxes:
isBBoxOperator(touchpoints).
isBBoxOperator(intersection).
isBBoxOperator(intersection_new).
isBBoxOperator(commonborder).
isBBoxOperator(commonborderscan).
isBBoxOperator(X) :- isBBoxPredicate(X).

%isBBoxOperator(minus).
%isBBoxOperator(minus_new).
%isBBoxOperator(union).
%isBBoxOperator(union_new).
%isBBoxOperator(crossings).
%isBBoxOperator(distance).
%isBBoxOperator(direction).
%isBBoxOperator(insidepathlength).
%isBBoxOperator(insidescanned).


/*
Commutative operators can be handled specially in some translation rules

*/
isCommutativeOP(intersects).
isCommutativeOP(intersects_new).
isCommutativeOP(p_intersects).
isCommutativeOP(adjacent).
isCommutativeOP(attached).
isCommutativeOP(overlaps).


/*
Aggregation operators use a common cost function. They can be recognized by predicate
~isAggregationOP(OP)~.

*/
isAggregationOP(count).
isAggregationOP(min).
isAggregationOP(max).
isAggregationOP(sum).
isAggregationOP(avg).
isAggregationOP(extract).

/*
For later extensions (though needing separate cost functions):

*/
isAggregationOP(aggregate).  % the cost of the provided function should be applied, works lineary
isAggregationOP(aggregateB). % the cost of the provided function should be applied,
                             %   Additionally, the operator works balanced (in log(CX) steps).


/*
PlanRewriting needs to identify join operators to allow for a generalized handling.
For each join operator ~j~, a fact ~isJoinOP(j)~ must be defined. Join operators
are expected to merge the attribute sets of their first two arguments. All other
operators are expected not to change the attribute set of the manipulated stream.

Otherwise, a dedicated rule must be added to predicate ~insertExtend/4~ in file
~optimizer.pl~.

*/
isJoinOP(sortmergejoin).
isJoinOP(mergejoin).
%isJoinOP(symmjoin). % has a dedicated rule for insertExtend/4
isJoinOP(symmproductextend). % could get a dedicated rule for insertExtend/4
isJoinOP(hashjoin).
isJoinOP(spatialjoin).
isJoinOP(loopjoin).
isJoinOP(product).
isJoinOP(symmproduct).
isJoinOP(pjoin).


/*
The interesting orders extension needs to recognize operators, that maintain/disturb existing
orderings. ~no~ means, the operator will destroy preexisting orderings. ~outer~ means, it will
keep the ordering of the outer input stream (the leftmost one). If no fact is stored about an
operator, it is assumed, that it will maintain any existing ordering.

*/


maintainsOrderOP(hashjoin,              no).
maintainsOrderOP(symmjoin,              no).
maintainsOrderOP(spatialjoin,           no).
maintainsOrderOP(loopjoin,           outer).
maintainsOrderOP(product,               no).
maintainsOrderOP(symmproduct,           no).
maintainsOrderOP(symmproductextend,     no).
maintainsOrderOP(sort,                  no).
maintainsOrderOP(sortby,                no).
maintainsOrderOP(sortmergejoin,    special).

/*
3 Properties Of Certain Datatypes

~noFlobType(Type)~ indicates that Secondo datatype ~type~ does not have any
flobs. For attributes of these datatypes, the average inline flob size is not queried
to determine attribute sizes.

*/

noFlobType(int).
noFlobType(real).
noFlobType(bool).
noFlobType(string).
noFlobType(xpoint).
noFlobType(xrectangle).
noFlobType(date).
noFlobType(point).
noFlobType(rect).
noFlobType(rect3).
noFlobType(rect4).
noFlobType(rint).
noFlobType(rreal).
noFlobType(periods).
noFlobType(ibool).
noFlobType(iint).
noFlobType(istring).
noFlobType(ireal).
noFlobType(ipoint).
noFlobType(ubool).
noFlobType(uint).
noFlobType(ustring).
noFlobType(ureal).
noFlobType(upoint).
noFlobType(instant).
noFlobType(duration).
noFlobType(tid).

