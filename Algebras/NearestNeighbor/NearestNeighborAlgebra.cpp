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

//paragraph [1] title: [{\Large \bf ]   [}]
//characters    [2]    verbatim:   [\verb@]    [@]
//[ue] [\"{u}]
//[toc] [\tableofcontents]

""[2]

[1] NearestNeighbor Algebra

June 2008, A. Braese

[toc]

0 Overview

This example algebra provides a distance-scan for a point set
in a R-Tree. A new datatype is not given but there are some operators:

  1. ~distanceScanStart~, which build the needed data structure for the scan

  2. ~distanceScanNext~, which gives the next nearest point.

  3. ~distanceScan~, which gives a stream of all input tuples in the right order

1 Preliminaries

1.1 Includes

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "RTreeAlgebra.h"
#include "NearestNeighborAlgebra.h"
#include "TemporalAlgebra.h"

/*
The file "Algebra.h" is included, since the new algebra must be a subclass of
class Algebra. All of the data available in Secondo has a nested list
representation. Therefore, conversion functions have to be written for this
algebra, too, and "NestedList.h" is needed for this purpose. The result of an
operation is passed directly to the query processor. An instance of
"QueryProcessor" serves for this. Secondo provides some standard data types, 
e.g. "CcInt", "CcReal", "CcString", "CcBool", which is needed as the 
result type of the implemented operations. To use them "StandardTypes.h" 
needs to be included. The file "RtreeAlgebra.h" is included because 
some operators get a rtree as argument.
   
*/  

extern NestedList* nl;
extern QueryProcessor *qp;

/*
The variables above define some global references to unique system-wide
instances of the query processor and the nested list storage.

1.2 Auxiliaries

Within this algebra module implementation, we have to handle values of
four different types defined in namespace ~symbols~: ~INT~ and ~REAL~, ~BOOL~ 
and ~STRING~.  They are constant values of the C++-string class.

Moreover, for type mappings some auxiliary helper functions are defined in the
file "TypeMapUtils.h" which defines a namespace ~mappings~.

*/

#include "TypeMapUtils.h"
#include "Symbols.h"

using namespace symbols;
using namespace mappings;

#include <string>
using namespace std;

/*
The implementation of the algebra is embedded into
a namespace ~near~ in order to avoid name conflicts with other modules.
   
*/   

