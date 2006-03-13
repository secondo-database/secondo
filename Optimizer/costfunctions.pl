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

*/

/*

File ~costfunctions.pl~

This file provides cost functions for all Secondo operators supported by the 
optimizer.


8 Computing Edge Costs for Plan Edges

8.1 The Costs of Terms

----    cost(Term+, Pred+, Sel+, Size-, TupleSize-, PredCost+, TC+, Cost-)
----

The cost of an executable ~Term~ representing predicate ~Pred~ with selectivity 
~Sel~, predicate cost ~PredCost~, tuple generation cost ~TC~ is ~Cost~ and the 
size of the result is ~Size~, while its tuplesize is ~TupleSize~.

This is evaluated recursively descending into the term. When the operator
realizing the predicate (e.g. ~filter~) is encountered, the selectivity ~Sel~ is
used to determine the size of the result. The cost for a single evaluation 
of that predicate ~PedCost~ and possibly the cost to generate a result tuple ~TC~
is taken into account when estimating the total cost. It is assumed that only a 
single operator of this kind occurs within the term.

The base cost factors are defined in file ``operators.pl''. 

A standard naming scheme for cost factors is used to enhance the readability of 
the cost functions:

----   RTM:  ReadTupleMem 
       WTM:  WriteTupleMem 
       RTD:  ReadTupleDisk 
       WTD:  WriteTupleDisk 
       RPD:  ReadPageDisk 
       WPD:  WritePageDisk 
       FND:  FileNewDisk 
       FDD:  FileDeleteDisk 
       FOD:  FileOpenDisk 
       FCD:  FileClose 
    MaxMem:  maximum memory size per operator
        TC:  TupleCreate
         B:  PredicateCost/cost of evaluating of an ordering predicate
       Sel:  Selectivity
        CX:  Cardinality of first argument
        CX:  Cardinality of second argument
        Tx:  (necessary) TupleSize of first argument
        Ty:  (necessary) TupleSize of second argument
       TSC:  TupleSizeCoretuple (always handled)
       TSI:  TupleSizeInlineFlob (always handled)
       TSE:  TupleSizeExternalFlob (only handled when used in predicate)
----

8.1.0 Auxiliary Predicates to cost

----    gsf(M, N, S)
        log(B, V, R)
----

These predicates calculate $S=\sum_{i=1}^{N-M}{i+M} = {{(N-M)(N+M+1)}\over{2}}$
and $R = log_B V$.

*/

:- arithmetic_function(gsf/2).
gsf(M, N, S) :-
  S is (N-M)*(N+M+1)*0.5.

:- arithmetic_function(log/2).
log(Base, Value, Result) :-
  Result is log(Value) / log(Base).

% add two tupleSizeData(A,B,C) terms component wise:
addTupleSizes(tupleSizeData(Core1, InlFlob1, ExtFlob1), 
              tupleSizeData(Core2, InlFlob2, ExtFlob2), 
              tupleSizeData(Core, InlFlob, ExtFlob)) :- 
  Core    is Core1    + Core2,
  InlFlob is InlFlob1 + InlFlob2,
  ExtFlob is ExtFlob1 + ExtFlob2.

/*
---- setCurrentResultSizes
     setCurrentResultSizes(ResCard, ResTupleSize)
     getCurrentResultSizes(Term)
----
Get cardinality and tuplesize of the current (intermediate) result, formatted as 
a 'fixed'-term ~Term~, suitable as an input for the predicate cost/8.

The predicates are used to calculate costs arising from Secondo operators 
inserted into the executable plan by the pre-processing steps of the optimizer's 
~sql/1~ predicate, e.g. the final ~consume~, ~count~, ~project~, or commands 
inserted between multiple optimized streams, like ~mergeunion~ or ~mergesec~.

~setCurrentResultSizes/0~ should be called after a stream has been optimized in 
~streamOptimize~.

~setCurrentResultSizes(ResCard, ResTupleSize)~ will update 
~storedCurrentResultSizes/1~ with its arguments.

*/

:- dynamic(storedCurrentResultSizes/1),
   assert(storedCurrentResultSizes(fixed(0, tupleSizeData(0,0,0)))).

getCurrentResultSizes(Term) :-
  storedCurrentResultSizes(Term).

setCurrentResultSizes :- 
  highNode(0),
  retract(storedCurrentResultSizes(_)),
  assert(storedCurrentResultSizes(fixed(0, tupleSizeData(0,0,0)))),
  !.

setCurrentResultSizes :-
  highNode(N),
  resultSize(N, ResCard),
  resultTupleSize(N, ResTS),
  retract(storedCurrentResultSizes(_)),
  assert(storedCurrentResultSizes(fixed(ResCard, ResTS))),
  !.

setCurrentResultSizes :-
  retract(storedCurrentResultSizes(_)),
  assert(storedCurrentResultSizes(fixed(0, tupleSizeData(0,0,0)))),
  !.

setCurrentResultSizes(ResCard, ResTupleSize) :-
  retract(storedCurrentResultSizes(_)),
  assert(storedCurrentResultSizes(fixed(ResCard, ResTupleSize))),
  !.


/*
8.1.1 Arguments

*/


cost(rel(Rel, X, Y), _, _, Size, TupleSize, _, _, 0) :-
  calculateQueryTupleSize(rel(Rel, X, Y),CoreTupleSize,InFlobSize,ExtFlobSize),
  TupleSize = tupleSizeData(CoreTupleSize,InFlobSize,ExtFlobSize),
  card(Rel, Size).

cost(res(N), _, _, Size, TupleSize, _, _, 0) :-
  resultTupleSize(N, TupleSize), 
  resultSize(N, Size).


/*
Argument with explicitly defined cardinality ~ResCard~ and tuple size 
~TupleSize~.  Used to calculate cost of operators inserted by the pre-processor, 
eg. in predicate ~mStreamOptimize~, and for the final count/consume.

*/

cost(fixed(ResCard, TupleSize), _, _, ResCard, TupleSize, _, _, 0).

/*
8.1.2 Operators

*/

cost(feed(X), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize, PredCost, TC, C1),
  cost_factors(_, _, _, _, RPD, _, _, _, FOD, _, _, BDBPS, _),
  feedTC(A,B),
  TupleSize = tupleSizeData(TSC, TSI, _),
  NumPages is ceiling((S * (TSC+TSI))/BDBPS),
  C is C1                        % create Relation, 
     + FOD                       % open relation
     + RPD * NumPages            % read tuples pagewise
     + A * S                     % consider overhead tuplewise
     + B * S * (TSC+TSI).        % consider overhead bytewise

/*
Here ~feedTC~ means ``feed tuple cost'', i.e., the cost per tuple, a constant to
be determined in experiments. These constants are kept in file ``Operators.pl''.

*/

cost(consume(X), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize, PredCost, TC, C1),
  consumeTC(A,B),
  TupleSize = tupleSizeData(TSC, TSI, TSE),
  C is C1                          % create input stream
     + A * S                       % consider overhead tuplewise
     + B * S * (TSC+TSI+TSE).      % consider overhead bytewise

/*
Generic cost function for aggregation operators like count, min, max, sum, avg.
For these operators, the fact ~isAggregationOP(name)~ should be defined in file
``operators.pl''.

*/

cost(Term, P, Sel, S, TupleSize, PredCost, TC, C) :-
  compound(Term),
  functor(Term, Op, 1),
  isAggregationOP(Op),               % recognize OP as an aggregation operator
  arg(1, Term, X),
  cost(X, P, Sel, S, TupleSize, PredCost, TC, C1),
  aggregateTC(A),
  C is C1                            % create input stream
     + A*S.                          % consider overhead tuplewise

     
/*
When filter is used on the output of a spatialjoin, the edge-selectivity is 
distributed among both operators. One can change the behaviour by changing 
~spatialjoinSDR/1~ in file ~operators.pl~.

*/

