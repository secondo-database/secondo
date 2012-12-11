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
  bool ok = OrderedRelation::Save(valueRecord, offset, relNumType, wrel);
  return ok;
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
1.1 ~comparePairJidDist~

*/

int comparePairJidDistByDist(const void* a, const void* b)
{
  pair<int,double>* ap = (pair<int,double>*) a;
  pair<int,double>* bp = (pair<int,double>*) b;
  if (ap->second < bp->second) return -1;
  if (ap->second > bp->second) return 1;
  if (ap->first < bp->first) return -1;
  if (ap->first > bp->first) return 1;
  return 0;
}

int comparePairJidDistById(const void* a, const void* b)
{
  pair<int,double>* ap = (pair<int,double>*) a;
  pair<int,double>* bp = (pair<int,double>*) b;
  if (ap->first < bp->first) return -1;
  if (ap->first > bp->first) return 1;
  if (ap->second < bp->second) return -1;
  if (ap->second > bp->second) return 1;
  return 0;
}

/*
1.1. reachedEndpoint

Checks if curPQElement endJID is enclosed in endJunctions list

*/

bool reachedEndpoint(const JPQEntry* actEntry,
                     const DbArray<pair<int, double> >* endJunctions,
                     double& dist)
{
  pair<int, double> curEntry;
  for (int i = 0; i < endJunctions->Size(); i++)
  {
    endJunctions->Get(i,curEntry);
    if (curEntry.first == actEntry->GetEndPartJID()){
      dist = curEntry.second;
      return true;
    }
  }
  return false;
}

/*
1.1 cleanSP

Cleans up memory at end of shortest path computation.

*/

