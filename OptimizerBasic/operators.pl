/*
//[=>] [$=>$]
//[toc] [\tableofcontents]

[toc]


1 Constants for Operators

[File ~operators.pl~]

The constants for the operators have been determined by experiments. For
those experiments, time measurement code was added to some relational operators 
(hashjoin, product, and sortmergejoin). This permitted determining how much
CPU time is needed for sorting tuples, how much CPU time is needed for hashing etc.
If one wants to experiment by oneself, the time measurement support in the
relational algebra can be easily switched on by uncommenting a line in the
relational algebra (and thus defining a symbol ~MEASURE\_OPERATORS~).

The experiments considered only a rather small set of queries. Although the
constants below are quite accurate e.g. for examples 14 to
21 in the optimizer, they may be inappropriate for other queries.

*/


leftrangeTC(10).

extendTC(1.5).

removeTC(0.6).

projectTC(0.71).





/*

1 Determining Cost Constants

Ralf's machine at university, Ubuntu 14.04, scale factor 1.0, end of 2014, 2048 MB global memory.

1.1 hashjoin

We consider relations with a few million small tuples. We assume the hashtable fits into memory.

Hashjoin first in the build phase processes the second argument stream and puts each tuple into a hash table. Then, in the probe phase, checks each tuple from the first input stream against the table. Time required also depends on the number of result tuples constructed.

----
cost(hashjoin(X, Y, _, _, 999997), Sel, S, C) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, 1, SizeY, CostY),
  hashjoinTC(A, B, D),
  S is SizeX * SizeY * Sel,
  C is CostX + CostY +		% producing the arguments
    A * SizeY +			% A - time [microsecond] per build
    B * SizeX +			% B - time per probe
    D * S.			% C - time per result tuple
				% table fits in memory assumed
----

Create large test relations:

----    
let plz100Even = plz feed thousand feed filter[.No <=100] product 
	  extend[R: randint(1000000)] filter[(.R mod 2) = 0] consume

let plz100Odd = plz feed thousand feed filter[.No <=100] product 
	  extend[R: randint(1000000)] filter[(.R mod 2) = 1] consume
----

Cardinalities: plz100Even 2063237, plz100Odd 2064214

Determine time for build:

----
query plz100Even feed count

4.63 seconds, 4.55

query plz100Even feed {a} plz100Even feed {b} hashjoin[R_a, R_b, 999997] 
  head[1] count

11.21, 11.06 seconds
----

Time for build phase: 11 - 4.5 seconds = 6.5 seconds needed for 2063237 tuples. Hence time per tuple is 6500000 / 2063237 = 3.15 microseconds.

Determine time for probe:

----
query plz100Even feed {a} plz100Odd feed {b} hashjoin[R_a, R_b, 999997] 
  count

Result: 0
17.3199, 17.3145 seconds
----

Time for probe phase: 

  * Scanning first argument: 4.5 seconds

  * Scanning second argument: 4.5 seconds

  * Build phase: 6.5 seconds

  * Remaining: 17.31 - 15.5 = 1.81 seconds

  * per tuple: 1810000 / 2064214 = 0.876 microseconds

Determine time for producing result tuples:

----
query plz100Even feed {a} plz100Even feed {b} hashjoin[R_a, R_b, 999997] 
  count

Result: 10589309
33.78, 33.67 seconds
----

Subtract time for previous join which did not have any result tuples. Remaining time is 33.7 - 17.3 seconds = 16.4 seconds. Per tuple 16400000 / 10589309 = 1.548 microseconds.

Enter in file operators.pl:

*/
hashjoinTC(3.15, 0.876, 1.55).

/*
1.2 feed

Cardinalities: plz100Even 2063237, plz100Odd 2064214

----
query plz100Even feed count

4.63 seconds, 4.55
----

Time per tuple is 4500000 / 2063237 = 2.181

*/
feedTC(2.181).

/*
1.3 consume

----
let x = plz100Even feed consume

42.64, 45.72, 45.02 seconds
----

We subtract 4.5 seconds: 45 - 4.5 = 40.5. Time per tuple is 40500000 / 2063237 = 19.63

*/
consumeTC(19.63).

/*
1.4 filter

This holds only for cheap predicates.

----
query plz100Even feed filter[.Ort = "Hamburg"] count

Result 41030
9.62, 9.396, 9.400 seconds
----

Time for filter is 9.4 - 4.5 = 4.9 seconds. Time per tuple is 4900000 / 2063237 = 2.37 microseconds.

*/
filterTC(2.37).

