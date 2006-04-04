/*

2005, Geraldo Zimbrao. Initial Version
   
April 2006, M. Spiekermann. As a replacement for the OPT++ library we use now the
iterative scaling algorithm.

   
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

//#include <pair>
#include <vector>

typedef std::pair<int, double> JointSelPair;
typedef std::vector<JointSelPair> JointProbabilityVec; 
typedef std::vector<double> MarginalProbabilityVec;

void maximize_entropy( const MarginalProbabilityVec& marginalProbabilities,
                       JointProbabilityVec& jointProbabilities,
                       JointProbabilityVec& estimatedProbabilities );

#endif
