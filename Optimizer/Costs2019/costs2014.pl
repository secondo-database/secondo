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
//[toc] [\tableofcontents]
//[newpage] [\newpage]

[10] Memory-Aware Cost Estimation

[toc]

1 The Costs of Terms

----	cost(+Term, +Sel, +Pred, +Res, +Mem, -Card, -NAttrs, -TSize, -Cost)
----

The cost of an executable ~Term~ representing a predicate ~Pred~ with selectivity ~Sel~ is ~Cost~ and the cardinality of the result (number of tuples) is ~Card~. The resulting tuples have ~NAttrs~ attributes and tuple size terms ~TSize~ = sizeTerm(MemoryFix, DiskCore, DiskLOB). The result will belong to node ~Res~. If the root operator of the ~Term~ uses a memory buffer, the amount of available memory is ~Mem~ (in MB).

This is evaluated recursively descending into the term. When the operator realizing the predicate (e.g. ~filter~) is encountered, the selectivity ~Sel~ is used to determine the size of the result. Usually only a single operator of this kind occurs within the term. However, with the filter and refine technique (used e.g. for spatial join), selectivity needs to be split into bounding-box selectivity and remaining selectivity.


1.1 Arguments

*/

cost(rel(Rel, _), _, _, _, _, Card, NAttrs, TSize, 0) :-
  card(Rel, Card),
  tupleSizeSplit(Rel, TSize),
  getRelAttrList(Rel, OrigAttrs, _),
  length(OrigAttrs, NAttrs).
  

cost(res(N), _, _, _, _, Card, NAttrs, TSize, 0) :-
  resultSize(N, Card),
  nodeTupleSize(N, TSize),
  nodeNAttrs(N, NAttrs).


cost(feed(X), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, NAttrs, TSize, CostX),
  feedC(PerTuple, PerAttr),
  Cost is CostX + Card * (PerTuple + PerAttr * NAttrs).

cost(feedproject(rel(Rel, _), Project), _, _, _, _, 
	Card, NAttrs, TSize, Cost) :-
  cost(rel(Rel, _), _, _, _, _, Card, _, _, CostX),
  getRelAttrList(Rel, OrigAttrs, _),
  attributes(Project, AList),
  projectAttrList(OrigAttrs, AList, _, TSize1),
  length(AList, NAttrs),
  % add memory required for embedding attributes in tuples
  TSize1 = sizeTerm(MemoryFix, S2, S3),
  TS1 is MemoryFix + 136,
  ( NAttrs > 10 -> TS is TS1 + 8 * NAttrs
                  ; TS is TS1
  ),
  TSize  =  sizeTerm(TS,S2,S3),
  feedprojectC(PerTuple, PerAttr, PerByte),
  Cost is CostX + Card * 
    (PerTuple + PerAttr * NAttrs + PerByte * TS).
	

