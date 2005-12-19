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

%feedTC(0.4).
%consumeTC(1.0).
%filterTC(1.68).
%productTC(1.26, 0.4).
%leftrangeTC(10).
%loopjoinTC(1.0).
%exactmatchTC(10.0).
%hashjoinTC(1.5, 0.65).
%sortmergejoinTC(0.3, 0.73).
%symmjoinTC(1.4, 0.7).
%extendTC(1.5).
%removeTC(0.6).
%projectTC(0.71).
%renameTC(0.1).
%windowintersectsTC(0.1).


/*
  General cost factors, given in milliseconds [ms] per operation. The constants have been derived
  from query times on a reference system and are multiplied by the actual machine's relative speed
  as inquired by ~machineSpeedFactor(CPU, FS)~.
 
  A standard schema is used to enhance the readybility of the cost functions.

*/

cost_factors(   RTM,   WTM,   RTD,   WTD,   RPD,   WPD,   FND,   FDD,   FOD,   FCD,  MaxMem) :-
  machineSpeedFactor(CPU, FS),
  RTM is CPU * 0.001, % RTM:  ReadTupleMem, 
  WTM is CPU * 0.001, % WTM:  WriteTupleMem, 
  RTD is FS  * 0.001, % RTD:  ReadTupleDisk, 
  WTD is FS  * 0.002, % WTD:  WriteTupleDisk, 
  RPD is FS  * 1.000, % RPD:  ReadPageDisk, 
  WPD is FS  * 1.500, % WPD:  WritePageDisk, 
  FND is FS  * 3.000, % FND:  FileNewDisk,
  FDD is FS  * 3.000, % FDD:  FileDeleteDisk, 
  FOD is FS  * 0.500, % FOD:  FileOpenDisk, 
  FCD is FS  * 0.500, % FCD:  FileClose, 
  MaxMem is 4194304,  % maximal memory size per operator
  !.
 
%                RTM,   WTM,   RTD,   WTD,   RPD,   WPD,   FND,   FDD,   FOD,   FCD,  MaxMem  
 cost_factors( 0.001, 0.001, 0.001, 0.002, 1.000, 1.500, 3.000, 3.000, 0.500, 0.500, 4194304).

/*
  Additional cost factors, that are special to certain operators should be defined as tuple constants
  ~operatorTC(...)~.

*/

loopjoinTC(0.100).      % overhead for opening stream Y,
sortTC(0.003).      % evaluating ordering predicate
filterTC(0.007).
feedTC(0.4).
consumeTC(1.0).
extendTC(0.014).
removeTC(0.009).
renameTC(0.007).
projectTC(0.014).


/*
2 Properties For Certain Operators

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



