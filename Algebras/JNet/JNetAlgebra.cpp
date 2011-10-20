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

2011, April Simone Jandt

1 Includes

*/

#include "QueryProcessor.h"
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
#include "JListTID.h"
#include "PairTIDRLoc.h"
#include "ListPTIDRLoc.h"
#include "PairTIDRInterval.h"
#include "ListPTIDRInt.h"
#include "NetDistanceGroup.h"
#include "ListNetDistGrp.h"
#include "JNetwork.h"
#include "JPoint.h"

using namespace std;
using namespace mappings;


extern NestedList* nl;
extern QueryProcessor* qp;

/*
1 Type Constructors

1.1 ~Direction~

*/

TypeConstructor directionTC(
  Direction::BasicType(),                //name
  Direction::Property,                   //describing signature
  Direction::Out, Direction::In,         //Out and In functions
  0, 0,                                  //SaveTo and RestoreFrom List functions
  Direction::Create, Direction::Delete,  //object creation and deletion
  Direction::Open, Direction::Save,      // object open and save
  Direction::Close, Direction::Clone,    //object close and clone
  Direction::Cast,                       //cast function
  Direction::SizeOf,                     //sizeof function
  Direction::KindCheck );                //kind checking function


/*
1.2 ~RouteLocation~

*/

TypeConstructor routeLocationTC(
  RouteLocation::BasicType(),                //name
  RouteLocation::Property,                   //describing signature
  RouteLocation::Out, RouteLocation::In,     //Out and In functions
  0, 0,                                  //SaveTo and RestoreFrom List functions
  RouteLocation::Create, RouteLocation::Delete, //object creation and deletion
  RouteLocation::Open, RouteLocation::Save,  // object open and save
  RouteLocation::Close, RouteLocation::Clone,//object close and clone
  RouteLocation::Cast,                       //cast function
  RouteLocation::SizeOf,                     //sizeof function
  RouteLocation::KindCheck );                //kind checking function

/*
1.3 ~JRouteInterval~

*/

TypeConstructor routeIntervalTC(
  JRouteInterval::BasicType(),                //name
  JRouteInterval::Property,                   //describing signature
  JRouteInterval::Out, JRouteInterval::In,     //Out and In functions
  0, 0,                                  //SaveTo and RestoreFrom List functions
  JRouteInterval::Create, JRouteInterval::Delete, //object creation and deletion
  JRouteInterval::Open, JRouteInterval::Save,  // object open and save
  JRouteInterval::Close, JRouteInterval::Clone,//object close and clone
  JRouteInterval::Cast,                       //cast function
  JRouteInterval::SizeOf,                     //sizeof function
  JRouteInterval::KindCheck );                //kind checking function

/*
1.4 ~JListTID~

*/

TypeConstructor jListTIDTC(
  JListTID::BasicType(),             //name
  JListTID::Property,                //describing signature
  JListTID::Out, JListTID::In,       //Out and In functions
  0, 0,                               //SaveTo and RestoreFrom List functions
  JListTID::Create, JListTID::Delete, //object creation and deletion
  JListTID::Open, JListTID::Save,     // object open and save
  JListTID::Close, JListTID::Clone,   //object close and clone
  JListTID::Cast,                     //cast function
  JListTID::SizeOf,                   //sizeof function
  JListTID::KindCheck );              //kind checking function

/*
1.5 ~PairTIDRLoc~

*/

TypeConstructor pTIDRLocTC(
  PairTIDRLoc::BasicType(),                //name
  PairTIDRLoc::Property,                   //describing signature
  PairTIDRLoc::Out, PairTIDRLoc::In,     //Out and In functions
  0, 0,                                  //SaveTo and RestoreFrom List functions
  PairTIDRLoc::Create, PairTIDRLoc::Delete, //object creation and deletion
  PairTIDRLoc::Open, PairTIDRLoc::Save,  // object open and save
  PairTIDRLoc::Close, PairTIDRLoc::Clone,//object close and clone
  PairTIDRLoc::Cast,                       //cast function
  PairTIDRLoc::SizeOf,                     //sizeof function
  PairTIDRLoc::KindCheck );                //kind checking function