cost(consume(X), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
	cost(X, Sel, Pred, Res, Mem, Card, NAttrs, TSize, CostX),
	nodeNAttrs(Res, NAttrs),
    nodeTupleSize(Res, TSize),
	consumeC(PerTuple),
	Cost is CostX + (Card*PerTuple).

% Nestedrelations operators work with the same cost models as their counterparts

cost(afeed(X), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, NAttrs, TSize, CostX),
  feedC(PerTuple, PerAttr),
  Cost is CostX + Card * (PerTuple + PerAttr * NAttrs).

cost(aconsume(X), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
	cost(X, Sel, Pred, Res, Mem, Card, NAttrs, TSize, CostX),
	nodeNAttrs(Res, NAttrs),
    nodeTupleSize(Res, TSize),
	consumeC(PerTuple),
	Cost is CostX + (Card*PerTuple).

% This is now needed, but i don't have any costs models 
% for this operations so i just make this work with no costs.

cost(unnest(X, _), Sel, Pred, Res, Mem, Card, _, _, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, _, _, Cost).
cost(nest(X, _, _), Sel, Pred, Res, Mem, Card, _, _, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, _, _, Cost).
cost(groupby(X, _, _), Sel, Pred, Res, Mem, Card, _, _, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, _, _, Cost).
cost(transformstream(X), Sel, Pred, Res, Mem, Card,  _, _, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, _, _, Cost).
cost(namedtransformstream(X, _), Sel, Pred, Res, Mem, Card,  _, _, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, _, _, Cost).
cost(predinfo(X, _, _), Sel, Pred, Res, Mem, Card,  _, _, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, _, _, Cost).
cost(renameattr(X, _), Sel, Pred, Res, Mem, Card,  _, _, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, _, _, Cost).
cost(extendstream(X, _), Sel, Pred, Res, Mem, Card,  _, _, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, _, _, Cost).
cost(count(X), Sel, Pred, Res, Mem, Card,  _, _, Cost) :-
 cost(X, Sel, Pred, Res, Mem, Card, _, _, Cost).
  
% See nr.pl:nrLookupRel
cost(attribute(_, _), _Sel, _Pred, _, _, 0, _, _, 0).
cost(a(_, _, _), _Sel, _Pred, _, _, 0, _, _, 0).

  
/*
1.2 filter

Filter evaluates a predicate on each tuple of its input stream, hence the cost is linear in the size of the input stream. 
On top of the basic cost for a cheap predicate, there may be the cost for evaluating an expensive predicate.

Derivation of constants: We use database nrw, constructed with script nrwImportShape.SEC from secondo/bin/Scripts. Relation Roads has schema

----	Roads(Osm_id: int, Name: string, Ref: string, Type: string, 
          Oneway: int, Bridge: int, Maxspeed: int, GeoData: Line
----

The total number of tuples is 735683

memtuplesize: 382.65

roottuplesize: 192

exttuplesize: 293.71

tuplesize: 1318.45

The sizes determined in the optimizer are:
[newpage]

----
Relation Roads	(Auxiliary objects:)
	AttributeName               Type          MemoryFix  DiskCore DiskLOB
	GeoData                     line          160        212.992  1124.74
	Maxspeed                    int           16         5.0      0
	Bridge                      int           16         5.0      0
	Oneway                      int           16         5.0      0
	Type                        string        64         22.0     0
	Ref                         string        64         7.44     0
	Name                        string        64         29.136   0
	Osm_id                      int           16         5.0      0

	Indices: 


	Ordering:  []


	Cardinality:   735683
	Avg.TSize: 1416.3 = sizeTerm(416, 291.568, 1124.74)
----

Evaluating a cheap predicate:

----	sql select count(*) from roads where maxspeed > 50.

Predicate Cost  : (1e-06/0.101117) ms
Selectivity     : 0.0111699

----

This query was executed as 

----	query Roads  feedproject[Maxspeed] 
          filter[(.Maxspeed > 50)]{0.0111587, 0.101117} count
----

within 3.57 seconds, returning the number 8404. Here the differential cost (obtained by running the above query without the filter operator) of the filter operator is 3.57 - 1.87 = 1.7 seconds, which is 1.7 / 735683 = 0.0023107 ms.

We construct a rather large line object by putting together all pieces of the Rhein river:

----	let rhein = Waterways  feed filter[(.Name contains "Rhein")] 
          project[GeoData] transformstream collect_line[TRUE]
----

We then assume this is a relatively expensive predicate:

----	select count(*) from Roads where GeoData intersects rhein

Selectivity query : query Roads_sample_s  feed {1}  
  filter[(bbox(.GeoData) bboxintersects bbox(rhein))] {2}  
  filter[(.GeoData intersects rhein)] timeout[10.0] count
Elapsed Time: 2325 ms
Predicate Cost  : (5.00463e-07/1.36684) ms
Selectivity     : 0.000587889
BBox-Selectivity: 0.462669

----

The query was executed as

----	query Roads  feedproject[GeoData] 
          filter[(.GeoData intersects rhein)]{0.000587302, 1.36684} count
----

within 924 seconds, returning 189. Here the differential cost of the filter operator is 924 - 2.3 seconds.

We interpret these results as follows. 
The cheap predicate has a predicate cost of 0.101117 ms. 
As the sample relation has about 1700 tuples, this corresponds to a query time for the sample query of 170 ms. 
However we have a basic time for processing a query of about that magnitude, so that the real predicate evaluation time is much smaller. 
This is why the predicate can be evaluated on the complete relation in 3.57 seconds.

On the other hand, the expensive predicate has a cost of 1.36684 ms, corresponding to dividing the total time for the sample query of 2325 seconds by 1700. 
In this case the predicate cost is close to the real one which leads to an estimate of 735683 * 1.36684 ms = 1005 seconds, a bit higher than the observed time of 924 seconds.

We will proceed as follows. We set a threshold value of 0.2 ms, corresponding to processing 2000 tuples, the standard sample size, within 400 ms.
Any value below this threshold is considered to be a cheap predicate with real costs 0 (more precisely, its cost is included in the tuple constant). 
A predicate with a higher cost is considered to be an expensive predicate, and its cost is added to the standard tuple cost.

*/

cost(filter(X, _), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :- 
  not(prefilter(X)), 	% this is the standard case
  !,
  cost(X, 1, Pred, Res, Mem, CardX, NAttrs, TSize, CostX),
  getPET(Pred, _, ExpPET),   % fetch stored predicate evaluation time
  filterC(PerTuple),
	  ( (ExpPET =< 0.2) -> (PET is 0.0); PET is ExpPET),
  Card is CardX * Sel,
  Cost is CostX + CardX * (PerTuple + PET).


cost(filter(X, _), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :- 
  prefilter(X), 	% filter and refinement case,
  simplePred(Pred, P),	% hence lookup bbox selectivity
  databaseName(DB),
  storedBBoxSel(DB, P, Sel2),
  cost(X, Sel2, Pred, Res, Mem, CardX, NAttrs, TSize, CostX),
  getPET(Pred, _, ExpPET),   % fetch stored predicate evaluation time
  filterC(PerTuple),
  ( (ExpPET =< 0.2) -> (PET is 0.0); PET is ExpPET),
  Card is CardX * (Sel / Sel2),
  Cost is CostX + CardX * (PerTuple + PET).


/*
1.3 rename

*/

cost(rename(X, _), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, NAttrs, TSize, CostX),
  renameC(PerTuple),
  Cost is CostX + Card * PerTuple.
  
/*
1.4 Extend, Remove, Project 

*/  

cost(extend(X, _), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, CardX, _, _, CostX),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  extendC(PerTuple, PerAttr),
  Card is Sel * CardX,
  Cost is CostX + Card * (PerTuple + NAttrs * PerAttr).

cost(remove(X, _), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, CardX, _, _, CostX),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  removeC(PerTuple),
  Card is Sel * CardX,
  Cost is CostX + PerTuple * CardX.

cost(project(X, _), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, CardX, _, _, CostX),
  projectC(PerTuple),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize), 
  Card is Sel * CardX,
  Cost is CostX + PerTuple * CardX.

cost(projectextend(X, ProjectionList, ExtensionList), Sel, 
	Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, CardX, _, _, CostX),
  length(ProjectionList, PL),
  length(ExtensionList, EL),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),  
  extendC(EC),
  projectC(PC),
  Card is CardX * Sel,
  Cost is CostX + (PC * PL + EC * EL) * CardX.  
  
/*
1.5 Exactmatch

*/

cost(exactmatchfun(_, Rel, _), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(Rel, 1, Pred, Res, Mem, CardX, _, _, _),
  exactmatchC(C),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  Card is Sel * CardX,
  Cost is Sel * CardX * C.

cost(exactmatch(_, Rel, _), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(Rel, 1, Pred, Res, Mem, CardX, _, _, _),
  exactmatchC(C),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  Card is Sel * CardX, 
  Cost is Sel * CardX * C.

cost(exactmatchS(dbobject(Index), _KeyValue), Sel, _Pred, Res, _, 
	Card, NAttrs, TSize, Cost) :-
  dcName2externalName(DcIndexName,Index),
  getIndexStatistics(DcIndexName, noentries, _DCrelName, CardX),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  exactmatchC(C),
  Card is Sel * CardX,
  Cost is Sel * CardX * C * 0.25 . % balance of 75% is for gettuples

/*
1.6 Windowintersects

*/ 

%fapra1590
cost(windowintersects(_, Rel, _), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(Rel, 1, Pred, Res, Mem, CardX, _, _, _),
  windowintersectsC(C),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize), 
  Card is Sel * CardX,
  Cost is Sel * CardX * C.

% Cost function copied from windowintersects
% May be wrong, but as it is usually used together
% with 'gettuples', the total cost should be OK
cost(windowintersectsS(dbobject(IndexName), _), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  % get relationName Rel from Index
  my_concat_atom([RelName|_],'_',IndexName),
  dcName2internalName(RelDC,RelName),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize), 
  Rel = rel(RelDC, *),
  cost(Rel, 1, Pred, Res, Mem, CardX, _, _, _),
  windowintersectsC(C),
  Card is Sel * CardX,  % bad estimation, may contain additional duplicates
  Cost is Sel * CardX * C * 0.25. % other 0.75 applied in 'gettuples'

cost(gettuples(X, _), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, CardX, _, _, CostX),
  windowintersectsC(C),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize), 
  Card is CardX,
  Cost is  CostX            % expected to include cost of 'windowintersectsS'
          + Card * C * 0.75. % other 0.25 applied in 'windowintersectsS'
 
 
cost(gettuples2(X, Rel, _IdAttrName), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(gettuples(X, Rel), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost).

cost(ksmallest(X, K), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, CardX, _, _, CostX),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize), 
  ksmallestC(A, B),
  Card is min(CardX, K),
  Cost is CostX +
    A * CardX +
    B * Card * log(Card + 1).
 
/*
1.7 Join Operator

1.7.1 Loopjoin

Cost is linear in the size of the first argument.

*/

cost(loopjoin(X, Y), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, 1, Pred, Res, Mem, CardX, _, _, CostX),
  cost(Y, Sel, Pred, Res, Mem, CardY, _, _, CostY),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  attrC(PerAttr),
  Card is CardX * CardY,
  loopjoinC(A, B),
  C1 is A * CardX + CostX + ((CardX * CostY) / B),
  C2 is Card * NAttrs * PerAttr, 
  Cost is C1 + C2.

/*
1.7.2 Symmjoin

Symmjoin is the fallback method for join as it is able to process an arbitrary join predicate. It is a symmetric version of a nested-loop join, considering all pairs of tuples from the two argument streams and evaluating the predicate for them. It works roughly as follows:

  1 For the two input streams, initialize two buffers ~A~ and ~B~ as empty.

  2 While not both input streams are exhausted, do:

    1 If stream 1 is not empty, take the first tuple $t_1$ from stream 1 and put it into buffer ~A~. For each tuple $t_2$ in buffer ~B~ evaluate the predicate on the pair $(t_1, t_2)$.

    2 If stream 2 is not empty, take the first tuple $t_2$ from stream 2 and put it into buffer ~B~. For each tuple $t_1$ in buffer ~A~ evaluate the predicate on the pair $(t_1, t_2)$.

Hence the procedure alternates between the two input streams and therefore is called symmetric. One advantage is that there is no blocking time as for standard nested loop which first loads one entire argument into the buffer. Instead, result tuples can be reported immediately.

Obviously the cost is quadratic, or more precisely, the product of the argument sizes. When the available memory is exceeded, there is some change in cost as tuples need to be written to disk. However, we do not need to model this in the cost function, as the quadratic behaviour dominates the cost. Except in rare cases, ~symmjoin~ will only be used if no other join technique is available, because it is so expensive.

Similar as for filter, we need to distinguish between cheap and expensive predicates. Again we introduce a threshold cost and add the cost for evaluating the predicate only if the measured cost exceeds the threshold.

Let ~ExpPET~ denote the predicate evaluation time measured in the selectivity query and ~th~ the threshold that determines whether we have an expensive predicate. The constant ~u~ denotes the cost for processing one pair of tuples with a cheap predicate.

The cost function is:

\[
\begin{array}{lll}
PET & = &
\left\{ \begin{array}{ll}
	ExpPET  & \mbox{if $ExpPET > th$} 	\\
	0 & \mbox{otherwise} \\
	\end{array}
\right. \\
%
& & \\
%
C_{symmjoin}  & = & 
Card_1 * Card_2 * (u + PET) \\
& & + Card \cdot y \cdot NAttrs
\end{array}
\]

*/

cost(symmjoin(Arg1, Arg2, _), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(Arg1, 1, Pred, Res, Mem, CardX, _, _, CostX), 
  cost(Arg2, 1, Pred, Res, Mem, CardY, _, _, CostY),
  Card is CardX * CardY * Sel,
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  symmjoinC(U),
  getPET(Pred, _, ExpPET),   % fetch stored predicate evaluation time
  ( (ExpPET =< 0.01) -> (PET is 0.0); PET is ExpPET),
  C1 is CardX * CardY * (U + PET),
  attrC(PerAttr), 			% Cost dep. on result size,
  C2 is Card * NAttrs * PerAttr,	% the same for all joins
  Cost is CostX + CostY + C1 + C2.



/* 
1.7.3 HashJoin

*/

cost(hashjoin(X, Y, _, _, NBuckets), Sel, Pred, Res, 
	Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, 1, Pred, Res, Mem, CardX, _, _, CostX),
  cost(Y, 1, Pred, Res, Mem, CardY, _, _, CostY),
  Card is CardX * CardY * Sel,
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  hashjoinC(_, B),
  H is		% producing the arguments
  B * NBuckets * (CardX/NBuckets +1) * (CardY/NBuckets+1),
  attrC(PerAttr),		% Cost depending on result size,
  C2 is Card * NAttrs * PerAttr,
  Cost is CostX + CostY + H + C2.		% producing the result tuples  

/*
1.7.4 itHashJoin

Iterative HashJoin reads the tuples from its first input stream into a hash table in memory. There are two cases:

  1 The tuples from the first stream fit completely into memory. 
  Then the tuples from the second stream are read and each of them probes against the hash table to find join partners.

  2 Only an initial part of the tuples from the first stream fits into memory.
  Then the second stream is read, probing against the hashtable as in case 1.
  However, additionally the tuples from the second stream are written into a tuple file (that is, to disk).

    When this is finished, the hash table is emptied and further tuples from the first input stream are read into it, called the second partition. 
	The tuples from the second argument are now read from the tuple file and probed against the second partition.

    This is repeated until all partitions have been processed.

For the cost formula we use the following notations:

  * $Mem$: the available memory, in MB

  * $Card_i$: the cardinalities of the first / second input stream

  * $NAttrs_i$: the numbers of attributes of the first / second argument stream

  * $Size_i$: the tuple sizes in memory of the first / second input stream. Tuple sizes are taken from the ~MemoryFix~ component of the ~sizeTerm~.

  * $Card$: the cardinality of the output.

  * $NAttrs$: the number of attributes of the output

  * $P$: the number of partitions where $P = \lceil(Card_1 \cdot Size_1) / (Mem * 1024 * 1024)\rceil$.

  * $u, v, w, x, y$: constants with the following meanings (denoted as $uHashJoin$ etc. in a global table of constants): 

    * $u$ - putting one tuple from stream 1 into the hash table

    * $v$ - time for processing one tuple of stream 2, if $P = 1$

    * $w$ - time for writing one byte (of stream 2) into a tuple file

    * $x$ - time for reading one byte from the tuple file

    * $y$ - time for processing one attribute in a result tuple

The cost for iterative hash join then is the following:

\[
\begin{array}{lll}
C_{itHashJoin}  & = & 
\left\{ \begin{array}{ll}
	Card_1 \cdot u + Card_2 \cdot v & \mbox{if $P$ = 1} 	\\
	Card_1 \cdot u	+ Card_2 \cdot Size_2 \cdot w & \mbox{otherwise} \\
	+ (P - 1) \cdot Card_2 \cdot Size_2 \cdot x &     	\\
	\end{array}
\right. \\
& & + Card \cdot y \cdot NAttrs
\end{array}
\]

These costs are reflected in the following cost function.

*/

/*
%KS ADDED
In the new version of this the cost function is adjusted to the version of the regular hashjoin.
If the relation fits in 1 Partition into the memory, then the hashing is ignored and a similiar cost function as of hashjoin is used here. 
This means only the probing is relevant in this case. No. of buckets is hard coded as its not part of the cost arguments that are given to the cost function.
If more then 1 Partition is needed, then the hashing is factored in additional to the write and read process of the second stream. 

*/

cost(itHashJoin(Arg1, Arg2, _, _), Sel, Pred, Res, Mem, 
Card, NAttrs, TSize, Cost) :-
  cost(Arg1, 1, Pred, Res, Mem, CardX, _, sizeTerm(SizeX, _, _), CostX), 
  cost(Arg2, 1, Pred, Res, Mem, CardY, _, sizeTerm(SizeY, _, _), CostY),
  Card is CardX * CardY * Sel,
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  P is 1 + floor((CardX * SizeX) / ((Mem * 1024 * 1024)*0.8)), % # of partitions
  itHashJoinC(U, V, W, X),
  ( P = 1 
    -> C1 is V * 999997 * ((CardX/999997) +1) * ((CardY/999997) +1)
    ;  C1 is CardX * U 
         + CardY * SizeY * W
         + (P - 1) * CardY * SizeY * X
  ),
  attrC(PerAttr), 			% Cost depending on result size,
  C2 is Card * NAttrs * PerAttr,  % the same for all joins
  Cost is CostX + CostY + C1 + C2.
  
/*
1.7.5 SpatialJoin 

Effort is essentially proportional to the sizes of the argument streams

*/

cost(spatialjoin(X, Y, _, _), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(X, 1, Pred, Res, Mem, CardX, _, _, CostX),
  cost(Y, 1, Pred, Res, Mem, CardY, _, _, CostY),
  spatialjoinC(A),
  attrC(PerAttr),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  Card is Sel * CardX * CardY,
  C1 is A * (CardX + CardY),
  C2 is Card * NAttrs * PerAttr,   	% Same for all joins
  Cost is C1 + C2 + CostX + CostY. 



/*
1.7.6 itSpatialJoin

The task is to join tuples from two input streams that have overlapping bounding boxes for some attribute.

This operator works in the same way as ~itHashJoin~. 
The only difference is that instead of a hash table, a main memory R-tree is used. 
Probes search with a rectangle on this R-tree. 
The cost formula is a bit different as for itHashJoin, as this operator does not involve buckets.
So the cost of Hashing and Probing are determined individual.

\[
\begin{array}{lll}
C_{itSpatialJoin}  & = & 
\left\{ \begin{array}{ll}
	Card_1 \cdot u + Card_2 \cdot v & \mbox{if $P$ = 1} 	\\
	Card_1 \cdot u	+ Card_2 \cdot Size_2 \cdot w & \mbox{otherwise} \\
	+ (P - 1) \cdot Card_2 \cdot Size_2 \cdot x &     	\\
	\end{array}
\right. \\
& & + Card \cdot y \cdot NAttrs
\end{array}
\]

*/


cost(itSpatialJoin(Arg1, Arg2, _, _), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(Arg1, 1, Pred, Res, Mem, CardX, _, sizeTerm(SizeX, _, _), CostX), 
  cost(Arg2, 1, Pred, Res, Mem, CardY, _, sizeTerm(SizeY, _, _), CostY),
  Card is CardX * CardY * Sel,
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  P is 1 + floor((CardX * SizeX) / (Mem * 1024 * 1024)), % # of partitions
  itSpatialJoinC(U, V, W, X),
  ( P = 1 
    -> C1 is CardX * U + CardY * V
    ;  C1 is CardX * U 
         + CardY * SizeY * W
         + (P - 1) * CardY * SizeY * X 
  ),
  attrC(PerAttr), 			% Cost depending on result size,
  C2 is Card * NAttrs * PerAttr,	% the same for all joins
  Cost is CostX + CostY + C1 + C2.
  

/*
1.7.7 Sortmergejoin

If cost for roth Relations is exactly the same, it is assumed that it is the same relation with the same sorting attribute. 
therefore: Cost is CostX + CostY 	
in that case the cost for the mergephase of sortmergejoin is 0, as the relation is the same and sorted beforehand.

Otherwise the regular cost of the mergephase is calculated.

*/

cost(sort(X), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, CardX, NAttrs, TSize, CostX),
  sortmergejoinC(A, _),
  Card is CardX,
  Cost is CostX + 	% producing the argument
    A * CardX.		% linear in practice


% Sortby with empty sorting list is ignored:
cost(sortby(X, []), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost).

cost(sortby(X, Y), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  Y \= [],
  cost(sort(X), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost).

cost(mergejoin(X, Y, _, _), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, 1, Pred, Res, Mem, CardX, _, _, CostX),
  cost(Y, 1, Pred, Res, Mem, CardY, _, _, CostY),
  sortmergejoinC(_, B),
  attrC(PerAttr),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  Card is CardX * CardY * Sel,
   ( CostX = CostY 	
	-> Cost is CostX + CostY
	; Cost is CostX + CostY + (Card * NAttrs * PerAttr) 
		+ (B * (CardX + CardY)) 	
  ). 

cost(sortmergejoin(X, Y, AX, AY), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(mergejoin(sortby(X, [AX]), sortby(Y, [AY]), AX, AY), 
	Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost).

% two rules used by the 'interesting orders extension':
cost(sortLeftThenMergejoin(X, Y, AX, AY), Sel, Pred, Res, 
	Mem, Card, NAttrs, TSize, Cost) :-
  cost(mergejoin(sortby(X, [AX]), Y, AX, AY), Sel, Pred, Res, 
	Mem, Card, NAttrs, TSize, Cost).

cost(sortRightThenMergejoin(X, Y, AX, AY), Sel, Pred, Res, 
	Mem, Card, NAttrs, TSize, Cost) :-
  cost(mergejoin(X, sortby(Y, [AY]), AX, AY), Sel, Pred, Res, 
	Mem, Card, NAttrs, TSize, Cost).


/*
1.7.8 Pjoin (Not essential) 

only used if adapativeJoin option is enabled

*/
cost(pjoin2(X, Y, [ _ | _ ]), Sel, Pred, Res, Mem, Card, _, _, Cost) :-
  cost(X, 1, Pred, Res, Mem, CardX, _, _, _),
  cost(Y, 1, Pred, Res, Mem, CardY, _, _, _),
  Card is Sel * CardX * CardY,
  cost(sortmergejoin(X, Y, _, _), Sel, Pred, Res, Mem, _, _, _, CostS),
  cost(hashjoin(X, Y, _, _, 99997), Sel, Pred, Res, Mem, _, _, _, CostH),
  Cost is min(CostS, CostH).

cost(pjoin2_hj(X, Y, [ _ | _ ]), Sel, Pred, 
	Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(hashjoin(X, Y, _, _, 99997), Sel, Pred, 
	Res, Mem, Card, NAttrs, TSize, Cost).

cost(pjoin2_smj(X, Y, [ _ | _ ]), Sel, Pred, 
	Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(sortmergejoin(X, Y, _, _), Sel, Pred, 
	Res, Mem, Card, NAttrs, TSize, Cost).

/*
1.7.9 Rdup \& krdup 

These use the cost constants from SortMergejoin. Here only the first one is used, this is the sorting constant.

----
Testcases for eventual further usage:
query Waterways feed sortby[GeoData asc] head[1] count
Result: 1
Time: 2.58 seconds

query Waterways feed sortby[GeoData asc] krdup[GeoData] count
Result: 110689
Time: 3.08 seconds

3.08 - 2.58 = 0.5 seconds for 110689 Tuples. This is 0.0045 ms / Tuple.

query Waterways feed sort count
Result: 110691
Time: 1.00 seconds

query Waterways feed sort rdup count
Result: 110691
Time: 1.07 seconds

1.07 - 1.00 = 0.07 for 110691 Tuples. This is 0.0006 ms per Tuple.
----

*/  
cost(rdup(X), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, CardX, _, _, CostX),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),  
  sortmergejoinC(A, _),
  Card is CardX * Sel,
  Cost is CostX + A * CardX.

cost(krdup(X,_AttrList), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(rdup(X), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost).  


/*
1.8 Other

1.8.1 CreateTempBTree 

*/
cost(createtmpbtree(rel(Rel, _), _), _, _, _, _, Card, _, _, Cost) :-
  createbtreeC(C),
  card(Rel, Card),
  Cost is C * Card * log(Card + 1).
  

/* 
1.8.2 PogStream

get the size from the POG's high node
 
*/
cost(pogstream, _, _, _, _, Card, _, _, 0) :-
  highNode(Node),
  resultSize(Node, Card).  
  
/*
1.8.3 Matching 

Cost = Card/2
For matching a collection of symbolic trajectories (mlabel or mstring)

*/
cost(matches(rel(Rel, _), _, _), Sel, _, Res,  _, 
	Card, NAttrs, TSize, Cost) :-
  card(Rel, CardX),  
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize), 
  Card is CardX * Sel,
  Cost is 0.5 * Card.

cost(filtermatches(rel(Rel, _), _, _), Sel, _, Res,  _, 
	Card, NAttrs, TSize, Cost) :-
  card(Rel, CardX),  
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize), 
  Card is CardX * Sel,
  Cost is 0.5 * Card.

cost(indexmatches(rel(Rel, _), _, _, _), Sel, _, Res,  _, 
	Card, NAttrs, TSize, Cost) :-
  card(Rel, CardX),  
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize), 
  Card is CardX * Sel,
  Cost is 0.5 * Card.
 
/*
1.8.4 Product

*/
cost(product(X, Y), _, Pred, Res, Mem, Card, _, _, Cost) :-
  cost(X, 1, Pred, Res, Mem, CardX, _, _, CostX),
  cost(Y, 1, Pred, Res, Mem, CardY, _, _, CostY),
  productC(A, B),
  Card is CardX * CardY,
  Cost is CostX + CostY + CardY * A + Card * B.

/*
1.8.5 Range

*/
cost(range(_, Rel, _), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(Rel, 1, Pred, Res, Mem, CardX, _, _, _),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  leftrangeC(C),
  Card is Sel * CardX,
  Cost is Sel * CardX * C.

cost(rangeS(dbobject(Index), _KeyValue), Sel, _Pred, Res, _, 
	Card, NAttrs, TSize, Cost) :-
  dcName2externalName(DcIndexName,Index),
  getIndexStatistics(DcIndexName, noentries, _DCrelName, CardX),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  leftrangeC(C),
  Card is Sel * CardX,
  Cost is Sel * CardX * C * 0.25 . % balance of 75% is for gettuples
  
cost(leftrange(_, Rel, _), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(Rel, 1, Pred, Res, Mem, CardX, _, _, _),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  leftrangeC(C),
  Card is Sel * CardX,
  Cost is Sel * CardX * C.

cost(leftrangeS(dbobject(Index), _KeyValue), Sel, _Pred, 
	Res, _, Card, NAttrs, TSize, Cost) :-
  dcName2externalName(DcIndexName,Index),
  getIndexStatistics(DcIndexName, noentries, _DCrelName, CardX),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  leftrangeC(C),
  Card is Sel * CardX,
  Cost is Sel * CardX * C * 0.25 . % balance of 75% is for gettuples 

cost(rightrange(_, Rel, _), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(Rel, 1, Pred, Res, Mem, CardX, _, _, _),
   nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  leftrangeC(C),
  Card is Sel * CardX,
  Cost is Sel * CardX * C.

cost(rightrangeS(dbobject(Index), _KeyValue), Sel, _Pred, 
	Res, _, Card, NAttrs, TSize, Cost) :-
  dcName2externalName(DcIndexName,Index),
  getIndexStatistics(DcIndexName, noentries, _DCrelName, CardX),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  leftrangeC(C),
  Card is Sel * CardX,
  Cost is Sel * CardX * C * 0.25 . % balance of 75% is for gettuples
  
/*
1.8.6 gettuples

*/

% Cost function for inverted file access. Not yet determined, for the moment
% using formula similar to rangeS.
cost(gettuples(rdup(sortby(
	project(SearchTerm, attrname(attr(tid, 1, u))),
    attrname(attr(tid, 1, u)))),
    Rel), Sel, _Pred, Res, _, Card, NAttrs, TSize, Cost) :-
  (SearchTerm = searchWord(dbobject(_), _); 
    SearchTerm = searchPrefix(dbobject(_), _)),
  cost(Rel, _, _, CardX, _),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  leftrangeC(C),
  Card is Sel * CardX,
  Cost is Sel * CardX * C * 0.25 . % balance of 75% is for gettuples
  
cost(fun(_, X), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost).

/*
1.8.7 indextmatches

*/

cost(indextmatches(_, Rel, _, _), Sel, Pred, Res, 
	Mem, Card, NAttrs, TSize, Cost) :-
  cost(Rel, 1, Pred, Res, Mem, CardX, _, _, _, _),
  indextmatchesC(C),
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  Card is Sel * CardX,
  Cost is CardX * C.  
  
/*
1.8.8 distancesan

*/  

cost(distancescan(_, Rel, _, _), Sel, Pred, Res, 
	Mem, Card, NAttrs, TSize, Cost) :-
  cost(Rel, Sel, Pred, Res, Mem, CardX, _, _ , CostX),
  distancescanC(C),
    nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  Card is CardX,
  Cost is CostX + C * Card * log(Card + 1).
  
/*
2 Auxiliary Predicates

----	prefilter(+X)
----  

is fulfilled if ~X~ is an operation like loopjoin or spatialjoin so that the expression ~filter(X)~ represents a filter and refinement strategy.

*/

prefilter(itSpatialJoin(_, _, _, _)).
prefilter(spatialjoin(_, _, _, _)).
prefilter(loopjoin(_, _)).

/*
3 Constants per Script

These constants are evaluated by the script cConst.psec. 
These must be deleted before the script is run. The script will attach the constants at the end of the file.



distancescanC(0.25). %manually evaluated, do not delete

feedprojectC(0.000636,0.000533,1e-06).	 
 
 feedC(0.005,9e-05).	 
 
 consumeC(0.220049).	 
 
 attrC(8.9e-05).	 

 filterC(0.000592).
 
renameC(0.000163). 
 
 extendC(0.001132, 0.000313).	 
 
 removeC(0.000764).	 
 
 projectC(0.000313).	 
 
 createbtreeC(0.013808).	 
 
 productC(0.005596,0.001123).	 
 
 exactmatchC(0.000027).	 
 
 windowintersectsC(0.000021).	 
 
 leftrangeC(0.008546).	 
 
 ksmallestC(9.9e-05, 0.29659933).	 
 
 loopjoinC(0.001885, 3.0588).	 
 
 symmjoinC(0.000356).	 
 
 hashjoinC(-1.7e-05, 0.001674).	 
 
 itHashJoinC(0.001531,0.000005,0.00322,0.0074). 
 
 spatialjoinC(0.030247). 
 
 itSpatialJoinC(0.009579, 0.0015, 2e-05, 1e-05). 
 
 sortmergejoinC(0.02222, 0.03111).
 
*/

distancescanC(0.25).
 
indextmatchesC(0.029269). 
 
feedprojectC(0.000817,0.000375,1e-06). 
 
feedC(0.004,0.00011). 
 
consumeC(0.147772). 
 
attrC(6.1e-05). 
 
filterC(0.000946). 
 
renameC(0.00016). 
 
extendC(0.00093,0.000279). 
 
removeC(0.000719). 
 
projectC(0.000224). 
 
productC(0.005603,0.001). 
 
ksmallestC(0.000109,0.29285133). 
 
createbtreeC(0.019358). 
 
exactmatchC(2.1e-05). 
 
windowintersectsC(2.1e-05). 
 
leftrangeC(0.002989). 
 
loopjoinC(0.002744,5.0).  
% the second should be greater than atleast 3 
% else the Index was already buffered and the  
% constant is off (reference value is 5) 
 
symmjoinC(0.000358). 
 
sortmergejoinC(0.01739,0.001406). 
 
hashjoinC(0,0.001819). 
 
spatialjoinC(0.030233). 
 
itHashJoinC(0.00129,9.6e-05,3.8e-05,0.000396).  
% Partitions for first Query: 1,  
% Partitions for second Query: 2,  
% Partitions for third Query: 3,  
% these should be 1, 2 and 3, else  
% the read/write Constants may be off 
 
itSpatialJoinC(0.007196,0.00114,0.000196,0.000297).  
% Partitions for first Query: 1,  
% Partitions for second Query: 2,  
% Partitions for third Query: 3,  
% these should be 1, 2 and 3, else  
% the read/write Constants may be off 
