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
//[=>] [\verb+=>+]
//[toc] [\tableofcontents]
//[newpage] [\newpage]

Diploma thesis

[10] Precise Cost Estimation in the SECONDO extensible Database System

Author:   Artur Nawra, March 2006 \newline
Modified: Christian D[ue]ntgen, June 2006

[toc]

[newpage]

1 File ~DatabaseN.pl~

The file was merged with file ~database.pl~.



2 File ~costnew.pl~

*/


/*
Determine the assessed costs of an input term using rules ~cost\_new~. Then charge the total costs by applying rule ~costtotal~.

*/

costterm(Term, Sel, Source, Result, Size, CostTotal) :-
  costnew(Term, Sel, Source, Result, Size, TupleSize, Cost),
  setNodeTupleSize(Result, TupleSize),
  costtotal(Cost, CostTotal).

/*
Calculates the total costs of a term considering the calibration factors.

*/

costtotal(Cost, CostTotal) :-
  vCostFactorCPU(V),
  wCostFactor(W),
  CostCal is Cost * V,
  CostTotal is CostCal * W.

/*
Calculates the costs for a term under consideration for the calibration factor.

*/

costCal(Cost, CostCal) :-
  vCostFactorCPU(V),
  CostCal is Cost * V.

/*
The new rules for the cost estimation. The cost estimation for an input term goes
recursively. The arguments are: ~Term~, ~Sel~, ~Source~ and ~Result~.  ~term~ is 
an executable plan of an edge in the POG. ~Sel~ is its selectivity. The variables
~Source~ and ~Result~ contain the node number of the considered edge in the
POG. The remaining variables are the return value straps. ~Size~ is the tuple
number, ~TupleSize~ is the tuple size term, ~Cost~ is the assessed costs of the input term. Cost factors are used in all functions. The cost factors are contained in corresponding clauses of the form ~storedOperatorTF~.

*/

/*
Cost estimation for arguments. The arguments can be relations or intermediate
results.

*/

costnew(rel(Rel,_,_), _, _, _, Size, TupleSize, 0) :-
  card(Rel, Size),
  tupleSizeSplit(Rel, TupleSize), !.

costnew(res(N), _, _, _, Size, TupleSize, 0) :-
  resultSize(N, Size),
  nodeTupleSize(N, TupleSize), !.

/*
Cost estimation for operator feed.

*/

costnew(feed(X), Sel, Source, Result, Size, TupleSize, Cost) :-
     costnew(X, Sel, Source, Result, SizeX, TupleSizeX, CostX),
     storedOperatorTF(feed, A, B),
     Size is SizeX,
     TupleSize = TupleSizeX,
     TupleSize = sizeTerm(Core,Inflob,_),
     Cost is CostX + (A * (Core+Inflob) + B) * SizeX, !.

/*
Cost estimation for operator consume.

*/

costnew(consume(X), Sel, Source, Result, Size, TupleSize, Cost) :-
     costnew(X, Sel, Source, Result, SizeX, TupleSizeX, CostX),
     storedOperatorTF(consume, A, B),
     Size is SizeX,
     TupleSize = TupleSizeX,
     TupleSize = sizeTerm(X1,X2,X3),
     Cost is CostX + (A * (X1+X2+X3) + B) * SizeX, !.

/*
Cost for operator filter. The costs of predicates are taken account of.

For ~filter~, there are several special cases to distinguish:
 
  1 ~filter(spatialjoin(...),P)~

  2 ~filter(gettuples(...),P)~

  3 ~filter(windowintersects(...),P)~

  4 ``normal'' ~filter(...)~

For the first three cases, the edge is the translation of a spatial predicate, that
makes use of bounding box checks. The first argument of filter will already reduce
the set of possible candidates, so that the cardinality of tuples processed by filter
will be smaller than the cardinality passed down in the 3rd argument of ~cost~. Also, the
selectivity passed with the second argument of ~cost~ if the ~total~ selectivity. To 
get the selectivity of the preselection, one can analyse the predicate and lookup
the table ~storedBBoxSel/3~ for that selectivity, which should be passed to the recursive
call of ~cost~.

PROBLEM: What happens with the entropy-optimizer? As for cases 2 + 3, there is no 
problem, as the index is used to create a fresh tuple stream. But, as for case 1, we 
might get into problems, as the selectivity of the bbox-check depends on earlier 
predicates - so we should consider both selectivities in the minimization of the entropy.


*/

costnew(filter(X, _), Sel, Source, Result, Size, TupleSize, Cost) :-
    term_to_atom(X, XAtom),
    atom_prefix(XAtom, 'spatialjoin'),
    costnew(X, Sel, Source, Result, SizeX, TupleSizeX, CostX),
    getEdgePredCost(Source, Result, PredCost),
    % filterTC(A),
    Size is SizeX,
    TupleSize = TupleSizeX,
    Cost is CostX + PredCost * SizeX, !.


costnew(filter(X, _), Sel, Source, Result, Size, TupleSize, Cost) :-
    costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
    getEdgePredCost(Source, Result, PredCost),
    % filterTCnew(A),
    Size is SizeX * Sel,
    TupleSize = TupleSizeX,
    Cost is CostX + PredCost * SizeX, !.