/*
1.6 ~ListPairTIDRLoc~

*/

TypeConstructor listPTIDRLocTC(
  ListPTIDRLoc::BasicType(),                //name
  ListPTIDRLoc::Property,                   //describing signature
  ListPTIDRLoc::Out, ListPTIDRLoc::In,     //Out and In functions
  0, 0,                                  //SaveTo and RestoreFrom List functions
  ListPTIDRLoc::Create, ListPTIDRLoc::Delete,//obj creation and deletion
  ListPTIDRLoc::Open, ListPTIDRLoc::Save,  // object open and save
  ListPTIDRLoc::Close, ListPTIDRLoc::Clone,//object close and clone
  ListPTIDRLoc::Cast,                       //cast function
  ListPTIDRLoc::SizeOf,                     //sizeof function
  ListPTIDRLoc::KindCheck );                //kind checking function

/*
1.7 ~PairTIDRInterval~

*/

TypeConstructor pTIDRIntTC(
  PairTIDRInterval::BasicType(),         //name
  PairTIDRInterval::Property,            //describing signature
  PairTIDRInterval::Out, PairTIDRInterval::In, //Out and In functions
  0, 0,                                //SaveTo and RestoreFrom List functions
  PairTIDRInterval::Create, PairTIDRInterval::Delete,//obj creation and deletion
  PairTIDRInterval::Open, PairTIDRInterval::Save, // object open and save
  PairTIDRInterval::Close, PairTIDRInterval::Clone, //object close and clone
  PairTIDRInterval::Cast,                 //cast function
  PairTIDRInterval::SizeOf,               //sizeof function
  PairTIDRInterval::KindCheck );          //kind checking function

/*
1.8 ~ListPTIDRInt~

*/

TypeConstructor listPTIDRIntTC(
  ListPTIDRInt::BasicType(),      //name
  ListPTIDRInt::Property,         //describing signature
  ListPTIDRInt::Out, ListPTIDRInt::In,   //Out and In functions
  0, 0,                                  //SaveTo and RestoreFrom List functions
  ListPTIDRInt::Create, ListPTIDRInt::Delete,
                                           //obj creation and deletion
  ListPTIDRInt::Open, ListPTIDRInt::Save,// object open and save
  ListPTIDRInt::Close, ListPTIDRInt::Clone,
                                            //object close and clone
  ListPTIDRInt::Cast,               //cast function
  ListPTIDRInt::SizeOf,             //sizeof function
  ListPTIDRInt::KindCheck );        //kind checking function

/*
1.9 ~NetDistanceGroup~

*/

TypeConstructor netDistGroupTC(
  NetDistanceGroup::BasicType(),         //name
  NetDistanceGroup::Property,            //describing signature
  NetDistanceGroup::Out, NetDistanceGroup::In, //Out and In functions
  0, 0,                                //SaveTo and RestoreFrom List functions
  NetDistanceGroup::Create, NetDistanceGroup::Delete,//obj creation and deletion
  NetDistanceGroup::Open, NetDistanceGroup::Save, // object open and save
  NetDistanceGroup::Close, NetDistanceGroup::Clone, //object close and clone
  NetDistanceGroup::Cast,                 //cast function
  NetDistanceGroup::SizeOf,               //sizeof function
  NetDistanceGroup::KindCheck );          //kind checking function

/*
1.10 ~ListNetDistGrp~

*/

TypeConstructor listNDGTC(
  ListNetDistGrp::BasicType(),      //name
  ListNetDistGrp::Property,         //describing signature
  ListNetDistGrp::Out, ListNetDistGrp::In,   //Out and In functions
  0, 0,                                  //SaveTo and RestoreFrom List functions
  ListNetDistGrp::Create, ListNetDistGrp::Delete,
  //obj creation and deletion
  ListNetDistGrp::Open, ListNetDistGrp::Save,// object open and save
  ListNetDistGrp::Close, ListNetDistGrp::Clone,
  //object close and clone
  ListNetDistGrp::Cast,               //cast function
  ListNetDistGrp::SizeOf,             //sizeof function
  ListNetDistGrp::KindCheck );        //kind checking function

/*
1.11 ~JNetwork~

*/

