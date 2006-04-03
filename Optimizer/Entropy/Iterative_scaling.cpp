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

March 2006, RHG. Implementation of the Iterative Scaling algorithm

April 2006, M. Spiekermann. Interface modifications and changes of the start value
initalization. Documentation of the encoding scheme of known predicate selectivities.

*/

#include <stdio.h>
#include <iostream>

#include "entropy.h"

#define BIT(j) (1<<(j))

using namespace std;

void maximize_entropy( const MarginalProbabilityVec& marginalSels,
                       JointProbabilityVec& jointSels,
                       JointProbabilityVec& resultSels             )
{
  bool trace = true;
  
  // number of predicates
  const int N = marginalSels.size();  
  
  // 2^N number of atoms
  const int NN = static_cast<const int>( pow(2.0,N*1.0) ); 

  // number of known selectivities
  int k = N + jointSels.size();   

  // to do: test if a pair [2^N,p] is included in jointSels
  // if xxx ...
  // else .....
  // Currently we assume that it sum=1 is not included
  jointSels.push_back( make_pair(0, 1.0) );
  k = k + 1; 
   
  
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
    cout << "e = " << e << endl;
    cout << "N = " << N << endl;
    cout << "NN = " << NN << endl;
  }  

  // initialize known selectivities

/* 

Below an example configuration for three predicates p1, p2, and p3 is shown.
The ith predicate is encoded in the ith bit, e.g. p1 = 100 = 4. The array
~selindex~ stores the integer value encoding a marginal or joint predicate and
the array ~selvalue~ stores the known selectivity. A set bit indicates that the
predicate must hold and an unset bit indicates that the predicate can be
fullfilled or not.

----
    100: selindex[0] = 4;  selvalue[0] = 0.1;
    010: selindex[1] = 2;  selvalue[1] = 0.2;
    001: selindex[2] = 1;  selvalue[2] = 0.25;
    110: selindex[3] = 6;  selvalue[3] = 0.05;
    101: selindex[4] = 5;  selvalue[4] = 0.03;
    000: selindex[5] = 0;  selvalue[5] = 1.0;   
  //selindex[6] = 0;	selvalue[6] = 1.0;
----

*/
  
  // marginal selectivities
  MarginalProbabilityVec::const_iterator it = marginalSels.begin();
  int exp1 = N-1;
  int pos = 0; 
  while( it != marginalSels.end() )
  {
   assert(pos < k);
   selindex[pos] = (int) pow(2.0, exp1*1.0); // 2^(N-1), 2^(N-2), ..., 2^0 
   selvalue[pos] = *it;
   it++;
   pos++;
   exp1--;
  } 
  assert(exp1 == -1);
  
  // joint selectivities
  JointProbabilityVec::const_iterator itj = jointSels.begin();
  while( itj != jointSels.end() )
  {
   assert( pos < k);
   selindex[ pos ] = itj->first; 
   selvalue[ pos ] = itj->second; 
   itj++;
   pos++;
  } 

  
  if (trace) 
  {
    cout << "Known selectivities:" << endl;
    for (j = 0; j < k; j++) {
      printf("s[%d] = %f\n", selindex[j], selvalue[j]);
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

  if (trace)
  { 
    for (i = 0; i < NN; i++)
    {
      printf("\ni = %d: ", i);
      for (j = 0; j < k; j++) 

        printf("%5d", atomtable[i][j]);

        //if (atomtable[i][j] < 0) printf("-");
        //else printf("+");
    }
  }

   // determine new z factors

  double epsilon = 0.000001;

  int iteration = 0;

  do
  { 
    iteration++;
    error = 0;
  
    printf("\n");
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
      if (trace)
        printf("   z[%d] = %f", j, z[j]);

      error += (fabs(z[j] - z_old) / z_old); 
    }

    if (trace)
      printf("  Error = %f", error);
  }
  while ( error > epsilon );

  printf("\nIteration stopped. Error = %f\n", error);
  printf("%d iterations needed\n\n", iteration);


  // Compute atom selectivities from z factors

  printf("Computing atom selectivities ... \n");

  for (i = 0; i < NN; i++)
  {
    prod = 1/e;

    for (j = 0; j < k; j++)
      if ( atomtable[i][j] >= 0 ) prod *= z[atomtable[i][j]];

    atomsel[i] = prod;

    if (trace)
      printf("i = %d: %f\n", i, atomsel[i]);
  }

  
  // Compute predicate selectivities

  printf("Computing predicate selectivities ... \n");


  for (i = 0; i < NN; i++)
  {
    sum = 0;
    for (i2 = 0; i2 < NN; i2++)
    {
      if ( i == (i & i2) ) sum += atomsel[i2];
    }

    predsel[i] = sum;

    if (trace)
      printf("i = %d: %f\n", i, predsel[i]);
  }

  return;
}

#ifdef STAND_ALONE

int main( int argc, const char* argv[] )
{
  if( argc == 1 )
  {
    cout << "Computes conditional probability using Maximum Entropy" 
         << endl << endl
         << "Usage: IterScale n p1 p2 p3 ... pn, cp1, cp2..." << endl
         << "where n is the number of predicates, (p1..pn) is the " 
         << "probability " << endl
         << "of each predicate and (cp1..cpn) is the joint probability " 
         << endl
         << "given as pairs cpi = int real. Example:" << endl << endl
         << "IterScale 3 0.1 0.2 0.25 6 0.05 5 0.03" << endl;

    exit(0);
  }

  int npred = atoi( argv[1] );
  int ngiven = argc - npred - 2;
  int nvars = 1 << npred;

  vector<double> marginalProb;
  vector<pair<int,double> > jointProb;
  vector<pair<int,double> > estimProb;

  for( int i = 0; i < npred; i++ )
    marginalProb.push_back( atof( argv[i+2] ) );

  int pos = 0;
  for( int i = 0; i < ngiven; i+=2 )
  {
    int k = npred+2+i;
    jointProb.push_back( pair<int,double>( atoi(argv[k]), atof( argv[k+1] ) ) );
    //cout << jointProb[pos].first << " " << jointProb[pos].second << endl;
    pos++;
  }

  maximize_entropy(marginalProb, jointProb, estimProb);
}

#endif

