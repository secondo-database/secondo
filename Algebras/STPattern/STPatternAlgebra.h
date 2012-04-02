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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the Spatiotemporal Pattern Algebra

June, 2009 Mahmoud Sakr

[TOC]

1 Overview

This header file essentially contains the necessary classes for evaluating the 
spatiotemporal pattern predicates (STP). The contents of the file are:
  * The class "STVector": which represents a vector temporal connector. Simple 
temporal connectors are vectors of length one. The "STVector" data type is used 
within the "stconstraint" that expresses a spatiotemporal constraints between 
to lifted predicates in the STPP.

  * The enumeration SimpleConnector: which assigns powers of two constants to 
the 26 possible simple temporal connectors.

  * The string array StrSimpleConnectors: which is used to translate  
the integer codes of the simple temporal connectors into their string 
representations and visa versa.  

  * The CSP class: which is our implementation for the constraint satisfaction 
problem solver. The class implements the algorithm "Solve Pattern" in the 
paper "Spatiotemporal Pattern Queries" 

2 Defines and includes

*/

#ifndef STPATTERNALGEBRA_H_
#define STPATTERNALGEBRA_H_

#ifndef ALGEBRA_H
#include "Algebra.h"
#endif

#ifndef NESTED_LIST_H
#include "NestedList.h"
#endif

#ifndef LISTUTILS_H
#include "ListUtils.h"
#endif

#ifndef QUERY_PROCESSOR_H
#include "QueryProcessor.h"
#endif

#ifndef STANDARDTYPES_H
#include "StandardTypes.h"
#endif

#ifndef CLASS_LOGMSG_H
#include "LogMsg.h"
#endif

#ifndef SECONDO_NLIST_H
#include "NList.h"
#endif

#ifndef _RELATION_ALGEBRA_H_
#include "RelationAlgebra.h"
#endif

#ifndef _TEMPORAL_ALGEBRA_H_
#include "TemporalAlgebra.h"
#endif

#include "TemporalReasoner.h"
#include <map>
using namespace datetime;
typedef DateTime Instant;
extern NestedList *nl;
extern QueryProcessor* qp;  


namespace STP {

/*
3 Global definitions

*/    
 

enum SimpleConnector {
  aabb=1,
  bbaa=2,
  aa_bb=4,
  bb_aa=8,
  abab=16,
  baba=32,
  baab=64,
  abba=128,
  a_bab=256,
  a_bba=512,
  baa_b=1024,
  aba_b=2048,
  a_ba_b=4096,
  a_abb=8192,
  a_a_bb=16384,
  ba_ab=32768,
  bb_a_a=65536,
  bba_a=131072,
  b_baa=262144,
  b_b_aa=524288,
  ab_ba=1048576,
  aa_b_b=2097152,
  aab_b=4194304,
  a_ab_b=8388608,
  a_a_b_b=16777216,
  b_ba_a=33554432
};
enum ClosureResult{inconsistent=0, consistent=1, notPA=2};



/*
4 Classes

4.1 Class STVector for Spatiotemporal Vector.

*/
class STVector
{
private:
/*
The Count helper function
Input: an integer representation for a vector temporal connector.
Process: the function counts the number of constituent simple temporal 
connectors.
Output: the count value

*/  
  int Count(int vec);
  
/*
The Str2Simple helper function.
Input: a string representation for a simple temporal connector. Note that since
the "." operator is commutative, one simple connector may have many string 
representations.
Process: uses the array StrSimpleConnectors for the translation.
Output: the integer representation of the connector.

*/ 
  inline int Str2Simple(string s);
/*
The Vector2List helper function.
Input: none. The function operates on the class member "v";
Process: construct the NestedList of string atoms corresponding to "v".
Output: the NestedList.

*/     
  inline ListExpr Vector2List();
public:

  int v;
  int count;
  STVector():v(0), count(0){}
  STVector(int vec):v(vec), count(Count(vec)){};
  STVector(STVector* vec):v(vec->v), count(vec->count){};
  ~STVector(){};
  static const string BasicType(){
    return "stvector";
  }
  static const bool checkType(const ListExpr list){
    return listutils::isSymbol(list, BasicType());
  }
/*
The Add function. Used to add a simple temporal connector to "this".
Input: a string representation for the simple connector.
Process: translates the string into integer and adds it to the STVector.
Output: none.

*/  
  bool Add(string simple);
  
/*
The Add function. Used to add a simple temporal connector to "this".
Input: an integer representation for the simple connector.
Process: adds it to the STVector.
Output: none.

*/  
  bool Add(int simple);
  
/*
The ApplyVector function. Checks whether "this" is fulfilled by two time
intervals.
Input: two time intervals. Note that Interval<CcReal> are used to speed up the 
processing but they represent Interval<Instant>
Process: iteratively checks the constituent simple connectors.
Output: fulfilled or not.

*/  

  bool ApplyVector(Interval<Instant>& p1, Interval<Instant>& p2);

/*
The ApplySimple function. Checks whether a simple temporal connector is 
fulfilled by two time intervals.
Input: two time intervals. Note that Interval<CcReal> are used to speed up the 
processing but they represent Interval<Instant>
Process: checks the simple connectors.
Output: fulfilled or not.

*/  

  bool ApplySimple(Interval<Instant>& p1, Interval<Instant>& p2,
      int simple);

/*
The Vector2PARelations function converts the IA relation represented in the
vector into a set of PA relations among the end points of the two intervals.
The function returns false if the conversion is not possible (i.e., the IA
relation does not belong to the continuous point algebra. That is, it cannot be
represented as point relations unless the != relation is used).

The output PA relations are reported in the rels argument. It has 10 places for
the relations aA ab aB ba bA bB Ab AB Ba BA Aa Bb in order. Each array elem has
the value 0 (relation not specified), or 1-7 (<, =, >, <=, >=, !=, ?). As
indicated above, if an elem in rels has the value 6 (!=), the function yields
false.

*/
  bool Vector2PARelations(int rels[12]);

/*
The Simple2PARelations function converts an IA simple relation (i.e., one term)
into a set of PA relations. It works similar to Vectore2PARelations, yet it is
able to convert one term only.

*/
  bool Simple2PARelations(int simple, int rels[12]);

  void Clear();
  
/*  
Secondo framework support functions

*/
  static Word In( const ListExpr typeInfo, const ListExpr instance,
              const int errorPos, ListExpr& errorInfo, bool& correct );
  static ListExpr Out( ListExpr typeInfo, Word value );
  static Word Create( const ListExpr typeInfo );
  static void Delete( const ListExpr typeInfo, Word& w );
  static bool Open( SmiRecord& valueRecord, size_t& offset, 
      const ListExpr typeInfo, Word& value );
  static bool Save( SmiRecord& valueRecord, size_t& offset, 
                    const ListExpr typeInfo, Word& value );
  static void Close( const ListExpr typeInfo, Word& w );
  static Word Clone( const ListExpr typeInfo, const Word& w );
  static int SizeOfObj();
  static ListExpr Property();
  static bool KindCheck( ListExpr type, ListExpr& errorInfo );
};

/*
4.2 Class CSP for Constraint Satisfaction Problem.

*/
class CSP  
{
private:
/*
The IntervalInstant2IntervalCcReal helper function. It converts the 
Interval<Instant> to Internal<CcReal> for more efficient processing

*/  
  void IntervalInstant2IntervalCcReal(const Interval<Instant>& in, 
      Interval<CcReal>& out);
  void IntervalCcReal2IntervalInstant(const Interval<CcReal>& in,
      Interval<Instant>& out);
/*
The MBool2Vec helper function. It constructs a vector from the true units in the
MBool argument.
 
*/  
  int MBool2Vec(const MBool* mb, vector<Interval<Instant> >& vec);

/* 
The Extend function as in the paper.  
    
*/  
  int Extend(int index, vector<Interval<Instant> >& domain );
    
/*
The IsSupported function.
Input: a partial assignment sa and the index of the newly evaluated variable.
Process: It checks whether the constraints that involve the new variable are
fulfilled.
Output: whether the partial assignment is consistent.
   
*/  
  bool IsSupported(vector<Interval<Instant> > sa, int index);

/*
The CheckConstraint helper function. It checks whether an STVector is fulfilled 
by two lifted predicates. 

*/

