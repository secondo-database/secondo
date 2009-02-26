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
spatialjoinTC(20.0, 0.7).

% sorttidTC and rdupTC are estimated without experiment
% They still need to be estimated
sorttidTC(0.1).  % used for sorting tuple ids after a windowIntersectsS 
rdupTC(0.1).

/*
1.2 General Constants

The predicate

---- secOptConstant(+ConstantName, -Value)
----

is used to define general constants to be used by the optimizer.

All constants can be printed using predicate

---- showOptConstants/0
----

*/

:-assert(helpLine(showOptConstants,0,[],'Display settings of several constants.')).

showOptConstant :-
  secOptConstant(X, Y),
  write('  secOptConstant( '),write(X), write(',\t'), write(Y), write(')'), nl.

showOptConstants :-
  nl, write('Overview on all optimizer constants secOptConstant/2:\n'),
  findall(_, showOptConstant, _).

/*
1.2.1 Constants for Buffer Size

The value of the buffer size in bytes. This is the amount of main memory, operators are allowed to use to keep their internal data (e.g. hashtables, tuple buffers etc.). This should be equal to the setting for ~MaxMemPerOperator~
in file ~SecondoConfig.ini~.

*/

secOptConstant(bufferSize, 16384000). % 15.625 MB ~ 16 MB

/*
1.2.2 Constants for Sampling

The maximum sample size in bytes. The cardinality of samples will be reduced, such that it hopefully does not get larger than this value.

Minimum and maximun cardinalities for selection and join relation samples

Standard scaling factor for samples.

*/

secOptConstant(sampleScalingFactor, 0.00001).  % scaling factor for samples

secOptConstant(sampleSelMaxDiskSize, 2048).    % maximum KB size for samples
secOptConstant(sampleSelMinCard, 100).         % minimum cardinality for samples
secOptConstant(sampleSelMaxCard, 2000).        % maximum cardinality for samples

secOptConstant(sampleJoinMaxDiskSize, 2048).   % maximum KB size for samples
secOptConstant(sampleJoinMinCard, 50).         % minimum cardinality for samples
secOptConstant(sampleJoinMaxCard, 500).        % maximum cardinality for samples

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
isCommutativeOP(everNearerThan).
isCommutativeOP(distance).


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
isAggregationOP(var).

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
Using the facts with predicate ~isBlockingOP()~, you can specify, whether an operaotor
blocking or not.
standard is nonblocking behaviour.

*/

isBlockingOP(product).
isBlockingOP(hashjoin).
isBlockingOP(sortmergejoin).
isBlockingOP(mergejoin).
isBlockingOP(sort).
isBlockingOP(sortby).
isBlockingOP(avg).
isBlockingOP(sum).
isBlockingOP(min).
isBlockingOP(max).
isBlockingOP(count).
isBlockingOP(consume).
isBlockingOP(tconsume).
isBlockingOP(makemvalue).

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
noFlobType(rect8).
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