cost(filter(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
%cost(filter(X, Pred), P, Sel, S, TupleSize, PredCost, TC, C) :- 
% A cost function for the ordering predicate built from Pred could be calculated
  X \= spatialjoin(_, _, _, _, _), % this is the normal case
  cost_factors(_, _, _, _, RPD, _, _, _, _, _, _, BDBPS, FCMR),
  filterTC(A),
  cost(X, P, Sel, SizeX, TupleSize, PredCost, TC, CostX),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  S is SizeX * Sel,
  C is CostX                     % create input stream
     + SizeX * (ExtFlobSize1 + ExtFlobSize2)/BDBPS * FCMR * RPD % read flobs from disk
     + (A + PredCost) * SizeX.   % consider predicate evaluation and overhead tuplewise

cost(filter(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
%cost(filter(X, Pred), P, Sel, S, TupleSize, PredCost, TC, C) :- 
% A cost function for the ordering predicate built from Pred could be calculated
  X = spatialjoin(_, _, _, _, _),  % this is the special case where filter is used 
                                   % on the output of a spatialjoin.
  cost_factors(_, _, _, _, RPD, _, _, _, _, _, _, BDBPS, FCMR),
  filterTC(A),
  spatialjoinSDR(Z),           % (spatial join selectivity distribution ratio)
  SelJ is Z + Sel * (1-Z/100), % distribute overall selectivity between
  SelF is Sel/SelJ,            % the spatialjoin and filter operator
  cost(X, P, SelJ, SizeX, TupleSize, PredCost, TC, CostX),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  S is SizeX * SelF,
  C is CostX                     % create input stream
     + SizeX * (ExtFlobSize1 + ExtFlobSize2)/BDBPS * FCMR * RPD % read flobs from disk
     + (A + PredCost) * SizeX.   % consider predicate evaluation and overhead tuplewise


cost(product(X, Y), P, _, ResultCard, ResultTupleSize, B, TC, Cost) :-
  % the product operator is not symmetric. Y is buffered and X will get consumed.
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  TY = tupleSizeData(TSCy, TSIy, _),
  Ty is TSCy + TSIy,
  cost_factors(RTM, WTM, _, _, _, _, _, _, _, _, MaxMem, _, _),
  OY is (MaxMem / Ty),
  CY =< OY,
  % write('\nProduct: Case 1\n'),
  ResultCard is CX * CY,
  addTupleSizes(TX,TY, ResultTupleSize),
  Cost is CostX + CostY          % produce input streams
        + WTM * CY               % write Y-tuples to in-memory buffer
        + RTM * CX * CY          % read Y-tuples from in-memory buffer
        + TC * ResultCard.       % generate result tuples

cost(product(X, Y), P, _, ResultCard, ResultTupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  TY = tupleSizeData(TSCy, TSIy, _),
  Ty is TSCy + TSIy,
  cost_factors( _, WTM, RTD, WTD, _, _, FND, FDD, _, _, MaxMem, _, _),
  OY is (MaxMem / Ty),
  CY > OY,
  % write('\nProduct: Case 2\n'),
  ResultCard is CX * CY,
  addTupleSizes(TX,TY, ResultTupleSize),
  Cost is CostX + CostY          % produce input streams
        + FND + FDD              % create and remove temp file
        + WTM * OY               % write Y-tuples to in-memory buffer
        + WTD * CY               % write Y-tuples to disk
        + RTD * CX * CY          % read Y-tuples from disk
        + TC * ResultCard.       % generate result tuple


/*

Operator ~leftrange~

Uses a btree-index to select tuples from a relation.
The index must be opened. The predicate is always ${X}\geq v$
for ~rightrange~, ${}\leq v$ for ~leftrange~, and ${}= v$ for 
~exactmatch~.
  
To estimate the number of disk accesses, we query the 
height and the keys-per-leafe size $K$ of the BTree.
then we expect to access 
$\mbox{height}+{\mbox{Sel} \cdot \mbox{RelSize}}\over K$ disk pages 
and evalute ${\mbox{Height}+K}\over 2$ times the comparisons.

XRIS: By now, the fan-out for Btrees is assumed to be 15.
  
*/

cost(leftrange(_, Rel, _), P, Sel, Size, TupleSize, B, TC, Cost) :-
  cost(Rel, P, 1, RelSize, TupleSize, B, TC, CostR),
  cost_factors(_, _, _, _, RPD, _, _, _, FOD, FCD, _, BDBPS, FCMR),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Size is Sel * RelSize,
  Cost is CostR                           % produce input stream
        + Size*(ExtFlobSize1 + ExtFlobSize2)/BDBPS * FCMR * RPD % read flobs from disk
        + FOD + FCD                       % open/close btree file        
        + B * log(15,RelSize)             % later: RPD * (Height + K/2)
        + RPD * log(15,RelSize)           % later: RPD * (Height + Sel*RelSize/K)
        + TC * Sel * RelSize.             % produce tuple stream

cost(rightrange(_, Rel, _), P, Sel, Size, TupleSize, B, TC, Cost) :-
  cost(Rel, P, 1, RelSize, TupleSize, B, TC, CostR),
  cost_factors(_, _, _, _, RPD, _, _, _, FOD, FCD, _, BDBPS, FCMR),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Size is Sel * RelSize,
  Cost is CostR                           % produce input stream
        + Size*(ExtFlobSize1 + ExtFlobSize2)/BDBPS * FCMR * RPD % read flobs from disk
        + FOD + FCD                       % open/close btree file        
        + B * log(15,RelSize)             % find all matches,           later: B * (Height + K/2)
        + RPD * log(15,RelSize)           % read index pages from disk, later: D * (Height + Sel*RelSize/K)
        + TC * Sel * RelSize.             % produce tuple stream

/*
Simplistic cost estimation for loop joins.

If attribute values are assumed independent, then the selectivity
of a subquery appearing in an index join equals the overall
join selectivity. Therefore it is possible to estimate
the result size and cost of a subquery
(i.e. ~exactmatch~ and ~exactmatchfun~). As a subquery in an
index join is executed as often as a tuple from the left
input stream arrives, it is also possible to estimate the
overall index join cost.

See: ~cost(leftrange(...),...)~ for key on tuple constants.

*/

cost(exactmatchfun(_, Rel, _), P, Sel, Size, TupleSize, B, TC, Cost) :-
  cost(Rel, P, 1, RelSize, TupleSize, B, TC, CostR),
  cost_factors(_, _, _, _, RPD, _, _, _, FOD, FCD, _, BDBPS, FCMR),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Size is Sel * RelSize,
  Cost is CostR                           % produce input stream
        + Size*(ExtFlobSize1 + ExtFlobSize2)/BDBPS * FCMR * RPD % read flobs from disk
        + FOD + FCD                       % open/close btree file        
        + B * (log(15,RelSize)+Size)      % find all matches,           later: B * (Height + K/2)
        + RPD * log(15,RelSize)           % read index pages from disk, later: D * (Height + Sel*RelSize/K)
        + TC * Sel * RelSize.             % produce tuple stream

cost(exactmatch(_, Rel, _), P, Sel, Size, TupleSize, B, TC, Cost) :-
  cost(Rel, P, 1, RelSize, TupleSize, B, TC, CostR),
  cost_factors(_, _, _, _, RPD, _, _, _, FOD, FCD, _, BDBPS, FCMR),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Size is Sel * RelSize,
  Cost is CostR                           % produce input stream
        + Size*(ExtFlobSize1 + ExtFlobSize2)/BDBPS * FCMR * RPD % read flobs from disk
        + FOD + FCD                       % open/close btree file        
        + B * (log(15,RelSize)+Size)      % find all matches,           later: B * (Height + K/2)
        + RPD * log(15,RelSize)           % read index pages from disk, later: D * (Height + Sel*RelSize/K)
        + TC * Sel * RelSize.             % produce tuple stream


/*
  Algorithm loopjoin: see file ``ExtRelationAlgebra.cpp''.

*/

cost(loopjoin(X, Y), P, Sel, ResCard, TupleSize, PredCost, TC, Cost) :-
  cost(X, P, 1, CX, TX, PredCost, TC, CostX),
  cost(Y, P, Sel, CY, _, PredCost, TC, CostY),
  addTupleSizes(TX, TX, TupleSize),
  ResCard is CX * CY,
  cost_factors(_, _, _, _, RPD, _, _, _, _, _, _, BDBPS, _),
  loopjoinTC(A),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CX * CostY        % produce input streams, stream Y is produced once per X-tuple
        + RPD * CX*(ExtFlobSize1)/BDBPS % read flobs from disk
        + RPD * CY*(ExtFlobSize2)/BDBPS % read flobs from disk
        + A * CX                    % overhead for opening stream Y
        + TC * ResCard.             % create result tuples

cost(fun(_, X), P, Sel, Size, TupleSize, PredCost, TC, Cost) :- % no overhead considered, by now
  cost(X, P, Sel, Size, TupleSize, PredCost, TC, Cost).


/*
Cost function for ~hashjoin~:

  * NBuckets: Number of buckets in hash table

  * MemSizeX = 0.25[*]MaxMem; this one is used to hold X-tuples hashed against 
the hash table

  * MemSizeY = 0.75[*]MaxMem; this one is used for the hash table of Y-tuples

For the evaluation of the join predicate, we know, that only `is equal' is 
allowed, as hashjoin is an equijoin. For all `is equal' predicates, it is 
assumed, that only in the case of $A=B$, the evaluation is really expensive, 
for in the contrary case, some cheap test will usually discover, that $A \neq B$.

Hence, we can estimate the number of expensive predicate evaluations by 
$\mbox{Sel} \cdot CX \cdot CY$.
All other predicate evaluations are expected to be 'cheap' (with cost ~MinPET~).

  For the calculation of a hash value, a cost of ~MinPET~ is assigned.

*/

cost(hashjoin(X, Y, _, _, NBuckets), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostArgX),
  cost(Y, P, 1, CY, TY, B, TC, CostArgY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, MaxMem, BDBPS, FCMR),
  MemSizeX is 0.25*MaxMem,
  MemSizeY is 0.75*MaxMem,
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  % test for case1: Both argument relations X,Y fit into memory  
  (CX * (Tx+12)) =< MemSizeX, 
  (CY * (Ty+12)) =< MemSizeY, 
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  minPET(MinPET),
  ResSize is CX * CY * Sel,
  addTupleSizes(TX,TY,TupleSize),
  Cost is CostArgX + CostArgY                  % create input streams
       + RPD * (CX * ExtFlobSize1)/BDBPS       % read flobs from disk
       + RPD * (CX * max((CY/NBuckets),1) * ExtFlobSize2)/BDBPS/(1-FCMR) % read flobs from disk
       + MinPET * (CX + CY)                    % calculate hash value (use MinPET as an approximation to cost)
       + WTM * CY                              % inserting Y-tuples into hash table
                                               % collide each X-tuple with Y-tuples from hashed bucket
       % +(RTM + B) * CX * max((CY/NBuckets),1)%  (this is the original line)
       + (RTM + B) * ResSize                   %  - expensive cases
       + (RTM + MinPET)                        %  - cheap cases
               * (CX * max((CY/NBuckets),1) - ResSize) 
       + TC * ResSize,                         % create result tuples
  (optDebugLevel(cost) *-> (
    RTMn is CX * max((CY/NBuckets),1),
    WTMn is CY,
    RTDn is 0,
    WTDn is 0,
    RPDn is (CX * ExtFlobSize1)/BDBPS + (CX * max((CY/NBuckets),1) * ExtFlobSize2)/BDBPS/(1-FCMR),
    WPDn is 0,
    FNDn is 0,
    FDDn is 0,
    FODn is 0,
    FCDn is 0,
    Bn   is ResSize,
    MinPETn is CX * max((CY/NBuckets),1) - ResSize + (CX + CY),
    TCn  is ResSize,
    write('\n\n\nhashjoin('), write(X), write(','), write(Y), 
    write(', _, _, '), write(NBuckets), write('), '),
    write(P), write(','), write(Sel), write(','), write(ResSize), 
    write(','), write(TupleSize), write(','), 
    write(B), write(','), write(TC), write(','), write(Cost), write(') Case1'),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn),
    write('\n\tMinPET:  '), write(MinPETn), 
    write('\n\tNeeded FlobSizes: '), write(ExtFlobSize1),
    write('/'), write(ExtFlobSize2), nl
  ); true).

cost(hashjoin(X, Y, _, _, NBuckets), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostArgX),
  cost(Y, P, 1, CY, TY, B, TC, CostArgY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  MemSizeX is 0.25*MaxMem,
  MemSizeY is 0.75*MaxMem,
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  % test for case4: neither X, nor Y fit in memory                                       
  (CX * (Tx+12)) > MemSizeX,
  (CY * (Ty+12)) > MemSizeY,
  MemSizeTY is max(1,(MemSizeY / Ty)),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  minPET(MinPET),
  ResSize is CX * CY * Sel,
  addTupleSizes(TX,TY,TupleSize),
  Cost is CostArgX + CostArgY          % create input streams
       + WTM * CY                      % insert pointers to Y in hash tables
       + RPD * (CY/MemSizeTY) * (CX * ExtFlobSize1)/BDBPS % read flobs from disk
       + RPD * CX * max(1,(MemSizeTY/NBuckets)) 
                  * (CY/MemSizeTY) * ExtFlobSize2/BDBPS/(1-FCMR) % read flobs from disk
                                       % compare X- and Y-tuples
       % +(B + RTM)  * CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) % (original line)
       + (B + RTM)  * ResSize          %  - cheap comparisons
       + (MinPET + RTM) 
           * (  CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) 
              - ResSize )              %  - expensive comparisons
       + 2 * (FND + FDD)               % create and delete temp files
       + WTD * CX                      % write X-tuples to disk
       + RTD * CX * CY/MemSizeTY       % X-tuples to be read from disk once per hash table
       + MinPET * (CY + CX * max(1,CY/MemSizeTY))  % calculate hash value (use MinPET as an approximation to cost)
       + TC * ResSize,                 % create result tuples
  (optDebugLevel(cost) *-> (
    RTMn is CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY),
    WTMn is CY,
    RTDn is CX * CY/MemSizeTY,
    WTDn is CX,
    RPDn is (CY/MemSizeTY) * (CX * ExtFlobSize1)/BDBPS + CX 
            * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) 
            * ExtFlobSize2/BDBPS/(1-FCMR),
    WPDn is 0,
    FNDn is 2,
    FDDn is 2,
    FODn is 0,
    FCDn is 0,
    Bn   is ResSize,
    MinPETn is CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) 
                         - ResSize + (CY + CX * max(1,CY/MemSizeTY)),
    TCn  is Sel * CX * CY,
    write('\n\nhashjoin('), write(X), write(','), write(Y), 
    write(', _, _, '), write(NBuckets), write('), '),
    write(P), write(','), write(Sel), write(','), write(ResSize), 
    write(','), write(TupleSize), write(','), 
    write(B), write(','), write(TC), write(','), write(Cost), write(') Case4'),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn), 
    write('\n\tMinPET:  '), write(MinPETn), 
    write('\n\tNeeded FlobSizes: '), write(ExtFlobSize1), write('/'), 
    write(ExtFlobSize2), nl
  ); true).

