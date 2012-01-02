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
#include "PairTID.h"
#include "JList.h"
#include "NetDistanceGroup.h"
#include "JNetwork.h"
#include "JPoint.h"
#include "JLine.h"

using namespace std;
using namespace mappings;


extern NestedList* nl;
extern QueryProcessor* qp;

/*
1 Type Constructors

1.1 ~Direction~

*/

TypeConstructor directionTC(
  Direction::BasicType(),
  Direction::Property,
  Direction::Out, Direction::In,
  0, 0,
  Direction::Create, Direction::Delete,
  Direction::Open, Direction::Save,
  Direction::Close, Direction::Clone,
  Direction::Cast,
  Direction::SizeOf,
  Direction::KindCheck );


/*
1.2 ~RouteLocation~

*/

TypeConstructor routeLocationTC(
  RouteLocation::BasicType(),
  RouteLocation::Property,
  RouteLocation::Out, RouteLocation::In,
  0, 0,
  RouteLocation::Create, RouteLocation::Delete,
  RouteLocation::Open, RouteLocation::Save,
  RouteLocation::Close, RouteLocation::Clone,
  RouteLocation::Cast,
  RouteLocation::SizeOf,
  RouteLocation::KindCheck );

/*
1.3 ~JRouteInterval~

*/

TypeConstructor routeIntervalTC(
  JRouteInterval::BasicType(),
  JRouteInterval::Property,
  JRouteInterval::Out, JRouteInterval::In,
  0, 0,
  JRouteInterval::Create, JRouteInterval::Delete,
  JRouteInterval::Open, JRouteInterval::Save,
  JRouteInterval::Close, JRouteInterval::Clone,
  JRouteInterval::Cast,
  JRouteInterval::SizeOf,
  JRouteInterval::KindCheck );

/*
1.4 ~JListTID~

*/

TypeConstructor jListTIDTC(
  JListTID::BasicType(),
  JListTID::Property,
  JListTID::Out, JListTID::In,
  0, 0,
  JListTID::Create, JListTID::Delete,
  JListTID::Open, JListTID::Save,
  JListTID::Close, JListTID::Clone,
  JListTID::Cast,
  JListTID::SizeOf,
  JListTID::KindCheck );

/*
1.5 ~PairTIDRLoc~

*/

TypeConstructor pTIDRLocTC(
  PairTIDRLoc::BasicType(),
  PairTIDRLoc::Property,
  PairTIDRLoc::Out, PairTIDRLoc::In,
  0, 0,
  PairTIDRLoc::Create, PairTIDRLoc::Delete,
  PairTIDRLoc::Open, PairTIDRLoc::Save,
  PairTIDRLoc::Close, PairTIDRLoc::Clone,
  PairTIDRLoc::Cast,
  PairTIDRLoc::SizeOf,
  PairTIDRLoc::KindCheck );

/*
1.6 ~ListPairTIDRLoc~

*/

TypeConstructor listPTIDRLocTC(
  ListPairTIDRLoc::BasicType(),
  ListPairTIDRLoc::Property,
  ListPairTIDRLoc::Out, ListPairTIDRLoc::In,
  0, 0,
  ListPairTIDRLoc::Create, ListPairTIDRLoc::Delete,
  ListPairTIDRLoc::Open, ListPairTIDRLoc::Save,
  ListPairTIDRLoc::Close, ListPairTIDRLoc::Clone,
  ListPairTIDRLoc::Cast,
  ListPairTIDRLoc::SizeOf,
  ListPairTIDRLoc::KindCheck );

/*
1.7 ~PairTIDRInt~

*/

TypeConstructor pTIDRIntTC(
  PairTIDRInt::BasicType(),
  PairTIDRInt::Property,
  PairTIDRInt::Out, PairTIDRInt::In,
  0, 0,
  PairTIDRInt::Create, PairTIDRInt::Delete,
  PairTIDRInt::Open, PairTIDRInt::Save,
  PairTIDRInt::Close, PairTIDRInt::Clone,
  PairTIDRInt::Cast,
  PairTIDRInt::SizeOf,
  PairTIDRInt::KindCheck );

/*
1.8 ~ListPairTIDRInt~

*/

