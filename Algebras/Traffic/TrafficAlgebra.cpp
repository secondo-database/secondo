/*
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]

October 2009 - Simone Jandt

1 Defines, includes, and constants

*/

#include "StandardTypes.h"
#include "DateTime.h"
#include "Algebra.h"
#include "TrafficAlgebra.h"
#include "NetworkAlgebra.h"
#include "TupleIdentifier.h"
#include "TemporalAlgebra.h"
#include "TemporalNetAlgebra.h"
#include "RelationAlgebra.h"
#include "QueryProcessor.h"
#include "NestedList.h"
#include "NList.h"
#include "ConstructorTemplates.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "DBArray.h"
#include <cmath>

using namespace symbols;

extern NestedList* nl;
extern QueryProcessor* qp;



/*
1 Operators

1.1 ~trafficflow~

The operation ~trafficflow~ gets a ascending sorted stream of ~mgpsecunits~ and
returns a relation containing for every section part and direction in the
stream a tuple with the sectionid(~int~), the part number(~int),
the direction(~int) and a flow value (~mint~) telling the number of cars on
this section part at every time interval of the observation time.

TypeMapping:

*/

static string trafficFlowRelationTypeInfo =
    "(rel (tuple ((secid int) (part int) (dir int) (flow mint))))";

ListExpr OpTrafficFlowTypeMap(ListExpr in_xArgs)
{
  NList type(in_xArgs);
  NList mgpsecunit = nl->SymbolAtom("mgpsecunit");
  if (type.checkStream(mgpsecunit))
  {
    ListExpr retList;
    nl->ReadFromString(trafficFlowRelationTypeInfo, retList);
    return retList;
  }
  else return NList::typeError("Expected a stream of mgpsecunits.");
}

/*
Value Mapping

*/

int OpTrafficFlowValueMap(Word* args, Word& result, int message,
                              Word& local, Supplier s)
{
  Word actual;
  GenericRelation* rel = (Relation*)((qp->ResultStorage(s)).addr);
  ListExpr relTypeInfo;
  nl->ReadFromString(trafficFlowRelationTypeInfo, relTypeInfo);
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType(relTypeInfo);
  int actSectId = -1;
  int actPartNo = 0;
  int actDir = None;
  int actFlow = 0;
  Instant actTStart(instanttype);
  Instant actTEnd(instanttype);
  MInt *flow = new MInt(0);
  if(rel->GetNoTuples() > 0)
  {
    rel->Clear();
  }
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  bool first = true;
  while (qp->Received(args[0].addr))
  {
    MGPSecUnit *actMGPSU = (MGPSecUnit*)actual.addr;
    if (first && actMGPSU->IsDefined() && actMGPSU->GetDirect() != None)
    {
      actTStart = actMGPSU->GetTimeInterval().start;
      actTEnd = actMGPSU->GetTimeInterval().end;
      actSectId = actMGPSU->GetSecId();
      actPartNo = actMGPSU->GetPart();
      actDir = actMGPSU->GetDirect();
      first = false;
      actFlow++;
      flow->StartBulkLoad();
    }
    else
    {
      if (actMGPSU->IsDefined()&&actMGPSU->GetDirect() != None)
      {
        if (actSectId == actMGPSU->GetSecId() &&
            actPartNo == actMGPSU->GetPart() &&
            actDir == actMGPSU->GetDirect())
        {
          if (actMGPSU->GetTimeInterval().start >= actTEnd)
          {
            flow->Add(UInt(Interval<Instant> (actTStart, actTEnd, true, false),
                          CcInt(true, actFlow)));
            actFlow--;
            flow->Add(UInt(Interval<Instant> (actTEnd,
                      actMGPSU->GetTimeInterval().start, true, false),
                      CcInt(true, actFlow)));
            actFlow++;
            actTStart = actMGPSU->GetTimeInterval().start;
            actTEnd = actMGPSU->GetTimeInterval().end;
          }
          else
          {
            flow->Add(UInt(Interval<Instant> (actTStart,
                      actMGPSU->GetTimeInterval().start, true, false),
                CcInt(true, actFlow)));
            actFlow++;

          }
        }
        else
        {
        }
      }
    }
  }
  Tuple *actTuple = new Tuple(relNumType);
  actTuple->PutAttribute(TRAFFICFLOW_SECID, new CcInt(true,actSectId));
  actTuple->PutAttribute(TRAFFICFLOW_PARTNO, new CcInt(true,actPartNo));
  actTuple->PutAttribute(TRAFFICFLOW_DIR, new CcInt(true,actDir));
  actTuple->PutAttribute(TRAFFICFLOW_FLOW, flow);
  rel->AppendTuple(actTuple);
  actTuple->DeleteIfAllowed();
  result.setAddr(rel);
  qp->Close(args[0].addr);
  return 0;
}


struct trafficflowInfo : OperatorInfo {

  trafficflowInfo()
  {
    name      = "trafficflow";
    signature = "stream(mgpsecunit)->rel(int,int,int,mint)";
    syntax    = "_ trafficflow";
    meaning   = "Computes the trafficflow relation.";
  }
};

/*
5 Creating the ~TrafficAlgebra~

*/

class TrafficAlgebra : public Algebra
{
 public:
  TrafficAlgebra() : Algebra()
  {
    AddOperator(trafficflowInfo(), OpTrafficFlowValueMap, OpTrafficFlowTypeMap);
  }
  ~TrafficAlgebra() {};
};

/*
6 Initialization

*/

extern "C"
Algebra*
InitializeTrafficAlgebra( NestedList* nlRef,
                          QueryProcessor* qpRef )
{
  return (new TrafficAlgebra());
}
