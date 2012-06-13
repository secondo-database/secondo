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

#include <string>
#include "SecondoCatalog.h"
#include "QueryProcessor.h"
#include "Attribute.h"
#include "AlgebraTypes.h"
#include "Operator.h"
#include "ConstructorTemplates.h"
#include "GenericTC.h"
#include "TypeMapUtils.h"
#include "ListUtils.h"
#include "Symbols.h"
#include "StandardTypes.h"
#include "JNetAlgebra.h"
#include "Direction.h"
#include "RouteLocation.h"
#include "JRouteInterval.h"
#include "NetDistanceGroup.h"
#include "JList.h"
#include "JNetwork.h"
#include "JPoint.h"
#include "JLine.h"
#include "RelationAlgebra.h"

using namespace std;
using namespace mappings;


extern NestedList* nl;
extern QueryProcessor* qp;

/*
1 Type Constructors

1.1 ~jdirection~

Tells if an network position is reachable from respectively allocated on
Up, Down or Both sides of the road.

*/

TypeConstructor jdirectionTC(
  Direction::BasicType(),
  Direction::Property,
  Direction::Out, Direction::In,
  0, 0,
  Direction::Create, Direction::Delete,
  OpenAttribute<Direction>,
  SaveAttribute<Direction>,
  Direction::Close, Direction::Clone,
  Direction::Cast,
  Direction::SizeOf,
  Direction::KindCheck);

/*
1.1 ~rloc~

Describes an position in a network by the road identifier the position is
allocated, the distance of the location from the start of the road and the
side of the road the place is reachable from respectively allocated on.

*/

TypeConstructor jrlocTC(
  RouteLocation::BasicType(),
  RouteLocation::Property,
  RouteLocation::Out, RouteLocation::In,
  0, 0,
  RouteLocation::Create, RouteLocation::Delete,
  OpenAttribute<RouteLocation>,
  SaveAttribute<RouteLocation>,
  RouteLocation::Close, RouteLocation::Clone,
  RouteLocation::Cast,
  RouteLocation::SizeOf,
  RouteLocation::KindCheck);

/*
1.1 ~jrint~

Describes an part of a route of the network by the road identifier of road the
interval is allocated, the distance of the start and the end point of the
interval from the start of the road and the side of the road covered by the
interval.

*/

TypeConstructor jrintTC(
  JRouteInterval::BasicType(),
  JRouteInterval::Property,
  JRouteInterval::Out, JRouteInterval::In,
  0, 0,
  JRouteInterval::Create, JRouteInterval::Delete,
  OpenAttribute<JRouteInterval>,
  SaveAttribute<JRouteInterval>,
  JRouteInterval::Close, JRouteInterval::Clone,
  JRouteInterval::Cast,
  JRouteInterval::SizeOf,
  JRouteInterval::KindCheck);

/*
1.1 ~netdistancegroup~

Consisting of the identifier of the source junction,
the identifier of the target junction, the identifier of next junction in the
path from source to target junction, the identifier of the next section in the
path to next junction and the distance to the target junction.

*/

TypeConstructor jndgTC(
  NetDistanceGroup::BasicType(),
  NetDistanceGroup::Property,
  NetDistanceGroup::Out, NetDistanceGroup::In,
  0, 0,
  NetDistanceGroup::Create, NetDistanceGroup::Delete,
  OpenAttribute<NetDistanceGroup>,
  SaveAttribute<NetDistanceGroup>,
  NetDistanceGroup::Close, NetDistanceGroup::Clone,
  NetDistanceGroup::Cast,
  NetDistanceGroup::SizeOf,
  NetDistanceGroup::KindCheck);


/*
1.1 ~listint~

Sorted list of integer values available as attribute in relations.

*/

TypeConstructor jlistintTC(
  JListInt::BasicType(),
  JListInt::Property,
  JListInt::Out, JListInt::In,
  0, 0,
  JListInt::Create, JListInt::Delete,
  OpenAttribute<JListInt>,
  SaveAttribute<JListInt>,
  JListInt::Close, JListInt::Clone,
  JListInt::Cast,
  JListInt::SizeOf,
  JListInt::KindCheck);

/*
1.1 ~listjrint~

Sorted list of ~jrint~ values available as attribute in relations.

*/

TypeConstructor jlistjrintTC(
  JListRInt::BasicType(),
  JListRInt::Property,
  JListRInt::Out, JListRInt::In,
  0, 0,
  JListRInt::Create, JListRInt::Delete,
  OpenAttribute<JListRInt>,
  SaveAttribute<JListRInt>,
  JListRInt::Close, JListRInt::Clone,
  JListRInt::Cast,
  JListRInt::SizeOf,
  JListRInt::KindCheck);

/*
1.1 ~listrloc~

Sorted list of ~rloc~ values available as attribute in relations.

*/

TypeConstructor jlistrlocTC(
  JListRLoc::BasicType(),
  JListRLoc::Property,
  JListRLoc::Out, JListRLoc::In,
  0, 0,
  JListRLoc::Create, JListRLoc::Delete,
  OpenAttribute<JListRLoc>,
  SaveAttribute<JListRLoc>,
  JListRLoc::Close, JListRLoc::Clone,
  JListRLoc::Cast,
  JListRLoc::SizeOf,
  JListRLoc::KindCheck);

/*
1.1 ~listndg~

Sorted list of ~ndg~ values available as attribute in relations.

*/

TypeConstructor jlistndgTC(
  JListNDG::BasicType(),
  JListNDG::Property,
  JListNDG::Out, JListNDG::In,
  0, 0,
  JListNDG::Create, JListNDG::Delete,
  OpenAttribute<JListNDG>,
  SaveAttribute<JListNDG>,
  JListNDG::Close, JListNDG::Clone,
  JListNDG::Cast,
  JListNDG::SizeOf,
  JListNDG::KindCheck);