/*
Cost estimation for operator product.

*/

costnew(product(X, Y), _, Source, Result, Size, TupleSize, Cost) :-
    costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
    costnew(Y, 1, Source, Result, SizeY, TupleSizeY, CostY),
    storedOperatorTF(producta, A1, B1),
    storedOperatorTF(productb, A2, B2),
    producttest(SizeY, TupleSizeY, C1),
    Size is SizeX * SizeY,
    addSizeTerms([TupleSizeX,TupleSizeY],TupleSize),
    TupleSizeX = sizeTerm(X1,X2,X3),
    TupleSizeY = sizeTerm(Y1,Y2,Y3),
    Cost is CostX + CostY + SizeX * ((X1+X2+X3) * A1 + B1)
             * SizeY * ((Y1+Y2+Y3) * A2 + B2) * C1, !.


/*
Cost estimation for operator loopjoin.

*/

costnew(loopjoin(X, Y), Sel, Source, Result, Size, TupleSize, Cost) :-
    costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
    costnew(Y, Sel, Source, Result, SizeY, TupleSizeY, CostY),
    storedOperatorTF(loopjoin, A2, B2),
    storedOperatorTF(concatenationflob, A3, _),
    % getEdgePredCost(Source, Result, _, PredCostET),
    Size is SizeX * SizeY,
    addSizeTerms([TupleSizeX,TupleSizeY],TupleSize),
    TupleSizeX = sizeTerm(X1,X2,X3),
    TupleSizeY = sizeTerm(Y1,Y2,Y3),
    FlobSize is X3+Y3,
    Cost is CostX + SizeX * CostY
                  + ((A2 * (X1+X2 + Y1+Y2) + B2) + (A3 * FlobSize))  * Size, !.


/*
Cost estimation for operator fun.

*/

costnew(fun(_, X), Sel, Source, Result, Size, TupleSize, Cost) :-
    costnew(X, Sel, Source, Result, Size, TupleSize, Cost), !.

/*
Cost estimate for operator hashjoin. 

Several aspects are taken into account: 

  * the number of the evaluations of the Join condition, 

  * the costs for the possible on-disk buffering of the first argument

  * the costs of the concatenation of the suitable tuples.

*/

costnew(hashjoin(X, Y, _, _, NBuckets), Sel, Source, Result, Size, TupleSize, Cost) :-
    costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
    costnew(Y, 1, Source, Result, SizeY, TupleSizeY, CostY),
    storedOperatorTF(hashtab, A, _),
    storedOperatorTF(concatenationjoin, A2, B2),
    storedOperatorTF(concatenationflob, A3, _),
    getEdgePredCost(Source, Result, PredCost),
    predEvalNumber(SizeX, SizeY, NBuckets, PredNr),
    tempRelCost(SizeX, TupleSizeX, SizeY, TupleSizeY, TempCost),
    Size is SizeX * SizeY * Sel,
    addSizeTerms([TupleSizeX,TupleSizeY],TupleSize),
    TupleSizeX = sizeTerm(X1,X2,X3),
    TupleSizeY = sizeTerm(Y1,Y2,Y3),
    FlobSize is X3 + Y3,
    Cost is CostX + CostY + (PredCost + A) * PredNr + TempCost
             + ((A2 * (X1+X2 + Y1+Y2) + B2) + (A3 * FlobSize))  * Size, !.


/*
Cost estimation for operator sortmergejoin.

Several aspects are taken into account: 

  * the number of the evaluations of the Join condition, 

  * the costs for the sorting of the arguments,

  * the costs of the concatenation of the suitable tuples.

The following rule has been replaced by a rule calling the rules for sortby and mergejoin,
please see below.

----
costnew(sortmergejoin(X, Y, _, _), Sel, Source, Result, Size, TupleSize, Cost) :-
    costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
    costnew(Y, 1, Source, Result, SizeY, TupleSizeY, CostY),
    storedOperatorTF(concatenationjoin, A2, B2),
    storedOperatorTF(concatenationflob, A3, _),
    getEdgePredCost(Source, Result, PredCost),
    sortcost(TupleSizeX, SizeX, SortCostX),
    sortcost(TupleSizeY, SizeY, SortCostY),
    Size is SizeX * SizeY * Sel,
    addSizeTerms([TupleSizeX,TupleSizeY],TupleSize),
    TupleSizeX = sizeTerm(X1,X2,X3),
    TupleSizeY = sizeTerm(Y1,Y2,Y3),
    FlobSize is X3 + Y3,
    Cost is CostX + CostY + SortCostX + SortCostY
            + PredCost * (SizeX + SizeY)
            + ((A2 * (X1+X2 + Y1+Y2) + B2) + (A3 * FlobSize)) * Size.

----

*/

/*
Cost estimation for operator extend. The costs for the compution of the value of the new attribute are taken into account.

*/

