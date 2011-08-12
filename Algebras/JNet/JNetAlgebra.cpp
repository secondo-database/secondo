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

#include "../../include/QueryProcessor.h"
#include "../../include/AlgebraTypes.h"
#include "../../include/Operator.h"
#include "../../include/ConstructorTemplates.h"
#include "../../include/ListUtils.h"
#include "../../include/Symbols.h"
#include "../../include/StandardTypes.h"
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



extern NestedList* nl;
extern QueryProcessor* qp;

/*
1 Type Constructors

1.1 ~Direction~

*/

struct directionInfo:ConstructorInfo
{
  directionInfo():ConstructorInfo()
  {
    name = Direction::BasicType();
    signature = "-> " + Kind::DATA();
    typeExample = Direction::BasicType();
    listRep = "(<JSide>)";
    valueExample = "(Up)";
    remarks = "Attributedatatype for side values.";
  }
};

struct directionFunctions:ConstructorFunctions<Direction>{
  directionFunctions():ConstructorFunctions<Direction>(){
    in = Direction::In;
    out = Direction::Out;
    create = Direction::Create;
    deletion = Direction::Delete;
    close = Direction::Close;
    clone = Direction::Clone;
    cast = Direction::Cast;
    sizeOf = Direction::SizeOf;
    kindCheck = Direction::KindCheck;
    open = Direction::Open;
    save = Direction::Save;
  }
};

directionInfo dirInfo;
directionFunctions dirFunc;
TypeConstructor directionTC(dirInfo, dirFunc);

/*
1.2 ~RouteLocation~

*/

struct routeLocationInfo:ConstructorInfo
{
  routeLocationInfo():ConstructorInfo()
  {
    name = RouteLocation::BasicType();
    signature = "-> " + Kind::DATA();
    typeExample = RouteLocation::BasicType();
    listRep = "(" + CcInt::BasicType() + " "+ CcReal::BasicType() + " " +
              Direction::BasicType() + ")";
    valueExample = "(1 2.0 Up)";
    remarks = "Attributedatatype for routelocation values.";
  }
};

struct routeLocationFunctions:ConstructorFunctions<RouteLocation>{
  routeLocationFunctions():ConstructorFunctions<RouteLocation>(){
    in = RouteLocation::In;
    out = RouteLocation::Out;
    create = RouteLocation::Create;
    deletion = RouteLocation::Delete;
    close = RouteLocation::Close;
    clone = RouteLocation::Clone;
    cast = RouteLocation::Cast;
    sizeOf = RouteLocation::SizeOf;
    kindCheck = RouteLocation::KindCheck;
    open = RouteLocation::Open;
    save = RouteLocation::Save;
  }
};

routeLocationInfo rLInfo;
routeLocationFunctions rLFunc;
TypeConstructor routeLocationTC(rLInfo, rLFunc);


/*
1.3 ~JRouteInterval~

*/

struct jRouteIntervalInfo:ConstructorInfo
{
  jRouteIntervalInfo():ConstructorInfo()
  {
    name = JRouteInterval::BasicType();
    signature = "-> "+ Kind::DATA();
    typeExample = JRouteInterval::BasicType();
    listRep = "("+ CcInt::BasicType() + " "+ CcInt::BasicType() + " " +
              CcReal::BasicType() + " " + Direction::BasicType() + ")";
    valueExample = "(1 2.0 5.0 Up)";
    remarks = "Attributedatatype for jrouteinterval values.";
  }
};

struct jRouteIntervalFunctions:ConstructorFunctions<JRouteInterval>{
  jRouteIntervalFunctions():ConstructorFunctions<JRouteInterval>(){
    in = JRouteInterval::In;
    out = JRouteInterval::Out;
    create = JRouteInterval::Create;
    deletion = JRouteInterval::Delete;
    close = JRouteInterval::Close;
    clone = JRouteInterval::Clone;
    cast = JRouteInterval::Cast;
    sizeOf = JRouteInterval::SizeOf;
    kindCheck = JRouteInterval::KindCheck;
    open = JRouteInterval::Open;
    save = JRouteInterval::Save;
  }
};

