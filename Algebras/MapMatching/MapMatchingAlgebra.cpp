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

[1] Implementation file of the MapMatching Algebra

January-April 2012, Matthias Roth

[TOC]

1 Overview

This implementation file contains the implementation of the class ~MapMatchingAlgebra~.

For more detailed information see MapMatchingAlgebra.h.

2 Defines and Includes

*/

#include "MapMatchingAlgebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "StandardTypes.h"

#include "../Network/NetworkAlgebra.h"
#include "../TemporalNet/TemporalNetAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;

#include "TypeMapUtils.h"
#include "Symbols.h"

#include <string>
using namespace std;

#include "MapMatchingSimple.h"
#include "MapMatchingMHT.h"


namespace mapmatch {

/*
3 mapmatchsimple-operator

3.1 Operator-Info

*/
struct MapMatchSimpleInfo : OperatorInfo {

  MapMatchSimpleInfo()
  {
    name      = "mapmatchsimple";
    signature = Network::BasicType() + " x " +
                MPoint::BasicType() + " -> " +
                MGPoint::BasicType();
    syntax    = "mapmatchsimple ( _ , _ )";
    meaning   = "The operation tries to map the mpoint to "
                "the given network as well as possible.";
    example   = "mapmatchsimple (TODO, TODO)";
  }
};

/*
3.2 Type-Mapping

*/
ListExpr OpMapMatchingTypeMap(ListExpr in_xArgs)
{
  NList param(in_xArgs);

  if( param.length() != 2)
    return listutils::typeError("two arguments expected");

  if (!param.first().isSymbol(Network::BasicType()))
    return listutils::typeError("1. argument must be " + Network::BasicType());

  if (!param.second().isSymbol(MPoint::BasicType()))
    return listutils::typeError("2. argument must be " + MPoint::BasicType());

  return nl->SymbolAtom( MGPoint::BasicType() );
}

/*
3.3 Value-Mapping

*/
int OpMapMatchingSimpleValueMapping(Word* args,
                                    Word& result,
                                    int message,
                                    Word& local,
                                    Supplier in_xSupplier)
{
  // cout << "OpMapMatching called" << endl;

  // Initialize Result
  result = qp->ResultStorage(in_xSupplier);
  MGPoint* res = static_cast<MGPoint*>(result.addr);

  // get Arguments
  Network *pNetwork = static_cast<Network*>(args[0].addr);
  MPoint *pMPoint = static_cast<MPoint*>(args[1].addr);

  // Do Map Matching

  MapMatchingSimple MapMatching(pNetwork, pMPoint);

  if (!MapMatching.DoMatch(res))
  {
      // Error
  }

  return 0;
}

/*
4 mapmatchmht-operator

4.1 Operator-Info

*/
struct MapMatchMHTInfo : OperatorInfo {

  MapMatchMHTInfo()
  {
    name      = "mapmatchmht";
    signature = Network::BasicType() + " x " +
                MPoint::BasicType() + " -> " +
                MGPoint::BasicType();
    syntax    = "mapmatchmht ( _ , _ )";
    meaning   = "The operation tries to map the mpoint to "
                "the given network as well as possible.";
    example   = "mapmatchmht (TODO, TODO)";
  }
};

/*
4.2 Type-Mapping
    s. OpMapMatchingTypeMap

*/

/*
4.3 Value-Mapping

*/
int OpMapMatchingMHTValueMapping(Word* args,
                                 Word& result,
                                 int message,
                                 Word& local,
                                 Supplier in_xSupplier)
{
  // cout << "OpMapMatching called" << endl;

  // Initialize Result
  result = qp->ResultStorage(in_xSupplier);
  MGPoint* res = static_cast<MGPoint*>(result.addr);

  // get Arguments
  Network *pNetwork = static_cast<Network*>(args[0].addr);
  MPoint *pMPoint = static_cast<MPoint*>(args[1].addr);

  // Do Map Matching

  MapMatchingMHT MapMatching(pNetwork, pMPoint);

  if (!MapMatching.DoMatch(res))
  {
      // Error
  }

  return 0;
}


/*
5 Implementation of the MapMatchingAlgebra Class

*/

MapMatchingAlgebra::MapMatchingAlgebra()
:Algebra()
{

/*
5.1 Registration of Types

*/


/*
5.2 Registration of Operators

*/

    AddOperator(MapMatchSimpleInfo(),
                OpMapMatchingSimpleValueMapping,
                OpMapMatchingTypeMap);

    AddOperator(MapMatchMHTInfo(),
                OpMapMatchingMHTValueMapping,
                OpMapMatchingTypeMap);
}

MapMatchingAlgebra::~MapMatchingAlgebra()
{
}

} // end of namespace ~mapmatch~


/*
6 Initialization

*/

extern "C" Algebra* InitializeMapMatchingAlgebra(NestedList* /*nlRef*/,
                                                 QueryProcessor* /*qpRef*/)
{
  return new mapmatch::MapMatchingAlgebra;
}

