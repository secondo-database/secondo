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

*/

/*

 Old tuple constants

*/

feedTC(0.4).
consumeTC(1.0).
filterTC(1.68).
productTC(1.26, 0.4).
leftrangeTC(10).
loopjoinTC(1.0).
exactmatchTC(10.0).
hashjoinTC(1.5, 0.65).
sortmergejoinTC(0.3, 0.73).
symmjoinTC(1.4, 0.7).
extendTC(1.5).
removeTC(0.6).
projectTC(0.71).
renameTC(0.1).
windowintersectsTC(0.1).

/*
  
  New tuple constants

  A: cost to insert a tuple into an in-memory array
  B: cost to compare tuples on equality (0.001 ms)
  C: cost to create a result tuple f(Tx + Ty)
  D: cost to write/read a tuple in/from an in-memory array
  E: cost to write a tuple to an on-disk array
  F: cost to read a tuple from an on-disk array
  MaxMem: maximal memory size per operator

*/

%            A       B       C       D       E    F      MaxMem
hashjoinTC(0.001, 0.001, 0.0035, 0.0001, 0.001, 0.002, 4194304).
symmjoinTC(0.001,   *  , 0.0035, 0.0001, 0.001, 0.002, 4194304).

/*
2 Properties For Certain Operators

Several operators, like geometric predicates who use bounding boxes, have properties, that
require them to be handle differently in some ways.

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
3 Properties For Certain OperatorsDatatypes

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



