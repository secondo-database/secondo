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

1.1 General Constants for Cost Estimation

General cost factors, given in milliseconds [ms] per operation. The constants have been derived
from query times on a reference system and are multiplied by the actual machine´s relative speed
as inquired by ~machineSpeedFactor(CPU, FS)~.
 
A standard schema is used to enhance the readybility of the cost functions.
The actual maximum mememory size is defined in ~secondo.rc~.

The value ~MaxMem~ is usually set to 16M, but could differ due to changes in file ~.secondorc~.

Berkeley DB will choose its disk page size according to the size used by the OS/filesystem -
usually 4Kb on a Linux, and 8Kb on a Windows system. Set BDBPS to the according value.

The flob-cache miss rate defines the chache miss rate for reading a flob, this is the probability, that
a flob must be loaded from the disk rather than is found in memory.

*/

cost_factors(   RTM,   WTM,   RTD,   WTD,   RPD,   WPD,   FND,   FDD,   FOD,   FCD,  MaxMem, BDBPS, FCMR) :-
  machineSpeedFactor(CPU, FS),
  RTM is CPU * 0.0007, % RTM:  ReadTupleMem,          ??
  WTM is CPU * 0.0008, % WTM:  WriteTupleMem,         ??
  RTD is FS  * 0.0150, % RTD:  ReadTupleDisk,         OK  
  WTD is FS  * 0.0250, % WTD:  WriteTupleDisk,        OK  
  RPD is FS  * 0.5170, % RPD:  ReadPageDisk,          OK?
  WPD is FS  * 0.7760, % WPD:  WritePageDisk,         OK?
  FND is FS  *15.0000, % FND:  FileNewDisk,           OK
  FDD is FS  *16.0000, % FDD:  FileDeleteDisk,        OK, More exactly: +0.025 ms per tuple/ +0.0016 ms per byte
  FOD is FS  * 5.0000, % FOD:  FileOpenDisk,          ??
  FCD is FS  *16.0000, % FCD:  FileClose,             ??
  MaxMem is  16777216, % MaxMem: maximal memory size per operator (usually 16Mb)
  BDBPS  is      8192, % BDBPS:  Berkeley-DB pagesize, usually 8192 byte for Windows, 4098 byte for Linux
  FCMR   is      0.25, % FCMR:   flob-cache miss rate
  !.
 
%                RTM,   WTM,   RTD,   WTD,   RPD,   WPD,   FND,   FDD,   FOD,   FCD,  MaxMem  
 cost_factors( 0.001, 0.001, 0.001, 0.002, 1.000, 1.500, 3.000, 3.000, 0.500, 0.500, 16777216, 8192, 0.25).


/*
----    ~minTgt(X)~
----
  Minimal tuple creation cost in milliseconds [ms].
  Default is ~minTgt(0.0035).~

*/
minTgt(0.0001).

/*
----    ~minPET(X)~
----
  Minimal predicate evaluation cost in milliseconds [ms].
  Default is ~minTgt(0.001).~
  This value is used for 'cheap predicates' and the 'cheap' cases of otherwise expensive predicates.

*/
minPET(0.001).


/*
1.2 Costants for Certain Operators

  Additional cost factors, that are special to certain operators should be defined as tuple constants
  ~operatorTC(...)~.

*/

loopjoinTC(X) :- machineSpeedFactor(CPU,_), X is CPU * 0.050,   !. % ??  overhead for opening stream Y,
sortTC(X)     :- machineSpeedFactor(CPU,_), X is CPU * 0.003,   !. % ??  evaluating ordering predicate
filterTC(X)   :- machineSpeedFactor(CPU,_), X is CPU * 0.001,   !. % OK
feedTC(X)     :- machineSpeedFactor(CPU,_), X is CPU * 0.0039,  !. % OK
consumeTC(X,Y):- machineSpeedFactor(CPU,_), X is CPU * 0.2086,     % OK Average cost
                 Y is CPU * 0.00002357, !.                         % OK more exact: 0.0013 ms per byte
extendTC(X)   :- machineSpeedFactor(CPU,_), X is CPU * 0.00367, !. % OK more exact: 0.00002357 ms per byte
removeTC(X)   :- machineSpeedFactor(CPU,_), X is CPU * 0.00364, !. % OK
renameTC(X)   :- machineSpeedFactor(CPU,_), X is CPU * 0.00017, !. % OK
projectTC(X,Y):- machineSpeedFactor(CPU,_), X is CPU * 0.00385,    % OK Average cost
                 Y is CPU * 0.00002479, !.                         % OK more exact: 0.00002479 ms per byte
