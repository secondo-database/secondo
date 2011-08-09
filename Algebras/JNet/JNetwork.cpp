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

2011, May Simone Jandt

1 Includes

*/

#include <assert.h>
#include "../../include/AlgebraTypes.h"
#include "../../include/ListUtils.h"
#include "../../include/SecondoSystem.h"
#include "../../include/Symbols.h"
#include "../Spatial/Point.h"
#include "Direction.h"
#include "JNetwork.h"
#include "RouteLocation.h"
#include "PairTIDRLoc.h"


extern NestedList* nl;

static bool DEBUG = false;

/*
2 Constructors and Deconstructor

*/

JNetwork::JNetwork() : nDef(false),
                       id(""),
                       junctions(0),
                       sections(0),
                       routes(0),
                       sectionsBTree(0),
                       sectionsRTree(0),
                       junctionsBTree(0),
                       junctionsRTree(0),
                       routesBTree(0)
{
  if (DEBUG) cout << "JNetwork::Network()" << endl;
}

JNetwork::JNetwork(const bool def) : nDef(def), id(""),
                                     junctions(0), sections(0), routes(0),
                                     sectionsBTree(0), sectionsRTree(0),
                                     junctionsBTree(0), junctionsRTree(0),
                                     routesBTree(0)
{
  if (DEBUG) cout << "JNetwork::Network(bool)" << endl;
}

JNetwork::JNetwork(const string nid, Relation* injunctions,
                   Relation* insections, Relation* inroutes) :
  nDef(true), id(nid), junctions(injunctions), sections(insections),
  routes(inroutes)
{
  if (DEBUG) cout << "JNetwork::Network(string, rel, rel, rel)" << endl;
  CreateTrees();
}

JNetwork::JNetwork(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo) : nDef(false), id("")
{
  if (DEBUG) cout << "JNetwork::Network(smirecord, ...)" << endl;
  Word w;
  ListExpr idLE;
  nl->ReadFromString(CcString::BasicType(), idLE);
  ListExpr numId = SecondoSystem::GetCatalog()->NumericType(idLE);
  if (!OpenAttribute<CcString>(valueRecord, offset, numId, w))
    return;
  id = ((CcString*)w.addr)->GetValue();

  junctions = OpenRelation(junctionsTypeInfo, valueRecord, offset);
  if (junctions == 0) return;

  sections = OpenRelation(sectionsTypeInfo, valueRecord, offset);
  if (sections == 0)
  {
    junctions->Delete();
    return;
  }

  routes = OpenRelation(routesTypeInfo, valueRecord, offset);
  if (routes == 0)
  {
    junctions->Delete();
    sections->Delete();
    return;
  }

  sectionsBTree = OpenBTree(sectionsBTreeTypeInfo, valueRecord, offset);
  if ( !sectionsBTree)
  {
    junctions->Delete();
    sections->Delete();
    routes->Delete();
    return;
  }

  Word wSRTree;

  if(!(sectionsRTree->Open(valueRecord, offset, sectionsRTreeTypeInfo,
                           wSRTree)))
  {
    junctions->Delete();
    sections->Delete();
    routes->Delete();
    delete sectionsBTree;
    return;
  }
  sectionsRTree = (R_Tree<2,TupleId>*) wSRTree.addr;

  junctionsBTree = OpenBTree(junctionsBTreeTypeInfo, valueRecord,offset);
  if (!junctionsBTree)
  {
    junctions->Delete();
    sections->Delete();
    routes->Delete();
    delete sectionsBTree;
    delete sectionsRTree;
    return;
  }

  Word wJRTree;
  if(!(junctionsRTree->Open(valueRecord, offset, junctionsRTreeTypeInfo,
                            wJRTree)))
  {
    junctions->Delete();
    sections->Delete();
    routes->Delete();
    delete sectionsBTree;
    delete sectionsRTree;
    delete junctionsBTree;
    return;
  }
  junctionsRTree = (R_Tree<2,TupleId>*) wJRTree.addr;

  routesBTree = OpenBTree(routesBTreeTypeInfo, valueRecord, offset);
  if (!routesBTree)
  {
    junctions->Delete();
    sections->Delete();
    routes->Delete();
    delete sectionsBTree;
    delete sectionsRTree;
    delete junctionsBTree;
    delete junctionsRTree;
    return;
  }
  nDef = true;
  return;
}

JNetwork::JNetwork(const JNetwork& net) : nDef(net.IsDefined()),
                                          id(net.GetId()),
                                          junctions(net.GetJunctions()),
                                          sections(net.GetSections()),
                                          routes(net.GetRoutes()),
                                          sectionsBTree(net.sectionsBTree),
                                          sectionsRTree(net.sectionsRTree),
                                          junctionsBTree(net.junctionsBTree),
                                          junctionsRTree(net.junctionsRTree),
                                          routesBTree(net.routesBTree)
{
  if (DEBUG) cout << "JNetwork::Network(JNetwork)" << endl;
}