cost(hashjoin(X, Y, _, _, NBuckets), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostArgX),
  cost(Y, P, 1, CY, TY, B, TC, CostArgY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, MaxMem, BDBPS, FCMR),
  MemSizeX is 0.25*MaxMem,
  MemSizeY is 0.75*MaxMem,
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  % case2: X fits in memory, but Y does not 
  (CX * (Tx+12)) =< MemSizeX, 
  (CY * (Ty+12)) >  MemSizeY,            
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  minPET(MinPET),
  ResSize is CX * CY * Sel,
  addTupleSizes(TX,TY,TupleSize),
  MemSizeTY is max(1,(MemSizeY / Ty)), % amount of Y-tuples fitting into MemY
  Cost is CostArgX + CostArgY          % create input streams
       + WTM * CY                      % insert Y-tuples into hash table
       + RPD * (CY/MemSizeTY) * (CX*ExtFlobSize1)/BDBPS % read flobs from disk
       + RPD * CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) 
                     * ExtFlobSize2/BDBPS/(1-FCMR) % read flobs from disk
                                       % compare each X-tuple with
                                       % the hashed Y-tuples from several hashtables
       %+ (B + RTM) * CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) % (original line)
       + (B + RTM) * ResSize           %  - expensive comparisons
       + (MinPET+RTM)*(  CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) 
                       - ResSize)      %  - cheap comparisons

       + RTM * CX * (CY/MemSizeTY)     % read X-tuples from memory once per hashtable
       + MinPET * (CY + CX * max(1,CY/MemSizeTY)) % calculate hash values (use MinPET as an approximation to cost)
       + TC * ResSize,                 % create result tuples
  (optDebugLevel(cost) *-> (
    RTMn is CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) + CX * (CY/MemSizeTY),
    WTMn is CY,
    RTDn is 0,
    WTDn is 0,
    RPDn is (CY/MemSizeTY) * (CX*ExtFlobSize1)/BDBPS 
             +  CX * max(1,(MemSizeTY/NBuckets)) 
                   * (CY/MemSizeTY) 
                   * ExtFlobSize2/BDBPS/(1-FCMR),
    WPDn is 0,
    FNDn is 0,
    FDDn is 0,
    FODn is 0,
    FCDn is 0,
    Bn   is ResSize,
    MinPETn is CX * max(1,(MemSizeTY/NBuckets)) 
                  * (CY/MemSizeTY) 
                  - ResSize + (CY + CX * max(1,CY/MemSizeTY)),
    TCn  is Sel * CX * CY,
    write('\n\nhashjoin('), write(X), write(','), write(Y), 
    write(', _, _, '), write(NBuckets), write('), '),
    write(P), write(','), write(Sel), write(','), write(ResSize), 
    write(','), write(TupleSize), write(','), 
    write(B), write(','), write(TC), write(','), write(Cost), write(') Case2'),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn),
    write('\n\tMinPET:  '), write(MinPETn), 
    write('\n\tNeeded FlobSizes: '), write(ExtFlobSize1), write('/'), 
    write(ExtFlobSize2), nl
  ); true).