namespace near {

/*
5 Creating Operators

5.1 Type Mapping Functions

A type mapping function checks whether the correct argument types are supplied
for an operator; if so, it returns a list expression for the result type,
otherwise the symbol ~typeerror~. Again we use interface ~NList.h~ for
manipulating list expressions. For every operator is one type mapping 
function given.

The function distanceScanTypeMap is the type map for distancescan.
The function knearestTypeMap is the type map for the operator knearest


*/
ListExpr
distanceScanTypeMap( ListExpr args )
{
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  char* errmsg = "Incorrect input for operator windowintersects.";
  string rtreeDescriptionStr, relDescriptionStr, argstr;

  CHECK_COND(!nl->IsEmpty(args), errmsg);
  CHECK_COND(!nl->IsAtom(args), errmsg);
  CHECK_COND(nl->ListLength(args) == 4, errmsg);

  /* Split argument in four parts */
  ListExpr rtreeDescription = nl->First(args);
  ListExpr relDescription = nl->Second(args);
  ListExpr searchWindow = nl->Third(args);
  ListExpr quantity = nl->Fourth(args);

  // check for fourth argument type == int
  nl->WriteToString(argstr, quantity);
  CHECK_COND((nl->IsAtom(quantity)) 
    && (nl->AtomType(quantity) == SymbolType) 
    && (nl->SymbolValue(quantity) == "int"),
  "Operator distancescan expects a fourth argument of type integer (k or -1).\n"
  "Operator distancescan gets '" + argstr + "'.");

  /* Query window: find out type of key */
  CHECK_COND(nl->IsAtom(searchWindow) &&
    nl->AtomType(searchWindow) == SymbolType &&
    (algMgr->CheckKind("SPATIAL2D", searchWindow, errorInfo)||
     algMgr->CheckKind("SPATIAL3D", searchWindow, errorInfo)||
     algMgr->CheckKind("SPATIAL4D", searchWindow, errorInfo)||
     algMgr->CheckKind("SPATIAL8D", searchWindow, errorInfo)||
     nl->SymbolValue(searchWindow) == "rect"  ||
     nl->SymbolValue(searchWindow) == "rect3" ||
     nl->SymbolValue(searchWindow) == "rect4" ||
     nl->SymbolValue(searchWindow) == "rect8" ),
    "Operator distancescan expects that the search window\n"
    "is of TYPE rect, rect3, rect4, rect8 or "
    "of kind SPATIAL2D, SPATIAL3D, SPATIAL4D, SPATIAL8D.");

  /* handle rtree part of argument */
  nl->WriteToString (rtreeDescriptionStr, rtreeDescription);
  CHECK_COND(!nl->IsEmpty(rtreeDescription) &&
    !nl->IsAtom(rtreeDescription) &&
    nl->ListLength(rtreeDescription) == 4,
    "Operator distancescan expects a R-Tree with structure "
    "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a R-Tree list with structure '"
    +rtreeDescriptionStr+"'.");

  ListExpr rtreeSymbol = nl->First(rtreeDescription),
           rtreeTupleDescription = nl->Second(rtreeDescription),
           rtreeKeyType = nl->Third(rtreeDescription),
           rtreeTwoLayer = nl->Fourth(rtreeDescription);

  CHECK_COND(nl->IsAtom(rtreeKeyType) &&
    nl->AtomType(rtreeKeyType) == SymbolType &&
    (algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo)||
     algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo)||
     algMgr->CheckKind("SPATIAL4D", rtreeKeyType, errorInfo)||
     algMgr->CheckKind("SPATIAL8D", rtreeKeyType, errorInfo)||
     nl->IsEqual(rtreeKeyType, "rect")||
     nl->IsEqual(rtreeKeyType, "rect3")||
     nl->IsEqual(rtreeKeyType, "rect4")||
     nl->IsEqual(rtreeKeyType, "rect8")),
   "Operator distancescan expects a R-Tree with key type\n"
   "of kind SPATIAL2D, SPATIAL3D, SPATIAL4D, SPATIAL8D\n"
   "or rect, rect3, rect4, rect8.");

  /* handle rtree type constructor */
  CHECK_COND(nl->IsAtom(rtreeSymbol) &&
    nl->AtomType(rtreeSymbol) == SymbolType &&
    (nl->SymbolValue(rtreeSymbol) == "rtree"  ||
     nl->SymbolValue(rtreeSymbol) == "rtree3" ||
     nl->SymbolValue(rtreeSymbol) == "rtree4" ||
     nl->SymbolValue(rtreeSymbol) == "rtree8") ,
   "Operator distancescan expects a R-Tree \n"
   "of type rtree, rtree3, rtree4,  or rtree8.");

  CHECK_COND(!nl->IsEmpty(rtreeTupleDescription) &&
    !nl->IsAtom(rtreeTupleDescription) &&
    nl->ListLength(rtreeTupleDescription) == 2,
    "Operator windowintersects expects a R-Tree with structure "
    "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  ListExpr rtreeTupleSymbol = nl->First(rtreeTupleDescription);
  ListExpr rtreeAttrList = nl->Second(rtreeTupleDescription);

  CHECK_COND(nl->IsAtom(rtreeTupleSymbol) &&
    nl->AtomType(rtreeTupleSymbol) == SymbolType &&
    nl->SymbolValue(rtreeTupleSymbol) == "tuple" &&
    IsTupleDescription(rtreeAttrList),
    "Operator windowintersects expects a R-Tree with structure "
    "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))) attrtype "
    "bool)\nbut gets a first list with wrong tuple description in "
    "structure \n'"+rtreeDescriptionStr+"'.");

  CHECK_COND(nl->IsAtom(rtreeTwoLayer) &&
    nl->AtomType(rtreeTwoLayer) == BoolType,
   "Operator windowintersects expects a R-Tree with structure "
   "(rtree||rtree3||rtree4||rtree8 (tuple ((a1 t1)...(an tn))) attrtype "
   "bool)\nbut gets a first list with wrong tuple description in "
   "structure \n'"+rtreeDescriptionStr+"'.");

  /* handle rel part of argument */
  nl->WriteToString (relDescriptionStr, relDescription);
  CHECK_COND(!nl->IsEmpty(relDescription), errmsg);
  CHECK_COND(!nl->IsAtom(relDescription), errmsg);
  CHECK_COND(nl->ListLength(relDescription) == 2, errmsg);

  ListExpr relSymbol = nl->First(relDescription);;
  ListExpr tupleDescription = nl->Second(relDescription);

  CHECK_COND(nl->IsAtom(relSymbol) &&
    nl->AtomType(relSymbol) == SymbolType &&
    nl->SymbolValue(relSymbol) == "rel" &&
    !nl->IsEmpty(tupleDescription) &&
    !nl->IsAtom(tupleDescription) &&
    nl->ListLength(tupleDescription) == 2,
    "Operator distancescan expects a R-Tree with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) as relation description\n"
    "but gets a relation list with structure \n"
    "'"+relDescriptionStr+"'.");

  ListExpr tupleSymbol = nl->First(tupleDescription);
  ListExpr attrList = nl->Second(tupleDescription);

  CHECK_COND(nl->IsAtom(tupleSymbol) &&
    nl->AtomType(tupleSymbol) == SymbolType &&
    nl->SymbolValue(tupleSymbol) == "tuple" &&
    IsTupleDescription(attrList),
    "Operator distancescan expects a R-Tree with structure "
    "(rel (tuple ((a1 t1)...(an tn)))) as relation description\n"
    "but gets a relation list with structure \n"
    "'"+relDescriptionStr+"'.");

  /* check that rtree and rel have the same associated tuple type */
  CHECK_COND(nl->Equal(attrList, rtreeAttrList),
   "Operator distancescan: The tuple type of the R-tree\n"
   "differs from the tuple type of the relation.");

  string attrTypeRtree_str, attrTypeWindow_str;
  nl->WriteToString (attrTypeRtree_str, rtreeKeyType);
  nl->WriteToString (attrTypeWindow_str, searchWindow);

  CHECK_COND(
    ( algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo) &&
      nl->IsEqual(searchWindow, "rect") ) ||
    ( algMgr->CheckKind("SPATIAL2D", rtreeKeyType, errorInfo) &&
      algMgr->CheckKind("SPATIAL2D", searchWindow, errorInfo) ) ||
    ( nl->IsEqual(rtreeKeyType, "rect") &&
      nl->IsEqual(searchWindow, "rect") ) ||
    ( algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo) &&
      nl->IsEqual(searchWindow, "rect3") ) ||
    ( algMgr->CheckKind("SPATIAL3D", rtreeKeyType, errorInfo) &&
      algMgr->CheckKind("SPATIAL3D", searchWindow, errorInfo) ) ||
    ( nl->IsEqual(rtreeKeyType, "rect3") &&
      nl->IsEqual(searchWindow, "rect3") ) ||
    ( algMgr->CheckKind("SPATIAL4D", rtreeKeyType, errorInfo) &&
      nl->IsEqual(searchWindow, "rect4") ) ||
    ( algMgr->CheckKind("SPATIAL4D", rtreeKeyType, errorInfo) &&
      algMgr->CheckKind("SPATIAL4D", searchWindow, errorInfo) ) ||
    ( nl->IsEqual(rtreeKeyType, "rect4") &&
      nl->IsEqual(searchWindow, "rect4") ) ||
    ( algMgr->CheckKind("SPATIAL8D", rtreeKeyType, errorInfo) &&
      nl->IsEqual(searchWindow, "rect8") ) ||
    ( algMgr->CheckKind("SPATIAL8D", rtreeKeyType, errorInfo) &&
      algMgr->CheckKind("SPATIAL8D", searchWindow, errorInfo) ) ||
    ( nl->IsEqual(rtreeKeyType, "rect8") &&
      nl->IsEqual(searchWindow, "rect8") ),
    "Operator distancescan expects joining attributes of same "
    "dimension.\nBut gets "+attrTypeRtree_str+
    " as left type and "+attrTypeWindow_str+" as right type.\n");


    return
      nl->TwoElemList(
        nl->SymbolAtom("stream"),
        tupleDescription);
}