jRouteIntervalInfo jriInfo;
jRouteIntervalFunctions jriFunc;
TypeConstructor jRouteIntervalTC(jriInfo, jriFunc);

/*
1.4 ~JListTID~

*/

struct jListTIDInfo:ConstructorInfo
{
  jListTIDInfo():ConstructorInfo()
  {
    name = JListTID::BasicType();
    signature = "-> " + Kind::DATA();
    typeExample = JListTID::BasicType();
    listRep = "("+ TupleIdentifier::BasicType() + " ... "+
              TupleIdentifier::BasicType()+")";
    valueExample = "(1 17 5648)";
    remarks = "list of tuple ids";
  }
};

struct jListTIDFunctions:ConstructorFunctions<JListTID>{
  jListTIDFunctions():ConstructorFunctions<JListTID>(){
    in = JListTID::In;
    out = JListTID::Out;
    create = JListTID::Create;
    deletion = JListTID::Delete;
    close = JListTID::Close;
    clone = JListTID::Clone;
    cast = JListTID::Cast;
    sizeOf = JListTID::SizeOf;
    kindCheck = JListTID::KindCheck;
    open = JListTID::Open;
    save = JListTID::Save;
  }
};

jListTIDInfo jLTIDInfo;
jListTIDFunctions jLTIDFunc;
TypeConstructor jListTIDTC(jLTIDInfo, jLTIDFunc);

/*
1.5 ~PairTIDRLoc~

*/

struct pTIDRLocInfo:ConstructorInfo
{
  pTIDRLocInfo():ConstructorInfo()
  {
    name = PairTIDRLoc::BasicType();
    signature = "-> " + Kind::DATA();
    typeExample = PairTIDRLoc::BasicType();
    listRep = "("+ TupleIdentifier::BasicType() + " " +
              RouteLocation::BasicType()+ ")";
    valueExample = "(17 (1 15.0 Up))";
    remarks = "Pair of TupleId and RouteLocation.";
  }
};

struct pTIDRLocFunctions:ConstructorFunctions<PairTIDRLoc>{
  pTIDRLocFunctions():ConstructorFunctions<PairTIDRLoc>(){
    in = PairTIDRLoc::In;
    out = PairTIDRLoc::Out;
    create = PairTIDRLoc::Create;
    deletion = PairTIDRLoc::Delete;
    close = PairTIDRLoc::Close;
    clone = PairTIDRLoc::Clone;
    cast = PairTIDRLoc::Cast;
    sizeOf = PairTIDRLoc::SizeOf;
    kindCheck = PairTIDRLoc::KindCheck;
    open = PairTIDRLoc::Open;
    save = PairTIDRLoc::Save;
  }
};

pTIDRLocInfo pairTIDRLocInfo;
pTIDRLocFunctions pairTIDRLocFunc;
TypeConstructor pTIDRLocTC(pairTIDRLocInfo, pairTIDRLocFunc);

/*
1.6 ~ListPairTIDRLoc~

*/

struct listPTIDRLocInfo:ConstructorInfo
{
  listPTIDRLocInfo():ConstructorInfo()
  {
    name = ListPTIDRLoc::BasicType();
    signature = "-> " + Kind::DATA();
    typeExample = ListPTIDRLoc::BasicType();
    listRep = "("+ PairTIDRLoc::BasicType() + " ... " +
              PairTIDRLoc::BasicType() +")";
    valueExample = "((231 (1 17.0 Up))(56 (2 13.5 Down))))";
    remarks = "list of pairs of tuple ids and routelocation";
  }
};

