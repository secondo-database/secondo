/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

2012, May- Simone Jandt

1 Includes

*/

#include <assert.h>
#include <utility>
#include "AlgebraTypes.h"
#include "ListUtils.h"
#include "SecondoSystem.h"
#include "Symbols.h"
#include "LogMsg.h"
#include "SpatialAlgebra.h"
#include "Direction.h"
#include "JNetwork.h"
#include "RouteLocation.h"
#include "JRouteInterval.h"
#include "MJPoint.h"
#include "JUnit.h"
#include "JRITree.h"
#include "JNetUtil.h"
#include "RTreeAlgebra.h"

extern NestedList* nl;

using namespace jnetwork;

/*
1 Helpful Operations

1.1 ~getRelationCopy~

Returns a pointer to the copy of the relation of relPointer.

*/

OrderedRelation* getRelationCopy(const string relTypeInfo,
                                 OrderedRelation* relPointer)
{

  ListExpr relType;
  nl->ReadFromString ( relTypeInfo, relType );
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType(relType);
  Word wrel;
  wrel.setAddr(relPointer);
  Word res = OrderedRelation::Clone(relNumType, wrel);
  return (OrderedRelation*) res.addr;
}

Relation* getRelationCopy(const string relTypeInfo,
                          const Relation* relPointer)
{
  ListExpr strPtr = listutils::getPtrList(relPointer);
  string querystring = "(consume (feed (" + relTypeInfo + "(ptr "+
                        nl->ToString(strPtr) + "))))";
  Word resultWord;
  int QueryExecuted = QueryProcessor::ExecuteQuery(querystring, resultWord);
  assert (QueryExecuted);
  return (Relation*) resultWord.addr;
}

/*
1.1 ~relationToList~

Returns the list representation of rel.

*/

ListExpr relationToList(Relation* rel, const string relTypeInfo)
{
  if (rel != 0)
  {
    GenericRelationIterator* it = rel->MakeScan();
    ListExpr typeInfo;
    nl->ReadFromString(relTypeInfo, typeInfo);
    ListExpr relList = Relation::Out(typeInfo, it);
    return relList;
  }
  else
    return nl->TheEmptyList();
}

ListExpr relationToList(OrderedRelation* rel, const string relTypeInfo)
{
  if (rel != 0)
  {
    ListExpr typeInfo;
    nl->ReadFromString(relTypeInfo, typeInfo);
    Word w;
    w.setAddr(rel);
    ListExpr relList = OrderedRelation::Out(typeInfo, w);
    return relList;
  }
  else
    return nl->TheEmptyList();
}

Relation* relationFromList(const ListExpr nlRel, const string descriptor,
                          const int errorPos, ListExpr& errorInfo,
                          bool& correct)
{
  ListExpr relType;
  nl->ReadFromString(descriptor, relType);
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType ( relType );
  return (Relation*) Relation::In(relNumType, nlRel, errorPos, errorInfo,
                                  correct);
}

OrderedRelation* ordRelationFromList(const ListExpr nlRel,
                                     const string descriptor,
                                     const int errorPos, ListExpr& errorInfo,
                                     bool& correct)
{
  ListExpr relType;
  nl->ReadFromString(descriptor, relType);
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType ( relType );
  Word wRel = OrderedRelation::In(relNumType, nlRel, errorPos, errorInfo,
                                  correct);
  return (OrderedRelation*) wRel.addr;
}

/*
1.1 ~createBTree~

Creates an BTree over the attribut attr of the given relation rel, which is
described by descriptor.

*/

BTree* createBTree(const Relation* rel, const string descriptor,
                   const string attr)
{
  ListExpr relPtr = listutils::getPtrList(rel);
  string strQuery = "(createbtree (" + descriptor +
                      " (ptr " + nl->ToString(relPtr) + "))" + attr + ")";
  Word w;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, w);
  assert ( QueryExecuted );
  return ( BTree* ) w.addr;
}

/*
1.1 ~createRTree~

Creates the RTree over the given spatial attribute attr of the given relation
rel, which is described by descriptor.

*/

R_Tree<2,TupleId>* createRTree(const Relation* rel, const string descriptor,
                               const string attr)
{
  ListExpr relPtr = listutils::getPtrList(rel);
  string strQuery = "(bulkloadrtree(sortby(addid(feed (" + descriptor +
                 " (ptr " + nl->ToString(relPtr) + "))))((" + attr +" asc)))" +
                 attr +" TID)";
  Word w;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, w );
  assert ( QueryExecuted );
  return ( R_Tree<2,TupleId>* ) w.addr;
}

/*
1.1 ~openRelation~

Opens the relation described by descriptor from valueRecord starting at offset.

*/


bool openRelation(Relation*& rel, const string descriptor,
                  SmiRecord& valueRecord, size_t& offset)
{
  ListExpr relType;
  nl->ReadFromString ( descriptor, relType );
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType(relType);
  rel = Relation::Open(valueRecord,offset,relNumType);
  return (rel != 0);
}

bool openRelation(OrderedRelation*& rel, const string descriptor,
                  SmiRecord& valueRecord, size_t& offset)
{
  rel = 0;
  ListExpr relType;
  nl->ReadFromString ( descriptor, relType );
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType(relType);
  Word wrel;
  bool ok = OrderedRelation::Open(valueRecord, offset, relNumType, wrel);
  if (ok)
    rel = (OrderedRelation*) wrel.addr;
  return ok;
}


/*
1.1 ~openBTree~

Opens the btree described by descriptor from valueRecord starting at offset.

*/

bool openBTree(BTree*& tree, const string descriptor, SmiRecord& valueRecord,
                 size_t& offset)
{
  ListExpr treeType;
  nl->ReadFromString(descriptor,treeType);
  ListExpr treeNumType = SecondoSystem::GetCatalog()->NumericType(treeType);
  tree = BTree::Open(valueRecord,offset,treeNumType);
  return (tree != 0);
}

/*
1.1 ~saveRelation~

*/

bool saveRelation(const string descriptor, Relation* rel,
                  SmiRecord& valueRecord, size_t& offset)
{
  ListExpr relType;
  nl->ReadFromString ( descriptor, relType );
  ListExpr relNumType =  SecondoSystem::GetCatalog()->NumericType ( relType );
  return rel->Save(valueRecord, offset, relNumType);
}

bool saveRelation(const string descriptor, OrderedRelation* rel,
                  SmiRecord& valueRecord, size_t& offset)
{
  ListExpr relType;
  nl->ReadFromString ( descriptor, relType );
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType ( relType );
  Word wrel;
  wrel.setAddr(rel);
  return OrderedRelation::Save(valueRecord, offset, relNumType, wrel);
}


/*
1.1 ~saveBTree~

*/

bool saveBTree(const string descriptor, BTree* tree, SmiRecord& valueRecord,
               size_t& offset)
{
  ListExpr treeType;
  nl->ReadFromString ( descriptor, treeType );
  ListExpr treeNumericType =
      SecondoSystem::GetCatalog()->NumericType ( treeType );
  return tree->Save(valueRecord, offset, treeNumericType);
}

/*
1.1. reachedEndpoint

Checks if curPQElement endJID is enclosed in endJunctions list

*/

bool reachedEndpoint(const JPQEntry* actEntry,
                     const DbArray<PosJNetSpatial>* endJunctions,
                     double& dist, int& tgtPosInArray, bool& tgtIsStartJunc,
                     int& srcStartPathJID)
{
  PosJNetSpatial curEntry;
  int i = 0;
  double tmp = numeric_limits< double >::max();
  while(i < endJunctions->Size())
  {
    endJunctions->Get(i,curEntry);
    if (curEntry.GetStartJID() == actEntry->GetEndPartJID())
    {
      if (tmp > curEntry.GetDistFromStartJunction())
      {

        tmp = curEntry.GetDistFromStartJunction();
        tgtPosInArray = i;
        tgtIsStartJunc = true;
        srcStartPathJID = actEntry->GetStartPathJID();
      }
    }
    if (curEntry.GetEndJID() == actEntry->GetEndPartJID())
    {
      if (tmp > curEntry.GetDistFromEndJunction())
      {
        tmp = curEntry.GetDistFromEndJunction();
        tgtPosInArray = i;
        tgtIsStartJunc = false;
        srcStartPathJID = actEntry->GetStartPathJID();
      }
    }
    i++;
  }
  dist = tmp;
  bool result = (tgtPosInArray > -1);
  return result;
}

void getStartPosJNet(const DbArray<PosJNetSpatial>* srcEntries,
                     const int srcStartPathJID,
                     int& srcPosInArray,
                     bool& srcOverStartJunc,
                     PosJNetSpatial& start)
{
  if (srcEntries != NULL && srcEntries->Size() > 0)
  {
    double minDist = numeric_limits< double >::max();
    int i = 0;
    PosJNetSpatial tmp;
    while (i < srcEntries->Size())
    {
      srcEntries->Get(i,tmp);
      if (tmp.GetStartJID() == srcStartPathJID)
      {
        if (minDist > tmp.GetDistFromStartJunction())
        {
          minDist = tmp.GetDistFromStartJunction();
          srcPosInArray = i;
          srcOverStartJunc = true;
          start = tmp;
        }
      }
      if (tmp.GetEndJID() == srcStartPathJID)
      {
        if (minDist > tmp.GetDistFromEndJunction())
        {
          minDist = tmp.GetDistFromEndJunction();
          srcPosInArray = i;
          srcOverStartJunc = false;
          start = tmp;
        }
      }
      i++;
    }
  }
}

double getLength(const DbArray<JRouteInterval>* sp)
{
  double result = 0.0;
  if (sp != NULL && sp->Size() > 0)
  {
    JRouteInterval rint(false);
    for (int i = 0; i < sp->Size(); i++)
    {
      sp->Get(i,rint);
      result = result + rint.GetLength();
    }
  }
  return result;
}

/*
1.1 correctedPos

Corrects rounding errors at start or end of an section curve.

*/

double correctedPos(const double oldpos, const double routeLength,
                    const double tolerance)
{
  if (AlmostEqualAbsolute(oldpos, routeLength, tolerance))
    return routeLength;
  else
    if (AlmostEqualAbsolute(oldpos, 0.0, tolerance))
      return 0.0;
    else
      return oldpos;
}

void checkEndTimeCorrected(bool& endTimeCorrected, Instant& instInter1,
                      Instant& instInter2, const Instant TIMECORRECTION)
{
  if (endTimeCorrected && instInter1 < instInter2)
    endTimeCorrected = false;
  if (instInter1 >= instInter2)
  {
    instInter2 = instInter1 + TIMECORRECTION;
    endTimeCorrected = true;
  }
}


Point* getPointFromCurveForPosRememberLRSPos(double pos,
                                             const SimpleLine* curve,
                                             LRS& lrs, int& lrspos)
{
  Point result(false);
  if( !curve->GetStartSmaller())
    pos = curve->Length() - pos;
  LRS lrs1( pos, 0 );
  lrs = lrs1;
  if(curve->Find( lrs, lrspos ))
  {
    result.SetDefined( true );
    curve->Get( lrspos, lrs );
    HalfSegment hs;
    curve->Get( lrs.hsPos, hs );
    result = hs.AtPosition( pos);
  }
  return new Point(result);
}

Point* getPointFromCurveForPosFromLRS(double pos,
                                      const SimpleLine* curve,
                                      LRS& lrs)
{
  if( !curve->GetStartSmaller())
    pos = curve->Length() - pos;
  HalfSegment hs;
  curve->Get( lrs.hsPos, hs );
  return new Point(hs.AtPosition(pos-lrs.lrsPos));
}

void addSimulatedTrip(const JUnit& ju,
                      const Instant starttime, const Instant endtime,
                      const bool lc, const bool rc, Point*& startPoint,
                      const double epos, SimpleLine*& lastCurve,
                      LRS& lrs, int& lrspos, Instant& lastEndTime,
                      MPoint& result, const bool up,
                      const Instant& TIMECORRECTION, bool& endTimeCorrected)
{
  Instant instInter1 = starttime;
  Instant instInter2 = endtime;
  Point interStart(*startPoint);
  Point interEnd(*startPoint);
  HalfSegment curHS;
  bool end = false;
  if (up)
  {
    while (!end && lrspos < lastCurve->Size()/2)
    {
      lrspos++;
      lastCurve->Get(lrspos, lrs);
      lastCurve->Get(lrs.hsPos, curHS);
      if (lrs.lrsPos <= epos)
      {
        interEnd = curHS.AtPosition(0);
        instInter2 = ju.TimeAtPos(lrs.lrsPos);
        checkEndTimeCorrected (endTimeCorrected, instInter1,
                                 instInter2, TIMECORRECTION);
        result.Add(UPoint(Interval<Instant> (instInter1, instInter2, lc,rc),
                          interStart, interEnd));
        instInter1 = instInter2;
        interStart = interEnd;
        end = AlmostEqual(epos, lrs.lrsPos);
      }
      else
        end = true;
    }
    if (lrs.lrsPos > epos)
    {
      lrspos--;
      lastCurve->Get(lrspos, lrs);
      lastCurve->Get(lrs.hsPos, curHS);
      interEnd = curHS.AtPosition(epos-lrs.lrsPos);
      instInter2 = endtime;
      checkEndTimeCorrected (endTimeCorrected, instInter1,
                                 instInter2, TIMECORRECTION);
      result.Add(UPoint(Interval<Instant> (instInter1, instInter2, lc,rc),
                        interStart, interEnd));
    }
  }
  else
  {
    while (!end && lrspos >= 0)
    {
      lastCurve->Get(lrs.hsPos, curHS);
      if (lrs.lrsPos >= epos)
      {
        interEnd = curHS.AtPosition(0);
        instInter2 = ju.TimeAtPos(lrs.lrsPos);
        checkEndTimeCorrected (endTimeCorrected, instInter1,
                               instInter2, TIMECORRECTION);
        result.Add(UPoint(Interval<Instant> (instInter1, instInter2, lc,rc),
                          interStart, interEnd));
        instInter1 = instInter2;
        interStart = interEnd;
        end = AlmostEqual(epos, lrs.lrsPos);
        if (!end) lrspos--;
        lastCurve->Get(lrspos, lrs);
      }
      else
        end = true;
    }
    if (lrs.lrsPos < epos)
    {
      lastCurve->Get(lrspos, lrs);
      lastCurve->Get(lrs.hsPos, curHS);
      interEnd = curHS.AtPosition(epos-lrs.lrsPos);
      instInter2 = endtime;
      checkEndTimeCorrected (endTimeCorrected, instInter1,
                                 instInter2, TIMECORRECTION);
      result.Add(UPoint(Interval<Instant> (instInter1, instInter2, lc,rc),
                          interStart, interEnd));
    }
  }
  lastEndTime = instInter2;
  *startPoint = interEnd;
}

void addUnitsToResult(const JUnit& ju, SimpleLine*& routeCurve,
                      const double tolerance, LRS& lrs, int& lrspos,
                      bool& endTimeCorrected, Instant& instInter1,
                      Instant& instInter2, const Instant TIMECORRECTION,
                      Point*& lastEndPoint, Instant& lastEndTime,
                      MPoint& result)
{
  Direction compD(Down);
  JRouteInterval jurint = ju.GetRouteInterval();
  Interval<Instant> jutime = ju.GetTimeInterval();
  double epos = correctedPos(jurint.GetEndPosition(),
                             routeCurve->Length(), tolerance);
  if (jurint.GetSide().Compare(compD) == 0)
  {
    if (epos >= lrs.lrsPos )
    {
      Point* endP = getPointFromCurveForPosFromLRS(epos, routeCurve,
                                                   lrs);
      checkEndTimeCorrected (endTimeCorrected, instInter1,
                             instInter2, TIMECORRECTION);
      result.Add(UPoint(Interval<Instant>( instInter1, instInter2,
                                           jutime.lc, jutime.rc),
                        *lastEndPoint, *endP));
      lastEndPoint->DeleteIfAllowed();
      lastEndPoint = endP;
      lastEndTime = instInter2;
      endP = 0;
    }
    else
    {
      addSimulatedTrip(ju, instInter1, instInter2, jutime.lc, jutime.rc,
                       lastEndPoint, epos, routeCurve, lrs, lrspos,
                       lastEndTime, result, false, TIMECORRECTION,
                       endTimeCorrected);
    }
  }
  else
  {
    int lrsnext = lrspos;
    if (lrsnext < routeCurve->Size()/2) lrsnext++;
    LRS lrsX;
    routeCurve->Get(lrsnext, lrsX);
    if (epos <= lrsX.lrsPos)
    {
      Point* endP = getPointFromCurveForPosFromLRS(epos, routeCurve,
                                                   lrs);
      checkEndTimeCorrected (endTimeCorrected, instInter1,
                             instInter2, TIMECORRECTION);
      result.Add(UPoint(Interval<Instant> (instInter1, instInter2,
                                          jutime.lc, jutime.rc),
                        *lastEndPoint, *endP));
      lastEndPoint->DeleteIfAllowed();
      lastEndPoint = endP;
      lastEndTime = instInter2;
      endP = 0;
    }
    else
    {
      addSimulatedTrip(ju, instInter1, instInter2, jutime.lc, jutime.rc,
                       lastEndPoint, epos, routeCurve, lrs, lrspos,
                       lastEndTime, result, true, TIMECORRECTION,
                       endTimeCorrected);
    }
  }
}

/*
1 Implementation of class JNetwork

1.1 Constructors and Deconstructor

*/

JNetwork::JNetwork()
{}

JNetwork::JNetwork(const bool def) :
  defined(def), tolerance(0.0), junctions(0), sections(0), routes(0),
  netdistances(0), junctionsBTree(0), junctionsRTree(0), sectionsBTree(0),
  sectionsRTree(0), routesBTree(0)
{
  strcpy(id,"");
}

JNetwork::JNetwork(const string nid, const double t,
                   const Relation* injunctions, const Relation* insections,
                   const Relation* inroutes) :
  defined(true), tolerance(t),
  junctions(getRelationCopy(JNetUtil::GetJunctionsRelationTypeInfo(),
                            injunctions)),
  sections(getRelationCopy(JNetUtil::GetSectionsRelationTypeInfo(),
                           insections)),
  routes(getRelationCopy(JNetUtil::GetRoutesRelationTypeInfo(), inroutes)),
  netdistances(0), junctionsBTree(0), junctionsRTree(0), sectionsBTree(0),
  sectionsRTree(0), routesBTree(0)
{
  strcpy(id, nid.c_str());
  InitNetdistances();
  CreateTrees();
}

JNetwork::JNetwork(const string nid, const double t,
                   const Relation* injunctions, const Relation* insections,
                   const Relation* inroutes, OrderedRelation* inDist) :
  defined(true), tolerance(t),
  junctions(getRelationCopy(JNetUtil::GetJunctionsRelationTypeInfo(),
                            injunctions)),
  sections(getRelationCopy(JNetUtil::GetSectionsRelationTypeInfo(),
                           insections)),
  routes(getRelationCopy(JNetUtil::GetRoutesRelationTypeInfo(), inroutes)),
  netdistances(getRelationCopy(JNetUtil::GetNetdistancesRelationTypeInfo(),
                               inDist)),
  junctionsBTree(0), junctionsRTree(0), sectionsBTree(0),
  sectionsRTree(0), routesBTree(0)
{
  strcpy(id, nid.c_str());
  CreateTrees();
}