JNetwork::JNetwork(const ListExpr instance, const int errorPos,
                   ListExpr& errorInfo, bool& correct) :
  nDef(true),
  id(""),
  junctions(0),
  sections(0),
  routes(0),
  sectionsBTree(0),
  sectionsRTree(0),
  junctionsBTree(0),
  junctionsRTree(0),
  routesBTree(0)
{
  if (DEBUG) cout << "JNetwork::JNetwork(instance..)" << endl;
  NList inlist(instance);

  if (inlist.length() == 1 && inlist.isAtom() &&
      inlist.isEqual(Symbol::UNDEFINED()))
  {
    correct = true;
    nDef = false;
    return;
  }

  if (inlist.length() != 4)
  {
    correct = false;
    cerr << "List of length 4 expected." << endl;
    return;
  }

  NList netId(inlist.first());
  NList juncList(inlist.second());
  NList sectList(inlist.third());
  NList routeList(inlist.fourth());
  if (DEBUG) cout << "Read id from: " << netId.str()<< endl;
  if (!(netId.isAtom() && netId.isString()))
  {
    correct = false;
    cerr << "First element should be string atom." << endl;
    return;
  }

  id = netId.str();

  if (DEBUG) cout << "Read junctions" << endl;
  ListExpr typeInf = nl->TheEmptyList();
  nl->ReadFromString(junctionsTypeInfo, typeInf);
  ListExpr relNumericType = SecondoSystem::GetCatalog()->NumericType(typeInf);
  junctions = (Relation*) Relation::In(relNumericType, juncList.listExpr(),
                                       errorPos, errorInfo, correct,false);
  if (!correct)
  {
    cerr << "Second Element must be junctions relation." << endl;
    return;
  }

  if (DEBUG) cout << "Read sections" << endl;
  nl->ReadFromString(sectionsTypeInfo, typeInf);
  relNumericType = SecondoSystem::GetCatalog()->NumericType(typeInf);
  sections = (Relation*)Relation::In(relNumericType, sectList.listExpr(),
                                     errorPos, errorInfo, correct,false);
  if (!correct)
  {
    delete junctions;
    cerr << "Third Element must be sections relation."<< endl;
    return;
  }

  if (DEBUG) cout << "Read routes" << endl;
  nl->ReadFromString(routesTypeInfo, typeInf);
  relNumericType = SecondoSystem::GetCatalog()->NumericType(typeInf);
  routes = (Relation*) Relation::In(relNumericType, routeList.listExpr(),
                                    errorPos, errorInfo, correct, false);
  if (!correct)
  {
    delete junctions;
    delete sections;
    cerr << "Fourth Element must be routes relation." << endl;
    return ;
  }

  correct = true;
  CreateTrees();
}

JNetwork::~JNetwork()
{
  if (DEBUG) cout << "JNetwork::~JNetwork()" << endl;
  delete junctions;
  delete sections;
  delete routes;
  delete sectionsBTree;
  delete sectionsRTree;
  delete junctionsBTree;
  delete junctionsRTree;
  delete routesBTree;
}

/*
1 Getter and Setter for private Attributes

*/

bool JNetwork::IsDefined() const
{
  return nDef;
}

string JNetwork::GetId() const
{
  return id;
}

Relation* JNetwork::GetJunctionsCopy() const
{
  return GetRelationCopy(junctionsTypeInfo, junctions);
}

Relation* JNetwork::GetRoutesCopy() const
{
  return GetRelationCopy(routesTypeInfo, routes);
}

Relation* JNetwork::GetSectionsCopy() const
{
  return GetRelationCopy(sectionsTypeInfo, sections);
}

void JNetwork::SetDefined(const bool def)
{
  nDef = def;
}

void JNetwork::SetId(const string nid)
{
  id = nid;
}


/*
1 Secondo Integration

*/

ListExpr JNetwork::Out(ListExpr typeInfo, Word value)
{
  if (DEBUG) cout << "JNetwork::Out" << endl;
  
  JNetwork* source = (JNetwork*) value.addr;
  if (!source->IsDefined())
  {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  else
  {
    ListExpr netId = nl->StringAtom(source->GetId());
    ListExpr junclist = source->JunctionsToList();
    ListExpr sectlist = source->SectionsToList();
    ListExpr routelist = source->RoutesToList();
    return nl->FourElemList(netId, junclist, sectlist, routelist);
  }
}

Word JNetwork::In(const ListExpr typeInfo, const ListExpr instance,
                  const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if (DEBUG) cout << "JNetwork::In " << endl;
  JNetwork* net = new JNetwork(instance, errorPos, errorInfo, correct);
  if (correct)
    return SetWord(net);
  else
  {
    delete net;
    return SetWord(Address(0));
  }
}

Word JNetwork::Create(const ListExpr typeInfo)
{
  if (DEBUG) cout << "JNetwork::Create" << endl;
  return SetWord ( new JNetwork(false));
}

void JNetwork::Delete( const ListExpr typeInfo, Word& w )
{
  if (DEBUG) cout << "JNetwork::Delete" << endl;
  JNetwork* net = (JNetwork*) w.addr;
  delete net;
  w.addr = 0;
}

void JNetwork::Close( const ListExpr typeInfo, Word& w )
{
  if (DEBUG) cout << "JNetwork::Close" << endl;
  delete static_cast<JNetwork*> ( w.addr );
  w.addr = 0;
}

Word JNetwork::Clone( const ListExpr typeInfo, const Word& w )
{
  if (DEBUG) cout << "JNetwork::Clone" << endl;
  JNetwork* clone = new JNetwork(*((JNetwork*)w.addr));
  clone->SetId(clone->GetId()+"clone");
  return SetWord (clone);
}

void* JNetwork::Cast( void* addr )
{
  if (DEBUG) cout << "JNetwork::Cast" << endl;
  return (new (addr) JNetwork);
}

bool JNetwork::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  if (DEBUG) cout << "JNetwork::KindCheck" << endl;
  return checkType(type);
}

int JNetwork::SizeOf()
{
  if (DEBUG) cout << "JNetwork::SizeOf" << endl;
  return sizeof(JNetwork);
}

bool JNetwork::Save(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo, Word& value)
{
  if (DEBUG) cout << "JNetwork::Save(...value)" << endl;
  JNetwork* source = (JNetwork*) value.addr;
  return source->Save(valueRecord, offset, typeInfo);
}

