/*
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

feedTC(0.4).

consumeTC(1.0).

filterTC(1.68).

productTC(1.26).

hashjoinTC(1.5, 0.65).

sortmergejoinTC(0.3, 0.73).

extendTC(1.5).

removeTC(0.6).

projectTC(0.71).

renameTC(0.1).