JNetwork::JNetwork(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, bool& ok) :
  defined(ok), tolerance(0.0), junctions(0), sections(0), routes(0),
  netdistances(0), junctionsBTree(0), junctionsRTree(0), sectionsBTree(0),
  sectionsRTree(0), routesBTree(0)
{
  Word w;
  ListExpr idLE;
  nl->ReadFromString(CcString::BasicType(), idLE);
  ListExpr numId = SecondoSystem::GetCatalog()->NumericType(idLE);
  ok = OpenAttribute<CcString>(valueRecord, offset, numId, w);

  if (ok)
  {
    CcString* stn = (CcString*)w.addr;
    strcpy(id, stn->GetValue().c_str());
    stn->DeleteIfAllowed();
  }
  else
  {
    strcpy(id,"");
  }


  if (ok && id == Symbol::UNDEFINED())
  {
    ok = false;
  }

  if (ok)
  {
    nl->ReadFromString(CcReal::BasicType(), idLE);
    numId = SecondoSystem::GetCatalog()->NumericType(idLE);
    ok = OpenAttribute<CcReal>(valueRecord, offset, numId, w);
  }

  if (ok)
  {
    CcReal* tol = (CcReal*) w.addr;
    tolerance = tol->GetRealval();
    tol->DeleteIfAllowed();
  }

  if (ok)
    ok = openRelation(junctions, JNetUtil::GetJunctionsRelationTypeInfo(),
                      valueRecord, offset);

  if (ok)
    ok = openRelation(sections, JNetUtil::GetSectionsRelationTypeInfo(),
                      valueRecord, offset);

  if (ok)
    ok = openRelation(routes, JNetUtil::GetRoutesRelationTypeInfo(),
                      valueRecord, offset);

  if (ok)
    ok = openRelation(netdistances, JNetUtil::GetNetdistancesRelationTypeInfo(),
                      valueRecord, offset);

  if (ok)
    ok = openBTree(junctionsBTree, JNetUtil::GetJunctionsBTreeTypeInfo(),
                   valueRecord, offset);

  if (ok)
    ok = openBTree(sectionsBTree, JNetUtil::GetSectionsBTreeTypeInfo(),
                   valueRecord, offset);

  if (ok)
    ok = openBTree(routesBTree, JNetUtil::GetRoutesBTreeTypeInfo(),
                   valueRecord, offset);

  Word wTree;

  if (ok)
    ok = junctionsRTree->Open(valueRecord, offset,
                              JNetUtil::GetJunctionsRTreeTypeInfo(), wTree);

  if (ok)
    junctionsRTree = (R_Tree<2,TupleId>*) wTree.addr;

  if (ok)
    ok = sectionsRTree->Open(valueRecord, offset,
                             JNetUtil::GetSectionsRTreeTypeInfo(), wTree);

  if (ok)
    sectionsRTree = (R_Tree<2,TupleId>*) wTree.addr;

  defined = ok;

  if (!ok) Destroy();
}


JNetwork::~JNetwork()
{
  delete junctions;
  delete sections;
  delete routes;
  delete netdistances;
  delete junctionsBTree;
  delete junctionsRTree;
  delete sectionsBTree;
  delete sectionsRTree;
  delete routesBTree;
}

void JNetwork::Destroy()
{
  if (junctions != 0)
  {
    junctions->Delete();
    junctions = 0;
  }

  if (sections != 0)
  {
    sections->Delete();
    sections = 0;
  }

  if (routes != 0)
  {
    routes->Delete();
    sections = 0;
  }

  if (netdistances != 0)
  {
    netdistances->Clear();
    netdistances = 0;
  }

  if (junctionsBTree != 0)
  {
    junctionsBTree->DeleteFile();
    junctionsBTree = 0;
  }

  if (sectionsBTree != 0)
  {
    sectionsBTree->DeleteFile();
    sectionsBTree = 0;
  }

  if (routesBTree != 0)
  {
    routesBTree->DeleteFile();
    routesBTree = 0;
  }

  if (junctionsRTree != 0)
  {
    junctionsRTree->DeleteFile();
    junctionsRTree = 0;
  }

  if (sectionsRTree != 0)
  {
    sectionsRTree->DeleteFile();
    sectionsRTree = 0;
  }

  SetDefined(false);
}

/*
1.1 Get Network Data

*/

bool JNetwork::IsDefined() const
{
  return defined;
}

const STRING_T* JNetwork::GetId() const
{
  return &id;
}

double JNetwork::GetTolerance() const
{
  return tolerance;
}

string JNetwork::GetJunctionsRelationType()
{
  return JNetUtil::GetJunctionsRelationTypeInfo();
}

string JNetwork::GetSectionsRelationType()
{
  return JNetUtil::GetSectionsRelationTypeInfo();
}

string JNetwork::GetRoutesRelationType()
{
  return JNetUtil::GetRoutesRelationTypeInfo();
}

string JNetwork::GetNetdistancesRelationType()
{
  return JNetUtil::GetNetdistancesRelationTypeInfo();
}

string JNetwork::GetNetdistancesTupleType()
{
  return JNetUtil::GetNetdistancesTupleTypeInfo();
}

Rectangle< 2 > JNetwork::GetBoundingBox() const
{
  if (sectionsRTree != 0)
    return sectionsRTree->BoundingBox();
  else
    return Rectangle<2>(false);
}


Relation* JNetwork::GetJunctionsCopy() const
{
  return junctions->Clone();
}

Relation* JNetwork::GetRoutesCopy() const
{
  return routes->Clone();
}

Relation* JNetwork::GetSectionsCopy() const
{
  return sections->Clone();
}

OrderedRelation* JNetwork::GetNetdistancesCopy() const
{
  return getRelationCopy(JNetUtil::GetNetdistancesRelationTypeInfo(),
                         netdistances);
}

void JNetwork::SetDefined(const bool def)
{
  defined = def;
}

void JNetwork::SetTolerance(const double t)
{
  tolerance = t;
}

/*
1.1 Secondo Integration

*/

ListExpr JNetwork::Out(ListExpr typeInfo, Word value)
{
  JNetwork* source = (JNetwork*) value.addr;

  if (source == 0 || !source->IsDefined())
  {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  else
  {
    ListExpr netId = nl->StringAtom(*source->GetId());
    ListExpr toleranceList = nl->RealAtom(source->GetTolerance());
    ListExpr junclist = source->JunctionsToList();
    ListExpr sectlist = source->SectionsToList();
    ListExpr routelist = source->RoutesToList();
    ListExpr dislist = source->NetdistancesToList();
    return nl->SixElemList(netId, toleranceList, junclist, sectlist,
                            routelist, dislist);
  }
}

Word JNetwork::In(const ListExpr typeInfo, const ListExpr instance,
                  const int errorPos, ListExpr& errorInfo, bool& correct)
{
  correct = true;
  if(nl->ListLength(instance) == 1)
  {
    if (nl->IsAtom(instance) &&
        nl->SymbolValue(instance) == Symbol::UNDEFINED())
    {
       correct = true;
       JNetwork* n = new JNetwork(false);
       return SetWord(n);
    }
  }
  else
  {
    if (nl->ListLength(instance) == 6)
    {
      ListExpr netId = nl->First(instance);
      string nid = nl->StringValue(netId);
      ListExpr tolList = nl->Second(instance);
      double tol = nl->RealValue(tolList);
      Relation* juncRel = 0;
      Relation* sectRel = 0;
      Relation* routeRel = 0;
      OrderedRelation* distRel = 0;

      juncRel = relationFromList(nl->Third(instance),
                                 JNetUtil::GetJunctionsRelationTypeInfo(),
                                 errorPos, errorInfo, correct);
      if (correct)
      {
         sectRel = relationFromList(nl->Fourth(instance),
                                    JNetUtil::GetSectionsRelationTypeInfo(),
                                    errorPos, errorInfo, correct);
      }

      if (correct)
      {
        routeRel = relationFromList(nl->Fifth(instance),
                                    JNetUtil::GetRoutesRelationTypeInfo(),
                                    errorPos, errorInfo, correct);
      }

      if (correct)
      {
        distRel = ordRelationFromList(nl->Sixth(instance),
                                   JNetUtil::GetNetdistancesRelationTypeInfo(),
                                      errorPos, errorInfo, correct);
      }

      if (!correct)
      {
        if (juncRel != 0) juncRel->Delete();
        if (sectRel != 0) sectRel->Delete();
        if (routeRel != 0) routeRel->Delete();
        if (distRel != 0)
        {
          distRel->Clear();
          delete distRel;
        }
        return SetWord(Address(0));
      }

      JNetwork* n = new JNetwork(nid, tol, juncRel, sectRel, routeRel, distRel);

      juncRel->Delete();
      sectRel->Delete();
      routeRel->Delete();
      distRel->Clear();
      delete distRel;
      return SetWord(n);
    }
  }
  correct = false;
  return SetWord(Address(0));
}

Word JNetwork::Create(const ListExpr typeInfo)
{
  return SetWord ( new JNetwork(false));
}

void JNetwork::Delete( const ListExpr typeInfo, Word& w )
{
  JNetwork* net = (JNetwork*) w.addr;
  delete net;
  w.addr = 0;
}

void JNetwork::Close( const ListExpr typeInfo, Word& w )
{
  delete static_cast<JNetwork*> ( w.addr );
  w.addr = 0;
}

Word JNetwork::Clone( const ListExpr typeInfo, const Word& w )
{
  JNetwork* source = (JNetwork*) w.addr;
  JNetwork* clone = new JNetwork(*source->GetId(),
                                 source->GetTolerance(),
                                 source->junctions,
                                 source->sections,
                                 source->routes,
                                 source->netdistances);
  return SetWord(clone);
}

void* JNetwork::Cast( void* addr )
{
  return (new (addr) JNetwork);
}

bool JNetwork::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  return nl->IsEqual(type, BasicType());
}

int JNetwork::SizeOf()
{
  return sizeof(JNetwork);
}

bool JNetwork::Save(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo, Word& value)
{
  JNetwork* source = (JNetwork*) value.addr;
  if (source->IsDefined())
  {
    return source->Save(valueRecord, offset, typeInfo);
  }
  else
  {
    CcString* stn = new CcString(true,Symbol::UNDEFINED());
    ListExpr idLE;
    nl->ReadFromString(CcString::BasicType(), idLE);
    ListExpr numId = SecondoSystem::GetCatalog()->NumericType(idLE);
    Attribute::Save(valueRecord, offset, numId, stn);
    return true;
  }
}

bool JNetwork::Save(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr  typeInfo)
{
  CcString* stn = new CcString(true, id);
  ListExpr idLE;
  nl->ReadFromString(CcString::BasicType(), idLE);
  ListExpr numId = SecondoSystem::GetCatalog()->NumericType(idLE);
  Attribute::Save(valueRecord, offset, numId, stn);
  stn->DeleteIfAllowed();

  CcReal* tol = new CcReal(true, tolerance);
  nl->ReadFromString(CcReal::BasicType(), idLE);
  numId = SecondoSystem::GetCatalog()->NumericType(idLE);
  Attribute::Save(valueRecord, offset, numId, tol);
  tol->DeleteIfAllowed();

  bool ok = saveRelation(JNetUtil::GetJunctionsRelationTypeInfo(), junctions,
                      valueRecord, offset);

  if (ok)
    ok = saveRelation(JNetUtil::GetSectionsRelationTypeInfo(), sections,
                      valueRecord, offset);

  if (ok)
    ok = saveRelation(JNetUtil::GetRoutesRelationTypeInfo(), routes,
                      valueRecord, offset);

  if (ok)
    ok = saveRelation(JNetUtil::GetNetdistancesRelationTypeInfo(),
                      netdistances, valueRecord, offset);

  if (ok)
    ok = saveBTree(JNetUtil::GetJunctionsBTreeTypeInfo(), junctionsBTree,
                   valueRecord, offset);

  if (ok)
    ok = saveBTree(JNetUtil::GetSectionsBTreeTypeInfo(), sectionsBTree,
                   valueRecord, offset);

  if (ok)
    ok = saveBTree(JNetUtil::GetRoutesBTreeTypeInfo(), routesBTree,
                   valueRecord, offset);

  if (ok)
    ok = junctionsRTree->Save(valueRecord, offset);

  if (ok)
    ok = sectionsRTree->Save(valueRecord, offset);

  return ok;
}

bool JNetwork::Open(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo, Word& value )
{
  bool ok = false;
  value.setAddr(new JNetwork(valueRecord, offset, typeInfo, ok));
  return ok;
}

ListExpr JNetwork::Property()
{
  return nl->TwoElemList(
    nl->FourElemList(
      nl->StringAtom("Signature"),
      nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"),
      nl->StringAtom("Example List")),
    nl->FourElemList(
      nl->StringAtom("-> " + Kind::JNETWORK()),
      nl->StringAtom(BasicType()),
      nl->TextAtom("(" + CcString::BasicType() + " " + CcReal::BasicType() +
        " " + JNetUtil::GetJunctionsRelationTypeInfo() + " " +
        JNetUtil::GetSectionsRelationTypeInfo() + " " +
        JNetUtil::GetRoutesRelationTypeInfo() + " " +
        JNetUtil::GetNetdistancesRelationTypeInfo() + "), the" +
        " string defines the name of the network, it is followed by the " +
        "tolerance value for the network representation used in map matching "+
        "algorithms, and the network data for junctions, sections, routes and "+
        "network distances in nested list format."),
      nl->StringAtom("(name tol junctions sections routes dist)")));
}

/*
1.1 Standard Operations

*/

ostream& JNetwork::Print(ostream& os) const
{
  os << "Network: ";
  if (!IsDefined())
    os << Symbol::UNDEFINED() << endl;
  else
  {
    os << "Id: " << id << endl;

    os << "Tolerance: " << tolerance << endl;

    os << "Junctions: " << endl;
    if (junctions !=  0)junctions->Print(os);
    else os << "not defined" << endl;

    os << "Sections: " << endl;
    if (sections != 0) sections->Print(os);
    else os << "not defined" << endl;

    os << "Routes: " << endl;
    if (routes != 0) routes->Print(os);
    else os << "not defined" << endl;

    os << "Network Distances:" << endl;
    if (netdistances != 0) netdistances->Print(os);
    else os << "not defined" << endl;

  }
  os << endl;
  return os;
}

const string JNetwork::BasicType()
{
  return "jnet";
}

const bool JNetwork::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.1 Translation operations

1.1.1 GetNetworkValueOf

Translates spatial data types into their Network representation.

*/

RouteLocation* JNetwork::GetNetworkValueOf(const Point* p) const
{
  RouteLocation* res = 0;
  double pos = 0.0;
  Tuple* actSect = GetSectionTupleFor(p, pos);
  if (actSect != 0)
  {
    JRouteInterval* actInt = GetSectionFirstRouteInterval(actSect);
    res = GetRLocOfPosOnRouteInterval(actInt, pos);
    actSect->DeleteIfAllowed();
    actInt->DeleteIfAllowed();
  }
  return res;
}

RouteLocation* JNetwork::GetNetworkValueOfOn(const Point* p,
                                             const Tuple* sectTup) const
{
  RouteLocation* result = 0;
  SimpleLine* actCurve = GetSectionCurve(sectTup);
  if (actCurve != 0)
  {
    double pos = 0.0;
    if(actCurve->AtPoint(*p, pos, tolerance))
    {
      JRouteInterval* actInt = GetSectionFirstRouteInterval(sectTup);
      result = GetRLocOfPosOnRouteInterval(actInt, pos);
      actInt->DeleteIfAllowed();
    }
    actCurve->DeleteIfAllowed();
  }
  return result;
}

JListRLoc* JNetwork::GetNetworkValuesOf(const Point* p) const
{
  JListRLoc* res = 0;
  double pos = 0.0;
  Tuple* actSect = GetSectionTupleFor(p, pos);
  if (AlmostEqual(pos, 0.0))
    res = GetSectionStartJunctionRLocs(actSect);
  else
  {
    if (AlmostEqual(pos, GetSectionLength(actSect)))
      res = GetSectionEndJunctionRLocs(actSect);
    else
    {
      res = new JListRLoc(0);
      res->StartBulkload();
      JListRInt* rints = GetSectionListRouteIntervals(actSect);
      JRouteInterval actInt;
      for(int i = 0; i < rints->GetNoOfComponents(); i++)
      {
        rints->Get(i,actInt);
        RouteLocation* result = GetRLocOfPosOnRouteInterval(&actInt, pos);
        res->operator+=(*result);
        result->DeleteIfAllowed();
      }
      res->EndBulkload();
      rints->Destroy();
      rints->DeleteIfAllowed();
    }
  }
  actSect->DeleteIfAllowed();
  return res;
}

bool JNetwork::GetNetworkValueOf(const Line* in, JLine* result) const
{
  result->Clear();
  result->SetNetworkId(*GetId());
  if (in != 0 && in->IsDefined() && !in->IsEmpty())
  {
    result->StartBulkload();
    HalfSegment hs;
    for (int i = 0; i < in->Size(); i++)
    {
      in->Get(i,hs);
      if (hs.IsLeftDomPoint())
      {
        JRouteInterval* actInt = GetNetworkValueOf(hs);
        if (actInt != NULL)
        {
          if (actInt->IsDefined())
            result->Add(*actInt);
          actInt->DeleteIfAllowed();
          actInt = 0;
        }
      }
    }
    result->EndBulkload();
    return true;
  }
  else
    return false;
}

bool JNetwork::GetNetworkValueOf(const MPoint* in, MJPoint* result)
{
  result->Clear();
  if (in != 0 && in->IsDefined())
  {
    result->SetDefined(true);
    result->SetNetworkId(*GetId());
    if (!in->IsEmpty())
    {
      UPoint actSource;
      int i = 0;
      RouteLocation* startPos = 0;
      RouteLocation* endPos = 0;
      Instant starttime(0.0);
      Instant endtime(0.0);
      bool lc = false;
      bool rc = false;
      Tuple* actSectTup = 0;
      Tuple* startSectTup = 0;
      Tuple* endSectTup = 0;
      JRouteInterval* actRInt = 0;
      SimpleLine* actCurve = 0;
      double distStart = 0.0;
      double distEnd = 0.0;
      result->StartBulkload();
      while (i < in->GetNoComponents())
      {
        //find valid startposition in network
        while((startPos == 0 || !startPos->IsDefined()) &&
               i < in->GetNoComponents())
        {
          if (startPos != 0)
          {
            startPos->DeleteIfAllowed();
            startPos = 0;
          }
          in->Get(i,actSource);
          if (actCurve != 0)
          {
            if (actCurve->AtPoint(actSource.p0, distStart, tolerance))
            {
              startPos = GetRLocOfPosOnRouteInterval(actRInt, distStart);
            }
            else
            {
              actCurve->DeleteIfAllowed();
              actCurve = 0;
              if (actSectTup != 0)
              {
                actSectTup->DeleteIfAllowed();
                actSectTup = 0;
              }
              if (actRInt != 0)
              {
                actRInt->DeleteIfAllowed();
                actRInt = 0;
              }
              if (startSectTup != 0)
              {
                startSectTup->DeleteIfAllowed();
                startSectTup = 0;
              }
            }
          }
          if (actSectTup == 0)
          {
            actSectTup = GetSectionTupleFor(&actSource.p0, distStart);
            if (actSectTup != 0)
            {
              actRInt = GetSectionFirstRouteInterval(actSectTup);
              actCurve = GetSectionCurve(actSectTup);
              startPos = GetRLocOfPosOnRouteInterval(actRInt, distStart);
              startSectTup = actSectTup;
              startSectTup->IncReference();
            }
          }
          starttime = actSource.getTimeInterval().start;
          lc = actSource.getTimeInterval().lc;
          if (startPos == 0 || !startPos->IsDefined())
            i++;
        }
        //find valid endposition in network
        while((endPos == 0 || !endPos->IsDefined()) &&
               i < in->GetNoComponents())
        {
          if (endPos != 0)
          {
            endPos->DeleteIfAllowed();
            endPos = 0;
          }
          in->Get(i, actSource);
          if (actCurve != 0 &&
              actCurve->AtPoint(actSource.p1, distEnd, tolerance))
          {
            endPos = GetRLocOfPosOnRouteInterval(actRInt, distEnd);
            endSectTup = actSectTup;
            endSectTup->IncReference();
          }
          else
          {
            if (actSectTup  != 0)
              actSectTup->DeleteIfAllowed();
            actSectTup = GetSectionTupleFor(&actSource.p1, distEnd);
            if (actSectTup != 0)
            {
              if (actRInt != 0)
                actRInt->DeleteIfAllowed();
              actRInt = GetSectionFirstRouteInterval(actSectTup);
              if (actCurve != 0)
                actCurve->DeleteIfAllowed();
              actCurve = GetSectionCurve(actSectTup);
              endPos = GetRLocOfPosOnRouteInterval(actRInt, distEnd);
              endSectTup = actSectTup;
              endSectTup->IncReference();
            }
          }
          endtime = actSource.getTimeInterval().end;
          rc = actSource.getTimeInterval().rc;
          if (endPos == 0 || !endPos->IsDefined())
            i++;
        }
        if (startPos != 0 &&  startPos->IsDefined() &&
            endPos != 0 && endPos->IsDefined() && actSource.p1.IsDefined() &&
            startSectTup != 0 && endSectTup != 0)
        { // got valid RouteLocations and
          SimulateTrip(*startPos, *endPos, starttime, endtime, lc, rc, result);
        }
        startPos->DeleteIfAllowed();
        startSectTup->DeleteIfAllowed();
        startSectTup = endSectTup;
        endSectTup = 0;
        startPos = endPos;
        endPos = 0;
        distStart = distEnd;
        starttime = endtime;
        lc = !rc;
        i++;
      }
      if (startPos != 0) startPos->DeleteIfAllowed();
      if (endPos != 0) endPos->DeleteIfAllowed();
      if (actSectTup  != 0) actSectTup->DeleteIfAllowed();
      if (actRInt != 0) actRInt->DeleteIfAllowed();
      if (actCurve != 0) actCurve->DeleteIfAllowed();
      if (startSectTup != 0) startSectTup->DeleteIfAllowed();
      if (endSectTup != 0) endSectTup->DeleteIfAllowed();
      result->EndBulkload();
    }
    return true;
  }
  else
    return false;
}


