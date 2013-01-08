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

//[TOC] [\tableofcontents]

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
#include "JUnit.h"
#include "JList.h"
#include "JNetwork.h"
#include "JPoint.h"
#include "JPoints.h"
#include "JLine.h"
#include "IJPoint.h"
#include "UJPoint.h"
#include "MJPoint.h"
#include "RelationAlgebra.h"
#include "JList.h"

using namespace std;
using namespace mappings;
using namespace jnetwork;

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
1.1 ~junit~

Pair of an time interval and ~jrint~ describing a single route position within
the given time interval.

*/

TypeConstructor junitTC(
  JUnit::BasicType(),
  JUnit::Property,
  JUnit::Out, JUnit::In,
  0, 0,
  JUnit::Create, JUnit::Delete,
  OpenAttribute<JUnit>,
  SaveAttribute<JUnit>,
  JUnit::Close, JUnit::Clone,
  JUnit::Cast,
  JUnit::SizeOf,
  JUnit::KindCheck);


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
1.1 ~jpoints~

Describes a set of positions in the network. Consists of an ~string~ as network
identifier and an set of ~rloc~s describing the network positions.

*/

TypeConstructor jpointsTC(
  JPoints::BasicType(),
  JPoints::Property,
  JPoints::Out, JPoints::In,
  0, 0,
  JPoints::Create, JPoints::Delete,
  OpenAttribute<JPoints>,
  SaveAttribute<JPoints>,
  JPoints::Close, JPoints::Clone,
  JPoints::Cast,
  JPoints::SizeOf,
  JPoints::KindCheck);


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
1.1 ~ijpoint~

Describes the position of an ~mjpoint~ in an ~jnet~ at the time ~instant~.
Consist of an time ~instant~ and an ~jpoint~ value.

*/

TypeConstructor ijpointTC(
  IJPoint::BasicType(),
  IJPoint::Property,
  IJPoint::Out, IJPoint::In,
  0, 0,
  IJPoint::Create, IJPoint::Delete,
  OpenAttribute<IJPoint>,
  SaveAttribute<IJPoint>,
  IJPoint::Close, IJPoint::Clone,
  IJPoint::Cast,
  IJPoint::SizeOf,
  IJPoint::KindCheck);

/*
1.1 ~ujpoint~

Describes the positions of an ~mjpoint~ in an ~jnet~ within the time interval.
Consist of an network identifier, an time interval and an ~jrint~ value.

*/

TypeConstructor ujpointTC(
  UJPoint::BasicType(),
  UJPoint::Property,
  UJPoint::Out, UJPoint::In,
  0, 0,
  UJPoint::Create, UJPoint::Delete,
  OpenAttribute<UJPoint>,
  SaveAttribute<UJPoint>,
  UJPoint::Close, UJPoint::Clone,
  UJPoint::Cast,
  UJPoint::SizeOf,
  UJPoint::KindCheck);

/*
1.1 ~mjpoint~

Describes the positions of an ~mjpoint~ in an ~jnet~.
Consist of an network identifier, and an set of ~ujpoint~.

*/

TypeConstructor mjpointTC(
  MJPoint::BasicType(),
  MJPoint::Property,
  MJPoint::Out, MJPoint::In,
  0, 0,
  MJPoint::Create, MJPoint::Delete,
  OpenAttribute<MJPoint>,
  SaveAttribute<MJPoint>,
  MJPoint::Close, MJPoint::Clone,
  MJPoint::Cast,
  MJPoint::SizeOf,
  MJPoint::KindCheck);


/*
1 Secondo Operators

1.1 Creation of Data Types

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

Operator createrlocJNet(
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

Operator createrintJNet( "createrint", createrintSpec, 1, createrintMap,
                         createrintSelect, createrintTM);

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

Operator createndgJNet("createndg", createndgSpec, 1, createndgMap,
                       createndgSelect, createndgTM);

/*
1.1.1 ~creatjnet~

The operator ~createjnet~ creates an single network object from the given
ressources. It expects five arguments:
- an string object with the object name for the new jnetwork
- an double value with the tolerance value for map matching for this network
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
  if (!nl->HasLength(args,5))
    return listutils::typeError("Five arguments expected.");

  ListExpr idList = nl->First(args);
  if (!listutils::isSymbol(idList, CcString::BasicType()))
    return listutils::typeError("First argument should be " +
                                CcString::BasicType());

  ListExpr tolList = nl->Second(args);
  if (!listutils::isSymbol(tolList, CcReal::BasicType()))
    return listutils::typeError("Second argument should be " +
                                 CcReal::BasicType());

  ListExpr juncList = nl->Third(args);
  if (!IsRelDescription(juncList))
    return listutils::typeError("Third argument must be an relation");

  ListExpr xType;
  nl->ReadFromString ( JNetwork::GetJunctionsRelationType(), xType );
  if (!CompareSchemas ( juncList, xType ))
    return (nl->SymbolAtom("First relation (junctions) has wrong schema." ));

  ListExpr sectList = nl->Fourth(args);
  if (!IsRelDescription(sectList))
    return listutils::typeError("Fourth argument must be an relation.");

  nl->ReadFromString ( JNetwork::GetSectionsRelationType(), xType );
  if (!CompareSchemas ( sectList, xType ))
    return (nl->SymbolAtom("Second relation (sections) has wrong schema."));

  ListExpr routesList = nl->Fifth(args);
  if (!IsRelDescription(routesList))
    return listutils::typeError("Fifth argument must be an relation.");

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
    double tolerance = ((CcReal*) args[1].addr)->GetRealval();
    Relation* juncRel = (Relation*) args[2].addr;
    Relation* sectRel = (Relation*) args[3].addr;
    Relation* routesRel = (Relation*) args[4].addr;

    if (juncRel != 0 && sectRel != 0 && routesRel != 0)
    {
      JNetwork* resNet = new JNetwork(netid, tolerance, juncRel, sectRel,
                                      routesRel);
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
  "(<text>" + CcString::BasicType() + " X " + CcReal::BasicType() + " X " +
  JNetwork::GetJunctionsRelationType() + "X " +
  JNetwork::GetSectionsRelationType() + " X " +
  JNetwork::GetRoutesRelationType() + " -> " +
  CcBool::BasicType() + "</text--->"
  "<text>createjnet( <id> , <tolerance> , <junctions relation> ," +
  "<sections relation> , <routes relation> ) </text--->"
  "<text>If the id is a possible object name in the database the operation"
  "creates the " + JNetwork::BasicType() + " with given data and object name "+
  "id and returns true, false otherwise. The " + CcReal::BasicType() +
  "is the tolerance value factor for map matching. It depends on the " +
  "spatial values of the network which factor is senseful.</text--->"
  "<text>query createjnet(testnet, 0.01, juncrel, sectrel, " +
  "routerel)</text--->))";

Operator createjnetJNet("createjnet", createjnetSpec, createjnetVM,
                      Operator::SimpleSelect, createjnetTM);

/*
1.1.1 ~createjpoint~

Creates an ~jpoint~ from an existing ~jnet~ and an ~rloc~ value.
The ~rloc~ must exist in the ~jnet~ otherwise the created ~jpoint~ is undefined.

*/

const string maps_createjpoint[1][3] =
{
  {JNetwork::BasicType(), RouteLocation::BasicType(), JPoint::BasicType()}
};

ListExpr createjpointTM (ListExpr args)
{
  return SimpleMaps<1,3>(maps_createjpoint, args);
}

int createjpointSelect(ListExpr args)
{
  return SimpleSelect<1,3>(maps_createjpoint, args);
}

int createjpointVM( Word* args, Word& result, int message, Word& local,
                    Supplier s)
{
  result = qp->ResultStorage(s);
  JPoint* res = static_cast<JPoint*> (result.addr);

  JNetwork* jnet = (JNetwork*) args[0].addr;
  RouteLocation* rloc = (RouteLocation*) args[1].addr;

  if (jnet != 0 && jnet->IsDefined() &&
      rloc != 0 && rloc->IsDefined())
  {
    res->SetDefined(true);
    res->SetNetId(*jnet->GetId());
    res->SetPosition(*rloc,true,jnet);
  }
  else
    res->SetDefined(false);

  return 0;
}

ValueMapping createjpointMap[] =
{
  createjpointVM
};

const string createjpointSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + JNetwork::BasicType() + " x " + RouteLocation::BasicType() +
  " -> " + JPoint::BasicType() + "</text--->"
  "<text>createjpoint( <jnet>, <rloc>) </text--->"
  "<text>Creates an " + JPoint::BasicType() + " at " +
  RouteLocation::BasicType() + " in the " + JNetwork::BasicType() + "if the" +
  " position exists in the given network, otherwise the result is " +
  " undefined.</text--->"
  "<text>query createjpoint(testjnet, createrloc(1, 0.0, [const jdirection " +
  " value(\"Both\")]) </text--->))";

Operator createjpointJNet("createjpoint", createjpointSpec, 1, createjpointMap,
                          createjpointSelect, createjpointTM);

/*
1.1.1 ~createjline~

Creates an ~jline~ from an existing ~jnet~ and an ~listjrint~ value.
The ~jrint~ in the list of jrint must all exist in the ~jnet~ otherwise the
created ~jline~ will be undefined.

*/

const string maps_createjline[1][3] =
{
  {JNetwork::BasicType(), JListRInt::BasicType(), JLine::BasicType()}
};

ListExpr createjlineTM (ListExpr args)
{
  return SimpleMaps<1,3>(maps_createjline, args);
}

int createjlineSelect(ListExpr args)
{
  return SimpleSelect<1,3>(maps_createjline, args);
}

int createjlineVM( Word* args, Word& result, int message, Word& local,
                   Supplier s)
{
  result = qp->ResultStorage(s);
  JLine* res = static_cast<JLine*> (result.addr);

  JNetwork* jnet = (JNetwork*) args[0].addr;
  JListRInt* lrint = (JListRInt*) args[1].addr;

  if (jnet != 0 && jnet->IsDefined() &&
    lrint != 0 && lrint->IsDefined())
  {
    res->Clear();
    res->SetNetworkId(*jnet->GetId());
    res->SetRouteIntervals(lrint->GetList(), true, false, jnet);
  }
  else
    res->SetDefined(false);

  return 0;
}