struct listPTIDRLocFunctions:ConstructorFunctions<ListPTIDRLoc>{
  listPTIDRLocFunctions():ConstructorFunctions<ListPTIDRLoc>(){
    in = ListPTIDRLoc::In;
    out = ListPTIDRLoc::Out;
    create = ListPTIDRLoc::Create;
    deletion = ListPTIDRLoc::Delete;
    close = ListPTIDRLoc::Close;
    clone = ListPTIDRLoc::Clone;
    cast = ListPTIDRLoc::Cast;
    sizeOf = ListPTIDRLoc::SizeOf;
    kindCheck = ListPTIDRLoc::KindCheck;
    open = ListPTIDRLoc::Open;
    save = ListPTIDRLoc::Save;
  }
};

listPTIDRLocInfo lPTIDRLocInfo;
listPTIDRLocFunctions lPTIDRLocFunc;
TypeConstructor listPTIDRLocTC(lPTIDRLocInfo, lPTIDRLocFunc);

/*
1.7 ~PairTIDRInterval~

*/

struct pTIDRIntInfo:ConstructorInfo
{
  pTIDRIntInfo():ConstructorInfo()
  {
    name = PairTIDRInterval::BasicType();
    signature = "-> "+ Kind::DATA() ;
    typeExample = PairTIDRInterval::BasicType();
    listRep = "(" + TupleIdentifier::BasicType() + " " +
              JRouteInterval::BasicType() + ")";
    valueExample = "(17 (1 15.0 27.8 Up))";
    remarks = "Pair of TupleId and RouteInterval.";
  }
};

struct pTIDRIntFunctions:ConstructorFunctions<PairTIDRInterval>{
  pTIDRIntFunctions():ConstructorFunctions<PairTIDRInterval>(){
    in = PairTIDRInterval::In;
    out = PairTIDRInterval::Out;
    create = PairTIDRInterval::Create;
    deletion = PairTIDRInterval::Delete;
    close = PairTIDRInterval::Close;
    clone = PairTIDRInterval::Clone;
    cast = PairTIDRInterval::Cast;
    sizeOf = PairTIDRInterval::SizeOf;
    kindCheck = PairTIDRInterval::KindCheck;
    open = PairTIDRInterval::Open;
    save = PairTIDRInterval::Save;
  }
};

pTIDRIntInfo pairTIDRIntInfo;
pTIDRIntFunctions pairTIDRIntFunc;
TypeConstructor pTIDRIntTC(pairTIDRIntInfo, pairTIDRIntFunc);

/*
1.8 ~ListPTIDRInt~

*/

struct listPTIDRIntInfo:ConstructorInfo
{
  listPTIDRIntInfo():ConstructorInfo()
  {
    name = ListPTIDRInt::BasicType();
    signature = "-> " + Kind::DATA();
    typeExample = ListPTIDRInt::BasicType();
    listRep = "(" + PairTIDRInterval::BasicType() + " ... " +
              PairTIDRInterval::BasicType() + ")";
    valueExample = "((231 (1 17.0 28.6 Down))(56 (2 1.5 6.8 Up))))";
    remarks = "list of pairs of TupleIds and RouteLocation";
  }
};

struct listPTIDRIntFunctions:ConstructorFunctions<ListPTIDRInt>{
  listPTIDRIntFunctions():ConstructorFunctions<ListPTIDRInt>(){
    in = ListPTIDRInt::In;
    out = ListPTIDRInt::Out;
    create = ListPTIDRInt::Create;
    deletion = ListPTIDRInt::Delete;
    close = ListPTIDRInt::Close;
    clone = ListPTIDRInt::Clone;
    cast = ListPTIDRInt::Cast;
    sizeOf = ListPTIDRInt::SizeOf;
    kindCheck = ListPTIDRInt::KindCheck;
    open = ListPTIDRInt::Open;
    save = ListPTIDRInt::Save;
  }
};

listPTIDRIntInfo lPTIDRIntInfo;
listPTIDRIntFunctions lPTIDRIntFunc;
TypeConstructor listPTIDRIntTC(lPTIDRIntInfo, lPTIDRIntFunc);