TypeConstructor listPTIDRIntTC(
  ListPairTIDRInt::BasicType(),
  ListPairTIDRInt::Property,
  ListPairTIDRInt::Out, ListPairTIDRInt::In,
  0, 0,
  ListPairTIDRInt::Create, ListPairTIDRInt::Delete,

  ListPairTIDRInt::Open, ListPairTIDRInt::Save,
  ListPairTIDRInt::Close, ListPairTIDRInt::Clone,

  ListPairTIDRInt::Cast,
  ListPairTIDRInt::SizeOf,
  ListPairTIDRInt::KindCheck );

/*
1.9 ~NetDistanceGroup~

*/

TypeConstructor netDistGroupTC(
  NetDistanceGroup::BasicType(),
  NetDistanceGroup::Property,
  NetDistanceGroup::Out, NetDistanceGroup::In,
  0, 0,
  NetDistanceGroup::Create, NetDistanceGroup::Delete,
  NetDistanceGroup::Open, NetDistanceGroup::Save,
  NetDistanceGroup::Close, NetDistanceGroup::Clone,
  NetDistanceGroup::Cast,
  NetDistanceGroup::SizeOf,
  NetDistanceGroup::KindCheck );

/*
1.10 ~ListNetDistGrp~

*/

TypeConstructor listNDGTC(
  ListNetDistGrp::BasicType(),
  ListNetDistGrp::Property,
  ListNetDistGrp::Out, ListNetDistGrp::In,
  0, 0,
  ListNetDistGrp::Create, ListNetDistGrp::Delete,
  ListNetDistGrp::Open, ListNetDistGrp::Save,
  ListNetDistGrp::Close, ListNetDistGrp::Clone,
  ListNetDistGrp::Cast,
  ListNetDistGrp::SizeOf,
  ListNetDistGrp::KindCheck );

/*
1.11 ~JNetwork~

*/

TypeConstructor jNetTC(
  JNetwork::BasicType(),
  JNetwork::Property,
  JNetwork::Out, JNetwork::In,
  0, 0,
  JNetwork::Create, JNetwork::Delete,
  JNetwork::Open, JNetwork::Save,
  JNetwork::Close, JNetwork::Clone,
  JNetwork::Cast,
  JNetwork::SizeOf,
  JNetwork::KindCheck );

/*
1.1 ~JPoint~

*/

TypeConstructor jPointTC(
  JPoint::BasicType(),
  JPoint::Property,
  JPoint::Out, JPoint::In,
  0, 0,
  JPoint::Create, JPoint::Delete,
  JPoint::Open, JPoint::Save,
  JPoint::Close, JPoint::Clone,
  JPoint::Cast,
  JPoint::SizeOf,
  JPoint::KindCheck );

/*
1.1 ~JLine~

*/

TypeConstructor jLineTC(
  JLine::BasicType(),
  JLine::Property,
  JLine::Out, JLine::In,
  0, 0,
  JLine::Create, JLine::Delete,
  OpenAttribute<JLine>, SaveAttribute<JLine>,
  JLine::Close, JLine::Clone,
  JLine::Cast,
  JLine::SizeOf,
  JLine::KindCheck );

/*
1.1 ~Listint~

*/

TypeConstructor jListIntTC(
  JListInt::BasicType(),
  JListInt::Property,
  JListInt::Out, JListInt::In,
  0, 0,
  JListInt::Create, JListInt::Delete,
  JListInt::Open, JListInt::Save,
  JListInt::Close, JListInt::Clone,
  JListInt::Cast,
  JListInt::SizeOf,
  JListInt::KindCheck );


/*
2 Secondo Operators

2.1 ~createnetdistgroup~

Creates ~NetdistanceGroup~ from 3 ~TupleIds~ and an ~real~ value.
The 3 ~TupleIds~ connect the target Junctions id, the next Junction on the way
to the target and the next section on the way to the target with the distance
to the target.

*/

const string maps_createNetDistGroup[1][5] =
{
  {TupleIdentifier::BasicType(), TupleIdentifier::BasicType(),
     TupleIdentifier::BasicType(), CcReal::BasicType(),
        NetDistanceGroup::BasicType()}
};

ListExpr CreateNetDistGroupTypeMap ( ListExpr args )
{
  return SimpleMaps<1,5>(maps_createNetDistGroup, args);
}

int CreateNetDistGroupSelect (ListExpr args)
{
  return SimpleSelect<1,5>(maps_createNetDistGroup, args);
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

ValueMapping CreateNetDistGroupMap[] =
{
  CreateNetDistGroupValueMap,
};

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
    1,
    CreateNetDistGroupMap,
    CreateNetDistGroupSelect,
    CreateNetDistGroupTypeMap
);