JListRLoc* JNetwork::GetNetworkValuesOf(const Tuple* actSect,
                                        const double distStart) const
{
  JListRLoc* res = 0;
  if (AlmostEqualAbsolute(distStart, 0.0, tolerance))
    res = GetSectionStartJunctionRLocs(actSect);
  else
  {
    if (AlmostEqualAbsolute(distStart, GetSectionLength(actSect), tolerance))
      res = GetSectionEndJunctionRLocs(actSect);
    else
    {
      if (0.0 <= distStart && distStart <= GetSectionLength(actSect))
      {
        JListRInt* rintList = GetSectionListRouteIntervals(actSect);
        if (rintList != 0)
        {
          if (rintList->IsDefined() && !rintList->IsEmpty())
          {
            res = new JListRLoc(true);
            res->StartBulkload();
            JRouteInterval  actInt;
            for (int i = 0; i < rintList->GetNoOfComponents(); i++)
            {
              rintList->Get(i, actInt);
              RouteLocation* result = GetRLocOfPosOnRouteInterval(&actInt,
                                                                  distStart);
              if (result != 0)
              {
                res->operator+=(*result);
                result->DeleteIfAllowed();
                result = 0;
              }
            }
            res->EndBulkload();
          }
          rintList->Destroy();
          rintList->DeleteIfAllowed();
        }
      }
    }
  }
  return res;
}

JListRLoc* JNetwork::GetNetworkValuesOf(const RouteLocation& rloc) const
{
  JListRLoc* res = 0;
  double pos = 0.0;
  Tuple* actSect = GetSectionTupleFor(rloc,pos);
  if (actSect != 0)
  {
    res = GetNetworkValuesOf(actSect,pos);
    actSect->DeleteIfAllowed();
  }
  return res;
}

JRouteInterval* JNetwork::GetNetworkValueOf(const HalfSegment& hs) const
{
  JRouteInterval* res = 0;
  Point lp = hs.GetLeftPoint();
  Point rp = hs.GetRightPoint();
  JListRLoc* leftrlocs = GetNetworkValuesOf(&lp);
  JListRLoc* rightrlocs = GetNetworkValuesOf(&rp);
  res = GetRouteIntervalFor(leftrlocs, rightrlocs, true);
  if (res != 0 && res->IsDefined())
    res->SetSide((Direction) Both);
  rightrlocs->DeleteIfAllowed();
  leftrlocs->DeleteIfAllowed();
  return res;
}

/*
1.1.1 GetSpatialValueOf

Transforms network datatypes into their corresponding spatial data types.

*/

Point* JNetwork::GetSpatialValueOf(const RouteLocation& rloc,
                                   const JListInt* routeSectList) const
{
  Point* res = 0;
  double pos = 0.0;
  int index = -1;
  Tuple* sectTup = GetSectionTupleFor(rloc, pos, routeSectList, index);
  res = GetSpatialValueOf(rloc, pos, sectTup);
  sectTup->DeleteIfAllowed();
  return res;
}

Point* JNetwork::GetSpatialValueOf(const JPoint& jp) const
{
  Point* res = 0;
  double relpos = 0.0;
  RouteLocation rloc = jp.GetLocation();
  Tuple* sectTup = GetSectionTupleFor(rloc, relpos);
  res = GetSpatialValueOf(rloc, relpos, sectTup);
  sectTup->DeleteIfAllowed();
  return res;
}

void JNetwork::GetSpatialValueOf(const JLine* jl, Line& result) const
{
  if (jl != 0 && jl->IsDefined() && !jl->IsEmpty())
  {
    result.Clear();
    result.StartBulkLoad();;
    JRouteInterval rint;
    for (int i = 0; i < jl->GetNoComponents(); i++)
    {
      jl->Get(i,rint);
      SimpleLine tmp(0);
      GetSpatialValueOf(rint, tmp);
      if (tmp.IsDefined() && !tmp.IsEmpty())
      {
        HalfSegment hs;
        for (int i = 0; i < tmp.Size(); i++)
        {
          tmp.Get(i,hs);
          result += hs;
        }
      }
    }
    result.EndBulkLoad();
  }
  else
    result.SetDefined(false);
}

void JNetwork::GetSpatialValueOf(const MJPoint* mjp, MPoint& result) const
{
  if (mjp != 0 && mjp->IsDefined() && !mjp->IsEmpty())
  {
    result.Clear();
    result.StartBulkLoad();
    JUnit ju;
    bool endTimeCorrected = false;
    Instant lastEnd(instanttype);
    JRouteInterval* lastRint = 0;
    SimpleLine* lastCurve = 0;
    Point* lastEndPoint = new Point(true,0.0,0.0);
    LRS lrs;
    int lrspos = -1;
    for (int i = 0; i < mjp->GetNoComponents(); i++)
    {
      mjp->Get(i,ju);
      SplitJUnit(ju, lastRint, lastCurve, endTimeCorrected, lastEnd,
                 lastEndPoint, lrs, lrspos, result);

    }
    result.EndBulkLoad();
    if (lastRint != 0) lastRint->DeleteIfAllowed();
    if (lastCurve != 0) lastCurve->DeleteIfAllowed();
    lastEndPoint->DeleteIfAllowed();
  }
  else
    result.SetDefined(false);
}


Point* JNetwork::GetSpatialValueOf(const RouteLocation& rloc,
                                   double relpos,
                                   const Tuple* actSect) const
{
  Point* res = 0;
  SimpleLine* sectCurve = GetSectionCurve(actSect);
  if (sectCurve != 0)
  {
    res = new Point(false);
    if (AlmostEqualAbsolute(relpos, sectCurve->Length(), tolerance))
      relpos = sectCurve->Length();
    else
      if (AlmostEqualAbsolute(relpos, 0.0, tolerance))
        relpos = 0.0;
    sectCurve->AtPosition(relpos, *res);
    sectCurve->DeleteIfAllowed();
  }
  return res;
}

void JNetwork::GetSpatialValueOf(const JRouteInterval& rint,
                                 SimpleLine& result) const
{
  JListInt* sectList = GetRouteSectionList(rint.GetRouteId());
  int fromInd = 0;
  int toInd = sectList->GetNoOfComponents()-1;
  GetSpatialValueOf(rint, sectList, fromInd, toInd, result);
  sectList->Destroy();
  sectList->DeleteIfAllowed();
}

Point* JNetwork::GetSpatialValueOf(const RouteLocation& rloc) const
{
  JListInt* sectList = GetRouteSectionList(rloc.GetRouteId());
  Point* res = GetSpatialValueOf(rloc, sectList);
  sectList->DeleteIfAllowed();
  return res;
}

void JNetwork::GetSpatialValueOf(const JRouteInterval& rint,
                                 const JListInt* sectList,
                                 const int fromIndex,
                                 const int toIndex,
                                 SimpleLine& result) const
{
  if (sectList != 0 &&
      0 <= fromIndex && fromIndex < sectList->GetNoOfComponents())
  {
    result.Clear();
    result.StartBulkLoad();
    HalfSegment hs;
    CcInt sidC;
    bool dirChanged = false;
    if (fromIndex > toIndex)
       dirChanged = true;
    int actindex = min(fromIndex, toIndex);
    if (actindex < 0) actindex = 0;
    int lastindex = max(fromIndex, toIndex);
    if (lastindex < 0 || lastindex > sectList->GetNoOfComponents()-1)
      lastindex = sectList->GetNoOfComponents()-1;
    while(actindex < sectList->GetNoOfComponents() && actindex <= lastindex)
    {
      sectList->Get(actindex,sidC);
      int sid = sidC.GetIntval();
      JListRInt* rintList = GetSectionListRouteIntervals(sid);
      if (rintList != 0)
      {
        JRouteInterval actInt;
        int j = 0;
        while (j < rintList->GetNoOfComponents())
        {
          rintList->Get(j,actInt);
          if (actInt.Overlaps(rint, false))
          {
            SimpleLine* actCurve = GetSectionCurve(sid);
            j = rintList->GetNoOfComponents();
            if (actInt.Inside(rint))
            {
              for (int i = 0; i < actCurve->Size(); i++)
              {
                actCurve->Get(i,hs);
                result += hs;
              }
            }
            else
            {
              double spos(0.0);
              double epos(0.0);
              SimpleLine* sl = new SimpleLine(0);
              if (actInt.Contains(rint))
              {
                spos = fabs(rint.GetFirstPosition() -
                                       actInt.GetFirstPosition());
                epos = fabs(rint.GetLastPosition() -
                                       actInt.GetFirstPosition());
              }
              else if (actInt.Contains(RouteLocation(rint.GetRouteId(),
                                                     rint.GetFirstPosition(),
                                                     rint.GetSide())))
              {
                spos = fabs(rint.GetFirstPosition() -
                                       actInt.GetFirstPosition());
                epos = fabs(actInt.GetLastPosition()-
                                       actInt.GetFirstPosition());
              }
              else if (actInt.Contains(RouteLocation(rint.GetRouteId(),
                                                     rint.GetLastPosition(),
                                                     rint.GetSide())))
              {
                spos = 0.0;
                epos = fabs(rint.GetLastPosition() -
                                       actInt.GetFirstPosition());
              }
              else
              {
                assert(false); //should never happen
              }
              actCurve->SubLine(min(spos,epos), max(spos,epos), *sl);
              if (sl != 0 && sl->IsDefined() && !sl->IsEmpty())
              {
                for (int i = 0; i < sl->Size(); i++)
                {
                  sl->Get(i,hs);
                  result += hs;
                }
              }
              if (sl != 0)
              {
                sl->Destroy();
                sl->DeleteIfAllowed();
              }
            }
            actCurve->DeleteIfAllowed();
            actCurve = 0;
          }
          j++;
        }
        rintList->Destroy();
        rintList->DeleteIfAllowed();
        rintList = 0;
      }
      actindex++;
    }
    result.EndBulkLoad();;
    if (dirChanged)
      result.SetStartSmaller(!result.StartsSmaller());
  }
  else
    result.SetDefined(false);
}

Point* JNetwork::GetSpatialValueOf(const RouteLocation& rloc,
                                   JRouteInterval*& routeRint,
                                   SimpleLine*& routeCurve,
                                   LRS& lrs, int& lrspos) const
{
  if (routeCurve != 0) routeCurve->DeleteIfAllowed();
  routeCurve = new SimpleLine(0);
  routeCurve->StartBulkLoad();
  Tuple* routeTuple = GetRouteTupleWithId(rloc.GetRouteId());
  JListInt* routeSectList = GetRouteSectionList(routeTuple);
  double routeLength = GetRouteLength(routeTuple);
  if (routeRint != 0) routeRint->DeleteIfAllowed();
  routeRint = new JRouteInterval(rloc.GetRouteId(), 0.0, routeLength, Both);
  Point startP, endP;
  int index = 0;
  CcInt cSid;
  Tuple* sectTup = 0;
  while(index < routeSectList->GetNoOfComponents())
  {
    routeSectList->Get(index, cSid);
    if (cSid.IsDefined())
    {
      sectTup = GetSectionTupleWithId(cSid.GetIntval());
      if (sectTup != 0)
      {
        JRouteInterval* curRint =
          GetSectionRouteIntervalForRID(rloc.GetRouteId(), sectTup);
        SimpleLine* sectCurve = GetSectionCurve(sectTup);
        if (AlmostEqual(curRint->GetFirstPosition(), 0.0))
          startP = sectCurve->StartPoint();
        if (AlmostEqual(curRint->GetLastPosition(), routeLength))
          endP = sectCurve->EndPoint();
        HalfSegment hs;
        for (int i = 0; i < sectCurve->Size(); i++)
        {
          sectCurve->Get(i,hs);
          routeCurve->operator+=(hs);
        }
        curRint->DeleteIfAllowed();
        curRint = 0;
        sectCurve->DeleteIfAllowed();
        sectCurve = 0;
        sectTup->DeleteIfAllowed();
        sectTup = 0;
      }
    }
    index++;
  }
  routeCurve->EndBulkLoad();
  routeTuple->DeleteIfAllowed();
  if (startP > endP) routeCurve->SetStartSmaller(false);
  else routeCurve->SetStartSmaller(true);
  double pos = correctedPos(rloc.GetPosition(), routeCurve->Length(),
                            tolerance);
  routeSectList->DeleteIfAllowed();
  return getPointFromCurveForPosRememberLRSPos(pos, routeCurve, lrs, lrspos);
}


Point* JNetwork::GetSpatialValueOf(const RouteLocation& rloc, int& curRid,
                                   JListInt*& routeSectList, int& index,
                                   JRouteInterval*& lastRint,
                                   SimpleLine*& lastCurve,
                                   LRS& lrs, int& lrspos) const
{
    if(rloc.GetRouteId() != curRid)
    {
      curRid = rloc.GetRouteId();
      if (routeSectList != 0)
        routeSectList->DeleteIfAllowed();
      routeSectList = GetRouteSectionList(curRid);
      index = 0;
    }
    CcInt cSid;
    bool found = false;
    Tuple* sectTup = 0;
    while(0 <= index && index < routeSectList->GetNoOfComponents() && !found)
    {
      if (sectTup != 0)
      {
        sectTup->DeleteIfAllowed();
        sectTup = 0;
      }
      if (lastRint != 0)
      {
        lastRint->DeleteIfAllowed();
        lastRint = 0;
      }
      routeSectList->Get(index, cSid);
      if (cSid.IsDefined())
      {
        sectTup = GetSectionTupleWithId(cSid.GetIntval());
        if (sectTup != 0)
        {
          lastRint = GetSectionRouteIntervalForRLoc(rloc, sectTup);
        }
      }
      if (lastRint == 0)
        index++;
      else
        found = true;
    }
    if (!found)
    {
      index = routeSectList->GetNoOfComponents() -1;
      while (index >= 0 && index < routeSectList->GetNoOfComponents() && !found)
      {
        if (sectTup != 0)
        {
          sectTup->DeleteIfAllowed();
          sectTup = 0;
        }
        if (lastRint != 0)
        {
          lastRint->DeleteIfAllowed();
          lastRint = 0;
        }
        routeSectList->Get(index, cSid);
        if (cSid.IsDefined())
        {
          sectTup = GetSectionTupleWithId(cSid.GetIntval());
          if (sectTup != 0)
            lastRint = GetSectionRouteIntervalForRLoc(rloc, sectTup);
        }
        if (lastRint == 0)
          index--;
        else
          found = true;
      }
    }
    if (found)
    {
      if (lastCurve != 0)
        lastCurve->DeleteIfAllowed();
      lastCurve = GetSectionCurve(sectTup);
      if (sectTup != 0)
        sectTup->DeleteIfAllowed();
      double pos = correctedPos(fabs(rloc.GetPosition() -
                                     lastRint->GetFirstPosition()),
                                lastCurve->Length(), tolerance);
      return getPointFromCurveForPosRememberLRSPos(pos, lastCurve, lrs, lrspos);
    }
    if (sectTup != 0)
      sectTup->DeleteIfAllowed();
    return 0;
}

Point* JNetwork::GetSpatialValueOf(const RouteLocation& rloc, int& curRid,
                                   JListInt*& routeSectList, int& index,
                                   JRouteInterval*& lastRint,
                                   SimpleLine*& lastCurve) const
{
  if (rloc.GetRouteId() != curRid)
  {
    curRid = rloc.GetRouteId();
    if (routeSectList != 0)
      routeSectList->DeleteIfAllowed();
    routeSectList = GetRouteSectionList(curRid);
    index = 0;
  }
  CcInt cSid;
  bool found = false;
  Tuple* sectTup = 0;
  while(0 <= index && index < routeSectList->GetNoOfComponents() && !found)
  {
    if (sectTup != 0)
    {
      sectTup->DeleteIfAllowed();
      sectTup = 0;
    }
    if (lastRint != 0)
    {
      lastRint->DeleteIfAllowed();
      lastRint = 0;
    }
    routeSectList->Get(index, cSid);
    if (cSid.IsDefined())
    {
      sectTup = GetSectionTupleWithId(cSid.GetIntval());
      if (sectTup != 0)
      {
        lastRint = GetSectionRouteIntervalForRLoc(rloc, sectTup);
      }
    }
    if (lastRint == 0)
      index++;
    else
      found = true;
  }
  if (!found)
  {
    index = routeSectList->GetNoOfComponents() -1;
    while (index >= 0 && index < routeSectList->GetNoOfComponents() && !found)
    {
      if (sectTup != 0)
      {
        sectTup->DeleteIfAllowed();
        sectTup = 0;
      }
      if (lastRint != 0)
      {
        lastRint->DeleteIfAllowed();
        lastRint = 0;
      }
      routeSectList->Get(index, cSid);
      if (cSid.IsDefined())
      {
        sectTup = GetSectionTupleWithId(cSid.GetIntval());
        if (sectTup != 0)
          lastRint = GetSectionRouteIntervalForRLoc(rloc, sectTup);
      }
      if (lastRint == 0)
        index--;
      else
        found = true;
    }
  }
  if (found)
  {
    if (lastCurve != 0)
      lastCurve->DeleteIfAllowed();
    lastCurve = GetSectionCurve(sectTup);
    if (sectTup != 0)
      sectTup->DeleteIfAllowed();
    double pos = correctedPos(fabs(rloc.GetPosition() -
                                   lastRint->GetFirstPosition()),
                              lastCurve->Length(), tolerance);
    Point* result = new Point(true);
    lastCurve->AtPosition(pos, *result);
    return result;
  }
  if (sectTup != 0)
    sectTup->DeleteIfAllowed();
  return 0;
}


/*
1.1. BoundingBox

*/
Rectangle< 3 > JNetwork::BoundingBox(const JUnit& ju) const
{
  SimpleLine resLine(0);
  GetSpatialValueOf(ju.GetRouteInterval(), resLine);
  Rectangle<2> curveBB = resLine.BoundingBox();
  double mintime = ju.GetTimeInterval().start.ToDouble();
  double maxtime = ju.GetTimeInterval().end.ToDouble();
  if (mintime == maxtime)
  {
    return Rectangle<3>(true,
                        curveBB.MinD(0) - tolerance,
                        curveBB.MaxD(0) + tolerance,
                        curveBB.MinD(1) - tolerance,
                        curveBB.MaxD(1) + tolerance,
                        mintime - tolerance,
                        maxtime + tolerance);
  }
  else
  {
    return Rectangle<3>(true,
                        curveBB.MinD(0) - tolerance,
                        curveBB.MaxD(0) + tolerance,
                        curveBB.MinD(1) - tolerance,
                        curveBB.MaxD(1) + tolerance,
                        mintime, maxtime);
  }

}

Rectangle< 2 > JNetwork::BoundingBox(const JRouteInterval& rint) const
{
  if (rint.IsDefined())
  {
    Rectangle<2> bBox(false);
    if (AlmostEqualAbsolute(rint.GetFirstPosition(), rint.GetLastPosition(),
                            tolerance))
    {
      Point* tmpRes = GetSpatialValueOf(rint.GetStartLocation());
      bBox = tmpRes->BoundingBox();
      tmpRes->DeleteIfAllowed();
    }
    else
    {
      SimpleLine tmpRes(0);
      GetSpatialValueOf(rint, tmpRes);
      bBox = tmpRes.BoundingBox();
    }
    return Rectangle<2>(true,
                        bBox.MinD(0), bBox.MaxD(0),
                        bBox.MinD(1), bBox.MaxD(1));
  }
  return Rectangle<2>(false, 0.0,0.0,0.0,0.0);
}

Rectangle<3> JNetwork::BoundingBox(const DbArray<JRouteInterval>& traj,
                                   const double mintime,
                                   const double maxtime) const
{
  JRouteInterval actRint;
  int i = 0;
  traj.Get(i,actRint);
  Rectangle<2> tmpres = BoundingBox(actRint);
  i++;
  while (i < traj.Size())
  {
    traj.Get(i,actRint);
    tmpres = tmpres.Union(BoundingBox(actRint));
    i++;
  }
  if (mintime == maxtime)
  {
    return Rectangle<3> (true,
                         tmpres.MinD(0), tmpres.MaxD(0),
                         tmpres.MinD(1), tmpres.MaxD(1),
                         mintime - tolerance, maxtime + tolerance);
  }
  else
  {
    return Rectangle<3> (true,
                         tmpres.MinD(0), tmpres.MaxD(0),
                         tmpres.MinD(1), tmpres.MaxD(1),
                         mintime, maxtime);
  }
}

/*
1.1 Operations for other network data types

1.1.1 ~Contains~

Checks if the given position(s) exist in the network.

*/