cost(hashjoin(X, Y, _, _, NBuckets), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostArgX),
  cost(Y, P, 1, CY, TY, B, TC, CostArgY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, MaxMem, BDBPS, FCMR),
  MemSizeX is 0.25*MaxMem,
  MemSizeY is 0.75*MaxMem,
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  % case3: X does not fit in memory, but Y does  (Nearly the same cost function as in case2!)
  (CX * (Tx+12)) >  MemSizeX, 
  (CY * (Ty+12)) =< MemSizeY,  
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  minPET(MinPET),
  ResSize is CX * CY * Sel,
  addTupleSizes(TX,TY,TupleSize),
  MemSizeTY is max(1,(MemSizeY / Ty)), % amount of Y-tuples fitting into MemY
  Cost is CostArgX + CostArgY          % create input streams
       + WTM * CY                      % insert Y-tuplesinto hash table
       + RPD * (CX * ExtFlobSize1)/BDBPS % read flobs from disk
       + RPD * CX * max(1,(MemSizeTY/NBuckets)) 
                  * (CY/MemSizeTY) 
                  * ExtFlobSize2/BDBPS/(1-FCMR) % read flobs from disk
                                       % compare each X-tuple with
                                       % the hashed Y-tuples from several hashtables
       %+ (B + RTM) * CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) % (original line)
       + (B + RTM) * ResSize           %  - expensive comparisons
       + (MinPET+RTM)*(  CX * max(1,(MemSizeTY/NBuckets)) * (CY/MemSizeTY) 
                       - ResSize)      %  - cheap comparisons
       + RTM * CX * (CY/MemSizeTY)     % read X-tuples from memory once per hashtable
       + MinPET * (CX + CY)            % calculate hash values (use MinPET as an approximation to cost)
       + TC * ResSize,                 % create result tuples
  (optDebugLevel(cost) *-> (
    RTMn is CX * max(1,(MemSizeTY/NBuckets)) 
               * (CY/MemSizeTY) 
               + CX * (CY/MemSizeTY),
    WTMn is CY,
    RTDn is 0,
    WTDn is 0,
    RPDn is (CX * ExtFlobSize1)/BDBPS + CX * max(1,(MemSizeTY/NBuckets)) 
                * (CY/MemSizeTY) * ExtFlobSize2/BDBPS/(1-FCMR),
    WPDn is 0,
    FNDn is 0,
    FDDn is 0,
    FODn is 0,
    FCDn is 0,
    Bn   is ResSize,
    MinPETn is CX * max(1,(MemSizeTY/NBuckets)) 
                  * (CY/MemSizeTY) - ResSize + (CX + CY),
    TCn  is ResSize,
    write('\n\nhashjoin('), write(X), write(','), write(Y), write(', _, _, '), 
    write(NBuckets), write('), '),
    write(P), write(','), write(Sel), write(','), write(ResSize), write(','), 
    write(TupleSize), write(','), 
    write(B), write(','), write(TC), write(','), write(Cost), write(') Case3'),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn), 
    write('\n\tMinPET:  '), write(MinPETn), 
    write('\n\tNeeded FlobSizes: '), write(ExtFlobSize1), write('/'), 
    write(ExtFlobSize2), nl
  );true).


/*
This cost function for ~sortby~ depends on the operator's  implementation
in ExtRelAlgPersistent, a variant of mergesort.
 
For more accurate estimations, information on the distribution and order of 
values of sort attributes would be needed.

Operator ~sort~ applies ~sortby~ using lexicographical ordering on all 
attributes.

Argument ~Order~ is either a float/integer representing the cost to evaluate 
the ordering predicate once, or a list of (Attrname SortOrder) elements.

*/

cost(sortby(X,Order), P, Sel, Card, TX, PredCost, TC, Cost) :- % XRIS: flob loading not yet considered here!
% A cost function for the ordering predicate built from Pred could be calculated
  cost(X, P, Sel, Card, TX, PredCost, TC, CostX),
  cost_factors(RTM, WTM, _, _, _, _, _, _, _, _, MaxMem, _, _),
  ((float(Order); integer(Order)) *-> B is Order; sortTC(B)),  % get cost for ordering predicate
  TX = tupleSizeData(TSCx, TSIx, _),
  Tx is TSCx + TSIx,
  HeapSize is MaxMem / (Tx + 12),      % determine heapsize in tuples
  % case1: complete relation fits into memory - no merging needed
  Card =< HeapSize,
  % write('\nCase1\n'),
  MCard is max(1,Card),                % ensure MCard >0
  Cost is CostX                        % produce argument stream
        + WTM * Card *0.5 *log(2,MCard)% inserts into heap 
        + RTM * Card                   % reads from heap 
        + B * Card * log(2,MCard),     % compare a pair of tuples (building the heap)
  (optDebugLevel(cost) *-> (
    RTMn is Card,
    WTMn is Card *0.5 *log(2,MCard),
    RTDn is 0,
    WTDn is 0,
    RPDn is 0,
    WPDn is 0,
    FNDn is 0,
    FDDn is 0,
    FODn is 0,
    FCDn is 0,
    Bn   is Card * log(2,MCard),
    TCn  is 0,
    write('\n\nsortby('), write(X), write(','), write(Order), write(') Case1'),
    write('\n\tCostX: '), write(CostX),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn), nl
  ); true).


