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

2012, May Simone Jandt

1 Includes

*/

#include <assert.h>
#include "AlgebraTypes.h"
#include "ListUtils.h"
#include "SecondoSystem.h"
#include "Symbols.h"
#include "LogMsg.h"
#include "Point.h"
#include "Direction.h"
#include "JNetwork.h"
#include "RouteLocation.h"
#include "JRouteInterval.h"
#include "JList.h"


extern NestedList* nl;

/*
1 Helpful Operations

1.1 ~getRelationCopy~

Returns a pointer to the copy of the relation of relPointer.

*/


Relation* getRelationCopy(const string relTypeInfo,
                         const Relation* relPointer)
{
  ListExpr strPtr = listutils::getPtrList(relPointer);
  string querystring = "(consume (feed (" + relTypeInfo +
                       " (ptr " + nl->ToString(strPtr) + "))))";
  Word resultWord;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( querystring, resultWord );
  assert ( QueryExecuted );
  return ( Relation * ) resultWord.addr;
}

OrderedRelation* getRelationCopy(const string relTypeInfo,
                                 const OrderedRelation* relPointer)
{
  ListExpr relType;
  nl->ReadFromString ( relTypeInfo, relType );
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType(relType);
  Word wrel;
  wrel.setAddr(const_cast<OrderedRelation*> (relPointer));
  Word res = OrderedRelation::Clone(relNumType, wrel);
  return (OrderedRelation*) res.addr;
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
1 Implementation of class JNetwork

1.1 Constructors and Deconstructor

*/

JNetwork::JNetwork()
{}

JNetwork::JNetwork(const bool def) :
  defined(def), id(""), junctions(0), sections(0), routes(0), netdistances(0),
  junctionsBTree(0), junctionsRTree(0), sectionsBTree(0),sectionsRTree(0),
  routesBTree(0)
{}

JNetwork::JNetwork(const string nid, const Relation* injunctions,
                   const Relation* insections, const Relation* inroutes) :
  defined(true), id(nid),
  junctions(getRelationCopy(junctionsRelationTypeInfo, injunctions)),
  sections(getRelationCopy(sectionsRelationTypeInfo, insections)),
  routes(getRelationCopy(routesRelationTypeInfo, inroutes)),
  netdistances(0), junctionsBTree(0), junctionsRTree(0), sectionsBTree(0),
  sectionsRTree(0), routesBTree(0)
{
  InitNetdistances();
  CreateTrees();
}

JNetwork::JNetwork(const string nid, const Relation* injunctions,
                   const Relation* insections, const Relation* inroutes,
                   const OrderedRelation* inDist) :
  defined(true), id(nid),
  junctions(getRelationCopy(junctionsRelationTypeInfo, injunctions)),
  sections(getRelationCopy(sectionsRelationTypeInfo, insections)),
  routes(getRelationCopy(routesRelationTypeInfo, inroutes)),
  netdistances(getRelationCopy(netdistancesRelationTypeInfo, inDist)),
  junctionsBTree(0), junctionsRTree(0), sectionsBTree(0), sectionsRTree(0),
  routesBTree(0)
{
 CreateTrees();
}


JNetwork::JNetwork(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo) :
  defined(false), id(""), junctions(0), sections(0), routes(0), netdistances(0),
  junctionsBTree(0), junctionsRTree(0), sectionsBTree(0),sectionsRTree(0),
  routesBTree(0)
{
  Word w;
  ListExpr idLE;
  nl->ReadFromString(CcString::BasicType(), idLE);
  ListExpr numId = SecondoSystem::GetCatalog()->NumericType(idLE);
  bool ok = OpenAttribute<CcString>(valueRecord, offset, numId, w);

  if (ok)
  {
    CcString* stn = (CcString*)w.addr;
    id = stn->GetValue();
    stn->DeleteIfAllowed();
  }


  if (ok && id == Symbol::UNDEFINED())
  {
    id = "";
    ok = false;
  }

  if (ok)
    ok = openRelation(junctions, junctionsRelationTypeInfo, valueRecord,
                      offset);

  if (ok)
    ok = openRelation(sections, sectionsRelationTypeInfo, valueRecord, offset);

  if (ok)
    ok = openRelation(routes, routesRelationTypeInfo, valueRecord, offset);

  if (ok)
    ok = openRelation(netdistances, netdistancesRelationTypeInfo, valueRecord,
                      offset);

  if (ok)
    ok = openBTree(junctionsBTree, junctionsBTreeTypeInfo, valueRecord,
                   offset);

  if (ok)
    ok = openBTree(sectionsBTree, sectionsBTreeTypeInfo, valueRecord, offset);

  if (ok)
    ok = openBTree(routesBTree, routesBTreeTypeInfo, valueRecord, offset);

  Word wTree;

  if (ok)
    ok = junctionsRTree->Open(valueRecord, offset, junctionsRTreeTypeInfo,
                              wTree);

  if (ok)
    junctionsRTree = (R_Tree<2,TupleId>*) wTree.addr;

  if (ok)
    ok = sectionsRTree->Open(valueRecord, offset, sectionsRTreeTypeInfo, wTree);

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

string JNetwork::GetId() const
{
  return id;
}

string JNetwork::GetJunctionsRelationType()
{
  return junctionsRelationTypeInfo;
}

string JNetwork::GetSectionsRelationType()
{
  return sectionsRelationTypeInfo;
}

string JNetwork::GetRoutesRelationType()
{
  return routesRelationTypeInfo;
}
string JNetwork::GetNetdistancesRelationType()
{
  return netdistancesRelationTypeInfo;
}

Relation* JNetwork::GetJunctionsCopy() const
{
  return getRelationCopy(junctionsRelationTypeInfo, junctions);
}

Relation* JNetwork::GetRoutesCopy() const
{
  return getRelationCopy(routesRelationTypeInfo, routes);
}

Relation* JNetwork::GetSectionsCopy() const
{
  return getRelationCopy(sectionsRelationTypeInfo, sections);
}

OrderedRelation* JNetwork::GetNedistancesRelationCopy() const
{
  return getRelationCopy(netdistancesRelationTypeInfo,
                         netdistances);
}

void JNetwork::SetDefined(const bool def)
{
  defined = def;
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
    ListExpr netId = nl->StringAtom(source->GetId());
    ListExpr junclist = source->JunctionsToList();
    ListExpr sectlist = source->SectionsToList();
    ListExpr routelist = source->RoutesToList();
    ListExpr dislist = source->NetdistancesToList();
    return nl->FiveElemList(netId, junclist, sectlist, routelist, dislist);
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
    if (nl->ListLength(instance) == 5)
    {
      ListExpr netId = nl->First(instance);
      string nid = nl->StringValue(netId);

      Relation* juncRel = relationFromList(nl->Second(instance),
                                           junctionsRelationTypeInfo,
                                           errorPos, errorInfo, correct);
      Relation* sectRel = relationFromList(nl->Third(instance),
                                           sectionsRelationTypeInfo,
                                           errorPos, errorInfo, correct);
      Relation* routeRel = relationFromList(nl->Fourth(instance),
                                            routesRelationTypeInfo,
                                            errorPos, errorInfo, correct);
      OrderedRelation* distRel =
        ordRelationFromList(nl->Fifth(instance), netdistancesRelationTypeInfo,
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

      JNetwork* n = new JNetwork(nid, juncRel, sectRel, routeRel, distRel);

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
  JNetwork* clone = new JNetwork(source->GetId()+"clone",
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
    ok = saveRelation(junctionsRelationTypeInfo, junctions, valueRecord,
                      offset);

  if (ok)
    ok = saveRelation(sectionsRelationTypeInfo, sections, valueRecord, offset);

  if (ok)
    ok = saveRelation(routesRelationTypeInfo, routes, valueRecord, offset);

  if (ok)
    ok = saveRelation(netdistancesRelationTypeInfo, netdistances, valueRecord,
                      offset);

  if (ok)
    ok = saveBTree(junctionsBTreeTypeInfo, junctionsBTree, valueRecord, offset);

  if (ok)
    ok = saveBTree(sectionsBTreeTypeInfo, sectionsBTree, valueRecord, offset);

  if (ok)
    ok = saveBTree(routesBTreeTypeInfo, routesBTree, valueRecord, offset);

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
      nl->TextAtom("(" + CcString::BasicType() + " " +
        junctionsRelationTypeInfo + " " + sectionsRelationTypeInfo + " " +
        routesRelationTypeInfo +" " + netdistancesRelationTypeInfo + "), the" +
        " string defines the name of the network, it is followed by the " +
        "network data for junctions, sections, routes and network distances "+
        "in nested list format."),
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
1.1 Operations for other network data types
1.1.1 ~Contains~
Checks if the given position(s) exist in the network.

*/

bool JNetwork::Contains(const RouteLocation* rloc) const {
  if (rloc->GetPosition() <= GetRouteLength(rloc->GetRouteId()))
    return true;
  else
    return false;
}

bool JNetwork::Contains(const JRouteInterval* rint) const{
  if (rint->GetFirstPosition() >= 0 &&
      rint->GetLastPosition()<= GetRouteLength(rint->GetRouteId()))
    return true;
  else
    return false;
}

/*
1.1 Relation Descriptors

*/

string JNetwork::junctionsTupleTypeInfo = Tuple::BasicType() + "((Id " +
  CcInt::BasicType() + ") (Pos " + Point::BasicType() + ") (Listjuncpos " +
  JListRLoc::BasicType() + ") (Listinsections " + JListInt::BasicType() +
  ") (Listoutsections " + JListInt::BasicType() + "))";

string JNetwork::sectionsTupleTypeInfo =  Tuple::BasicType() + "((Id " +
  CcInt::BasicType() + ") (Curve " + SimpleLine::BasicType() +
  ") (StartJunctionId " + CcInt::BasicType() + ") (EndJunctionId " +
  CcInt::BasicType() + ") (Direction " + Direction::BasicType() +
  ") (VMax " + CcReal::BasicType() + ") (Lenth " + CcReal::BasicType() +
  ") (ListSectRouteIntervals " +  JListRInt::BasicType() +
  ") (ListAdjSectUp " + JListInt::BasicType() +
  ") (ListAdjSectDown " + JListInt::BasicType() + ") (ListRevAdjSectUp " +
  JListInt::BasicType() + ") (ListRevAdjSectDown " + JListInt::BasicType() +
  "))";

string JNetwork::routesTupleTypeInfo = Tuple::BasicType() + "((Id " +
  CcInt::BasicType() + ") (ListJunctions " + JListInt::BasicType() +
  ") (ListSections " + JListInt::BasicType() + ") (Lenth " +
  CcReal::BasicType() + "))";

string JNetwork::junctionsRelationTypeInfo = "("+ Relation::BasicType() + "(" +
  junctionsTupleTypeInfo + "))";

string JNetwork::sectionsRelationTypeInfo = "("+ Relation::BasicType() + "(" +
  sectionsTupleTypeInfo + "))";

string JNetwork::routesRelationTypeInfo = "("+ Relation::BasicType() + "(" +
  routesTupleTypeInfo + "))";

string JNetwork::netdistancesRelationTypeInfo = "("+
  OrderedRelation::BasicType() + "("+ Tuple::BasicType() +
  "((Source " + CcInt::BasicType() + ")(Target " + CcInt::BasicType() +
  ")(NextJunct " + CcInt::BasicType() + ")(ViaSect " + CcInt::BasicType() +
  ")(NetDist " + CcReal::BasicType() + "))) (Source Target))";

string JNetwork::junctionsBTreeTypeInfo = "("+ BTree::BasicType() + "(" +
  junctionsTupleTypeInfo + ")" + CcInt::BasicType() + ")";

string JNetwork::junctionsRTreeTypeInfo = "("+ R_Tree<2,TupleId>::BasicType() +
  "(" + junctionsTupleTypeInfo + ")" + Point::BasicType() + " FALSE)";

string JNetwork::sectionsBTreeTypeInfo = "("+ BTree::BasicType() + "(" +
  sectionsTupleTypeInfo + ")" + CcInt::BasicType() + ")";

string JNetwork::sectionsRTreeTypeInfo = "("+ R_Tree<2, TupleId>::BasicType() +
 "(" + sectionsTupleTypeInfo + ")" + SimpleLine::BasicType() + " FALSE)";

string JNetwork::routesBTreeTypeInfo = "("+ BTree::BasicType() + "(" +
  routesTupleTypeInfo + ")" + CcInt::BasicType() + ")";


/*
1.1 Private functions

1.1.1 Build ListExpr for Out-Function of Network

*/

ListExpr JNetwork::JunctionsToList() const
{
  return relationToList(junctions, junctionsRelationTypeInfo);
}

ListExpr JNetwork::SectionsToList() const
{
  return relationToList(sections, sectionsRelationTypeInfo);
}

ListExpr JNetwork::RoutesToList() const
{
  return relationToList(routes, routesRelationTypeInfo);
}

ListExpr JNetwork::NetdistancesToList() const
{
  return relationToList(netdistances, netdistancesRelationTypeInfo);
}


/*
1.1 Creates Trees

*/

void JNetwork::CreateTrees()
{
  junctionsBTree = createBTree(junctions, junctionsRelationTypeInfo, "Id");
  junctionsRTree = createRTree(junctions, junctionsRelationTypeInfo, "Pos");
  sectionsBTree = createBTree(sections, sectionsRelationTypeInfo, "Id");
  sectionsRTree = createRTree(sections, sectionsRelationTypeInfo, "Curve");
  routesBTree = createBTree(routes, routesRelationTypeInfo,"Id");
}

/*
1.1 Initialize netdistance ordered relation

*/

void JNetwork::InitNetdistances()
{
  ListExpr relType;
  nl->ReadFromString(netdistancesRelationTypeInfo, relType);
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType ( relType );
  netdistances = new OrderedRelation(relNumType);
  GenericRelationIterator* it = sections->MakeScan();
  Tuple* sectTuple = 0;
  Tuple* insertTuple = 0;
  while ((sectTuple = it->GetNextTuple())){
    insertTuple = new Tuple(nl->Second(relNumType));
    insertTuple->CopyAttribute(SEC_STARTNODE_ID, sectTuple, 0);
    insertTuple->CopyAttribute(SEC_ENDNODE_ID, sectTuple, 1);
    insertTuple->CopyAttribute(SEC_ENDNODE_ID, sectTuple, 2);
    insertTuple->CopyAttribute(SEC_ID, sectTuple, 3);
    insertTuple->CopyAttribute(SEC_LENGTH, sectTuple, 4);
    netdistances->AppendTuple(insertTuple);
    sectTuple->DeleteIfAllowed();
    sectTuple = 0;
    insertTuple->DeleteIfAllowed();
    insertTuple = 0;
  }
  delete it;
}

/*
1.1 Get Tuples by identifier

*/

Tuple* JNetwork::GetRouteTupleWithId(const int rid) const
{
  CcInt* crid = new CcInt(true, rid);
  Tuple* res = 0;
  BTreeIterator* it = routesBTree->ExactMatch(crid);
  crid->DeleteIfAllowed();
  if (it->Next() != 0)
    res = routes->GetTuple ( it->GetId(), false );
  delete it;
  return res;
}

/*
1.1 Access to (tuple) attributes for identifier

*/

double JNetwork::GetRouteLength(const int rid) const
{
  double res = -1.0;
  Tuple* routeTup = GetRouteTupleWithId(rid);
  if (routeTup != 0){
    res = ((CcReal*)routeTup->GetAttribute(ROUTE_LENGTH))->GetRealval();
    routeTup->DeleteIfAllowed();
    routeTup = 0;
  }
  return res;
}

/*
1 Overwrite output operator

*/
ostream& operator<< (ostream& os, const JNetwork& n)
{
  n.Print(os);
  return os;
}