/*
1.1 ~jnetwork~

JNetwork object consists of an defined flag, an id, three relations with the
network data, three BTree and two RTree indices. The content of the network
data object and the meaning is described in Network.h in the description of
class ~JNetwork~.

*/

TypeConstructor jnetworkTC(
  JNetwork::BasicType(),
  JNetwork::Property,
  JNetwork::Out, JNetwork::In,
  0, 0,
  JNetwork::Create, JNetwork::Delete,
  JNetwork::Open,
  JNetwork::Save,
  JNetwork::Close, JNetwork::Clone,
  JNetwork::Cast,
  JNetwork::SizeOf,
  JNetwork::KindCheck);

/*
1.1 ~jpoint~

Describes a single position in a given network. Consists of an network
identifier (~string~) and an ~rloc~ for the position in the  network.

*/

TypeConstructor jpointTC(
  JPoint::BasicType(),
  JPoint::Property,
  JPoint::Out, JPoint::In,
  0, 0,
  JPoint::Create, JPoint::Delete,
  OpenAttribute<JPoint>,
  SaveAttribute<JPoint>,
  JPoint::Close, JPoint::Clone,
  JPoint::Cast,
  JPoint::SizeOf,
  JPoint::KindCheck);

/*
1.1 ~jline~

Describes a region in the network. Consists of an ~string~ as network
identifier and an set of ~jrint~ describing the network part covered by the
region.

*/

TypeConstructor jlineTC(
  JLine::BasicType(),
  JLine::Property,
  JLine::Out, JLine::In,
  0, 0,
  JLine::Create, JLine::Delete,
  OpenAttribute<JLine>,
  SaveAttribute<JLine>,
  JLine::Close, JLine::Clone,
  JLine::Cast,
  JLine::SizeOf,
  JLine::KindCheck);

/*
1 Secondo Operators

1.1 ~creatjnet~

The operator ~createjnet~ creates an single network object from the given
ressources. It expects four arguments:
- an string object with the object name for the new jnetwork
- an relation with the junctions data.The tuples are expected to have the
  attribute data types (meaning):
  -- ~int~ (junction identifier)
  -- ~point~ (spatial position)
  -- ~listrloc~ (list of route locations in the network of the junction)
  -- ~listint~ (list of section identifiers of incoming sections)
  -- ~listint~ (list of section identifiers of the outgoing sections)
- an relation with the sections data. The tuples are expected to have the
  attributes (meaning):
  -- ~int~ (section identifier)
  -- ~sline~ (spatial curve of the section)
  -- ~int~ (identifier of start junction)
  -- ~int~ (identifier of end junction)
  -- ~jdirection~ (allowed moving direction)
  -- ~real~ (allowed maximum speed at this section)
  -- ~real~ (length of the section in meter)
  -- ~listrint~ (list of route intervals represented by this section)
  -- ~listint~ (list of adjacent section identifiers in up direction)
  -- ~listint~ (list of adjacent section identifiers in down direction)
  -- ~listint~ (list of reverse adjacent sections for up direction)
  -- ~listint~ (list of reverse adjacent sections for down direction)
- an relation with the routes data. The tuples are expected to have the
  attribute data types (meaning):
  -- ~int~ (route identifier)
  -- ~listint~ (list of junction identifiers of the junctions on this route)
  -- ~listint~ (list of section identifiers of this route)
  -- ~real~ (length of the route in meter)
  All this is checked by the type mapping.

The ValueMapping checks if the network identifier is available as object name
for the current database. If this is the case, the given network object is
created and stored in the database with the given object name and ~true~ is
returned, ~false~ elsewhere.

*/

ListExpr createjnetTM (ListExpr args)
{
  if (!nl->HasLength(args,4))
    return listutils::typeError("Four arguments expected.");

  ListExpr idList = nl->First(args);
  if (!listutils::isSymbol(idList, CcString::BasicType()))
    return listutils::typeError("First argument should be " +
                                CcString::BasicType());

  ListExpr juncList = nl->Second(args);
  if (!IsRelDescription(juncList))
    return listutils::typeError("Second argument must be an relation");

  ListExpr xType;
  nl->ReadFromString ( JNetwork::GetJunctionsRelationType(), xType );
  if (!CompareSchemas ( juncList, xType ))
    return (nl->SymbolAtom("First relation (junctions) has wrong schema." ));

  ListExpr sectList = nl->Third(args);
  if (!IsRelDescription(sectList))
    return listutils::typeError("Third argument must be an relation.");

  nl->ReadFromString ( JNetwork::GetSectionsRelationType(), xType );
  if (!CompareSchemas ( sectList, xType ))
    return (nl->SymbolAtom("Second relation (sections) has wrong schema."));

  ListExpr routesList = nl->Fourth(args);
  if (!IsRelDescription(routesList))
    return listutils::typeError("Fourth argument must be an relation.");

  nl->ReadFromString ( JNetwork::GetRoutesRelationType(), xType );
  if (!CompareSchemas ( routesList, xType ))
    return ( nl->SymbolAtom ( "Third relation (routes) has wrong schema." ) );

  //everything correct
  return nl->SymbolAtom(CcBool::BasicType());
}

int createjnetVM ( Word* args, Word& result, int message, Word& local,
                   Supplier s )
{
  result = qp->ResultStorage(s);
  CcBool* res = (CcBool*) result.addr;
  CcString* netP = (CcString*) args[0].addr;
  if (netP->IsDefined())
  {
    string netid = netP->GetValue();

    //check if netid is an allowed new object identifier in the database
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    if (sc->IsObjectName(netid))
    {
      cerr << netid << " is already defined." << endl;
      res->Set(true, false);
      return 0;
    }

    string errMsg = "error";
    if (!sc->IsValidIdentifier(netid, errMsg, true))
    {
      cerr << netid << "is not an valid identifier. " << errMsg << endl;
      res->Set(true, false);
      return 0;
    }

    if (sc->IsSystemObject(netid))
    {
      cerr << netid << " is a reserved name" << endl;
      res->Set(true, false);
      return 0;
    }
    //Create jnetwork
    Relation* juncRel = (Relation*) args[1].addr;
    Relation* sectRel = (Relation*) args[2].addr;
    Relation* routesRel = (Relation*) args[3].addr;

    if (juncRel != 0 && sectRel != 0 && routesRel != 0)
    {
      JNetwork* resNet = new JNetwork(netid, juncRel, sectRel, routesRel);
      //store new jnetwork in database
      Word netWord;
      netWord.setAddr(resNet);
      res->Set(true, sc->InsertObject(netid, "",
                                      nl->SymbolAtom(JNetwork::BasicType()),
                                      netWord, true));
      return 0;
    }
  }
  res->Set(true, false);
  return 0;
}