TypeConstructor jNetTC(
  JNetwork::BasicType(),      //name
  JNetwork::Property,         //describing signature
  JNetwork::Out, JNetwork::In,//Out and In functions
  0, 0,                       //SaveTo and RestoreFrom List functions
  JNetwork::Create, JNetwork::Delete, //obj creation and deletion
  JNetwork::Open, JNetwork::Save,// object open and save
  JNetwork::Close, JNetwork::Clone,  //object close and clone
  JNetwork::Cast,               //cast function
  JNetwork::SizeOf,             //sizeof function
  JNetwork::KindCheck );        //kind checking function

/*
1.1 ~JPoint~

*/

TypeConstructor jPointTC(
  JPoint::BasicType(),      //name
  JPoint::Property,         //describing signature
  JPoint::Out, JPoint::In,//Out and In functions
  0, 0,                       //SaveTo and RestoreFrom List functions
  JPoint::Create, JPoint::Delete, //obj creation and deletion
  JPoint::Open, JPoint::Save,// object open and save
  JPoint::Close, JPoint::Clone,  //object close and clone
  JPoint::Cast,               //cast function
  JPoint::SizeOf,             //sizeof function
  JPoint::KindCheck );        //kind checking function

/*
2 Secondo Operators

2.1 ~createpair~

Creates Pairs of ~TupleId~s and ~RouteLocation~s and ~RouteInterval~s.

*/

const string maps_createPair[2][3] =
{
  {TupleIdentifier::BasicType(), RouteLocation::BasicType(),
     PairTIDRLoc::BasicType()},
  {TupleIdentifier::BasicType(), JRouteInterval::BasicType(),
     PairTIDRInterval::BasicType()},
};

ListExpr CreatePairTypeMap ( ListExpr args )
{
  return SimpleMaps<2,3>(maps_createPair, args);
}

int CreatePairSelect (ListExpr args)
{
  return SimpleSelect<2,3>(maps_createPair, args);
}

template<class secParam, class resParam>
int CreatePairValueMap( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  TupleIdentifier* t = (TupleIdentifier*) args[0].addr;
  secParam* r = (secParam*) args[1].addr;
  resParam* pResult = ( resParam* ) qp->ResultStorage ( s ).addr;
  resParam* res = 0;
  if (t->IsDefined() && r->IsDefined())
    res = new resParam(*t,*r);
  else
    res = new resParam(false);
  *pResult = *res;
  res->DeleteIfAllowed();
  result = SetWord(pResult);
  return 1;
}

ValueMapping CreatePairMap [] =
{
  CreatePairValueMap<RouteLocation, PairTIDRLoc>,
  CreatePairValueMap<JRouteInterval, PairTIDRInterval>
};

const string CreatePairSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + TupleIdentifier::BasicType() + " x " + RouteLocation::BasicType()
  + " -> " + PairTIDRLoc::BasicType() + ", " + TupleIdentifier::BasicType() +
  " x " + JRouteInterval::BasicType() + " -> " + PairTIDRInterval::BasicType()
  + "</text--->"
  "<text> createpair(_,_) </text--->"
  "<text> Creates a pair of " + TupleIdentifier::BasicType() + " and " +
  RouteLocation::BasicType() + " or " + JRouteInterval::BasicType() +
  ". Connecting a junction tuple with a route position, respectively, a " +
  "section tuple with a route part.</text--->"
  "<text> query createpair(tid, routelocation)</text--->))";

Operator jnetcreatepair(
  "createpair",
  CreatePairSpec,
  2,
  CreatePairMap,
  CreatePairSelect,
  CreatePairTypeMap
);

/*
2.1 ~createnetdistgroup~

Creates ~NetdistanceGroup~ from 3 ~TupleIds~ and an ~real~ value.
The 3 ~TupleIds~ connect the target Junctions id, the next Junction on the way
to the target and the next section on the way to the target with the distance
to the target.

*/

ListExpr CreateNetDistGroupTypeMap ( ListExpr args )
{
  const string createndgmapping [] = {TupleIdentifier::BasicType(),
        TupleIdentifier::BasicType(),TupleIdentifier::BasicType(),
         CcReal::BasicType(), NetDistanceGroup::BasicType()};
  return SimpleMap(createndgmapping, 5, args);
}

