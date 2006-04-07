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

March 2006, RHG. Implementation of the Iterative Scaling algorithm.

April 2006, M. Spiekermann. Interface modifications and changes of the start value
initalization including a check which avoids doubly defined predicate selectivities. 
Documentation of the encoding scheme of known predicate selectivities.

*/

#include <math.h>
#include <stdio.h>
#include <iostream>
#include <set>

#include "entropy.h"

using namespace std;

typedef set<int> PredicateSet; 

/*
The function below checks if no predicate selecivity is doubly defined.

*/

bool registerPredCode(PredicateSet& ps, const int code) {

  PredicateSet::const_iterator sit;
  sit = ps.find(code);
  if ( sit == ps.end() )
  { 
    ps.insert(code);
    return true;
  }
  else
  {
    cerr << "WARNING: The selectivity for predicate " 
         << code << " is already defined! The value will " 
         << "be omitted!" << endl; 
    return false;
  }
} 


void maximize_entropy( const ProbabilityPairVec& marginalSels,
                       ProbabilityPairVec& jointSels,
                       ProbabilityPairVec& resultSels         )
{
  bool trace = true;
  bool traceIter = false;
  
  // A set of int needed to avoid doubly definitions of predicates 
  PredicateSet definedPreds;
  
  // number of predicates
  const int N = marginalSels.size();  
  
  // 2^N number of atoms
  const int NN = static_cast<const int>( pow(2.0,N*1.0) ); 

  // number of known selectivities.
  // The + 1 is needed since we will add predicate sum=1
  int k = N + jointSels.size() + 1;   

  unsigned i=0, j=0, j2=0, i2=0;

  int m=0;
  
  // binary encoding for each known selectivity	
  unsigned selindex[k];	
  double selvalue[k];	// value for each known selectivity
  double z[k];		// Lagrange factor z[j] = e**lambda_j
  
  // each entry is an index into the z vector, or -1
  int atomtable[NN][k];	

  double e = exp(1.0);
  double error=0.0, sum=0.0, prod=0.0, z_old=0.0;

  double atomsel[NN];
  double predsel[NN];

  if (trace) 
  {
    cout << "Input parameters:" << endl;
    //cout << "e = " << e << endl;
    cout << "N = " << N << endl;
    cout << "NN = " << NN << endl;
  }  

  // initialize known selectivities

/* 

Below an example configuration for three predicates p1, p2, and p3 is shown.
The ith predicate is encoded with the integer value $2^{i-1}$, e.g. $p1 = 001 =
2^0 = 1$. The array ~selindex~ stores the integer value encoding a marginal or
joint predicate and the array ~selvalue~ stores the known selectivity. A set bit
indicates that the predicate must hold and an unset bit indicates that the
predicate can be fullfilled or not.

----
    001: selindex[0] = 1;  selvalue[0] = 0.1;
    010: selindex[1] = 2;  selvalue[1] = 0.2;
    100: selindex[2] = 4;  selvalue[2] = 0.25;
    110: selindex[3] = 6;  selvalue[3] = 0.05;
    101: selindex[4] = 5;  selvalue[4] = 0.03;
    000: selindex[5] = 0;  selvalue[5] = 1.0;   
    //selindex[6] = 0;	selvalue[6] = 1.0;
----

Inside the optimizer a predicate ~maximze\_entropy/3~ is provided which
can be called with the values above by

----
    maximize_entropy([[1 0.1], [2, 0.2], [4, 0.25]], [[6, 0.05], [5, 0.03]], R).
----

*/
  
  // process marginal and joint selectivities
  const ProbabilityPairVec* args[2] = { &marginalSels, &jointSels };

  int pos = 0; 
  for (int i = 0; i<2; i++) 
  { 
    const ProbabilityPairVec& v = *(args[i]);
    ProbabilityPairVec::const_iterator itj = v.begin();
    while( itj != v.end() )
    {
     int code = itj->first; 
     bool ok = registerPredCode(definedPreds, code);
     
     if (ok) { 
       selindex[pos] = code; 
       selvalue[pos] = itj->second;
       pos++;
     }
     else {
      // decrement number of used selectivites
       k = k-1;
     }  
     itj++;
    }
  } 

   // Check if predicate 0 ( sum=1 ) is already included
  if (registerPredCode(definedPreds, 0))
  {
    cout << "INFO: Adding canonical predicate selectivity s[0]=1 !" << endl; 
    selindex[pos] = 0; 
    selvalue[pos] = 1;
    definedPreds.insert(0);
  } 
  else {
    // decrement number of used selectivites
    k = k - 1;
  }
  
  if (trace) 
  {
    cout << endl << "There are " << k << " known selectivities:" << endl;
    for (j = 0; j < k; j++) {
      cout << "s[" << selindex[j] << "] = " << selvalue[j] << endl;
    }
    //exit(1);
  } 
    
  // initialize z

  for (j = 0; j < k; j++) z[j] = 1;

  // initialize atomtable

  // for (i = 0; i < NN; i++) atomtable[i][0] = 0;

  for (j = 0; j < k; j++) {
    for (i = 0; i < NN; i++) {

      if ( selindex[j] == (selindex[j] & i) )	// atom i has all bits of
						// preds j
        atomtable[i][j] = j;
      else
	atomtable[i][j] = -1;
    }
  }  

  if (traceIter)
  { 
    for (i = 0; i < NN; i++)
    {
      printf("\ni = %d: ", i);
      for (j = 0; j < k; j++) 

        printf("%5d", atomtable[i][j]);

        //if (atomtable[i][j] < 0) printf("-");
        //else printf("+");
    }
    cout << endl;
  }

  // determine new z factors
  double epsilon = 0.000001;
  int iteration = 0;

  do
  { 
    iteration++;
    error = 0;
  
    for (j = 0; j < k; j++)	// for each equation
    {

      //printf("\nequation %d\n", j);

      z_old = z[j];
      sum = 0;
      for (i = 0; i < NN; i++)
      {
        if ( atomtable[i][j] >= 0 ) 	// atom occurs in this equation
        {
   	  prod = 1;
	
	  for (j2 = 0; j2 < k; j2++)
	    if ( (j2 != j) && (atomtable[i][j2] >= 0) )
	      prod *= z[atomtable[i][j2]];

	  sum += prod;
        }
      }
    
      z[j] = selvalue[j] * e / sum; 
      if (traceIter)
        printf("   z[%d] = %f", j, z[j]);

      error += (fabs(z[j] - z_old) / z_old); 
    }

    if (traceIter)
      printf("\n  Error = %f\n", error);
  }
  while ( error > epsilon );

  cout << "\nIteration stopped. Error = " << error << endl;
  cout << iteration << " iterations needed." << endl << endl;


  // Compute atom selectivities from z factors

 cout << "Computing atom selectivities ... " << endl;

  for (i = 0; i < NN; i++)
  {
    prod = 1/e;

    for (j = 0; j < k; j++)
      if ( atomtable[i][j] >= 0 ) prod *= z[atomtable[i][j]];

    atomsel[i] = prod;

    if (trace)
      cout << "i = " << i << ":" << atomsel[i] << endl;
  }

  
  // Compute predicate selectivities

  cout << "\nComputing predicate selectivities ..." << endl;


  for (i = 0; i < NN; i++)
  {
    sum = 0;
    for (i2 = 0; i2 < NN; i2++)
    {
      if ( i == (i & i2) ) sum += atomsel[i2];
    }

    predsel[i] = sum;
    resultSels.push_back( make_pair(i, sum) );

    if (trace)
      cout << "i = " << i << ":" << predsel[i] << endl;
  }

  cout.flush();
  return;
}