cost(sortby(X,Order), P, Sel, Card, TX, PredCost, TC, Cost) :-  % XRIS: flob loading not yet considered here!
% A cost function for the ordering predicate built from Pred could be calculated
  cost(X, P, Sel, Card, TX, PredCost, TC, CostX),             
  cost_factors(RTM, WTM, RTD, WTD, _, _, FND, FDD, FOD, FCD, MaxMem, _, _),
  ((float(Order); integer(Order)) *-> B is Order; sortTC(B)),   % get cost for ordering predicate
  TX = tupleSizeData(TSCx, TSIx, _),
  Tx is TSCx + TSIx,
  HeapSize is max(1,MaxMem / (Tx + 12)),           % determine heapsize in tuples
  % case2: relation does not fit into memory
  Card > HeapSize,
  PartCard is 2 * HeapSize,                        % calculate average cardinality of partion files generated
  PartNum is ceiling(max(0,(Card-HeapSize)) / max(1,PartCard)),  % calculate average number of partion files, 
  Cost is CostX                                    % produce argument stream
        + WTM * max(0,(Card-HeapSize))*0.5*log(2,HeapSize)  % inserts into heap =(Card-HeapSize)*0.5 * O(log HeapSize)
        + RTM * Card                               % reads from heap =Card * O(1)
        + WTD * max(0,(Card-HeapSize))             % write tuple to partition file (=Card-HeapSize)
        + RTD * max(0,(Card-HeapSize))             % read tuple from partition file (=Card-HeapSize)
        + (FND + FCD + FOD + FDD) * PartNum        % create, close, open and remove a partition file (=PartNum)
        + B * Card * log(2,HeapSize)               % compare a pair of tuples (partition step)
        + B * Card * log(2,PartNum+2),             % compare a pair of tuples (merge step)
  (optDebugLevel(cost) *-> (
    RTMn is Card,
    WTMn is max(0,(Card-HeapSize))*0.5*log(2,HeapSize),
    RTDn is max(0,(Card-HeapSize)),
    WTDn is max(0,(Card-HeapSize)),
    RPDn is 0,
    WPDn is 0,
    FNDn is PartNum,
    FDDn is PartNum,
    FODn is PartNum,
    FCDn is PartNum,
    Bn   is Card * log(2,HeapSize) + Card * log(2,PartNum+2),
    TCn  is 0,
    write('\n\nsortby('), write(X), write(','), write(Order), write(') Case2'),
    write('\n\tCostX: '), write(CostX),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn), nl
  ); true).

cost(sort(X), P, Sel, CardX, TX, PredCost, TC, Cost) :- % 
% A cost function for the ordering predicate built from Pred could be calculated
  cost(sortby(X,lexall), P, Sel, CardX, TX, PredCost, TC, Cost).


/*
Cost function for operator groupby, as defined in file ``ExtRelationAlgebra.cpp''

Expects argument to be sorted on GroupAttr.
Returns one extended tuple per group.

*/
cost(groupby(X, _, _), P, Sel, Card, TS, PredCost, TC, Cost) :- 
% XRIS: cost function missing!
%cost(groupby(X, GroupAttr, NewAttr-Fun-List), P, Sel, Card, TX, PredCost, TC, Cost) :-
  % for each tuple: check if GroupAttr-values matches predecessor, then store to buffer
  % for each group: for each NewAttr: calculate value by evaluating functions - similar to k-times ``extend''
  cost(X, P, Sel, Card, TS, PredCost, TC, Cost).
  

/*
Cost function for operator ~rdup~. Similar to ~sort~, using comparison to test 
on equality of tuples. Only the effords for forwarding and comparison of tuples
is considered here.

Expects a totally ordered input.

*/

cost(rdup(X), P, Sel, Card, TS, PredCost, TC, Cost) :-
  cost(X, P, Sel, CardX, TS, PredCost, TC, CostX),
  sortTC(B),
  Cost is CostX                  % produce argument stream
        + B * max(0,(CardX-1)),  % compare subsequent tuples for equality
  Card is CardX * Sel.


/*
Detailed cost estimation for sortmergejoin

A realistic cost estimation would need information, like histograms, 
on the values of the join attributes. An option would be to recognize only
those cases, that perform very bad, because the buffers overflow to disk.

Problem: We do not know, how large the equivalence classes of $A \times B$ 
with respect to ``A=B'' are. So, we even don't know how much of the buffers
is used and how many tuples have to be temporarily stored to disk relations.

For the sake of simplicity we assume, that all tuples with a common join 
value fit into memory.

*/