ListExpr
knearestTypeMap( ListExpr args )
{
  string argstr, argstr2;

  CHECK_COND(nl->ListLength(args) == 4,
    "Operator knearest expects a list of length four.");

  ListExpr first = nl->First(args),
           second = nl->Second(args),
           third = nl->Third(args),
           quantity = nl->Fourth(args);

  ListExpr tupleDescription = nl->Second(first);

  nl->WriteToString(argstr, first);
  CHECK_COND(nl->ListLength(first) == 2  &&
    (TypeOfRelAlgSymbol(nl->First(first)) == stream) &&
    (nl->ListLength(nl->Second(first)) == 2) &&
    (TypeOfRelAlgSymbol(nl->First(nl->Second(first))) == tuple) &&
    (nl->ListLength(nl->Second(first)) == 2) &&
    (IsTupleDescription(nl->Second(tupleDescription))),
    "Operator knearest expects as first argument a list with structure "
    "(stream (tuple ((a1 t1)...(an tn))))\n"
    "Operator knearest gets as first argument '" + argstr + "'." );

  nl->WriteToString(argstr, second);
  CHECK_COND(nl->IsAtom(second) &&
    nl->AtomType(second) == SymbolType,
    "Operator knearest expects as second argument an atom "
    "(an attribute name).\n"
    "Operator knearest gets '" + argstr + "'.\n");

  string attrName = nl->SymbolValue(second);
  ListExpr attrType;
  int j;
  if( !(j = FindAttribute(nl->Second(nl->Second(first)), 
                          attrName, attrType)) )
  {
    nl->WriteToString(argstr, nl->Second(nl->Second(first)));
    ErrorReporter::ReportError(
      "Operator knearest expects as secong argument an attribute name.\n"
      "Attribute name '" + attrName + 
      "' does not belong to the tuple of the first argument.\n"
      "Known Attribute(s): " + argstr + "\n");
    return nl->SymbolAtom("typeerror");
  }

  // check for third argument type == mpoint
  nl->WriteToString(argstr, quantity);
  CHECK_COND((nl->IsEqual( third, "mpoint" )),
  "Operator knearest expects a third argument of type point.\n"
  "Operator knearest gets '" + argstr + "'.");

  // check for fourth argument type == int
  nl->WriteToString(argstr, quantity);
  CHECK_COND((nl->IsAtom(quantity)) 
    && (nl->AtomType(quantity) == SymbolType) 
    && (nl->SymbolValue(quantity) == "int"),
  "Operator knearest expects a fourth argument of type integer (k or -1).\n"
  "Operator knearest gets '" + argstr + "'.");

  return
    nl->ThreeElemList(
    nl->SymbolAtom("APPEND"),
    nl->OneElemList(nl->IntAtom(j)),
    nl->TwoElemList(
      nl->SymbolAtom("stream"),
      tupleDescription));
}


/*
5.2 Selection Function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; this has already been checked by the type mapping function. 
A selection function is only called if the type mapping was successful. This
makes programming easier as one can rely on a correct structure of the list
~args~.

*/

int
distanceScanSelect( ListExpr args )
{
  AlgebraManager *algMgr = SecondoSystem::GetAlgebraManager();
  ListExpr searchWindow = nl->Third(args),
           errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  if (nl->SymbolValue(searchWindow) == "rect" ||
      algMgr->CheckKind("SPATIAL2D", searchWindow, errorInfo))
    return 0;
  else if (nl->SymbolValue(searchWindow) == "rect3" ||
           algMgr->CheckKind("SPATIAL3D", searchWindow, errorInfo))
    return 1;
  else if (nl->SymbolValue(searchWindow) == "rect4" ||
           algMgr->CheckKind("SPATIAL4D", searchWindow, errorInfo))
    return 2;
  else if (nl->SymbolValue(searchWindow) == "rect8" ||
           algMgr->CheckKind("SPATIAL4D", searchWindow, errorInfo))
    return 3;

  return -1; /* should not happen */
}  


/*
5.3 Value Mapping Functions

For any operator a value mapping function must be defined. It contains
the code which gives an operator its functionality

The struct DistanceScanLocalInfo is needet to save the data
from one to next function call

*/