costnew(extend(X, Term), Sel, Source, Result, Size, TupleSize, Cost) :-
     costnew(X, Sel, Source, Result, SizeX, TupleSizeX, CostX),
     storedOperatorTF(extend, A, _),
     Size is SizeX,
     attrlistExtend(Term, ExtendSize, TermCost),
     addSizeTerms([TupleSizeX,ExtendSize],TupleSize),
     Cost is CostX + A  * SizeX + TermCost * SizeX, !.


/*
Cost estimation for operator remove.

*/

costnew(remove(X, L), Sel, Source, Result, Size, TupleSize, Cost) :-
     costnew(X, Sel, Source, Result, SizeX, TupleSizeX, CostX),
     storedOperatorTF(remove, A, _),
     Size is SizeX,
     attrlistRemove(L, RemoveSize),
     addSizeTerms([TupleSizeX,RemoveSize],TupleSize),
     Cost is CostX + A * SizeX, !.


/*
Cost estimation for operator rename.

*/

costnew(rename(X, _), Sel, Source, Result, Size, TupleSize, Cost) :-
     costnew(X, Sel, Source, Result, SizeX, TupleSizeX, CostX),
     storedOperatorTF(rename, A, _),
     Size is SizeX,
     TupleSize = TupleSizeX,
     Cost is CostX + A * SizeX, !.


/*
Cost estimation for operator leftrange.

*/

costnew(leftrange(_, X, _), Sel, Source, Result, Size, TupleSize, Cost) :-
     costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
     storedOperatorTF(leftrange, A, B),
     getEdgePredCost(Source, Result, PredCost),
     Size is SizeX * Sel,
     TupleSize = TupleSizeX,
     TupleSizeX = sizeTerm(Core, InFlob, _),
     Cost is CostX + PredCost * Size + (A * (Core + InFlob) + B) * Size, !.

/*
Cost estimation for operator rightrange.

*/

costnew(rightrange(_, X, _), Sel, Source, Result, Size, TupleSize, Cost) :-
     costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
     storedOperatorTF(leftrange, A, B),
     getEdgePredCost(Source, Result, PredCost),
     Size is SizeX * Sel,
     TupleSize = TupleSizeX,
     TupleSizeX = sizeTerm(Core, InFlob, _),
     Cost is CostX + PredCost * Size + (A * (Core + InFlob) + B) * Size, !.

/*
Cost estimation for operator exactmatchfun.

*/

costnew(exactmatchfun(_, X, _), Sel, Source, Result, Size, TupleSize, Cost) :-
     costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
     storedOperatorTF(exactmatchfun, A, B),
     getEdgePredCost(Source, Result, PredCost),
     Size is SizeX * Sel,
     TupleSize = TupleSizeX,
     TupleSizeX = sizeTerm(Core, InFlob, _),
     Cost is CostX + PredCost * Size + (A * (Core + InFlob) + B) * Size, !.


/*
Cost estimation for operator exactmatch.

*/

costnew(exactmatch(_, X, _), Sel, Source, Result, Size, TupleSize, Cost) :-
     costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
     storedOperatorTF(exactmatch, A, B),
     getEdgePredCost(Source, Result, PredCost),
     Size is SizeX * Sel,
     TupleSize = TupleSizeX,
     TupleSize = sizeTerm(Core, InFlob, _),
     Cost is CostX + PredCost * Size + (A * (Core + InFlob) + B) * Size, !.

/*
Cost estimation for operator windowintersects.

*/

costnew(windowintersects(_, X, _), Sel, Source, Result, Size, TupleSize, Cost) :-
     costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
     storedOperatorTF(windowintersects, A, B),
     Size is Sel * SizeX,
     TupleSize = TupleSizeX,
     TupleSize = sizeTerm(Core, InFlob, _),
     Cost is CostX + (A * (Core + InFlob) + B) * Size, !.


/*
Cots estimation for operator windowintersectsS.

*/
% Cost function not verified:
costnew(windowintersectsS(IndexName, _), Sel, Source, Result, 
        Size, sizeTerm(12,0,0), Cost) :- % constant tuplesize for [tid]
  % get relationName Rel from Index
  concat_atom([RelNameSmall|_],'_',IndexName),
  spelled(RelNameSmall, RelName, RelCase),
  Rel = rel(RelName, *, RelCase),
  costnew(Rel, 1, Source, Result, RelSize, TupleSize, _),
  asserta(storedWindowIntersectsS(TupleSize)), % store tuplesize for gettuples
  storedOperatorTF(windowintersects, A, B),
  Size is Sel * RelSize,
  Cost is (A * 12 + B) * RelSize, !. % returns a stream of tids

/*
Cost estimation for operator gettuples

*/
% Cost function not verified:
costnew(gettuples(X, _), Sel, Source, Result, Size, TupleSize, Cost) :-
  costnew(X, Sel, Source, Result, SizeX, _, CostX),
  storedWindowIntersectsS(TupleSize), % get tuplesize stored by windowintersectsS
  retract(storedWindowIntersectsS(TupleSize)),
  storedOperatorTF(feed, A, B),
  TupleSize = sizeTerm(Core, InFlob, _),
  Size is SizeX,
  Cost is CostX + (A * (Core + InFlob) + B) * SizeX, !.