bool JNetwork::Save(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr  typeInfo)
{
  if (DEBUG) cout << "JNetwork::Save" << endl;
  Word w;
  w.setAddr(new CcString(true,id));
  ListExpr idLE;
  nl->ReadFromString(CcString::BasicType(), idLE);
  ListExpr numId = SecondoSystem::GetCatalog()->NumericType(idLE);
  if (!SaveAttribute<CcString>(valueRecord, offset, numId, w))
    return false;

  ListExpr relType;
  nl->ReadFromString ( junctionsTypeInfo, relType );
  ListExpr relNumericType =
    SecondoSystem::GetCatalog()->NumericType ( relType );
  if (!junctions->Save(valueRecord, offset, relNumericType))
    return false;

  nl->ReadFromString(sectionsTypeInfo, relType);
  relNumericType = SecondoSystem::GetCatalog()->NumericType(relType);
  if (!sections->Save(valueRecord, offset, relNumericType))
    return false;

  nl->ReadFromString(routesTypeInfo, relType);
  relNumericType = SecondoSystem::GetCatalog()->NumericType(relType);
  if (!routes->Save(valueRecord, offset, relNumericType))
    return false;

  nl->ReadFromString ( sectionsBTreeTypeInfo, relType );
  relNumericType =SecondoSystem::GetCatalog()->NumericType ( relType );
  if (!sectionsBTree->Save( valueRecord, offset, relNumericType))
    return false;

  if (!sectionsRTree->Save( valueRecord, offset))
    return false;

  nl->ReadFromString ( junctionsBTreeTypeInfo, relType );
  relNumericType =SecondoSystem::GetCatalog()->NumericType ( relType );
  if (!junctionsBTree->Save( valueRecord, offset, relNumericType))
    return false;

  if (!junctionsRTree->Save( valueRecord, offset))
    return false;

  nl->ReadFromString ( routesBTreeTypeInfo, relType );
  relNumericType =SecondoSystem::GetCatalog()->NumericType ( relType );
  if (!routesBTree->Save( valueRecord, offset, relNumericType))
    return false;

  return true;
}

JNetwork* JNetwork::Open(SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo)
{
  return new JNetwork(valueRecord, offset, typeInfo);
}

bool JNetwork::Open(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo, Word& value )
{
  if (DEBUG) cout << "JNetwork::Open(..value)" << endl;

  value.addr = JNetwork::Open(valueRecord, offset, typeInfo);
  return value.addr != 0;
}

/*
1 Standard Operations

*/

JNetwork& JNetwork::operator=(const JNetwork& net)
{
  if (DEBUG) cout << "JNetwork::operator=" << endl;
  nDef = net.IsDefined();
  if (net.IsDefined())
  {
    id = net.GetId();
    junctions = net.GetJunctions();
    sections = net.GetSections();
    routes = net.GetRoutes();
    junctionsBTree = net.junctionsBTree;
    sectionsBTree = net.sectionsBTree;
    routesBTree = net.routesBTree;
    junctionsRTree = net.junctionsRTree;
    sectionsRTree = net.sectionsRTree;
  }
  return *this;
}

bool JNetwork::operator==(const JNetwork& net) const
{
  if (DEBUG) cout << "JNetwork::operator==" << endl;
  if (Compare(net) == 0) return true;
  else return false;
}

int JNetwork::Compare(const JNetwork& net) const
{
  if (DEBUG) cout << "JNetwork::compare" << endl;
  if (!IsDefined() && !net.IsDefined()) return 0;
  if (!IsDefined() && net.IsDefined()) return -1;
  if (IsDefined() && !net.IsDefined()) return 1;
  return id.compare(net.GetId());
}

ostream& JNetwork::Print(ostream& os) const
{
  if (DEBUG) cout << "JNetwork::Print" << endl;
  os << "Network: ";
  if (!IsDefined())
    os << Symbol::UNDEFINED();
  else
  {
    os << "Id: " << id << endl;

    os << "Junctions: " << endl;
    junctions->Print(os);

    os << "Sections: " << endl;
    sections->Print(os);

    os << "Routes: " << endl;
    routes->Print(os);
  }
  os << endl;
  return os;
}

const string JNetwork::BasicType()
{
  return "jnetwork";
}

const bool JNetwork::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}
/*
1 ~CreateNetwork~

*/

void JNetwork::CreateNetwork(const string netid, const Relation* juncRel,
                             const Relation* routesRel)
{
  if (DEBUG) cout << "JNetwork::CreateNetwork" << endl;
  id = netid;
  nDef = true;

  InitJunctions(juncRel);
  junctionsBTree = CreateBTree(junctions, junctionsTypeInfo, (string) "id");

  //Initialize routes and sections relation
  InitRoutesAndSections(routesRel);
  routesBTree = CreateBTree(routes, routesTypeInfo, (string)"id");
  sectionsBTree = CreateBTree(sections, sectionsTypeInfo, (string) "id");

  //update respectively fill remaining list fields in relations
  UpdateJunctions();
  UpdateSections();

  //Build spatial indices
  junctionsRTree = CreateRTree(junctions, junctionsTypeInfo, (string) "pos");
  sectionsRTree = CreateRTree(sections, sectionsTypeInfo, (string) "curve");
}

/*
1 Relation Descriptors

*/

string JNetwork::sectionsTypeInfo = "(rel ( tuple ( (id int) (curve sline)"
  "(startjunc tid) (endjunc tid) (listsectrint listptidrint) "
  "(listadjsectup jlisttid) (listadjsectdown jlisttid) "
  "(listrevadjsectup jlisttid) (listrevadjsectdown jlisttid)"
  "(lenth real) (vmax real) (sectdir jdirection))))";