int CreateNetDistGroupValueMap( Word* args, Word& result, int message,
                               Word& local, Supplier s )
{
  TupleIdentifier* target = (TupleIdentifier*) args[0].addr;
  TupleIdentifier* nextSect = (TupleIdentifier*) args[1].addr;
  TupleIdentifier* nextJunct = (TupleIdentifier*) args[2].addr;
  double netdist = ((CcReal*) args[3].addr)->GetRealval();
  NetDistanceGroup* pResult = (NetDistanceGroup*) qp->ResultStorage ( s ).addr;
  NetDistanceGroup* res = 0;
  if (target->IsDefined() && nextSect->IsDefined() && nextJunct->IsDefined())
    res = new NetDistanceGroup(*target,*nextSect, *nextJunct,netdist);
  else
    res = new NetDistanceGroup(false);
  *pResult = *res;
  res->DeleteIfAllowed();
  result = SetWord(pResult);
  return 1;
}

const string CreateNetDistGroupSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text> " + TupleIdentifier::BasicType() + " x " +
  TupleIdentifier::BasicType() + " x " + TupleIdentifier::BasicType() + " x " +
  CcReal::BasicType() + " -> " + NetDistanceGroup::BasicType() + "</text--->"
  "<text> createnedistancegroup( _ , _ , _ , _ ) </text--->"
  "<text> Creates a netdistance group from the three " +
  TupleIdentifier::BasicType() + " of target node, next junction and next "
  "section on path, and the network distance value.</text--->"
  "<text> query createnetdistancegroup(target, nextjunction, nextsection, "
  "network distance)</text--->))";

Operator jnetcreatendg(
  "createnetdistgroup",
  CreateNetDistGroupSpec,
  CreateNetDistGroupValueMap,
  Operator::SimpleSelect,
  CreateNetDistGroupTypeMap
);


/*
2.2 ~createlistj~

Gets a stream of argument objects from type ~X~ and creates a list of the
corresponding listtype ~Y~.

X            | Y
================================
tid          | jlisttid
ptidrloc     | listpairtidrloc
ptidrint     | listpairtidrint
netdistgroup | listndg

*/

ListExpr CreateListTypeMap ( ListExpr args )
{
  NList param(args);
  if (param.length()==1)
  {
    NList stream(param.first());

    NList tupid(TupleIdentifier::BasicType());
    if (stream.checkStream(tupid))
      return nl->SymbolAtom(JListTID::BasicType());

    NList ptidrloc(PairTIDRLoc::BasicType());
    if (stream.checkStream(ptidrloc))
      return nl->SymbolAtom(ListPTIDRLoc::BasicType());

    NList ptidrint(PairTIDRInterval::BasicType());
    if (stream.checkStream(ptidrint))
      return nl->SymbolAtom(ListPTIDRInt::BasicType());

    NList ndg(NetDistanceGroup::BasicType());
    if (stream.checkStream(ndg))
      return nl->SymbolAtom(ListNetDistGrp::BasicType());
  }
  return listutils::typeError("Expected " + Symbol::STREAM() + "(T) with T "
    + "in: "+ TupleIdentifier::BasicType() + ", " + PairTIDRLoc::BasicType() +
    ", " + PairTIDRInterval::BasicType() + ", or " +
    NetDistanceGroup::BasicType());
}

template <class cList, class cElement>
int CreateListValueMap( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  cList* pResult = (cList*) ((qp->ResultStorage(s)).addr);
  pResult->SetDefined(true);
  cElement* t = 0;
  Word curAddr;
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, curAddr);
  while (qp->Received(args[0].addr))
  {
    t = (cElement*) curAddr.addr;
    pResult->Append(*t);
    t->DeleteIfAllowed();
    qp->Request(args[0].addr, curAddr);
  }
  qp->Close(args[0].addr);
  result.setAddr(pResult);
  return 0;
}

ValueMapping CreateListMap [] =
{
  CreateListValueMap<JListTID, TupleIdentifier>,
  CreateListValueMap<ListPTIDRLoc, PairTIDRLoc>,
  CreateListValueMap<ListPTIDRInt, PairTIDRInterval>,
  CreateListValueMap<ListNetDistGrp, NetDistanceGroup>
};