ValueMapping createjlineMap[] =
{
  createjlineVM
};

const string createjlineSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + JNetwork::BasicType() + " x " + JListRInt::BasicType() +
  " -> " + JLine::BasicType() + "</text--->"
  "<text>createjline( <jnet>, <listjrint>) </text--->"
  "<text>Creates an " + JLine::BasicType() + " covering the parts of the " +
  JNetwork::BasicType() + " described by " + JListRInt::BasicType() + "if" +
  "the route intervals exist in the jnet. Otherwise the result is undefined." +
  "</text--->"
  "<text>query createjline(testjnet, testlistjrint) </text--->))";

Operator createjlineJNet("createjline", createjlineSpec, 1, createjlineMap,
                         createjlineSelect, createjlineTM);

/*
1.1.1 ~createjpoints~

Creates an ~jpoints~ object from an ~jnet~ and an ~listjrloc~ value.
The ~rloc~ in the list of rloc must all exist in the ~jnet~ otherwise the
created ~jpoints~ will be undefined.

*/

const string maps_createjpoints[1][3] =
{
  {JNetwork::BasicType(), JListRLoc::BasicType(), JPoints::BasicType()}
};

ListExpr createjpointsTM (ListExpr args)
{
  return SimpleMaps<1,3>(maps_createjpoints, args);
}

int createjpointsSelect(ListExpr args)
{
  return SimpleSelect<1,3>(maps_createjpoints, args);
}

int createjpointsVM( Word* args, Word& result, int message, Word& local,
                     Supplier s)
{
  result = qp->ResultStorage(s);
  JPoints* res = static_cast<JPoints*> (result.addr);

  JNetwork* jnet = (JNetwork*) args[0].addr;
  JListRLoc* lrloc = (JListRLoc*) args[1].addr;

  if (jnet != 0 && jnet->IsDefined() &&
      lrloc != 0 && lrloc->IsDefined())
  {
    res->SetNetworkId(*jnet->GetId());
    res->SetRouteLocations(lrloc->GetList(), true, false, jnet);
  }
  else
    res->SetDefined(false);

  return 0;
}

ValueMapping createjpointsMap[] =
{
  createjpointsVM
};

const string createjpointsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + JNetwork::BasicType() + " x " + JListRLoc::BasicType() +
  " -> " + JPoints::BasicType() + "</text--->"
  "<text>createjpoints( <jnet>, <listrloc>) </text--->"
  "<text>Creates an " + JPoints::BasicType() + " containing a set of " +
  JNetwork::BasicType() + " positions described by " + JListRLoc::BasicType() +
  "if the route locations exist in the jnet. Otherwise the result is " +
  "undefined." +"</text--->"
  "<text>query createjpoints(testjnet, testlistrloc) </text--->))";

Operator createjpointsJNet("createjpoints", createjpointsSpec, 1,
                           createjpointsMap, createjpointsSelect,
                           createjpointsTM);

/*
1.1.1 ~createijpoint~

Creates an ~ijpoint~ from an existing ~jpoint~ and an ~instant~ value.

*/

const string maps_createijpoint[1][3] =
{
  {JPoint::BasicType(), Instant::BasicType(), IJPoint::BasicType()}
};

ListExpr createijpointTM (ListExpr args)
{
  return SimpleMaps<1,3>(maps_createijpoint, args);
}

int createijpointSelect(ListExpr args)
{
  return SimpleSelect<1,3>(maps_createijpoint, args);
}

int createijpointVM( Word* args, Word& result, int message, Word& local,
                     Supplier s)
{
  result = qp->ResultStorage(s);
  IJPoint* res = static_cast<IJPoint*> (result.addr);

  JPoint* jp = (JPoint*) args[0].addr;
  Instant* time = (Instant*) args[1].addr;

  if (jp != 0 && jp->IsDefined() &&
      time != 0 && time->IsDefined())
  {
    res->SetDefined(true);
    res->SetInstant(*time);
    res->SetPoint(*jp);
  }
  else
    res->SetDefined(false);
  return 0;
}

ValueMapping createijpointMap[] =
{
  createijpointVM
};

const string createijpointSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + JPoint::BasicType() + " x " + Instant::BasicType() +
  " -> " + IJPoint::BasicType() + "</text--->"
  "<text>createijpoint( <jpoint>, <instant>) </text--->"
  "<text>Creates an " + IJPoint::BasicType() + " fromt an " +
  JPoint::BasicType() + " and an  " + Instant::BasicType() + ".</text--->"
  "<text>query createijpoint(createjpoint(testjnet, createrloc(1, 0.0, " +
  "[const jdirection value(\"Both\")])), createinstant(0.5)) </text--->))";

Operator createijpointJNet("createijpoint", createijpointSpec, 1,
                           createijpointMap, createijpointSelect,
                           createijpointTM);

/*
1.1.1 ~createujpoint~

Creates an ~ujpoint~ from an ~jnet~, an ~jrint~, two ~instant~ and two
~bool~ values.

*/

const string maps_createujpoint[1][7] =
{
  {JNetwork::BasicType(), JRouteInterval::BasicType(), Instant::BasicType(),
   Instant::BasicType(), CcBool::BasicType(), CcBool::BasicType(),
   UJPoint::BasicType()}
};

ListExpr createujpointTM (ListExpr args)
{
  return SimpleMaps<1,7>(maps_createujpoint, args);
}

int createujpointSelect(ListExpr args)
{
  return SimpleSelect<1,7>(maps_createujpoint, args);
}

int createujpointVM( Word* args, Word& result, int message, Word& local,
                     Supplier s)
{
  result = qp->ResultStorage(s);
  UJPoint* res = static_cast<UJPoint*> (result.addr);

  JNetwork* jnet = (JNetwork*) args[0].addr;
  JRouteInterval* rint = (JRouteInterval*) args[1].addr;
  Instant* starttime = (Instant*) args[2].addr;
  Instant* endtime = (Instant*) args[3].addr;
  CcBool* lc = (CcBool*) args[4].addr;
  CcBool* rc = (CcBool*) args[5].addr;

  if (jnet != 0 && jnet->IsDefined() &&
      rint != 0 && rint->IsDefined() &&
      starttime != 0 && starttime->IsDefined() &&
      endtime != 0 && endtime->IsDefined() &&
      lc != 0 && lc->IsDefined() &&
      rc != 0 && rc->IsDefined())
  {
    res->SetDefined(true);
    res->SetNetworkId(*jnet->GetId());
    res->SetUnit(JUnit(Interval<Instant> (*starttime, *endtime,
                                          lc->GetBoolval(), rc->GetBoolval()),
                       *rint),
                 true, jnet);
  }
  else
    res->SetDefined(false);
  return 0;
}

ValueMapping createujpointMap[] =
{
  createujpointVM
};

const string createujpointSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + JNetwork::BasicType() + " x " + JRouteInterval::BasicType() +
  " x " + Instant::BasicType() + " x " + Instant::BasicType() + " x " +
  CcBool::BasicType() + " x " + CcBool::BasicType() +
  " -> " + UJPoint::BasicType() + "</text--->"
  "<text>createujpoint( <jnet>, <jrint>, <instant>, <instant>, <bool>, " +
  "<bool>) </text--->"
  "<text>Creates an " + UJPoint::BasicType() + " in the " +
  JNetwork::BasicType() + " which moves on the given " +
  JRouteInterval::BasicType() + " within the time interval given by the " +
  "two " + Instant::BasicType() + " and the two " + CcBool::BasicType() +
  " values.</text--->"
  "<text>query createujpoint(netname, createjrint(2, 0.0, 35.4, " +
  "[const jdirection value(Up)]), createinstant(0.5), createinstant(0.6)," +
  " TRUE, FALSE) </text--->))";

Operator createujpointJNet("createujpoint", createujpointSpec, 1,
                           createujpointMap, createujpointSelect,
                           createujpointTM);

/*

1.1.1 ~createmjpoint~

Creates an ~mjpoint~ from an ~ujpoint~.

*/

const string maps_createmjpoint[1][2] =
{
  {UJPoint::BasicType(), MJPoint::BasicType()}
};

ListExpr createmjpointTM (ListExpr args)
{
  return SimpleMaps<1,2>(maps_createmjpoint, args);
}

int createmjpointSelect(ListExpr args)
{
  return SimpleSelect<1,2>(maps_createmjpoint, args);
}

int createmjpointVM( Word* args, Word& result, int message, Word& local,
                   Supplier s)
{
  result = qp->ResultStorage(s);
  MJPoint* res = static_cast<MJPoint*> (result.addr);

  UJPoint* u = (UJPoint*) args[0].addr;
  if (u != 0 && u->IsDefined())
  {
    res->SetDefined(true);
    res->SetNetworkId(*u->GetNetworkId());
    res->StartBulkload();
    res->Add(u->GetUnit());
    res->EndBulkload(false,true);
  }
  else
    res->SetDefined(false);
  return 0;
}

ValueMapping createmjpointMap[] =
{
  createmjpointVM
};

const string createmjpointSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + UJPoint::BasicType() + " -> " + MJPoint::BasicType() +
  "</text--->"
  "<text>createmjpoint( <ujpoint>) </text--->"
  "<text>Creates an " + MJPoint::BasicType() + " consisting of an single " +
  UJPoint::BasicType() + " defined by the input value.</text--->"
  "<text>query createmjpoint(createujpoint(netname, createjrint(2, 0.0, 35.4," +
  " [const jdirection value(Up)]), createinstant(0.5), createinstant(0.6)," +
  " TRUE, FALSE)) </text--->))";

Operator createmjpointJNet("createmjpoint", createmjpointSpec, 1,
                           createmjpointMap, createmjpointSelect,
                           createmjpointTM);

