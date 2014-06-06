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

----	cost(+Term, +Sel, +Pred, +Res, +Mem, -Card, -NAttrs, 
          -TSize, -Cost)
----

The cost of an executable ~Term~ representing a predicate ~Pred~ with
selectivity ~Sel~ is ~Cost~ and the cardinality of the result (number of tuples) is ~Card~. The resulting tuples have ~NAttrs~ attributes and tuple size terms ~TSize~ = sizeTerm(MemoryFix, DiskCore, DiskLOB). The result will belong to node ~Res~. If the root operator of the ~Term~ uses a memory buffer, the amount of available memory is ~Mem~ (in MB).

This is evaluated recursively descending into the term. When the operator
realizing the predicate (e.g. ~filter~) is encountered, the selectivity ~Sel~ is
used to determine the size of the result. Usually only a single operator of this kind occurs within the term. However, with the filter and refine technique (used e.g. for spatial join), selectivity needs to be split into bounding-box selectivity and remaining selectivity.


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

/*
1.2 feed, feedproject

Cost constant definitions at the end of the file.

*/

% feedC(0.0011, 0.0004). 	% PerTuple, PerAttr [ms]
	% original constants with machine factor 3.35 multiplied by
	% 1.33 / 3.35 = 0.4 to adapt to currrent machine

cost(feed(X), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost1),
  feedC(PerTuple, PerAttr),
  Cost is Cost1 + Card * (PerTuple + PerAttr * NAttrs).



% feedprojectC(0.0033, 0.0003, 0.000006).  
	% PerTuple, PerAttr, PerByte [ms]
	% constants adapted to this machine, keeping relative
	% sizes the same

cost(feedproject(rel(Rel, _), Project), _, _, _, _, 
	Card, NAttrs, TSize, Cost) :-
  cost(rel(Rel, _), _, _, _, _, Card, _, _, Cost1),
  getRelAttrList(Rel, OrigAttrs, _),
  attributes(Project, AList),
  projectAttrList(OrigAttrs, AList, _, TSize),
  length(AList, NAttrs),
  TSize = sizeTerm(MemoryFix, _, _),
  feedprojectC(PerTuple, PerAttr, PerByte),
  Cost is Cost1 + Card * 
    (PerTuple + PerAttr * NAttrs + PerByte * MemoryFix).



/*
1.3 filter

Filter evaluates a predicate on each tuple of its input stream, hence the cost is linear in the size of the input stream. On top of the basic cost for a cheap predicate, there may be the cost for evaluating an expensive predicate.

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

We interpret these results as follows. The cheap predicate has a predicate cost of 0.101117 ms. As the sample relation has about 1700 tuples, this corresponds to a query time for the sample query of 170 ms. However we have a basic time for processing a query of about that magnitude, so that the real predicate evaluation time is much smaller. This is why the predicate can be evaluated on the complete relation in 3.57 seconds.

On the other hand, the expensive predicate has a cost of 1.36684 ms, corresponding to dividing the total time for the sample query of 2325 seconds by 1700. In this case the predicate cost is close to the real one which leads to an estimate of 735683 * 1.36684 ms = 1005 seconds, a bit higher than the observed time of 924 seconds.

We will proceed as follows. We set a threshold value of 0.2 ms, corresponding to processing 2000 tuples, the standard sample size, within 400 ms. Any value below this threshold is considered to be a cheap predicate with real costs 0 (more precisely, its cost is included in the tuple constant). A predicate with a higher cost is considered to be an expensive predicate, and its cost is added to the standard tuple cost.


*/

% filterC(0.00111).	% PerTuple [ms]

cost(filter(X, _), Sel, Pred, Res, Mem, Card, 
	NAttrs, TSize, Cost) :- 
  not(prefilter(X)), 	% this is the standard case
  !,
  cost(X, 1.0, Pred, Res, Mem, Card1, NAttrs, TSize, Cost1),
  getPET(Pred, _, ExpPET),   % fetch stored predicate evaluation time
  filterC(PerTuple),
  ( (ExpPET =< 0.2) -> (PET is 0.0); PET is ExpPET),
  Card is Card1 * Sel,
  Cost is Cost1 + Card1 * (PerTuple + PET).


cost(filter(X, _), Sel, Pred, Res, Mem, Card, 
	NAttrs, TSize, Cost) :- 
  prefilter(X), 	% filter and refinement case,
  simplePred(Pred, P),	% hence lookup bbox selectivity
  databaseName(DB),
  storedBBoxSel(DB, P, Sel2),
	write('passed selectivity is '), write(Sel), nl,
	write('filter step called with selectivity '), write(Sel2), nl,
  cost(X, Sel2, Pred, Res, Mem, Card1, NAttrs, TSize, Cost1),
	write('returned filter step cardinality is '), write(Card1), nl, 
  getPET(Pred, _, ExpPET),   % fetch stored predicate evaluation time
  filterC(PerTuple),
  ( (ExpPET =< 0.2) -> (PET is 0.0); PET is ExpPET),
  Card is Card1 * (Sel / Sel2),
	write('returned total cardinality is '), write(Card), nl,
  Cost is Cost1 + Card1 * (PerTuple + PET).