#ifdef STAND_ALONE


void appendProbabilityArgs( ProbabilityPairVec& v, 
                            const char* argv[], 
                            const int offset, 
                            const int n            )
{ 
  for( int i = 0; i < n*2; i+=2 )
  {
    int pos = offset+i;
    int code = atoi(argv[pos]);
    double prob = atof(argv[pos+1]);
      
    v.push_back( ProbabilityPair( code, prob ) );
  }
}  


int main( int argc, const char* argv[] )
{
  if( argc == 1 )
  {
    cout << endl
         << "Computes conditional probabilites by maximizing their entropy. " 
         << endl
         << "Unknown joint predictes are computed by the " 
         << "Interative Scaling algorithm." << endl << endl
         << "Usage: " << argv[0] << " n m1 m2 ...  j1 j2 ..." 
         << endl << endl
         << "        n: the number of predicates" << endl
         << "  m1...mn: is the known marginal probability of a predicate."
         << endl
         << "  j1...jk: is the joint probability of two ore more predicates;"
         << endl
         << "           Every probability is denoted as pair (int real). "
         << endl 
         << "           2^i determines the i-th predicate!" << endl
         << "Example:" << endl 
         << "  IterScale 1 0.1 2 0.2 4 0.25 6 0.05 5 0.03" << endl;

    exit(0);
  }

  
  const int offset = 2;
  int npreds = atoi(argv[1]);
  
  int ngiven = (argc - offset) / 2;
  cout << "ngiven: " << ngiven << endl;
  if ( (ngiven * 2) != (argc-offset) ) 
  {
    cerr << "ERROR: The number of arguments must be 2n+1!" << endl;
    exit(1+ngiven);
  }  

  ProbabilityPairVec marginalProb;
  ProbabilityPairVec jointProb;

  appendProbabilityArgs(marginalProb, argv, offset, npreds);
  appendProbabilityArgs(jointProb, argv, offset + npreds*2, ngiven - npreds);
  
  ProbabilityPairVec estimProb;
  maximize_entropy(marginalProb, jointProb, estimProb);

  ProbabilityPairVec::const_iterator it = estimProb.begin();
  cout << endl << "Returned values:" << endl;
  while ( it != estimProb.end() )
  {
    cout <<  "p[" << it->first << "] = " << it->second << endl;
    it++;
  } 
  
}

#endif