/*
1.1.1 ~createlist~

Creates a list from type ~X~ from a stream of the corresponding datatype ~Y~

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

ListExpr createlistTM (ListExpr args)
{
  if(!nl->HasLength(args,1)){
    return listutils::typeError("stream(data) expected");
  }
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
  createlistVM<JListNDG, JListNDG>
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
  JListInt::BasicType() + ", \n" +
  Symbol::STREAM() + "("+ RouteLocation::BasicType() + ") -> " +
  JListRLoc::BasicType() + ", \n" +
  Symbol::STREAM() + "("+ JRouteInterval::BasicType() + ") -> " +
  JListRInt::BasicType() + ", \n" +
  Symbol::STREAM() + "("+ NetDistanceGroup::BasicType() + ") -> " +
  JListNDG::BasicType() +  ", \n" +
  Symbol::STREAM() + "("+ JListInt::BasicType() + ") -> " +
  JListInt::BasicType() +  ", \n" +
  Symbol::STREAM() + "("+ JListRLoc::BasicType() + ") -> " +
  JListRLoc::BasicType() +  ", \n" +
  Symbol::STREAM() + "("+ JListRInt::BasicType() + ") -> " +
  JListRInt::BasicType() +  ", \n" +
  Symbol::STREAM() + "("+ JListNDG::BasicType() + ") -> " +
  JListNDG::BasicType() + " </text--->"
  "<text>_ createlist </text--->"
  "<text>Collects the values of a stream of type T into an single list" +
  "of the corresponding list data type.</text--->"
  "<text>query createstream(testlistint) createlist</text--->))";

Operator createlistJNet("createlist", createlistSpec, 8, createlistMap,
                      createlistSelect, createlistTM);

/*
1.1 Return Data Types and Parts of Data Types as Stream

1.1.1 ~createstream~

Creates a stream of data type ~Y~ from a list from type ~X~

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
  CcInt::BasicType() + "), \n" +
  JListRLoc::BasicType() + " -> " + Symbol::STREAM() + "(" +
  RouteLocation::BasicType() + "), \n" +
  JListRInt::BasicType() + " -> " + Symbol::STREAM() + "(" +
  JRouteInterval::BasicType() + "), \n" +
  JListNDG::BasicType() + " -> " + Symbol::STREAM() + "(" +
  NetDistanceGroup::BasicType() + ")</text--->"
  "<text>createstream (<list>) </text--->"
  "<text>The operator gets a list of type T and returns an " +
  Symbol::STREAM() + " of the corresponding data type T with the values " +
  "from the list.</text--->"
  "<text>query createstream(testlistint) createlist</text--->))";

Operator createstreamJNet("createstream", createstreamSpec, 4, createstreamMap,
                         createstreamSelect, createstreamTM);

/*
1.1.1 ~units~

Returns a stream of ~junit~ for an given ~mjpoint~ and an stream of ~jrint~
for an given ~jline~

*/

ListExpr unitsTM (ListExpr args)
{
  if (!nl->HasLength(args,1))
    return listutils::typeError("One argument expected.");

  if (listutils::isSymbol(nl->First(args), MJPoint::BasicType()))
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           nl->SymbolAtom(UJPoint::BasicType()));

  if (listutils::isSymbol(nl->First(args), JLine::BasicType()))
    return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                           nl->SymbolAtom(JRouteInterval::BasicType()));

  return listutils::typeError("Argument should be " + MJPoint::BasicType() +
                              " or " + JLine::BasicType());
}

int unitsSelect(ListExpr args)
{
  if ( nl->SymbolValue ( nl->First ( args )) == MJPoint::BasicType())
    return 0;
  if ( nl->SymbolValue ( nl->First(args)) == JLine::BasicType())
    return 1;
  return -1; // This point should never be reached
};

template<class InClass>
struct locInfoUnits {
  locInfoUnits()
  {
    index = 0;
    in = 0;
  }

  InClass* in;
  int index;
};