const string createjnetSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + CcString::BasicType() + " X " +
  JNetwork::GetJunctionsRelationType() + "X " +
  JNetwork::GetSectionsRelationType() + " X " +
  JNetwork::GetRoutesRelationType() + " -> " +
  CcBool::BasicType() + "</text--->"
  "<text>createjnet( <id> , <junctions relation> , <sections relation> , "+
  "<routes relation> ) </text--->"
  "<text>If the id is a possible object name in the database the operation"
  "creates the " + JNetwork::BasicType() + " with given data and object name "+
  "id and returns true, false otherwise.</text--->"
  "<text>query createjnet(testnet, juncrel, sectrel, routerel)</text--->))";

Operator createjnetOp("createjnet", createjnetSpec, createjnetVM,
                      Operator::SimpleSelect, createjnetTM);

/*
1.1 Create

1.1.1 list from stream and stream from list

The next operators create list X from stream of Y and stream of Y from list X.
X          | Y
=======================
~listint~  | ~int~
~listrloc~ | ~rloc~
~listjrint~| ~jrint~
~listndg~  | ~ndg~
~listint~  | ~listint~
~listrloc~ | ~listrloc~
~listjrint~| ~listjrint~
~listndg~  | ~listndg~

*/

/*
1.1.1.1 ~createlist~

Creates a list from type ~X~ from a stream of the corresponding datatype ~Y~

*/

ListExpr createlistTM (ListExpr args)
{
  ListExpr stream = nl->First(args);
  if(!listutils::isDATAStream(stream)){
    return listutils::typeError("Expects a DATA stream.");
  }

  ListExpr T = nl->Second(stream);

  if(listutils::isSymbol(T,CcInt::BasicType()) ||
     listutils::isSymbol(T,JListInt::BasicType()))
    return nl->SymbolAtom(JListInt::BasicType());

  if (listutils::isSymbol(T, RouteLocation::BasicType()) ||
      listutils::isSymbol(T, JListRLoc::BasicType()))
    return nl->SymbolAtom(JListRLoc::BasicType());

  if (listutils::isSymbol(T, JRouteInterval::BasicType()) ||
      listutils::isSymbol(T, JListRInt::BasicType()))
    return nl->SymbolAtom(JListRInt::BasicType());

  if(listutils::isSymbol(T, NetDistanceGroup::BasicType()) ||
     listutils::isSymbol(T, JListNDG::BasicType()))
    return nl->SymbolAtom(JListNDG::BasicType());

  return listutils::typeError("Expected " + Symbol::STREAM() + "of T" +
      " with T in: " + CcInt::BasicType() + ", " + RouteLocation::BasicType() +
      ", " + JRouteInterval::BasicType() + ", " + NetDistanceGroup::BasicType()
      + JListInt::BasicType() + ", "+ JListRLoc::BasicType()+ ", " +
      JListRInt::BasicType() + ", or " + JListNDG::BasicType() + ".");
}



template <class CListElem, class CList>
int createlistVM ( Word* args, Word& result, int message, Word& local,
                   Supplier s )
{
  result = qp->ResultStorage(s);
  CList* createRes = static_cast<CList*> (result.addr);
  createRes->Clear();
  createRes->StartBulkload();
  CListElem* curElem = 0;
  Word curAddr;
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, curAddr);
  while (qp->Received(args[0].addr))
  {
    curElem = (CListElem*) curAddr.addr;
    if (curElem != 0 && curElem->IsDefined())
      createRes->operator+=(*curElem);
    curElem->DeleteIfAllowed();
    qp->Request(args[0].addr, curAddr);
  }
  qp->Close(args[0].addr);
  createRes->EndBulkload();
  return 0;
}

ValueMapping createlistMap [] =
{
  createlistVM<CcInt, JListInt>,
  createlistVM<RouteLocation, JListRLoc>,
  createlistVM<JRouteInterval, JListRInt>,
  createlistVM<NetDistanceGroup, JListNDG>,
  createlistVM<JListInt, JListInt>,
  createlistVM<JListRLoc, JListRLoc>,
  createlistVM<JListRInt, JListRInt>,
  createlistVM<JListNDG, JListNDG>,
};

int createlistSelect ( ListExpr args )
{
  NList param(args);
  if (param.first().second() == CcInt::BasicType()) return 0;
  if (param.first().second() == RouteLocation::BasicType()) return 1;
  if (param.first().second() == JRouteInterval::BasicType()) return 2;
  if (param.first().second() == NetDistanceGroup::BasicType()) return 3;
  if (param.first().second() == JListInt::BasicType()) return 4;
  if (param.first().second() == JListRLoc::BasicType()) return 5;
  if (param.first().second() == JListRInt::BasicType()) return 6;
  if (param.first().second() == JListNDG::BasicType()) return 7;
  return -1; // this point should never been reached.
}