string JNetwork::junctionsTypeInfo = "(rel (tuple( (id int) (pos point)"
  "(listjuncpos listptidrloc) (listinsections jlisttid)"
  "(listoutsections jlisttid) (listdistances listnetdistgrp))))";

string JNetwork::routesTypeInfo = "(rel (tuple ( (id int) "
  "(listjunctions listptidrloc) (listsections listptidrint)"
  "(lenth real))))";

string JNetwork::sectionsBTreeTypeInfo = "(btree(tuple((id int)"
  "(curve sline) (startjunc tid) (endjunc tid) (listsectrint listptidrint) "
  "(listadjsectup jlisttid) (listadjsectdown jlisttid) "
  "(listrevadjsectup jlisttid) (listrevadjsectdown jlisttid)"
  "(lenth real) (vmax real) (sectdir jdirection))) int)";

string JNetwork::sectionsRTreeTypeInfo = "(rtree ( tuple ((id int)"
  "(curve sline) (startjunc tid) (endjunc tid) (listsectrint listptidrint) "
  "(listadjsectup jlisttid) (listadjsectdown jlisttid) "
  "(listrevadjsectup jlisttid) (listrevadjsectdown jlisttid)"
  "(lenth real) (vmax real) (sectdir jdirection))) sline FALSE)";

string JNetwork::junctionsBTreeTypeInfo = "(btree (tuple((id int)"
  "(pos point) (listjuncpos listptidrloc) (listinsections jlisttid)"
  "(listoutsections jlisttid) (listdistances listnetdistgrp))) int)";

string JNetwork::junctionsRTreeTypeInfo = "(rtree (tuple((id int) "
  "(pos point) (listjuncpos listptidrloc) (listinsections jlisttid)"
  "(listoutsections jlisttid) (listdistances listnetdistgrp))) point FALSE)";

string JNetwork::routesBTreeTypeInfo = "(btree (tuple ( (id int) "
  "(listjunctions listptidrloc) (listsections listptidrint)"
  "(lenth real))) int)";


/*
1 Private functions

1.1 Build ListExpr for Out-Function of Network

*/

ListExpr JNetwork::JunctionsToList() const
{
  if (DEBUG) cout << "JNetwork::JunctionsToList" << endl;
  return RelationToList(junctions, junctionsTypeInfo);
}

ListExpr JNetwork::SectionsToList() const
{
  if (DEBUG) cout << "JNetwork::SectionsToList" << endl;
  return RelationToList(sections, sectionsTypeInfo);
}

ListExpr JNetwork::RoutesToList() const
{
  if (DEBUG) cout << "JNetwork::RouteToList" << endl;
  return RelationToList(routes, routesTypeInfo);
}

/*
1.2 Access to Internal Relations

*/

Relation* JNetwork::GetJunctions() const
{
  return junctions;
}

Relation* JNetwork::GetSections() const
{
  return sections;
}

Relation* JNetwork::GetRoutes() const
{
  return routes;
}

BTree* JNetwork::GetSectionsBTree() const
{
  return sectionsBTree;
}

R_Tree< 2, TupleId >* JNetwork::GetSectionsRTree() const
{
  return sectionsRTree;
}

BTree* JNetwork::GetJunctionsBTree() const
{
  return junctionsBTree;
}

R_Tree< 2, TupleId >* JNetwork::GetJunctionsRTree() const
{
  return junctionsRTree;
}

BTree* JNetwork::GetRoutesBTree() const
{
  return routesBTree;
}

/*
1 Initialize And Update Relations

1.1 Initialize Relations

1.1.1 junctions

*/

void JNetwork::InitJunctions(const Relation* junRel)
{
  if (DEBUG) cout << "JNetwork::InitJunctions" << endl;
  ListExpr junctionsNumType;
  junctions = CreateRelation(junctionsTypeInfo, junctionsNumType);
  GenericRelationIterator* itJuncRel = junRel->MakeScan();
  ListPTIDRLoc* listRLoc = new ListPTIDRLoc(true);
  JListTID* listInSections = new JListTID(true);
  JListTID* listOutSections = new JListTID(true);
  ListNetDistGrp* listDistances = new ListNetDistGrp(true);
  int actJunctionId = -1;
  int curJunctionId = -1;
  bool first = true;
  Point* actPos = 0;
  Tuple* actJunctionTup = 0;
  while ((actJunctionTup = itJuncRel->GetNextTuple()) != 0)
  {
    actJunctionId = ((CcInt*)actJunctionTup->GetAttribute(0))->GetIntval();
    if (curJunctionId != actJunctionId)
    {
      //next new junction
      if (!first)
      {
        //write result tuple
        WriteJunctionTuple(curJunctionId, actPos, listRLoc, listInSections,
                           listOutSections, listDistances, junctionsNumType);

        //Reinitialize result tuple values
        listRLoc = new ListPTIDRLoc(true);
        listInSections = new JListTID(true);
        listOutSections = new JListTID(true);
        listDistances = new ListNetDistGrp(true);;
      }
      first = false;
      //initialize result tuple values
      curJunctionId = actJunctionId;
      actPos = new Point(*(Point*)actJunctionTup->GetAttribute(1));
      listRLoc->Append(PairTIDRLoc(TupleIdentifier(false,0),
          RouteLocation(((CcInt*)actJunctionTup->GetAttribute(2))->GetIntval(),
                      ((CcReal*)actJunctionTup->GetAttribute(3))->GetRealval(),
                      Both)));
    }
    else
    {
      //same junction
      listRLoc->Append(PairTIDRLoc(TupleIdentifier(false,0),
        RouteLocation(((CcInt*)actJunctionTup->GetAttribute(2))->GetIntval(),
                      ((CcReal*)actJunctionTup->GetAttribute(3))->GetRealval(),
                       Both)));
    }
    actJunctionTup->DeleteIfAllowed();
    actJunctionTup = 0;
  }
  // write last tuple
  WriteJunctionTuple(curJunctionId, actPos, listRLoc, listInSections,
                     listOutSections, listDistances, junctionsNumType);

  //cleanup
  delete itJuncRel;
  itJuncRel = 0;
}