bool JNetwork::Contains(const RouteLocation& rloc) const {
  return ((rloc.GetPosition() >= 0.0 &&
          rloc.GetPosition() <= GetRouteLength(rloc.GetRouteId())) ||
          AlmostEqualAbsolute(rloc.GetPosition(), 0.0, tolerance) ||
          AlmostEqualAbsolute(rloc.GetPosition(),0.0, tolerance));
}

bool JNetwork::Contains(const JRouteInterval& rint) const{
  return ((rint.GetFirstPosition() >= 0.0 &&
          rint.GetLastPosition()<= GetRouteLength(rint.GetRouteId()))||
          AlmostEqualAbsolute(rint.GetFirstPosition(),0.0,tolerance) ||
          AlmostEqualAbsolute(rint.GetLastPosition(), 0.0,
                              GetRouteLength(rint.GetRouteId())));
}

/*
1.1.1 ~SimulateTrip~

*/

void JNetwork::SimulateTrip(const RouteLocation& source,
                            const RouteLocation& target,
                            const Instant& starttime,
                            const Instant& endtime,
                            MJPoint* result)
{
  bool lc = true;
  bool rc = (starttime == endtime);
  SimulateTrip(source, target, starttime, endtime, lc, rc, result);
}

void JNetwork::SimulateTrip(const RouteLocation& source,
                            const RouteLocation& target,
                            const Instant& starttime,
                            const Instant& endtime,
                            const bool& lc, const bool& rc,
                            MJPoint* result)
{
  JPath* tmpPath = new JPath(true);
  ShortestPath(source, target, tmpPath);
  double length = tmpPath->Length();
  if (tmpPath != 0)
  {
    if (length == 0 &&
        starttime.ToDouble() != endtime.ToDouble())
    {
      JUnit actUnit(JUnit(Interval<Instant>(starttime, endtime, lc, rc),
                          JRouteInterval(source.GetRouteId(),
                                       min(source.GetPosition(),
                                           target.GetPosition()),
                                       max(source.GetPosition(),
                                           target.GetPosition()),
                                       (Direction) Both)));
      result->Add(actUnit);
    }
    else
    {
      if (tmpPath->GetNoComponents() > 0)
      {
        ComputeAndAddUnits(tmpPath, starttime, endtime, lc, rc, length, result);
      }
    }
    tmpPath->Destroy();
    delete tmpPath;
  }
}

void JNetwork::ComputeAndAddUnits(const JPath* sp,
                                  const Instant& starttime,
                                  const Instant& endtime,
                                  const bool lc, const bool rc, double length,
                                  MJPoint* result) const
{
  JRouteInterval actInt;
  Instant unitstart = starttime;
  bool unitlc = lc;
  Instant unitend = endtime;
  bool unitrc = rc;
  for (int i = 0; i < sp->GetNoComponents(); i++)
  {
    sp->Get(i,actInt);
    if (actInt.GetLength() != 0.0)
    {
      if (i == sp->GetNoComponents()-1)
      {
        unitend = endtime;
        unitrc = rc;
      }
      else
      {
        unitend = unitstart +
                  ((endtime - starttime) * (actInt.GetLength() / length));
      }
      if (unitstart < unitend)
      {
        JUnit actUnit(JUnit(Interval<Instant>(unitstart, unitend,
                                              unitlc, unitrc),
                            actInt));
        result->Add(actUnit);
        unitstart = unitend;
        unitlc = !unitrc;
        unitrc = !unitlc;
        unitend = endtime;
      }
    }
  }
}

/*
1.1.1 ShortestPathTree

*/

void JNetwork::ShortestPathTree(const RouteLocation& source,
                                DbArray<PairIntDouble>* result,
                                const double distLimit)
{
  if (result != NULL)
  {
    result->clean();
    if (source.IsDefined())
    {
      PQManagement* pqueue = new PQManagement();
      InitPriorityQueue(pqueue, source, result);
      ProcessPriorityQueue(pqueue, result, distLimit);
      pqueue->Destroy();
      delete pqueue;
    }
  }
}

void JNetwork::ReverseShortestPathTree(const RouteLocation& source,
                                       DbArray<PairIntDouble>* result,
                                       const double distLimit)
{
  if (result != NULL)
  {
    result->clean();
    if (source.IsDefined())
    {
      PQManagement* pqueue = new PQManagement();
      InitReversePriorityQueue(pqueue, source, result);
      ProcessReversePriorityQueue(pqueue, result, distLimit);
      pqueue->Destroy();
      delete pqueue;
    }
  }
}



/*
1.1.1 ShortestPath

*/

void JNetwork::ShortestPath(const RouteLocation& source,
                            const RouteLocation& target,
                            JPath* result)
{
  DbArray<RouteLocation>* srcPositions = new DbArray<RouteLocation>(0);
  srcPositions->Append(source);
  DbArray<RouteLocation>* tgtPositions = new DbArray<RouteLocation>(0);
  tgtPositions->Append(target);
  ShortestPath(srcPositions, tgtPositions, result);
  srcPositions->Destroy();
  delete srcPositions;
  tgtPositions->Destroy();
  delete tgtPositions;
}

void JNetwork::ShortestPath(const RouteLocation& source,
                            const DbArray< RouteLocation >& target,
                            JPath* result)
{
  DbArray<RouteLocation>* srcPositions = new DbArray<RouteLocation>(0);
  srcPositions->Append(source);
  ShortestPath(srcPositions, &target, result);
  srcPositions->Destroy();
  delete srcPositions;
}

void JNetwork::ShortestPath(const DbArray< RouteLocation >& source,
                            const RouteLocation& target,
                            JPath* result)
{
  DbArray<RouteLocation>* tgtPositions = new DbArray<RouteLocation>(0);
  tgtPositions->Append(target);
  ShortestPath(&source, tgtPositions, result);
  tgtPositions->Destroy();
  delete tgtPositions;
}


void JNetwork::ShortestPath(const DbArray<RouteLocation>* sources,
                            const DbArray<RouteLocation>* targets,
                            JPath* result)
{
  if (sources != NULL && targets != NULL && result != NULL &&
      sources->Size() > 0 && targets->Size() > 0)
  {
    result->Clear();
    result->SetNetworkId(id);
    if (!JNetUtil::ArrayContainIntersections(*sources, *targets))
    {
      DbArray<PosJNetSpatial>* tgtEntries = new DbArray<PosJNetSpatial> (0);
      Points* spatialEndPositions = new Points(0);
      ConnectSpatialPositions(targets, tgtEntries, false, spatialEndPositions);
      DbArray<PosJNetSpatial>* srcEntries = new DbArray<PosJNetSpatial> (0);
      ConnectSpatialPositions(sources, srcEntries, true, 0);
      DbArray<JRouteInterval>* sp = 0;
      int tgtPosInArray = -1;
      bool tgtOverStartJunction = false;
      int srcStartPathJID = -1;
      double minDist = numeric_limits< double >::max();
      if (!CheckForSameSections(srcEntries,tgtEntries,sp))
      {
        if (!CheckForExistingNetdistance(srcEntries, tgtEntries,
                                         srcStartPathJID, tgtPosInArray,
                                         tgtOverStartJunction, minDist))
        {
          PQManagement* pqueue = new PQManagement();
          if (InitPriorityQueue(pqueue, srcEntries, spatialEndPositions))
          {
            if (ProcessPriorityQueue(pqueue, tgtEntries, spatialEndPositions,
                                     tgtPosInArray, tgtOverStartJunction,
                                     srcStartPathJID, minDist))
            {
              sp = new DbArray<JRouteInterval>(0);
              WriteShortestPath(srcEntries, tgtEntries, srcStartPathJID,
                                tgtPosInArray, tgtOverStartJunction, sp);
            }
          }
          pqueue->Destroy();
          delete pqueue;
        }
        else
        {
          sp = new DbArray<JRouteInterval>(0);
          WriteShortestPath(srcEntries, tgtEntries, srcStartPathJID,
                            tgtPosInArray, tgtOverStartJunction, sp);
        }
      }
      if (sp != NULL)
      {
        result->SetPath(*sp, false, this);
        result->SetDefined(true);
        sp->Destroy();
        delete sp;
      }
      else
      {
        result->SetDefined(false);
      }
      tgtEntries->Destroy();
      delete tgtEntries;
      srcEntries->Destroy();
      delete srcEntries;
      spatialEndPositions->Destroy();
      spatialEndPositions->DeleteIfAllowed();
    }
  }
  else
  {
    result->SetDefined(false);
  }
}

/*
1.1.1 Netdistance

*/

void JNetwork::Netdistance(const RouteLocation& source,
                           const RouteLocation& target,
                           CcReal* result)
{
  DbArray<RouteLocation>* srcPositions = new DbArray<RouteLocation>(0);
  srcPositions->Append(source);
  DbArray<RouteLocation>* tgtPositions = new DbArray<RouteLocation>(0);
  tgtPositions->Append(target);
  Netdistance(srcPositions, tgtPositions, result);
  srcPositions->Destroy();
  delete srcPositions;
  tgtPositions->Destroy();
  delete tgtPositions;
}

void JNetwork::Netdistance(const RouteLocation& source,
                           const DbArray< RouteLocation >& target,
                           CcReal* result)
{
  DbArray<RouteLocation>* srcPositions = new DbArray<RouteLocation>(0);
  srcPositions->Append(source);
  Netdistance(srcPositions, &target, result);
  srcPositions->Destroy();
  delete srcPositions;
}

void JNetwork::Netdistance(const DbArray< RouteLocation >& source,
                           const RouteLocation& target,
                           CcReal* result)
{
  DbArray<RouteLocation>* tgtPositions = new DbArray<RouteLocation>(0);
  tgtPositions->Append(target);
  Netdistance(&source, tgtPositions, result);
  tgtPositions->Destroy();
  delete tgtPositions;
}


void JNetwork::Netdistance(const DbArray<RouteLocation>* sources,
                           const DbArray<RouteLocation>* targets,
                           CcReal* result)
{
  if (sources != NULL && targets != NULL && result != NULL &&
      sources->Size() > 0 && targets->Size() > 0)
  {
    if (!JNetUtil::ArrayContainIntersections(*sources, *targets))
    {
      DbArray<PosJNetSpatial>* tgtEntries = new DbArray<PosJNetSpatial> (0);
      Points* spatialEndPositions = new Points(0);
      ConnectSpatialPositions(targets, tgtEntries, false, spatialEndPositions);
      DbArray<PosJNetSpatial>* srcEntries = new DbArray<PosJNetSpatial> (0);
      ConnectSpatialPositions(sources, srcEntries, true, 0);
      DbArray<JRouteInterval>* sp = 0;
      int tgtPosInArray = -1;
      bool tgtOverStartJunction = false;
      int srcStartPathJID = -1;
      double minDist = numeric_limits< double >::max();
      if (!CheckForSameSections(srcEntries,tgtEntries,sp))
      {
        if (!CheckForExistingNetdistance(srcEntries, tgtEntries,
                                         srcStartPathJID, tgtPosInArray,
                                         tgtOverStartJunction, minDist))
        {
          PQManagement* pqueue = new PQManagement();
          if (InitPriorityQueue(pqueue, srcEntries, spatialEndPositions))
          {
            if (ProcessPriorityQueue(pqueue, tgtEntries, spatialEndPositions,
                                     tgtPosInArray, tgtOverStartJunction,
                                     srcStartPathJID, minDist))
            {
              WriteNetdistance(srcEntries, tgtEntries, srcStartPathJID,
                               tgtPosInArray, tgtOverStartJunction, result);
            }
          }
          pqueue->Destroy();
          delete pqueue;
        }
        else
        {
          WriteNetdistance(srcEntries, tgtEntries, srcStartPathJID,
                           tgtPosInArray, tgtOverStartJunction, result);
        }
      }
      else
      {
        if (sp != NULL)
        {
          result->SetDefined(true);
          result->Set(getLength(sp));
          sp->Destroy();
          delete sp;
        }
        else
        {
          result->SetDefined(false);
        }
      }
      tgtEntries->Destroy();
      delete tgtEntries;
      srcEntries->Destroy();
      delete srcEntries;
      spatialEndPositions->Destroy();
      spatialEndPositions->DeleteIfAllowed();
    }
    else
    {
      result->SetDefined(true);
      result->Set(0.0);
    }
  }
  else
  {
    result->SetDefined(false);
  }
}

/*
1.1 Private functions

1.1.1 Build ListExpr for Out-Function of Network

*/

ListExpr JNetwork::JunctionsToList() const
{
  return relationToList(junctions, JNetUtil::GetJunctionsRelationTypeInfo());
}

ListExpr JNetwork::SectionsToList() const
{
  return relationToList(sections, JNetUtil::GetSectionsRelationTypeInfo());
}

ListExpr JNetwork::RoutesToList() const
{
  return relationToList(routes, JNetUtil::GetRoutesRelationTypeInfo());
}

ListExpr JNetwork::NetdistancesToList() const
{
  return relationToList(netdistances,
                        JNetUtil::GetNetdistancesRelationTypeInfo());
}


/*
1.1 Creates Trees

*/

void JNetwork::CreateTrees()
{
  junctionsBTree = createBTree(junctions,
                               JNetUtil::GetJunctionsRelationTypeInfo(), "Id");
  junctionsRTree = createRTree(junctions,
                               JNetUtil::GetJunctionsRelationTypeInfo(), "Pos");
  sectionsBTree = createBTree(sections,
                              JNetUtil::GetSectionsRelationTypeInfo(), "Id");
  sectionsRTree = createRTree(sections,
                              JNetUtil::GetSectionsRelationTypeInfo(), "Curve");
  routesBTree = createBTree(routes,
                            JNetUtil::GetRoutesRelationTypeInfo(),"Id");
}

/*
1.1 Initialize netdistance ordered relation

*/

void JNetwork::InitNetdistances()
{
  ListExpr relType;
  nl->ReadFromString(GetNetdistancesRelationType(), relType);
  ListExpr relNumType =
    SecondoSystem::GetCatalog()->NumericType(relType);
  netdistances = new OrderedRelation(relNumType);
  GenericRelationIterator* it = sections->MakeScan();
  Tuple* actTuple = 0;
  while ((actTuple = it->GetNextTuple()) != 0)
  {
    Direction* sectDir = GetSectionDirection(actTuple);
    Direction compD(Down);
    if (sectDir->Compare(compD) != 0)
    {
      InsertNetdistanceTuple(
        ((CcInt*)actTuple->GetAttribute(SEC_STARTNODE_ID))->GetIntval(),
        ((CcInt*)actTuple->GetAttribute(SEC_ENDNODE_ID))->GetIntval(),
        ((CcInt*)actTuple->GetAttribute(SEC_ENDNODE_ID))->GetIntval(),
        ((CcInt*)actTuple->GetAttribute(SEC_ID))->GetIntval(),
        ((CcReal*)actTuple->GetAttribute(SEC_LENGTH))->GetRealval());
    }
    Direction compU(Up);
    if (sectDir->Compare(compU) != 0)
    {
      InsertNetdistanceTuple(
        ((CcInt*)actTuple->GetAttribute(SEC_ENDNODE_ID))->GetIntval(),
        ((CcInt*)actTuple->GetAttribute(SEC_STARTNODE_ID))->GetIntval(),
        ((CcInt*)actTuple->GetAttribute(SEC_STARTNODE_ID))->GetIntval(),
        ((CcInt*)actTuple->GetAttribute(SEC_ID))->GetIntval(),
        ((CcReal*)actTuple->GetAttribute(SEC_LENGTH))->GetRealval());
    }
    actTuple->DeleteIfAllowed();
    actTuple = 0;
    sectDir->DeleteIfAllowed();
  }
  delete it;
  it = junctions->MakeScan();
  while ((actTuple = it->GetNextTuple()) != 0)
  {
    InsertNetdistanceTuple(
      ((CcInt*)actTuple->GetAttribute(JUNC_ID))->GetIntval(),
      ((CcInt*)actTuple->GetAttribute(JUNC_ID))->GetIntval(),
      ((CcInt*)actTuple->GetAttribute(JUNC_ID))->GetIntval(),
      -1, 0.0);
    actTuple->DeleteIfAllowed();
    actTuple = 0;
  }
  delete it;
}

void JNetwork::InsertNetdistanceTuple(const int fromjid,
                                      const JPQEntry* entry,
                                      DbArray<InterSP>* wayEntries)
{
  if (entry != NULL)
  {
    InsertNetdistanceTuple(fromjid, entry);
    if (wayEntries != NULL)
    {
      int i = 0;
      int oldWayEntrySize = wayEntries->Size();
      InterSP curSPEntry;
      while (i < oldWayEntrySize)
      {
        wayEntries->Get(i,curSPEntry);
        if (curSPEntry.GetOrigStartPathJID() == fromjid)
        {
          if (curSPEntry.GetNextJID() < 0)
          {
            curSPEntry.SetNextJID(entry->GetEndPartJID());
            curSPEntry.SetNextSID(entry->GetSectionId());
            wayEntries->Put(i, curSPEntry);
          }
          InsertNetdistanceTuple(curSPEntry.GetCurStartPathJID(),
                                 entry->GetEndPartJID(),
                                 curSPEntry.GetNextJID(),
                                 curSPEntry.GetNextSID(),
                                 entry->GetDistFromStartPoint() -
                                   curSPEntry.GetDistFromOrigStartPath());
        }
        i++;
      }
      wayEntries->Append(InterSP(fromjid, entry->GetEndPartJID(),
                                 entry->GetDistFromStartPoint(),
                                 -1,-1, entry->GetDistStartToStartJID()));
    }
  }
}

void JNetwork::InsertNetdistanceTuple(const int fromjid, const JPQEntry* jp)
{
  if (jp != 0 && jp->GetEndPartJID() > -1 && jp->GetStartNextJID() > -1 &&
      jp->GetStartNextSID() > -1 && jp->GetDistFromStartPoint() >= 0.0 &&
      jp->GetStartNextJID() != jp->GetEndPartJID())
  {
    InsertNetdistanceTuple(fromjid, jp->GetEndPartJID(),jp->GetStartNextJID(),
                           jp->GetStartNextSID(), jp->GetDistFromStartPoint() -
                           jp->GetDistStartToStartJID());
  }
}

void JNetwork::InsertNetdistanceTuple(const int fromjid, const int tojid,
                                      const int viajid, const int viasid,
                                      const double dist)
{
  if (fromjid != tojid && viasid > -1)
  {
    Tuple* existingTuple = GetNetdistanceTupleFor(fromjid, tojid);
    if (existingTuple == 0)
    {
      ListExpr tupType;
      nl->ReadFromString(GetNetdistancesRelationType(), tupType);
      ListExpr tupNumType =
        SecondoSystem::GetCatalog()->NumericType(nl->Second(tupType));
      Tuple* insertTuple = new Tuple(tupNumType);
      insertTuple->PutAttribute(NETDIST_FROM_JID, new CcInt(true, fromjid));
      insertTuple->PutAttribute(NETDIST_TO_JID, new CcInt(true, tojid));
      insertTuple->PutAttribute(NETDIST_NEXT_JID, new CcInt(true, viajid));
      insertTuple->PutAttribute(NETDIST_NEXT_SID, new CcInt(true, viasid));
      insertTuple->PutAttribute(NETDIST_DIST, new CcReal(true, dist));
      netdistances->AppendTuple(insertTuple);
      insertTuple->DeleteIfAllowed();
    }
    else
    {
      double existingDist =
        ((CcReal*)existingTuple->GetAttribute(NETDIST_DIST))->GetRealval();
      if (!AlmostEqualAbsolute(existingDist,dist,tolerance) &&
          existingDist > dist)
      {
        vector<int> indexes(0);
        vector<Attribute *> attrs(0);
        indexes.push_back(NETDIST_NEXT_JID);
        attrs.push_back(new CcInt(true, viajid));
        indexes.push_back(NETDIST_NEXT_SID);
        attrs.push_back(new CcInt(true, viasid));
        indexes.push_back(NETDIST_DIST);
        attrs.push_back(new CcReal(true, dist));
        netdistances->UpdateTuple(existingTuple, indexes, attrs);
        indexes.clear();
        attrs.clear();
      }
      existingTuple->DeleteIfAllowed();
    }
  }
}

/*
1.1 Get Tuples by identifier

*/

Tuple* JNetwork::GetRouteTupleWithId(const int rid) const
{
  if (rid > -1)
    return GetTupleWithId(routesBTree, routes, rid);
  else
    return 0;
}

Tuple* JNetwork::GetSectionTupleWithId(const int sid) const
{
  if (sid > -1)
    return GetTupleWithId(sectionsBTree, sections, sid);
  else
    return 0;
}

Tuple* JNetwork::GetSectionTupleWithTupleId(const TupleId tid) const
{
  return sections->GetTuple(tid, false);
}