/*
2.1 ~createroutelocation~

Creates ~RouteLocation~ from an 3 tuple of ~int~, ~real~ and ~jdirection~.
With ~int~ = road id, ~real~ = dist from start of road, ~jdirection~ = side
of road.

*/

const string maps_createRouteLocation[1][4] =
{
  {CcInt::BasicType(), CcReal::BasicType(), Direction::BasicType(),
    RouteLocation::BasicType()}
};

ListExpr CreateRouteLocationTypeMap ( ListExpr args )
{
  return SimpleMaps<1,4>(maps_createRouteLocation, args);
}

int CreateRouteLocationSelect (ListExpr args)
{
  return SimpleSelect<1,4>(maps_createRouteLocation, args);
}

int CreateRouteLocationValueMap( Word* args, Word& result, int message,
                                Word& local, Supplier s )
{
  CcInt* rid = (CcInt*) args[0].addr;
  CcReal* pos = (CcReal*) args[1].addr;
  Direction* side = (Direction*) args[2].addr;
  RouteLocation* pResult = (RouteLocation*) qp->ResultStorage ( s ).addr;
  RouteLocation* res = 0;
  if (rid->IsDefined() && pos->IsDefined() && side->IsDefined())
    res = new RouteLocation(rid->GetIntval(),pos->GetRealval(),
                            side->GetDirection());
  else
    res = new RouteLocation(false);
  *pResult = *res;
  res->DeleteIfAllowed();
  result = SetWord(pResult);
  return 1;
}

ValueMapping CreateRouteLocationMap[] =
{
  CreateRouteLocationValueMap,
};

const string CreateRouteLocationSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text> " + CcInt::BasicType() + " x " + CcReal::BasicType() + " x " +
  Direction::BasicType() + " -> " + RouteLocation::BasicType() + "</text--->"
  "<text> createroutelocation( _ , _ , _ ) </text--->"
  "<text> Creates a route location from the route id, the position on the "
  "route and the side value.</text--->"
  "<text> query createroutelocation(rid, pos, side)</text--->))";

Operator jnetcreaterloc(
    "createroutelocation",
    CreateRouteLocationSpec,
    1,
    CreateRouteLocationMap,
    CreateRouteLocationSelect,
    CreateRouteLocationTypeMap
);

/*
2.1 ~createrouteinterval~

Creates ~JRouteInterval~ from an 4 tuple of ~int~, ~real~, ~real~ and
~jdirection~. With ~int~ = road id, ~real~ = startpos, ~real~ = endpos
(startpos <= endpos) and ~jdirection~ = side of road.

*/

const string maps_createRouteInterval[1][5] =
{
  {CcInt::BasicType(), CcReal::BasicType(), CcReal::BasicType(),
   Direction::BasicType(), JRouteInterval::BasicType()}
};

ListExpr CreateRouteIntervalTypeMap ( ListExpr args )
{
  return SimpleMaps<1,5>(maps_createRouteInterval, args);
}

int CreateRouteIntervalSelect (ListExpr args)
{
  return SimpleSelect<1,5>(maps_createRouteInterval, args);
}

int CreateRouteIntervalValueMap( Word* args, Word& result, int message,
                                 Word& local, Supplier s )
{
  CcInt* rid = (CcInt*) args[0].addr;
  CcReal* spos = (CcReal*) args[1].addr;
  CcReal* epos = (CcReal*) args[2].addr;
  Direction* side = (Direction*) args[3].addr;
  JRouteInterval* pResult = (JRouteInterval*) qp->ResultStorage ( s ).addr;
  JRouteInterval* res = 0;
  if (rid->IsDefined() && spos->IsDefined() && epos->IsDefined() &&
      side->IsDefined())
    res = new JRouteInterval(rid->GetIntval(), spos->GetRealval(),
                             epos->GetRealval(), side->GetDirection());
  else
    res = new JRouteInterval(false);
  *pResult = *res;
  res->DeleteIfAllowed();
  result = SetWord(pResult);
  return 1;
}

ValueMapping CreateRouteIntervalMap[] =
{
  CreateRouteIntervalValueMap,
};