cost(sortmergejoin(X, Y, _, _), P, Sel, ResultCard, 
     ResultTupleSize, B, TC, Cost) :-
  cost(sortby(X, B), P, 1, CX, TX, PredCost, TC, CostX),
  cost(sortby(Y, B), P, 1, CY, TY, PredCost, TC, CostY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, _, BDBPS, FCMR),
  addTupleSizes(TX,TY,ResultTupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  ResultCard is CX * CY * Sel,
  Cost is CostX + CostY                % producing and sorting the arguments
        + (CX * ExtFlobSize1)/BDBPS * RPD/FCMR % load flobs
        + (CY * ExtFlobSize2)/BDBPS * RPD/FCMR % load flobs
        + WTM * (CX+CY) * sqrt(Sel)    % write tuples to buffer/memory; A BAD ESTIMATION!
        + B   * (CX+CY)                % compare tuples 
        + RTM * (CX+CY) * sqrt(Sel)    % read tuples from buffer/memory; A BAD ESTIMATION!
        + TC  * ResultCard,            % create result tuple
  (optDebugLevel(cost) *-> (
    RTMn is (CX+CY) * sqrt(Sel),
    WTMn is (CX+CY) * sqrt(Sel),
    RTDn is 0,
    WTDn is 0,
    RPDn is (CX * ExtFlobSize1)/BDBPS/FCMR + (CY * ExtFlobSize2)/BDBPS/FCMR,
    WPDn is 0,
    FNDn is 0,
    FDDn is 0,
    FODn is 0,
    FCDn is 0,
    Bn   is 0,
    TCn  is Sel * CX * CY,
    write('\n\nSortmergejoin('), write(X), write(','), write(Y), write(',_,_)'),
    write('\n\tCostX: '), write(CostX),
    write('\n\tCostY: '), write(CostY),
    write('\n\tRTM: '), write(RTMn),
    write('\n\tWTM: '), write(WTMn),
    write('\n\tRTD: '), write(RTDn),
    write('\n\tWTD: '), write(WTDn),
    write('\n\tRPD: '), write(RPDn),
    write('\n\tWPD: '), write(WPDn),
    write('\n\tFND: '), write(FNDn),
    write('\n\tFDD:  '), write(FDDn),
    write('\n\tFOD:  '), write(FODn),
    write('\n\tFCD:  '), write(FCDn),
    write('\n\tB  :  '), write(Bn),
    write('\n\tTC :  '), write(TCn),
    write('\nNeeded FlobSizes: '), write(ExtFlobSize1), write(ExtFlobSize2), nl
  ); true).



/*
Cost function for mergejoin

Same function as for ~sortmergejoin~, just without sorting the argument streams.

*/

cost(mergejoin(X, Y, _, _), P, Sel, ResultCard, ResultTupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, PredCost, TC, CostX),
  cost(Y, P, 1, CY, TY, PredCost, TC, CostY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, _, BDBPS, FCMR),
  addTupleSizes(TX,TY,ResultTupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  ResultCard is CX * CY * Sel,
  Cost is CostX + CostY                % producing and sorting the arguments
                                       % XRIS: Individual cost of ordering still not applied!
        + (CX * ExtFlobSize1)/BDBPS * RPD/FCMR % load flobs
        + (CY * ExtFlobSize2)/BDBPS * RPD/FCMR % load flobs
        + WTM * (CX+CY) * sqrt(Sel)    % write tuples to buffer/memory; A BAD ESTIMATION!
        + B   * (CX+CY)                % compare tuples (B could also be fixed, e.g. 0.001)
        + RTM * (CX+CY) * sqrt(Sel)    % read tuples from buffer/memory; A BAD ESTIMATION!
        + TC  * ResultCard.            % create result tuple

/*
Quick and dirty cost function for Operator ~mergeunion~

*/

cost(mergeunion(X,Y), P, _, ResultCard, ResultTupleSize, PredCost, TC, Cost) :-
  cost(X, P, 1, CX, ResultTupleSize, PredCost, TC, CostX),
  cost(Y, P, 1, CY, ResultTupleSize, PredCost, TC, CostY),
  sortTC(A),
  ResultCard is CX + CY,               % neglect cardinality of intersection (similar to concat-operator)
  Cost is CostX + CostY                % producing and sorting the arguments
        + A * (CX + CY).               % compare tuples

/*
Quick and dirty Cost function for Operator ~mergesec~

*/

cost(mergesec(X,Y), P, _, ResultCard, ResultTupleSize, PredCost, TC, Cost) :-
  cost(X, P, 1, CX, ResultTupleSize, PredCost, TC, CostX),
  cost(Y, P, 1, CY, ResultTupleSize, PredCost, TC, CostY),
  sortTC(A),
  ResultCard is min(CX,CY),            % not a very good estimation
  Cost is CostX + CostY                % producing and sorting the arguments
        + A * (CX + CY).               % compare tuples


/*
Detailed cost function for ~symmjoin~

*/

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  % case1: X and Y fit into memory; OK
  (CX * (Tx+12)) =< (0.5 * MaxMem), (CY * (Ty+12)) =<  (0.5 * MaxMem),
  % write('\nSymmjoin Case1\n'),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  (CX =< CY -> (Min is CX, Max is CY, 
                MinFlob is ExtFlobSize1/BDBPS, 
                MaxFlob is ExtFlobSize2/BDBPS)
             ; (Min is CY, Max is CX, 
                MinFlob is ExtFlobSize2/BDBPS, 
                MaxFlob is ExtFlobSize1/BDBPS)),
  addTupleSizes(TX,TY,TupleSize),
  Cost is CostX + CostY                    % produce arguments
       + RPD * FCMR * (Min * (Min+1)/2 +       (Max-Min)) * MaxFlob % load flobs
       + RPD * FCMR * (Min * (Min+1)/2 + Min * (Max-Min)) * MinFlob % load flobs
       + WTM * 2 * Min                     % write X/Y-tuples to buffer in memory.
       + B   * Min * Max                   % check join condition
       + TC  * Min * Max * Sel             % create result tuples  
       + RTM * (Min*Min + Min*(Max-Min)),  % read X/Y-tuples from memory
  ResSize is CX * CY * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OY is min(CY,MaxMem/(2 * Ty)), % from this tuple+1 on, Y will be entirely buffered on disk
  % case 2.1: X fits in memory, but Y does not; CX =< CY; OY =< CX
  (CX * (Tx+12)) =< (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CX =< CY, OY =< CX,
  % write('\nSymmjoin Case2.1\n'),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  addTupleSizes(TX,TY,TupleSize), 
  Cost is CostX + CostY              % produce arguments
       + RPD * FCMR * (CX * (CX+1)/2 +      (CY-CX)) * ExtFlobSize2/BDBPS % load flobs
       + RPD * FCMR * (CX * (CX+1)/2 + CX * (CY-CX)) * ExtFlobSize1/BDBPS % load flobs
       + WTM * (CX + OY)             % write elements to buffer in memory
       + B * CX * CY                 % check join condition
       + TC * CX * CY * Sel          % create result tuples  
       + RTM * ( OY*OY               
                +gsf(OY,CX)
                +(CY-CX)*CX)         % read X/Y-tuples from memory
       + FND + FDD                   % create and delete tmp file
       + WTD * CX                    % write Y-tuples to disk
       + RTD * gsf(OY,CX),           % read Y-tuples from disk
  ResSize is CX * CY * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OY is min(CY,MaxMem/(2 * Ty)),    % from this tuple+1 on, Y will be entirely buffered on disk
  % case 2.2: X fits in memory, but Y does not; CX =< CY; OY > CX
  % this case has the same cost function as case1!
  (CX * (Tx+12)) =< (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CX =< CY, OY > CX,
  % write('\nSymmjoin Case2.2\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY              % produce arguments
       + RPD * FCMR * (CX * (CX+1)/2 +      (CY-CX)) * ExtFlobSize2/BDBPS % load flobs
       + RPD * FCMR * (CX * (CX+1)/2 + CX * (CY-CX)) * ExtFlobSize1/BDBPS % load flobs
       + WTM * (CX + CX)             % write X/Y-tuples to buffer in memory
       + B * CX * CY                 % check join condition
       + TC * CX * CY * Sel          % create result tuples  
       + RTM * ( CX*CX
               +(CY-CX)*CX),         % read X/Y-tuples from memory
  ResSize is CX * CY * Sel.
 
cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, _, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OY is min(CY,MaxMem/(2 * Ty)),    % from this tuple+1 on, Y will be entirely buffered on disk
  % case 2.3: X fits in memory, but Y does not; CX > CY
  (CX * (Tx+12)) =< (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CX > CY,
  % write('\nSymmjoin Case2.3\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY              % produce arguments
       + RPD * FCMR * (CY * (CY+1)/2 +      (CX-CY)) * ExtFlobSize1/BDBPS % load flobs
       + RPD * FCMR * (CY * (CY+1)/2 + CY * (CX-CY)) * ExtFlobSize2/BDBPS % load flobs
       + WTM * (CY + OY)             % write X/Y-tuples to buffer in memory
       + B * CX * CY                 % check join condition
       + TC * CX * CY * Sel          % create result tuples  
       + RTM * ( OY*OY
              +gsf(OY,CY))           % read X/Y-tuples from memory
       + FND + FDD                   % create and delete tmp file
       + WTD * CY                    % write Y-tuples to disk
       + RTD * ( gsf(OY,CY)
              +(CX-CY)*CY),          % read Y-tuples from disk
  ResSize is CX * CY * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)), % from this tuple+1 on, X will be entirely buffered on disk
  % case 3.1: Y fits in memory, but X does not; CY =< CX; OX =< CY
  (CY * (Ty+12)) =< (0.5 * MaxMem), 
  (CX * (Tx+12)) > (0.5 * MaxMem), CY =< CX, OX =< CY, 
  % write('\nSymmjoin Case3.1\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY              % produce arguments
       + RPD * FCMR * (CY * (CY+1)/2 +      (CX-CY)) * ExtFlobSize1/BDBPS % load flobs
       + RPD * FCMR * (CY * (CY+1)/2 + CY * (CX-CY)) * ExtFlobSize2/BDBPS % load flobs
       + WTM * (CY + OX)             % write elements to buffer in memory
       + B * CY * CX                 % check join condition
       + TC * CY * CX * Sel          % create result tuples  
       + RTM * ( OX*OX               
              +gsf(OX,CY)
              +(CX-CY)*CY)           % read X/Y-tuples from memory
       + FND + FDD                   % create and delete tmp file
       + WTD * CY                    % write X-tuples to disk
       + RTD * gsf(OX,CY),           % read X-tuples from disk
  ResSize is CY * CX * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, _, _, RPD, _, _, _, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)),    % from this tuple+1 on, X will be entirely buffered on disk
  % case 3.2: Y fits in memory, but X does not; CY =< CX; OX > CY
  % this case has the same cost function as case1!
  (CY * (Ty+12)) =< (0.5 * MaxMem), 
  (CX * (Tx+12)) > (0.5 * MaxMem), CY =< CX, OX > CY,
  % write('\nSymmjoin Case3.2\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY              % produce arguments
       + RPD * FCMR * (CY * (CY+1)/2 +      (CX-CY)) * ExtFlobSize1/BDBPS % load flobs
       + RPD * FCMR * (CY * (CY+1)/2 + CY * (CX-CY)) * ExtFlobSize2/BDBPS % load flobs
       + WTM * (CY + CY)             % write X/Y-tuples to buffer in memory
       + B * CY * CX                 % check join condition
       + TC * CY * CX * Sel          % create result tuples  
       + RTM * ( CY*CY
              +(CX-CY)*CY),          % read X/Y-tuples from memory
  ResSize is CY * CX * Sel.
 
cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)),    % from this tuple+1 on, X will be entirely buffered on disk
  % case 3.3: Y fits in memory, but X does not; CY > CX
  (CY * (Ty+12)) =< (0.5 * MaxMem), (CX * (Tx+12)) > (0.5 * MaxMem), CY > CX,
  % write('\nSymmjoin Case3.3\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY              % produce arguments
       + RPD * FCMR * (CX * (CX+1)/2 +      (CY-CX)) * ExtFlobSize2/BDBPS % load flobs
       + RPD * FCMR * (CX * (CX+1)/2 + CX * (CY-CX)) * ExtFlobSize1/BDBPS % load flobs
       + WTM * (CX + OX)             % write X/Y-tuples to buffer in memory
       + B * CY * CX                 % check join condition
       + TC * CY * CX * Sel          % create result tuples  
       + RTM * ( OX*OX
              +gsf(OX,CX))           % read X/Y-tuples from memory
       + FND + FDD                   % create and delete tmp file
       + WTD * CX                    % write X-tuples to disk
       + RTD * ( gsf(OX,CX)
                +(CY-CX)*CX),        % read X/Y-tuples from disk
  ResSize is CY * CX * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)), % from this tuple+1 on, X will be entirely buffered on disk
  OY is min(CY,MaxMem/(2 * Ty)), % from this tuple+1 on, Y will be entirely buffered on disk
  % case 4.1: neither X nor Y fit into memory; CX =< CY, OX =< OY
  (CX * (Tx+12)) > (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CX =< CY, OX =< OY,
  % write('\nSymmjoin Case4.1\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY
       + RPD * FCMR * (CX * (CX+1)/2 +      (CY-CX)) * ExtFlobSize2/BDBPS % load flobs
       + RPD * FCMR * (CX * (CX+1)/2 + CX * (CY-CX)) * ExtFlobSize1/BDBPS % load flobs
       + WTM * (OX + OY)             % write elements to buffer in memory
       + B * CX * CY                 % check join condition
       + TC * CX * CY * Sel          % create result tuples  
       + RTM * ( OX*OX                 
              +gsf(OX,OY)            
              +(CY-CX)*CX)           % read X/Y-tuples from memory
       + 2 * (FND + FDD)             % create and delete tmp files
       + WTD * (CX*CY)               % write X/Y-tuples to disk
       + RTD * ( gsf(OX,OY)            
              +gsf(OY,CX)            
              +(CY-CX)*CX),          % read X/Y-tuples from disk
  ResSize is CX * CY * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)), % from this tuple+1 on, X will be entirely buffered on disk
  OY is min(CY,MaxMem/(2 * Ty)), % from this tuple+1 on, Y will be entirely buffered on disk
  % case 4.2: neither X nor Y fit into memory; CX =< CY, OX > OY
  (CX * (Tx+12)) > (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CX =< CY, OX > OY,
  % write('\nSymmjoin Case4.2\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY
       + RPD * FCMR * (CX * (CX+1)/2 +      (CY-CX)) * ExtFlobSize2/BDBPS % load flobs
       + RPD * FCMR * (CX * (CX+1)/2 + CX * (CY-CX)) * ExtFlobSize1/BDBPS % load flobs
       + WTM * (OX + OY)             % write elements to buffer in memory.
       + B * CX * CY                 % check join condition
       + TC * CX * CY * Sel          % create result tuples  
       + RTM * ( OY*OY
              +gsf(OY,CX)
              +(CY-CX)*CX)           % read X/Y-tuples from memory
       + 2 * (FND + FDD)             % create and delete tmp files
       + WTD * (CX*CY)               % write X/Y-tuples to disk
       + RTD * ( gsf(OY,OX)
                +gsf(OX,CX)
                +(CY-CX)*CX),        % read X/Y-tuples from disk
  ResSize is CX * CY * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)), % from this tuple+1 on, X will be entirely buffered on disk
  OY is min(CY,MaxMem/(2 * Ty)), % from this tuple+1 on, Y will be entirely buffered on disk
  % case 4.3: neither X nor Y fit into memory; CY =< CX, OY =< OX
  (CX * (Tx+12)) > (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CY =< CX, OY =< OX,
  % write('\nSymmjoin Case4.3\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY
       + RPD * FCMR * (CY * (CY+1)/2 +      (CX-CY)) * ExtFlobSize1/BDBPS % load flobs
       + RPD * FCMR * (CY * (CY+1)/2 + CY * (CX-CY)) * ExtFlobSize2/BDBPS % load flobs
       + WTM * (OY + OX)             % write elements to buffer in memory.
       + B * CY * CX                 % check join condition
       + TC * CY * CX * Sel          % create result tuples  
       + RTM * ( OY*OY
              +gsf(OY,OX)
              +(CX-CY)*CY)           % read X/Y-tuples from memory
       + 2 * (FND + FDD)             % create and delete tmp files
       + WTD * (CY*CX)               % write X/Y-tuples to disk
       + RTD * ( gsf(OY,OX)
                +gsf(OX,CY)
                +(CX-CY)*CY),        % read X/Y-tuples from disk
  ResSize is CY * CX * Sel.

cost(symmjoin(X, Y, _), P, Sel, ResSize, TupleSize, B, TC, Cost) :-
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM, WTM, RTD, WTD, RPD, _, FND, FDD, _, _, MaxMem, BDBPS, FCMR),
  TX = tupleSizeData(TSCx, TSIx, _),
  TY = tupleSizeData(TSCy, TSIy, _),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  OX is min(CX,MaxMem/(2 * Tx)), % from this tuple+1 on, X will be entirely buffered on disk
  OY is min(CY,MaxMem/(2 * Ty)), % from this tuple+1 on, Y will be entirely buffered on disk
  % case 4.4: neither X nor Y fit into memory; CY =< CX, OY > OX
  (CX * (Tx+12)) > (0.5 * MaxMem), 
  (CY * (Ty+12)) > (0.5 * MaxMem), CY =< CX, OY > OX,
  % write('\nSymmjoin Case4.4\n'),
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2),
  Cost is CostX + CostY
       + RPD * FCMR * (CY * (CY+1)/2 +      (CX-CY)) * ExtFlobSize1/BDBPS % load flobs
       + RPD * FCMR * (CY * (CY+1)/2 + CY * (CX-CY)) * ExtFlobSize2/BDBPS % load flobs
       + WTM * (OY + OX)             % write elements to buffer in memory.
       + B * CY * CX                 % check join condition
       + TC * CY * CX * Sel          % create result tuples  
       + RTM * ( OX*OX
              +gsf(OX,CY)
              +(CX-CY)*CY)           % read X/Y-tuples from memory
       + 2 * (FND + FDD)             % create and delete tmp files
       + WTD * (CY*CX)               % write X/Y-tuples to disk
       + RTD * ( gsf(OX,OY)
                +gsf(OY,CY)
                +(CX-CY)*CY),        % read X/Y-tuples from disk
  ResSize is CX * CY * Sel.

cost(extend(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize1, PredCost, TC, C1),
  addTupleSizes(TupleSize1, tupleSizeData(12,0,0), TupleSize), 
  % XRIS: TupleSize should be increased by the real extension size
  extendTC(A),
  C is C1 + A * S.                   % XRIS: costs for evaluation of mapping to generate value 
                                     %       for new attributes not yet considered!

cost(remove(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize1, PredCost, TC, C1),
  addTupleSizes(TupleSize1,tupleSizeData(-12,0,0), TupleSize), 
  % XRIS: TupleSize should be decreased by the real removal size
  removeTC(A),
  C is C1 + A * S.

cost(project(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize, PredCost, TC, C1), 
  % XRIS: TupleSize should be reduced according to the projection!
  projectTC(A,B),
  TupleSize = tupleSizeData(TSC, TSI, _),
  C is C1 + max(A*S, B*S*(TSC+TSI)).

cost(rename(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize, PredCost, TC, C1),
  renameTC(A),
  C is C1 + A * S.

cost(head(X, N), P, Sel, Card, TupleSize, PredCost, TC, Cost) :-
  cost(X, P, Sel, CX, TupleSize, PredCost, TC, C1),
  aggregateTC(B),
  Card is min(N, CX),
  Cost is C1                              % create argument
        + B * Card.                       % overhead for counting each tuple

cost(counter(X, _), P, Sel, S, TupleSize, PredCost, TC, C) :-
  cost(X, P, Sel, S, TupleSize, PredCost, TC, C).


%fapra1590
% See cost(leftrange(...),...) for comments on future refinements.
cost(windowintersects(_, Rel, _), P, Sel, Size, TupleSize, B, TC, Cost) :-
  cost(Rel, P, 1, RelSize, TupleSize, TC, B, _),
  cost_factors(_,_,_,_,RPD,_,_,_,FOD,FCD,_, _, _),
  Size is Sel * RelSize,
  Cost is FOD + FCD                       % open/close RTree file        
        + B * (log(15,RelSize)+Size)      % find all matches,           later: B * (Height + K/2)
        + RPD * log(15,RelSize)           % read index pages from disk, later: D * (Height + Sel*RelSize/K)
        + TC * Sel * RelSize.             % produce tuple stream


/*

Cost function for operator spatialjoin as defined in file PlugJoinAlgebra.cpp.
The smaller relation should be passed as the inner relation (Y), as it is used
to build an index (R[*]Tree), while tuples from the outer relation are queried 
against the index. The join performs a test on intersection of BBoxes.

  1 All records of Y fit into memory. getMaxLeaves() returns infinite, number of 
    leaves remains unlimited. 

  2 Y does not completely fit into memory. X and Y will be partitioned pairwise. 
    Each pair of partitions corresponds to one leaf of the index, getMaxLeaves() 
    determines how many pairs of partitions are created.
    As long as getMaxLeaves() is not reached, tuples from Y are inserted into
    the Rtree. After the treshold getMaxLeaves() is passed, any overflow in a 
    leaf does not result in a structural change of the tree, but the tuple is 
    simply written to into the according Y-partition file. Partition files are 
    organized in buckets of size MaxMem/\#leaves.
    
    When all Y-tuples have been processed, each leaf in the RTree is examined. 
    If its Y-partition is not empty, all tuples from that leaf are flushed to 
    that Y-partition.

    Now, all tuples from X are processed: Each tuple is queried against the 
    Rtree and directed to all leaves and Y-partitions that might contain join
    partners (replication of x may occur). If a leaf is empty, x is spooled to
    the S-partition of that leaf, otherwise the join results are immeadeatly 
    reported.

    When all queries are processed, the Rtree is deleted and the operator is 
    restarted recursively for each partition with non-empty S- and R-part on 
    disk.

    Splitting an R[star]-tree costs $O(d \cdot M \log_2 M)$, for node capacity 
    $M$, number of dimensions $d$. 

    $\mbox{height}(Y) = \log_M (Y)$, $\mbox{nodes}(Y) = M^{\mbox{height}(Y)-1}$ 
    
    Creating an R[star]-tree ideally causes $\mbox{nodes}(Y) = M^{\mbox{height}(Y)-1}-1$ 
    splits.

*/


cost(spatialjoin(X,Y,_,_,D), P, Sel, Size, TupleSize, B, TC, Cost) :- 
  % XRIS: Flobs not yet considered
  % D is the dimension of the joinattributes' bounding boxes
  cost(X, P, 1, CX, TX, B, TC, CostX),
  cost(Y, P, 1, CY, TY, B, TC, CostY),
  cost_factors(RTM,WTM,_,_,RPD,WPD,FND,FDD,FOD,FCD,MaxMem,BDBPS,FCMR),
  TX = tupleSizeData(TSCx, TSIx, TSEx),
  TY = tupleSizeData(TSCy, TSIy, TSEy),
  Tx is TSCx + TSIx,
  Ty is TSCy + TSIy,
  addTupleSizes(TX,TY,TupleSize),
  getNeededExtFlobSize(P, ExtFlobSize1, ExtFlobSize2), % Flobs should not be needed...
  Size is Sel * CX * CY,
  MaxLeaves  is max(0,(MaxMem/(Ty+24))),        % determine number of leaves fitting into in-memory Rtree
  PartNumber is max(0,ceiling(CY/MaxLeaves)-1), % determine how many partitions will be created
  ( PartNumber > 1 -> 
               (
                  PartSizeY  is (CY-MaxLeaves)/PartNumber, % determine average size of spooled partitions
                  PartSizeX  is (CX/CY)*PartSizeY, 
                  % estimate cost for one recursive call:
                  cost(spatialjoin(fixed(PartSizeX,TX),
                       fixed(PartSizeY,TY),_,_,D), P, Sel, _, _, B, TC, CostR)
               )
	      ;(
                  CostR is 0
               )
  ),
         
/*
MaxLeavesOfRtree      is 10000, % parameter found in PlugJoinAlgebra.h
MinLeaveOfRtree       is    40, % parameter found in PlugJoinAlgebra.h
DefaultEntriesPerNode is    15, % parameter found in PlugJoinAlgebra.h
ScalingFactor         is    17, % parameter found in PlugJoinAlgebra.h
NodesToUse is max(min((ScalingFactor*(CX+CY)/(DefaultEntriesPerNode*MaxLeavesOfRtree)+1.0),
                       MaxLeavesOfRtree),
                  MinLeaveOfRtree), % found in PlugAndJoinAlgebra.cpp

*/
  Height = max(1,log(15,min(MaxLeaves,CY))),% fanout/node capacity set to 15 
  M = min(MaxLeaves, CY),
  INSERT_CONST is (RTM + WTM),  
  QUERY_CONST  is RTM * Height,
  SPLIT_CONST  is (RTM + WTM) * D * M * Height,

  Cost is CostX + CostY                     % produce arguments
        + INSERT_CONST * max(0,(CY-M))      % insert Y-tuples into Rtree
        + SPLIT_CONST * (15 ** (Height-1))  % resolve splits on insertions
        + QUERY_CONST * CX                  % query X-tuple against Rtree  
                                            % spool Y-tuples to partition file
        + (WPD+RPD) * ceiling((CY-M)*(Ty+TSEy)/BDBPS)
                                            % spool X-tuples to partition file 
        + (WPD+RPD) * ceiling(CX * (1 - M/CY)*(Tx+TSEx)/BDBPS)
        + (FND+FOD+FCD+FDD) * PartNumber    % handle partitions on disk
        + CostR * PartNumber                % cost for recursive calls
        + TC * Sel * M * CX * M/CY.         % create result tuples