/*
1.1.2 Routes and Sections

*/

void JNetwork::InitRoutesAndSections(const Relation* routesRel)
{
  if (DEBUG) cout << "JNetwork::InitRouteAndSections" << endl;
  ListExpr routesNumType;
  ListExpr sectionsNumType;
  routes = CreateRelation(routesTypeInfo,routesNumType);
  sections = CreateRelation(sectionsTypeInfo, sectionsNumType);
  //Fill routes and sections relation with initial values
  GenericRelationIterator* itRoutesRel = routesRel->MakeScan();
  int secId = 1;
  TupleId secTID = 0;
  Tuple* actRouteTup = 0;
  int curJid = -1;
  TupleId curJunTID = 0;
  double curJuncPosOnRoute = 0.0;
  double curRouteLength = 0.0;
  int curRouteId = -1;
  SimpleLine* curRouteCurve = 0;
  JSide dir = Both;
  ListPTIDRLoc* juncList = new ListPTIDRLoc(true);
  ListPTIDRInt* sectList = new ListPTIDRInt(true);
  double curMaxSpeed = 0.0;
  bool first = true;
  while ((actRouteTup = itRoutesRel->GetNextTuple()) != 0)
  {
    int actRouteId = ((CcInt*)actRouteTup->GetAttribute(0))->GetIntval();
    if (curRouteId != actRouteId)
    {
      if (!first)
      {
        //Write Tuple to routes Relation
        WriteRoutesTuple(curRouteId, curRouteLength, juncList, sectList,
                         routesNumType);
        juncList = new ListPTIDRLoc(true);
        sectList = new ListPTIDRInt(true);
      }
      first = false;
      //initialize result tuple values
      curRouteId = actRouteId;
      if (curRouteCurve != 0) curRouteCurve->DeleteIfAllowed();
      curRouteCurve =
        new SimpleLine(*(SimpleLine*)actRouteTup->GetAttribute(4));
      curRouteLength = curRouteCurve->Length();
      curMaxSpeed =
         (double)(((CcInt*)actRouteTup->GetAttribute(3))->GetIntval());
      curJid = ((CcInt*)actRouteTup->GetAttribute(1))->GetIntval();
      curJunTID = GetJunctionTupleId(curJid);
      curJuncPosOnRoute = ((CcReal*)actRouteTup->GetAttribute(2))->GetRealval();
      dir = ((Direction*) actRouteTup->GetAttribute(5))->GetDirection();
      juncList->Append(PairTIDRLoc(TupleIdentifier(true, curJunTID),
            RouteLocation(actRouteId, curJuncPosOnRoute, dir )));
    }
    else
    {
      // get values for new section tuple
      int actJid = ((CcInt*)actRouteTup->GetAttribute(1))->GetIntval();
      TupleId actJunTID = GetJunctionTupleId(actJid);
      SimpleLine* sectCurve = new SimpleLine(0);
      double actJuncPosOnRoute =
        ((CcReal*)actRouteTup->GetAttribute(2))->GetRealval();
      curRouteCurve->SubLine(curJuncPosOnRoute,
                             actJuncPosOnRoute,
                             curRouteCurve->GetStartSmaller(),
                             *sectCurve);
      dir = ((Direction*) actRouteTup->GetAttribute(5))->GetDirection();
      //write new section tuple
      WriteSectionTuple(secId, sectCurve, curJunTID, actJunTID, actRouteId,
                        curJuncPosOnRoute, actJuncPosOnRoute, dir, curMaxSpeed,
                        sectionsNumType);
      secTID++;
      //update lists of routes relation
      juncList->Append(PairTIDRLoc(TupleIdentifier(true,actJunTID),
                                 RouteLocation(curRouteId, actJuncPosOnRoute,
                                               Direction(dir))));
      sectList->Append(PairTIDRInterval(TupleIdentifier(true,secTID),
                          JRouteInterval(actRouteId, curJuncPosOnRoute,
                                         actJuncPosOnRoute, Direction(dir))));

      //Init values for next section
      secId++;
      curJuncPosOnRoute = actJuncPosOnRoute;
      curJid = actJid;
      curJunTID = actJunTID;
    }
    actRouteTup->DeleteIfAllowed();
    actRouteTup = 0;
  }

  //write last tuple
  WriteRoutesTuple(curRouteId, curRouteLength, juncList, sectList,
                   routesNumType);
  if (curRouteCurve != 0) curRouteCurve->DeleteIfAllowed();
  delete itRoutesRel;
  itRoutesRel = 0;
}

/*
1Update Relations

1.1 junctions

*/