const string CreateRouteIntervalSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text> " + CcInt::BasicType() + " x " + CcReal::BasicType() + " x " +
  CcReal::BasicType() + " x " + Direction::BasicType() + " -> " +
  JRouteInterval::BasicType() + "</text--->"
  "<text> createrouteinterval( _ , _ , _ , _ ) </text--->"
  "<text> Creates a route interval from the route id, the start and end "
  "position on the route and the side value.</text--->"
  "<text> query createrouteinterval(rid, startpos, endpos, side)</text--->))";

Operator jnetcreaterint(
    "createrouteinterval",
    CreateRouteIntervalSpec,
    1,
    CreateRouteIntervalMap,
    CreateRouteIntervalSelect,
    CreateRouteIntervalTypeMap
);

/*
2.1 ~createpair~

Creates Pairs of ~TupleId~s and ~RouteLocation~s and ~RouteInterval~s.

*/

const string maps_createPair[2][3] =
{
  {TupleIdentifier::BasicType(),  RouteLocation::BasicType(),
    PairTIDRLoc::BasicType()},
  {TupleIdentifier::BasicType(),  JRouteInterval::BasicType(),
    PairTIDRInt::BasicType()},
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

ValueMapping CreatePairMap[] =
{
  CreatePairValueMap<RouteLocation, PairTIDRLoc>,
  CreatePairValueMap<JRouteInterval, PairTIDRInt>
};

const string CreatePairSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + TupleIdentifier::BasicType() + " x " + RouteLocation::BasicType()
  + " -> " + PairTIDRLoc::BasicType() + ", " + TupleIdentifier::BasicType() +
  " x " + JRouteInterval::BasicType() + " -> " + PairTIDRInt::BasicType()
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
2.2 ~createlistj~

Gets a stream of argument objects from type ~X~ and creates a list of the
corresponding listtype ~Y~.

X            | Y
================================
tid          | jlisttid
ptidrloc     | listpairtidrloc
ptidrint     | listpairtidrint
netdistgroup | listndg
int          | listint

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
      return nl->SymbolAtom(ListPairTIDRLoc::BasicType());

    NList ptidrint(PairTIDRInt::BasicType());
    if (stream.checkStream(ptidrint))
      return nl->SymbolAtom(ListPairTIDRInt::BasicType());

    NList ndg(NetDistanceGroup::BasicType());
    if (stream.checkStream(ndg))
      return nl->SymbolAtom(ListNetDistGrp::BasicType());

    NList ccInt(CcInt::BasicType());
    if (stream.checkStream(ccInt))
        return nl->SymbolAtom(JListInt::BasicType());
  }
  return listutils::typeError("Expected " + Symbol::STREAM() + "(T) with T "
    + "in: "+ TupleIdentifier::BasicType() + ", " + PairTIDRLoc::BasicType() +
    ", " + PairTIDRInt::BasicType() + ", " + NetDistanceGroup::BasicType() +
    ", or " + CcInt::BasicType());
}

template <class cList, class cElement>
int CreateListValueMap( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  cList* createRes = static_cast<cList*> (result.addr);
  createRes->Clear();
  createRes->StartBulkload();
  cElement* t = 0;
  Word curAddr;
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, curAddr);
  while (qp->Received(args[0].addr))
  {
    t = (cElement*) curAddr.addr;
    createRes->operator+=(*t);
    t->DeleteIfAllowed();
    qp->Request(args[0].addr, curAddr);
  }
  qp->Close(args[0].addr);
  createRes->EndBulkload();
  return 0;
}

ValueMapping CreateListMap [] =
{
  CreateListValueMap<JListTID, TupleIdentifier>,
  CreateListValueMap<ListPairTIDRLoc, PairTIDRLoc>,
  CreateListValueMap<ListPairTIDRInt, PairTIDRInt>,
  CreateListValueMap<ListNetDistGrp, NetDistanceGroup>,
  CreateListValueMap<JListInt, CcInt>
};

int CreateListSelect ( ListExpr args )
{
  NList param(args);
  if (param.first().second() == TupleIdentifier::BasicType()) return 0;
  if (param.first().second() == PairTIDRLoc::BasicType()) return 1;
  if (param.first().second() == PairTIDRInt::BasicType()) return 2;
  if (param.first().second() == NetDistanceGroup::BasicType()) return 3;
  if (param.first().second() == CcInt::BasicType()) return 4;
  return -1; // this point should never been reached.
}