const string createlistSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + Symbol::STREAM() + "("+ CcInt::BasicType() + ") -> " +
  JListInt::BasicType() + ", " +
  Symbol::STREAM() + "("+ RouteLocation::BasicType() + ") -> " +
  JListRLoc::BasicType() + ", " +
  Symbol::STREAM() + "("+ JRouteInterval::BasicType() + ") -> " +
  JListRInt::BasicType() + ", " +
  Symbol::STREAM() + "("+ NetDistanceGroup::BasicType() + ") -> " +
  JListNDG::BasicType() +  ", " +
  Symbol::STREAM() + "("+ JListInt::BasicType() + ") -> " +
  JListInt::BasicType() +  ", " +
  Symbol::STREAM() + "("+ JListRLoc::BasicType() + ") -> " +
  JListRLoc::BasicType() +  ", " +
  Symbol::STREAM() + "("+ JListRInt::BasicType() + ") -> " +
  JListRInt::BasicType() +  ", " +
  Symbol::STREAM() + "("+ JListNDG::BasicType() + ") -> " +
  JListNDG::BasicType() + " </text--->"
  "<text>_ createlist </text--->"
  "<text>Collects the values of a stream of type T into an single list" +
  "of the corresponding list data type.</text--->"
  "<text>query createstream(testlistint) createlist</text--->))";

Operator createlistOp("createlist", createlistSpec, 8, createlistMap,
                      createlistSelect, createlistTM);

/*
1.1.1.1 ~createstream~

Creates a stream of data type ~Y~ from a list from type ~X~

*/