aggregateTC(X):- machineSpeedFactor(CPU,_), X is CPU * 0.00017, !. % Not tested!


/*
2 Properties Of Certain Operators

Several operators, like geometric predicates who use bounding boxes, have properties, that
require them to be handled differently in some ways.

*/

isBBoxOperator(intersects).
isBBoxOperator(inside).
isBBoxOperator(insideold).
isBBoxOperator(touches).
isBBoxOperator(attached).
isBBoxOperator(overlaps).
isBBoxOperator(onborder).
isBBoxOperator(ininterior).
isBBoxOperator(touchpoints).
isBBoxOperator(commonborder).
isBBoxOperator(commonborderscan).
%isBBoxOperator(minus).
%isBBoxOperator(union).
%isBBoxOperator(crossings).
%isBBoxOperator(distance).
%isBBoxOperator(direction).
%isBBoxOperator(insidepathlength).
%isBBoxOperator(insidescanned).


/*
Aggregation operators use a common cost function. They can be recognized by predicate
~isAggregationOP(OP)~.

*/
isAggregationOP(count).
isAggregationOP(min).
isAggregationOP(max).
isAggregationOP(sum).
isAggregationOP(avg).

/*
Blocking operators are recognized by predicate ~isBlockingOP(OP)~. Non-blocking
operators can be aborted early, e.g. by the operator ``head''.

*/

isBlockingOP(X) :- isAggregationOP(X), !.
isBlockingOP(sort).
isBlockingOP(sortby).
isBlockingOP(hashjoin).
isBlockingOP(loopjoin).
isBlockingOP(sortmergejoin).
isBlockingOP(product).


/*
Some operators expect sorted input:

*/

expectsSortedInput(mergejoin).  % ordered on join attributes
expectsSortedInput(rdup).       % totally ordered
expectsSortedInput(groupby).    % ordered on group attributes

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
% noFlobType(text). ???
noFlobType(rect).
noFlobType(rect3).
noFlobType(rect4).
noFlobType(rint).
noFlobType(rreal).
% noFlobType(periods). ???
noFlobType(ibool).
noFlobType(iint).
noFlobType(ireal).
noFlobType(ipoint).
noFlobType(ubool).
noFlobType(uint).
noFlobType(ureal).
noFlobType(upoint).
noFlobType(instant).
noFlobType(duration).
noFlobType(tid).

/*
4 Properties of the optimizer

Some features may be switched on or off:

 * use uniform machine speed factor (1.0) / use determined system speed (var.)
 * apply costs to pre- and post optimization operations 
 * debugging messages

*/

:- dynamic(optConjunctiveCosts/0),
   dynamic(optDebug/0).
   % assert(optUniformSpeed),     % Uncomment to use uniform uniform machine speed factor (1.0)
   % assert(optConjunctiveCosts), % Uncomment to apply costs only to operators considered by dijkstra
   % assert(optDebug),            % Uncomment to see debugging output


ppCostFactor(0) :-
  optConjunctiveCosts, !.

ppCostFactor(1) :- !.
 
optionTotalCosts :-
  retractall(optConjunctiveCosts), 
  write('\nNow, all costs will be applied! - type \'optionConjunctiveCosts.\' to calculate cost of the conjunctive query only.\n\n'),
  !.

optionConjunctiveCosts :-
  retractall(optConjunctiveCosts),
  assert(optConjunctiveCosts), 
  write('\nNow, only costs from the conjunctive query are applied! - type \'optionTotalCosts.\' to calculate all costs.\n\n'),
  !.

toggleDebug :-
  optDebug,
  retract(optDebug),
  write('\nNow surpressing debugging output.'),
  !.

toggleDebug :-
  not(optDebug),
  assert(optDebug),
  write('\nNow displaying debugging output.'),
  !.

toggleCosts :-
  optConjunctiveCosts,
  retract(optConjunctiveCosts),
  write('\nNow, all costs will be applied! - Type \'optionConjunctiveCosts.\' or \'toggleCosts.\' to calculate cost of the conjunctive query only.\n\n'),
  !.

toggleCosts :-
  not(optConjunctiveCosts),
  assert(optConjunctiveCosts),
  write('\nNow, only costs from the conjunctive query are applied! - Type \'optionTotalCosts.\' or \'toggleCosts.\' to calculate all costs instead.\n\n'),
  !.
 