const string CreateListSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text> " + Symbol::STREAM() + "(" + TupleIdentifier::BasicType()  +
  ") -> " + JListTID::BasicType() +", "+
  Symbol::STREAM() + "(" + PairTIDRLoc::BasicType()  +
  ") -> " + ListPairTIDRLoc::BasicType() +", "+
  Symbol::STREAM() + "(" + PairTIDRInt::BasicType()  +
  ") -> " + ListPairTIDRInt::BasicType()+ ", "+
  Symbol::STREAM() + "(" + NetDistanceGroup::BasicType()  +
  ") -> " + ListNetDistGrp::BasicType() + ", " +
  Symbol::STREAM() + "(" + CcInt::BasicType()  +
  ") -> " + JListInt::BasicType()+"</text--->"
  "<text> _ createlistj </text--->"
  "<text> Collects a stream of the data type T of JNetAlgebra into a single "
  "ListT  of the data type.</text--->"
  "<text> query createsteramj(tupleidlist) createlistj</text--->))";

Operator jnetcreatelistj(
  "createlistj",
  CreateListSpec,
  5,
  CreateListMap,
  CreateListSelect,
  CreateListTypeMap
);

/*
2.2 ~collectlistj~

Gets a stream of argument objects from type ~ListX~ and collects them into
a single ~ListX~ object.
X
================
jlisttid
listpairtidrloc
listpairtidrint
listndg
listint

*/

ListExpr CollectListTypeMap ( ListExpr args )
{
  NList param(args);
  if (param.length()==1)
  {
    NList stream(param.first());

    NList ltupid(JListTID::BasicType());
    if (stream.checkStream(ltupid))
      return nl->SymbolAtom(JListTID::BasicType());

    NList lptidrloc(ListPairTIDRLoc::BasicType());
    if (stream.checkStream(lptidrloc))
      return nl->SymbolAtom(ListPairTIDRLoc::BasicType());

    NList lptidrint(ListPairTIDRInt::BasicType());
    if (stream.checkStream(lptidrint))
      return nl->SymbolAtom(ListPairTIDRInt::BasicType());

    NList lndg(ListNetDistGrp::BasicType());
    if (stream.checkStream(lndg))
      return nl->SymbolAtom(ListNetDistGrp::BasicType());

    NList lccInt(JListInt::BasicType());
    if (stream.checkStream(lccInt))
      return nl->SymbolAtom(JListInt::BasicType());
  }
    return listutils::typeError("Expected " + Symbol::STREAM() + "(T) with T "
    + "in: "+ JListTID::BasicType() + ", " + ListPairTIDRLoc::BasicType() +
    ", " + ListPairTIDRInt::BasicType() + ", " + ListNetDistGrp::BasicType() +
    ", or " + JListInt::BasicType());
}

template <class cList>
int CollectListValueMap( Word* args, Word& result, int message,
                        Word& local, Supplier s )
{
  result = qp->ResultStorage(s);
  cList* colRes = static_cast<cList*> (result.addr);
  cList* t = 0;
  colRes->Clear();
  colRes->StartBulkload();
  Word curAddr;
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, curAddr);
  while (qp->Received(args[0].addr))
  {
    t = (cList*) curAddr.addr;
    colRes->operator+=(*t);
    t->DeleteIfAllowed();
    t=0;
    qp->Request(args[0].addr, curAddr);
  }
  qp->Close(args[0].addr);
  colRes->EndBulkload();
  return 0;
}

ValueMapping CollectListMap [] =
{
  CollectListValueMap<JListTID>,
  CollectListValueMap<ListPairTIDRLoc>,
  CollectListValueMap<ListPairTIDRInt>,
  CollectListValueMap<ListNetDistGrp>,
  CollectListValueMap<JListInt>
};

int CollectListSelect ( ListExpr args )
{
  NList param(args);
  if (param.first().second() == JListTID::BasicType()) return 0;
  if (param.first().second() == ListPairTIDRLoc::BasicType()) return 1;
  if (param.first().second() == ListPairTIDRInt::BasicType()) return 2;
  if (param.first().second() == ListNetDistGrp::BasicType()) return 3;
  if (param.first().second() == JListInt::BasicType()) return 4;
  return -1; // this point should never been reached.
}