/*
1.9 ~NetDistanceGroup~

*/

struct ndgInfo:ConstructorInfo
{
  ndgInfo():ConstructorInfo()
  {
    name = NetDistanceGroup::BasicType();
    signature = "-> " + Kind::DATA();
    typeExample = NetDistanceGroup::BasicType();
    listRep = "("+ TupleIdentifier::BasicType() + " " +
              TupleIdentifier::BasicType() + " " +
              TupleIdentifier::BasicType() + " " + CcReal::BasicType() + ")";
    valueExample = "(17 535 121 27.8)";
    remarks = "List of tid and netdistance value.";
  }
};

struct ndgFunctions:ConstructorFunctions<NetDistanceGroup>{
  ndgFunctions():ConstructorFunctions<NetDistanceGroup>(){
    in = NetDistanceGroup::In;
    out = NetDistanceGroup::Out;
    create = NetDistanceGroup::Create;
    deletion = NetDistanceGroup::Delete;
    close = NetDistanceGroup::Close;
    clone = NetDistanceGroup::Clone;
    cast = NetDistanceGroup::Cast;
    sizeOf = NetDistanceGroup::SizeOf;
    kindCheck = NetDistanceGroup::KindCheck;
    open = NetDistanceGroup::Open;
    save = NetDistanceGroup::Save;
  }
};

ndgInfo netDistGroupInfo;
ndgFunctions netDistGroupFunc;
TypeConstructor netDistGroupTC(netDistGroupInfo, netDistGroupFunc);

/*
1.10 ~ListNetDistGrp~

*/

struct listNetDistGrpInfo:ConstructorInfo
{
  listNetDistGrpInfo():ConstructorInfo()
  {
    name = ListNetDistGrp::BasicType();
    signature = "-> " + Kind::DATA();
    typeExample = ListNetDistGrp::BasicType();
    listRep = "(" + NetDistanceGroup::BasicType() + " ... " +
              NetDistanceGroup::BasicType() + ")";
    valueExample = "((1 15 6 28.6)(56 2 8 1.5))";
    remarks = "list of netdistancegroups";
  }
};

struct listNetDistGrpFunctions:ConstructorFunctions<ListNetDistGrp>{
  listNetDistGrpFunctions():ConstructorFunctions<ListNetDistGrp>(){
    in = ListNetDistGrp::In;
    out = ListNetDistGrp::Out;
    create = ListNetDistGrp::Create;
    deletion = ListNetDistGrp::Delete;
    close = ListNetDistGrp::Close;
    clone = ListNetDistGrp::Clone;
    cast = ListNetDistGrp::Cast;
    sizeOf = ListNetDistGrp::SizeOf;
    kindCheck = ListNetDistGrp::KindCheck;
    open = ListNetDistGrp::Open;
    save = ListNetDistGrp::Save;
  }
};

listNetDistGrpInfo lNDGInfo;
listNetDistGrpFunctions lNDGFunc;
TypeConstructor listNDGTC(lNDGInfo, lNDGFunc);

/*
1.11 ~JNetwork~

*/

struct jNetworkInfo:ConstructorInfo
{
  jNetworkInfo():ConstructorInfo()
  {
    name = JNetwork::BasicType();
    signature = "-> " + Kind::JNETWORK();
    typeExample = JNetwork::BasicType();
    listRep = "(" + CcString::BasicType() + " " + Relation::BasicType() + " " +
              Relation::BasicType() + " " + Relation::BasicType() + ")";
    valueExample = "(netname (rel(Junctions)) (rel(Sections)) (rel(Routes)))";
    remarks = "Central network object.";
  }
};