/*
Cost estimation for operator project

*/
costnew(project(X, Arg), Sel, Source, Result, Size, TupleSize, Cost) :-
  costnew(X, Sel, Source, Result, SizeX, _, CostX),
  attrlistSize(Arg, ArgTupleSize),
  TupleSize = ArgTupleSize,
  Size is SizeX,
  Cost is CostX, !.

/*
Cost estimation for operator symmjoin

*/
costnew(symmjoin(X, Y, _), Sel, Source, Result, Size, TupleSize, Cost) :-
    costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
    costnew(Y, 1, Source, Result, SizeY, TupleSizeY, CostY),
    storedOperatorTF(concatenationjoin, A2, B2),
    storedOperatorTF(concatenationflob, A3, _),
    getEdgePredCost(Source, Result, PredCost),
    tempRelCostsymmjoin(SizeX, TupleSizeX, SizeY, TupleSizeY, TempCost),
    Size is SizeX * SizeY * Sel,
    addSizeTerms([TupleSizeX,TupleSizeY], TupleSize),
    TupleSizeX = sizeTerm(CoreX, InFlobX, ExtFlobX),
    TupleSizeY = sizeTerm(CoreY, InFlobY, ExtFlobY),
    FlobSize     is ExtFlobX + ExtFlobY,
    TupleSizeExt is CoreX + CoreY + InFlobX + InFlobY,
    Cost is CostX + CostY + PredCost * (SizeX * SizeY) + TempCost
             + ((A2 * TupleSizeExt + B2) + (A3 * FlobSize))  * Size, !.

/*
Cost estimation for operator spatialjoin

*/
costnew(spatialjoin(X, Y, _, _), Sel, Source, Result, Size, TupleSize, Cost) :-
    costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
    costnew(Y, 1, Source, Result, SizeY, TupleSizeY, CostY),
    storedOperatorTF(concatenationjoin, A2, B2),
    storedOperatorTF(concatenationflob, A3, _),
    storedOperatorTF(spatialjoinx, A1x, B1x),
    storedOperatorTF(spatialjoiny, A1y, B1y),
    % getEdgePredCost(Source, Result, PredCost),
    Size is SizeX * SizeY * Sel,
    addSizeTerms([TupleSizeX,TupleSizeY], TupleSize),
    TupleSizeX = sizeTerm(CoreX, InFlobX, ExtFlobX),
    TupleSizeY = sizeTerm(CoreY, InFlobY, ExtFlobY),
    FlobSize        is ExtFlobX + ExtFlobY,
    TupleSizeExt    is CoreX + CoreY + InFlobX + InFlobY,
    TotalTupleSizeX is CoreX + InFlobX + ExtFlobX,
    TotalTupleSizeY is CoreY + InFlobY + ExtFlobY,
    Cost is CostX + CostY
             + ((SizeX * log(SizeX + 1) * TotalTupleSizeX * A1x) 
             + (SizeX^2 * TotalTupleSizeX * B1x)) /2
             + ((SizeY * log(SizeY + 1) * TotalTupleSizeY * A1y) 
             + (SizeY^2 * TotalTupleSizeY * B1y)) /2
             + ((A2 * TupleSizeExt + B2) + (A3 * FlobSize))  * Size, !.


/*
Cost estimation for operator consume

*/
costnew(consume(X), Sel, Source, Result, Size, TupleSize, Cost) :-
  costnew(X, Sel, Source, Result, SizeX, TupleSizeX, CostX),
  consumeTC(A),
  Cost is CostX + A * Size,
  Size is SizeX,
  TupleSize = TupleSizeX, !.


/*
For operator pjoin, only dummy functions are implemented:

*/
costnew(pjoin2(_, _, [ _ | _ ]), Sel, _, _, Sel, _, Cost) :-
  Cost is 1, !.

costnew(pjoin1(_, _, [ _ | _ ]), Sel, _, _, Sel, _, Cost) :-
  Cost is 0, !.


/*
Cost estimation for operator rdup

*/
% Cost function not verified:
costnew(rdup(X), Sel, Source, Result, Size, TupleSize, Cost) :-
  costnew(X, Sel, Source, Result, SizeX, TupleSize, Cost1),
  sortcost(TupleSize, SizeX, CostRDUP),
  Size is SizeX,
  Cost is Cost1 + CostRDUP, !.


/*
Cost estimation for operator sort

*/
costnew(sort(X), Sel, Source, Result, Size, TupleSize, Cost) :- 
  costnew(X, Sel, Source, Result, SizeX, TupleSizeX, CostX),
  sortcost(TupleSizeX, SizeX, SortCost),
  TupleSize = TupleSizeX,
  Size is SizeX,
  Cost is CostX + SortCost, !.


/*
Cost estimation for operator sortby

*/
costnew(sortby(X, _), Sel, Source, Result, S, TupleSize, C) :-
  costnew(sort(X), Sel, Source, Result, S, TupleSize, C), !.
  