Tuple* JNetwork::GetJunctionTupleWithId(const int jid) const
{
  if (jid > -1)
    return GetTupleWithId(junctionsBTree, junctions, jid);
  else
    return 0;
}

Tuple* JNetwork::GetTupleWithId(BTree* tree, const Relation* rel,
                                const int id) const
{
  CcInt* cid = new CcInt(true, id);
  Tuple* res = 0;
  BTreeIterator* it = tree->ExactMatch(cid);
  cid->DeleteIfAllowed();
  if (it->Next() != 0)
    res = rel->GetTuple(it->GetId(), false);
  delete it;
  return res;
}

Tuple* JNetwork::GetNetdistanceTupleFor(const int fid, const int tid) const
{
  CcInt* minNodeId = new CcInt(true,0);
  CcInt* maxNodeId = new CcInt(true,numeric_limits<int>::max());
  vector<void*> attributes(2);
  vector<SmiKey::KeyDataType> kElems(2);
  SmiKey test((long int) 0);
  kElems[0] = test.GetType();
  kElems[1] = test.GetType();
  CcInt* from = new CcInt(true,fid);
  attributes[0] = from;
  attributes[1] = minNodeId;
  CompositeKey lowerKey(attributes,kElems,false);
  attributes[1] = maxNodeId;
  CompositeKey upperKey(attributes,kElems,true);
  OrderedRelationIterator* it =
              (OrderedRelationIterator*) netdistances->MakeRangeScan(lowerKey,
                                                                    upperKey);
  Tuple* res = it->GetNextTuple();
  bool found = false;
  while(res != 0 && !found)
  {
    if (tid == ((CcInt*)res->GetAttribute(NETDIST_TO_JID))->GetIntval())
    {
      found = true;
    }
    else
    {
      res->DeleteIfAllowed();
      res = it->GetNextTuple();
    }
  }
  from->DeleteIfAllowed();
  minNodeId->DeleteIfAllowed();
  maxNodeId->DeleteIfAllowed();
  attributes.clear();
  kElems.clear();
  delete it;
  return res;
}


/*
1.1. Get Tuple by Spatial Position

*/

Tuple* JNetwork::GetSectionTupleFor(const Point* p, double& pos) const
{
  const Rectangle<2> pbox = p->BoundingBox();
  const Rectangle<2> searchbox(true,
                               pbox.MinD(0) - tolerance,
                               pbox.MaxD(0) + tolerance,
                               pbox.MinD(1) - tolerance,
                               pbox.MaxD(1) + tolerance);
  R_TreeLeafEntry<2,TupleId> curEntry;
  Tuple* actSect = 0;
  if (sectionsRTree->First(searchbox, curEntry))
    actSect = sections->GetTuple(curEntry.info, false);
  else
    return 0;
  double diff;
  double mindiff = numeric_limits<double>::max();
  TupleId minTupId;
  bool found = false;
  SimpleLine* actCurve = GetSectionCurve(actSect);
  if (actCurve != 0)
  {
    found = actCurve->AtPoint(*p, pos, tolerance);
    if (!found)
    {
      mindiff = actCurve->Distance(*p);
      minTupId = curEntry.info;
    }
  }
  while (!found && sectionsRTree->Next(curEntry))
  {
    if (actSect != 0) actSect->DeleteIfAllowed();
    actSect = sections->GetTuple(curEntry.info, false);
    if (actSect != 0)
    {
      if (actCurve != 0) actCurve->DeleteIfAllowed();
      actCurve = GetSectionCurve(actSect);
      if (actCurve != 0)
      {
        found = actCurve->AtPoint(*p, pos, tolerance);
        if (!found)
        {
          diff = actCurve->Distance(*p);
          if (diff < mindiff)
          {
            mindiff = diff;
            minTupId = curEntry.info;
          }
        }
      }
    }
  }
  if (!found)
  {
    if (actSect != 0) actSect->DeleteIfAllowed();
    actSect = sections->GetTuple(minTupId, false);
    if (actSect != 0)
    {
      if (actCurve != 0) actCurve->DeleteIfAllowed();
      actCurve = GetSectionCurve(actSect);
      if (actCurve != 0){
        HalfSegment hs;
        double k1, k2;
        Point left, right;
        for ( int i = 0; i < actCurve->Size()-1; i++ )
        {
          actCurve->Get ( i, hs );
          left = hs.GetLeftPoint();
          right = hs.GetRightPoint();
          if (left.IsDefined() && right.IsDefined())
          {

            double xl = left.GetX();
            double yl = left.GetY();
            double xr = right.GetX();
            double yr = right.GetY();
            double x = p->GetX();
            double y = p->GetY();
            if((AlmostEqualAbsolute(x, xl, tolerance) &&
                AlmostEqualAbsolute(y, yl, tolerance)) ||
              (AlmostEqualAbsolute(x, xr, tolerance) &&
                AlmostEqualAbsolute(y, yr, tolerance)))
            {
              diff = 0.0;
              found = true;
            }
            else
            {
              if ( xl != xr && xl != x )
              {
                k1 = ( y - yl ) / ( x - xl );
                k2 = ( yr - yl ) / ( xr - xl );
                if ((( xl < xr &&
                    ( x > xl || AlmostEqualAbsolute(x, xl, tolerance)) &&
                    ( x < xr || AlmostEqualAbsolute(x, xr, tolerance))) ||
                    (xl > xr &&
                    ( x < xl || AlmostEqualAbsolute(x, xl, tolerance)) &&
                    ( x > xr || AlmostEqualAbsolute(x, xr, tolerance)))) &&
                    ((( yl < yr || AlmostEqualAbsolute(yl, yr, tolerance)) &&
                      ( y > yl || AlmostEqualAbsolute(y, yl, tolerance)) &&
                      ( y < yr || AlmostEqualAbsolute(y, yr, tolerance))) ||
                      ( yl > yr &&
                      ( y < yl || AlmostEqualAbsolute(y, yl, tolerance)) &&
                      ( y > yr || AlmostEqualAbsolute(y, yr, tolerance)))))
                {
                  diff = fabs ( k1-k2 );
                  found = true;
                }
                else
                {
                  found = false;
                }
              }
              else
              {
                if((AlmostEqualAbsolute(xl, xr, tolerance) &&
                    AlmostEqualAbsolute(xl, x, tolerance)) &&
                  ((( yl < yr || AlmostEqualAbsolute(yl, yr, tolerance)) &&
                    ( yl < y || AlmostEqualAbsolute(yl, y , tolerance)) &&
                    ( y < yr || AlmostEqualAbsolute(y, yr, tolerance))) ||
                    ( yl > yr  &&
                    ( yl > y || AlmostEqualAbsolute(yl, y, tolerance)) &&
                    ( y > yr || AlmostEqualAbsolute(y, yr, tolerance)))))
                {
                  diff = 0.0;
                  found = true;
                }
                else
                {
                  found = false;
                }
              }
            }
            if ( found )
            {
              LRS lrs;
              actCurve->Get ( hs.attr.edgeno, lrs );
              actCurve->Get ( lrs.hsPos, hs );
              pos = lrs.lrsPos + p->Distance(hs.GetDomPoint());
              if(!actCurve->GetStartSmaller())
                pos = actCurve->Length() - pos;
              if ( pos < 0.0 || AlmostEqualAbsolute( pos, 0.0, tolerance))
                pos = 0.0;
              else if ( pos > actCurve->Length() ||
                AlmostEqualAbsolute(pos, actCurve->Length(), tolerance))
                pos = actCurve->Length();
              break;
            }
          }
        }
      }
    }
  }
  if (actCurve != 0) actCurve->DeleteIfAllowed();
  return actSect;
}

Tuple* JNetwork::GetSectionTupleFor(const RouteLocation& rloc, double& relpos)
const
{
  Tuple* res = 0;
  JListInt* sectList = GetRouteSectionList(rloc.GetRouteId());
  if (sectList != 0)
  {
    int index = -1;
    res = GetSectionTupleFor(rloc, relpos, sectList, index);
    sectList->Destroy();
    sectList->DeleteIfAllowed();
  }
  return res;
}

Tuple* JNetwork::GetSectionTupleFor(const RouteLocation& rloc, double& pos,
                                    const JListInt* sectList,
                                    int& index) const
{
  if (sectList != 0)
  {
    if (sectList->IsDefined() && !sectList->IsEmpty())
    {
      Tuple* actSect = 0;
      CcInt actSid;
      if (index > -1 && index < sectList->GetNoOfComponents())
      {
        sectList->Get(index, actSid);
        actSect = GetSectionTupleWithId(actSid.GetIntval());
        if (actSect != 0)
        {
          if (CheckTupleForRLoc(actSect, rloc, pos))
          return actSect;
        }
      }
      int i = 0;
      while (i < sectList->GetNoOfComponents())
      {
        sectList->Get(i, actSid);
        actSect = GetSectionTupleWithId(actSid.GetIntval());
        if (actSect != 0)
        {
          if (CheckTupleForRLoc(actSect, rloc, pos))
          {
            index = i;
            return actSect;
          }
          actSect->DeleteIfAllowed();
          actSect = 0;
        }
        i++;
      }
    }
  }
  index = -1;
  return 0;
}

void JNetwork::GetSectionTuplesFor(const Rectangle<2> bbox,
                                   vector<TupleId>& listSectTupIds) const
{
  listSectTupIds.clear();
  R_TreeLeafEntry<2,TupleId> curEntry;
  if (sectionsRTree->First(bbox, curEntry))
    listSectTupIds.push_back(curEntry.info);
  while (sectionsRTree->Next(curEntry))
  {
    listSectTupIds.push_back(curEntry.info);
  }
}

/*
1.1.1.1 By parts of Network

*/

void JNetwork::GetCoveredSections(const JRouteInterval& rint,
                                  DbArray<SectionInterval>* result) const
{
  JListInt* sectList = GetRouteSectionList(rint.GetRouteId());
  if (sectList != 0)
  {
    if (sectList->IsDefined() && !sectList->IsEmpty())
    {
       CcInt curSid(false);
      for (int i = 0; i < sectList->GetNoOfComponents(); i++)
      {
        sectList->Get(i,curSid);
         Tuple* sectTuple = GetSectionTupleWithId(curSid.GetIntval());
        if(sectTuple != 0)
        {
           JListRInt* sectRis = GetSectionListRouteIntervals(sectTuple);
          if (sectRis != 0)
          {
             JRouteInterval actInt(false);
            int j = 0;
            while (j < sectRis->GetNoOfComponents())
            {
              sectRis->Get(j,actInt);
               if (rint.Overlaps(actInt, false))
              {
                 SectionInterval actSectInt(curSid.GetIntval(),
                                       JRouteInterval(
                                         rint.GetRouteId(),
                                         max(rint.GetFirstPosition(),
                                             actInt.GetFirstPosition()),
                                         min(rint.GetLastPosition(),
                                             actInt.GetLastPosition()),
                                         actInt.GetSide()),
                                        (actInt.GetFirstPosition() >=
                                          rint.GetFirstPosition()),
                                        (actInt.GetLastPosition() <=
                                          rint.GetLastPosition()));
                 result->Append(actSectInt);
                j = sectRis->GetNoOfComponents();
              }
              j++;
            }
            sectRis->DeleteIfAllowed();
            sectRis = 0;
          }
          sectTuple->DeleteIfAllowed();
          sectTuple = 0;
        }
      }
    }
    sectList->DeleteIfAllowed();
    sectList = 0;
  }
}

/*
1.1 Access to attributes of the relations

1.1.1 Routes Relation Attributes

*/

double JNetwork::GetRouteLength(const int rid) const
{
  Tuple* routeTup = GetRouteTupleWithId(rid);
  double res = GetRouteLength(routeTup);
  routeTup->DeleteIfAllowed();
  return res;
}

double JNetwork::GetRouteLength(const Tuple* routeTuple) const
{
  if (routeTuple != 0)
    return ((CcReal*)routeTuple->GetAttribute(ROUTE_LENGTH))->GetRealval();
  else
    return 0.0;
}

JListInt* JNetwork::GetRouteSectionList(const int rid) const
{
  JListInt* res = 0;
  Tuple* actRouteTup = GetRouteTupleWithId(rid);
  if (actRouteTup != 0)
  {
    res = GetRouteSectionList(actRouteTup);
    actRouteTup->DeleteIfAllowed();
  }
  return res;
}

JListInt* JNetwork::GetRouteSectionList(const Tuple* actRoute) const
{
  if (actRoute != 0)
    return new JListInt(*(
                  (JListInt*) actRoute->GetAttribute(ROUTE_LIST_SECTIONS)));
  else
    return 0;
}

/*
1.1.1 Sections Relation Attributes

*/

SimpleLine* JNetwork::GetSectionCurve(const int sid) const
{
  Tuple* actSect = GetSectionTupleWithId(sid);
  SimpleLine* res = GetSectionCurve(actSect);
  actSect->DeleteIfAllowed();
  return res;
}

SimpleLine* JNetwork::GetSectionCurve(const Tuple* sectTuple) const
{
  if (sectTuple != 0)
    return new SimpleLine(*((SimpleLine*) sectTuple->GetAttribute(SEC_CURVE)));
  else
    return 0;
}

SimpleLine* JNetwork::GetSectionCurve(const RouteLocation& rloc, double& relpos)
const
{
  Tuple* actSect = GetSectionTupleFor(rloc, relpos);
  SimpleLine* res = GetSectionCurve(actSect);
  actSect->DeleteIfAllowed();
  return res;
}

JListRInt* JNetwork::GetSectionListRouteIntervals(const int sectid) const
{
  Tuple* sectTup = GetSectionTupleWithId(sectid);
  JListRInt* res = GetSectionListRouteIntervals(sectTup);
  sectTup->DeleteIfAllowed();
  return res;
}

JListRInt* JNetwork::GetSectionListRouteIntervals(const Tuple* sectTuple) const
{
  if (sectTuple != 0)
    return new JListRInt(*(
      (JListRInt*) sectTuple->GetAttribute(SEC_LIST_ROUTEINTERVALS)));
  else
    return 0;
}

JRouteInterval* JNetwork::GetSectionFirstRouteInterval(const Tuple* sectTuple)
const
{
  JRouteInterval* result = 0;
  if (sectTuple != 0)
  {
    JListRInt* rintList = GetSectionListRouteIntervals(sectTuple);
    if(rintList != 0)
    {
      if (!rintList->IsEmpty())
      {
        JRouteInterval res;
        rintList->Get(0,res);
        result = new JRouteInterval(res);
      }
      rintList->DeleteIfAllowed();
      rintList = 0;
    }
  }
  return result;
}

JRouteInterval* JNetwork::GetSectionRouteIntervalForRID(int rid,
                                                    const Tuple* sectTup) const
{
  if (sectTup != 0)
  {
    JListRInt* rintList = GetSectionListRouteIntervals(sectTup);
    if (rintList != 0)
    {
      if (rintList->IsDefined() && !rintList->IsEmpty())
      {
        JRouteInterval actInt;
        int j = 0;
        while (j < rintList->GetNoOfComponents())
        {
          rintList->Get(j,actInt);
          if (actInt.GetRouteId() == rid)
          {
            rintList->Destroy();
            rintList->DeleteIfAllowed();
            return new JRouteInterval(actInt);
          }
          j++;
        }
      }
      rintList->Destroy();
      rintList->DeleteIfAllowed();
    }
  }
  return 0;
}

JRouteInterval* JNetwork::GetSectionRouteIntervalForRLoc(
                        const RouteLocation& rloc,
                        const Tuple* sectTup) const
{
if (sectTup != 0)
  {
    JListRInt* rintList = GetSectionListRouteIntervals(sectTup);
    if (rintList != 0)
    {
      if (rintList->IsDefined() && !rintList->IsEmpty())
      {
        JRouteInterval actInt;
        int j = 0;
        while (j < rintList->GetNoOfComponents())
        {
          rintList->Get(j,actInt);
          if (actInt.Contains(rloc, tolerance))
          {
            rintList->Destroy();
            rintList->DeleteIfAllowed();
            return new JRouteInterval(actInt);
          }
          j++;
        }
      }
      rintList->Destroy();
      rintList->DeleteIfAllowed();
    }
  }
  return 0;
}

double JNetwork::GetSectionLength(const Tuple* actSect) const
{
  if (actSect != 0)
    return ((CcReal*)actSect->GetAttribute(SEC_LENGTH))->GetRealval();
  else
    return 0.0;
}

Direction* JNetwork::GetSectionDirection(const Tuple* actSect) const
{
  if (actSect != 0)
    return new Direction(*((Direction*)actSect->GetAttribute(SEC_DIRECTION)));
  else
    return 0;
}

int JNetwork::GetSectionId(const Tuple* actSect) const
{
  if (actSect != 0)
    return ((CcInt*)actSect->GetAttribute(SEC_ID))->GetIntval();
  else
    return -1;
}

JRouteInterval* JNetwork::GetRouteIntervalFor(const JListRLoc* leftrlocs,
                                              const JListRLoc* rightrlocs,
                                              const bool allowResetSide) const
{
  JRouteInterval* res = 0;
  if (leftrlocs != 0 && rightrlocs != 0 &&
      leftrlocs->IsDefined() && rightrlocs->IsDefined() &&
      !leftrlocs->IsEmpty() && !rightrlocs->IsEmpty())
  {
    int i = 0;
    int j = 0;
    while (i < leftrlocs->GetNoOfComponents() &&
           j < rightrlocs->GetNoOfComponents())
    {
      RouteLocation left, right;
      leftrlocs->Get(i,left);
      rightrlocs->Get(j,right);
      if (left.IsOnSameRoute(right))
      {
        if (res == 0)
        {
          res = new JRouteInterval(left, right, allowResetSide);
        }
        else
        {
          JRouteInterval* inter =
            new JRouteInterval(left, right, allowResetSide);
          if (inter->GetLength() < res->GetLength())
          {
            res->DeleteIfAllowed();
            res = inter;
          }
          else
          {
            inter->DeleteIfAllowed();
            inter = 0;
          }
        }
        i++;
        j++;
      }
      else
      {
        switch(left.Compare(right))
        {
          case -1:
          {
            i++;
            break;
          }

          case 1:
          {
            j++;
            break;
          }

          default:
          {//should never happen
            assert(false);
            i = leftrlocs->GetNoOfComponents();
            j = rightrlocs->GetNoOfComponents();
            break;
          }
        }
      }
    }
  }
  return res;
}

JRouteInterval* JNetwork::GetRouteIntervalFor(const RouteLocation& left,
                                              const RouteLocation& right,
                                              const bool allowResetSide) const
{
  JListRLoc* leftrlocs = GetNetworkValuesOf(left);
  JListRLoc* rightrlocs = GetNetworkValuesOf(right);
  JRouteInterval* res =
    GetRouteIntervalFor(leftrlocs, rightrlocs, allowResetSide);
  leftrlocs->Destroy();
  leftrlocs->DeleteIfAllowed();
  rightrlocs->Destroy();
  rightrlocs->DeleteIfAllowed();
  return res;
}

void JNetwork::GetAdjacentSections(const int sid,
                                   const jnetwork::Direction* dir,
                                   JListInt* result) const
{
  result->Clear();
  Tuple* sectTuple = GetSectionTupleWithId(sid);
  if (sectTuple != 0)
  {
    result->StartBulkload();
    JListInt* tmp = 0;
    Direction compD(Down);
    int compResult = dir->Compare(compD);
    if ( compResult > -1) //down or both
    {
      tmp = GetSectionListAdjSectionsDown(sectTuple);
      if (tmp != 0)
      {
        result->operator+=(*tmp);
        tmp->Destroy();
        tmp->DeleteIfAllowed();
        tmp = 0;
      }
    }
    if (compResult != 0) //up or both
    {
      tmp = GetSectionListAdjSectionsUp(sectTuple);
      if (tmp != 0)
      {
        result->operator+=(*tmp);
        tmp->Destroy();
        tmp->DeleteIfAllowed();
        tmp = 0;
      }
    }
    result->EndBulkload();
    sectTuple->DeleteIfAllowed();
    sectTuple = 0;
  }
}

void JNetwork::GetReverseAdjacentSections(const int sid,
                                   const jnetwork::Direction* dir,
                                   JListInt* result) const
{
  result->Clear();
  Tuple* sectTuple = GetSectionTupleWithId(sid);
  if (sectTuple != 0)
  {
    result->StartBulkload();
    JListInt* tmp = 0;
    Direction compD(Down);
    int compResult = dir->Compare(compD);
    if ( compResult > -1) //down or both
    {
      tmp = GetSectionListReverseAdjSectionsDown(sectTuple);
      if (tmp != 0)
      {
        result->operator+=(*tmp);
        tmp->Destroy();
        tmp->DeleteIfAllowed();
        tmp = 0;
      }
    }
    if (compResult != 0) //up or both
    {
      tmp = GetSectionListReverseAdjSectionsUp(sectTuple);
      if (tmp != 0)
      {
        result->operator+=(*tmp);
        tmp->Destroy();
        tmp->DeleteIfAllowed();
        tmp = 0;
      }
    }
    result->EndBulkload();
    sectTuple->DeleteIfAllowed();
    sectTuple = 0;
  }
}