struct jNetworkFunctions:ConstructorFunctions<JNetwork>{
  jNetworkFunctions():ConstructorFunctions<JNetwork>(){
    in = JNetwork::In;
    out = JNetwork::Out;
    create = JNetwork::Create;
    deletion = JNetwork::Delete;
    close = JNetwork::Close;
    clone = JNetwork::Clone;
    cast = JNetwork::Cast;
    sizeOf = JNetwork::SizeOf;
    kindCheck = JNetwork::KindCheck;
    open = JNetwork::Open;
    save = JNetwork::Save;
  }
};

jNetworkInfo jNetInfo;
jNetworkFunctions jNetFunc;
TypeConstructor jNetTC(jNetInfo, jNetFunc);

/*
1.1 ~JPoint~

*/

struct jPointInfo:ConstructorInfo
{
  jPointInfo():ConstructorInfo()
  {
    name = JPoint::BasicType();
    signature = "-> " + Kind::DATA();
    typeExample = JPoint::BasicType();
    listRep = "(" + CcString::BasicType() + " " + RouteLocation::BasicType() +
              ")";
    valueExample = "(testnet (12 45.6 Down))";
    remarks = "Single position in network.";
  }
};

struct jPointFunctions:ConstructorFunctions<JPoint>{
  jPointFunctions():ConstructorFunctions<JPoint>(){
    in = JPoint::In;
    out = JPoint::Out;
    create = JPoint::Create;
    deletion = JPoint::Delete;
    close = JPoint::Close;
    clone = JPoint::Clone;
    cast = JPoint::Cast;
    sizeOf = JPoint::SizeOf;
    kindCheck = JPoint::KindCheck;
    open = JPoint::Open;
    save = JPoint::Save;
  }
};

jPointInfo jPInfo;
jPointFunctions jPFunc;
TypeConstructor jPTC(jPInfo, jPFunc);


/*
2 Secondo Operators

2.1 ~createpair~

Creates Pairs of ~TupleId~s and ~RouteLocation~s and ~RouteInterval~s.

*/