/*
Cost estimation for operator mergejoin

*/
costnew(mergejoin(X, Y, _, _), Sel, Source, Result, Size, TupleSize, Cost) :-
  costnew(X, 1, Source, Result, SizeX, TupleSizeX, CostX),
  costnew(Y, 1, Source, Result, SizeY, TupleSizeY, CostY),
  storedOperatorTF(concatenationjoin, A2, B2),
  storedOperatorTF(concatenationflob, A3, _),
  getEdgePredCost(Source, Result, PredCost),
  Size is SizeX * SizeY * Sel,
  addSizeTerms([TupleSizeX,TupleSizeY],TupleSize),
  TupleSizeX = sizeTerm(X1,X2,X3),
  TupleSizeY = sizeTerm(Y1,Y2,Y3),
  FlobSize is X3 + Y3,
  Cost is CostX + CostY 
          + PredCost * (SizeX + SizeY)
          + ((A2 * (X1+X2 + Y1+Y2) + B2) + (A3 * FlobSize)) * Size, !.

/*
Cost estimation for operator sortmergejoin

*/

costnew(sortmergejoin(X, Y, AX, AY), Sel, Source, Result, S, TS, C) :-
  costnew(mergejoin(sortby(X, [AX]),sortby(Y, [AY]), AX, AY), 
          Sel, Source, Result, S, TS, C), !.


% two rules used by the 'interesting orders extension':
costnew(sortLeftThenMergejoin(X, Y, AX, AY), Sel, Source, Target, S, TS, C) :-
  costnew(mergejoin(sortby(X, [AX]), Y, AX, AY), Sel, Source, Target, S, TS, C), 
  !.

costnew(sortRightThenMergejoin(X, Y, AX, AY), Sel, Source, Target, S, TS, C) :-
  costnew(mergejoin(X, sortby(Y, [AY]), AX, AY), Sel, Source, Target, S, TS, C), 
  !.


/* 
Predicates auxiliary to cost:
 
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

/*
Rules to estimation costs for internal and external sorting.

*/

sortcost(TupleSize, Size, Cost) :-
   TupleSize = sizeTerm(Core, InFlob, _),
   bufferSize(Memory),
   V is (Core + InFlob) * Size,
   Memory > V,
   storedOperatorTF(intsort, A1, B1),
   Cost is Size * log(Size) * (A1 * (Core + InFlob) + B1),
% Should be:   
%   Cost is Size * log(Size) * (A1 * (Core + InFlob) + B1),
   !.

sortcost(TupleSize, Size, Cost) :-
   TupleSize = sizeTerm(Core, InFlob, _),
   TupleSizeExt is Core + InFlob,
   bufferSize(Memory),
   storedOperatorTF(intsort, A1, B1),
   storedOperatorTF(extsort, A2, B2),
   PSize is Memory / TupleSizeExt,
   L is Size / PSize, 
   Cost is Size * log(PSize) * (A1 * TupleSizeExt + B1)
           + Size * log(L) * 1.44 * (A2 * TupleSizeExt + B2),
% Should be:
%   L is Size / (2 * PSize), 
%   Cost is Size * log(2,PSize) * (A1 * TupleSizeExt + B1)
%           + Size * log(2,L) * 1.44 * (A2 * TupleSizeExt + B2),
   !.


/*
Estimating the number of evaluations of a predicate during the hashjoin.

*/

predEvalNumber(SizeX, SizeY, NBuckets, PredNr) :-
   NBuckets > SizeY,
   PredNr is SizeX, !.

predEvalNumber(SizeX, SizeY, NBuckets, PredNr) :-
   PredNr is SizeX * (SizeY / NBuckets), !.

/*
Cost estimation for temporary storing an argument during the hashjoin.

*/

tempRelCost(SizeX, TupleSizeX, SizeY, TupleSizeY, TempCost) :-
   TupleSizeX = sizeTerm(X1,X2,_),
   TupleSizeY = sizeTerm(Y1,Y2,_),   
   TupleSizeExtX is X1 + X2,
   TupleSizeExtY is Y1 + Y2,
   bufferSize(Memory),
   PSizeX is (Memory * 0.25) / TupleSizeExtX,
   PSizeY is (Memory * 0.75) / TupleSizeExtY,
   PSizeX < SizeX,
   PSizeY < SizeY,
   storedOperatorTF(hashtemp, A1, B1),
   storedOperatorTF(hashcachecal, A2, B2),
   TempRelSize is SizeX - PSizeX,
   HashTabNr is SizeY / (Memory * 0.75) * (TupleSizeExtY * A2 + B2),
   TempCost is TempRelSize * (TupleSizeExtX * A1 + B1) * (HashTabNr + 1),
   !.

tempRelCost(_, _, _, _, 0).