int CreateListSelect ( ListExpr args )
{
  NList param(args);
  if (param.first().second() == TupleIdentifier::BasicType()) return 0;
  if (param.first().second() == PairTIDRLoc::BasicType()) return 1;
  if (param.first().second() == PairTIDRInterval::BasicType()) return 2;
  if (param.first().second() == NetDistanceGroup::BasicType()) return 3;
  return -1; // this point should never been reached.
}

const string CreateListSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text> " + Symbol::STREAM() + "(T) -> ListT</text--->"
  "<text> createlistj(_) </text--->"
  "<text> Collects streams of the datatypes " + TupleIdentifier::BasicType() +
  ", " + PairTIDRLoc::BasicType() + ", " + PairTIDRInterval::BasicType() +
  ", and " + NetDistanceGroup::BasicType() +" of JNetAlgebra into single "
  "list objects of the datatypes as there are: "+ JListTID::BasicType() + ", " +
  ListPTIDRLoc::BasicType() + ", " + ListPTIDRInt::BasicType() + ", and " +
  ListNetDistGrp::BasicType() + ".</text--->"
  "<text> query createsteramj(tupleidlist) createlistj</text--->))";

Operator jnetcreatelistj(
  "createlistj",
  CreateListSpec,
  4,
  CreateListMap,
  CreateListSelect,
  CreateListTypeMap
);

/*
2.3 createstreamj

Creates a stream of objects from type ~X~ from a corresponding listtype ~Y~.

X            | Y
================================
tid          | jlisttid
ptidrloc     | listpairtidrloc
ptidrint     | listpairtidrint
netdistgroup | listndg

Creation of Object Streams from Lists

*/