template <unsigned dim, class LeafInfo>
struct DistanceScanLocalInfo
{
  Relation* relation;
  R_Tree<dim, TupleId>* rtree;
  BBox<dim> position;
  int quantity, noFound;
  bool scanFlag;
//  NNpriority_queue* pq;
};



/*
5.3.1 The ~distanceScan~ results a stream of all input tuples in the 
right order. It returns the k tuples with the lowest distance to a given
reference point. The are ordered by distance to the given reference point.
If the last parameter k has a value <= 0, all tuples of the input stream
will returned.

*/

template <unsigned dim>
int distanceScanFun (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  DistanceScanLocalInfo<dim, TupleId> *localInfo;

  switch (message)
  {
    case OPEN :
    {
      localInfo = new DistanceScanLocalInfo<dim, TupleId>;
      localInfo->rtree = (R_Tree<dim, TupleId>*)args[0].addr;
      localInfo->relation = (Relation*)args[1].addr;
      StandardSpatialAttribute<dim>* pos = 
          (StandardSpatialAttribute<dim> *)args[2].addr;
      localInfo->position = pos->BoundingBox();

      localInfo->quantity = ((CcInt*)args[3].addr)->GetIntval();
      localInfo->noFound = 0;
      localInfo->scanFlag = true;

      assert(localInfo->rtree != 0);
      assert(localInfo->relation != 0);

      localInfo->rtree->FirstDistanceScan(localInfo->position);

      local = SetWord(localInfo);
      return 0;
    }

    case REQUEST :
    {
      localInfo = (DistanceScanLocalInfo<dim, TupleId>*)local.addr;
      if ( !localInfo->scanFlag )
      {
        return CANCEL;
      }

      if ( localInfo->quantity > 0 && localInfo->noFound >=localInfo->quantity)
      {
        return CANCEL;
      }

      TupleId tid;
      if ( localInfo->rtree->NextDistanceScan( localInfo->position, tid ) )
      {
          Tuple *tuple = localInfo->relation->GetTuple(tid);
          result = SetWord(tuple);
          ++localInfo->noFound;
          return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      localInfo = (DistanceScanLocalInfo<dim, TupleId>*)local.addr;
      localInfo->rtree->LastDistanceScan();
      delete localInfo;
      return 0;
    }
  }
  return 0;

}

/*
5.3.2 The ~knearest~ operator results a stream of all input tuples which 
contains the k-nearest units to the given mpoint. The tuples are splitted 
into multiple tuples with disjoint units if necessary. The tuples in the 
result stream are not necessarily ordered by time or distance to the 
given mpoint 

The struct knearestLocalInfo is needet to save the data
from one to next function call

*/
enum EventType {E_LEFT, E_RIGHT, E_INTERSECT};
struct EventElem
{
  EventType type;
  Instant pointInTime; //x-axes, sortkey in the priority queue
  Tuple* tuple;
  Tuple* intersectTuple;
  const UPoint* up;
  MReal* distance;
  EventElem(EventType t, Instant i, Tuple* tu, const UPoint* upt,
    MReal* d) : type(t), pointInTime(i), tuple(tu), intersectTuple(NULL),
    up(upt), distance(d){}
  EventElem(EventType t, Instant i, Tuple* tu, Tuple* tu2) 
    : type(t), pointInTime(i), tuple(tu), intersectTuple(tu2),
    up(NULL), distance(NULL){}
  bool operator<( const EventElem& e ) const 
  {
    return e.pointInTime < pointInTime;
  }
};

class ActiveElem
{
public:
  //double distance;  
  Tuple* tuple;
  Instant start; //the start time where the element is needed
  bool lc;
  ActiveElem(Tuple* t, Instant s, bool l) : tuple(t), start(s), lc(l){}
};

double CalcDistance( const MReal *mr, Instant t);
class ActiveKey
{
public:
  static Instant currtime;
  MReal *distance;
    ActiveKey( ) : distance(0){}
    ActiveKey( MReal *dist) : distance(dist){}
    bool operator==( const ActiveKey& k ) const 
    {
      return CalcDistance(distance,currtime) 
        == CalcDistance(k.distance,currtime);
    }
    bool operator<( const ActiveKey& k ) const 
    {
      return CalcDistance(distance,currtime) 
        < CalcDistance(k.distance,currtime);
    }
};

Instant ActiveKey::currtime;
typedef multimap< ActiveKey, ActiveElem >::const_iterator CI;
typedef multimap< ActiveKey, ActiveElem >::iterator IT;

bool GetPosition( const UPoint* up, Instant t, Coord& x, Coord& y)
{
  //calculates the pos. at time t. x and y is the result
  Instant t0 = up->timeInterval.start;  
  Instant t1 = up->timeInterval.end; 
  if( t == t0 )
  {
    x = up->p0.GetX();
    y = up->p0.GetY();
    return true;
  }
  if( t == t1 )
  {
    x = up->p1.GetX();
    y = up->p1.GetY();
    return true;
  }
  if( t < t0 || t > t1 ){ return false;}
  double factor = (t - t0) / (t1 - t0);
  x = up->p0.GetX() + factor * (up->p1.GetX() - up->p0.GetX());
  y = up->p0.GetY() + factor * (up->p1.GetY() - up->p0.GetY());
  return true;
}

void GetDistance( const MPoint* mp, const UPoint* up, 
                   int &mpos, MReal *erg)
{
  //this function calculates the distance 
  //mpos ist the startposition in mp where the function start to look
  //This is a optimization cause the upoints are sorted
  //by time. A lower time cannot come
  const UPoint *upn;
  Instant time1 = up->timeInterval.start;
  Instant time2 = up->timeInterval.end;
  int noCo = mp->GetNoComponents();
  for (int ii=mpos; ii < noCo; ++ii)
  {
    mp->Get( ii, upn);
    if( time1 < upn->timeInterval.end 
      || (time1 == upn->timeInterval.end && upn->timeInterval.rc))
    { 
      mpos = ii;
      UReal firstu(true);
//      cout << "up start: " << up->timeInterval.start.ToString()
//        << ", end: " << up->timeInterval.end.ToString() << endl;
//      cout << "Mp start: " << upn->timeInterval.start.ToString()
//        << ", end: " << upn->timeInterval.end.ToString() << endl;
      Instant start = up->timeInterval.start < upn->timeInterval.start
        ? upn->timeInterval.start : up->timeInterval.start;
      Instant end = up->timeInterval.end < upn->timeInterval.end
        ? up->timeInterval.end : upn->timeInterval.end; 
      bool lc = up->timeInterval.lc; 
      if( lc && (start > up->timeInterval.start || (up->timeInterval.start
        == upn->timeInterval.start && !up->timeInterval.lc)))
      {
        lc = false;
      }
      bool rc = up->timeInterval.rc;
      if( rc && (end < up->timeInterval.end || (up->timeInterval.end
        == upn->timeInterval.end && !upn->timeInterval.rc)))
      {
        rc = false;
      }

      Interval<Instant> iv(start, end, lc, rc);
      Coord x1, y1, x2, y2;
      GetPosition( up, start, x1, y1);
      GetPosition( up, end, x2, y2);
//cout << "erste Distanz Intervall: " << start.ToString()
//  << ", " << end.ToString() << ", " << lc << ", " << rc << endl;
//cout << "PosU: " << x1 << ", " << y1 << ", " << x2 << ", " << y2;
      UPoint up1(iv, x1, y1, x2, y2);
      GetPosition( upn, start, x1, y1);
      GetPosition( upn, end, x2, y2);
//cout << " PosM: " << x1 << ", " << y1 << ", " << x2 << ", " 
//  << y2 << endl;
      UPoint upMp(iv, x1, y1, x2, y2);
//cout << "nach upMp" << endl;
      up1.Distance(upMp, firstu);
      cout << "nach up1.Distance ";
      firstu.Print(std::cout);
      cout << endl;
      erg->Add( firstu );
//cout << "Distanz a: " << firstu.a << ", b: " << firstu.b << ", c: "
//<< firstu.c << endl;
//double d = CalcDistance(erg,start);
//cout << ", dist: " << d << endl;
      int jj = ii;
      while( ++jj < noCo && (time2 > end
        || (time2 == end && !rc && up->timeInterval.rc)))
      {
        mp->Get( jj, upn);
        UReal nextu(true);
        start = end;
        lc = true;
        end = up->timeInterval.end < upn->timeInterval.end
        ? up->timeInterval.end : upn->timeInterval.end;
        rc = up->timeInterval.rc;
        if( rc && (end < up->timeInterval.end || (up->timeInterval.end
          == upn->timeInterval.end && !upn->timeInterval.rc)))
        {
          rc = false;
        }
        Interval<Instant> iv(start, end, lc, rc);
        Coord x1, y1, x2, y2;
        GetPosition( up, start, x1, y1);
        GetPosition( up, end, x2, y2);
        UPoint up1(iv, x1, y1, x2, y2);
        GetPosition( upn, start, x1, y1);
        GetPosition( upn, end, x2, y2);
        UPoint upMp(iv, x1, y1, x2, y2);
        up1.Distance(upMp, nextu);
        erg->Add( nextu );
     }
      break;
    }
  }
}

double CalcDistance( const MReal *mr, Instant t)
{
//  cout << "Zeit in CalcDistance: " << t.ToString() << endl;
  int noCo = mr->GetNoComponents();
  const UReal *ur;
  for (int ii=0; ii < noCo; ++ii)
  {
    mr->Get( ii, ur);
//    cout << "timeintervall: " << ur->timeInterval.start.ToString()
//      << " bis " << ur->timeInterval.end.ToString() << endl;
    if( t < ur->timeInterval.end 
      || (t == ur->timeInterval.end && ur->timeInterval.rc)
      || (t == ur->timeInterval.end && ii+1 == noCo))
    { 
      double time = (t - ur->timeInterval.start).ToDouble();
      double erg = ur->a * time * time + ur->b * time + ur->c;
      return ( ur->r) ? sqrt(erg) : erg;
    }
  }
  return -1;
}

bool intersects( MReal* m1, MReal* m2, Instant &start, Instant& result )
{
  //returns true if m1 intersects m2. The first intersection time is the result
  //The intersection after the time start is calculated
  int noCo1 = m1->GetNoComponents();
  int ii = 0;
  const UReal* u1;
  if( !noCo1 ){ return false; }
  m1->Get( ii, u1);
  while( (start > u1->timeInterval.end || (start == u1->timeInterval.end
    && !u1->timeInterval.rc)) && ++ii < noCo1 )
  {
    m1->Get( ii, u1);
  }
  int noCo2 = m2->GetNoComponents();
  const UReal* u2;
  if( !noCo2 ){ return false; }
  int jj = 0;
  m2->Get( jj, u2);
  while( (start > u2->timeInterval.end || (start == u2->timeInterval.end
    && !u2->timeInterval.rc)) && ++jj < noCo2 )
  {
    m2->Get( jj, u2);
  }
  cout << "In intersects" << endl;
  if( start > u1->timeInterval.end || start > u2->timeInterval.end 
    || start < u1->timeInterval.start || start < u2->timeInterval.start)
  {
    return false;
  }
  //u1, u2 are the ureals with the time start to begin with the calculation
  //formula: t = (-b +- sqrt(b*b - 4ac)) / 2a where a,b,c are a1-a2 ...
  //if b*b - 4ac > 0 the equation has a result
  bool hasResult = false;
  double a, b, c, d, r1, r2;
  while( !hasResult  && ii < noCo1 && jj < noCo2)
  {
    a = u1->a - u2->a;
    b = u1->b - u2->b;
    c = u1->c - u2->c;
    d = b * b - 4 * a * c;
    if( d >= 0 )
    {
      d = sqrt(d);
      r1 = (-b - d) / (2 * a);
      r2 = (-b + d) / (2 * a);
      if( r1 > r2 ){ swap( r1, r2 );}
      result.ReadFrom(r1);
      if( result > start && result <= u1->timeInterval.end 
        && result <= u2->timeInterval.end)
      {
        hasResult = true;
      }
      else
      {
        result.ReadFrom(r2);
        if( result > start && result <= u1->timeInterval.end 
          && result <= u2->timeInterval.end)
        {
          hasResult = true;
        }
      }
    }
    if( !hasResult )
    {
      if( u1->timeInterval.end < u2->timeInterval.end )
      {
        ++ii;
      }
      else if( u2->timeInterval.end < u1->timeInterval.end )
      {
        ++jj;
      }
      else
      {
        //both ends are the same
        ++ii;
        ++jj;
      }
      if( ii < noCo1 && jj < noCo2)
      {
        m1->Get( ii, u1);
        m2->Get( jj, u2);
      }
    }
  }
  return hasResult;
}

IT checkFirstK( IT checkIt, int k, multimap< ActiveKey, ActiveElem >& m, 
               int& outpos)
{
  //checks if "it" is one of the first k, then give out the k+1 iterator
  //else give out m->end()
  outpos = 0;
  IT it = m.begin();
  while( outpos < k && it != checkIt ) {++outpos; ++it;}
  cout << "checkFirstK Pos: " << outpos << ", k: " << k << endl;
  if( outpos < k )
  {
    int pos = outpos;
    //something in the first k has changed
    while( pos < k && it != m.end())
    { 
      ++pos;
      ++it;
    }
    return it;
  }
  else
  {
    return m.end();
  }
}

Tuple* changeTupleUnit( Tuple *tuple, int attrNr, Instant start, 
  Instant end, bool lc, bool rc)
{
  Tuple* res = new Tuple( tuple->GetTupleType() );
  for( int ii = 0; ii < tuple->GetNoAttributes(); ++ii)
  {
    if( ii != attrNr )
    {
      res->CopyAttribute( ii, tuple, ii);
    }
    else
    {
      const UPoint* upointAttr 
          = (const UPoint*)tuple->GetAttribute(attrNr);
      Coord x1, y1, x2, y2;
      GetPosition( upointAttr, start, x1, y1);
      GetPosition( upointAttr, end, x2, y2);
      Interval<Instant> iv( start, end, lc, rc);
      UPoint* up = new UPoint( iv, x1, y1, x2, y2);
      res->PutAttribute( ii, up);
    }
  }
  return res;
}


struct KnearestLocalInfo
{
  //To use the plane sweep algrithmus a priority queue for the events and
  //a map to save the active segments is needed. 
  int k, max;
  bool scanFlag;
  Instant startTime, endTime;
  multimap< ActiveKey, ActiveElem > activeLine;
  priority_queue<EventElem> eventQueue;
};


int knearestFun (Word* args, Word& result, int message, 
             Word& local, Supplier s)
{
  // The argument vector contains the following values:
  // args[0] = stream of tuples, 
  //  the attribute given in args[1] has to be a unit
  //  the operator expects that the tuples are sorted by the time of the units
  // args[1] = attribute name
  // args[2] = mpoint
  // args[3] = int k, how many nearest are searched
  // args[4] = int j, attributenumber

  KnearestLocalInfo *localInfo;
  const MPoint *mp = (MPoint*)args[2].addr;

  switch (message)
  {
    case OPEN :
    {
      localInfo = new KnearestLocalInfo;
      localInfo->max = 50;
      int mpos = 0;
      localInfo->k = ((CcInt*)args[3].addr)->GetIntval();
      localInfo->scanFlag = true;
      int j = ((CcInt*)args[4].addr)->GetIntval() - 1;
      local = SetWord(localInfo);
      if (mp->IsEmpty())
      {
        return 0;
      }
      const UPoint *up1, *up2;
      mp->Get( 0, up1);
      mp->Get( mp->GetNoComponents() - 1, up2);
      localInfo->startTime = up1->timeInterval.start;
      localInfo->endTime = up2->timeInterval.end;
      cout << "MPoint starttime: " << localInfo->startTime.ToString() << endl;
      cout << "MPoint endtime: " << localInfo->endTime.ToString() << endl;

      qp->Open(args[0].addr);
      Word currentTupleWord; 
      qp->Request( args[0].addr, currentTupleWord );
      while( qp->Received( args[0].addr ) )
      {
        //fill eventqueue with start- and endpoints
        Tuple* currentTuple = (Tuple*)currentTupleWord.addr;
        const UPoint* upointAttr 
            = (const UPoint*)currentTuple->GetAttribute(j);
        Instant t1 = upointAttr->timeInterval.start;  
        Instant t2 = upointAttr->timeInterval.end; 
        if( t1 > localInfo->endTime 
          || (t1 == localInfo->endTime && !upointAttr->timeInterval.lc) 
          || t2 < localInfo->startTime
          || (t2 == localInfo->startTime && !upointAttr->timeInterval.rc))
        {
          currentTuple->DeleteIfAllowed();
        }
        else
        {
          cout << "upoint starttime: " << t1.ToString() << endl;
          MReal *mr = new MReal(0);
          GetDistance( mp, upointAttr, mpos, mr);
          localInfo->eventQueue.push( EventElem(E_LEFT, t1, 
             currentTuple, upointAttr, mr) );
          cout << "upoint endtime: " << t2.ToString() << endl;
          localInfo->eventQueue.push( EventElem(E_RIGHT, t2, 
           currentTuple, upointAttr, mr) );
        }
        qp->Request( args[0].addr, currentTupleWord );
      }
      return 0;
    }

    case REQUEST :
    {
     int attrNr = ((CcInt*)args[4].addr)->GetIntval() - 1;
     localInfo = (KnearestLocalInfo*)local.addr;
      if ( !localInfo->scanFlag )
      {
        return CANCEL;
      }

      if ( !localInfo->k)
      {
        return CANCEL;
      }

      cout << "in request, max: " << localInfo->max << endl;
      while ( localInfo->max-- && !localInfo->eventQueue.empty() )
      {
        EventElem elem = localInfo->eventQueue.top();
        localInfo->eventQueue.pop();
        ActiveKey::currtime = elem.pointInTime;
        double d = CalcDistance(elem.distance, elem.pointInTime);
        cout << "time: " << elem.pointInTime.ToString() << 
          ", distance: " << d << endl;
        switch ( elem.type ){
          case E_LEFT:
          {
            cout << "found left element" << endl;
            Instant start = elem.pointInTime >= localInfo->startTime 
              ? elem.pointInTime : localInfo->startTime;
            bool lc = elem.pointInTime >= localInfo->startTime 
              ? elem.up->timeInterval.lc : false;
            pair<ActiveKey, ActiveElem> pIns(ActiveKey(elem.distance),
              ActiveElem(elem.tuple, start, lc));
            cout << "hinter pIns def" << endl;
            IT itNew = localInfo->activeLine.insert(pIns);
            cout << "Elements: " << localInfo->activeLine.size() << endl;
            CI itNeighbor = itNew;
            Instant intersectTime( start.GetType() );
            if( ++itNeighbor != localInfo->activeLine.end() &&
              intersects( elem.distance, itNeighbor->first.distance, 
              start, intersectTime ))
            {
              localInfo->eventQueue.push( EventElem(E_INTERSECT, intersectTime, 
                elem.tuple, itNeighbor->second.tuple) );
            }
            itNeighbor = itNew;
            if( itNeighbor != localInfo->activeLine.begin() &&
              intersects( elem.distance, (--itNeighbor)->first.distance, 
              start, intersectTime ))
            {
              localInfo->eventQueue.push( EventElem(E_INTERSECT, intersectTime, 
                elem.tuple, itNeighbor->second.tuple) );
            }
            if( localInfo->activeLine.size() > (unsigned)localInfo->k )
            {
              //check if the first k has changed. Then give out the k+1.
              //It was the k. element before the insert
              int pos;
              IT it = checkFirstK( itNew, localInfo->k, 
                localInfo->activeLine, pos);
              if( it != localInfo->activeLine.end() )
              {
                //something in the first k has changed
                //now give out the element "it", but only until this time. 
                //Clone the tuple and change the unit-attribut to the 
                //new time interval
                Tuple* cloneTuple = changeTupleUnit( it->second.tuple, 
                  attrNr, it->second.start, elem.pointInTime, 
                  it->second.lc, true);
                it->second.start = elem.pointInTime;
                it->second.lc = false;
                result = SetWord(cloneTuple);
                return YIELD;
              }
            }
            cout << "hinter insert map" << endl;
            break;
          }
          case E_RIGHT:
          {
            //a unit ends. It has to be removed from the map
            cout << "found right element" << endl;
            IT itDel = localInfo->activeLine.find(ActiveKey(elem.distance));
            //look for the right tuple 
            //(more elements can have the same distance)
            while( itDel != localInfo->activeLine.end() 
              && itDel->second.tuple != elem.tuple )
            { 
              ++itDel;
            }
           Tuple* cloneTuple = NULL;
           if( itDel != localInfo->activeLine.end())
           {
              //check if this tuple is one of the first k, then give out this
              //and change the start of the k+1 to this time
              int pos;
              IT it = checkFirstK( itDel, localInfo->k, 
                localInfo->activeLine, pos);
              if( pos < localInfo->k )
              {
                cloneTuple = changeTupleUnit( itDel->second.tuple, attrNr, 
                  itDel->second.start, elem.pointInTime, itDel->second.lc, 
                  elem.up->timeInterval.rc);
              }
              if( it != localInfo->activeLine.end() )
              {
                it->second.start = elem.pointInTime;
                it->second.lc = false;
              }
              //now calculate the intersection of the neighbors
              if( itDel != localInfo->activeLine.begin() )
              {
                cout << "Calculate intersection after delete" << endl;
                CI itNeighbor1 = itDel;
                ++itNeighbor1;
                if( itNeighbor1 != localInfo->activeLine.end() )
                {
                  CI itNeighbor2 = itDel;
                  --itNeighbor2;
                  Instant intersectTime( elem.pointInTime.GetType() );
                  if( intersects( itNeighbor1->first.distance, 
                    itNeighbor2->first.distance, 
                    elem.pointInTime, intersectTime ) )
                  {
                    localInfo->eventQueue.push( EventElem(E_INTERSECT, 
                      intersectTime, itNeighbor1->second.tuple, 
                      itNeighbor2->second.tuple) );
                  }
                }
                cout << "after calculation intersection after delete" << endl;
              }

              elem.distance->Destroy();
              delete elem.distance;
              elem.tuple->DeleteIfAllowed();
              localInfo->activeLine.erase( itDel );
            }
            else
            {
              //the program should never be here. This is a program error!
              assert(false);
            }
            if( cloneTuple )
            {
              result = SetWord(cloneTuple);
              return YIELD;
            }
            break;
          }
          case E_INTERSECT:
            cout << "found intersection" << endl;
            //  MReal* xx;
            //  xx = *it->first.distance;
            //  *it->first.distance = *itDel->first.distance;
            //  *itDel->first.distance = xx;
            break;
        }
      }
      return CANCEL;
    }

    case CLOSE :
    {
      qp->Close(args[0].addr);
      localInfo = (KnearestLocalInfo*)local.addr;
      delete localInfo;
      return 0;
    }
  }

  return 0;
}

/*
5.4 Definition of value mapping vectors

*/
ValueMapping distanceScanMap [] = { distanceScanFun<2>,
                                    distanceScanFun<3>,
                                    distanceScanFun<4>,
                                    distanceScanFun<8>};


/*
5.5 Specification of operators

*/

const string distanceScanSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>rtree(tuple ((x1 t1)...(xn tn))"
      " ti) x rel(tuple ((x1 t1)...(xn tn))) x T x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))\n"
      "For T = ti and ti in SPATIAL<d>D, for"
      " d in {2, 3, 4, 8}</text--->"
      "<text>_ _ distancescan [ _, _ ]</text--->"
      "<text>Uses the given rtree to find all k tuples"
      " in the given relation in order of their distance from the "
      " position T. The nearest tuple first.</text--->"
      "<text>query kinos_geoData Kinos distancescan "
      "[[const point value (10539.0 14412.0)], 5] consume; "
      "where kinos_geoData "
      "is e.g. created with 'let kinos_geoData = "
      "Kinos creatertree [geoData]'</text--->"
      ") )";

const string knearestSpec  =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
      "( <text>stream(tuple ((x1 t1)...(xn tn))"
      " ti) x xi x mpoint x k ->"
      " (stream (tuple ((x1 t1)...(xn tn))))"
      "</text--->"
      "<text>_ _ knearest [ _, _ ]</text--->"
      "<text>The operator results a stream of all input tuples "
      "which contains the k-nearest units to the given mpoint. "
      "The tuples are splitted into multiple tuples with disjoint "
      "units if necessary. The tuples in the result stream are "
      "not necessarily ordered by time or distance to the given "
      "mpoint. The operator expects that the input stream with "
      "the tuples are sorted by the time of the units</text--->"
      "<text>query query UnitTrains feed head[20] UTrip knearest "
      "[train1, 2] consume;</text--->"
      ") )";



/*
5.6 Definition of operators

*/
Operator distancescan (
         "distancescan",        // name
         distanceScanSpec,      // specification
         4,                         //number of overloaded functions
         distanceScanMap,  // value mapping
         distanceScanSelect, // trivial selection function
         distanceScanTypeMap    // type mapping
);

Operator knearest (
         "knearest",        // name
         knearestSpec,      // specification
         knearestFun,      // value mapping
         Operator::SimpleSelect, // trivial selection function
         knearestTypeMap    // type mapping
);



/*
6 Implementation of the Algebra Class

*/

class NearestNeighborAlgebra : public Algebra
{
 public:
  NearestNeighborAlgebra() : Algebra()
  {


/*   
6.1 Registration of Operators

*/
    AddOperator( &distancescan );
    AddOperator( &knearest );
  }
  ~NearestNeighborAlgebra() {};
};

/*
7 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime (if it is built as a dynamic link library). The name
of the initialization function defines the name of the algebra module. By
convention it must start with "Initialize<AlgebraName>". 

To link the algebra together with the system you must create an 
entry in the file "makefile.algebra" and to define an algebra ID in the
file "Algebras/Management/AlgebraList.i.cfg".

*/

} // end of namespace ~near~

extern "C"
Algebra*
InitializeNearestNeighborAlgebra( NestedList* nlRef, 
                               QueryProcessor* qpRef )
{
  // The C++ scope-operator :: must be used to qualify the full name 
  return new near::NearestNeighborAlgebra; 
}

/*
8 Examples and Tests

The file "NearestNeighbor.examples" contains for every operator one example.
This allows one to verify that the examples are running and to provide a 
coarse regression test for all algebra modules. The command "Selftest <file>" 
will execute the examples. Without any arguments, the examples for all active
algebras are executed. This helps to detect side effects, if you have touched
central parts of Secondo or existing types and operators. 

In order to setup more comprehensive automated test procedures one can write a
test specification for the ~TestRunner~ application. You will find the file
"example.test" in directory "bin" and others in the directory "Tests/Testspecs".
There is also one for this algebra. 

Accurate testing is often treated as an unpopular daunting task. But it is
absolutely inevitable if you want to provide a reliable algebra module. 

Try to write tests covering every signature of your operators and consider
special cases, as undefined arguments, illegal argument values and critical
argument value combinations, etc.


*/