JListInt* JNetwork::GetSectionListAdjSectionsUp(const Tuple* sectTuple) const
{
  if (sectTuple != 0)
    return new JListInt(*(
              (JListInt*)sectTuple->GetAttribute(SEC_LIST_ADJ_SECTIONS_UP)));
  else
    return 0;
}

JListInt* JNetwork::GetSectionListAdjSectionsDown(const Tuple* sectTuple) const
{
  if (sectTuple != 0)
    return new JListInt(*(
          (JListInt*)sectTuple->GetAttribute(SEC_LIST_ADJ_SECTIONS_DOWN)));
  else
    return 0;
}

JListInt* JNetwork::GetSectionListReverseAdjSectionsUp(const Tuple* sectTuple)
  const
{
  if (sectTuple != 0)
    return new JListInt(*(
        (JListInt*)sectTuple->GetAttribute(SEC_LIST_REV_ADJ_SECTIONS_UP)));
  else
    return 0;
}

JListInt* JNetwork::GetSectionListReverseAdjSectionsDown(const Tuple* sectTuple)
  const
{
  if (sectTuple != 0)
    return new JListInt(*(
       (JListInt*)sectTuple->GetAttribute(SEC_LIST_REV_ADJ_SECTIONS_DOWN)));
  else
    return 0;
}

Tuple* JNetwork::GetSectionStartJunctionTuple(const Tuple* sectTuple) const
{
  if (sectTuple != 0)
    return GetJunctionTupleWithId(
      ((CcInt*)sectTuple->GetAttribute(SEC_STARTNODE_ID))->GetIntval());
  else
    return 0;
}

Tuple* JNetwork::GetSectionEndJunctionTuple(const Tuple* sectTuple) const
{
  return GetJunctionTupleWithId(
    ((CcInt*)sectTuple->GetAttribute(SEC_ENDNODE_ID))->GetIntval());
}

JListRLoc* JNetwork::GetSectionStartJunctionRLocs(const Tuple* sectTuple) const
{
  JListRLoc* res = 0;
  if (sectTuple != 0)
  {
    Tuple* juncTup = GetSectionStartJunctionTuple(sectTuple);
    res =  GetJunctionListRLoc(juncTup);
    juncTup->DeleteIfAllowed();
  }
  return res;
}

JListRLoc* JNetwork::GetSectionEndJunctionRLocs(const Tuple* sectTuple) const
{
  JListRLoc* res = 0;
  if (sectTuple != 0)
  {
    Tuple* juncTup = GetSectionEndJunctionTuple(sectTuple);
    res = GetJunctionListRLoc(juncTup);
    juncTup->DeleteIfAllowed();
  }
  return res;
}

int JNetwork::GetSectionStartJunctionID(const Tuple* sectTuple) const
{
  if (sectTuple != 0)
    return ((CcInt*)sectTuple->GetAttribute(SEC_STARTNODE_ID))->GetIntval();
  else
    return -1;
}

int JNetwork::GetSectionEndJunctionID(const Tuple* sectTuple) const
{
  if (sectTuple != 0)
    return ((CcInt*)sectTuple->GetAttribute(SEC_ENDNODE_ID))->GetIntval();
  else
    return -1;
}

Point* JNetwork::GetSectionStartPoint(const Tuple* sectTuple) const
{
  Point* res =  0;
  if (sectTuple != 0)
  {
    int jid = GetSectionStartJunctionID(sectTuple);
    if (jid > -1)
    {
      Tuple* juncTuple = GetJunctionTupleWithId(jid);
      if (juncTuple != 0)
      {
        res = GetJunctionSpatialPos(juncTuple);
        juncTuple->DeleteIfAllowed();
      }
    }
  }
  return res;
}

Point* JNetwork::GetSectionEndPoint(const Tuple* sectTuple) const
{
  Point* res =  0;
  if (sectTuple != 0)
  {
    int jid = GetSectionEndJunctionID(sectTuple);
    if (jid > -1)
    {
      Tuple* juncTuple = GetJunctionTupleWithId(jid);
      if (juncTuple != 0)
      {
        res = GetJunctionSpatialPos(juncTuple);
        juncTuple->DeleteIfAllowed();
      }
    }
  }
  return res;
}

double JNetwork::GetSectionMaxSpeed(const Tuple* sectTuple) const
{
  if (sectTuple != 0)
    return ((CcReal*)sectTuple->GetAttribute(SEC_VMAX))->GetRealval();
  else
    return 0.0;
}

/*
1.1.1 Juctions Attributes

*/

int JNetwork::GetJunctionId(const Tuple* juncTup) const
{
  if (juncTup != 0)
    return ((CcInt*)juncTup->GetAttribute(JUNC_ID))->GetIntval();
  else
    return -1;
}

JListRLoc* JNetwork::GetJunctionListRLoc(const Tuple* juncTuple) const
{
  if (juncTuple != 0)
    return new JListRLoc(*((JListRLoc*)
                          juncTuple->GetAttribute(JUNC_LIST_ROUTEPOSITIONS)));
  else
    return 0;
}

Point* JNetwork::GetJunctionSpatialPos(const Tuple* juncTuple) const
{
  if (juncTuple != 0)
    return new Point(*((Point*) juncTuple->GetAttribute(JUNC_POS)));
  else
    return 0;
}

JListInt* JNetwork::GetJunctionOutSectionList(const Tuple* juncTup) const
{
  if (juncTup != 0)
    return new JListInt(*(
      (JListInt*) juncTup->GetAttribute(JUNC_LIST_OUTSECTIONS)));
  else
    return 0;
}

JListInt* JNetwork::GetJunctionInSectionList(const Tuple* juncTup) const
{
  if (juncTup != 0)
    return new JListInt(*(
      (JListInt*) juncTup->GetAttribute(JUNC_LIST_INSECTIONS)));
  else
    return 0;
}

/*
1.1.1 Netdistance Relation

*/

int JNetwork::GetNetdistanceNextSID(const Tuple* actNetDistTup) const
{
  if (actNetDistTup != 0)
    return ((CcInt*)actNetDistTup->GetAttribute(NETDIST_NEXT_SID))->GetIntval();
  else
    return -1;
}

double JNetwork::GetNetdistanceDistance(const Tuple* netDistTup) const
{
  if (netDistTup != NULL)
  {
    return ((CcReal*)netDistTup->GetAttribute(NETDIST_DIST))->GetRealval();
  }
  else
    return numeric_limits< double >::max();
}


/*
1.1 DirectConnectionExists

*/

JRouteInterval* JNetwork::DirectConnection(const int sectId,
                                           const RouteLocation& source,
                                           const RouteLocation& target) const
{
  JRouteInterval* res = 0;
  Tuple* sectTup = GetSectionTupleWithId(sectId);
  if (sectTup != NULL)
  {
    Direction* sectDir = GetSectionDirection(sectTup);
    if (sectDir != NULL)
    {
      Direction moveDir(Both);
      if (source.GetRouteId() == target.GetRouteId())
      {
        if (source.GetPosition() <= target.GetPosition())
          moveDir.SetDirection((Direction)Up);
        else
          moveDir.SetDirection((Direction)Down);
        if (sectDir->SameSide(moveDir, false))
        {
          res = new JRouteInterval(source.GetRouteId(), source.GetPosition(),
                                  target.GetPosition(), moveDir);
        }
      }
      sectDir->DeleteIfAllowed();
    }
    sectTup->DeleteIfAllowed();
  }
  return res;
}