ListExpr CreateStreamTypeMap ( ListExpr args )
{
  NList param(args);
  if (param.length()==1)
  {
    if (param.first().isEqual(JListTID::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(TupleIdentifier::BasicType()));

    if (param.first().isEqual(ListPTIDRLoc::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(PairTIDRLoc::BasicType()));

    if (param.first().isEqual(ListPTIDRInt::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(PairTIDRInterval::BasicType()));

    if (param.first().isEqual(ListNetDistGrp::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(NetDistanceGroup::BasicType()));
  }
  return listutils::typeError("Expected " + JListTID::BasicType() + ", " +
    ListPTIDRLoc::BasicType() + ", " + ListPTIDRInt::BasicType() + ", " +
    ListNetDistGrp::BasicType());
}

template<class cElement>
struct locInfoCreateStream {
  locInfoCreateStream():list(0)
  {
    it = 0;
  }

  DbArray<cElement> list;
  int it;
};

template <class cElement, class cList>
int CreateStreamValueMap( Word* args, Word& result, int message,
                          Word& local, Supplier s )
{
  locInfoCreateStream<cElement>* li = 0;
  switch(message)
  {
    case OPEN:
    {
      li = new locInfoCreateStream<cElement>();
      li->list = ((cList*) args[0].addr)->GetList();
      li->it = 0;
      local.addr = li;
      return 0;
      break;
    }

    case REQUEST:
    {
      result = qp->ResultStorage(s);
      if (local.addr == 0) return CANCEL;
      li = (locInfoCreateStream<cElement>*) local.addr;
      if (0 <= li->it && li->it < li->list.Size())
      {
        cElement elem;
        li->list.Get(li->it,elem);
        li->it = li->it + 1;
        result = SetWord(new cElement(elem));
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
        li = (locInfoCreateStream<cElement>*) local.addr;
        delete li;
      }
      li = 0;
      local.addr = 0;
      return 0;
      break;
    }

    default:
    {
      return CANCEL; // Should never happen
      break;
    }
  }
}

ValueMapping CreateStreamMap [] =
{
  CreateStreamValueMap<TupleIdentifier, JListTID>,
  CreateStreamValueMap<PairTIDRLoc, ListPTIDRLoc>,
  CreateStreamValueMap<PairTIDRInterval, ListPTIDRInt>,
  CreateStreamValueMap<NetDistanceGroup, ListNetDistGrp>
};

int CreateStreamSelect ( ListExpr args )
{
  NList param(args);
  if (param.first() == JListTID::BasicType()) return 0;
  if (param.first() == ListPTIDRLoc::BasicType()) return 1;
  if (param.first() == ListPTIDRInt::BasicType()) return 2;
  if (param.first() == ListNetDistGrp::BasicType()) return 3;
  return -1; // this point should never been reached.
}

const string CreateStreamJSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text> ListT -> " + Symbol::STREAM() + "(T)</text--->"
  "<text> createstreamj(_) </text--->"
  "<text> Expands the listdatatypes" + JListTID::BasicType() + ", " +
  ListPTIDRLoc::BasicType() + ", " + ListPTIDRInt::BasicType() + ", and " +
  ListNetDistGrp::BasicType() + " of JNetAlgebra into a stream of the "
  "corresponding datatypes "+ TupleIdentifier::BasicType() + ", " +
  PairTIDRLoc::BasicType() + ", " + PairTIDRInterval::BasicType() + ", and " +
  NetDistanceGroup::BasicType() + "</text--->"
  "<text> query createsteramj(tupleidlist) createlistj</text--->))";

Operator jnetcreatestreamj(
  "createstreamj",
  CreateStreamJSpec,
  4,
  CreateStreamMap,
  CreateStreamSelect,
  CreateStreamTypeMap
);


/*
2.4 Creation of JNetwork Object

The operator expects an string value and two relations.

The string value defines the name of the network object in the database.

The first relation defines the nodes of the network by four values of type
~int~, ~point~, ~int~, ~real~, whereas the meaning is JUNC\_ID, JUNC\_POS,
the id of the road the junctions belongs to and the position of the junction on
that road, it should be sorted by the jid and rid.

The second relation defines the roads of the network by six value of type
~int~, ~int~, ~point~, ~real~, ~real~, ~sline~,whereas the meaning is ROUTE\_ID,
junction id, position of junction on that road, the maximum allowed speed on
the road and the route curve.

*/

ListExpr CreateJNetworkTM ( ListExpr args )
{
  NList param(args);
  if (param.length() != 3)
    return listutils::typeError("Expected 3 arguments.");

  NList netId(param.first());
  if (!netId.isSymbol(CcString::BasicType()))
    return listutils::typeError("1.Argument must be" + CcString::BasicType());

  NList juncRel(param.second());
  NList juncAttrs;
  if (!juncRel.checkRel(juncAttrs))
    return listutils::typeError("2.Argument must be junction relation.");

  if (juncAttrs.length()!= 4)
    return listutils::typeError("juncrel must have 4 attributes.");

  //junction id
  if (!juncAttrs.first().second().isSymbol(CcInt::BasicType()))
    return listutils::typeError("1.attr. of 1.rel must be " +
        CcInt::BasicType());

  //spatial position
  if (!juncAttrs.second().second().isSymbol(Point::BasicType()))
    return listutils::typeError("2.attr. of 1.rel must be " +
        Point::BasicType());

  //route id
  if (!juncAttrs.third().second().isSymbol(CcInt::BasicType()))
    return listutils::typeError("3.attr. of 1.rel must be " +
        CcInt::BasicType());

  //position on route
  if (!juncAttrs.fourth().second().isSymbol(CcReal::BasicType()))
    return listutils::typeError("4.attr. of 1.rel must be " +
        CcReal::BasicType());

  NList routeRel(param.third());
  NList routeAttrs;

  if (!routeRel.checkRel(routeAttrs))
    return listutils::typeError("3.Argument must be routes relation.");

  if (routeAttrs.length() != 6)
    return listutils::typeError("routes rel must have 6 attributes");

  //routes id
  if (!routeAttrs.first().second().isSymbol(CcInt::BasicType()))
    return listutils::typeError("1.attr. of 2.rel must be " +
        CcInt::BasicType());

  //junction id
  if (!routeAttrs.second().second().isSymbol(CcInt::BasicType()))
    return listutils::typeError("2.attr. of 2.rel must be " +
      CcInt::BasicType());

  //position of junction on route
  if (!routeAttrs.third().second().isSymbol(CcReal::BasicType()))
    return listutils::typeError("3.attr. of 2.rel must be " +
      CcReal::BasicType());

  //maxspeed
  if (!routeAttrs.fourth().second().isSymbol(CcReal::BasicType()))
    return listutils::typeError("4.attr. of 2.rel must be " +
      CcReal::BasicType());

  //curve
  if (!routeAttrs.fifth().second().isSymbol(SimpleLine::BasicType()))
    return listutils::typeError("5.attr. of 2.rel must be " +
      SimpleLine::BasicType());

  //direction
  if (!routeAttrs.sixth().second().isSymbol(Direction::BasicType()))
    return listutils::typeError("6.attr. of 2.rel must be " +
      Direction::BasicType());

  return nl->SymbolAtom(JNetwork::BasicType());
}

int CreateJNetworkVM( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  string nid = ((CcString*)args[0].addr)->GetValue();
  Relation* juncRel = (Relation*) args[1].addr;
  Relation* routesRel = (Relation*) args[2].addr;
  JNetwork* pResult = (JNetwork*) qp->ResultStorage ( s ).addr;
  result = SetWord(pResult);
  pResult->CreateNetwork(nid, juncRel, routesRel);
  return 0;
}

const string CreateJNetworkSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + CcString::BasicType() + " x " + Relation::BasicType() +
  "(" + Tuple::BasicType() + "(" + CcInt::BasicType() + " " +
  Point::BasicType() + " " + CcInt::BasicType() + " " + CcReal::BasicType() +
  ")) x " + Relation::BasicType() + "(" + Tuple::BasicType() + "(" +
  CcInt::BasicType() + " " + CcInt::BasicType() + " " + CcReal::BasicType() +
  " " + CcReal::BasicType() + " " + SimpleLine::BasicType() + ")) -> " +
  JNetwork::BasicType() + "</text--->"
  "<text> createjnetwork(name, junctionsRelation, routesRelation) </text--->"
  "<text> Creates the jnetwork object with name. From the two input relations"
  "The attributes of the junctions relation have the meaning: "
  "junction id, spatial position, route id, and position on route. "
  "The attributes of the routes relation are: route identifier, junction id,"
  "position of junction on that road, the maximum allowed speed, and the "
  "route curve.</text--->"
  "<text> let name = createjnetwork('name', junctionsRelation, "
  "routesRelation)</text--->))";

Operator jnetcreatenetwork(
    "createjnetwork",
    CreateJNetworkSpec,
    CreateJNetworkVM,
    Operator::SimpleSelect,
    CreateJNetworkTM
  );

/*
2.5 Access to JNetwork parts

2.5.1 Routes

Returns the routes relation of the jnetwork object.

*/

ListExpr routesTM ( ListExpr args )
{
  NList param(args);
  if (param.length() != 1)
    return listutils::typeError("Expected 1 argument.");

  NList network(param.first());
  if (!network.isSymbol(JNetwork::BasicType()))
    return listutils::typeError("Argument must be " + JNetwork::BasicType());

  ListExpr xType;
  nl->ReadFromString ( JNetwork::GetRoutesTypeInfo(), xType );
  return xType;
}

int routesVM( Word* args, Word& result, int message,
              Word& local, Supplier s )
{
  JNetwork* network = (JNetwork*)args[0].addr;
  result = SetWord ( network->GetRoutesCopy() );
  Relation *resultSt = ( Relation* ) qp->ResultStorage ( s ).addr;
  resultSt->Close();
  qp->ChangeResultStorage ( s, result );
  return 0;
}

const string RoutesSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + JNetwork::BasicType() + " -> " + JNetwork::GetRoutesTypeInfo() +
   "</text--->"
   "<text> routes(_) </text--->"
   "<text> Returns the routes relation of the jnetwork object.</text--->"
   "<text> query routes(jnetworkobject)</text--->))";

Operator jnetroutes(
  "routes",
  RoutesSpec,
  routesVM,
  Operator::SimpleSelect,
  routesTM
);

/*
2.5.2 Junctions

*/

ListExpr junctionsTM ( ListExpr args )
{
  NList param(args);
  if (param.length() != 1)
    return listutils::typeError("Expected 1 argument.");

  NList network(param.first());
  if (!network.isSymbol(JNetwork::BasicType()))
    return listutils::typeError("Argument must be "+ JNetwork::BasicType());

  ListExpr xType;
  nl->ReadFromString ( JNetwork::GetJunctionsTypeInfo(), xType );
  return xType;
}

int junctionsVM( Word* args, Word& result, int message,
                 Word& local, Supplier s )
{
  JNetwork* network = (JNetwork*)args[0].addr;
  result = SetWord ( network->GetJunctionsCopy() );
  Relation *resultSt = ( Relation* ) qp->ResultStorage ( s ).addr;
  resultSt->Close();
  qp->ChangeResultStorage ( s, result );
  return 0;
}

const string JunctionsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + JNetwork::BasicType() + " -> " +
  JNetwork::GetJunctionsTypeInfo()+ "</text--->"
  "<text> junctions(_) </text--->"
  "<text> Returns the junctions relation of the jnetwork object.</text--->"
  "<text> query junctions(jnetworkobject)</text--->))";

  Operator jnetjunctions(
    "junctions",
    JunctionsSpec,
    junctionsVM,
    Operator::SimpleSelect,
    junctionsTM
  );

/*
2.5.3 Sections

*/

ListExpr sectionsTM ( ListExpr args )
{
  NList param(args);
  if (param.length() != 1)
    return listutils::typeError("Expected 1 argument.");

  NList network(param.first());
  if (!network.isSymbol(JNetwork::BasicType()))
    return listutils::typeError("Argument must be "+ JNetwork::BasicType());

  ListExpr xType;
  nl->ReadFromString ( JNetwork::GetSectionsTypeInfo(), xType );
  return xType;
}

int sectionsVM( Word* args, Word& result, int message,
              Word& local, Supplier s )
{
  JNetwork* network = (JNetwork*)args[0].addr;
  result = SetWord ( network->GetSectionsCopy() );
  Relation *resultSt = ( Relation* ) qp->ResultStorage ( s ).addr;
  resultSt->Close();
  qp->ChangeResultStorage ( s, result );
  return 0;
}

const string SectionsSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + JNetwork::BasicType() + " -> " + JNetwork::GetSectionsTypeInfo() +
  "</text--->"
  "<text> sections(_) </text--->"
  "<text> Returns the sections relation of the jnetwork object.</text--->"
  "<text> query sections(jnetworkobject)</text--->))";

Operator jnetsections(
  "sections",
  SectionsSpec,
  sectionsVM,
  Operator::SimpleSelect,
  sectionsTM
);

/*
3. ~class JNetAlgebra~

3.1 Constructor

*/

JNetAlgebra::JNetAlgebra():Algebra()
{

/*
3.1.1 Integration of Data Types by Type Constructors

*/
  AddTypeConstructor(&directionTC);
  directionTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&routeLocationTC);
  routeLocationTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&routeIntervalTC);
  routeIntervalTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&jListTIDTC);
  jListTIDTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&pTIDRLocTC);
  pTIDRLocTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&listPTIDRLocTC);
  listPTIDRLocTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&pTIDRIntTC);
  pTIDRIntTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&listPTIDRIntTC);
  listPTIDRIntTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&netDistGroupTC);
  netDistGroupTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&listNDGTC);
  listNDGTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&jNetTC);
  jNetTC.AssociateKind(Kind::JNETWORK());

  AddTypeConstructor(&jPointTC);
  jPointTC.AssociateKind(Kind::DATA());

/*
3.1.2 Integration of Operators

3.1.2.1 Creation of not simple Datatypes

*/

  AddOperator(&jnetcreatepair);
  AddOperator(&jnetcreatendg);
  AddOperator(&jnetcreatelistj);
  AddOperator(&jnetcreatestreamj);
  AddOperator(&jnetcreatenetwork);

/*
3.1.2.2 Access to Network Parameters

*/
  AddOperator(&jnetroutes);
  AddOperator(&jnetjunctions);
  AddOperator(&jnetsections);
}

/*
3.2 Deconstructor

*/

JNetAlgebra::~JNetAlgebra()
{}

/*
4. Initialization

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