ListExpr createstreamTM (ListExpr args)
{
  NList param(args);
  if (param.length()==1)
  {
    if (param.first().isEqual(JListInt::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(CcInt::BasicType()));

    if (param.first().isEqual(JListRLoc::BasicType()))
        return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                               nl->SymbolAtom(RouteLocation::BasicType()));

    if (param.first().isEqual(JListRInt::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(JRouteInterval::BasicType()));

    if (param.first().isEqual(JListNDG::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(NetDistanceGroup::BasicType()));
  }
  return listutils::typeError("Expected " + JListInt::BasicType() + ", " +
    JListRLoc::BasicType() + ", " + JListRInt::BasicType() +", or " +
    JListNDG::BasicType() + ".");
}

template<class CListElem>
struct locInfoCreateStream {
  locInfoCreateStream() : list(0)
  {
    it = 0;
  }

  DbArray<CListElem> list;
  int it;
};

template <class CListElem, class CList>
int createstreamVM ( Word* args, Word& result, int message, Word& local,
                   Supplier s )
{
  locInfoCreateStream<CListElem>* li = 0;
  switch(message)
  {
    case OPEN:
    {
      li = new locInfoCreateStream<CListElem>();
      CList* t = (CList*) args[0].addr;
      if (t != 0 && t->IsDefined())
        li->list = t->GetList();
      li->it = 0;
      local.addr = li;
      return 0;
      break;
    }

    case REQUEST:
    {
      result = qp->ResultStorage(s);
      if (local.addr == 0) return CANCEL;
      li = (locInfoCreateStream<CListElem>*) local.addr;
      if (0 <= li->it && li->it < li->list.Size())
      {
        CListElem elem;
        li->list.Get(li->it,elem);
        li->it = li->it + 1;
        result = SetWord(new CListElem(elem));
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
      break;
    }

    case CLOSE:
    {
      if (local.addr)
      {
        li = (locInfoCreateStream<CListElem>*) local.addr;
        delete li;
      }
      li = 0;
      local.addr = 0;
      return 0;
      break;
    }

    default:
    {
      return CANCEL; // Should never been reached
      break;
    }
  }
}

ValueMapping createstreamMap [] =
{
  createstreamVM<CcInt, JListInt>,
  createstreamVM<RouteLocation, JListRLoc>,
  createstreamVM<JRouteInterval, JListRInt>,
  createstreamVM<NetDistanceGroup, JListNDG>
};

int createstreamSelect ( ListExpr args )
{
  NList param(args);
  if (param.first() == JListInt::BasicType()) return 0;
  if (param.first() == JListRLoc::BasicType()) return 1;
  if (param.first() == JListRInt::BasicType()) return 2;
  if (param.first() == JListNDG::BasicType()) return 3;
  return -1; // this point should never been reached.
}

const string createstreamSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  JListInt::BasicType() + " -> " + Symbol::STREAM() + "(" +
  CcInt::BasicType() + "), " +
  JListRLoc::BasicType() + " -> " + Symbol::STREAM() + "(" +
  RouteLocation::BasicType() + "), " +
  JListRInt::BasicType() + " -> " + Symbol::STREAM() + "(" +
  JRouteInterval::BasicType() + "), " +
  JListNDG::BasicType() + " -> " + Symbol::STREAM() + "(" +
  NetDistanceGroup::BasicType() + ")</text--->"
  "<text>createstream (<list>) </text--->"
  "<text>The operator gets a list of type T and returns an " +
  Symbol::STREAM() + " of the corresponding data type T with the values " +
  "from the list.</text--->"
  "<text>query createstream(testlistint) createlist</text--->))";

Operator createstreamOp("createstream", createstreamSpec, 4, createstreamMap,
                         createstreamSelect, createstreamTM);

/*
1.1.1 ~createrloc~

Creates an ~rloc~ from an ~int~ (route identifier), an ~real~ (distance
from start of route), and an ~jdirection~ (side of route) value.

*/

const string maps_createrloc[1][4] =
{
  {CcInt::BasicType(), CcReal::BasicType(), Direction::BasicType(),
   RouteLocation::BasicType()}
};

ListExpr createrlocTM (ListExpr args)
{
  return SimpleMaps<1,4>(maps_createrloc, args);
}

int createrlocSelect(ListExpr args)
{
  return SimpleSelect<1,4>(maps_createrloc, args);
}

int createrlocVM( Word* args, Word& result, int message, Word& local,
                  Supplier s)
{
  result = qp->ResultStorage(s);
  RouteLocation* res = static_cast<RouteLocation*> (result.addr);

  CcInt* tint = (CcInt*) args[0].addr;
  CcReal* treal = (CcReal*) args[1].addr;
  Direction* tdir = (Direction*) args[2].addr;

  if (tint != 0 && tint->IsDefined() &&
      treal != 0 && treal->IsDefined() &&
      tdir != 0 && tdir->IsDefined())
  {
    RouteLocation* t = new RouteLocation(tint->GetIntval(),
                                         treal->GetRealval(),
                                         (Direction)tdir->GetDirection());
    *res = *t;
    t->DeleteIfAllowed();
  }
  else
    res->SetDefined(false);

  return 0;
}

ValueMapping createrlocMap[] =
{
  createrlocVM
};

const string createrlocSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + CcInt::BasicType() + " x " + CcReal::BasicType() + " x " +
  Direction::BasicType() + " -> " + RouteLocation::BasicType() + "</text--->"
  "<text>createrloc( <rid> , <pos> , <side>) </text--->"
  "<text>Creates an " + RouteLocation::BasicType() +" from the route id, " +
  "the position on the route and the side value.</text--->"
  "<text>query createrloc(rid, pos, side)</text--->))";

Operator createrlocOp(
  "createrloc",
  createrlocSpec,
  1,
  createrlocMap,
  createrlocSelect,
  createrlocTM
);

/*
1.1.1 ~createrint~

Creates an ~jrint~ from an ~int~ (route identifier), two ~real~ (distance
of start and end from start of route), and an ~jdirection~ (side of route)
value.

*/

const string maps_createrint[1][5] =
{
  {CcInt::BasicType(), CcReal::BasicType(), CcReal::BasicType(),
    Direction::BasicType(), JRouteInterval::BasicType()}
};

ListExpr createrintTM (ListExpr args)
{
  return SimpleMaps<1,5>(maps_createrint, args);
}

int createrintSelect(ListExpr args)
{
  return SimpleSelect<1,5>(maps_createrint, args);
}

int createrintVM( Word* args, Word& result, int message, Word& local,
                  Supplier s)
{
  result = qp->ResultStorage(s);
  JRouteInterval* res = static_cast<JRouteInterval*> (result.addr);

  CcInt* tint = (CcInt*) args[0].addr;
  CcReal* tspos = (CcReal*) args[1].addr;
  CcReal* tepos = (CcReal*) args[2].addr;
  Direction* tdir = (Direction*) args[3].addr;

  if (tint != 0 && tint->IsDefined() &&
      tspos != 0 && tspos->IsDefined() &&
      tepos != 0 && tepos->IsDefined() &&
      tdir != 0 && tdir->IsDefined())
  {
    JRouteInterval* t = new JRouteInterval(tint->GetIntval(),
                                           tspos->GetRealval(),
                                           tepos->GetRealval(),
                                           (Direction)tdir->GetDirection());
    *res = *t;
    t->DeleteIfAllowed();
  }
  else
    res->SetDefined(false);

  return 0;
}

ValueMapping createrintMap[] =
{
  createrintVM
};

const string createrintSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + CcInt::BasicType() + " x " + CcReal::BasicType() + " x " +
  CcReal::BasicType() + " x " +  Direction::BasicType() + " -> " +
  JRouteInterval::BasicType() + "</text--->"
  "<text>createrint( <rid> , <startpos> , <endpos> , <side>) </text--->"
  "<text>Creates a " + JRouteInterval::BasicType() + " from the route id, " +
   "the distances of the start and the end point from the start of the route "+
   "and the side value.</text--->"
  "<text>query createrint(rid, spos, epos, side)</text--->))";

Operator createrintOp(
  "createrint",
  createrintSpec,
  1,
  createrintMap,
  createrintSelect,
  createrintTM
);

/*
1.1.1 ~createndg~

Creates an ~ndg~ from an four ~int~ (identifier of source junction,
identifier of target junction, identifier of next junction and next section
on the path) values and one ~real~ (network distance) value.

*/

const string maps_createndg[1][6] =
{
  {CcInt::BasicType(), CcInt::BasicType(), CcInt::BasicType(),
   CcInt::BasicType(), CcReal::BasicType(), NetDistanceGroup::BasicType()}
};

ListExpr createndgTM (ListExpr args)
{
  return SimpleMaps<1,6>(maps_createndg, args);
}

int createndgSelect(ListExpr args)
{
  return SimpleSelect<1,6>(maps_createndg, args);
}

int createndgVM( Word* args, Word& result, int message, Word& local,
                  Supplier s)
{
  result = qp->ResultStorage(s);
  NetDistanceGroup* res = static_cast<NetDistanceGroup*> (result.addr);

  CcInt* tsource = (CcInt*) args[0].addr;
  CcInt* ttarget = (CcInt*) args[1].addr;
  CcInt* tnextjunc = (CcInt*) args[2].addr;
  CcInt* tnextsect = (CcInt*) args[3].addr;
  CcReal* tnetdist = (CcReal*) args[4].addr;

  if (tsource != 0 && tsource->IsDefined() &&
      ttarget != 0 && ttarget->IsDefined() &&
      tnextjunc != 0 && tnextjunc->IsDefined() &&
      tnextsect != 0 && tnextsect->IsDefined() &&
      tnetdist != 0 && tnetdist->IsDefined())
  {
    NetDistanceGroup* t = new NetDistanceGroup(tsource->GetIntval(),
                                               ttarget->GetIntval(),
                                               tnextjunc->GetIntval(),
                                               tnextsect->GetIntval(),
                                               tnetdist->GetRealval());
    *res = *t;
    t->DeleteIfAllowed();
  }
  else
    res->SetDefined(false);

  return 0;
}

ValueMapping createndgMap[] =
{
  createndgVM
};

const string createndgSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + CcInt::BasicType() + " x " + CcInt::BasicType() + " x " +
  CcInt::BasicType() + " x " + CcInt::BasicType() + " x " +
  CcReal::BasicType() + " -> " + NetDistanceGroup::BasicType() + "</text--->"
  "<text>createndg( <jid> , <jid> , <jid> , <sectid>, <netdist>) </text--->"
  "<text>Creates a " + NetDistanceGroup::BasicType() + " from the " +
  "identifiers of the source, the target, the next junction, the identifier "
   "of the next section and the network distance from source to target"+
   "junction.</text--->"
  "<text>query createndg(sourcejid, targetjid, nextjid, nextsectid, netdist)"+
  "</text--->))";

Operator createndgOp(
  "createndg",
  createndgSpec,
  1,
  createndgMap,
  createndgSelect,
  createndgTM
);

/*
1.1 Comparision of Data Types

The mapping, TypeMap- and Select-Function are equal for alle comparision
operators

*/

const string maps_compare[10][3] =
{
  {Direction::BasicType(), Direction::BasicType(), CcBool::BasicType()},
  {RouteLocation::BasicType(), RouteLocation::BasicType(),CcBool::BasicType()},
  {JRouteInterval::BasicType(), JRouteInterval::BasicType(),
    CcBool::BasicType()},
  {NetDistanceGroup::BasicType(), NetDistanceGroup::BasicType(),
    CcBool::BasicType()},
  {JPoint::BasicType(), JPoint::BasicType(), CcBool::BasicType()},
  {JLine::BasicType(), JLine::BasicType(), CcBool::BasicType()},
  {JListInt::BasicType(), JListInt::BasicType(), CcBool::BasicType()},
  {JListRLoc::BasicType(), JListRLoc::BasicType(), CcBool::BasicType()},
  {JListRInt::BasicType(), JListRInt::BasicType(), CcBool::BasicType()},
  {JListNDG::BasicType(), JListNDG::BasicType(), CcBool::BasicType()}
};

ListExpr compareTM (ListExpr args)
{
  return SimpleMaps<10,3>(maps_compare, args);
}

int compareSelect(ListExpr args)
{
  return SimpleSelect<10,3>(maps_compare, args);
}

/*
1.1.1 ~eq~

Returns true if the both values are the equal, false otherwise.

*/

template<class Elem>
int eqVM( Word* args, Word& result, int message, Word& local,
             Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*> (result.addr);

  Elem* arg1 = (Elem*) args[0].addr;
  Elem* arg2 = (Elem*) args[1].addr;

  if (arg1 != 0 && arg1->IsDefined() &&
      arg2 != 0 && arg2->IsDefined())
    res->Set(true, *arg1 == *arg2);
  else
    res->Set(false, false);
  return 0;
}

ValueMapping eqMap[] =
{
  eqVM<Direction>,
  eqVM<RouteLocation>,
  eqVM<JRouteInterval>,
  eqVM<NetDistanceGroup>,
  eqVM<JPoint>,
  eqVM<JLine>,
  eqVM<JListInt>,
  eqVM<JListRLoc>,
  eqVM<JListRInt>,
  eqVM<JListNDG>
};

const string eqSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + Direction::BasicType() + " x " + Direction::BasicType() +
  " -> " + CcBool::BasicType() +", " +
  RouteLocation::BasicType() + " x " + RouteLocation::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JRouteInterval::BasicType() + " x " + JRouteInterval::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  NetDistanceGroup::BasicType() + " x " + NetDistanceGroup::BasicType() +" -> "+
  CcBool::BasicType() + ", " +
  JPoint::BasicType() +" x "+ JPoint::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JLine::BasicType() + " x " + JLine::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JListInt::BasicType() + "x " + JListInt::BasicType() + " -> " +
  CcBool::BasicType() +", " +
  JListRLoc::BasicType()+ " x " + JListRLoc::BasicType()+ " -> " +
  CcBool::BasicType() + ", " +
  JListRInt::BasicType() + " x " + JListRInt::BasicType() + " -> " +
  CcBool::BasicType() + ", "+
  JListNDG::BasicType() + " x " + JListNDG::BasicType() + " -> " +
  CcBool::BasicType() + "</text--->"
  "<text>x = y </text--->"
  "<text>Returns TRUE if x and y are equal, false otherwise.</text--->"
  "<text>query x = x</text--->))";

  Operator eqOp(
    "=",
    eqSpec,
    10,
    eqMap,
    compareSelect,
    compareTM
  );

/*
1.1.1 ~lt~

Returns true if the left hand value is lower than the right hand value, false
otherwise.

*/

template<class Elem>
int ltVM( Word* args, Word& result, int message, Word& local,
          Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*> (result.addr);

  Elem* arg1 = (Elem*) args[0].addr;
  Elem* arg2 = (Elem*) args[1].addr;

  if (arg1 != 0 && arg1->IsDefined() &&
      arg2 != 0 && arg2->IsDefined())
    res->Set(true, *arg1 < *arg2);
  else
    res->Set(false, false);
  return 0;
}

ValueMapping ltMap[] =
{
  ltVM<Direction>,
  ltVM<RouteLocation>,
  ltVM<JRouteInterval>,
  ltVM<NetDistanceGroup>,
  ltVM<JPoint>,
  ltVM<JLine>,
  ltVM<JListInt>,
  ltVM<JListRLoc>,
  ltVM<JListRInt>,
  ltVM<JListNDG>
};

const string ltSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + Direction::BasicType() + " x " + Direction::BasicType() +
  " -> " + CcBool::BasicType() +", " +
  RouteLocation::BasicType() + " x " + RouteLocation::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JRouteInterval::BasicType() + " x " + JRouteInterval::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  NetDistanceGroup::BasicType() + " x " + NetDistanceGroup::BasicType() +" -> "+
  CcBool::BasicType() + ", " +
  JPoint::BasicType() +" x "+ JPoint::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JLine::BasicType() + " x " + JLine::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JListInt::BasicType() + "x " + JListInt::BasicType() + " -> " +
  CcBool::BasicType() +", " +
  JListRLoc::BasicType()+ " x " + JListRLoc::BasicType()+ " -> " +
  CcBool::BasicType() + ", " +
  JListRInt::BasicType() + " x " + JListRInt::BasicType() + " -> " +
  CcBool::BasicType() + ", "+
  JListNDG::BasicType() + " x " + JListNDG::BasicType() + " -> " +
  CcBool::BasicType() + "</text--->"
  "<text>x < y </text--->"
  "<text>Returns TRUE if x is lower than y, false otherwise.</text--->"
  "<text>query x < y</text--->))";

Operator ltOp(
  "<",
  ltSpec,
  10,
  ltMap,
  compareSelect,
  compareTM
);

/*
1.1.1 ~gt~

Returns true if the left hand value is greater than the right hand value, false
otherwise.

*/

template<class Elem>
int gtVM( Word* args, Word& result, int message, Word& local,
          Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*> (result.addr);

  Elem* arg1 = (Elem*) args[0].addr;
  Elem* arg2 = (Elem*) args[1].addr;

  if (arg1 != 0 && arg1->IsDefined() &&
    arg2 != 0 && arg2->IsDefined())
    res->Set(true, *arg1 > *arg2);
  else
    res->Set(false, false);
  return 0;
}

ValueMapping gtMap[] =
{
  gtVM<Direction>,
  gtVM<RouteLocation>,
  gtVM<JRouteInterval>,
  gtVM<NetDistanceGroup>,
  gtVM<JPoint>,
  gtVM<JLine>,
  gtVM<JListInt>,
  gtVM<JListRLoc>,
  gtVM<JListRInt>,
  gtVM<JListNDG>
};

const string gtSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + Direction::BasicType() + " x " + Direction::BasicType() +
  " -> " + CcBool::BasicType() +", " +
  RouteLocation::BasicType() + " x " + RouteLocation::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JRouteInterval::BasicType() + " x " + JRouteInterval::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  NetDistanceGroup::BasicType() + " x " + NetDistanceGroup::BasicType() +" -> "+
  CcBool::BasicType() + ", " +
  JPoint::BasicType() +" x "+ JPoint::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JLine::BasicType() + " x " + JLine::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JListInt::BasicType() + "x " + JListInt::BasicType() + " -> " +
  CcBool::BasicType() +", " +
  JListRLoc::BasicType()+ " x " + JListRLoc::BasicType()+ " -> " +
  CcBool::BasicType() + ", " +
  JListRInt::BasicType() + " x " + JListRInt::BasicType() + " -> " +
  CcBool::BasicType() + ", "+
  JListNDG::BasicType() + " x " + JListNDG::BasicType() + " -> " +
  CcBool::BasicType() + "</text--->"
  "<text>x > y </text--->"
  "<text>Returns TRUE if x is greater than y, false otherwise.</text--->"
  "<text>query x > y</text--->))";

Operator gtOp(
  ">",
  gtSpec,
  10,
  gtMap,
  compareSelect,
  compareTM
);

/*
1.1.1 ~le~

Returns true if the left hand value is lower than or equal the right hand value,
false otherwise.

*/

template<class Elem>
int leVM( Word* args, Word& result, int message, Word& local,
          Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*> (result.addr);

  Elem* arg1 = (Elem*) args[0].addr;
  Elem* arg2 = (Elem*) args[1].addr;

  if (arg1 != 0 && arg1->IsDefined() &&
    arg2 != 0 && arg2->IsDefined())
    res->Set(true, *arg1 <= *arg2);
  else
    res->Set(false, false);
  return 0;
}

ValueMapping leMap[] =
{
  leVM<Direction>,
  leVM<RouteLocation>,
  leVM<JRouteInterval>,
  leVM<NetDistanceGroup>,
  leVM<JPoint>,
  leVM<JLine>,
  leVM<JListInt>,
  leVM<JListRLoc>,
  leVM<JListRInt>,
  leVM<JListNDG>
};

const string leSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + Direction::BasicType() + " x " + Direction::BasicType() +
  " -> " + CcBool::BasicType() +", " +
  RouteLocation::BasicType() + " x " + RouteLocation::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JRouteInterval::BasicType() + " x " + JRouteInterval::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  NetDistanceGroup::BasicType() + " x " + NetDistanceGroup::BasicType() +" -> "+
  CcBool::BasicType() + ", " +
  JPoint::BasicType() +" x "+ JPoint::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JLine::BasicType() + " x " + JLine::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JListInt::BasicType() + "x " + JListInt::BasicType() + " -> " +
  CcBool::BasicType() +", " +
  JListRLoc::BasicType()+ " x " + JListRLoc::BasicType()+ " -> " +
  CcBool::BasicType() + ", " +
  JListRInt::BasicType() + " x " + JListRInt::BasicType() + " -> " +
  CcBool::BasicType() + ", "+
  JListNDG::BasicType() + " x " + JListNDG::BasicType() + " -> " +
  CcBool::BasicType() + "</text--->"
  "<text>x < y </text--->"
  "<text>Returns TRUE if x is lower than or equal y, false otherwise."+
  "</text--->"
  "<text>query x <= y</text--->))";

Operator leOp(
  "<=",
  leSpec,
  10,
  leMap,
  compareSelect,
  compareTM
);

/*
1.1.1 ~ge~

Returns true if the left hand value is greater than or equal the right hand
value, false otherwise.

*/

template<class Elem>
int geVM( Word* args, Word& result, int message, Word& local,
          Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*> (result.addr);

  Elem* arg1 = (Elem*) args[0].addr;
  Elem* arg2 = (Elem*) args[1].addr;

  if (arg1 != 0 && arg1->IsDefined() &&
    arg2 != 0 && arg2->IsDefined())
    res->Set(true, *arg1 >= *arg2);
  else
    res->Set(false, false);
  return 0;
}

ValueMapping geMap[] =
{
  geVM<Direction>,
  geVM<RouteLocation>,
  geVM<JRouteInterval>,
  geVM<NetDistanceGroup>,
  geVM<JPoint>,
  geVM<JLine>,
  geVM<JListInt>,
  geVM<JListRLoc>,
  geVM<JListRInt>,
  geVM<JListNDG>
};

const string geSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + Direction::BasicType() + " x " + Direction::BasicType() +
  " -> " + CcBool::BasicType() +", " +
  RouteLocation::BasicType() + " x " + RouteLocation::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JRouteInterval::BasicType() + " x " + JRouteInterval::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  NetDistanceGroup::BasicType() + " x " + NetDistanceGroup::BasicType() +" -> "+
  CcBool::BasicType() + ", " +
  JPoint::BasicType() +" x "+ JPoint::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JLine::BasicType() + " x " + JLine::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JListInt::BasicType() + "x " + JListInt::BasicType() + " -> " +
  CcBool::BasicType() +", " +
  JListRLoc::BasicType()+ " x " + JListRLoc::BasicType()+ " -> " +
  CcBool::BasicType() + ", " +
  JListRInt::BasicType() + " x " + JListRInt::BasicType() + " -> " +
  CcBool::BasicType() + ", "+
  JListNDG::BasicType() + " x " + JListNDG::BasicType() + " -> " +
  CcBool::BasicType() + "</text--->"
  "<text>x >= y </text--->"
  "<text>Returns TRUE if x is greater than or equal y, false otherwise."+
  "</text--->"
  "<text>query x >= y</text--->))";

  Operator geOp(
    ">=",
    geSpec,
    10,
    geMap,
    compareSelect,
    compareTM
  );

/*
1.1.1 ~ne~

Returns true if the left hand value is not equal the right hand value, false
otherwise.

*/

template<class Elem>
int neVM( Word* args, Word& result, int message, Word& local,
          Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*> (result.addr);

  Elem* arg1 = (Elem*) args[0].addr;
  Elem* arg2 = (Elem*) args[1].addr;

  if (arg1 != 0 && arg1->IsDefined() &&
    arg2 != 0 && arg2->IsDefined())
    res->Set(true, *arg1 != *arg2);
  else
    res->Set(false, false);
  return 0;
}

  ValueMapping neMap[] =
  {
    neVM<Direction>,
    neVM<RouteLocation>,
    neVM<JRouteInterval>,
    neVM<NetDistanceGroup>,
    neVM<JPoint>,
    neVM<JLine>,
    neVM<JListInt>,
    neVM<JListRLoc>,
    neVM<JListRInt>,
    neVM<JListNDG>
  };

const string neSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + Direction::BasicType() + " x " + Direction::BasicType() +
  " -> " + CcBool::BasicType() +", " +
  RouteLocation::BasicType() + " x " + RouteLocation::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JRouteInterval::BasicType() + " x " + JRouteInterval::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  NetDistanceGroup::BasicType() + " x " + NetDistanceGroup::BasicType() +" -> "+
  CcBool::BasicType() + ", " +
  JPoint::BasicType() +" x "+ JPoint::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JLine::BasicType() + " x " + JLine::BasicType() + " -> " +
  CcBool::BasicType() + ", " +
  JListInt::BasicType() + "x " + JListInt::BasicType() + " -> " +
  CcBool::BasicType() +", " +
  JListRLoc::BasicType()+ " x " + JListRLoc::BasicType()+ " -> " +
  CcBool::BasicType() + ", " +
  JListRInt::BasicType() + " x " + JListRInt::BasicType() + " -> " +
  CcBool::BasicType() + ", "+
  JListNDG::BasicType() + " x " + JListNDG::BasicType() + " -> " +
  CcBool::BasicType() + "</text--->"
  "<text>x # y </text--->"
  "<text>Returns TRUE if x not equals y, false otherwise.</text--->"
  "<text>query x # y</text--->))";

Operator neOp(
  "#",
  neSpec,
  10,
  neMap,
  compareSelect,
  compareTM
);

/*
1 Implementation of ~class JNetAlgebra~

1.1 Constructor

*/


JNetAlgebra::JNetAlgebra():Algebra()
{

/*
1.1 Integration of Data Types by Type Constructors

1.1.1  Basic Data Types Used By Network Data Types

1.1.1.1 Simple Data Types

*/

  AddTypeConstructor(&jdirectionTC);
  jdirectionTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&jrlocTC);
  jrlocTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&jrintTC);
  jrintTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&jndgTC);
  jndgTC.AssociateKind(Kind::DATA());

/*
1.1.1.1 List Data Types

*/

  AddTypeConstructor(&jlistintTC);
  jlistintTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&jlistrlocTC);
  jlistrlocTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&jlistjrintTC);
  jlistjrintTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&jlistndgTC);
  jlistndgTC.AssociateKind(Kind::DATA());

