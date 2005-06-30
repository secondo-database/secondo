/*
1 The Header File of entropy

1.1 Remarks

Function to compute the conditional probabilities using Maximum Entropy Approach
usage: 

---- maximize_entropy( [p1 p2 p3 ... pn], [[1, cp1], [2, cp2] ...], result )
----

In this function is assumed the same codification of predicates using bits, as
done in POG construction - that is, to the predicate n, if the ith-bit is set to 1
then the ith-predicate is already evaluated.
Each $p_i$ is the probability of predicate $2^i$
Each pair $[n, cp]$ is the given probability cp of joint predicates n using the ith-bit
convention above.
Result is in the form of a list of pairs [n, cp] also.

---- Example: if we call
       maximize_entropy( [0.1, 0.3, 0.4], [[3,0.03],[5,0.04]], result )
     the result should be
       [[1,0.1],[2,0.3],[3,0.03],[4,0.4],[5,0.04],[6,0.12],[7,0.012]]
----

*/
#ifndef __ENTROPY_H__
#define __ENTROPY_H__
#include <vector>

void maximize_entropy( vector<double>& marginalProbabilities,
                       vector<pair<int,double> >& jointProbabilities,
                       vector<pair<int,double> >& estimatedProbabilities );

#endif