bool JNetwork::DirectConnectionExists(const int startSID,
                                      const int endSID,
                                      const Tuple* sourceSectTup,
                                      const Tuple* targetSectTup,
                                      const RouteLocation& source,
                                      const RouteLocation& target,
                                      DbArray<JRouteInterval>* res,
                                      double& length) const
{
  if (sourceSectTup != 0 && targetSectTup != 0)
  {
    if (source.GetRouteId() == target.GetRouteId())
    {
      Direction* sectDir = GetSectionDirection(sourceSectTup);
      Direction dirU(Up);
      Direction dirD(Down);
      Direction movDir(Both);
      JRouteInterval* tmp = 0;
      if (source.GetPosition() > target.GetPosition())
        movDir = dirD;
      else
        movDir = dirU;
      if (startSID == endSID && sectDir->SameSide(movDir, false))
      {
        tmp = new JRouteInterval(source.GetRouteId(), source.GetPosition(),
                                target.GetPosition(), movDir);
      }
      else
      {
        JListInt* sectlist = GetRouteSectionList(source.GetRouteId());
        CcInt curSid;
        for (int i = 0; i < sectlist->GetNoOfComponents(); i++)
        {
          sectlist->Get(i,curSid);
          Tuple* actSect = GetSectionTupleWithId(curSid.GetIntval());
          JListRInt* sectRis = GetSectionListRouteIntervals(actSect);
          JRouteInterval actInt;
          int j = 0;
          while (j < sectRis->GetNoOfComponents())
          {
            sectRis->Get(j,actInt);
            if (actInt.GetRouteId() == source.GetRouteId() &&
                (actInt.Between(source,target) ||
                 actInt.Contains(source) ||
                 actInt.Contains(target)) &&
                !movDir.SameSide(actInt.GetSide(), false))
            {
              sectRis->Destroy();
              sectRis->DeleteIfAllowed();
              sectlist->Destroy();
              sectlist->DeleteIfAllowed();
              actSect->DeleteIfAllowed();
              sectDir->DeleteIfAllowed();
              return false;
            }
            j++;
          }
          sectRis->Destroy();
          sectRis->DeleteIfAllowed();
          actSect->DeleteIfAllowed();
        }
        sectlist->Destroy();
        sectlist->DeleteIfAllowed();
        tmp = new JRouteInterval(source.GetRouteId(),
                                min(source.GetPosition(), target.GetPosition()),
                                max(source.GetPosition(), target.GetPosition()),
                                movDir);
      }
      res->Append(*tmp);
      length += tmp->GetLength();
      tmp->DeleteIfAllowed();
      sectDir->DeleteIfAllowed();
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

/*
1.1.1.1 AddAdjacentSections

*/

template <class SpatialPos>
void JNetwork::AddAdjacentSections(PQManagement* pq, JPQEntry curEntry,
                                   const SpatialPos* targetPos)
{
  if (pq !=  0)
  {
    Tuple* pqSectTup = GetSectionTupleWithId(curEntry.GetSectionId());
    if (pqSectTup != 0)
    {
      Direction movDir = curEntry.GetDirection();
      JListInt* listSID = 0;
      Direction compU(Up);
      if (movDir.Compare(compU) == 0)
        listSID = GetSectionListAdjSectionsUp(pqSectTup);
      else
        listSID = GetSectionListAdjSectionsDown(pqSectTup);
      if (listSID != 0)
      {
        AddAdjacentSections<SpatialPos>(pq, listSID, curEntry, targetPos);
        listSID->Destroy();
        listSID->DeleteIfAllowed();
        listSID = 0;
      }
      pqSectTup->DeleteIfAllowed();
      pqSectTup = 0;
    }

  }
}

template<class SpatialPos>
void JNetwork::AddAdjacentSections(PQManagement* pq, const JListInt* listSID,
                                   JPQEntry curEntry,
                                   const SpatialPos* targetPos)
{
  if (pq != 0 && listSID != 0)
  {
    Tuple* curSectTup = 0;
    CcInt nextSID;
    JPQEntry* tmpEntry= 0;
    for (int i = 0; i < listSID->GetNoOfComponents(); i++)
    {
      listSID->Get(i,nextSID);
      int curSID = nextSID.GetIntval();
      tmpEntry = new JPQEntry(curEntry);
      if (curEntry.GetSectionId() < 0 && curEntry.GetStartNextSID() < 0)
      {
        tmpEntry->SetSectionId(curSID);
        tmpEntry->SetStartNextSID(curSID);
      }
      tmpEntry->SetSectionId(curSID);
      curSectTup = GetSectionTupleWithId(nextSID.GetIntval());
      if (curSectTup != 0)
      {
        int sectEndJuncId = GetSectionEndJunctionID(curSectTup);
        int sectStartJuncId = GetSectionStartJunctionID(curSectTup);
        if (sectEndJuncId == tmpEntry->GetEndPartJID())
        {
          tmpEntry->SetDirection((Direction) Down);
          tmpEntry->SetStartPartJID(sectEndJuncId);
          tmpEntry->SetEndPartJID(sectStartJuncId);
        }
        else
        {
          tmpEntry->SetDirection((Direction) Up);
          tmpEntry->SetStartPartJID(sectStartJuncId);
          tmpEntry->SetEndPartJID(sectEndJuncId);
        }
        Tuple* curJunc = GetJunctionTupleWithId(tmpEntry->GetEndPartJID());
        if (curJunc != 0)
        {
          Point* curEndPoint = GetJunctionSpatialPos(curJunc);
          if (curEndPoint != 0)
          {
            double curDist = curEntry.GetDistFromStartPoint() +
                                      GetSectionLength(curSectTup);
            tmpEntry->SetDistFromStartPoint(curDist);
            if (targetPos != NULL)
              tmpEntry->SetPriority(curDist+targetPos->Distance(*curEndPoint));
            else
              tmpEntry->SetPriority(curDist);
            pq->Insert(*tmpEntry);
            curEndPoint->DeleteIfAllowed();
            curEndPoint = 0;
          }
          curJunc->DeleteIfAllowed();
          curJunc = 0;
        }
        curSectTup->DeleteIfAllowed();
        curSectTup = 0;
      }
      delete tmpEntry;
      tmpEntry = 0;
    }
  }
}

void JNetwork::AddReverseAdjacentSections(PQManagement* pq, JPQEntry curEntry)
{
  if (pq !=  0)
  {
    Tuple* pqSectTup = GetSectionTupleWithId(curEntry.GetSectionId());
    if (pqSectTup != 0)
    {
      Direction movDir = curEntry.GetDirection();
      JListInt* listSID = 0;
      Direction compU(Up);
      if (movDir.Compare(compU) == 0)
        listSID = GetSectionListReverseAdjSectionsUp(pqSectTup);
      else
        listSID = GetSectionListReverseAdjSectionsDown(pqSectTup);
      if (listSID != 0)
      {
        AddReverseAdjacentSections(pq, listSID, curEntry);
        listSID->Destroy();
        listSID->DeleteIfAllowed();
        listSID = 0;
      }
      pqSectTup->DeleteIfAllowed();
      pqSectTup = 0;
    }

  }
}

void JNetwork::AddReverseAdjacentSections(PQManagement* pq,
                                          const JListInt* listSID,
                                          JPQEntry curEntry)
{
  if (pq != 0 && listSID != 0)
  {
    Tuple* curSectTup = 0;
    CcInt nextSID;
    JPQEntry* tmpEntry= 0;
    for (int i = 0; i < listSID->GetNoOfComponents(); i++)
    {
      listSID->Get(i,nextSID);
      int curSID = nextSID.GetIntval();
      tmpEntry = new JPQEntry(curEntry);
      if (curEntry.GetSectionId() < 0 && curEntry.GetStartNextSID() < 0)
      {
        tmpEntry->SetSectionId(curSID);
        tmpEntry->SetStartNextSID(curSID);
      }
      tmpEntry->SetSectionId(curSID);
      curSectTup = GetSectionTupleWithId(curSID);
      if (curSectTup != 0)
      {
        int sectEndJuncId = GetSectionEndJunctionID(curSectTup);
        int sectStartJuncId = GetSectionStartJunctionID(curSectTup);
        if (sectEndJuncId == tmpEntry->GetEndPartJID())
        {
          tmpEntry->SetDirection((Direction) Up);
          tmpEntry->SetStartPartJID(sectEndJuncId);
          tmpEntry->SetEndPartJID(sectStartJuncId);
        }
        else
        {
          tmpEntry->SetDirection((Direction) Down);
          tmpEntry->SetStartPartJID(sectStartJuncId);
          tmpEntry->SetEndPartJID(sectEndJuncId);
        }
        Tuple* curJunc = GetJunctionTupleWithId(tmpEntry->GetEndPartJID());
        if (curJunc != 0)
        {
          Point* curEndPoint = GetJunctionSpatialPos(curJunc);
          if (curEndPoint != 0)
          {
            double curDist = curEntry.GetDistFromStartPoint() +
                                      GetSectionLength(curSectTup);
            tmpEntry->SetDistFromStartPoint(curDist);
            tmpEntry->SetPriority(curDist);
            pq->Insert(*tmpEntry);
            curEndPoint->DeleteIfAllowed();
            curEndPoint = 0;
          }
          curJunc->DeleteIfAllowed();
          curJunc = 0;
        }
        curSectTup->DeleteIfAllowed();
        curSectTup = 0;
      }
      delete tmpEntry;
      tmpEntry = 0;
    }
  }
}


/*
1.1.1.1 WriteShortestPath

*/

void JNetwork::WriteShortestPath(const DbArray< PosJNetSpatial >* sources,
                                 const DbArray< PosJNetSpatial >* targets,
                                 const int srcStartPathJID,
                                 const int tgtPosInArray,
                                 const bool tgtOverStartJunction,
                                 DbArray<JRouteInterval>*& result)
{
  if (sources != NULL && targets != NULL && result != NULL &&
      sources->Size() > 0 && targets->Size() > 0 &&
      targets->Size() > tgtPosInArray)
  {
    JRouteInterval* curRint = 0;
    JRouteInterval* tmp = 0;
    int srcPosInArray(-1);
    bool srcOverStartJunc(false);
    PosJNetSpatial start, end;
    Tuple* sectTup = 0;
    getStartPosJNet(sources, srcStartPathJID, srcPosInArray, srcOverStartJunc,
                    start);
    int startJID = srcStartPathJID;
    if (start.GetSectionId() > -1)
    {
      sectTup = GetSectionTupleWithId(start.GetSectionId());
      if (sectTup != NULL)
      {
        curRint = GetSectionFirstRouteInterval(sectTup);
        if (curRint != NULL)
        {
          if (srcOverStartJunc)
          {
            if (start.GetDistFromStartJunction() >= 0.0)
            {
              tmp = new JRouteInterval(curRint->GetRouteId(),
                                     curRint->GetFirstPosition(),
                                     curRint->GetFirstPosition() +
                                     start.GetDistFromStartJunction(), Down);
            }
            startJID = start.GetStartJID();
          }
          else
          {
            if (start.GetDistFromEndJunction() >= 0.0)
            {
              tmp = new JRouteInterval(curRint->GetRouteId(),
                                     curRint->GetLastPosition() -
                                       start.GetDistFromEndJunction(),
                                     curRint->GetLastPosition(), Up);
            }
            startJID = start.GetEndJID();
          }
          if (tmp != NULL)
          {
            if (tmp->GetLength() > 0.0)
            {
              result->Append(*tmp);
            }
            tmp->DeleteIfAllowed();
            tmp = 0;
          }
          curRint->DeleteIfAllowed();
          curRint = 0;
        }
        sectTup->DeleteIfAllowed();
        sectTup = 0;
      }
    }
    bool found = false;
    targets->Get(tgtPosInArray,end);
    if (srcStartPathJID == end.GetStartJID() ||
        srcStartPathJID == end.GetEndJID())
    {
      found = true;
    }
    Tuple* netDistTup = 0;
    int endPathJID;
    if (tgtOverStartJunction)
      endPathJID = end.GetStartJID();
    else
      endPathJID = end.GetEndJID();
    while (!found)
    {
      netDistTup = GetNetdistanceTupleFor(startJID, endPathJID);
      if (netDistTup != NULL)
      {
        sectTup = GetSectionTupleWithId(GetNetdistanceNextSID(netDistTup));
        if (sectTup != NULL)
        {
          curRint = GetSectionFirstRouteInterval(sectTup);
          if (curRint != NULL)
          {
            if (startJID == GetSectionStartJunctionID(sectTup))
            {
              curRint->SetSide((Direction) Up);
              startJID = GetSectionEndJunctionID(sectTup);
            }
            else
            {
              curRint->SetSide((Direction) Down);
              startJID = GetSectionStartJunctionID(sectTup);
            }
            result->Append(*curRint);
            if (endPathJID == startJID)
            {
              found = true;
            }
            curRint->DeleteIfAllowed();
            curRint = 0;
          }
          sectTup->DeleteIfAllowed();
          sectTup = 0;
        }
        netDistTup->DeleteIfAllowed();
        netDistTup = 0;
      }
    }
    targets->Get(tgtPosInArray, end);
    if (end.GetSectionId() > -1)
    {
      sectTup = GetSectionTupleWithId(end.GetSectionId());
      if (sectTup != NULL)
      {
        curRint = GetSectionFirstRouteInterval(sectTup);
        if (curRint != NULL)
        {
          if (tgtOverStartJunction)
          {
            if (end.GetDistFromStartJunction() >= 0.0)
            {
              tmp = new JRouteInterval(curRint->GetRouteId(),
                                     curRint->GetFirstPosition(),
                                     curRint->GetFirstPosition() +
                                       end.GetDistFromStartJunction(), Up);
            }
          }
          else
          {
            if (end.GetDistFromEndJunction() >= 0.0)
            {
              tmp = new JRouteInterval(curRint->GetRouteId(),
                                     curRint->GetLastPosition(),
                                     curRint->GetLastPosition() -
                                       end.GetDistFromEndJunction(), Down);
            }
          }
          if (tmp != NULL)
          {
            if (tmp->GetLength() > 0.0)
            {
              result->Append(*tmp);
            }
            tmp->DeleteIfAllowed();
            tmp = 0;
          }
          curRint->DeleteIfAllowed();
          curRint = 0;
        }
        sectTup->DeleteIfAllowed();
        sectTup = 0;
      }
    }
  }
}

/*
1.1.1.1 WriteNetdistance

*/

void JNetwork::WriteNetdistance(const DbArray< PosJNetSpatial >* sources,
                                const DbArray< PosJNetSpatial >* targets,
                                const int srcStartPathJID,
                                const int tgtPosInArray,
                                const bool tgtOverStartJunction,
                                CcReal* res)
{
  if (sources != NULL && targets != NULL && res != NULL &&
      sources->Size() > 0 && targets->Size() > 0 &&
      targets->Size() > tgtPosInArray)
  {
    double result = 0.0;
    res->SetDefined(true);
    int srcPosInArray(-1);
    bool srcOverStartJunc(false);
    PosJNetSpatial start, end;
    getStartPosJNet(sources, srcStartPathJID, srcPosInArray, srcOverStartJunc,
                    start);
    if (srcOverStartJunc)
      result = result + start.GetDistFromStartJunction();
    else
      result = result + start.GetDistFromEndJunction();
    targets->Get(tgtPosInArray, end);
    int endPathJID;
    if (tgtOverStartJunction)
    {
      result = result + end.GetDistFromStartJunction();
      endPathJID = end.GetStartJID();
    }
    else
    {
      result = result + end.GetDistFromEndJunction();
      endPathJID = end.GetEndJID();
    }
    if (srcStartPathJID != endPathJID)
    {
      Tuple* netDistTup = GetNetdistanceTupleFor(srcStartPathJID, endPathJID);
      if (netDistTup != NULL)
      {
        result = result + GetNetdistanceDistance(netDistTup);
        netDistTup->DeleteIfAllowed();
        netDistTup = 0;
      }
    }
    res->Set(result);
  }
  else
    res->SetDefined(false);
}

/*
1.1.1 ExistsCommonRoute

*/

bool JNetwork::ExistsCommonRoute(RouteLocation& src, RouteLocation& tgt) const
{
  if (!src.IsDefined() || !tgt.IsDefined())
  {
    return false;
  }
  if (src.IsOnSameRoute(tgt))
  {
    return true;
  }
  else
  {
    JListRLoc* left = GetNetworkValuesOf(src);
    if (left != 0)
    {
      JListRLoc* right = GetNetworkValuesOf(tgt);
      if (right != 0)
      {
        if (left->GetNoOfComponents() > 0 && right->GetNoOfComponents() > 0)
        {
          int i = 0;
          while (i < left->GetNoOfComponents())
          {
            left->Get(i,src);
            int j = 0;
            while (j < right->GetNoOfComponents())
            {
              right->Get(j,tgt);
              if (src.IsOnSameRoute(tgt))
              {
                left->Destroy();
                left->DeleteIfAllowed();
                right->Destroy();
                right->DeleteIfAllowed();
                return true;
              }
              j++;
            }
            i++;
          }
        }
        right->Destroy();
        right->DeleteIfAllowed();
      }
      left->Destroy();
      left->DeleteIfAllowed();
    }
    return false;
  }
}

/*
1.1.1 GetRLocOfPosOnRouteInterval

*/

RouteLocation* JNetwork::GetRLocOfPosOnRouteInterval(
    const JRouteInterval* actInt, const double pos) const
{
  if (actInt != 0)
  {
    Direction compD(Down);
    if (actInt->GetSide().Compare(compD) != 0)
      return new RouteLocation(actInt->GetRouteId(),
                               actInt->GetFirstPosition() + pos,
                               actInt->GetSide());
    else
      return new RouteLocation(actInt->GetRouteId(),
                               actInt->GetLastPosition() - pos,
                               actInt->GetSide());
  }
  else
    return 0;
}

/*
1.1.1 Split Junit

*/

void JNetwork::SplitJUnit(const JUnit& ju,
                          JRouteInterval*& lastRint,
                          SimpleLine*& lastRouteCurve,
                          bool& endTimeCorrected,
                          Instant& lastEndTime,
                          Point*& lastEndPoint,
                          LRS& lrs, int& lrspos,
                          MPoint& result) const
{
  if (ju.IsDefined())
  {
    JRouteInterval jurint = ju.GetRouteInterval();
    Interval<Instant> jutime = ju.GetTimeInterval();
    Instant instInter1 = jutime.start;
    Instant instInter2 = jutime.end;
    Direction compD(Down);
    if (lrspos < 0 || jutime.start > lastEndTime)
    {
      lastEndPoint->DeleteIfAllowed();
      lastEndPoint = 0;
    }
    const Instant TIMECORRECTION(0,1, durationtype);
    if (endTimeCorrected)
    {
      endTimeCorrected = false;
      if (instInter1 < lastEndTime)
        instInter1 = lastEndTime;
      if (instInter1 >= instInter2)
      {
        endTimeCorrected = true;
        instInter2 = instInter1 + TIMECORRECTION;
      }
    }
    if (lastEndPoint == 0)
    {
      lastEndPoint = GetSpatialValueOf(jurint.GetStartLocation(), lastRint,
                                       lastRouteCurve, lrs, lrspos);
    }
    if (AlmostEqual(jurint.GetLength(), 0.0))
    {
      checkEndTimeCorrected (endTimeCorrected, instInter1,
                             instInter2, TIMECORRECTION);

      result.Add(UPoint(Interval<Instant>(instInter1, instInter2,
                                          jutime.lc, jutime.rc),
                        *lastEndPoint, *lastEndPoint));
      lastEndTime = instInter2;
    }
    else if(jurint.GetRouteId() == lastRint->GetRouteId())
    {
      addUnitsToResult(ju, lastRouteCurve, tolerance, lrs, lrspos,
                       endTimeCorrected, instInter1, instInter2, TIMECORRECTION,
                       lastEndPoint, lastEndTime, result);
    }
    else
    {
      if (lastEndPoint != 0) lastEndPoint->DeleteIfAllowed();
      lastEndPoint =  GetSpatialValueOf(jurint.GetStartLocation(),
                                        lastRint, lastRouteCurve, lrs, lrspos);
      addUnitsToResult(ju, lastRouteCurve, tolerance, lrs, lrspos,
                       endTimeCorrected, instInter1, instInter2, TIMECORRECTION,
                       lastEndPoint, lastEndTime, result);
    }
  }
}

void JNetwork::SplitJUnit(const JUnit& ju, int& curRid,
                          JRouteInterval*& lastRint,
                          JListInt*& routeSectList,
                          int& lastRouteSecListIndex,
                          bool& endTimeCorrected, Instant& lastEnd,
                          SimpleLine*& lastCurve, MPoint& result) const
{
  if (ju.IsDefined())
  {
    JRouteInterval jurint = ju.GetRouteInterval();
    Interval<Instant> jutime = ju.GetTimeInterval();
    SimpleLine resLine(0);
    Point* startP = 0;
    Point* endP = 0;
    double spos = 0.0;
    double epos = 0.0;
    if(lastRint != 0 && lastRint->IsDefined() && lastRint->Contains(jurint))
    {
      spos = correctedPos(fabs(jurint.GetStartPosition() -
                               lastRint->GetFirstPosition()),
                          lastCurve->Length(),tolerance);
      epos = correctedPos(fabs(jurint.GetEndPosition() -
                               lastRint->GetFirstPosition()),
                          lastCurve->Length(), tolerance);
      lastCurve->SubLine(min(spos, epos), max(spos,epos), resLine);
      startP = new Point(false);
      lastCurve->AtPosition(spos, *startP);
      endP = new Point(false);
      lastCurve->AtPosition(epos, *endP);
    }
    else
    {
      if(lastRint != 0 && lastRint->IsDefined() &&
         lastRint->Contains(jurint.GetStartLocation()))
      {
        spos = correctedPos(fabs(jurint.GetStartPosition() -
                                lastRint->GetFirstPosition()),
                           lastCurve->Length(), tolerance);
        startP = new Point(false);
        lastCurve->AtPosition(spos, *startP);
      }
      else
      {
        startP = GetSpatialValueOf(jurint.GetStartLocation(), curRid,
                                   routeSectList, lastRouteSecListIndex,
                                   lastRint, lastCurve);
      }
      int startIndex = lastRouteSecListIndex;
      if (lastRint != 0 && lastRint->IsDefined() &&
          lastRint->Contains(jurint.GetEndLocation()))
      {
        epos = correctedPos(fabs(jurint.GetEndPosition() -
                                lastRint->GetFirstPosition()),
                           lastCurve->Length(), tolerance);
        endP = new Point(false);
        lastCurve->AtPosition(epos, *endP);
      }
      else
      {
        endP = GetSpatialValueOf(jurint.GetEndLocation(), curRid, routeSectList,
                                 lastRouteSecListIndex, lastRint, lastCurve);
      }
      int lastIndex = lastRouteSecListIndex;
      GetSpatialValueOf(jurint, routeSectList, startIndex, lastIndex, resLine);
    }
    if (startP != 0 && endP != 0 && startP->IsDefined() && endP->IsDefined())
    {
      Point interP1 = *startP;
      Point interP2 = *endP;
      Instant instInter1 = jutime.start;
      Instant instInter2 = jutime.end;
      const Instant TIMECORRECTION(0,1, durationtype);
      if (endTimeCorrected)
      {
        endTimeCorrected = false;
        instInter1 = lastEnd;
        if (instInter1 >= instInter2)
        {
          endTimeCorrected = true;
          instInter2 = instInter1 + TIMECORRECTION;
        }
      }
      if (*startP == *endP)
      {
        result.Add(UPoint(Interval<Instant> (instInter1, instInter2,
                                             true, false),
                          *startP, *endP));
      }
      else
      {
        if (resLine.IsDefined() && !resLine.IsEmpty() && resLine.Size() > 2)
        {
          LRS lrs;
          HalfSegment hs;
          double actDist = 0.0;
          if ((*startP <= *endP && resLine.StartsSmaller()) ||
              (*startP > *endP && !resLine.StartsSmaller()))
          {
            int lrsIndex = 0;
            LRS lrsA(0.0, 0);
            resLine.Get(lrsA, lrsIndex);
            bool end = false;
            while(lrsIndex < resLine.Size()/2 && !end)
            {
              resLine.Get(lrsIndex, lrs);
              resLine.Get(lrs.hsPos, hs);
              actDist += hs.Length();
              interP2 = hs.AtPosition(hs.Length());
              instInter2 = ju.TimeAtPos(actDist);
              checkEndTimeCorrected (endTimeCorrected, instInter1, instInter2,
                                     TIMECORRECTION);
              result.Add(UPoint(Interval<Instant> (instInter1, instInter2,
                                            true, false),
                                interP1, interP2));
              interP1 = interP2;
              instInter1 = instInter2;
              end = (instInter2 >= jutime.end);
              lrsIndex++;
            }
          }
          else
          {
            if ((*startP <= *endP && !resLine.StartsSmaller()) ||
                (*startP > *endP && resLine.StartsSmaller()))
            {
              int lrsIndex = resLine.Size()/2 -1;
              bool end = false;
              actDist = resLine.Length();
              LRS lrsA(actDist,0);
              resLine.Get(lrsA, lrsIndex);
              while (lrsIndex >= 0 && !end)
              {
                resLine.Get(lrsIndex, lrs);
                resLine.Get(lrs.hsPos, hs);
                actDist -= hs.Length();
                interP2 = hs.AtPosition(0.0);
                instInter2 = ju.TimeAtPos(actDist);
                checkEndTimeCorrected (endTimeCorrected, instInter1, instInter2,
                                     TIMECORRECTION);
                result.Add(UPoint(Interval<Instant> (instInter1, instInter2,
                                            true, false),
                                  interP1, interP2));
                interP1 = interP2;
                instInter1 = instInter2;
                end = (instInter2 >= jutime.end);
                lrsIndex--;
              }
            }
          }
        }
        else if(resLine.IsDefined() && !resLine.IsEmpty())
        {
          checkEndTimeCorrected (endTimeCorrected, instInter1, instInter2,
                                     TIMECORRECTION);
          result.Add(UPoint(Interval<Instant> (instInter1, instInter2,
                                             true, false),
                            *startP, *endP));
        }
      }
      startP->DeleteIfAllowed();
      endP->DeleteIfAllowed();
      lastEnd = instInter2;
    }
  }
}



bool JNetwork::CheckTupleForRLoc(const Tuple* actSect,
                                 const RouteLocation& rloc,
                                 double& pos) const
{
  if (actSect != 0)
  {
    JRouteInterval* actInt = GetSectionRouteIntervalForRLoc(rloc, actSect);
    if (actInt != 0)
    {
      if (actInt->Contains(rloc, tolerance))
      {
        pos =
          correctedPos(fabs(rloc.GetPosition() - actInt->GetFirstPosition()),
                       actInt->GetLength(),
                       tolerance);
        actInt->DeleteIfAllowed();
        return true;
      }
      actInt->DeleteIfAllowed();
    }
  }
  return false;
}

/*
1.1.1. ConnectSpatialPositions

*/

void JNetwork::ConnectSpatialPositions(const DbArray<RouteLocation>* targets,
                                      DbArray<PosJNetSpatial>* tgtEntries,
                                      const bool start,
                                      Points* spatialPositions /*=0*/)
{
  if (targets != NULL && tgtEntries != NULL && targets->Size() > 0)
  {
    tgtEntries->clean();
    if (spatialPositions != 0)
    {
      spatialPositions->Clear();
      spatialPositions->StartBulkLoad();
    }
    RouteLocation rloc;
    for (int i = 0; i < targets->Size(); i ++)
    {
      targets->Get(i, rloc);
      double distFromStart;
      Tuple* sectTup = GetSectionTupleFor(rloc, distFromStart);
      if (sectTup != NULL)
      {
        Point* p = GetSpatialValueOf(rloc, distFromStart, sectTup);
        Direction* sectDir = GetSectionDirection(sectTup);
        if (sectDir != NULL)
        {
          if (p != NULL)
          {
            switch(sectDir->GetDirection())
            {
              case Up:
              {
                if (spatialPositions != 0)
                  spatialPositions->operator+=(*p);
                if (start)
                {
                  if (AlmostEqualAbsolute(distFromStart, 0.0, tolerance))
                  {
                    tgtEntries->Append(PosJNetSpatial(rloc, p,
                                           GetSectionId(sectTup),
                                           GetSectionStartJunctionID(sectTup),
                                           0.0,
                                           GetSectionEndJunctionID(sectTup),
                                           GetSectionLength(sectTup)));
                  }
                  else
                  {
                    tgtEntries->Append(PosJNetSpatial(rloc, p,
                                           GetSectionId(sectTup),
                                           -1,
                                           -1000.0,
                                           GetSectionEndJunctionID(sectTup),
                                           GetSectionLength(sectTup)-
                                           distFromStart));
                  }

                }
                else
                {
                  if (AlmostEqualAbsolute(GetSectionLength(sectTup),
                                          distFromStart,
                                          tolerance))
                  {
                    tgtEntries->Append(PosJNetSpatial(rloc, p,
                                           GetSectionId(sectTup),
                                           GetSectionStartJunctionID(sectTup),
                                           GetSectionLength(sectTup),
                                           GetSectionEndJunctionID(sectTup),
                                           0.0));
                  }
                  else
                  {
                    tgtEntries->Append(PosJNetSpatial(rloc, p,
                                           GetSectionId(sectTup),
                                           GetSectionStartJunctionID(sectTup),
                                           distFromStart, -1, -1000.0));
                  }

                }
                break;
              }

              case Down:
              {
                if (spatialPositions != 0)
                  spatialPositions->operator+=(*p);
                if (start)
                {
                  if (AlmostEqualAbsolute(distFromStart,
                                          GetSectionLength(sectTup),
                                          tolerance))
                  {
                    tgtEntries->Append(PosJNetSpatial(rloc, p,
                                           GetSectionId(sectTup),
                                           GetSectionStartJunctionID(sectTup),
                                           distFromStart,
                                           GetSectionEndJunctionID(sectTup),
                                           0.0));
                  }
                  else
                  {
                    tgtEntries->Append(PosJNetSpatial(rloc, p,
                                           GetSectionId(sectTup),
                                           GetSectionStartJunctionID(sectTup),
                                           distFromStart,
                                           -1,-1000.0));
                  }

                }
                else
                {
                  if (AlmostEqualAbsolute(distFromStart, 0.0, tolerance))
                  {
                    tgtEntries->Append(PosJNetSpatial(rloc, p,
                                           GetSectionId(sectTup),
                                           GetSectionStartJunctionID(sectTup),
                                           0.0,
                                           GetSectionEndJunctionID(sectTup),
                                           GetSectionLength(sectTup)));
                  }
                  else
                  {
                    tgtEntries->Append(PosJNetSpatial(rloc, p,
                                           GetSectionId(sectTup),
                                           -1,-1000.0,
                                           GetSectionEndJunctionID(sectTup),
                                           GetSectionLength(sectTup) -
                                             distFromStart));
                  }

                }

                break;
              }

              case Both:
              {
                if (spatialPositions != 0)
                  spatialPositions->operator+=(*p);
                tgtEntries->Append(PosJNetSpatial(rloc, p,
                                           GetSectionId(sectTup),
                                           GetSectionStartJunctionID(sectTup),
                                           distFromStart,
                                           GetSectionEndJunctionID(sectTup),
                                           GetSectionLength(sectTup) -
                                           distFromStart));
                break;
              }

              default:
              {
                break;
              }
            }
            p->DeleteIfAllowed();
            p = 0;
          }
          sectDir->DeleteIfAllowed();
          sectDir = 0;
        }
        sectTup->DeleteIfAllowed();
        sectTup = 0;
      }
    }
    if (spatialPositions != NULL)
      spatialPositions->EndBulkLoad();
  }
}

/*
1.1.1 InitPriorityQueue

*/

bool JNetwork::InitPriorityQueue(PQManagement* pqueue,
                                 const DbArray<PosJNetSpatial>* sources,
                                 const Points* endPositions)
{
  if (pqueue != NULL && sources != NULL && endPositions != NULL &&
      sources->Size() > 0 && endPositions->Size() > 0)
  {
    PosJNetSpatial curSource;
    Tuple *juncTup = 0;
    JListInt* adjSectList = 0;
    Direction compD(Down);
    int jid = -1;
    for (int i = 0; i < sources->Size(); i++)
    {
      sources->Get(i, curSource);
      if (AlmostEqual(curSource.GetDistFromStartJunction(), 0.0) ||
          AlmostEqual(curSource.GetDistFromEndJunction(), 0.0))
      {
        jid = -1;
        if (AlmostEqual(curSource.GetDistFromStartJunction(), 0.0))
        {
          jid = curSource.GetStartJID();
          juncTup = GetJunctionTupleWithId(jid);
        }
        else
        {
          jid = curSource.GetEndJID();
          juncTup = GetJunctionTupleWithId(jid);
        }
        if (juncTup != NULL)
        {
          adjSectList = GetJunctionOutSectionList(juncTup);
          if (adjSectList != NULL)
          {
            JPQEntry tmp(curSource.GetNetworkPos().GetSide(), -1, jid,
                                 jid, -1, jid, jid, 0.0,
                            endPositions->Distance(curSource.GetSpatialPos()),
                            0.0);
            pqueue->InsertJunctionAsVisited(tmp);
            AddAdjacentSections<Points>(pqueue, adjSectList, tmp,
                                        endPositions);
            adjSectList->Destroy();
            adjSectList->DeleteIfAllowed();
          }
          juncTup->DeleteIfAllowed();
        }
      }
      else
      {
        Tuple* sectTup = GetSectionTupleWithId(curSource.GetSectionId());
        if (sectTup != 0)
        {
          Direction* sectDir = GetSectionDirection(sectTup);
          if (sectDir != NULL)
          {
            int test = sectDir->Compare(compD);
            if (test != 0)
            {
              jid = curSource.GetEndJID();
              juncTup = GetJunctionTupleWithId(jid);
              if (juncTup != 0)
              {
                Point* curJuncPoint = GetJunctionSpatialPos(juncTup);
                if (curJuncPoint != 0)
                {
                  adjSectList = GetSectionListAdjSectionsUp(sectTup);
                  JPQEntry tmp1((Direction) Up,
                                         -1, jid, jid,
                                         -1, jid, jid,
                                         curSource.GetDistFromEndJunction(),
                                         curSource.GetDistFromEndJunction() +
                                          endPositions->Distance(*curJuncPoint),
                                         curSource.GetDistFromEndJunction());
                  pqueue->InsertJunctionAsVisited(tmp1);
                  AddAdjacentSections<Points>(pqueue, adjSectList, tmp1,
                                              endPositions);
                  adjSectList->Destroy();
                  adjSectList->DeleteIfAllowed();
                  curJuncPoint->DeleteIfAllowed();
                }
                juncTup->DeleteIfAllowed();
              }
            }
            if (test >= 0)
            {
              jid = curSource.GetStartJID();
              juncTup = GetJunctionTupleWithId(jid);
              if (juncTup != 0)
              {
                Point* curJuncPoint = GetJunctionSpatialPos(juncTup);
                if (curJuncPoint != 0)
                {
                  adjSectList = GetSectionListAdjSectionsDown(sectTup);
                  JPQEntry tmp2((Direction) Down,
                                         -1, jid, jid,
                                         -1, jid, jid,
                                         curSource.GetDistFromStartJunction(),
                                         curSource.GetDistFromStartJunction() +
                                          endPositions->Distance(*curJuncPoint),
                                          curSource.GetDistFromStartJunction());
                  pqueue->InsertJunctionAsVisited(tmp2);
                  AddAdjacentSections<Points>(pqueue, adjSectList, tmp2,
                                              endPositions);
                  adjSectList->Destroy();
                  adjSectList->DeleteIfAllowed();
                  curJuncPoint->DeleteIfAllowed();
                }
                juncTup->DeleteIfAllowed();
              }
            }
            sectDir->DeleteIfAllowed();
            sectDir = 0;
          }
          sectTup->DeleteIfAllowed();
          sectTup = 0;
        }
      }
    }
    return true;
  }
  else
    return false;
}

void JNetwork::InitPriorityQueue(PQManagement* pqueue,
                                 const RouteLocation& source,
                                 DbArray<PairIntDouble>* result)
{
  double distFromStart(0.0);
  Tuple* sectTup = GetSectionTupleFor(source, distFromStart);
  if (sectTup != NULL)
  {
    int endJuncId = GetSectionEndJunctionID(sectTup);
    double distFromEnd = GetSectionLength(sectTup)- distFromStart;
    JPQEntry end((Direction) Up, -1, endJuncId, endJuncId,
                      -1, endJuncId, endJuncId,
                      distFromEnd, distFromEnd, distFromEnd);
    int startJuncId = GetSectionStartJunctionID(sectTup);
    JPQEntry start((Direction) Down, -1, startJuncId, startJuncId,
                   -1, startJuncId, startJuncId, distFromStart, distFromStart,
                   distFromStart);
    Direction* sectDir = GetSectionDirection(sectTup);
    if (sectDir != NULL)
    {
      if (AlmostEqualAbsolute(distFromStart, 0.0, tolerance) ||
        AlmostEqualAbsolute(distFromStart, GetSectionLength(sectTup),
                                     tolerance))
      {
        if (AlmostEqualAbsolute(distFromStart, 0.0, tolerance))
        {
          WriteToLists(pqueue, result, start);
        }
        else
        {
          WriteToLists(pqueue, result, end);
        }
      }
      else
      {
        Direction compD(Down);
        switch(sectDir->Compare(compD))
        {
          case -1: //sectDir up
          {
            WriteToLists(pqueue, result, end);
            break;
          }

          case 0: //sectDir down
          {
            WriteToLists(pqueue, result, start);
            break;
          }

          case 1: //sectDir both
          {
            WriteToLists(pqueue, result, start, end, sectTup);
            break;
          }

          default: //should never been reached
          {
            assert(false);
            break;
          }
        }
      }
      sectDir->DeleteIfAllowed();
      sectDir = 0;
    }
    sectTup->DeleteIfAllowed();
    sectTup = 0;
  }
}

void JNetwork::InitReversePriorityQueue(PQManagement* pqueue,
                                        const RouteLocation& source,
                                        DbArray<PairIntDouble>* result)
{
  double distFromStart(0.0);
  Tuple* sectTup = GetSectionTupleFor(source, distFromStart);
  if (sectTup != NULL)
  {
    int endJuncId = GetSectionEndJunctionID(sectTup);
    double distFromEnd = GetSectionLength(sectTup)- distFromStart;
    JPQEntry end((Direction) Down, -1, endJuncId, endJuncId,
                      -1, endJuncId, endJuncId,
                      distFromEnd, distFromEnd, distFromEnd);
    int startJuncId = GetSectionStartJunctionID(sectTup);
    JPQEntry start((Direction) Up, -1, startJuncId, startJuncId,
                   -1, startJuncId, startJuncId, distFromStart, distFromStart,
                   distFromStart);
    Direction* sectDir = GetSectionDirection(sectTup);
    if (sectDir != NULL)
    {
      if (AlmostEqualAbsolute(distFromStart, 0.0, tolerance) ||
          AlmostEqualAbsolute(distFromStart, GetSectionLength(sectTup),
                                     tolerance))
      {
        if (AlmostEqualAbsolute(distFromStart, 0.0, tolerance))
        {
          WriteToReverseLists(pqueue, result, start);
        }
        else
        {
          WriteToReverseLists(pqueue, result, end);
        }
      }
      else
      {
        Direction compD(Down);
        switch(sectDir->Compare(compD))
        {
          case -1: //sectDir up
          {
            WriteToReverseLists(pqueue, result, start);
            break;
          }

          case 0: //sectDir down
          {
            WriteToReverseLists(pqueue, result, end);
            break;
          }

          case 1: //sectDir both
          {
            WriteToReverseLists(pqueue, result, start, end, sectTup);
            break;
          }

          default: //should never been reached
          {
            assert(false);
            break;
          }
        }
      }
      sectDir->DeleteIfAllowed();
      sectDir = 0;
    }
    sectTup->DeleteIfAllowed();
    sectTup = 0;
  }
}

bool JNetwork::CheckForSameSections(const DbArray< PosJNetSpatial >* sources,
                                    const DbArray< PosJNetSpatial >* targets,
                                    DbArray< JRouteInterval >*& sp)
{
  if (sources != NULL && targets != NULL && sources->Size() > 0 &&
      targets->Size() > 0)
  {
    JRouteInterval* jri = 0;
    JRouteInterval* tmp = 0;
    PosJNetSpatial src, tgt;
    for (int i = 0; i < sources->Size(); i++)
    {
      sources->Get(i, src);
      for (int j = 0; j < targets->Size(); j++)
      {
        targets->Get(j,tgt);
        if (src.GetSectionId() == tgt.GetSectionId())
        {
          RouteLocation sour = src.GetNetworkPos();
          RouteLocation targ = tgt.GetNetworkPos();
          if (ExistsCommonRoute(sour, targ))
          {
            tmp = DirectConnection(src.GetSectionId(), sour, targ);
            if (jri != NULL)
            {
              if (jri->GetLength() > tmp->GetLength())
              {
                jri->DeleteIfAllowed();
                jri = new JRouteInterval(*tmp);
              }
            }
            else
            {
              jri = new JRouteInterval(*tmp);
            }
            tmp->DeleteIfAllowed();
          }
        }
      }
    }
    if (jri != NULL)
    {
      sp = new DbArray<JRouteInterval>(0);
      if (jri->GetLength() > 0.0)
        sp->Append(*jri);
      jri->DeleteIfAllowed();
    }
  }
  return (sp != NULL);
}

/*
Process Priority Queue

*/

bool JNetwork::ProcessPriorityQueue(PQManagement* pqueue,
                                    const DbArray< PosJNetSpatial >* targets,
                                    const Points* spatialPosTargets,
                                    int& tgtPosInArray,
                                    bool& tgtIsStartJunction,
                                    int& srcStartPathJID,
                                    double& minDist)
{
  bool found = false;
  JPQEntry* curPQElement = 0;
  DbArray<InterSP>* wayEntries = new DbArray<InterSP>(0);
  while(!found && !pqueue->IsEmpty())
  {
    curPQElement = pqueue->GetAndDeleteMin();
     if (minDist >= curPQElement->GetPriority())
    {
      InsertNetdistanceTuple(curPQElement->GetStartPathJID(), curPQElement,
                             wayEntries);
      double distLastJuncEndPoint = -1.0;
      if (reachedEndpoint(curPQElement, targets, distLastJuncEndPoint,
                          tgtPosInArray, tgtIsStartJunction, srcStartPathJID))
      {
        found = true;
        minDist = curPQElement->GetDistFromStartPoint()+ distLastJuncEndPoint;
        //check for shorter other end
        bool testOther = false;
        JPQEntry* test = 0;
        while(!testOther && !pqueue->IsEmpty())
        {
          test = pqueue->GetAndDeleteMin();
          if (test->GetPriority() < minDist)
          {
            double testDistLastJuncEndPoint = -1.0;
            int testPosInArray = -1;
            bool testIsStartJunction = false;
            int testSrcStartPathJID = -1;
            if (reachedEndpoint(test, targets, testDistLastJuncEndPoint,
                                testPosInArray, testIsStartJunction,
                                testSrcStartPathJID))
            {
              if (minDist > test->GetDistFromStartPoint() +
                                testDistLastJuncEndPoint)
              {
                if (curPQElement != 0)
                  delete curPQElement;
                curPQElement = test;
                minDist =
                  test->GetDistFromStartPoint() + testDistLastJuncEndPoint;
                tgtPosInArray = testPosInArray;
                tgtIsStartJunction = testIsStartJunction;
                srcStartPathJID = testSrcStartPathJID;
              }
            }
            else
            {
              AddAdjacentSections<Points>(pqueue, *test, spatialPosTargets);
            }
          }
          else
          {
            testOther = true;
          }
          if (test != NULL)
          {
            if (test != curPQElement)
              delete test;
            test = 0;
          }
        }
      }
      else
      {
        AddAdjacentSections<Points>(pqueue, *curPQElement, spatialPosTargets);
      }
    }
    else
    {
      found = true;
    }
    if (curPQElement != NULL)
      delete curPQElement;
    curPQElement = 0;
  }
  wayEntries->Destroy();
  delete wayEntries;
  return found;
}

void JNetwork::ProcessPriorityQueue(PQManagement* pqueue,
                                    DbArray<PairIntDouble>* result,
                                    const double distLimit)
{
  JPQEntry* curPQElement = 0;
  DbArray<InterSP>* wayEntries = new DbArray<InterSP>(0);
  double lastDist = numeric_limits<double>::min();
  while(!pqueue->IsEmpty() && lastDist <= distLimit)
  {
    curPQElement = pqueue->GetAndDeleteMin();
    lastDist = curPQElement->GetDistFromStartPoint();
    if(curPQElement != NULL)
    {
      PairIntDouble curPair(curPQElement->GetEndPartJID(),
                            curPQElement->GetDistFromStartPoint());
      result->Append(curPair);
      InsertNetdistanceTuple(curPQElement->GetStartPathJID(), curPQElement,
                           wayEntries);
      AddAdjacentSections<Point>(pqueue, *curPQElement, 0);
      delete curPQElement;
      curPQElement = 0;
    }
  }
  wayEntries->Destroy();
  delete wayEntries;
}

void JNetwork::ProcessReversePriorityQueue(PQManagement* pqueue,
                                           DbArray<PairIntDouble>* result,
                                           const double distLimit)
{
  JPQEntry* curPQElement = 0;
  double lastDist = numeric_limits<double>::min();
  while(!pqueue->IsEmpty() && lastDist <= distLimit)
  {
    curPQElement = pqueue->GetAndDeleteMin();
    lastDist = curPQElement->GetDistFromStartPoint();
    if(curPQElement != NULL)
    {
      PairIntDouble curPair(curPQElement->GetEndPartJID(),
                            curPQElement->GetDistFromStartPoint());
      result->Append(curPair);
      AddReverseAdjacentSections(pqueue, *curPQElement);
      delete curPQElement;
      curPQElement = 0;
    }
  }
}

bool JNetwork::CheckForExistingNetdistance(
                                const DbArray< PosJNetSpatial >* srcEntries,
                                const DbArray< PosJNetSpatial >* tgtEntries,
                                int& srcStartPathJID, int& tgtPosInArray,
                                bool& tgtOverStartJunction, double& minDist)
{
  bool result = false;
  if (srcEntries != NULL && tgtEntries != NULL &&
      srcEntries->Size() > 0 && tgtEntries->Size() > 0)
  {
    PosJNetSpatial start, end;
    double tmpMinDist = minDist;
    int tmpSrcStartPathJID = srcStartPathJID;
    bool tmpTgtOverStartJunction = tgtOverStartJunction;
    for (int i = 0; i < srcEntries->Size(); i++)
    {
      srcEntries->Get(i,start);
      for (int j = 0; j < tgtEntries->Size(); j++)
      {
        tgtEntries->Get(j,end);
        if (ExistsNetdistancesFor(start, end, tmpMinDist, tmpSrcStartPathJID,
                                  tmpTgtOverStartJunction))
        {
          result = true;
          if (tmpMinDist < minDist)
          {
            minDist = tmpMinDist;
            srcStartPathJID = tmpSrcStartPathJID;
            tgtPosInArray = j;
            tgtOverStartJunction = tmpTgtOverStartJunction;
          }
        }
      }
    }
  }
  return result;
}

bool JNetwork::ExistsNetdistancesFor(const PosJNetSpatial& start,
                                     const PosJNetSpatial& end,
                                     double& minDist,
                                     int& srcStartPathJID,
                                     bool& tgtOverStartJunction)
{
  bool result = false;
  bool tmp = false;
  if (start.GetStartJID() > -1)
  {
    if (end.GetStartJID() > -1)
    {
      tmp = ExistsNetdistanceFor(start.GetStartJID(),
                                    start.GetDistFromStartJunction(),
                                    end.GetStartJID(),
                                    end.GetDistFromStartJunction(),
                                    true,
                                    minDist, srcStartPathJID,
                                    tgtOverStartJunction);
      result = result || tmp;
    }
    if (end.GetEndJID() > -1)
    {
      tmp = ExistsNetdistanceFor(start.GetStartJID(),
                                    start.GetDistFromStartJunction(),
                                    end.GetEndJID(),
                                    end.GetDistFromEndJunction(),
                                    false,
                                    minDist, srcStartPathJID,
                                    tgtOverStartJunction);
      result = result || tmp;
    }
  }
  if (start.GetEndJID() > -1)
  {
    if (end.GetStartJID() > -1)
    {
      tmp = ExistsNetdistanceFor(start.GetEndJID(),
                                 start.GetDistFromEndJunction(),
                                 end.GetStartJID(),
                                 end.GetDistFromStartJunction(),
                                 true,
                                 minDist, srcStartPathJID,
                                 tgtOverStartJunction);
      result = result || tmp;
    }
    if (end.GetEndJID() > -1)
    {
      tmp = ExistsNetdistanceFor(start.GetEndJID(),
                                 start.GetDistFromEndJunction(),
                                 end.GetEndJID(),
                                 end.GetDistFromEndJunction(),
                                 false,
                                 minDist, srcStartPathJID,
                                 tgtOverStartJunction);
      result = result || tmp;
    }
  }
  return result;
}

bool JNetwork::ExistsNetdistanceFor(const int startjid,
                                    const double startdist,
                                    const int endjid,
                                    const double enddist,
                                    const bool endJIDisStartJunc,
                                    double& minDist,
                                    int& srcStartPathJID,
                                    bool& tgtOverStartJunction)
{
  Tuple* netDistTup = GetNetdistanceTupleFor(startjid, endjid);
  if (netDistTup != 0)
  {
    double netDist = GetNetdistanceDistance(netDistTup);
    double tmpMinDist = startdist + netDist + enddist;
    if (tmpMinDist < minDist)
    {
      minDist = tmpMinDist;
      srcStartPathJID = startjid;
      tgtOverStartJunction = endJIDisStartJunc;
    }
    netDistTup->DeleteIfAllowed();
    netDistTup = 0;
    return true;
  }
  else
  {
    return false;
  }
}

void JNetwork::WriteToLists(PQManagement* pqueue,
                            DbArray<PairIntDouble>* result,
                            JPQEntry& junc)
{
  pqueue->InsertJunctionAsVisited(junc);
  PairIntDouble curPair(junc.GetEndPartJID(), junc.GetDistFromStartPoint());
  result->Append(curPair);
  Tuple* juncTup = GetJunctionTupleWithId(junc.GetEndPartJID());
  if (juncTup != NULL)
  {
    JListInt* adjSect = GetJunctionOutSectionList(juncTup);
    if (adjSect != NULL)
    {
      AddAdjacentSections<Point>(pqueue, adjSect, junc, 0);
      adjSect->Destroy();
      adjSect->DeleteIfAllowed();
    }
    juncTup->DeleteIfAllowed();
  }
}

void JNetwork::WriteToLists(PQManagement* pqueue,
                            DbArray<PairIntDouble>* result,
                            JPQEntry& start, JPQEntry& end,
                            const Tuple* sectTup)
{
  pqueue->InsertJunctionAsVisited(start);
  pqueue->InsertJunctionAsVisited(end);
  PairIntDouble startPair(start.GetEndPartJID(), start.GetDistFromStartPoint());
  result->Append(startPair);
  PairIntDouble endPair(end.GetEndPartJID(), end.GetDistFromStartPoint());
  result->Append(endPair);
  if (sectTup != 0)
  {
    JListInt* adjSect = GetSectionListAdjSectionsUp(sectTup);
    if (adjSect != NULL)
    {
      AddAdjacentSections<Point>(pqueue, adjSect, end, 0);
      adjSect->Destroy();
      adjSect->DeleteIfAllowed();
      adjSect = 0;
    }
    adjSect = GetSectionListAdjSectionsDown(sectTup);
    if (adjSect != NULL)
    {
      AddAdjacentSections<Point>(pqueue, adjSect, start, 0);
      adjSect->Destroy();
      adjSect->DeleteIfAllowed();
      adjSect = 0;
    }
  }
}

void JNetwork::WriteToReverseLists(PQManagement* pqueue,
                                   DbArray<PairIntDouble>* result,
                                   JPQEntry& junc)
{
  pqueue->InsertJunctionAsVisited(junc);
  PairIntDouble curPair(junc.GetEndPartJID(), junc.GetDistFromStartPoint());
  result->Append(curPair);
  Tuple* juncTup = GetJunctionTupleWithId(junc.GetEndPartJID());
  if (juncTup != NULL)
  {
    JListInt* adjSect = GetJunctionInSectionList(juncTup);
    if (adjSect != NULL)
    {
      AddReverseAdjacentSections(pqueue, adjSect, junc);
      adjSect->Destroy();
      adjSect->DeleteIfAllowed();
    }
    juncTup->DeleteIfAllowed();
  }
}

void JNetwork::WriteToReverseLists(PQManagement* pqueue,
                                   DbArray<PairIntDouble>* result,
                                   JPQEntry& start, JPQEntry& end,
                                   const Tuple* sectTup)
{
  pqueue->InsertJunctionAsVisited(start);
  pqueue->InsertJunctionAsVisited(end);
  PairIntDouble startPair(start.GetEndPartJID(), start.GetDistFromStartPoint());
  result->Append(startPair);
  PairIntDouble endPair(end.GetEndPartJID(), end.GetDistFromStartPoint());
  result->Append(endPair);
  if (sectTup != 0)
  {
    JListInt* adjSect = GetSectionListReverseAdjSectionsUp(sectTup);
    if (adjSect != NULL)
    {
      AddReverseAdjacentSections(pqueue, adjSect, start);
      adjSect->Destroy();
      adjSect->DeleteIfAllowed();
      adjSect = 0;
    }
    adjSect = GetSectionListReverseAdjSectionsDown(sectTup);
    if (adjSect != NULL)
    {
      AddReverseAdjacentSections(pqueue, adjSect, end);
      adjSect->Destroy();
      adjSect->DeleteIfAllowed();
      adjSect = 0;
    }
  }
}

/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const JNetwork& n)
{
  n.Print(os);
  return os;
}