const string CollectListSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text> " + Symbol::STREAM() + "(" + JListTID::BasicType()  +
  ") -> " + JListTID::BasicType() +", "+
  Symbol::STREAM() + "(" + ListPairTIDRLoc::BasicType()  +
  ") -> " + ListPairTIDRLoc::BasicType() +", "+
  Symbol::STREAM() + "(" + ListPairTIDRInt::BasicType()  +
  ") -> " + ListPairTIDRInt::BasicType()+ ", "+
  Symbol::STREAM() + "(" + ListNetDistGrp::BasicType()  +
  ") -> " + ListNetDistGrp::BasicType() + ", " +
  Symbol::STREAM() + "(" + JListInt::BasicType()  +
  ") -> " + JListInt::BasicType()+"</text--->"
  "<text> _ collectlistj </text--->"
  "<text> Collects a stream of listT of JNetAlgebra into a single "
  "ListT  of the data type.</text--->"
  "<text> query createsteramj(tupleidlist) createlistj"+
  " projecttransformstream collectlistj</text--->))";

  Operator jnetcollectlistj(
    "collectlistj",
    CollectListSpec,
    5,
    CollectListMap,
    CollectListSelect,
    CollectListTypeMap
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
int          | listint

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

    if (param.first().isEqual(ListPairTIDRLoc::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(PairTIDRLoc::BasicType()));

    if (param.first().isEqual(ListPairTIDRInt::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(PairTIDRInt::BasicType()));

    if (param.first().isEqual(ListNetDistGrp::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(NetDistanceGroup::BasicType()));

    if (param.first().isEqual(JListInt::BasicType()))
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                             nl->SymbolAtom(CcInt::BasicType()));
  }
  return listutils::typeError("Expected " + JListTID::BasicType() + ", " +
    ListPairTIDRLoc::BasicType() + ", " + ListPairTIDRInt::BasicType() + ", " +
    ListNetDistGrp::BasicType() + ", or " + JListInt::BasicType());
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
  CreateStreamValueMap<PairTIDRLoc, ListPairTIDRLoc>,
  CreateStreamValueMap<PairTIDRInt, ListPairTIDRInt>,
  CreateStreamValueMap<NetDistanceGroup, ListNetDistGrp>,
  CreateStreamValueMap<CcInt, JListInt>
};

int CreateStreamSelect ( ListExpr args )
{
  NList param(args);
  if (param.first() == JListTID::BasicType()) return 0;
  if (param.first() == ListPairTIDRLoc::BasicType()) return 1;
  if (param.first() == ListPairTIDRInt::BasicType()) return 2;
  if (param.first() == ListNetDistGrp::BasicType()) return 3;
  if (param.first() == JListInt::BasicType()) return 4;
  return -1; // this point should never been reached.
}

const string CreateStreamJSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + JListTID::BasicType() + " -> " + Symbol::STREAM() + "(" +
    TupleIdentifier::BasicType() + "), "
    + ListPairTIDRLoc::BasicType() + " -> " + Symbol::STREAM() + "("+
    PairTIDRLoc::BasicType() + "), " +
    ListPairTIDRInt::BasicType() + " -> " + Symbol::STREAM() + "("+
    PairTIDRInt::BasicType() + "), " +
    ListNetDistGrp::BasicType() + " -> " + Symbol::STREAM() + "("+
    NetDistanceGroup::BasicType() + "), " +
    JListInt::BasicType() + " -> " + Symbol::STREAM() + "("+
    CcInt::BasicType() + ")</text--->"
  "<text> createstreamj(_) </text--->"
  "<text> Expands a list of data type ListT of JNetAlgebra into a stream of "+
  " the corresponding data type T.</text--->"
  "<text> query createsteramj(tupleidlist) createlistj</text--->))";

Operator jnetcreatestreamj(
  "createstreamj",
  CreateStreamJSpec,
  5,
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
1. Access to parts of data type

1.1 Get List Elements

The operators return the specified elements of the listX
 X
==============
 jlisttid
 listpairtidrloc
 listpairtidrint
 listndg
 listint

1.1.1 Common operator methods for all getelem operators.

*/

const string maps_getElem[5][2] =
{
  {JListTID::BasicType(),  TupleIdentifier::BasicType()},
  {ListPairTIDRLoc::BasicType(),  PairTIDRLoc::BasicType()},
  {ListPairTIDRInt::BasicType(),  PairTIDRInt::BasicType()},
  {ListNetDistGrp::BasicType(),  NetDistanceGroup::BasicType()},
  {JListInt::BasicType(),  CcInt::BasicType()}
};

ListExpr GetElemTypeMap ( ListExpr args )
{
  return SimpleMaps<5,2>(maps_getElem, args);
}

int GetElemSelect (ListExpr args)
{
  return SimpleSelect<5,2>(maps_getElem, args);
}

template<class secParam, class resParam, bool selectfirst>
int GetElemValueMap( Word* args, Word& result, int message,
                     Word& local, Supplier s )
{
  secParam* t = (secParam*) args[0].addr;
  resParam* pResult = ( resParam* ) qp->ResultStorage ( s ).addr;
  if (selectfirst) t->Get(0,*pResult);
  else t->Get(t->GetNoOfComponents()-1, *pResult);
  result = SetWord(pResult);
  return 1;
}


/*
1.1.1 Different operator methods for the getelem operators

1.1.1.1 getfirstelem

*/

ValueMapping GetFirstElemValueMap[] =
{
  GetElemValueMap<JListTID, TupleIdentifier, true>,
  GetElemValueMap<ListPairTIDRLoc, PairTIDRLoc, true>,
  GetElemValueMap<ListPairTIDRInt, PairTIDRInt, true>,
  GetElemValueMap<ListNetDistGrp, NetDistanceGroup, true>,
  GetElemValueMap<JListInt, CcInt, true>
};

const string GetFirstElemSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + JListTID::BasicType() + " -> " + TupleIdentifier::BasicType() +
  ", " + ListPairTIDRLoc::BasicType() + " -> " + RouteLocation::BasicType() +
  ", " + ListPairTIDRInt::BasicType() + " -> " + JRouteInterval::BasicType() +
  ", " + ListNetDistGrp::BasicType() + " -> " + NetDistanceGroup::BasicType() +
  ", " + JListInt::BasicType() + " -> " + CcInt::BasicType() +
  "</text--->"
  "<text> getfirstelem(_) </text--->"
  "<text> Returns the first element of the input list.</text--->"
  "<text> query getfirstelem(listndg)</text--->))";

  Operator jnetgetfirstelem(
    "getfirstelem",
    GetFirstElemSpec,
    5,
    GetFirstElemValueMap,
    GetElemSelect,
    GetElemTypeMap
);

/*
1.1.1.1 getlastelem

*/

ValueMapping GetLastElemValueMap[] =
{
  GetElemValueMap<JListTID, TupleIdentifier, false>,
  GetElemValueMap<ListPairTIDRLoc, PairTIDRLoc, false>,
  GetElemValueMap<ListPairTIDRInt, PairTIDRInt, false>,
  GetElemValueMap<ListNetDistGrp, NetDistanceGroup, false>,
  GetElemValueMap<JListInt, CcInt, false>
};

const string GetLastElemSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "(<text>" + JListTID::BasicType() + " -> " + TupleIdentifier::BasicType() +
  ", " + ListPairTIDRLoc::BasicType() + " -> " + RouteLocation::BasicType() +
  ", " + ListPairTIDRInt::BasicType() + " -> " + JRouteInterval::BasicType() +
  ", " + ListNetDistGrp::BasicType() + " -> " + NetDistanceGroup::BasicType() +
  ", " + JListInt::BasicType() + " -> " + CcInt::BasicType() +
  "</text--->"
  "<text> getlastelem(_) </text--->"
  "<text> Returns the last element of the input list.</text--->"
  "<text> query getfirstelem(listndg)</text--->))";

  Operator jnetgetlastelem(
    "getlastelem",
    GetLastElemSpec,
    5,
    GetLastElemValueMap,
    GetElemSelect,
    GetElemTypeMap
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

  AddTypeConstructor(&jLineTC);
  jLineTC.AssociateKind(Kind::DATA());

  AddTypeConstructor(&jListIntTC);
  jListIntTC.AssociateKind(Kind::DATA());

/*
3.1.2 Integration of Operators

3.1.2.1 Creation of not simple Datatypes

*/

  AddOperator(&jnetcreatendg);
  AddOperator(&jnetcreaterloc);
  AddOperator(&jnetcreaterint);
  AddOperator(&jnetcreatepair);
  AddOperator(&jnetcreatelistj);
  AddOperator(&jnetcollectlistj);
  AddOperator(&jnetcreatestreamj);
  AddOperator(&jnetcreatenetwork);

/*
3.1.2.2 Access to Network Parameters

*/
  AddOperator(&jnetroutes);
  AddOperator(&jnetjunctions);
  AddOperator(&jnetsections);

/*
1.1.1.1 Access to parts of datatypes

*/

  AddOperator(&jnetgetfirstelem);
  AddOperator(&jnetgetlastelem);
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