void JNetwork::UpdateJunctions()
{
  if (DEBUG) cout << "JNetwork::UpdateJunctions" << endl;
  GenericRelationIterator* jit = junctions->MakeScan();
  Tuple* actJunction = 0;
  while ((actJunction = jit->GetNextTuple()) != 0 )
  {
    vector<int> indices;
    vector<Attribute*> attrs;
    ListPTIDRLoc* listRLocOld =
      (ListPTIDRLoc*) actJunction->GetAttribute(JUNC_LIST_ROUTEPOSITIONS);
    ListPTIDRLoc* listRLocNew = new ListPTIDRLoc(true);
    JListTID* listInSections = new JListTID(true);
    JListTID* listOutSections = new JListTID(true);
    PairTIDRLoc pairRLoc;
    for (int i = 0; i < listRLocOld->GetNoOfComponents(); i++)
    {
      //Update list of route locations with tuple ids
      listRLocOld->Get(i, pairRLoc);
      TupleId rtid =
        GetRoutesTupleId((pairRLoc.GetRouteLocation()).GetRouteId());
      pairRLoc.SetTID(TupleIdentifier(true, rtid));
      listRLocNew->Append(pairRLoc);
      //Fill Lists of in and out sections
      Tuple* actRoute = routes->GetTuple(rtid,false);
      ListPTIDRInt* listRouteSections =
      (ListPTIDRInt*) actRoute->GetAttribute(ROUTE_LIST_SECTIONS);
      PairTIDRInterval actRouteInterval;
      for (int j = 0; j < listRouteSections->GetNoOfComponents(); j++)
      {
        listRouteSections->Get(j,actRouteInterval);
        if (AlmostEqual(actRouteInterval.GetRouteInterval().GetStartPosition(),
                        pairRLoc.GetRouteLocation().GetPosition()))
        {
          if (actRouteInterval.GetRouteInterval().GetSide() == (JSide) Up ||
            actRouteInterval.GetRouteInterval().GetSide() == (JSide) Both)
            listOutSections->Append(actRouteInterval.GetTID());
          if (actRouteInterval.GetRouteInterval().GetSide() == (JSide) Down ||
            actRouteInterval.GetRouteInterval().GetSide() == (JSide) Both)
            listInSections->Append(actRouteInterval.GetTID());
        }
        if (AlmostEqual(actRouteInterval.GetRouteInterval().GetEndPosition(),
            pairRLoc.GetRouteLocation().GetPosition()))
        {
          if (actRouteInterval.GetRouteInterval().GetSide() == (JSide) Up ||
            actRouteInterval.GetRouteInterval().GetSide() == (JSide) Both)
            listInSections->Append(actRouteInterval.GetTID());
          if (actRouteInterval.GetRouteInterval().GetSide() == (JSide) Down ||
            actRouteInterval.GetRouteInterval().GetSide() == (JSide) Both)
            listOutSections->Append(actRouteInterval.GetTID());
        }
      }
      actRoute->DeleteIfAllowed();
      actRoute = 0;
    }

    //update tuple
    indices.push_back(JUNC_LIST_ROUTEPOSITIONS );
    attrs.push_back(listRLocNew);
    indices.push_back(JUNC_LIST_INSECTIONS);
    attrs.push_back(listInSections);
    indices.push_back(JUNC_LIST_OUTSECTIONS);
    attrs.push_back(listOutSections);
    junctions->UpdateTuple(actJunction, indices, attrs);

    //cleanup memory

    actJunction->DeleteIfAllowed();
    actJunction = 0;
  }
  delete jit;
  jit = 0;
}

/*
1.2 sections

*/

void JNetwork::UpdateSections()
{
  if (DEBUG) cout << "JNetwork::UpdateSections" << endl;
  GenericRelationIterator* sit = sections->MakeScan();
  Tuple* actSection = 0;
  while ((actSection = sit->GetNextTuple()) != 0 )
  {
    vector<int> indices;
    vector<Attribute*> attrs;
    ListPTIDRInt* listSectOld =
      (ListPTIDRInt*) actSection->GetAttribute(SEC_LIST_ROUTEINTERVALS);
    ListPTIDRInt* listSectNew = new ListPTIDRInt(true);
    PairTIDRInterval actPTIDRI;
    for (int i = 0 ; i < listSectOld->GetNoOfComponents(); i++)
    {
      listSectOld->Get(i, actPTIDRI);
      actPTIDRI.SetTID(
        GetRoutesTupleId(actPTIDRI.GetRouteInterval().GetRouteId()));
      listSectNew->Append(actPTIDRI);
    }
    JListTID* listAdjSecUp = new JListTID(true);
    JListTID* listAdjSecDown = new JListTID(true);
    JListTID* listRevAdjSecUp = new JListTID(true);
    JListTID* listRevAdjSecDown = new JListTID(true);
    //get adjaceny values
    TupleId startJunctTID =
      ((TupleIdentifier*)actSection->GetAttribute(SEC_TID_STARTNODE))->GetTid();
    Tuple* startJunction = junctions->GetTuple(startJunctTID,false);
    JListTID* listStartJuncInSect =
      (JListTID*)startJunction->GetAttribute(JUNC_LIST_INSECTIONS);
    JListTID* listStartJuncOutSect =
      (JListTID*)startJunction->GetAttribute(JUNC_LIST_OUTSECTIONS);
    Tuple* endJunction = junctions->GetTuple(((TupleIdentifier*)
      actSection->GetAttribute(SEC_TID_ENDNODE))->GetTid(), false);
    JListTID* listEndJuncInSect =
      (JListTID*)endJunction->GetAttribute(JUNC_LIST_INSECTIONS);
    JListTID* listEndJuncOutSect =
      (JListTID*)endJunction->GetAttribute(JUNC_LIST_OUTSECTIONS);
    Direction* side = (Direction*)actSection->GetAttribute(SEC_DIRECTION);
    // fill section lists.
    TupleIdentifier tid;
    if (side->operator==(Direction(Up)) || side->operator==(Direction(Both)))
    {
      for (int j = 0; j < listStartJuncInSect->GetNoOfComponents(); j++)
      {
        listStartJuncInSect->Get(j,tid);
        listRevAdjSecUp->Append(tid);
      }
      for (int j = 0; j < listEndJuncOutSect->GetNoOfComponents(); j++)
      {
        listEndJuncOutSect->Get(j,tid);
        listAdjSecUp->Append(tid);
      }
    }
    if (side->operator==(Direction(Down))|| side->operator==(Direction(Both)))
    {
      for (int j = 0; j < listEndJuncInSect->GetNoOfComponents(); j++)
      {
        listEndJuncInSect->Get(j,tid);
        listRevAdjSecDown->Append(tid);
      }
      for (int j = 0; j < listStartJuncOutSect->GetNoOfComponents();j++)
      {
        listStartJuncOutSect->Get(j,tid);
        listAdjSecDown->Append(tid);
      }
    }
    //update list entries in the sections tuple
    indices.push_back(SEC_LIST_ROUTEINTERVALS);
    attrs.push_back(listSectNew);
    indices.push_back(SEC_LIST_ADJSECTIONS_UP);
    attrs.push_back(listAdjSecUp);
    indices.push_back(SEC_LIST_ADJSECTIONS_DOWN);
    attrs.push_back(listAdjSecDown);
    indices.push_back(SEC_LIST_REV_ADJSECTIONS_UP);
    attrs.push_back(listRevAdjSecUp);
    indices.push_back(SEC_LIST_REV_ADJSECTIONS_DOWN);
    attrs.push_back(listRevAdjSecDown);
    sections->UpdateTuple(actSection, indices, attrs);
    //clean up memory

    startJunction->DeleteIfAllowed();
    startJunction = 0;
    endJunction->DeleteIfAllowed();
    endJunction = 0;
    actSection->DeleteIfAllowed();
    actSection = 0;
  }
  delete sit;
  sit = 0;
}