/*
1.5 product

Product reads the second argument stream into a buffer. It then scans the first argument stream and connects each tuple with all tuples in the buffer. Again we assume the buffer fits into memory. The cost function is:

----
cost(product(X, Y), _, S, C) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, 1, SizeY, CostY),
  productTC(A, B),
  S is SizeX * SizeY,
  C is CostX + CostY + SizeY * A + S * B.
----

----
query plz100Even feed head[1] {a} plz100Even feed {b} product head[1] count

Result 1
8.32, 7.90, 7.86, 8.53, 8.25

query plz100Even feed  {a} plz100Even feed head[1] {b} product head[1] count

Result 1
0.08, 0.12
----

Indeed it is the second argument which is put into the buffer.

Time for filling the buffer is 8.1 - 4.5 = 3.6 seconds. Time per tuple is 3600000 / 2063237 = 1.744 microseconds.

----
query plz100Even feed head[10] {a} plz100Even feed {b} product count

Result 20632370
29.28, 30.33 seconds
----

Time for producing result tuples is 30 - 8.1 = 21.9 seconds. Time per tuple is 21900000 / 20632370 = 1.06 microseconds.

*/
productTC(1.75, 1.06).

/*
1.6 sortmergejoin

Sorts both arguments streams, then joins in a parallel scan. Cost depends on number of results. It has been observed that the cost of sorting is practically linear, i.e., does not show a logarithmic factor as expected in theory.


The cost formula is:

----
cost(sortmergejoin(X, Y, _, _), Sel, S, C) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, 1, SizeY, CostY),
  sortmergejoinTC(A, B, D),
  S is SizeX * SizeY * Sel,
  C is CostX + CostY + 			% producing the arguments
    A * SizeX +				% sorting the first argument
    A * SizeY + 			% sorting the second argument
    B * (SizeX + SizeY) +		% parallel scan of sorted relations
    D * S.                           	% producing result tuples
----

----
query plz100Even feed head[X] sortby[R] count

X =  400000: 7.86, 6.07, 5.73, 5.76 	->  5.85 |  6.5 = 1 * 6.5
X =  800000: 12.28, 12.21		-> 12.25 | 13.0 = 2 * 6.5
X = 1200000: 19.10, 19.08		-> 19.10 | 19.5 = 3 * 6.5
X = 1600000: 26.05, 25.98		-> 26.00 | 	= 4 * 6.5
X = 2000000: 25.987, 33.38, 33.79	-> 33.60 | 32.5	= 5 * 6.5
----

This shows the roughly linear development of the cost for sorting, at least in this range of sizes.


1.7 Change of Computer

New computer: Ubuntu 14.04, Ralf at home, end of 2014.

plz100Even and plz100Odd recreated:

plz100Even: 2062619

plz100Odd: 2064277


----
query plz100Even feed head[2000000] sortby[R] count

29.99 sec, 29.63, 29.32 at 512 MB memory

29.81, 29.37, 29.40 at 2048 MB memory => 29.5 seconds
----

Leads to a machine factor for this machine of 33.5 / 29.5 = 1.1355 [=>] 1.14


----
query plz100Even feed count

Result: 2062619
4.09 sec, 3.97, 4.25 => 4.1 seconds
----

1.8 rename

----
query plz100Even feed {p1} count

Result: 2062619
5.23, 4.89, 5.04 => 5.0
----

Rename 5.0 - 4.9 = 0.9, per tuple 900000 / 2062619 = 0.43

Machine factor: 0.43 * 1.14 = 0.49

*/
renameTC(0.49).

/*
1.9 sortmergejoin

First step: just the sorting.

----
query plz100Even feed {p1} plz100Even feed {p2} sortmergejoin[R_p1, R_p2] head[1] count

Result: 1
54.10, 54.09 seconds
----


  * 10 seconds for feed .. count

  * 44.1 seconds for sorting the two arguments

  * 22.05 seconds for sorting one argument

  * is 22050000 / 2062619 = 10.69 microseconds per tuple

The second query adds the cost of the merge phase, but does not produce any results.

----
query plz100Even feed {p1} plz100Odd feed {p2} sortmergejoin[R_p1, R_p2] count

Result: 0
63.98, 62.49 seconds => 63.1 seconds
----

Cost for merge 63.1 - 54.1 = 9.0 seconds. Per tuple 9000000 / (2062619 + 2064277) = 2.18 microseconds.

The third query adds the cost of producing result tuples.

----
query plz100Even feed {p1} plz100Even feed {p2} sortmergejoin[R_p1, R_p2] count

Result 10575947
79.80, 78.86 seconds => 79.40 seconds
----

Cost for results is 79.4 - 63.1 = 16.3 seconds. Per result tuple 16300000 / 10575947 = 1.54 microseconds.

Applying machine factors: 

----
10.69 * 1.14 = 12.18
2.18 * 1.14 = 2.48
1.54 * 1.14 = 1.75
----

*/
sortmergejoinTC(12.18, 2.48, 1.75).


