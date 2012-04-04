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

//let mregs = TrainsFlocks feed  flocks feed 
//mflock2mregion[create_duration(0, 10000)] transformstream consume

#ifndef FLOCKALGEBRA_H_
#define FLOCKALGEBRA_H_

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "LogMsg.h"
#include "NList.h"
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"
#include "ListUtils.h"
#include "MovingRegionAlgebra.h"
#include "SpatialAlgebra.h"
#include <algorithm>
#include <vector>
#include <set>

#include "OctreeElements.h"
#include "OctreeDatParser.h"
#include "SkipTreeFunctions.h"

#include "RegionInterpolator.h"

using namespace RegionInterpol;
using namespace datetime;
typedef DateTime Instant;
extern NestedList *nl;
extern QueryProcessor* qp; 
extern AlgebraManager* am;


namespace FLOCK {
#define maxFlockSize 500
#define maxCoordsSize 64
#define _2Dim 1
#define _BruteForce 2
#define _Square 3
#define _Pruning 4
class Helpers
{
public:  
  static inline bool string2int(char* digit, int& result);
  static inline string ToString( int number );
  static inline string ToString( double number );
};

/* represantation of a flock */
class Flock: public Attribute {
 public:
   Flock(){} //{pointsCount= 0; coordsCount=0;};
   Flock(int numElem){pointsCount= numElem; defined=true; coordsCount=0;}
   Flock(double* coords, int cnt);
   Flock(bool def){pointsCount= 0; defined=true; coordsCount=0;}
   Flock(const Flock& arg);
   ~Flock();

   size_t HashValue() const; 
   void CopyFrom(const Attribute* right);
   int Compare( const Attribute* rhs ) const;
   bool operator==(const Flock& rhs) const;
   bool operator<(const Flock& rhs) const;
   int IntersectionCount(Flock* arg);
   bool IsSubset(const Flock& rhs) const;
   ostream& Print( ostream &os ) const;
   bool IsDefined() const;
   void SetDefined(const bool def);

   //members required for the Attribute interface 
   size_t Sizeof() const { return sizeof(Flock); }
   bool Adjacent(const Attribute*) const {return false;}
   Attribute* Clone() const ;
   
   static Word     In( const ListExpr typeInfo, const ListExpr instance,
                         const int errorPos, ListExpr& errorInfo,
                         bool& correct );
   static ListExpr Out( ListExpr typeInfo, Word value );
   static Word     Create( const ListExpr typeInfo );
   static void     Delete( const ListExpr typeInfo, Word& w );
   static void     Close( const ListExpr typeInfo, Word& w );
   static Word     Clone( const ListExpr typeInfo, const Word& w );
   static void*    Cast(void* addr);
   static bool     KindCheck( ListExpr type, ListExpr& errorInfo );
   static int      SizeOfObj();
   static ListExpr Property();
   // type name used in Secondo:
   inline static const string BasicType();
   static const bool checkType(const ListExpr type);
   
   int addPoint(OctreePoint* point);
   int addPointID(int point);
   int getPoints(int*& res);
   void printPoints();
   double* getCoordinates(){return this->coordinates;};
   void printCoordinates();
   int Intersection(Flock* arg, Flock* res);
   Points* Flock2Points(Instant& curTime, vector<int>* ids, 
       vector<MPoint*>*sourceMPoints);
   
   bool defined;
   //Total size of the flock. This is set before the points are
   //added to the flock
   double coordinates[maxCoordsSize];
   int coordsCount;
   int points[maxFlockSize];
   int pointsCount;
   //The number of points currently stored in the points array.
};


typedef ConstTemporalUnit<Flock> UFlock;

class MFlock : public  Mapping< UFlock, Flock > 
{
public:
  MFlock(){}
  MFlock(const int n):
    Mapping<UFlock, Flock>(n), finalized(true){}
  inline bool CanAdd(Flock* arg);
  void MFlockMergeAdd(UFlock& unit);
  MRegion* MFlock2MRegion(vector<int>* ids, vector<MPoint*>* sourceMPoints,
      Instant& samplingDuration);
  static bool KindCheck( ListExpr type, ListExpr& errorInfo );
  static ListExpr Property();
  
  int minSize;
  double maxRaduis;
  double tolerance;
  int minDuration;
  bool finalized;
};


/*
find all flocks in the given flockfile using the 2Dim exact query region

*/
::std::vector<Flock*>*
findFlocks2Dim(OctreeDatParser* myParser, 
    double radius, double tolerance, int flocksize);

::std::vector<Flock*>*
findFlocks2DimBruteforce(OctreeDatParser* myParser, double radius, 
    int flocksize);


/*
find all flocks in the given flockfile using the squared skiptree version

*/
::std::vector<Flock*>*
findFlocksSkiptreeSquare(OctreeDatParser* myParser, double radius, 
    double tolerance, int flocksize);

/*
find all flocks in the given flockfile using the squared skiptree version
and pruning the pointset by first looking to four timesteps

*/
::std::vector<Flock*>*
findFlocksSkiptreeSquareWithPruning(OctreeDatParser* myParser, 
    double radius, double tolerance, int flocksize);


} // namespace FLOCK





#endif /* FLOCKALGEBRA_H_ */