/*
1 Create Relation from string type descriptor

*/

Relation* JNetwork::CreateRelation(const string descriptor, ListExpr& numType)
{
  if (DEBUG) cout << "JNetwork::CreateRelation:" << endl;
    ListExpr xType;
    nl->ReadFromString ( descriptor, xType );
    numType = SecondoSystem::GetCatalog()->NumericType ( xType );
    return new Relation(numType, false);
}

/*
1 Creates Trees

*/

void JNetwork::CreateTrees()
{
  sectionsBTree = CreateBTree(sections, sectionsTypeInfo, "id");
  sectionsRTree = CreateRTree(sections, sectionsTypeInfo, "curve");
  junctionsBTree = CreateBTree(junctions, junctionsTypeInfo, "id");
  junctionsRTree = CreateRTree(junctions, junctionsTypeInfo, "pos");
  routesBTree = CreateBTree(routes, routesTypeInfo,"id");
}

/*
1.1 BTree from relation type descriptor and attr name

*/

BTree* JNetwork::CreateBTree(const Relation* rel, const string descriptor,
                             const string attr)
{
  if (DEBUG) cout << "JNetwork::CreateBTree: " << endl;
  ListExpr relPtr = listutils::getPtrList(rel);
  string strQuery = "(createbtree (" + descriptor +
                    " (ptr " + nl->ToString(relPtr) + "))" + attr + ")";
  Word w;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, w);
  assert ( QueryExecuted );
  return ( BTree* ) w.addr;
}

/*
1.2 RTree from relation, descriptor and attr

*/

R_Tree<2,TupleId>* JNetwork::CreateRTree(const Relation* rel,
                                         const string descriptor,
                                         const string attr)
{
  if (DEBUG) cout << "JNetwork::CreateRTree:" << endl;
  ListExpr relPtr = listutils::getPtrList(rel);
  string strQuery = "(bulkloadrtree(sortby(addid(feed (" + descriptor +
           " (ptr " + nl->ToString(relPtr) + "))))((" + attr +" asc)))" +
           attr + " TID)";
  Word w;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( strQuery, w );
  assert ( QueryExecuted );
  return ( R_Tree<2,TupleId>* ) w.addr;
}

/*
1 Open Relations and Trees

11 Relation

*/

Relation* JNetwork::OpenRelation(const std::string descriptor,
                                 SmiRecord& valueRecord, size_t& offset)
{
  if (DEBUG) cout << "JNetwork::OpenRelation:" << endl;
  ListExpr relType;
  nl->ReadFromString ( descriptor, relType );
  ListExpr relNumericType = SecondoSystem::GetCatalog()->NumericType(relType);
  return Relation::Open(valueRecord,offset,relNumericType);
}

/*
1.2 BTree

*/

BTree* JNetwork::OpenBTree(const std::string descriptor,
                           SmiRecord& valueRecord, size_t& offset)
{
  if (DEBUG) cout << "JNetwork::OpenBTree: " << endl;
  ListExpr treeType;
  nl->ReadFromString(descriptor,treeType);
  ListExpr treeNumType = SecondoSystem::GetCatalog()->NumericType(treeType);
  return BTree::Open(valueRecord,offset,treeNumType);
}


/*
1 Return TupleId for identifier from BTree

*/

TupleId JNetwork::GetTupleId(BTree* tree, const int ident)const
{
  if (DEBUG) cout << "JNetwork::GetTupleId: " << endl;
  CcInt* identify = new CcInt(true, ident);
  BTreeIterator* bit = tree->ExactMatch ( identify );
  int nextIter = bit->Next();
  assert ( nextIter );
  TupleId result = bit->GetId();
  delete bit;
  bit = 0;
  identify->DeleteIfAllowed();
  identify = 0;
  return result;
}

/*
1.1 junction id

*/

TupleId JNetwork::GetJunctionTupleId(const int jid) const
{
  if (DEBUG) cout << "JNetwork::GetJunctionTupleId: " << endl;
  return GetTupleId(junctionsBTree, jid);
}