tempRelCostsymmjoin(SizeX, TupleSizeX, SizeY, TupleSizeY, TempCost) :-
   bufferSize(Memory),
   TupleSizeX = sizeTerm(CoreX, InFlobX, _),
   TupleSizeY = sizeTerm(CoreY, InFlobY, _),
   PSizeX is (Memory * 0.5) / (CoreX + InFlobX),
   PSizeX < SizeX,
   storedOperatorTF(hashtemp, A1, B1),
   storedOperatorTF(hashcachecal, A2, B2),
   TempRelSize is SizeX - PSizeX,
   Inter is SizeY * ((CoreY + InFlobY) * A2 + B2),
   TempCost is TempRelSize * ((CoreX + InFlobX) * A1 + B1) * (Inter + 1),
   !.

tempRelCostsymmjoin(SizeX, TupleSizeX, SizeY, TupleSizeY, TempCost) :-
   bufferSize(Memory),
   TupleSizeX = sizeTerm(CoreX, InFlobX, _),
   TupleSizeY = sizeTerm(CoreY, InFlobY, _),
   PSizeY is (Memory * 0.5) / (CoreY + InFlobY),
   PSizeY < SizeY,
   storedOperatorTF(hashtemp, A1, B1),
   storedOperatorTF(hashcachecal, A2, B2),
   TempRelSize is SizeY - PSizeY,
   Inter is SizeX * ((CoreX + InFlobX) * A2 + B2),
   TempCost is TempRelSize * ((CoreY + InFlobY) * A1 + B1) * (Inter + 1),
   !.

tempRelCostsymmjoin(_, _, _, _, 0) :- !.

/*
Test for buffer overflow in operator product.

*/

producttest(Size, TupleSize, CF) :-
  TupleSize = sizeTerm(Core,Inflob,Extflob),
  bufferSize(Memory),
  V is Size * (Core+Inflob+Extflob),
  V > Memory,
  CF is 1,
  !.

producttest(_, _, CF) :-
  storedOperatorTF(productc, CF, _), !.


/*
Return the argument, if it is positive. Otherwise return 0.

*/

notnegativ(Zahl, Zahl) :- Zahl >= 0, !.
notnegativ(_, 0) :- !.

/*
Return the estimated cost of a single application of the predicate belonging 
to the POG-egde (Source Result). 

XRIS: The choosing of PET type should become more intelligent!

*/

getEdgePredCost(Source, Target, PET) :-
  getPredNoPET(Source, Target, _, PET), !.  % chooce calculated PETs
%  getPredNoPET(Source, Target, PET, _), !. % choose mesured PETs
  

/*
The clauses test, if a number (attribute size) is smaller than 1000.

*/

sizeKlass(Size, Size, Size) :- Size < 1000, !.
sizeKlass(Size, 0, Size) :- !.

/*
Rules calculating tuple size after applying operator project.
The clauses have been moved to file ``database.pl''.
*/


/*
Calculate the joint attribute size (tuple size) after execution of
the extend operator as well as the costs, which are connected to the expansion.

---- attrlistExtend(+AttrList, -TupleSize, -Cost)
----

The clauses use a dynamic predicate ~storedExtendAttrSize/2~ to store 
information on extension attributes.

---- attrlistRemove(+AttrList, Size)
----

Calculates the joint attribute sizes for list ~AttrList~ and multiplies it by -1

*/

attrlistExtend([X], Size, Cost) :-
  attrlistExtend(X, Size, Cost), !.

attrlistExtend([X|Xs], Size, Cost) :-
  attrlistExtend(X, SizeX, CostX),
  attrlistExtend(Xs, SizeXs, CostXs),
  addSizeTerms([SizeX,SizeXs],Size),
  Cost is CostX + CostXs, !.

attrlistExtend(newattr(Attr, Term), Size, Cost) :-
  simpleTerm(Term, STerm, Size),
  setExtendAttrSize(Attr, Size),
  extendSTermCost(STerm, Cost), !.


setExtendAttrSize(attrname(attr(Attr, Arg, Case)), Size) :-
  storedExtendAttrSize(attrname(attr(Attr, _, _)), _, _),
  retract(storedExtendAttrSize(attrname(attr(Attr, _, _)), _)),
  assert(storedExtendAttrSize(attrname(attr(Attr, Arg, Case)), Size)),
  !.

setExtendAttrSize(Attr, Size) :-
  assert(storedExtendAttrSize(Attr, Size)), !.

resExtendAttrSize(Attr, Size) :-
  storedExtendAttrSize(Attr, Size), !.


attrlistRemove([], sizeTerm(0,0,0)) :- !.

attrlistRemove([X|Xs], Size) :-
  attrlistRemove(X, SizeX),
  attrlistRemove(Xs, SizeXs),
  addSizeTerms([SizeX,SizeXs],Size), !.

% special case: TupleIDs in _small relations
attrlistRemove(X, sizeTerm(-12,0,0)) :-
  (  X = attrname(attr(_:Attr, _, _))
   ; X = attr(_:Attr, _, _)
   ; X = _:Attr
   ; X = attrname(attr(Attr, _, _))
   ; X = attr(Attr, _, _)
   ; X = Attr   
  ),
  atomic(Attr),
  atom_concat('xxxID', Rel, Attr),
  databaseName(DB),
  ( storedSpell(DB, DCRel, Rel) ; storedSpell(DB, DCRel, lc(Rel)) ),
  relation(DCRel,_), !.

