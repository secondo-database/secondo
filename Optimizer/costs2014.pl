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

1 The Costs of Terms

----	cost(+Term, +Sel, +Pred, +Result, +Memory, -Size, -NAttrs, 
          -TupleSize, -Cost)
----

The cost of an executable ~Term~ representing a predicate ~Pred~ with
selectivity ~Sel~ is ~Cost~ and the size of the result (number of tuples) is ~Size~. The resulting tuples have ~NAttrs~ attributes and tuple size terms ~TupleSize~ = sizeTerm(MemoryFix, DiskCore, DiskLOB). The result will belong to node ~Result~. If the root operator of the ~Term~ uses a memory buffer, the amount of available memory is ~Memory~ (in MB).

This is evaluated recursively descending into the term. When the operator
realizing the predicate (e.g. ~filter~) is encountered, the selectivity ~Sel~ is
used to determine the size of the result. It is assumed that only a single operator of this kind occurs within the term.

1.1 Arguments

*/

cost(rel(Rel, _), _, _, _, _, Size, NAttrs, TupleSize, 0) :-
  card(Rel, Size),
  tupleSizeSplit(Rel, TupleSize),
  getRelAttrList(Rel, OrigAttrs, _),
  length(OrigAttrs, NAttrs).
  

cost(res(N), _, _, _, _, Size, NAttrs, TupleSize, 0) :-
  resultSize(N, Size),
  nodeTupleSize(N, TupleSize),
  nodeNAttrs(N, NAttrs).

/*
1.2 feed, feedproject

*/

feedC(0.0011, 0.0004). 	% PerTuple, PerAttr [ms]
	% original constants with machine factor 3.35 multiplied by
	% 1.33 / 3.35 = 0.4 to adapt to currrent machine

cost(feed(X), _, _, _, _, Size, NAttrs, TupleSize, Cost) :-
  cost(X, _, _, _, _, Size, NAttrs, TupleSize, Cost1),
  feedC(PerTuple, PerAttr),
  Cost is Cost1 + Size * (PerTuple + PerAttr * NAttrs).



feedprojectC(0.0033, 0.0003, 0.000006).  % PerTuple, PerAttr, PerByte [ms]
	% constants adapted to this machine, keeping relative sizes
	% the same

cost(feedproject(rel(Rel, _), Project), _, _, _, _, 
	Size, NAttrs, TupleSize, Cost) :-
  cost(rel(Rel, _), _, _, _, _, Size, _, _, Cost1),
  getRelAttrList(Rel, OrigAttrs, _),
  attributes(Project, AList),
  projectAttrList(OrigAttrs, AList, _, TupleSize),
  length(AList, NAttrs),
  TupleSize = sizeTerm(MemoryFix, _, _),
  feedprojectC(PerTuple, PerAttr, PerByte),
  Cost is Cost1 + Size * (PerTuple + PerAttr * NAttrs + PerByte * MemoryFix).


/*
1.3 filter

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
	Avg.TupleSize: 1416.3 = sizeTerm(416, 291.568, 1124.74)
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

We interpret these results as follows. The cheap predicate has a predicate cost of 0.101117 ms. As the sample relation has about 1700 tuples, this corresponds to a query time for the sample query of 170 ms. However we have a basic time for processing a query of about that magnitude, so that the real predicate evaluation time is much smaller. This is why the predicate can be evaluated on the complete relation in 3.57 seconds.

On the other hand, the expensive predicate has a cost of 1.36684 ms, corresponding to dividing the total time for the sample query of 2325 seconds by 1700. In this case the predicate cost is close to the real one which leads to an estimate of 735683 * 1.36684 ms = 1005 seconds, a bit higher than the observed time of 924 seconds.

We will proceed as follows. We set a threshold value of 0.2 ms, corresponding to processing 2000 tuples, the standard sample size, within 400 ms. Any value below this threshold is considered to be a cheap predicate with real costs 0 (more precisely, its cost is included in the tuple constant). A predicate with a higher cost is considered to be an expensive predicate, and its cost is added to the standard tuple cost.


*/

filterC(0.00111).	% PerTuple [ms]

cost(filter(X, _), Sel, Pred, _, _, Size, NAttrs, TupleSize, Cost) :-  
  cost(X, 1, Pred, _, _, SizeX, NAttrs, TupleSize, CostX),
  getPET(Pred, _, ExpPET),   % fetch stored predicate evaluation time
  filterC(PerTuple),
  ( (ExpPET =< 0.2) -> (PET is 0.0); PET is ExpPET),
  Size is SizeX * Sel,
  Cost is CostX + SizeX * (PerTuple + PET).


/*
1.4 itHashJoin

Iterative HashJoin reads the tuples from its first input stream into a hash table in memory. There are two cases:

  1 The tuples from the first stream fit completely into memory. Then the tuples from the second stream are read and each of them probes against the hash table to find join partners.

  2 Only an initial part of the tuples from the first stream fits into memory. Then the second stream is read, probing against the hashtable as in case 1. However, additionally the tuples from the second stream are written into a tuple file (that is, to disk).

    When this is finished, the hash table is emptied and further tuples from the first input stream are read into it, called the second partition. The tuples from the second argument are now read from the tuple file and probed against the second partition.

    This is repeated until all partitions have been processed.

For the cost formula we use the following notations:

  * $M$: the available memory, in MB

  * $P$: the number of partitions where $P = \lceil(p_1.Card \cdot p_1.Size) / (M * 1024 * 1024)\rceil$.

  * $p_1.Card, p_2.Card$: the cardinalities of the first / second input stream

  * $p_i.Size$: the tuple sizes of the first / second input stream. Tuple sizes are taken from the ~MemoryFix~ component of the size terms.

  * $p_i.noAttrs$: the numbers of attributes of the first / second argument stream

  * $pRes.Card$: the cardinality of the output.

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
	p_1.Card \cdot u + p2.Card \cdot v & \mbox{if $P$ = 1} 	\\
	p_1.Card \cdot u	& \mbox{otherwise}		\\
	+ p_2.Card \cdot p_2.Size \cdot w  &  			\\
	+ (P - 1) \cdot p_2.Card \cdot p_2.Size \cdot x &     	\\
	\end{array}
\right. \\
& & + pRes.Card \cdot y \cdot (p_1.noAttrs + p_2.noAttrs)
\end{array}
\]

These costs are reflected in the following cost function provided by module ~CostEstimation~.


*/


cost(itHashJoin(X, Y), Sel, Pred, ResultNode, Memory, Size, NAttrs, 
	TupleSize, Cost) :-
  cost(X, 1, Pred, _, _, SizeX, NAttrsX, sizeTerm(MemX, _, _), CostX), 
  cost(Y, 1, Pred, _, _, SizeY, NAttrsY, sizeTerm(MemY, _, _), CostY),
  getOpIndexes(itHashJoin, [algebra, all], _, AlId, OpId, _),
  getCosts([AlId, OpId, 0], 
    [SizeX, MemX, NAttrsX], [SizeY, MemY, NAttrsY], Sel, Memory, C),
  Size is SizeX * SizeY * Sel,
  nodeNAttrs(ResultNode, NAttrs),
  nodeTupleSize(ResultNode, TupleSize),
  Cost is CostX + CostY + C.
  
  
    
  
   




