/*
1.1.1 Network Data Types

*/

  AddTypeConstructor (&jnetworkTC);
  jnetworkTC.AssociateKind(Kind::JNETWORK());

  AddTypeConstructor (&jpointTC);
  jpointTC.AssociateKind(Kind::DATA());

  AddTypeConstructor (&jlineTC);
  jlineTC.AssociateKind(Kind::DATA());


/*
1.1 Integration of Operators

1.1.1 Creation

1.1.1.1 Network Construction

*/
  AddOperator(&createjnetOp);

/*
1.1.1.1 Simple Datatypes

*/

  AddOperator(&createrlocOp);
  AddOperator(&createrintOp);
  AddOperator(&createndgOp);

/*
1.1.1.1 Lists and streams of Data Types

*/

  AddOperator(&createlistOp);
  AddOperator(&createstreamOp);

/*
1.1.1 Comparision of Data types

*/

  AddOperator(&eqOp);
  AddOperator(&ltOp);
  AddOperator(&gtOp);
  AddOperator(&leOp);
  AddOperator(&geOp);
  AddOperator(&neOp);
}

/*
1.1 Deconstructor

*/

 JNetAlgebra::~JNetAlgebra()
 {}


/*
1 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra* InitializeJNetAlgebra ( NestedList* nlRef,
                                 QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return ( new JNetAlgebra() );
}