/* 
1.10 loopjoin

The cost for loopjoin is linear in the size of the first argument.

----
cost(loopjoin(X, Y), Sel, S, Cost) :-
  cost(X, 1, SizeX, CostX),
  cost(Y, Sel, SizeY, CostY),
  S is SizeX * SizeY,
  loopjoinTC(A),
  Cost is CostX +		% producing the first argument
    SizeX * A + 		% base cost for loopjoin 
    SizeX * CostY.		% sum of query costs
----

Create an empty relation: 

----
let Empty = ten feed filter[.No > 100] consume
----

----
query plz100Even feed {p1} loopjoin[Empty feed] count

Result: 0
14.3, 14.36, 14.90, 14.44 seconds => 14.4 seconds
----

We consider this as the base cost for loopjoin. 

Cost for feed .. count is 5.0 seconds (see rename). Remaining cost is 9.4 seconds. Per tuple 9400000 / 2062619 = 4.55 microseconds.

Machine factor: 4.55 * 1.14 = 5.187

*/
loopjoinTC(5.19).

/*
1.11 exactmatch

----
let plz100Even_R = plz100Even createbtree[R]

let plz100Odd_R = plz100Odd createbtree[R]

let plz100Odd200000 = plz100Odd feed head[200000] consume

let plz100Odd200000_R = plz100Odd200000 createbtree[R]

let plz100Odd20000 = plz100Odd feed head[20000] consume

let plz100Odd20000_R = plz100Odd20000 createbtree[R]

let plz100Odd2000 = plz100Odd feed head[2000] consume

let plz100Odd2000_R = plz100Odd2000 createbtree[R]



query plz100Even feed {p1} loopjoin[plz100Odd_R plz100Odd 
exactmatch[.R_p1] {p2}] count

Result: 0
4:17 min (256.82 seconds)

query plz100Even feed {p1} loopjoin[plz100Odd200000_R plz100Odd200000 
exactmatch[.R_p1] {p2}] count

Result 0
2:34 min (154 sec)

query plz100Even feed {p1} loopjoin[plz100Odd20000_R plz100Odd20000 
exactmatch[.R_p1] {p2}] count

Result 0
2:13 min (132 seconds)

query plz100Even feed {p1} loopjoin[plz100Odd2000_R plz100Odd2000 
exactmatch[.R_p1] {p2}] count

Result 0
1:21 min (80.74 seconds)
----


Curve fitting results in a running time for all queries in the loopjoin of 80 + 59 * ($\log_{10} N - 3.301$) seconds where N is the size of the relation indexed in the B-tree.

Per query (and subtracting the base cost) this is ((80000000 - 14400000) / 2062619) + (59000000 / 2062619) * ($\log_{10} N - 3.301$) = 31.80 + 28.60 * ($\log_{10} N - 3.301$) microseconds.


The following two queries differ only in producing result tuples:

----
query plz100Even feed head[500000] {p1} loopjoin[plz100Odd_R plz100Odd 
exactmatch[.R_p1] {p2}] count

Result 0
78.24, 40.98, 41.21 => 41 seconds


query plz100Even feed head[500000] {p1} loopjoin[plz100Even_R plz100Even 
exactmatch[.R_p1] {p2}] count

Result 2563645
180, 86, 85 seconds
----

One can observe that there is a big difference between cold and warm buffer states.

The time difference in warm state is 85 - 41 seconds = 44 seconds. Per tuple 44000000 / 2563645 = 17.16 microseconds.

Hence the cost for the exactmatch is

----
cost(exactmatch(_, Rel, _), Sel, Size, Cost) :-
  cost(Rel, 1, RelSize, _),
  exactmatchTC(A, B, C, D),
  Size is Sel * RelSize,
  Cost is A + B * (log10(RelSize) - C) +	% query cost
    Sel * RelSize * D.				% size of result
----

Adapting to machine factor:

----
31.8 * 1.14 = 36.25
28.6 * 1.14 = 32.6
17.16 * 1.14 = 19.56
----

*/

exactmatchTC(36.25, 32.6, 3.3, 19.56).



