  bool CheckConstraint(Interval<Instant>& p1, Interval<Instant>& p2,
      vector<Supplier> constraint);
/*
The PickVariable function. It implements the picking methodology based on the
Connectivity rank as in the paper.

*/
  int PickVariable();

/*
The Temporal Reasoner. It implements temporal reasoning over Point Algebra to
make sure that the network is consistent, and to help restrict the
trajectories.

*/

  ClosureResult closureRes;
  
public:
/*
The list of supported assignments

*/  
  vector< vector<Interval<Instant> > > SA;
  vector<Supplier> Agenda;
  vector<bool> UsedAgendaVars;
  
/*
A helper data structure to translate the string aliases into their integer
poistion in the Agenda, SA and ConstraintGeraph.

*/
  map<string, int> VarAliasMap;
  vector< vector< vector<Supplier> > >ConstraintGraph;
/*
The total number of variables in the CSP.
 
*/  
  int count;
  
/*
The iterator is used in the "start" and "end" operators to iterate over the SA

*/
  int iterator;
  Interval<Instant> nullInterval;
  
/*
A list of the variable that have been consumed so far.

*/
  vector<int> assignedVars;
  
  
  CSP();
  ~CSP();
  
/* 
The AddVariable function.
Input: the alias of the lifted predicate and a pointer to the its node in the 
operator tree.
Process: adds the variable to the Agenda and resizes the ConstraintGraph.
Output: error code

*/ 
  int AddVariable(string alias, Supplier handle);

/* 
The AddConstraint function.
Input: the two aliases of the two lifted predicates and a pointer to the 
"stconstraint" node in the operator tree.
Process: adds the constraint to the ConstraintGraph.
Output: error code

*/ 
 
  int AddConstraint(string alias1, string alias2, Supplier handle);

/*
The ComputeClosure function. It converts the CSP into a PA network, and
computes the closure of the PA network. The return codes are: 0 for inconsistent
network, 1 for consistent network, and 2 if the IA not convertible into PA.

*/
//  int ComputeClosure();
  void SetConsistentPeriods(
      int varIndex, Periods* periodsArg, PointAlgebraReasoner* paReasoner);
  
/*
The Solve function. It implements the Solve Pattern algorithm in the paper.

*/
  
  bool Solve();
  bool Solve(Periods* periodsArg, PointAlgebraReasoner* paReasoner);
  
/*
The MoveNext function. It is used to iterate over the SA list. The function is
used by the "start" and "end" operators in the extended STPP.
  
*/
  bool MoveNext();
  
/*
The GetStart function. It is the impelementation of the "start" operator.
  
*/
  bool GetStart(string alias, Instant& result);
  
/*
The GetStart function. It is the impelementation of the "end" operator.
    
*/
  bool GetEnd(string alias, Instant& result);

/*
The GetSA function. Reads the SA entries.

*/
  bool GetSA(unsigned int saIndex, unsigned int varIndex, Periods& res);

/*
The AppendSolutionToTuple function. It appends one sa to the given tuple. The
function is used inside the stpatternextend/extendstream operators.

*/
  bool AppendSolutionToTuple(int saIndex, Tuple* oldTup, Tuple* resTup);
  bool AppendUnDefsToTuple(Tuple* oldTup, Tuple* resTup);
/*
The Print function. It is used for debug purposes.
 
*/  
  void Print();
/*
The Clear function. It is used to intialize the CSP. It is necessary to 
call it before processing every tuple in order to reintialize the CSP.

*/
  int Clear();
/*
The ResetTuple function. It is used to reset the CSP before evaluating it for
a new tuple. The Agenda, and the ConstraintGraph are kept. Other members
related to the evaluation are reset.

*/
  int ResetTuple();

/*
Reading/writing the closure computation result.

*/
  void SetClosureResult(ClosureResult _res);
  ClosureResult GetClosureResult();
};

} // namespace STP



#endif /* STPATTERNALGEBRA_H_ */