void cleanShortestPathMemory(Direction* dir,
                             DbArray<pair<int, double> >* ej,
                             PQManagement* pq, Tuple* cj, JPQEntry* jpq)
{
  if (dir != 0) dir->DeleteIfAllowed();
  if (ej != 0)
  {
    ej->Destroy();
    delete ej;
  }
  if (pq != 0)
  {
    pq->Destroy();
    delete pq;
  }
  if (cj != 0) cj->DeleteIfAllowed();
  if (jpq != 0) delete jpq;
}

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
{}

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
                   const ListExpr typeInfo) :
  defined(false), tolerance(0.0), junctions(0), sections(0), routes(0),
  netdistances(0), junctionsBTree(0), junctionsRTree(0), sectionsBTree(0),
  sectionsRTree(0), routesBTree(0)
{
  Word w;
  ListExpr idLE;
  nl->ReadFromString(CcString::BasicType(), idLE);
  ListExpr numId = SecondoSystem::GetCatalog()->NumericType(idLE);
  bool ok = OpenAttribute<CcString>(valueRecord, offset, numId, w);

  if (ok)
  {
    CcString* stn = (CcString*)w.addr;
    strcpy(id, stn->GetValue().c_str());
    stn->DeleteIfAllowed();
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
                      valueRecord,
                      offset);

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
      Relation* juncRel = relationFromList(nl->Third(instance),
                                      JNetUtil::GetJunctionsRelationTypeInfo(),
                                           errorPos, errorInfo, correct);
      Relation* sectRel = relationFromList(nl->Fourth(instance),
                                        JNetUtil::GetSectionsRelationTypeInfo(),
                                           errorPos, errorInfo, correct);
      Relation* routeRel = relationFromList(nl->Fifth(instance),
                                        JNetUtil::GetRoutesRelationTypeInfo(),
                                            errorPos, errorInfo, correct);
      OrderedRelation* distRel =
        ordRelationFromList(nl->Sixth(instance),
                            JNetUtil::GetNetdistancesRelationTypeInfo(),
                            errorPos, errorInfo, correct);
      if (!correct){
        if (juncRel != 0) juncRel->Delete();
        if (sectRel != 0) sectRel->Delete();
        if (routeRel != 0) routeRel->Delete();
        if (distRel != 0) {
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
    Word w;
    w.setAddr(new CcString(true,Symbol::UNDEFINED()));
    ListExpr idLE;
    nl->ReadFromString(CcString::BasicType(), idLE);
    ListExpr numId = SecondoSystem::GetCatalog()->NumericType(idLE);
    return SaveAttribute<CcString>(valueRecord, offset, numId, w);
  }
}

bool JNetwork::Save(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr  typeInfo)
{
  Word w;
  CcString* stn = new CcString(true, id);
  w.setAddr(stn);
  ListExpr idLE;
  nl->ReadFromString(CcString::BasicType(), idLE);
  ListExpr numId = SecondoSystem::GetCatalog()->NumericType(idLE);
  bool ok = SaveAttribute<CcString>(valueRecord, offset, numId, w);
  stn->DeleteIfAllowed();

  if (ok)
  {
    CcReal* tol = new CcReal(true, tolerance);
    w.setAddr(tol);
    nl->ReadFromString(CcReal::BasicType(), idLE);
    numId = SecondoSystem::GetCatalog()->NumericType(idLE);
    ok = SaveAttribute<CcReal>(valueRecord, offset, numId, w);
    tol->DeleteIfAllowed();
  }

  if (ok)
    ok = saveRelation(JNetUtil::GetJunctionsRelationTypeInfo(), junctions,
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
  value.addr = new JNetwork(valueRecord, offset, typeInfo);
  return (value.addr != 0);
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
      nl->TextAtom("(netname junctionsrel sectionsrel routesrel distrel)")));
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
      JRouteInterval* actInt = GetNetworkValueOf(hs);
      if (actInt != NULL)
      {
        if (actInt->IsDefined())
          result->Add(*actInt);
        actInt->DeleteIfAllowed();
        actInt = 0;
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
             endPos != 0 && endPos->IsDefined())
        { // got valid RouteLocations and
          SimulateTrip(*startPos, *endPos,
                       &actSource.p1,
                       starttime, endtime, lc, rc,
                       startSectTup, endSectTup,
                       distStart, distEnd, result);
        }
        startPos->DeleteIfAllowed();
        startSectTup->DeleteIfAllowed();
        startSectTup = endSectTup;
        endSectTup = 0;
        startPos = endPos;
        endPos = 0;
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
  if (AlmostEqual(distStart, 0.0))
    res = GetSectionStartJunctionRLocs(actSect);
  else
  {
    if (AlmostEqual(distStart, GetSectionLength(actSect)))
      res = GetSectionEndJunctionRLocs(actSect);
    else
    {
      if (distStart > 0.0 && distStart < GetSectionLength(actSect))
      {
        JListRInt* rintList = GetSectionListRouteIntervals(actSect);
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
            res->operator+=(*result);
            result->DeleteIfAllowed();
          }
          res->EndBulkload();
        }
        if (rintList != 0)
        {
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
  double pos = 0.0;
  Tuple* actSect = GetSectionTupleFor(rloc,pos);
  JListRLoc* res = GetNetworkValuesOf(actSect,pos);
  actSect->DeleteIfAllowed();
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
    JListInt* routeSectList = 0;
    int lastRouteSecListIndex = -1;
    int curRid = -1;
    JRouteInterval* lastRint = 0;
    SimpleLine* lastCurve = 0;
    for (int i = 0; i < mjp->GetNoComponents(); i++)
    {
      mjp->Get(i,ju);
      SplitJUnit(ju, curRid, lastRint, routeSectList,
                 lastRouteSecListIndex, endTimeCorrected, lastEnd,
                 lastCurve, result);
    }
    result.EndBulkLoad();
    if (routeSectList != 0) routeSectList->DeleteIfAllowed();
    if (lastRint != 0) lastRint->DeleteIfAllowed();
    if (lastCurve != 0) lastCurve->DeleteIfAllowed();
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
      lastindex = sectList->GetNoOfComponents();
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
              SimpleLine* sl = new SimpleLine(0);
              if (actInt.Contains(rint))
                actCurve->SubLine(fabs(rint.GetFirstPosition() -
                                       actInt.GetFirstPosition()),
                                  fabs(rint.GetLastPosition() -
                                       actInt.GetFirstPosition()),
                                  *sl);
              else
              {
                if (actInt.Contains(RouteLocation(rint.GetRouteId(),
                                                  rint.GetFirstPosition(),
                                                  rint.GetSide())))
                  actCurve->SubLine(fabs(rint.GetFirstPosition() -
                                         actInt.GetFirstPosition()),
                                    fabs(actInt.GetLastPosition()-
                                         actInt.GetFirstPosition()),
                                    *sl);
                else
                {
                  if (actInt.Contains(RouteLocation(rint.GetRouteId(),
                                                    rint.GetLastPosition(),
                                                    rint.GetSide())))
                    actCurve->SubLine(0,
                                      fabs(rint.GetLastPosition() -
                                           actInt.GetFirstPosition()),
                                      *sl);
                }
              }
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
  while(index < routeSectList->GetNoOfComponents() && !found)
  {
    routeSectList->Get(index, cSid);
    if (sectTup != 0)
      sectTup->DeleteIfAllowed();
    sectTup = GetSectionTupleWithId(cSid.GetIntval());
    if (lastRint != 0)
    {
      lastRint->DeleteIfAllowed();
      lastRint = 0;
    }
    lastRint = GetSectionRouteIntervalForRLoc(rloc, sectTup);
    if (lastRint == 0)
      index++;
    else
      found = true;
  }
  if (!found)
  {
    index = routeSectList->GetNoOfComponents() -1;
    while (index >= 0 && !found)
    {
      routeSectList->Get(index, cSid);
      if (sectTup != 0)
        sectTup->DeleteIfAllowed();
      sectTup = GetSectionTupleWithId(cSid.GetIntval());
      if (lastRint != 0)
      {
        lastRint->DeleteIfAllowed();
        lastRint = 0;
      }
      lastRint = GetSectionRouteIntervalForRLoc(rloc, sectTup);
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
  while (i < traj.Size())
  {
    traj.Get(i,actRint);
    Rectangle<2> actRect = BoundingBox(actRint);
    if (actRect.IsDefined() && tmpres.IsDefined())
      tmpres.Union(actRect);
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

bool JNetwork::Contains(const RouteLocation* rloc) const {
  return (rloc->GetPosition() <= GetRouteLength(rloc->GetRouteId()));
}

bool JNetwork::Contains(const JRouteInterval* rint) const{
  return (rint->GetFirstPosition() >= 0.0 &&
          rint->GetLastPosition()<= GetRouteLength(rint->GetRouteId()));
}

/*
1.1.1 ~SimulateTrip~

*/

void JNetwork::SimulateTrip(const RouteLocation& source,
                            const RouteLocation& target,
                            const Point* sourcePos,
                            const Point* targetPos,
                            const Instant& starttime,
                            const Instant& endtime,
                            MJPoint* result)
{
  bool lc = true;
  bool rc = (starttime == endtime);
  double distSourceStartSect = 0.0;
  Tuple* startSectTup = GetSectionTupleFor(sourcePos, distSourceStartSect);
  double distTargetStartSect = 0.0;
  Tuple* endSectTup = GetSectionTupleFor(targetPos, distTargetStartSect);
  SimulateTrip(source, target, targetPos, starttime, endtime, lc, rc,
               startSectTup, endSectTup, distSourceStartSect,
               distTargetStartSect, result);
  startSectTup->DeleteIfAllowed();
  endSectTup->DeleteIfAllowed();
}

void JNetwork::SimulateTrip(const RouteLocation& source,
                            const RouteLocation& target,
                            const Point* targetPos,
                            const Instant& starttime,
                            const Instant& endtime,
                            const bool& lc, const bool& rc,
                            const Tuple* startSectTup,
                            const Tuple* endSectTup,
                            const double distSourceStartSect,
                            const double distTargetStartSect,
                            MJPoint* result)
{
  double length = 0.0;
  DbArray<JRouteInterval>* sp = ShortestPath(source, target, targetPos, length,
                                             startSectTup, endSectTup,
                                             distSourceStartSect,
                                             distTargetStartSect);
  if (sp != 0)
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
      if (sp->Size() > 0)
      {
        JRouteInterval actInt;
        Instant unitstart = starttime;
        bool unitlc = lc;
        Instant unitend = unitstart;
        bool unitrc = rc;
        for (int i = 0; i < sp->Size(); i++)
        {
          sp->Get(i,actInt);
          if (i == sp->Size()-1)
          {
            unitend = endtime;
            unitrc = rc;
          }
          else
            unitend = (endtime - starttime) * actInt.GetLength() /
                                  length + unitstart;
          JUnit actUnit(JUnit(Interval<Instant>(unitstart, unitend,
                                                unitlc, unitrc),
                              actInt));
          result->Add(actUnit);
          unitstart = unitend;
          unitlc = !unitrc;
          unitrc = !unitlc;
          unitend = unitstart;
        }
      }
    }
    sp->Destroy();
    delete sp;
  }
}

/*
1.1.1 ShortestPath

*/

DbArray<JRouteInterval>* JNetwork::ShortestPath(const RouteLocation& source,
                                              const RouteLocation& target,
                                              const Point* targetPos,
                                              double& length,
                                              const Tuple* startSectTup,
                                              const Tuple* endSectTup,
                                              const double distSourceStartSect,
                                              const double distTargetStartSect)
{
  Direction compU(Up);
  Direction compD(Down);
  int startSectId = GetSectionId(startSectTup);
  int endSectId = GetSectionId(endSectTup);
  DbArray<JRouteInterval>* res = new DbArray<JRouteInterval> (0);
  length = 0.0;
  RouteLocation src(source);
  RouteLocation tgt(target);
  if ((startSectId == endSectId || ExistsCommonRoute(src, tgt)) &&
       DirectConnectionExists(startSectId, endSectId, startSectTup, endSectTup,
                              src, tgt, res, length))
  {
    //Special case computation already finished.
    return res;
  }
  Direction* curSectDir = 0;
  DbArray<pair<int, double> >* endJunctions =
      new DbArray<pair<int, double> >(0);
  //compute (set) of junctions where computation ends
  if (AlmostEqual(distTargetStartSect, 0.0) ||
      AlmostEqual(distTargetStartSect, GetSectionLength(endSectTup)))
  {
    //end is junction
    if (AlmostEqual(distTargetStartSect, 0.0))
      endJunctions->Append(make_pair<int, double>(
        GetSectionStartJunctionID(endSectTup), 0.0));
    else
      endJunctions->Append(make_pair<int, double>(
        GetSectionEndJunctionID(endSectTup), 0.0));
  }
  else
  {
    //end inside section.
    //check possible moving direction on end section
    curSectDir = GetSectionDirection(endSectTup);
    if (curSectDir->Compare(compD) != 0)
      endJunctions->Append(
        make_pair<int, double> (GetSectionStartJunctionID(endSectTup),
                                distTargetStartSect));
    if (curSectDir->Compare(compU) != 0)
      endJunctions->Append(
        make_pair<int, double> (GetSectionEndJunctionID(endSectTup),
                    fabs(GetSectionLength(endSectTup) - distTargetStartSect)));
  }
  PQManagement* pq = new PQManagement();
  JPQEntry* pqEntry = 0;
  int startPathJID = -1;
  int endPathJID = -1;
  Tuple* curJunc = 0;
  //init priority queue with starting intervals
  if (AlmostEqual(distSourceStartSect, 0.0) ||
      AlmostEqual(distSourceStartSect, GetSectionLength(startSectTup)))
  {
    //start on junction
    if (AlmostEqual(distSourceStartSect, 0.0))
    {
      curSectDir = new Direction(Down);
      curJunc = GetSectionStartJunctionTuple(startSectTup);
      startPathJID = GetJunctionId(curJunc);
    }
    else
    {
      curSectDir = new Direction(Up);
      curJunc = GetSectionEndJunctionTuple(startSectTup);
      startPathJID = GetJunctionId(curJunc);
    }
    if (ExistsNetworkdistanceFor(startPathJID, endJunctions, endPathJID))
    {
      WriteShortestPath(source,  distSourceStartSect, target,
                        distTargetStartSect, startSectTup, endSectTup,
                        startPathJID, endPathJID, res, length);
      cleanShortestPathMemory( curSectDir, endJunctions, pq, curJunc, pqEntry);
      return res;
    }
    else
    {
      JListInt* curAdjSec = GetJunctionOutSectionList(curJunc);
      pqEntry = new JPQEntry(*curSectDir, -1, startPathJID, -1, -1,
                             -1, startPathJID, 0.0, 0.0);
      AddAdjacentSections(pq, curAdjSec, *pqEntry, targetPos);
      delete pqEntry;
      pqEntry = 0;
      curAdjSec->Destroy();
      curAdjSec->DeleteIfAllowed();
    }
  }
  else
  {
    //start within section insert route intervals to the end junction(s) of
    //the section reachable from start.
    //check possible moving direction
    curSectDir = GetSectionDirection(startSectTup);
    JListInt* curAdjSec = 0;
    if (curSectDir->Compare(compD) != 0)
    {
      //curSectDir Up or Both use end junction for inital entries in pq
      curJunc = GetSectionEndJunctionTuple(startSectTup);
      startPathJID = GetJunctionId(curJunc);
      if (ExistsNetworkdistanceFor(startPathJID, endJunctions, endPathJID))
      {
        WriteShortestPath(source, distSourceStartSect,target,
                          distTargetStartSect, startSectTup, endSectTup,
                          startPathJID, endPathJID, res, length);
        cleanShortestPathMemory( curSectDir, endJunctions, pq, curJunc,
                                 pqEntry);
        return res;
      }
      else
      {
        curAdjSec = GetSectionListAdjSectionsUp(startSectTup);
        pqEntry = new JPQEntry((Direction) Up, startSectId, startPathJID, -1,
                               startSectId, -1, startPathJID,
                               fabs(GetSectionLength(startSectTup) -
                                 distSourceStartSect),
                               fabs(GetSectionLength(startSectTup) -
                                 distSourceStartSect));
        AddAdjacentSections(pq, curAdjSec, *pqEntry, targetPos);
        delete pqEntry;
        pqEntry = 0;
        curAdjSec->Destroy();
        curAdjSec->DeleteIfAllowed();
      }
    }
    if (curSectDir->Compare(compU) != 0)
    {
      //curSectDir Down or Both use start junction for inital entries in pq
      curJunc = GetSectionStartJunctionTuple(startSectTup);
      startPathJID = GetJunctionId(curJunc);
      if (ExistsNetworkdistanceFor(startPathJID, endJunctions, endPathJID))
      {
        WriteShortestPath(source, distSourceStartSect, target,
                          distTargetStartSect, startSectTup, endSectTup,
                          startPathJID, endPathJID, res, length);
        cleanShortestPathMemory( curSectDir, endJunctions, pq, curJunc,
                                 pqEntry);
        return res;
      }
      else
      {
        curAdjSec = GetSectionListAdjSectionsDown(startSectTup);
        pqEntry = new  JPQEntry((Direction) Down, startSectId, startPathJID, -1,
                                startSectId, -1, startPathJID,
                                distSourceStartSect,
                                distSourceStartSect);
        AddAdjacentSections(pq, curAdjSec, *pqEntry, targetPos);
        delete pqEntry;
        pqEntry = 0;
        curAdjSec->Destroy();
        curAdjSec->DeleteIfAllowed();
      }
    }
  }
  JPQEntry* curPQElement = 0;
  bool found = false;
  double minDist = numeric_limits< double >::max();
  //process priority queue
  while (!found && !pq->IsEmpty())
  {
    if (curPQElement != 0)
      delete curPQElement;
    curPQElement = pq->GetAndDeleteMin();
    InsertNetdistanceTuple(startPathJID, curPQElement);
    double distLastJuncEndPoint;
    if (reachedEndpoint(curPQElement, endJunctions, distLastJuncEndPoint))
    {
      found = true;
      minDist =
        curPQElement->GetDistFromStart() + distLastJuncEndPoint;
      //check if alternative end is possible
      if (endJunctions->Size() > 1)
      {
        // might exist shorter path over other end Junction.
        bool testedOtherEnd = false;
        JPQEntry* test = 0;
        while (!testedOtherEnd && !pq->IsEmpty())
        {
           if (test != 0)
              delete test;
          test = pq->GetAndDeleteMin();
          InsertNetdistanceTuple(startPathJID, test);
          if (test->GetPriority() < minDist)
          {
            double testDistLastJuncEndPoint;
            if (reachedEndpoint(test, endJunctions, testDistLastJuncEndPoint))
            {
              if (minDist > test->GetDistFromStart() +
                testDistLastJuncEndPoint)
              {
                delete curPQElement;
                curPQElement = test;
                minDist = test->GetDistFromStart() + testDistLastJuncEndPoint;
              }
            }
            else
              AddAdjacentSections(pq, *test, targetPos);
          }
          else
            testedOtherEnd = true;
        }
      }
      if (ExistsNetworkdistanceFor(startPathJID, endJunctions, endPathJID))
      {
        WriteShortestPath(source, distSourceStartSect, target,
                          distTargetStartSect, startSectTup, endSectTup,
                          startPathJID, endPathJID, res, length);
        cleanShortestPathMemory( curSectDir, endJunctions, pq, curJunc,
                                 pqEntry);
        return res;
      }
    }
    else
      AddAdjacentSections(pq, *curPQElement, targetPos);
  }
  cleanShortestPathMemory( curSectDir, endJunctions, pq, curJunc, pqEntry);
  return res;
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

void JNetwork::InsertNetdistanceTuple(const int fromjid, const JPQEntry* jp)
{
  InsertNetdistanceTuple(fromjid,jp->GetEndPartJID(),jp->GetStartNextJID(),
                         jp->GetStartNextSID(), jp->GetDistFromStart());
}

void JNetwork::InsertNetdistanceTuple(const int fromjid, const int tojid,
                                      const int viajid, const int viasid,
                                      const double dist)
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
    if (((CcReal*)existingTuple->GetAttribute(NETDIST_DIST))->GetRealval()
          > dist)
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

/*
1.1 Get Tuples by identifier

*/

Tuple* JNetwork::GetRouteTupleWithId(const int rid) const
{
  return GetTupleWithId(routesBTree, routes, rid);
}

Tuple* JNetwork::GetSectionTupleWithId(const int sid) const
{
  return GetTupleWithId(sectionsBTree, sections, sid);
}

Tuple* JNetwork::GetJunctionTupleWithId(const int jid) const
{
  return GetTupleWithId(junctionsBTree, junctions, jid);
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
  if (actCurve != 0) actCurve->DeleteIfAllowed();
  return actSect;
}

Tuple* JNetwork::GetSectionTupleFor(const RouteLocation& rloc, double& relpos)
const
{
  JListInt* sectList = GetRouteSectionList(rloc.GetRouteId());
  int index = -1;
  Tuple* res = GetSectionTupleFor(rloc, relpos, sectList, index);
  sectList->Destroy();
  sectList->DeleteIfAllowed();
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
        if (CheckTupleForRLoc(actSect, rloc, pos))
          return actSect;
      }
      int i = 0;
      while (i < sectList->GetNoOfComponents())
      {
        sectList->Get(i, actSid);
        actSect = GetSectionTupleWithId(actSid.GetIntval());
        if (CheckTupleForRLoc(actSect, rloc, pos))
        {
          index = i;
          return actSect;
        }
        if (actSect != 0)
          actSect->DeleteIfAllowed();
        actSect = 0;
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
  return ((CcReal*)routeTuple->GetAttribute(ROUTE_LENGTH))->GetRealval();
}

JListInt* JNetwork::GetRouteSectionList(const int rid) const
{
  Tuple* actRouteTup = GetRouteTupleWithId(rid);
  JListInt* res = GetRouteSectionList(actRouteTup);
  actRouteTup->DeleteIfAllowed();
  return res;
}

JListInt* JNetwork::GetRouteSectionList(const Tuple* actRoute) const
{
  return
    new JListInt(*((JListInt*) actRoute->GetAttribute(ROUTE_LIST_SECTIONS)));
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
  return
    new JListRInt(*(
      (JListRInt*) sectTuple->GetAttribute(SEC_LIST_ROUTEINTERVALS)));
}

JRouteInterval* JNetwork::GetSectionFirstRouteInterval(const Tuple* sectTuple)
const
{
  JRouteInterval res;
  JListRInt* rintList = GetSectionListRouteIntervals(sectTuple);
  rintList->Get(0,res);
  rintList->DeleteIfAllowed();
  return new JRouteInterval(res);
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
  return ((CcReal*)actSect->GetAttribute(SEC_LENGTH))->GetRealval();
}

Direction* JNetwork::GetSectionDirection(const Tuple* actSect) const
{
  return new Direction(*((Direction*)actSect->GetAttribute(SEC_DIRECTION)));
}

int JNetwork::GetSectionId(const Tuple* actSect) const
{
  return ((CcInt*)actSect->GetAttribute(SEC_ID))->GetIntval();
}

JRouteInterval* JNetwork::GetRouteIntervalFor(const JListRLoc* leftrlocs,
                                              const JListRLoc* rightrlocs,
                                              const bool allowResetSide) const
{
  JRouteInterval* res = 0;
  if (leftrlocs->IsDefined() && rightrlocs->IsDefined() &&
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

JListInt* JNetwork::GetSectionListAdjSectionsUp(const Tuple* sectTuple) const
{
  return new JListInt(*(
      (JListInt*)sectTuple->GetAttribute(SEC_LIST_ADJ_SECTIONS_UP)));
}

JListInt* JNetwork::GetSectionListAdjSectionsDown(const Tuple* sectTuple) const
{
  return new JListInt(*(
        (JListInt*)sectTuple->GetAttribute(SEC_LIST_ADJ_SECTIONS_DOWN)));
}

JListInt* JNetwork::GetSectionListReverseAdjSectionsUp(const Tuple* sectTuple)
  const
{
  return new JListInt(*(
        (JListInt*)sectTuple->GetAttribute(SEC_LIST_REV_ADJ_SECTIONS_UP)));
}

JListInt* JNetwork::GetSectionListReverseAdjSectionsDown(const Tuple* sectTuple)
  const
{
  return new JListInt(*(
     (JListInt*)sectTuple->GetAttribute(SEC_LIST_REV_ADJ_SECTIONS_DOWN)));
}

Tuple* JNetwork::GetSectionStartJunctionTuple(const Tuple* sectTuple) const
{
  return GetJunctionTupleWithId(
    ((CcInt*)sectTuple->GetAttribute(SEC_STARTNODE_ID))->GetIntval());
}

Tuple* JNetwork::GetSectionEndJunctionTuple(const Tuple* sectTuple) const
{
  return GetJunctionTupleWithId(
    ((CcInt*)sectTuple->GetAttribute(SEC_ENDNODE_ID))->GetIntval());
}

JListRLoc* JNetwork::GetSectionStartJunctionRLocs(const Tuple* sectTuple) const
{
  Tuple* juncTup = GetSectionStartJunctionTuple(sectTuple);
  JListRLoc* res =  GetJunctionListRLoc(juncTup);
  juncTup->DeleteIfAllowed();
  return res;
}

JListRLoc* JNetwork::GetSectionEndJunctionRLocs(const Tuple* sectTuple) const
{
  Tuple* juncTup = GetSectionEndJunctionTuple(sectTuple);
  JListRLoc* res = GetJunctionListRLoc(juncTup);
  juncTup->DeleteIfAllowed();
  return res;
}

int JNetwork::GetSectionStartJunctionID(const Tuple* sectTuple) const
{
  return ((CcInt*)sectTuple->GetAttribute(SEC_STARTNODE_ID))->GetIntval();
}

int JNetwork::GetSectionEndJunctionID(const Tuple* sectTuple) const
{
  return ((CcInt*)sectTuple->GetAttribute(SEC_ENDNODE_ID))->GetIntval();
}

Point* JNetwork::GetSectionStartPoint(const Tuple* sectTuple) const
{
  Tuple* juncTuple =
    GetJunctionTupleWithId(GetSectionStartJunctionID(sectTuple));
  Point* res = GetJunctionSpatialPos(juncTuple);
  juncTuple->DeleteIfAllowed();
  return res;
}

Point* JNetwork::GetSectionEndPoint(const Tuple* sectTuple) const
{
  Tuple* juncTuple =
    GetJunctionTupleWithId(GetSectionEndJunctionID(sectTuple));
  Point* res = GetJunctionSpatialPos(juncTuple);
  juncTuple->DeleteIfAllowed();
  return res;
}

double JNetwork::GetSectionMaxSpeed(const Tuple* sectTuple) const
{
  return ((CcReal*)sectTuple->GetAttribute(SEC_VMAX))->GetRealval();
}

/*
1.1.1 Juctions Attributes

*/

int JNetwork::GetJunctionId(const Tuple* juncTup) const
{
  return ((CcInt*)juncTup->GetAttribute(JUNC_ID))->GetIntval();
}

JListRLoc* JNetwork::GetJunctionListRLoc(const Tuple* juncTuple) const
{
  return
    new JListRLoc(*((JListRLoc*)
      juncTuple->GetAttribute(JUNC_LIST_ROUTEPOSITIONS)));
}

Point* JNetwork::GetJunctionSpatialPos(const Tuple* juncTuple) const
{
  return new Point(*((Point*) juncTuple->GetAttribute(JUNC_POS)));
}

JListInt* JNetwork::GetJunctionOutSectionList(const Tuple* juncTup) const
{
  return new JListInt(*(
    (JListInt*) juncTup->GetAttribute(JUNC_LIST_OUTSECTIONS)));
}

JListInt* JNetwork::GetJunctionInSectionList(const Tuple* juncTup) const
{
  return new JListInt(*(
    (JListInt*) juncTup->GetAttribute(JUNC_LIST_INSECTIONS)));
}

/*
1.1.1 Netdistance Relation

*/

int JNetwork::GetNetdistanceNextSID(const Tuple* actNetDistTup) const
{
  return ((CcInt*)actNetDistTup->GetAttribute(NETDIST_NEXT_SID))->GetIntval();
}

bool JNetwork::ExistsNetworkdistanceFor(const int startPathJID,
                              const DbArray<pair<int, double> >* endJunctions,
                              int& endPathJID) const
{
  pair<int, double> curEntry;
  double minDist = numeric_limits<double>::max();
  bool found = false;
  for (int i = 0; i < endJunctions->Size();i++)
  {
    endJunctions->Get(i,curEntry);
    Tuple* netDistTup = GetNetdistanceTupleFor(startPathJID, curEntry.first);
    if (netDistTup != 0)
    {
      found = true;
      endPathJID = curEntry.first;
      double actDist =
        ((CcReal*)netDistTup->GetAttribute(NETDIST_DIST))->GetRealval();
      if (minDist > actDist)
        minDist = actDist;
      netDistTup->DeleteIfAllowed();
    }
  }
  return found;
}

/*
1.1 DirectConnectionExists

*/
bool JNetwork::DirectConnectionExists(const int startSID,
                                      const int endSID,
                                      const Tuple* sourceSectTup,
                                      const Tuple* targetSectTup,
                                      const RouteLocation& source,
                                      const RouteLocation& target,
                                      DbArray<JRouteInterval>* res,
                                      double& length) const
{
  Direction* sectDir = GetSectionDirection(sourceSectTup);
  Direction dirU(Up);
  Direction dirD(Down);
  Direction movDir(false);
  if (source.GetPosition() > target.GetPosition())
    movDir = dirD;
  else
    movDir = dirU;
  if (startSID == endSID && sectDir->SameSide(movDir, false))
  {
    res->Append(JRouteInterval(source.GetRouteId(), source.GetPosition(),
                               target.GetPosition(), movDir));
    length += fabs(target.GetPosition()-source.GetPosition());
    sectDir->DeleteIfAllowed();
    return true;
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
    sectDir->DeleteIfAllowed();
    res->Append(JRouteInterval(source.GetRouteId(),
                               min(source.GetPosition(), target.GetPosition()),
                               max(source.GetPosition(), target.GetPosition()),
                               movDir));
    length += fabs(target.GetPosition()-source.GetPosition());
    return true;
  }
  sectDir->DeleteIfAllowed();
  return false;
}

/*
1.1.1.1 AddAdjacentSections

*/

void JNetwork::AddAdjacentSections(PQManagement* pq, JPQEntry curEntry,
                                   const Point* targetPos)
{
  Tuple* pqSectTup = GetSectionTupleWithId(curEntry.GetSectionId());
  Direction movDir = curEntry.GetDirection();
  JListInt* listSID = 0;
  Direction compU(Up);
  if (movDir.Compare(compU) == 0)
    listSID = GetSectionListAdjSectionsUp(pqSectTup);
  else
    listSID = GetSectionListAdjSectionsDown(pqSectTup);
  AddAdjacentSections(pq, listSID, curEntry, targetPos);
  listSID->Destroy();
  listSID->DeleteIfAllowed();
  pqSectTup->DeleteIfAllowed();
}

void JNetwork::AddAdjacentSections(PQManagement* pq, const JListInt* listSID,
                                   JPQEntry curEntry, const Point* targetPos)
{
  Tuple* curSectTup = 0;
  CcInt nextSID;
  for (int i = 0; i < listSID->GetNoOfComponents(); i++)
  {
    listSID->Get(i,nextSID);
    curSectTup = GetSectionTupleWithId(nextSID.GetIntval());
    int curSID = GetSectionId(curSectTup);
    if (curEntry.GetStartNextJID() < 0)
    {
      curEntry.SetStartNextSID(curSID);
      if (GetSectionEndJunctionID(curSectTup) == curEntry.GetStartPathJID())
      {
        curEntry.SetDirection((Direction) Down);
        curEntry.SetStartNextJID(GetSectionStartJunctionID(curSectTup));
        curEntry.SetStartPartJID(GetSectionEndJunctionID(curSectTup));
        curEntry.SetEndPartJID(GetSectionStartJunctionID(curSectTup));
      }
      else
      {
        curEntry.SetStartNextJID(GetSectionEndJunctionID(curSectTup));
        curEntry.SetStartPartJID(GetSectionStartJunctionID(curSectTup));
        curEntry.SetEndPartJID(GetSectionEndJunctionID(curSectTup));
      }

    }
    else
    {
      if (GetSectionEndJunctionID(curSectTup) == curEntry.GetEndPartJID())
      {
        curEntry.SetDirection((Direction) Down);
        curEntry.SetStartPartJID(GetSectionEndJunctionID(curSectTup));
        curEntry.SetEndPartJID(GetSectionStartJunctionID(curSectTup));
      }
      else
      {
        curEntry.SetStartPartJID(GetSectionStartJunctionID(curSectTup));
        curEntry.SetEndPartJID(GetSectionEndJunctionID(curSectTup));
      }
    }
    Tuple* curJunc = GetJunctionTupleWithId(curEntry.GetEndPartJID());
    Point* curEndPoint = GetJunctionSpatialPos(curJunc);
    double curDist =
      curEntry.GetDistFromStart() + GetSectionLength(curSectTup);
    double prio = curDist + curEndPoint->Distance(*targetPos);
    pq->Insert(JPQEntry(curEntry.GetDirection(),
                        nextSID.GetIntval(),
                        curEntry.GetStartPathJID(),
                        curEntry.GetStartNextJID(),
                        curEntry.GetStartNextSID(),
                        curEntry.GetStartPartJID(),
                        curEntry.GetEndPartJID(),
                        curDist,
                        prio));
    curJunc->DeleteIfAllowed();
    curEndPoint->DeleteIfAllowed();
    curSectTup->DeleteIfAllowed();
  }
}

/*
1.1.1.1 WriteShortestPath

*/

void JNetwork::WriteShortestPath(const RouteLocation& source,
                                 const double distSourceStartSect,
                                 const RouteLocation& target,
                                 const double distTargetStartSect,
                                 const Tuple* startSectTup,
                                 const Tuple* endSectTup,
                                 const int startPathJID,
                                 const int endPathJID,
                                 DbArray< JRouteInterval >* result,
                                 double& length) const
{
  bool found = false;
  int curStartJID = startPathJID;
  JRouteInterval* curRint = 0;
  curRint = GetSectionFirstRouteInterval(startSectTup);
  //write first part of path
  if (startPathJID == GetSectionStartJunctionID(startSectTup))
  {
    curRint->SetSide((Direction) Down);
    curRint->SetInterval(curRint->GetFirstPosition(),
                         curRint->GetFirstPosition()+distSourceStartSect);
    result->Append(*curRint);
    length = length + curRint->GetLength();
    if (endPathJID == GetSectionEndJunctionID(startSectTup))
    {
      found = true;
      curStartJID = endPathJID;
    }
  }
  else
  {
    curRint->SetSide((Direction) Up);
    curRint->SetInterval(curRint->GetLastPosition() -
                  fabs(GetSectionLength(startSectTup)-distSourceStartSect),
                   curRint->GetLastPosition());
    result->Append(*curRint);
    length = length + curRint->GetLength();
    if (endPathJID == GetSectionStartJunctionID(startSectTup))
    {
      found = true;
      curStartJID = endPathJID;
    }
  }
  Tuple* actNetDistTup = 0;
  Tuple* curSectTup = 0;
  //get path between start and end section
  while (!found)
  {
    if (actNetDistTup != 0)
      actNetDistTup->DeleteIfAllowed();
    actNetDistTup = GetNetdistanceTupleFor(curStartJID, endPathJID);
    if (curSectTup != 0)
      curSectTup->DeleteIfAllowed();
    curSectTup = GetSectionTupleWithId(GetNetdistanceNextSID(actNetDistTup));
    if (curRint != 0) curRint->DeleteIfAllowed();
    curRint = GetSectionFirstRouteInterval(curSectTup);
    if (curStartJID == GetSectionStartJunctionID(curSectTup))
    {
      curRint->SetSide((Direction) Up);
      curStartJID = GetSectionEndJunctionID(curSectTup);
    }
    else
    {
      curRint->SetSide((Direction) Down);
      curStartJID = GetSectionStartJunctionID(curSectTup);
    }
    result->Append(*curRint);
    length = length + curRint->GetLength();
    if (endPathJID == curStartJID)
      found = true;
  }
  curRint = GetSectionFirstRouteInterval(endSectTup);
  //write last part
  if (endPathJID == GetSectionStartJunctionID(endSectTup))
  {
    curRint->SetSide((Direction) Up);
    curRint->SetInterval(curRint->GetFirstPosition(),
                         curRint->GetFirstPosition()+distTargetStartSect);
    result->Append(*curRint);
    length = length + curRint->GetLength();
  }
  else
  {
    curRint->SetSide((Direction) Down);
    curRint->SetInterval(curRint->GetLastPosition(),
                         curRint->GetLastPosition() -
                        fabs(GetSectionLength(endSectTup)-distTargetStartSect));
    result->Append(*curRint);
    length = length + curRint->GetLength();
  }
  if (actNetDistTup != 0) actNetDistTup->DeleteIfAllowed();
  if (curSectTup != 0) curSectTup->DeleteIfAllowed();
  if (curRint != 0) curRint->DeleteIfAllowed();
}

/*
1.1.1 ExistsCommonRoute

*/

bool JNetwork::ExistsCommonRoute(RouteLocation& src, RouteLocation& tgt) const
{
  if (src.IsOnSameRoute(tgt)) {
    return true;
  }
  else
  {
    JListRLoc* left = GetNetworkValuesOf(src);
    JListRLoc* right = GetNetworkValuesOf(tgt);
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
    left->Destroy();
    left->DeleteIfAllowed();
    right->Destroy();
    right->DeleteIfAllowed();
    return false;
  }
}

/*
1.1.1 GetRLocOfPosOnRouteInterval

*/

RouteLocation* JNetwork::GetRLocOfPosOnRouteInterval(
    const JRouteInterval* actInt, const double pos) const
{
  Direction compD(Down);
  if (actInt->GetSide().Compare(compD) != 0)
    return new RouteLocation(actInt->GetRouteId(),
                             actInt->GetFirstPosition() + pos,
                             actInt->GetSide());
  else
    return new RouteLocation(actInt->GetRouteId(),
                             actInt->GetFirstPosition() - pos,
                             actInt->GetSide());
}

/*
1.1.1 Split Junit

*/

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
      lastCurve->SubLine(spos, epos, resLine);
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
      double startIndex = lastRouteSecListIndex;
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
      double lastIndex = lastRouteSecListIndex;
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
        if (instInter1 > instInter2)
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
              if (endTimeCorrected && instInter1 < instInter2)
                endTimeCorrected = false;
              if (instInter1 == instInter2)
              {
                instInter2 = instInter2 + TIMECORRECTION;
                endTimeCorrected = true;
              }
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
                instInter2 = ju.TimeAtPos(resLine.Length() - actDist);
                if (endTimeCorrected && instInter1 < instInter2)
                  endTimeCorrected = false;
                if (instInter1 == instInter2)
                {
                  instInter2 = instInter2 + TIMECORRECTION;
                  endTimeCorrected = true;
                }
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
        else
        {
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
      pos = fabs(rloc.GetPosition() - actInt->GetFirstPosition());
      actInt->DeleteIfAllowed();
      return true;
    }
  }
  return false;
}


/*
1 Overwrite output operator

*/
ostream& operator<< (ostream& os, const JNetwork& n)
{
  n.Print(os);
  return os;
}