attrlistRemove(attrname(attr(Attr, _, _)), 
               sizeTerm(CoreR, InFlobR, ExtFlobR)) :-
  storedExtendAttrSize(attrname(attr(Attr, _, _)), 
                       sizeTerm(Core, InFlob, ExtFlob)),
  CoreR    is -1 * Core,
  InFlobR  is -1 * InFlob,
  ExtFlobR is -1 * ExtFlob, !.

attrlistRemove(attrname(attr(Var:Attr, _, _)), 
               sizeTerm(CoreR, InFlobR, ExtFlobR)) :-
  argument( _, rel(Rel, Var, _)),
  downcase_atom(Attr, DCAttr),
  downcase_atom(Rel, DCRel),
  attrSize(DCRel:DCAttr, sizeTerm(Core, InFlob, ExtFlob)), 
  CoreR    is -1 * Core,
  InFlobR  is -1 * InFlob,
  ExtFlobR is -1 * ExtFlob, !.
%  sizeKlass(AttrSize, SizeExt, Size). % Omitted here.

attrlistRemove(attrname(attr(Attr, _, _)), AttrSize) :-
  % queryRel(_, rel(Rel, _, _)),
  argument( _, rel(Rel, _, _)),
  downcase_atom(Attr, DCAttr),
  downcase_atom(Rel, DCRel),
  attrSize(DCRel:DCAttr, AttrSize), !.
%  sizeKlass(AttrSize, SizeExt, Size). % omitted here.



attrlistRemove(X, _) :-
  throw(sql_ERROR(nawra_attrlistRemove(X, undefined))),
  fail, !.


/*
Calculation of Attribute/Projection List Sizes Terms

The following Rules calculate the combined tuple sizes for a list of attributes, 
e.g. to use when applying a ~project~ operator.

---- attrlistSize(+AttrList, sizeTerm(-CoreSize, -InFlobSize, -ExtFlobSize))
----

Returns a term ~sizeTerm(CoreSize, InFlobSize, ExtFlobSize)~ with the 
accumulated tuplesize fractions for all attributes found in ~AttrList~.

The predicate can cope with list elements of different types: 
~attrname(attr/3)~-terms, ~attr/3~-terms, and plain ~attribute names~ 
resp. ~var:attr-name~.

PRECONDITION: Can only be called during a query, when the query has been looked up!

*/

attrlistSize([], sizeTerm(0, 0, 0)) :- !.

attrlistSize([X|Xs], SizeTerm) :-
  attrlistSize2(X, SizeTerm1),
  attrlistSize(Xs, SizeTerm2),
  addSizeTerms([SizeTerm1, SizeTerm2], SizeTerm), !.

attrlistSize(X, _) :-
  throw(sql_ERROR(nawra_attrlistSize(X, undefined))),
  fail, !.

attrlistSize2(X, AttrSize) :-
  (  X = attrname(attr(Var:Attr, _, _))
   ; X = attr(Var:Attr, _, _)
   ; X = Var:Attr
  ),
  atomic(Attr), atomic(Var),
  argument( _, rel(Rel, Var, _)),
  downcase_atom(Attr, DCAttr),
  downcase_atom(Rel, DCRel),
  relation(DCRel,RelAttrList),
  member(DCAttr,RelAttrList),
  attrSize(DCRel:DCAttr, AttrSize), !.
%  sizeKlass(AttrSize, TupleSizeExt, TupleSize). % Omitted here

% special case: TupleIDs in _small relations
attrlistSize2(X, sizeTerm(12, 0, 0)) :-
  (  X = attrname(attr(_:Attr, _, _))
   ; X = attr(_:Attr, _, _)
   ; X = _:Attr
   ; X = attrname(attr(Attr, _, _))
   ; X = attr(Attr, _, _)
   ; X = Attr
  ),
  atomic(Attr),
  atom_concat('xxxID', Rel, Attr),
  databaseName(DB),
  ( storedSpell(DB, DCRel, Rel) ; storedSpell(DB, DCRel, lc(Rel)) ),
  relation(DCRel, _), !.

attrlistSize2(X, AttrSize) :-
  (  X = attrname(attr(Attr, _, _))
   ; X = attr(Attr, _, _)
   ; X = Attr
  ),
  atomic(Attr),
  % queryRel(_, rel(Rel, _, _)),
  argument( _, rel(Rel, _, _)),
  downcase_atom(Attr, DCAttr),
  downcase_atom(Rel, DCRel),
  relation(DCRel,RelAttrList),
  member(DCAttr,RelAttrList),
  attrSize(DCRel:DCAttr, AttrSize), !.
%  sizeKlass(AttrSize, TupleSizeExt, TupleSize). % Omitted here


/*
---- simpleTerm(+Term, -STerm, -Size)
----

Simplify a ~Term~ representing extension mapping function, return the simplified 
term ~STerm~, and the corresponding sizeTerm ~Size~ (containing the componentwise maxima).

*/