int unitsMJPointVM ( Word* args, Word& result, int message, Word& local,
                     Supplier s )
{
  locInfoUnits<MJPoint>* li = 0;
  switch(message)
  {
    case OPEN:
    {
      li = new locInfoUnits<MJPoint>();
      MJPoint* in = (MJPoint*) args[0].addr;
      if (in != 0 && in->IsDefined())
        li->in = in;
      li->index = 0;
      local.addr = li;
      return 0;
      break;
    }

    case REQUEST:
    {
      result = qp->ResultStorage(s);
      if (local.addr == 0) return CANCEL;
      li = (locInfoUnits<MJPoint>*) local.addr;
      if (0 <= li->index && li->index < li->in->GetNoComponents())
      {
        JUnit elem(false);
        (li->in)->Get(li->index, elem);
        result = SetWord(new UJPoint(*li->in->GetNetworkId(), elem, false));
        li->index++;
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
        li = (locInfoUnits<MJPoint>*) local.addr;
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

int unitsJLineVM ( Word* args, Word& result, int message, Word& local,
              Supplier s )
{
  locInfoUnits<JLine>* li = 0;
  switch(message)
  {
    case OPEN:
    {
      li = new locInfoUnits<JLine>();
      JLine* in = (JLine*) args[0].addr;
      if (in != 0 && in->IsDefined())
        li->in = in;
      li->index = 0;
      local.addr = li;
      return 0;
      break;
    }

    case REQUEST:
    {
      result = qp->ResultStorage(s);
      if (local.addr == 0) return CANCEL;
      li = (locInfoUnits<JLine>*) local.addr;
      if (0 <= li->index && li->index < li->in->GetNoComponents())
      {
        JRouteInterval elem(false);
        (li->in)->Get(li->index, elem);
        result = SetWord(new JRouteInterval(elem));
        li->index++;
        while (!elem.IsDefined() &&
               li->index < li->in->GetNoComponents())
        {
          (li->in)->Get(li->index, elem);
          li->index++;
        }
        result = SetWord(elem.Clone());
        if (!elem.IsDefined())
          return CANCEL;
        else
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
        li = (locInfoUnits<JLine>*) local.addr;
        delete li;
      }
      li = 0;
      local.addr = 0;
      return 0;
      break;
    }

    default:
    {
      assert(false);
      return CANCEL; // Should never been reached
      break;
    }
  }
}

ValueMapping unitsMap [] =
{
  unitsMJPointVM,
  unitsJLineVM
};


const string unitsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + MJPoint::BasicType() + " -> " +  Symbol::STREAM() + "("+
  UJPoint::BasicType() + "), \n"+
  JLine::BasicType() +" -> "  +  Symbol::STREAM() + "("+
  JRouteInterval::BasicType() + ") </text--->"
  "<text>units(<mjpoint>) </text--->"
  "<text>Returns the components of the input value as stream.</text--->"
  "<text>query units(testmjp)</text--->))";

  Operator unitsJNet("units", unitsSpec, 2, unitsMap, unitsSelect, unitsTM);

/*
1.1.1 ~altrlocs~

Returns for an given jpoint a stream of jpoints with all possible network
descriptions of this jpoint. This is interesting because a junction belongs to
different routes and has therefore different jnet representations.

*/

ListExpr altrlocsTM (ListExpr args)
{
  NList param(args);
  if (param.length()==1)
  {
    if (param.first().isEqual(JPoint::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(JPoint::BasicType()));
  }
  return listutils::typeError("Expected " + JPoint::BasicType() + ".");
}

struct altrlocsLocInfo {
  altrlocsLocInfo() : list(0)
  {
    it = 0;
  }

  JListRLoc* list;
  STRING_T jnetId;
  int it;
};

int altrlocsVM ( Word* args, Word& result, int message, Word& local,
                 Supplier s )
{
  altrlocsLocInfo* li = 0;
  switch(message)
  {
    case OPEN:
    {
      li = new altrlocsLocInfo();
      JPoint* jp = (JPoint*) args[0].addr;
      if (jp != 0 && jp->IsDefined())
      {
        strcpy(li->jnetId, *jp->GetNetworkId());
        li->list = jp->OtherNetworkPositions();
        if (li->list != 0)
        {
          li->it = 0;
          local.addr = li;
          return 0;
        }
      }
      delete li;
      local.addr = 0;
      return 0;
      break;
    }

    case REQUEST:
    {
      if (local.addr == 0) return CANCEL;
      li = (altrlocsLocInfo*) local.addr;
      if (li->list->IsDefined() &&
          0 <= li->it && li->it < li->list->GetNoOfComponents())
      {
        RouteLocation rloc(false);
        li->list->Get(li->it, rloc);
        li->it = li->it + 1;
        result = SetWord ( new JPoint(li->jnetId, rloc, false));
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
        li = (altrlocsLocInfo*) local.addr;
        li->list->Destroy();
        li->list->DeleteIfAllowed();
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

ValueMapping altrlocsMap [] =
{
  altrlocsVM
};

int altrlocsSelect ( ListExpr args )
{
  NList param(args);
  if (param.first() == JPoint::BasicType()) return 0;
  return -1; // this point should never been reached.
}

const string altrlocsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  JPoint::BasicType() + " -> " + Symbol::STREAM() + "(" +
  JPoint::BasicType() + ")</text--->"
  "<text>altrlocs(<jpoint>) </text--->"
  "<text>The operator gets an jpoint and returns an " +
  Symbol::STREAM() + " of the corresponding " + JPoint::BasicType() +
  " representing the same spatial position in the network. This function is"+
  " needed because spatial positions of junctions have more than one network"+
  " representation.</text--->"
  "<text>query altrlocs(testjp) transformstream consume</text--->))";

Operator altrlocsJNet("altrlocs", altrlocsSpec, 1, altrlocsMap, altrlocsSelect,
                      altrlocsTM);


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

const string compareTypeCombinations =
  Direction::BasicType() + " x " + Direction::BasicType() +
  " -> " + CcBool::BasicType() +", \n" +
  RouteLocation::BasicType() + " x " + RouteLocation::BasicType() + " -> " +
  CcBool::BasicType() + ", \n" +
  JRouteInterval::BasicType() + " x " + JRouteInterval::BasicType() + " -> " +
  CcBool::BasicType() + ", \n" +
  NetDistanceGroup::BasicType() + " x " + NetDistanceGroup::BasicType() +" -> "+
  CcBool::BasicType() + ", \n" +
  JPoint::BasicType() +" x "+ JPoint::BasicType() + " -> " +
  CcBool::BasicType() + ", \n" +
  JLine::BasicType() + " x " + JLine::BasicType() + " -> " +
  CcBool::BasicType() + ", \n" +
  JListInt::BasicType() + "x " + JListInt::BasicType() + " -> " +
  CcBool::BasicType() +", \n" +
  JListRLoc::BasicType()+ " x " + JListRLoc::BasicType()+ " -> " +
  CcBool::BasicType() + ", \n" +
  JListRInt::BasicType() + " x " + JListRInt::BasicType() + " -> " +
  CcBool::BasicType() + ", \n"+
  JListNDG::BasicType() + " x " + JListNDG::BasicType() + " -> " +
  CcBool::BasicType();

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
  "(<text>" + compareTypeCombinations + "</text--->"
  "<text>x = y </text--->"
  "<text>Returns TRUE if x and y are equal, false otherwise.</text--->"
  "<text>query x = x</text--->))";

  Operator eqJNet("=", eqSpec, 10, eqMap, compareSelect, compareTM);

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
  "(<text>" + compareTypeCombinations + "</text--->"
  "<text>x < y </text--->"
  "<text>Returns TRUE if x is lower than y, false otherwise.</text--->"
  "<text>query x < y</text--->))";

Operator ltJNet("<", ltSpec, 10, ltMap, compareSelect, compareTM);

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
  "(<text>" + compareTypeCombinations + "</text--->"
  "<text>x > y </text--->"
  "<text>Returns TRUE if x is greater than y, false otherwise.</text--->"
  "<text>query x > y</text--->))";

Operator gtJNet(">", gtSpec, 10, gtMap, compareSelect, compareTM);

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
  "(<text>" + compareTypeCombinations + "</text--->"
  "<text>x < y </text--->"
  "<text>Returns TRUE if x is lower than or equal y, false otherwise."+
  "</text--->"
  "<text>query x <= y</text--->))";

Operator leJNet("<=", leSpec, 10, leMap, compareSelect, compareTM);

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
  "(<text>" + compareTypeCombinations + "</text--->"
  "<text>x >= y </text--->"
  "<text>Returns TRUE if x is greater than or equal y, false otherwise."+
  "</text--->"
  "<text>query x >= y</text--->))";

  Operator geJNet(">=", geSpec, 10, geMap, compareSelect, compareTM);

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
  "(<text>" + compareTypeCombinations + "</text--->"
  "<text>x # y </text--->"
  "<text>Returns TRUE if x not equals y, false otherwise.</text--->"
  "<text>query x # y</text--->))";

Operator neJNet("#", neSpec, 10, neMap, compareSelect, compareTM);

/*
1.1 Bounding Boxes

1.1.1 ~tempnetbox~

Returns a 3 dimensional rectangle where x1 and x2 are identic and respresent
the route id, y1 represents the start position on this route and y2 represents
the end position on this route, and z1 is the start time and z2 is the end
time. All coordinates are double values.

*/

ListExpr tempnetboxTM (ListExpr args)
{
  if (!nl->HasLength(args,1))
    return listutils::typeError("One argument expected.");

  if (!listutils::isSymbol(nl->First(args), UJPoint::BasicType()))
    return listutils::typeError("Argument should be " + UJPoint::BasicType());

  return nl->SymbolAtom(Rectangle<3>::BasicType());
}

int tempnetboxVM ( Word* args, Word& result, int message, Word& local,
                   Supplier s )
{
  result = qp->ResultStorage( s );
  UJPoint *ju = ( UJPoint* ) args[0].addr;
  Rectangle<3>* r = static_cast<Rectangle<3>* > (result.addr);
  if (ju != NULL && ju->IsDefined())
  {
    *r = ju->TempNetBox();
  }
  else
    r->SetDefined(false);
  return 0;
}

const string tempnetboxSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + UJPoint::BasicType() + " -> " +  Rectangle<3>::BasicType() +
  "</text--->"
  "<text>tempnetbox(<ujpoint>) </text--->"
  "<text>Returns a three dimensional rectangle with double coordinates. "
  "The edges represent a temporal bounding box in terms of network. "
  " Whereas x1 and x2 are defined by the route id, y1 by the startposition "
  "on this route, y2 by the end position on this route, z1 by the start time "
  "and z2 by the end time of the junit.</text--->"
  "<text>query tempnetbox(testujp)</text--->))";

Operator tempnetboxJNet("tempnetbox", tempnetboxSpec, tempnetboxVM,
                         Operator::SimpleSelect, tempnetboxTM);

/*
1.1.1 ~netbox~

Returns a 2 dimensional rectangle where x1 and x2 are identic and respresent
the route id, y1 represents the start position on this route and y2 represents
the end position on this route, and z1 is the start time and z2 is the end
time. All coordinates are double values.

*/

const string maps_netbox[3][2] =
{
  {UJPoint::BasicType(), Rectangle<2>::BasicType()},
  {JRouteInterval::BasicType(), Rectangle<2>::BasicType()},
  {JPoint::BasicType(), Rectangle<2>::BasicType()}
};

ListExpr netboxTM (ListExpr args)
{
  return SimpleMaps<3,2>(maps_netbox, args);
}

int netboxSelect(ListExpr args)
{
  return SimpleSelect<3,2>(maps_netbox, args);
}

template<class InClass>
int netboxVM ( Word* args, Word& result, int message, Word& local,
                Supplier s )
{
  result = qp->ResultStorage( s );
  InClass *in = ( InClass* ) args[0].addr;
  Rectangle<2>* r = static_cast<Rectangle<2>* > (result.addr);
  if (in != NULL && in->IsDefined())
  {
    *r = in->NetBox();
  }
  else
    r->SetDefined(false);
  return 0;
}

ValueMapping netboxMap[] =
{
  netboxVM<UJPoint>,
  netboxVM<JRouteInterval>,
  netboxVM<JPoint>
};

const string netboxSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  UJPoint::BasicType() + " -> " +  Rectangle<2>::BasicType() + ", \n" +
  JRouteInterval::BasicType() + " -> " +  Rectangle<2>::BasicType() + ", \n" +
  JPoint::BasicType() + " -> " + Rectangle<2>::BasicType() +
  "</text--->"
  "<text>netbox(<junit>) </text--->"
  "<text>Returns a two dimensional rectangle with double coordinates. "
  "The edges represent a bounding box in terms of network. "
  " Whereas x1 and x2 are defined by the route id, y1 by the startposition "
  "on this route, and y2 by the end position on this route.</text--->"
  "<text>query netbox(testjunit)</text--->))";

Operator netboxJNet("netbox", netboxSpec, 3, netboxMap, netboxSelect, netboxTM);

/*
1.1.1 ~bbox~

Returns a 3 dimensional rectangle with the spatial temporal bounding box
of the input value.

*/

const string maps_bbox[2][2] =
{
 {MJPoint::BasicType(), Rectangle<3>::BasicType()},
 {UJPoint::BasicType(), Rectangle<3>::BasicType()}
};

ListExpr bboxTM (ListExpr args)
{
 return SimpleMaps<2,2>(maps_bbox, args);
}

int bboxSelect(ListExpr args)
{
 return SimpleSelect<2,2>(maps_bbox, args);
}

template<class InClass>
int bboxVM ( Word* args, Word& result, int message, Word& local,
            Supplier s )
{
 result = qp->ResultStorage( s );
 InClass *in = ( InClass* ) args[0].addr;
 Rectangle<3>* r = static_cast<Rectangle<3>* > (result.addr);
 if (in != NULL && in->IsDefined())
 {
   *r = in->BoundingBox();
 }
 else
   r->SetDefined(false);
 return 0;
}

ValueMapping bboxMap[] =
{
  bboxVM<MJPoint>,
  bboxVM<UJPoint>
};

const string bboxSpec =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "(<text>" +
   MJPoint::BasicType() + " -> " +  Rectangle<3>::BasicType() + ", \n"+
   UJPoint::BasicType() + " -> " +  Rectangle<3>::BasicType() +
   "</text--->"
   "<text>bbox(<mjpoint>) </text--->"
   "<text>Returns a three dimensional rectangle representing the spatial"
   " temporal bounding box of the in value.</text--->"
   "<text>query bbox(testmjp)</text--->))";

Operator bboxJNet( "bbox", bboxSpec, 2, bboxMap, bboxSelect, bboxTM);
/*
1.1. Extend Datatypes

1.1.1 ~union~

Concats the given values to one single object if possible, otherwise an
undefined object is returned.

*/


const string maps_union[2][3] =
{
  {MJPoint::BasicType(), MJPoint::BasicType(), MJPoint::BasicType()},
  {JLine::BasicType(), JLine::BasicType(), JLine::BasicType()}
};

ListExpr unionTM (ListExpr args)
{
  return SimpleMaps<2,3>(maps_union, args);
}

int unionSelect(ListExpr args)
{
  return SimpleSelect<2,3>(maps_union, args);
}

template<class IOClass>
int unionVM ( Word* args, Word& result, int message, Word& local,
              Supplier s )
{
  result = qp->ResultStorage( s );
  IOClass* src1 = ( IOClass* ) args[0].addr;
  IOClass* src2 = (IOClass*) args[1].addr;
  IOClass* r = static_cast<IOClass*> (result.addr);
  if (src1 != NULL && src2 != NULL)
  {
    src1->Union(src2, r);
  }
   else
     r->SetDefined(false);
   return 0;
}

ValueMapping unionMap[] =
{
  unionVM<MJPoint>,
  unionVM<JLine>
};

const string unionSpec =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "(<text>" +
   MJPoint::BasicType() + " X " +  MJPoint::BasicType() + " -> " +
   MJPoint::BasicType() + ", \n"+
   JLine::BasicType() + " X " +  JLine::BasicType() + " -> " +
   JLine::BasicType() +
   "</text--->"
   "<text><mjpoint> union <mjpoint2> </text--->"
   "<text>Returns an object consisting  of the union of both input objects"
   " if possible. Otherwise an undefined value is returned.</text--->"
   "<text>query testmjp union testmjp1</text--->))";

Operator unionJNet( "union", unionSpec, 2, unionMap, unionSelect, unionTM);

/*
1.1 Restrict Data Types

1.1.1 Restrict List Data Types

The TypeMap and Select Functions are equal for the update operators of lists.

*/

const string maps_updateLists[8][3] =
{
  {JListInt::BasicType(), CcInt::BasicType(), JListInt::BasicType()},
  {JListRLoc::BasicType(), RouteLocation::BasicType(), JListRLoc::BasicType()},
  {JListRInt::BasicType(), JRouteInterval::BasicType(), JListRInt::BasicType()},
  {JListNDG::BasicType(), NetDistanceGroup::BasicType(), JListNDG::BasicType()},
  {JListInt::BasicType(), JListInt::BasicType(), JListInt::BasicType()},
  {JListRLoc::BasicType(), JListRLoc::BasicType(), JListRLoc::BasicType()},
  {JListRInt::BasicType(), JListRInt::BasicType(), JListRInt::BasicType()},
  {JListNDG::BasicType(), JListNDG::BasicType(), JListNDG::BasicType()}
};

ListExpr updateListsTM (ListExpr args)
{
  return SimpleMaps<8,3>(maps_updateLists, args);
}

int updateListsSelect(ListExpr args)
{
  return SimpleSelect<8,3>(maps_updateLists, args);
}

const string updateTypeCombinations =
  JListInt::BasicType() + " x " + CcInt::BasicType() + " -> " +
  JListInt::BasicType() +", \n" +
  JListRLoc::BasicType() + " x " + RouteLocation::BasicType() + " -> " +
  JListRLoc::BasicType() + ", \n" +
  JListRInt::BasicType() + " x " + JRouteInterval::BasicType() + " -> " +
  JListRInt::BasicType() + ", \n" +
  JListNDG::BasicType() + " x " + NetDistanceGroup::BasicType() +" -> "+
  JListNDG::BasicType() + ", \n" +
  JListInt::BasicType() +" x "+ JListInt::BasicType() + " -> " +
  JListInt::BasicType() + ", \n" +
  JListRLoc::BasicType() + " x " + JListRLoc::BasicType() + " -> " +
  JListRLoc::BasicType() + ", \n" +
  JListRInt::BasicType() + "x " + JListRInt::BasicType() + " -> " +
  JListRInt::BasicType() +", \n" +
  JListNDG::BasicType() + " x " + JListNDG::BasicType() + " -> " +
  JListNDG::BasicType();

/*
1.1.1.1 ~minus~

The operator ~minus~ removes the given element, respectively list of elements,
from the list if it is contained.

*/

template<class List, class Elem>
int minusVM( Word* args, Word& result, int message, Word& local,
             Supplier s)
{
  result = qp->ResultStorage(s);
  List* res = static_cast<List*> (result.addr);

  List* list = (List*) args[0].addr;
  Elem* elem = (Elem*) args[1].addr;

  if (list != 0 && list->IsDefined() &&
      elem != 0 && elem->IsDefined())
  {
    *res = *list;
    res->operator-=(*elem);
  }
  else
    res->SetDefined(false);
  return 0;
}

ValueMapping minusMap[] =
{
  minusVM<JListInt, CcInt>,
  minusVM<JListRLoc, RouteLocation>,
  minusVM<JListRInt, JRouteInterval>,
  minusVM<JListNDG, NetDistanceGroup>,
  minusVM<JListInt, JListInt>,
  minusVM<JListRLoc, JListRLoc>,
  minusVM<JListRInt, JListRInt>,
  minusVM<JListNDG, JListNDG>
};

const string minusSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + updateTypeCombinations + "</text--->"
  "<text>x - y </text--->"
  "<text>Returns x without y.</text--->"
  "<text>query x - y</text--->))";

Operator minusJNet("-", minusSpec, 8, minusMap, updateListsSelect,
                   updateListsTM);

/*
1.1.1.1 ~restrict~

The operator ~restrict~ restricts the list to contain only the given elements,
respectively list of elements, if they have been inside before.

*/

template<class List, class Elem>
int restrictVM( Word* args, Word& result, int message, Word& local,
                Supplier s)
{
  result = qp->ResultStorage(s);
  List* res = static_cast<List*> (result.addr);

  List* list = (List*) args[0].addr;
  Elem* elem = (Elem*) args[1].addr;

  if (list != 0 && list->IsDefined() &&
    elem != 0 && elem->IsDefined())
  {
    *res = *list;
    res->Restrict(*elem);
  }
  else
    res->SetDefined(false);
  return 0;
}

ValueMapping restrictMap[] =
{
  restrictVM<JListInt, CcInt>,
  restrictVM<JListRLoc, RouteLocation>,
  restrictVM<JListRInt, JRouteInterval>,
  restrictVM<JListNDG, NetDistanceGroup>,
  restrictVM<JListInt, JListInt>,
  restrictVM<JListRLoc, JListRLoc>,
  restrictVM<JListRInt, JListRInt>,
  restrictVM<JListNDG, JListNDG>
};

const string restrictSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + updateTypeCombinations + "</text--->"
  "<text>restrict(x,y) </text--->"
  "<text>Returns only the elements of x which are also in y.</text--->"
  "<text>query restrict (x,y)</text--->))";

Operator restrictJNet("restrict", restrictSpec, 8, restrictMap,
                      updateListsSelect, updateListsTM);

/*
1.1.1 Moving Data Types

1.1.1.1 By Time

1.1.1.1.1 ~initialJNet~

Returns the start position and time of the ~mjpoint~ as ~ijpoint~

*/

const string maps_initial[1][2] =
{
  {MJPoint::BasicType(), IJPoint::BasicType()}
};

ListExpr initialTM (ListExpr args)
{
  return SimpleMaps<1,2>(maps_initial, args);
}

int initialSelect(ListExpr args)
{
  return SimpleSelect<1,2>(maps_initial, args);
}

int initialVM ( Word* args, Word& result, int message, Word& local,
                Supplier s )
{
  result = qp->ResultStorage( s );
  MJPoint* mjp = ( MJPoint* ) args[0].addr;
  IJPoint* ijp = static_cast<IJPoint* > (result.addr);
  if (mjp != NULL && mjp->IsDefined())
  {
    *ijp = mjp->Initial();
  }
  else
    ijp->SetDefined(false);
  return 0;
}

ValueMapping initialMap[] =
{
  initialVM
};

const string initialSpec =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "(<text>" +
   MJPoint::BasicType() + " -> " + IJPoint::BasicType() +
    "</text--->"
    "<text>initial(<mjpoint>) </text--->"
    "<text>Returns an " + IJPoint::BasicType() + " with the start time and"
    " network position of the " + MJPoint::BasicType()+ "."
    "</text--->"
    "<text>query testmjp atinstant create_instant(0.5)</text--->))";

Operator initialJNet( "initial", initialSpec, 1, initialMap, initialSelect,
                      initialTM);

/*
1.1.1.1.1 ~atinstant~

Returns an ijpoint telling the position of the mjpoint at the given timeinstant.

*/

const string maps_atinstant[1][3] =
{
  {MJPoint::BasicType(), Instant::BasicType(),IJPoint::BasicType()}
};

ListExpr atinstantTM (ListExpr args)
{
  return SimpleMaps<1,3>(maps_atinstant, args);
}

int atinstantSelect(ListExpr args)
{
  return SimpleSelect<1,3>(maps_atinstant, args);
}

int atinstantVM ( Word* args, Word& result, int message, Word& local,
             Supplier s )
{
  result = qp->ResultStorage( s );
  MJPoint* mjp = ( MJPoint* ) args[0].addr;
  Instant* inst = (Instant*) args[1].addr;
  IJPoint* ijp = static_cast<IJPoint* > (result.addr);
  if (mjp != NULL && mjp->IsDefined() &&
      inst != NULL && inst->IsDefined())
  {
    *ijp = mjp->AtInstant(inst);
  }
   else
     ijp->SetDefined(false);
   return 0;
}

ValueMapping atinstantMap[] =
{
  atinstantVM
};

const string atinstantSpec =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "(<text>" +
   MJPoint::BasicType() + " X " + Instant::BasicType() +" -> " +
   IJPoint::BasicType() +
   "</text--->"
   "<text><mjpoint> atinstant <instant> </text--->"
   "<text>Returns an " + IJPoint::BasicType() + " describing the network "
   "position of the " + MJPoint::BasicType()+ " at the given time instant."
   "</text--->"
   "<text>query testmjp atinstant create_instant(0.5)</text--->))";

Operator atinstantJNet( "atinstant", atinstantSpec, 1, atinstantMap,
                        atinstantSelect, atinstantTM);

/*

1.1.1.1.1 ~atperiods~

Restricts the mjpoint to the given periods.

*/

const string maps_atperiods[1][3] =
{
  {MJPoint::BasicType(), Periods::BasicType(), MJPoint::BasicType()}
};

ListExpr atperiodsTM (ListExpr args)
{
  return SimpleMaps<1,3>(maps_atperiods, args);
}

int atperiodsSelect(ListExpr args)
{
  return SimpleSelect<1,3>(maps_atperiods, args);
}

int atperiodsVM ( Word* args, Word& result, int message, Word& local,
                  Supplier s )
{
  result = qp->ResultStorage( s );
  MJPoint* mjp = ( MJPoint* ) args[0].addr;
  Periods* inst = (Periods*) args[1].addr;
  MJPoint* res = static_cast<MJPoint* > (result.addr);
  if (mjp != NULL && mjp->IsDefined() &&
      inst != NULL && inst->IsDefined())
  {
    mjp->AtPeriods(inst, *res);
  }
  else
    res->SetDefined(false);
  return 0;
}

ValueMapping atperiodsMap[] =
{
  atperiodsVM
};

const string atperiodsSpec =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "(<text>" +
   MJPoint::BasicType() + " X " + Periods::BasicType() +" -> " +
   MJPoint::BasicType() +
   "</text--->"
   "<text><mjpoint> atperiods <periods> </text--->"
   "<text>Returns an " + MJPoint::BasicType() + " restricted to the given " +
   Periods::BasicType()  + ".</text--->"
   "<text>query testmjp atperiods testperiod </text--->))";

Operator atperiodsJNet( "atperiods", atperiodsSpec, 1, atperiodsMap,
                        atperiodsSelect, atperiodsTM);

/*
1.1.1.1 By Place

1.1.1.1.1 ~at~

Restricts the ~mjpoint~ to the times it was at the given positions

*/

const string maps_at[2][3] =
{
  {MJPoint::BasicType(), JPoint::BasicType(), MJPoint::BasicType()},
  {MJPoint::BasicType(), JLine::BasicType(), MJPoint::BasicType()}
};

ListExpr atTM (ListExpr args)
{
  return SimpleMaps<2,3>(maps_at, args);
}

int atSelect(ListExpr args)
{
  return SimpleSelect<2,3>(maps_at, args);
}

const string atTypeCombinations =
  MJPoint::BasicType() + " X " + JPoint::BasicType() +" -> " +
  MJPoint::BasicType() + ", \n" +
  MJPoint::BasicType() + " X " + JLine::BasicType() +" -> " +
  MJPoint::BasicType() ;

template<class Places>
int atVM ( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  MJPoint* mjp = ( MJPoint* ) args[0].addr;
  Places* p = (Places*) args[1].addr;
  MJPoint* res = static_cast<MJPoint* > (result.addr);
  if (mjp != NULL && mjp->IsDefined() &&
      p != NULL && p->IsDefined())
  {
    mjp->At(p,*res);
  }
  else
    res->SetDefined(false);
  return 0;
}

ValueMapping atMap[] =
{
  atVM<JPoint>,
  atVM<JLine>
};

const string atSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + atTypeCombinations +
  "</text--->"
  "<text><mjpoint> at <jpoint> </text--->"
  "<text>Restricts the " + MJPoint::BasicType() + " to the given places."
  "</text--->"
  "<text>query testmjp at testjp</text--->))";

Operator atJNet( "at", atSpec, 2, atMap, atSelect, atTM);

/*
1.1 Access to parts of data types

1.1.1 ~length~

*/

const string maps_length[1][2] =
{
  {MJPoint::BasicType(), CcReal::BasicType()}
};

ListExpr lengthTM (ListExpr args)
{
  return SimpleMaps<1,2>(maps_length, args);
}

int lengthSelect(ListExpr args)
{
  return SimpleSelect<1,2>(maps_length, args);
}

int lengthVM ( Word* args, Word& result, int message, Word& local,
               Supplier s )
{
  result = qp->ResultStorage( s );
  MJPoint* mjp = ( MJPoint* ) args[0].addr;
  CcReal* res = static_cast<CcReal* > (result.addr);
  if (mjp != NULL && mjp->IsDefined())
  {
    res->Set(true, mjp->Length());
  }
  else
    res->SetDefined(false);
  return 0;
}

ValueMapping lengthMap[] =
{
  lengthVM
};

const string lengthSpec =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
   "(<text>" +
   MJPoint::BasicType() + " -> " + CcReal::BasicType() +
   "</text--->"
   "<text>length(<mjpoint>) </text--->"
   "<text>Returns the total driven length of an " + MJPoint::BasicType() +
   ".</text--->"
   "<text>query length(testmjp) </text--->))";

Operator lengthJNet( "length", lengthSpec, 1, lengthMap, lengthSelect,
                     lengthTM);

/*
1.1.1 JNetwork Data

1.1.1.1 ~sections~

Returns the sections relation of a jnet.

*/

ListExpr sectionsTM (ListExpr args)
{
  if (!nl->HasLength(args,1))
    return listutils::typeError("One argument expected.");

  if (!listutils::isSymbol(nl->First(args), JNetwork::BasicType()))
    return listutils::typeError("Argument should be " + JNetwork::BasicType());

  ListExpr retType;
  nl->ReadFromString ( JNetwork::GetSectionsRelationType(), retType );
  return retType;
}

int sectionsVM ( Word* args, Word& result, int message, Word& local,
                 Supplier s )
{
  JNetwork *jnet = ( JNetwork* ) args[0].addr;
  if (jnet != NULL && jnet->IsDefined())
  {
    Relation *resultSt = ( Relation* ) qp->ResultStorage ( s ).addr;
    resultSt->Close();
    result = SetWord ( jnet->GetSectionsCopy());
    qp->ChangeResultStorage ( s, result );
  }
  return 0;
}

const string sectionsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  JNetwork::BasicType() + " -> " + JNetwork::GetSectionsRelationType() +
  "</text--->"
  "<text>sections(<jnet>) </text--->"
  "<text>Returns a copy of the sections relation of the given jnet.</text--->"
  "<text>query sections(testnet)</text--->))";

Operator sectionsJNet("sections", sectionsSpec, sectionsVM,
                    Operator::SimpleSelect, sectionsTM);

/*

1.1.1.1 ~routes~

Returns the routes relation of a jnet.

*/

ListExpr routesTM (ListExpr args)
{
  if (!nl->HasLength(args,1))
    return listutils::typeError("One argument expected.");

  if (!listutils::isSymbol(nl->First(args), JNetwork::BasicType()))
    return listutils::typeError("Argument should be " + JNetwork::BasicType());

  ListExpr retType;
  nl->ReadFromString ( JNetwork::GetRoutesRelationType(), retType );
  return retType;
}

int routesVM ( Word* args, Word& result, int message, Word& local,
                 Supplier s )
{
  JNetwork *jnet = ( JNetwork* ) args[0].addr;
  if (jnet != NULL && jnet->IsDefined())
  {
    Relation *resultSt = ( Relation* ) qp->ResultStorage ( s ).addr;
    resultSt->Close();
    result = SetWord ( jnet->GetRoutesCopy());
    qp->ChangeResultStorage ( s, result );
  }
  return 0;
}

const string routesSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  JNetwork::BasicType() + " -> " +   JNetwork::GetRoutesRelationType() +
  "</text--->"
  "<text>routes(<jnet>) </text--->"
  "<text>Returns a copy of the routes relation of the given jnet.</text--->"
  "<text>query routes(testnet)</text--->))";

Operator routesJNet("routes", routesSpec, routesVM, Operator::SimpleSelect,
                  routesTM);

/*
1.1.1.1 ~junctions~

Returns the junctions relation of a jnet.

*/

ListExpr junctionsTM (ListExpr args)
{
  if (!nl->HasLength(args,1))
    return listutils::typeError("One argument expected.");

  if (!listutils::isSymbol(nl->First(args), JNetwork::BasicType()))
    return listutils::typeError("Argument should be " + JNetwork::BasicType());

  ListExpr retType;
  nl->ReadFromString ( JNetwork::GetJunctionsRelationType(), retType );
  return retType;
}

int junctionsVM ( Word* args, Word& result, int message, Word& local,
                  Supplier s )
{
  JNetwork *jnet = ( JNetwork* ) args[0].addr;
  if (jnet != NULL && jnet->IsDefined())
  {
    Relation *resultSt = ( Relation* ) qp->ResultStorage ( s ).addr;
    resultSt->Close();
    result = SetWord ( jnet->GetJunctionsCopy());
    qp->ChangeResultStorage ( s, result );
  }
  return 0;
}

const string junctionsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  JNetwork::BasicType() + " -> " + JNetwork::GetJunctionsRelationType() +
  "</text--->"
  "<text>junctions(<jnet>) </text--->"
  "<text>Returns a copy of the junctions relation of the given jnet.</text--->"
  "<text>query junctions(testnet)</text--->))";

Operator junctionsJNet("junctions", junctionsSpec, junctionsVM,
                     Operator::SimpleSelect, junctionsTM);

/*
1.1.1.1 ~distances~

Returns the netdistances relation of a jnet.

*/

ListExpr distancesTM (ListExpr args)
{
  if (!nl->HasLength(args,1))
    return listutils::typeError("One argument expected.");

  if (!listutils::isSymbol(nl->First(args), JNetwork::BasicType()))
    return listutils::typeError("Argument should be " + JNetwork::BasicType());

  ListExpr retType;
  nl->ReadFromString ( JNetwork::GetNetdistancesRelationType(), retType );
  return retType;
}

int distancesVM ( Word* args, Word& result, int message, Word& local,
                  Supplier s )
{
  JNetwork *jnet = ( JNetwork* ) args[0].addr;
  if (jnet != NULL && jnet->IsDefined())
  {
    result = qp->ResultStorage(s).addr;
    ListExpr typList;
    nl->ReadFromString(jnet->GetNetdistancesRelationType(),typList);
    ListExpr numType = SecondoSystem::GetCatalog()->NumericType(typList);
    OrderedRelation::Close(numType, result);
    result = SetWord ( jnet->GetNetdistancesCopy());
    qp->ChangeResultStorage ( s, result );
  }
  return 0;
}

const string distancesSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  JNetwork::BasicType() + " -> " + JNetwork::GetNetdistancesRelationType() +
  "</text--->"
  "<text>distances(<jnet>) </text--->"
  "<text>Returns a copy of the netdistances relation of the given jnet."+
  "</text--->"
  "<text>query distances(testnet)</text--->))";

Operator distancesJNet("distances", distancesSpec, distancesVM,
                       Operator::SimpleSelect, distancesTM);


/*
1.1.1 Of MJPoint

1.1.1.1 trajectory

Returns an ~jline~ representing the movement of the mjpoint in the network.

*/

ListExpr trajectoryTM (ListExpr args)
{
  if (!nl->HasLength(args,1))
    return listutils::typeError("One argument expected.");

  if (!listutils::isSymbol(nl->First(args), MJPoint::BasicType()))
    return listutils::typeError("Argument should be " + MJPoint::BasicType());

  return nl->SymbolAtom(JLine::BasicType());
}

int trajectoryVM ( Word* args, Word& result, int message, Word& local,
                   Supplier s )
{
  result = qp->ResultStorage( s );
  MJPoint *mjp = ( MJPoint* ) args[0].addr;
  JLine* jl = static_cast<JLine*> (result.addr);
  if (mjp != NULL)
  {
    mjp->Trajectory(jl);
  }
  else
    jl->SetDefined(false);
  return 0;
}

const string trajectorySpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  MJPoint::BasicType() + " -> " + JLine::BasicType() +
  "</text--->"
  "<text>trajectory(<testmjp>) </text--->"
  "<text>Returns an "+ JLine::BasicType() + " representing the movement of "
  "the "+ MJPoint::BasicType() + " in the network.</text--->"
  "<text>query trajectory(testmjp)</text--->))";

Operator trajectoryJNet("trajectory", trajectorySpec, trajectoryVM,
                         Operator::SimpleSelect, trajectoryTM);

/*
1.1.1 Of IJPoint

1.1.1.1 ~val~

Returns the ~jpoint~ part of an ~ijpoint~.

*/

const string maps_val[1][2] =
{
  {IJPoint::BasicType(), JPoint::BasicType()}
};

ListExpr valTM (ListExpr args)
{
  return SimpleMaps<1,2>(maps_val, args);
}

int valSelect(ListExpr args)
{
  return SimpleSelect<1,2>(maps_val, args);
}

int valVM ( Word* args, Word& result, int message, Word& local,
            Supplier s )
{
  result = qp->ResultStorage( s );
  IJPoint *in = ( IJPoint* ) args[0].addr;
  JPoint* jp = static_cast<JPoint* > (result.addr);
  if (in != NULL && in->IsDefined())
  {
    *jp = in->GetPoint();
  }
  else
     jp->SetDefined(false);
  return 0;
}

ValueMapping valMap[] =
{
  valVM
};

const string valSpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
      "(<text>" +
      IJPoint::BasicType() + " -> " +  JPoint::BasicType() +
      "</text--->"
      "<text>val(<ijpoint>) </text--->"
      "<text>Returns the "+ JPoint::BasicType() +" value of the given " +
      IJPoint::BasicType() + " value.</text--->"
      "<text>query val(testijp)</text--->))";

Operator valJNet( "val", valSpec, 1, valMap, valSelect, valTM);

/*
1.1.1.1 ~inst~

Returns the timeinstant part of an ijpoint.

*/

const string maps_inst[1][2] =
{
  {IJPoint::BasicType(), Instant::BasicType()}
};

ListExpr instTM (ListExpr args)
{
  return SimpleMaps<1,2>(maps_inst, args);
}

int instSelect(ListExpr args)
{
  return SimpleSelect<1,2>(maps_inst, args);
}


int instVM ( Word* args, Word& result, int message, Word& local,
             Supplier s )
{
  result = qp->ResultStorage( s );
  IJPoint *in = ( IJPoint* ) args[0].addr;
  Instant* inst = static_cast<Instant* > (result.addr);
  if (in != NULL && in->IsDefined())
    *inst = in->GetInstant();
  else
    inst->SetDefined(false);
  return 0;
}

ValueMapping instMap[] =
{
  instVM
};

const string instSpec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"(<text>" +
IJPoint::BasicType() + " -> " +  Instant::BasicType() +
"</text--->"
"<text>inst(<ijpoint>) </text--->"
"<text>Returns the time instant of the " + IJPoint::BasicType() +" value "
"</text--->"
"<text>query inst(testijp)</text--->))";

Operator instJNet( "inst", instSpec, 1, instMap, instSelect, instTM);

/*
1.1 Check Properties of Data Types

1.1.1 ~isempty~

Returns true if the DbArray of the data type has no content.

*/


const string maps_isempty[2][2] =
{
  {MJPoint::BasicType(), CcBool::BasicType()},
  {JLine::BasicType(), CcBool::BasicType()}
};

ListExpr isemptyTM (ListExpr args)
{
  return SimpleMaps<2,2>(maps_isempty, args);
}

int isemptySelect(ListExpr args)
{
  return SimpleSelect<2,2>(maps_isempty, args);
}

template<class InClass>
int isemptyVM ( Word* args, Word& result, int message, Word& local,
            Supplier s )
{
  result = qp->ResultStorage( s );
  InClass *in = ( InClass* ) args[0].addr;
  CcBool* res = static_cast<CcBool* > (result.addr);
  if (in != NULL && in->IsDefined())
    res->Set(true, in->IsEmpty());
  else
    res->SetDefined(false);
  return 0;
}

ValueMapping isemptyMap[] =
{
  isemptyVM<MJPoint>,
  isemptyVM<JLine>
};

const string isemptySpec =
      "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
      "(<text>" +
      MJPoint::BasicType() + " -> " +  CcBool::BasicType() + ", \n" +
      JLine::BasicType() + " -> " +  CcBool::BasicType() +
      "</text--->"
      "<text>isempty(<mjpoint>) </text--->"
      "<text>Returns true if the inserted value is empty.</text--->"
      "<text>query isempty(testmjp)</text--->))";

Operator isemptyJNet( "isempty", isemptySpec, 2, isemptyMap, isemptySelect,
                      isemptyTM);

/*
1.1.1 ~passes~

Returns true if the ~mjpoint~ passes at least once the given ~jline~ or
~jpoint~.

*/

const string maps_passes[2][3] =
{
  {MJPoint::BasicType(), JPoint::BasicType(), CcBool::BasicType()},
  {MJPoint::BasicType(), JLine::BasicType(), CcBool::BasicType()}
};

ListExpr passesTM (ListExpr args)
{
  return SimpleMaps<2,3>(maps_passes, args);
}

int passesSelect(ListExpr args)
{
  return SimpleSelect<2,3>(maps_passes, args);
}

template<class Arg2>
int passesVM( Word* args, Word& result, int message, Word& local,
              Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*> (result.addr);
  MJPoint* mjp = static_cast<MJPoint*> (args[0].addr);
  Arg2* obj = static_cast<Arg2*> (args[1].addr);
  if (mjp != NULL && mjp->IsDefined() &&
      obj != NULL && obj->IsDefined())
    res->Set(true, mjp->Passes(obj));
  else
    res->Set(false, false);
  return 0;
}

ValueMapping passesMap[] =
{
  passesVM<JPoint>,
  passesVM<JLine>
};

const string passesSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  MJPoint::BasicType() + " x " + JPoint::BasicType() + " -> " +
  CcBool::BasicType() + ", \n" +
  MJPoint::BasicType() + " x " + JLine::BasicType() + " -> " +
  CcBool::BasicType() +
  "</text--->"
  "<text><mjpoint> passes <jpoint> </text--->"
  "<text>Returns true if the "+MJPoint::BasicType() + " passes at least once"
  " the given "+JPoint::BasicType()+" resp. "+ JLine::BasicType() + ", false "
  "elsewhere.</text--->"
  "<text>query testmjp passes testjp </text--->))";

Operator passesJNet("passes", passesSpec, 2, passesMap, passesSelect, passesTM);

/*

1.1.1 ~intersects~

Returns true if the two ~mjpoint~ pass the same place at the same time. False
otherwise.

*/

const string maps_intersects[1][3] =
{
  {MJPoint::BasicType(), MJPoint::BasicType(), CcBool::BasicType()}
};

ListExpr intersectsTM (ListExpr args)
{
  return SimpleMaps<1,3>(maps_intersects, args);
}

int intersectsSelect(ListExpr args)
{
  return SimpleSelect<1,3>(maps_intersects, args);
}

int intersectsVM( Word* args, Word& result, int message, Word& local,
                  Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*> (result.addr);
  MJPoint* mjp1 = static_cast<MJPoint*> (args[0].addr);
  MJPoint* mjp2 = static_cast<MJPoint*> (args[1].addr);
  if (mjp1 != NULL && mjp1->IsDefined() && !mjp1->IsEmpty() &&
      mjp2 != NULL && mjp2->IsDefined() && !mjp2->IsEmpty())
    res->Set(true, mjp1->Intersects(mjp2));
  else
    res->Set(false, false);
  return 0;
}

ValueMapping intersectsMap[] =
{
  intersectsVM
};

const string intersectsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  MJPoint::BasicType() + " x " + MJPoint::BasicType() + " -> " +
  CcBool::BasicType() +
  "</text--->"
  "<text><mjpoint1> intersects <mjpoint2> </text--->"
  "<text>Returns true if the two "+ MJPoint::BasicType() + " are at least once"
  " at the same place at the same time, false otherwise.</text--->"
  "<text>query testmjp intersects testmjp </text--->))";

Operator intersectsJNet("intersects", intersectsSpec, 1, intersectsMap,
                        intersectsSelect, intersectsTM);

/*
1.1.1 ~present~

Returns true if the ~mjpoint~ is defined at least once in the given ~periods~.

*/

const string maps_present[2][3] =
{
  {MJPoint::BasicType(), Periods::BasicType(), CcBool::BasicType()},
  {MJPoint::BasicType(), Instant::BasicType(), CcBool::BasicType()}
};

ListExpr presentTM (ListExpr args)
{
  return SimpleMaps<2,3>(maps_present, args);
}

int presentSelect(ListExpr args)
{
  return SimpleSelect<2,3>(maps_present, args);
}

template<class InClass>
int presentVM( Word* args, Word& result, int message, Word& local,
              Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*> (result.addr);
  MJPoint* mjp = static_cast<MJPoint*> (args[0].addr);
  InClass* per = static_cast<InClass*> (args[1].addr);
  if (mjp != NULL && mjp->IsDefined() &&
      per != NULL && per->IsDefined())
    res->Set(true, mjp->Present(per));
  else
    res->Set(false, false);
  return 0;
}

ValueMapping presentMap[] =
{
  presentVM<Periods>,
  presentVM<Instant>
};

const string presentSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  MJPoint::BasicType() + " x " + Periods::BasicType() + " -> " +
  CcBool::BasicType() + ", \n" +
  MJPoint::BasicType() + " x " + Instant::BasicType() + " -> " +
  CcBool::BasicType() +
  "</text--->"
  "<text><mjpoint> present <periods> </text--->"
  "<text>Returns true if the "+MJPoint::BasicType() + " is defined at least "
   "once in  the given "+ Periods::BasicType()+ " of time, respectively" +
   " at the given time instant, false elsewhere.</text--->"
  "<text>query testmjp present testperiod </text--->))";

Operator presentJNet("present", presentSpec, 2, presentMap, presentSelect,
                     presentTM);

/*
1.1.1 ~inside~

Returns true if the ~jpoint~ is inside the ~jline~, false otherwise.

*/

const string maps_inside[1][3] =
{
  {JPoint::BasicType(), JLine::BasicType(), CcBool::BasicType()}
};

ListExpr insideTM(ListExpr args)
{
  return SimpleMaps<1,3>(maps_inside, args);
}

int insideSelect(ListExpr args)
{
  return SimpleSelect<1,3>(maps_inside, args);
}

int insideVM( Word* args, Word& result, int message, Word& local,
              Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* res = static_cast<CcBool*> (result.addr);
  JPoint* jp = static_cast<JPoint*> (args[0].addr);
  JLine* jl = static_cast<JLine*> (args[1].addr);
  if (jp != NULL && jp->IsDefined() &&
      jl != NULL && jl->IsDefined())
    res->Set(true, jl->Contains(jp));
  else
    res->Set(false, false);
  return 0;
}

ValueMapping insideMap[] =
{
  insideVM
};

const string insideSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  JPoint::BasicType() + " x " + JLine::BasicType() + " -> " +
  CcBool::BasicType() +
  "</text--->"
  "<text><jpoint> inside <jline> </text--->"
  "<text>Returns true if the "+ JPoint::BasicType() + " is inside the "
  + JLine::BasicType()+ ", false otherwise.</text--->"
  "<text>query testmjp present testjp </text--->))";

Operator insideJNet("inside", insideSpec, 1, insideMap, insideSelect,
                    insideTM);

/*
1.1 Network Operations

1.1.1 ~netdistance~

1.1 Translation beteween spatial(-temporal) and network(-temporal) data types

1.1.1 ~tonetwork~

Computes to a given spatial(-temporal) value the corresponding jnet value for
the given jnet.

*/

const string maps_tonetwork[3][3] =
{
  {JNetwork::BasicType(), Point::BasicType(), JPoint::BasicType()},
  {JNetwork::BasicType(), Line::BasicType(), JLine::BasicType()},
  {JNetwork::BasicType(), MPoint::BasicType(), MJPoint::BasicType()}
};

ListExpr tonetworkTM (ListExpr args)
{
  return SimpleMaps<3,3>(maps_tonetwork, args);
}

int tonetworkSelect(ListExpr args)
{
  return SimpleSelect<3,3>(maps_tonetwork, args);
}

template<class InType, class OutType>
int tonetworkVM( Word* args, Word& result, int message, Word& local,
                 Supplier s)
{
  result = qp->ResultStorage(s);
  OutType* res = static_cast<OutType*> (result.addr);
  JNetwork* jnet = static_cast<JNetwork*> (args[0].addr);
  InType* in = static_cast<InType*> (args[1].addr);
  if (jnet != NULL && jnet->IsDefined() &&
    in != NULL && in->IsDefined())
    res->FromSpatial(jnet, in);
  else
    res->SetDefined(false);
  return 0;
}

ValueMapping tonetworkMap[] =
{
  tonetworkVM<Point, JPoint>,
  tonetworkVM<Line, JLine>,
  tonetworkVM<MPoint, MJPoint>
};

const string tonetworkSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  JNetwork::BasicType() + " x " + Point::BasicType() + " -> " +
  JPoint::BasicType() + ", \n" +
  JNetwork::BasicType() + " x " + Line::BasicType() + " -> " +
  JLine::BasicType() + ", \n" +
  JNetwork::BasicType() + " x " + MPoint::BasicType() + " -> " +
  MJPoint::BasicType() + "</text--->"
  "<text>tonetwork( <jnetwork> <spatialobject>) </text--->"
  "<text>Translates the spatial or spatiotemporal object into the " +
  "corresponding object of the given " +  JNetwork::BasicType() +
  " if possible.</text--->"
  "<text>query tonetwork(testjnet, [const mpoint value(((\"2007-01-01-" +
  "10:02:01.000\" \"2007-01-01-10:02:03.000\" TRUE FALSE)(8209.0 8769.0 " +
  " 8293.0 8768.0)))]) </text--->))";

Operator tonetworkJNet("tonetwork", tonetworkSpec, 3, tonetworkMap,
                       tonetworkSelect, tonetworkTM);

/*
1.1.1 ~fromnetwork~

Computes from a given jnet data type the corresponding  spatial(-temporal)
data type.

*/

const string maps_fromnetwork[3][2] =
{
  {JPoint::BasicType(), Point::BasicType()},
  {JLine::BasicType(), Line::BasicType()},
  {MJPoint::BasicType(), MPoint::BasicType()}
};

ListExpr fromnetworkTM (ListExpr args)
{
  return SimpleMaps<3,2>(maps_fromnetwork, args);
}

int fromnetworkSelect(ListExpr args)
{
  return SimpleSelect<3,2>(maps_fromnetwork, args);
}

template<class InType, class OutType>
int fromnetworkVM( Word* args, Word& result, int message, Word& local,
                  Supplier s)
{
  result = qp->ResultStorage(s);
  OutType* res = static_cast<OutType*> (result.addr);
  InType* in = static_cast<InType*> (args[0].addr);
  if (in != NULL && in->IsDefined())
    in->ToSpatial(*res);
  else
    res->SetDefined(false);
  return 0;
}

ValueMapping fromnetworkMap[] =
{
  fromnetworkVM<JPoint, Point>,
  fromnetworkVM<JLine, Line>,
  fromnetworkVM<MJPoint, MPoint>
};

const string fromnetworkSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" +
  JPoint::BasicType() + " -> " + Point::BasicType() +  ", \n" +
  JLine::BasicType() + " -> " + Line::BasicType() + ", \n" +
  MJPoint::BasicType() + " -> " + MPoint::BasicType() + "</text--->"
  "<text>fromnetwork(<jnetobject>) </text--->"
  "<text>Translates the jnet object into corresponding spatial or "+
  " spatiotemporal object.</text--->"
  "<text>query fromnetwork(testjp) </text--->))";

Operator fromnetworkJNet("fromnetwork", fromnetworkSpec, 3, fromnetworkMap,
                         fromnetworkSelect, fromnetworkTM);

/*
1 Implementation of ~class JNetAlgebra~

1.1 Constructor

*/

JNetAlgebra::JNetAlgebra():Algebra()
{

/*
1.1.1 Integration of Data Types by Type Constructors

1.1.1.1  Basic Data Types Used By Network Data Types

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
1.1.1.1 Simple Temporal Data Types

*/

  AddTypeConstructor(&junitTC);
  junitTC.AssociateKind(Kind::DATA());
  junitTC.AssociateKind(Kind::TEMPORAL());


/*
1.1.1.1 The Central JNetwork Datatype

*/

  AddTypeConstructor (&jnetworkTC);
  jnetworkTC.AssociateKind(Kind::JNETWORK());

/*
1.1.1.1 Datatypes Depending on Existing Networks

*/

  AddTypeConstructor (&jpointTC);
  jpointTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&jpointsTC);
  jpointsTC.AssociateKind(Kind::DATA());

  AddTypeConstructor (&jlineTC);
  jlineTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&ijpointTC);
  ijpointTC.AssociateKind(Kind::DATA());
  ijpointTC.AssociateKind(Kind::TEMPORAL());

  AddTypeConstructor(&ujpointTC);
  ujpointTC.AssociateKind(Kind::DATA());
  ujpointTC.AssociateKind(Kind::TEMPORAL());

  AddTypeConstructor(&mjpointTC);
  mjpointTC.AssociateKind(Kind::DATA());
  mjpointTC.AssociateKind(Kind::TEMPORAL());

/*
1.1.1 Integration of Operators

1.1.1.1 Creation

1.1.1.1.1 Of Datatypes

*/

  AddOperator(&createrlocJNet);
  AddOperator(&createrintJNet);
  AddOperator(&createndgJNet);

  AddOperator(&createjnetJNet);

  AddOperator(&createjpointJNet);
  AddOperator(&createjlineJNet);
  AddOperator(&createjpointsJNet);

  AddOperator(&createijpointJNet);
  AddOperator(&createujpointJNet);
  AddOperator(&createmjpointJNet);

  AddOperator(&createlistJNet);

/*
1.1.1.1.1 Of Streams

*/
  AddOperator(&createstreamJNet);
  AddOperator(&unitsJNet);
  AddOperator(&altrlocsJNet);

/*
1.1.1.1 Comparision of Equal Datatypes

*/

  AddOperator(&eqJNet);
  AddOperator(&ltJNet);
  AddOperator(&gtJNet);
  AddOperator(&leJNet);
  AddOperator(&geJNet);
  AddOperator(&neJNet);


/*
1.1.1.1 Bounding Boxes and Network Bounding Boxes

*/

  AddOperator(&tempnetboxJNet);
  AddOperator(&netboxJNet);
  AddOperator(&bboxJNet);
/*
1.1.1.1 Extend Datatypes

*/

  AddOperator(&unionJNet);

/*
1.1.1.1 Restrict Datatypes

1.1.1.1.1 List Datatypes

*/
  AddOperator(&minusJNet);
  AddOperator(&restrictJNet);

/*
1.1.1.1.1 Moving Data Types

1.1.1.1.1.1 By Time

*/

  AddOperator(&initialJNet);
  AddOperator(&atinstantJNet);
  AddOperator(&atperiodsJNet);

/*
1.1.1.1.1 By Place

*/

 AddOperator(&atJNet);


/*
1.1.1.1 Access to parts of Data Types

*/

  AddOperator(&lengthJNet);

/*
1.1.1.1.1 JNet Components

*/

  AddOperator(&sectionsJNet);
  AddOperator(&routesJNet);
  AddOperator(&junctionsJNet);
  AddOperator(&distancesJNet);

/*
1.1.1.1.1 IJPoint Components

*/

  AddOperator(&valJNet);
  AddOperator(&instJNet);

/*
1.1.1.1.1 MJPoint Components

*/

  AddOperator(&trajectoryJNet);


/*
1.1.1.1 Check Properties of Data Types

*/

  AddOperator(&isemptyJNet);
  AddOperator(&passesJNet);
  AddOperator(&intersectsJNet);
  AddOperator(&presentJNet);
  AddOperator(&insideJNet);

/*
1.1.1.1 Network Operations

*/

  //AddOperator(&netdistanceJNet);

/*
1.1.1.1 Translation between spatial(-temporal) and network(-temporal) data types

*/

  AddOperator(&tonetworkJNet);
  AddOperator(&fromnetworkJNet);

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