/*
1.4 rename

Cost is linear in the number of tuples, does not depend on the number of attributes or tuple size as just one pointer for the tuple is passed to the next operator.

The query (database nrw again)

----	query Roads feed count
----

yields 13.22, 3.22, 3.24, 3.20 seconds; average of last three is 3.22.

The query

----	query Roads feed {r} count
----

has running times 3.65, 3.58, 3.23 seconds, average 3.49. Hence the cost of rename per tuple is (3.49 - 3.22) / 735683 = 0.000000367 seconds = 0.000367 ms.


*/

% renameC(0.000367). 	% PerTuple

cost(rename(X, _), Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost) :-
  cost(X, Sel, Pred, Res, Mem, Card, NAttrs, TSize, Cost1),
  renameC(PerTuple),
  Cost is Cost1 + Card * PerTuple.
  
  

/*
1.5 itHashJoin

Iterative HashJoin reads the tuples from its first input stream into a hash table in memory. There are two cases:

  1 The tuples from the first stream fit completely into memory. Then the tuples from the second stream are read and each of them probes against the hash table to find join partners.

  2 Only an initial part of the tuples from the first stream fits into memory. Then the second stream is read, probing against the hashtable as in case 1. However, additionally the tuples from the second stream are written into a tuple file (that is, to disk).

    When this is finished, the hash table is emptied and further tuples from the first input stream are read into it, called the second partition. The tuples from the second argument are now read from the tuple file and probed against the second partition.

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

% attrC(0.00016).	% Cost per attribute in result tuple. in ms.
% itHashJoinC(0.003, 0.49, 0.00016, 0.000047) % see above

cost(itHashJoin(Arg1, Arg2, _, _), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(Arg1, 1, Pred, Res, Mem, Card1, _, sizeTerm(Size1, _, _), Cost1), 
  cost(Arg2, 1, Pred, Res, Mem, Card2, _, sizeTerm(Size2, _, _), Cost2),
  Card is Card1 * Card2 * Sel,
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  P is 1 + floor((Card1 * Size1) / (Mem * 1024 * 1024)), % # of partitions
  itHashJoinC(U, V, W, X),
  ( P = 1 
    -> C1 is Card1 * U + Card2 * V
    ;  C1 is Card1 * U 
         + Card2 * Size2 * W
         + (P - 1) * Card2 * Size2 * X
  ),
  attrC(PerAttr), 			% Cost depending on result size,
  C2 is Card * NAttrs * PerAttr,	% the same for all joins
  Cost is Cost1 + Cost2 + C1 + C2.
  

/*
1.5 itSpatialJoin

The task is to join tuples from two input streams that have overlapping bounding boxes for some attribute.

This operator works in the same way as ~itHashJoin~. The only difference is that instead of a hash table, a main memory R-tree is used. Probes search with a rectangle on this R-tree. We can therefore use the same cost function, only the constants will be different.

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


% itSpatialJoinC(0.059, 0.125, 0.006, 0.001) % see above

cost(itSpatialJoin(Arg1, Arg2, _, _), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(Arg1, 1, Pred, Res, Mem, Card1, _, sizeTerm(Size1, _, _), Cost1), 
  cost(Arg2, 1, Pred, Res, Mem, Card2, _, sizeTerm(Size2, _, _), Cost2),
  Card is Card1 * Card2 * Sel,
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  P is 1 + floor((Card1 * Size1) / (Mem * 1024 * 1024)), % # of partitions
  itSpatialJoinC(U, V, W, X),
  ( P = 1 
    -> C1 is Card1 * U + Card2 * V
    ;  C1 is Card1 * U 
         + Card2 * Size2 * W
         + (P - 1) * Card2 * Size2 * X
  ),
  attrC(PerAttr), 			% Cost depending on result size,
  C2 is Card * NAttrs * PerAttr,	% the same for all joins
  Cost is Cost1 + Cost2 + C1 + C2.
  


/*
1.7 symmjoin

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

1.7.1 Determining Constants

We use database ~opt~.

----	let plzEven = plz feed filter[(.PLZ mod 2) = 0] consume

	let plzOdd = plz feed filter[(.PLZ mod 2) = 1] consume
----

The two relations ~plzEven~ and ~plzOdd~ have cardinalities 19773 and 21494, respectively. The query 

----	query plzEven feed {p1} plzOdd feed {p2} 
	itHashJoin[PLZ_p1, PLZ_p2] count
----

has result 0 and runs in 0.12 seconds.

----	query plzEven feed head[A] {p1} 
	plzOdd feed head[B] {p2} 
	symmjoin[.PLZ_p1 = ..PLZ_p2] {0.00000001, 0.01} count
----

for all parameters A and B has result 0 and has running times:

B = 1000:

  * A = 1000: 3.04 sec

  * A = 2000: 6.05 sec

  * A = 3000: 9.07 sec

  * A = 4000: 12.05 sec

B = 2000:

  * A = 1000: 6.05 sec

  * A = 2000: 12.07 sec

  * A = 3000: 18.12 sec

  * A = 4000: 24.19 sec

One can observe the product cost very well. Clearly the cost is 3 secs per million pairs processed, hence the constant ~u~ is $u = 3000 / 1000000$ = 0.003 ms.

To determine the cost for an expensive predicate, we use the database berlintest.

----	query Trains feed {t1} Trains feed {t2} 
	symmjoin[.Id_t1 = ..Id_t2] count
----

The result is 562, the number of trains, and the running times are 0.99, 0.96, 0.97 secs. The prediction using the constant above is 562 * 562 * 0.003 = 947.532 ms which is close.

----	query Trains feed head[200] {t1} Trains feed head[B] {t2} 
	symmjoin[sometimes(distance(.Trip_t1, ..Trip_t2) < 50.0)] count
----

  * B =  50: 10.67 sec

  * B = 100: 16.08

  * B = 150: 23.58

  * B = 200: 36.93

The cost per predicate evaluation is about 8 secs per 10000 pairs, that is, 0.8 ms.

For this predicate, the optimizer determines in the sample query a cost of 0.76 ms. For a cheap predicate like comparing two numbers it is 0.0035. We use a threshold of 0.01 ms to distinguish an expensive predicate.

*/
% symmjoinC(0.003).	% Cost per tuple pair for cheap preds in ms