/*
1.2 routes id

*/

TupleId JNetwork::GetRoutesTupleId(const int rid) const
{
 if (DEBUG) cout << "JNetwork::GetRoutesTupleId: " << endl;

 return GetTupleId(routesBTree, rid);
}

/*
1.3 sections id

*/

TupleId JNetwork::GetSectionsTupleId(const int sid) const
{
  if (DEBUG) cout << "JNetwork::GetSectionsTupleId: " << endl;

  return GetTupleId(sectionsBTree, sid);
}

/*
1 Write Tuple to Relation

1.1 Junction

*/

void JNetwork::WriteJunctionTuple(const int jid, Point* pos,
                                  ListPTIDRLoc* listRLoc,
                                  JListTID* listinsect,
                                  JListTID* listoutsect,
                                  ListNetDistGrp* listdist,
                                  const ListExpr& juncNumType)
{
  Tuple* newJunctionTup = new Tuple(nl->Second(juncNumType));
  newJunctionTup->PutAttribute(JUNC_ID, new CcInt(true, jid));
  newJunctionTup->PutAttribute(JUNC_POS, pos);
  newJunctionTup->PutAttribute(JUNC_LIST_ROUTEPOSITIONS, listRLoc);
  newJunctionTup->PutAttribute(JUNC_LIST_INSECTIONS, listinsect);
  newJunctionTup->PutAttribute(JUNC_LIST_OUTSECTIONS, listoutsect);
  newJunctionTup->PutAttribute(JUNC_LIST_NETDISTANCES, listdist);
  junctions->AppendTuple(newJunctionTup);
  newJunctionTup->DeleteIfAllowed();
  newJunctionTup = 0;
}

/*
1.1 Routes

*/

void JNetwork::WriteRoutesTuple(const int rid,
                                const double length,
                                ListPTIDRLoc* listjunc,
                                ListPTIDRInt* listsect,
                                const ListExpr routesNumType)
{
  Tuple* newRouteTup = new Tuple(nl->Second(routesNumType));
  newRouteTup->PutAttribute(ROUTE_ID, new CcInt(true, rid));
  newRouteTup->PutAttribute(ROUTE_LENGTH, new CcReal(true, length));
  newRouteTup->PutAttribute(ROUTE_LIST_JUNCTIONS, listjunc);
  newRouteTup->PutAttribute(ROUTE_LIST_SECTIONS, listsect);
  routes->AppendTuple(newRouteTup);
  newRouteTup->DeleteIfAllowed();
  newRouteTup = 0;
}

/*
1.1 Sections

*/

void JNetwork::WriteSectionTuple(const int sectId,
                                 SimpleLine* curve,
                                 const TupleId& curJunTID,
                                 const TupleId& actJunTID,
                                 const int actRouteId,
                                 const double curJuncPosOnRoute,
                                 const double actJuncPosOnRoute,
                                 const JSide dir,
                                 const double curMaxSpeed,
                                 const ListExpr& sectionsNumType)
{
  Tuple* actSectTup = new Tuple(nl->Second(sectionsNumType));
  actSectTup->PutAttribute(SEC_ID, new CcInt(true, sectId));
  actSectTup->PutAttribute(SEC_CURVE, curve);
  actSectTup->PutAttribute(SEC_TID_STARTNODE,
                           new TupleIdentifier (true, curJunTID));
  actSectTup->PutAttribute(SEC_TID_ENDNODE,
                           new TupleIdentifier(true, actJunTID));
  actSectTup->PutAttribute(SEC_LIST_ROUTEINTERVALS,
                           new ListPTIDRInt(
                              PairTIDRInterval(TupleIdentifier(false,0),
                                               JRouteInterval(actRouteId,
                                                           curJuncPosOnRoute,
                                                           actJuncPosOnRoute,
                                                           Direction(dir)))));
  actSectTup->PutAttribute(SEC_LIST_ADJSECTIONS_UP, new JListTID(true));
  actSectTup->PutAttribute(SEC_LIST_ADJSECTIONS_DOWN, new JListTID(true));
  actSectTup->PutAttribute(SEC_LIST_REV_ADJSECTIONS_UP, new JListTID(true));
  actSectTup->PutAttribute(SEC_LIST_REV_ADJSECTIONS_DOWN, new JListTID(true));
  actSectTup->PutAttribute(SEC_LENGTH, new CcReal(true, curve->Length()));
  actSectTup->PutAttribute(SEC_VMAX, new CcReal(true, curMaxSpeed));
  actSectTup->PutAttribute(SEC_DIRECTION, new Direction(dir));
  sections->AppendTuple(actSectTup);
  actSectTup->DeleteIfAllowed();
  actSectTup = 0;
}

/*
1 Access internal Relations

Some helpful tools to transform internal relations

1.1 Copy

*/

Relation* JNetwork::GetRelationCopy(const string relTypeInfo,
                                    Relation* relPointer) const
{
  ListExpr strPtr = listutils::getPtrList(relPointer);
  string querystring = "(consume (feed (" + relTypeInfo +
                       " (ptr " + nl->ToString(strPtr) + "))))";
  Word resultWord;
  int QueryExecuted = QueryProcessor::ExecuteQuery ( querystring, resultWord );
  assert ( QueryExecuted );
  return ( Relation * ) resultWord.addr;
}

/*
1.2 ToList

*/

ListExpr JNetwork::RelationToList(Relation* rel, const string relTypeInfo) const
{
  GenericRelationIterator* it = rel->MakeScan();
  ListExpr typeInfo = nl->TheEmptyList();
  nl->ReadFromString(relTypeInfo, typeInfo);
  ListExpr relList = Relation::Out(typeInfo, it);
  return relList;
}