ListExpr CreatePairTypeMap ( ListExpr args )
{
  NList param(args);
  if (param.length() != 2)
    return listutils::typeError("Expected 2 arguments.");

  NList tupId(param.first());
  if (!tupId.isSymbol(TupleIdentifier::BasicType()))
    return listutils::typeError("1.Argument must be tid.");

  NList rLoc(param.second());
  if (rLoc.isSymbol(RouteLocation::BasicType()))
    return nl->SymbolAtom(PairTIDRLoc::BasicType());
  else
    if (rLoc.isSymbol(JRouteInterval::BasicType()))
      return nl->SymbolAtom(PairTIDRInterval::BasicType());
    else
      return
        listutils::typeError("2.Arg. must be routelocation or jrouteinterval.");
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

int CreatePairSelect ( ListExpr args )
{
  NList param(args);
  if (param.second() == RouteLocation::BasicType()) return 0;
  if (param.second() == JRouteInterval::BasicType()) return 1;
  return -1; // this point should never been reached.
}

OperatorInfo CreatePairInfo(
  "createpair",
  "tid x routelocation -> pairtidrloc, "
  "tid x jrouteinterval-> pairtidjrouteinterval",
  "createpair( _ , _ )",
  "Creates a pair of tuple id and route location or route interval.",
  "query createpair(tid, routelocation)"
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
  NList param(args);
  if (param.length() != 4)
    return listutils::typeError("Expected 4 arguments.");

  NList tupId1(param.first());
  if (!tupId1.isSymbol(TupleIdentifier::BasicType()))
    return listutils::typeError("1.Argument must be tid.");

  NList tupId2(param.second());
  if (!tupId2.isSymbol(TupleIdentifier::BasicType()))
    return listutils::typeError("2.Argument must be tid.");

  NList tupId3(param.third());
  if (!tupId3.isSymbol(TupleIdentifier::BasicType()))
    return listutils::typeError("3.Argument must be tid.");

  NList dist(param.fourth());
  if (!dist.isSymbol(CcReal::BasicType()))
    return listutils::typeError("4.Argument must be real.");

  return nl->SymbolAtom(NetDistanceGroup::BasicType());
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


OperatorInfo CreateNetDistGroupInfo(
  "createnetdistgroup",
  "tid x tid x tid x real -> netdistancegroup",
  "createnetdistancegroup( _ , _ , _ , _ )",
  "Creates a netdistance group from tid target node, tid next section, "
  "tid next node on path, real netdistance.",
  "query createnetdistancegroup(tid, tid, tid, real)"
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
  return listutils::typeError("Expected ((stream X)). X in tid, pairtidrloc,"
                              " pairtidrint, netdistancegroup.");
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

OperatorInfo CreateListInfo(
  "createlistj",
  "stream(X) -> listX",
  "_ createlistj",
  "Collects a stream of objects in a list. The objects X can be of type tid,"
  " pairtidrloc, pairtidrint, netdistancegroup.",
  "query createstreamj(listtid) createlistj"
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
  return listutils::typeError("Expected jlisttid, listptidrloc,"
                              " listptidrint, listnetdistgrp.");
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

OperatorInfo CreateStreamInfo(
  "createstreamj",
  "listX -> stream(X)",
  "createstreamj(_)",
  "Expands an object list into a stream of objects. The list can be of type"
  "jlisttid, listptidrloc, listptidrint, listnetdistgrp.",
  "query createstreamj(listtid) createlistj"
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

  ListExpr attrtype;
  int j = listutils::findAttribute(juncAttrs.listExpr(), "jid", attrtype);
  if (!(j!=0 && nl->IsEqual(attrtype, CcInt::BasicType())))
    return listutils::typeError("1.attr. of 1.relation must be jid int");

  j = listutils::findAttribute(juncAttrs.listExpr(), "pos", attrtype);
  if (!(j!=0 && nl->IsEqual(attrtype, Point::BasicType())))
    return listutils::typeError("2.attr. of 1.relation must be pos point");

  j = listutils::findAttribute(juncAttrs.listExpr(), "rid", attrtype);
  if (!(j!=0 && nl->IsEqual(attrtype, CcInt::BasicType())))
    return listutils::typeError("3.attr. of 1.relation must be rid int");

  j = listutils::findAttribute(juncAttrs.listExpr(), "r_pos", attrtype);
  if (!(j!=0 && nl->IsEqual(attrtype, CcReal::BasicType())))
    return listutils::typeError("4.attr. of 1.relation must be r_pos real");

  NList routeRel(param.third());
  NList routeAttrs;

  if (!routeRel.checkRel(routeAttrs))
    return listutils::typeError("3.Argument must be routes relation.");

  if (routeAttrs.length() != 6)
    return listutils::typeError("routes rel must have 6 attributes");

  j = listutils::findAttribute(routeAttrs.listExpr(), "rid", attrtype);
  if (!(j!=0 && nl->IsEqual(attrtype, CcInt::BasicType())))
    return listutils::typeError("1.attr. of 2.relation must be rid int");

  j = listutils::findAttribute(routeAttrs.listExpr(), "jid", attrtype);
  if (!(j!=0 && nl->IsEqual(attrtype, CcInt::BasicType())))
    return listutils::typeError("2.attr. of 2.relation must be jid int");

  j = listutils::findAttribute(routeAttrs.listExpr(), "r_pos", attrtype);
  if (!(j!=0 && nl->IsEqual(attrtype, CcReal::BasicType())))
    return listutils::typeError("3.attr. of 2.relation must be r_pos real");

  j = listutils::findAttribute(routeAttrs.listExpr(), "vmax", attrtype);
  if (!(j!=0 && nl->IsEqual(attrtype, CcInt::BasicType())))
    return listutils::typeError("4.attr. of 2.relation must be vmax int");

  j = listutils::findAttribute(routeAttrs.listExpr(), "curve", attrtype);
  if (!(j!=0 && nl->IsEqual(attrtype, SimpleLine::BasicType())))
    return listutils::typeError("5.attr. of 2.relation must be curve sline");

  j = listutils::findAttribute(routeAttrs.listExpr(), "side", attrtype);
  if (!(j!=0 && nl->IsEqual(attrtype, Direction::BasicType())))
    return listutils::typeError("6.attr. of 2.rel must be side jdirection");

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

OperatorInfo CreateJNetworkInfo(
  "createjnetwork",
  "string x rel(tuple((jid int)(pos point)(rid int)(r_pos real))) x "
  " rel(tuple((rid int)(jid int)(r_pos real)(vmax real)(curve sline)))"
  " -> jnetwork",
  "createjnetwork(name, junctionsRelation, routesRelation)",
  "Creates a network object name from the two inputrelations.",
  "let netname = createjnetwork('netname', junctionsRelation, routesRelation)"
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
    return listutils::typeError("Argument must be jnetwork.");

  ListExpr xType;
  nl->ReadFromString ( JNetwork::routesTypeInfo, xType );
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

OperatorInfo routesInfo(
  "routes",
  "jnetwork -> rel (tuple ( (id int) (listjunctions listptidrloc)"
  "(listsections listptidrint) (lenth real)))",
  "routes(_)",
  "Returns the routes relation of the jnetworkobject",
  "query routes(jnetworkobject)"
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
    return listutils::typeError("Argument must be jnetwork.");

  ListExpr xType;
  nl->ReadFromString ( JNetwork::junctionsTypeInfo, xType );
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

OperatorInfo junctionsInfo(
  "junctions",
  "jnetwork -> rel (tuple( (id int) (pos point)"
  "(listjuncpos listptidrloc) (listinsections jlisttid)"
  "(listoutsections jlisttid) (listdistances listnetdistgrp)))",
  "junctions(_)",
  "Returns the junctions relation of the jnetworkobject",
  "query junctions(jnetworkobject)"
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
    return listutils::typeError("Argument must be jnetwork.");

  ListExpr xType;
  nl->ReadFromString ( JNetwork::sectionsTypeInfo, xType );
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

OperatorInfo sectionsInfo(
  "sections",
  "jnetwork -> rel( tuple ( (id int) (curve sline)"
  "(startjunc tid) (endjunc tid) (listsectrint listptidrint) "
  "(listadjsectup jlisttid) (listadjsectdown jlisttid) "
  "(listrevadjsectup jlisttid) (listrevadjsectdown jlisttid)"
  "(lenth real) (vmax real) (sectdir jdirection)))",
  "sections(_)",
  "Returns the routes relation of the jnetworkobject",
  "query sections(jnetworkobject)"
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

  AddTypeConstructor(&jRouteIntervalTC);
  jRouteIntervalTC.AssociateKind(Kind::DATA());

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

  AddTypeConstructor(&jPTC);
  jPTC.AssociateKind(Kind::DATA());

/*
3.1.2 Integration of Operators

3.1.2.1 Creation of not simple Datatypes

*/

  AddOperator(CreatePairInfo, CreatePairMap, CreatePairSelect,
              CreatePairTypeMap);
  AddOperator(CreateNetDistGroupInfo, CreateNetDistGroupValueMap,
              CreateNetDistGroupTypeMap);
  AddOperator(CreateListInfo, CreateListMap, CreateListSelect,
              CreateListTypeMap);
  AddOperator(CreateStreamInfo, CreateStreamMap, CreateStreamSelect,
              CreateStreamTypeMap);
  AddOperator(CreateJNetworkInfo, CreateJNetworkVM, CreateJNetworkTM);

/*
3.1.2.2 Access to Network Parameters

*/
  AddOperator(routesInfo, routesVM, routesTM);
  AddOperator(junctionsInfo, junctionsVM, junctionsTM);
  AddOperator(sectionsInfo, sectionsVM, sectionsTM);
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