simpleTerm(attr(Var:Attr, _, _), Rel:Attr, AttrSize) :-
  argument(_, rel(Rel, Var, _)),
  downcase_atom(Rel, DCRel),
  downcase_atom(Attr, DCAttr),
  attrSize(DCRel:DCAttr, AttrSize), !.

simpleTerm(attr(Attr, _, _), Rel:Attr, AttrSize) :-
  argument(_, rel(Rel, _, _)),
  downcase_atom(Rel, DCRel),
  downcase_atom(Attr, DCAttr),
  attrSize(DCRel:DCAttr, AttrSize), !.

simpleTerm(Term, STerm, Size) :-
  compound(Term),
  functor(Term, T, 1), !,
  arg(1, Term, Arg1),
  simpleTerm(Arg1, SArg1, Size),
  functor(STerm, T, 1),
  arg(1, STerm, SArg1), !.

simpleTerm(Term, STerm, sizeTerm(Core, InFlob, ExtFlob)) :-
  compound(Term),
  functor(Term, T, 2), !,
  arg(1, Term, Arg1),
  arg(2, Term, Arg2),
  simpleTerm(Arg1, SArg1, sizeTerm(Core1, InFlob1, ExtFlob1)),
  simpleTerm(Arg2, SArg2, sizeTerm(Core2, InFlob2, ExtFlob2)),
  Core    is max(Core1,    Core2),
  InFlob  is max(InFlob1,  InFlob2),
  ExtFlob is max(ExtFlob1, ExtFlob2),
  functor(STerm, T, 2),
  arg(1, STerm, SArg1),
  arg(2, STerm, SArg2), !.

simpleTerm(Term, Term, 0) :- !.


/*
Calculate the cost of an extend operator only once. Store the result a a dynamic 
fact and reuse the information.

*/

extendSTermCost(STerm, Cost) :- storedExtendSTermCost(STerm, Cost), !.

extendSTermCost(STerm, Cost) :-
  predCost(STerm, Cost),
  setExtendSTermCost(STerm, Cost), !.


setExtendSTermCost(STerm, _) :- storedExtendSTermCost(STerm, _), !.

setExtendSTermCost(STerm, Cost) :- 
  assert(storedExtendSTermCost(STerm, Cost)), !.

/*
List with operators which not listing in the POG, are contained in the plan.
The tuple size of the result and the tuple size without Flobs of the result are
shown on the display.

*/

costnewPlan(_, 0) :- 
  write('There isn\'t a POG. Costs aren\'t calculated '), nl, !.

costnewPlan(Plan, _) :-
  write('Operators without POG :'),
  highNode(HighNode),
  endcostnew(Plan, HighNode, _, TupleSize),
  nl, write('The tuple size of the result (tuplesize) is: '),
  write(TupleSize), nl, !.





/*
Estimate costs for the non-conjunctive part of the query.

*/

/*
Estimate costs for operator consume.

*/

endcostnew(consume(X), HighNode, Size, TupleSize) :-
 
  endcostnew(X, HighNode, SizeX, TupleSizeX),
  TupleSizeX = sizeTerm(Core, InFlob, ExtFlob),
  TotalResultTupleSize is Core + InFlob + ExtFlob,
  storedOperatorTF(consume, A, B),
  Cost is (A * TotalResultTupleSize + B) * SizeX,
  costtotal(Cost, CostTotal),
  wCostFactor(W),
  Cs is CostTotal / W,
  write(' consume(Cost: '),
  write(Cs), write(' sec.),'),
  Size is SizeX,
  TupleSize = TupleSizeX,
  !.

/*
Determine costs for operator project.

*/

endcostnew(project(X, Arg), HighNode, Size, TupleSize) :-
  write(' project,'),
  endcostnew(X, HighNode, SizeX, _, _),
  attrlistSize(Arg, ArgTupleSize),
  TupleSize = ArgTupleSize,
  Size is SizeX,
  !.

/*
Determine costs for operator count.

*/

endcostnew(count(X), HighNode, Size, TupleSize) :-
   write(' count,'),
  endcostnew(X, HighNode, SizeX, TupleSizeX),
  TupleSize = TupleSizeX,
  Size is SizeX,
  !.

/*
Determine costs for operator sortby.

*/

endcostnew(sortby(X, _), HighNode, Size, TupleSize) :-
   write(' sortby,'),
  endcostnew(X, HighNode, SizeX, TupleSizeX),
  TupleSize = TupleSizeX,
  Size is SizeX,
  !.

/*
Determine costs for operator groupby.

*/

endcostnew(groupby( X, _, _), HighNode, Size, TupleSize) :-
   write(' groupby,'),
  endcostnew(X, HighNode, SizeX, TupleSizeX),
  TupleSize = TupleSizeX,
  Size is SizeX,
  !.



endcostnew(_, HighNode, Size, TupleSize) :-
  write(' (Costs'),
  resultSize(HighNode, Size),
  nodeTupleSize(HighNode, TupleSize),
  write(' aren`t calculated),'), !.

endcostnew( _, _, 0, 0) :- 
  nl, 
  write('Error at the determination of the operators without POG.'), 
  nl.