cost(symmjoin(Arg1, Arg2, _), Sel, Pred, Res, Mem, 
	Card, NAttrs, TSize, Cost) :-
  cost(Arg1, 1, Pred, Res, Mem, Card1, _, _, Cost1), 
  cost(Arg2, 1, Pred, Res, Mem, Card2, _, _, Cost2),
  Card is Card1 * Card2 * Sel,
  nodeNAttrs(Res, NAttrs),
  nodeTupleSize(Res, TSize),
  symmjoinC(U),
  getPET(Pred, _, ExpPET),   % fetch stored predicate evaluation time
  ( (ExpPET =< 0.01) -> (PET is 0.0); PET is ExpPET),
  C1 is Card1 * Card2 * (U + PET),
  attrC(PerAttr), 			% Cost dep. on result size,
  C2 is Card * NAttrs * PerAttr,	% the same for all joins
  Cost is Cost1 + Cost2 + C1 + C2.



/*
2 Cost Constants

2.1 Defined here

*/

feedC(0.0011, 0.0004). 	% PerTuple, PerAttr [ms]
	% original constants with machine factor 3.35 multiplied by
	% 1.33 / 3.35 = 0.4 to adapt to currrent machine

feedprojectC(0.0033, 0.0003, 0.000006). 
	% PerTuple, PerAttr, PerByte [ms]
	% constants adapted to this machine, keeping relative sizes
	% the same

filterC(0.00111).	% PerTuple [ms]

renameC(0.000367). 	% PerTuple

itHashJoinC(0.003, 0.49, 0.00016, 0.000047).
	% u, v, w, x constants for itHashJoin, see above

itSpatialJoinC(0.059, 0.125, 0.006, 0.001).

symmjoinC(0.003).

attrC(0.00016).	% PerAttr [ms], cost per attribute in result tuple












/*

2.2 Looking up constants determined in kernel system

These are retrieved from the kernel system and the stored so that each constant is retrieved only once.

*/

	/* does not yet work

:- dynamic costConstant/3.

costConstant(Algebra, ConstName, Const) :-
  costConst(Algebra, ConstName, Const).

costConstant(Algebra, ConstName, Const) :-
  secondo('query

	*/

  

/*
3 Auxiliary Predicates

----	prefilter(+X)
----  

is fulfilled if ~X~ is an operation like loopjoin or spatialjoin so that the expression ~filter(X)~ represents a filter and refinement strategy.

*/

prefilter(itSpatialJoin(_, _, _, _)).
prefilter(spatialjoin(_, _, _, _)).
prefilter(loopjoin(_, _)).






    
  
   




















