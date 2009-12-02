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

1 Introduction

The TrafficAlgebra contains operations to estimate trafficflow data from
histories of network constrained moving gpoint objects. Defined in the
NetworkAlgebra respectively in the TemporalNetAlgebra.

2 Defines, includes, and constants

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
#include <queue>

using namespace symbols;

extern NestedList* nl;
extern QueryProcessor* qp;

/*
Auxiliary Functions

*/
void WriteTuple(GenericRelation* rel, ListExpr relNumType, int actSectId,
                int actPartNo, int actDir, MInt* flow)
{
  Tuple *actTuple = new Tuple(nl->Second(relNumType));
  actTuple->PutAttribute(TRAFFICFLOW_SECID,
                         new CcInt(true,actSectId));
  actTuple->PutAttribute(TRAFFICFLOW_PARTNO,
                         new CcInt(true,actPartNo));
  actTuple->PutAttribute(TRAFFICFLOW_DIR, new CcInt(true,actDir));
  actTuple->PutAttribute(TRAFFICFLOW_FLOW, flow);
  rel->AppendTuple(actTuple);
  actTuple->DeleteIfAllowed();
};

/*
3 Operators

3.1 ~trafficflow~

The operation ~trafficflow~ expects to get a ascending sorted stream of
~mgpsecunits~ and returns a relation containing for every section part and
direction in the stream a tuple with the sectionid(~int~),
the part number(~int), the direction(~int) and a flow value (~mint~)
telling the number of cars on this section part at every time interval of the
observation time.

TypeMapping:

*/

static string trafficFlowRelationTypeInfo =
    "(rel (tuple ((secid int) (part int) (dir int) (flow mint))))";

ListExpr OpTrafficFlowTypeMap(ListExpr in_xArgs)
{
  NList type(in_xArgs);
  if (type.length()==1)
  {
    NList st = type.first();
    NList attr("mgpsecunit");
    if (st.checkStreamTuple(attr))
    {
      ListExpr retList;
      nl->ReadFromString(trafficFlowRelationTypeInfo, retList);
      return retList;
    }
  }
  return NList::typeError("Expected a stream of mgpsecunits.");
}

/*
Value Mapping

*/

int OpTrafficFlowValueMap(Word* args, Word& result, int message,
                              Word& local, Supplier s)
{
  Word actual;
  GenericRelation* rel = (Relation*)((qp->ResultStorage(s)).addr);
  result.setAddr(rel);
  ListExpr relInfo;
  nl->ReadFromString(trafficFlowRelationTypeInfo, relInfo);
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType(relInfo);
  int actSectId = -1;
  int actPartNo = 0;
  int actDir = None;
  int actFlow = 0;
  Instant actTStart(instanttype);
  Instant actTEnd(instanttype);
  MInt *flow = new MInt(0);
  priority_queue<Instant, deque<Instant>, greater<Instant> > endInstants;
  if(rel->GetNoTuples() > 0) rel->Clear();
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  bool first = true;
  flow->StartBulkLoad();
  while (qp->Received(args[0].addr))
  {
    Tuple *curTuple = (Tuple*)actual.addr;
    MGPSecUnit *actMGPSU = (MGPSecUnit*)curTuple->GetAttribute(0);
    if (first && actMGPSU->IsDefined() && actMGPSU->GetDirect() != None)
    {
      actTStart = actMGPSU->GetTimeInterval().start;
      endInstants.push(actMGPSU->GetTimeInterval().end);
      actSectId = actMGPSU->GetSecId();
      actPartNo = actMGPSU->GetPart();
      actDir = actMGPSU->GetDirect();
      first = false;
      actFlow = 1;
    }
    else
    {
      if (actMGPSU->IsDefined() && actMGPSU->GetDirect() != None)
      {
        if (actSectId == actMGPSU->GetSecId() &&
            actPartNo == actMGPSU->GetPart() &&
            actDir == actMGPSU->GetDirect())
        {
          endInstants.push(actMGPSU->GetTimeInterval().end);
          if (actMGPSU->GetTimeInterval().start > actTStart)
          {
            while (!endInstants.empty() &&
                   actMGPSU->GetTimeInterval().start > endInstants.top())
            {
              if (actTStart != endInstants.top())
                flow->Add(UInt(Interval<Instant>(actTStart, endInstants.top(),
                                               true, false),
                             CcInt(true, actFlow)));
              actTStart = endInstants.top();
              endInstants.pop();
              actFlow--;
            }
            if (!endInstants.empty() &&
                actMGPSU->GetTimeInterval().start == endInstants.top())
            {
              endInstants.pop();
              while(!endInstants.empty() &&
                    actMGPSU->GetTimeInterval().start == endInstants.top())
              {
                actFlow--;
                endInstants.pop();
              }
            }
            else
            {
              if (!endInstants.empty() &&
                   actMGPSU->GetTimeInterval().start < endInstants.top())
              {
                if (actTStart != actMGPSU->GetTimeInterval().start)
                {
                  flow->Add(UInt(Interval<Instant> (actTStart,
                                              actMGPSU->GetTimeInterval().start,
                                                 true,false),
                              CcInt(true, actFlow)));
                  actTStart = actMGPSU->GetTimeInterval().start;
                  actFlow++;
                }
                else actFlow++;
              }
            }
          }
          else
          {
            if (actMGPSU->GetTimeInterval().start == actTStart) actFlow++;
            else // shoult not happen stream not well sorted.
            {
              cerr << "Trafficflow stopped computation."
                  << " Stream is not sorted." << endl;
              flow->EndBulkLoad();
              WriteTuple(rel, relNumType, actSectId, actPartNo, actDir, flow);
              delete flow;
              while (!endInstants.empty()) endInstants.pop();
              qp->Close(args[0].addr);
              return -1;
            }
          }
        }
        else //section, partition, or direction changed
        {
          if (!endInstants.empty())
          {
            actTEnd = endInstants.top();
            endInstants.pop();
          }
          if (actTStart != actTEnd)
            flow->Add(UInt(Interval<Instant>(actTStart, actTEnd, true, false),
                        CcInt(true, actFlow)));
          actTStart = actTEnd;
          actFlow--;
          if (!endInstants.empty())
          {
            actTEnd = endInstants.top();
            endInstants.pop();
          }
          while (!endInstants.empty())
          {
            if (actTEnd < endInstants.top())
            {
              if (actTStart != actTEnd)
                flow->Add(UInt(Interval<Instant>(actTStart, actTEnd,
                                              true, false),
                        CcInt(true, actFlow)));
              actFlow--;
              actTStart = actTEnd;
              actTEnd = endInstants.top();
              endInstants.pop();
            }
            else
            {
              if(actTEnd == endInstants.top())
              {
                actFlow--;
                endInstants.pop();
              }
              else //should never happen
              {
                cerr << "Trafficflow stopped computation."
                    << " Failure in endInstants queue." << endl;
                flow->EndBulkLoad();
                WriteTuple(rel, relNumType, actSectId, actPartNo, actDir, flow);
                flow->Clear();
                delete flow;
                while (!endInstants.empty())
                {
                  endInstants.pop();
                }
                qp->Close(args[0].addr);
                return -1;
              }
            }
          }
          if (actTStart != actTEnd)
            flow->Add(UInt(Interval<Instant> (actTStart, actTEnd, true, false),
                    CcInt(true, actFlow)));
          flow->EndBulkLoad();
          WriteTuple(rel, relNumType, actSectId, actPartNo, actDir, flow);
          actTStart = actMGPSU->GetTimeInterval().start;
          endInstants.push(actMGPSU->GetTimeInterval().end);
          actSectId = actMGPSU->GetSecId();
          actPartNo = actMGPSU->GetPart();
          actDir = actMGPSU->GetDirect();
          actFlow = 1;
          first = false;
          flow = new MInt(0);
          flow->StartBulkLoad();
        }
      }
    }
    qp->Request(args[0].addr, actual);
    curTuple->DeleteIfAllowed();
  }
  if (!endInstants.empty())
  {
    actTEnd = endInstants.top();
    endInstants.pop();
  }
  if (actTStart != actTEnd)
    flow->Add(UInt(Interval<Instant>(actTStart, actTEnd, true, false),
            CcInt(true, actFlow)));
  actTStart = actTEnd;
  actFlow--;
  if (!endInstants.empty())
  {
    actTEnd = endInstants.top();
    endInstants.pop();
  }
  if (endInstants.empty() && actTStart != actTEnd)
    flow->Add(UInt(Interval<Instant> (actTStart, actTEnd, true, false),
              CcInt(true,actFlow)));
  else {
    while (!endInstants.empty())
    {
      if (actTEnd < endInstants.top())
      {
        if (actTStart != actTEnd)
          flow->Add(UInt(Interval<Instant>(actTStart, actTEnd,
                  true, false),
                  CcInt(true, actFlow)));
        actFlow--;
        actTStart = actTEnd;
        actTEnd = endInstants.top();
        endInstants.pop();
      }
      else
      {
        if(actTEnd == endInstants.top())
        {
          actFlow--;
          endInstants.pop();
          if (endInstants.empty() && actTStart != actTEnd)
            flow->Add(UInt(Interval<Instant>(actTStart, actTEnd,
                      true, false),
                      CcInt(true, actFlow)));
        }
        else //should never happen
        {
          cerr << "Trafficflow stopped computation."
              << " Failure in endInstants queue." << endl;
          flow->EndBulkLoad();
          WriteTuple(rel, relNumType, actSectId, actPartNo, actDir, flow);
          flow->Clear();
          delete flow;
          while (!endInstants.empty()) endInstants.pop();
          qp->Close(args[0].addr);
          return -1;
        }
      }
    }
  }
  if (actTStart != actTEnd)
    flow->Add(UInt(Interval<Instant> (actTStart, actTEnd, true, false),
              CcInt(true, actFlow)));
  flow->EndBulkLoad();
  WriteTuple(rel, relNumType, actSectId, actPartNo, actDir, flow);
  qp->Close(args[0].addr);
  return 0;
}

/*
Operator Information for the user:

*/

struct trafficflowInfo : OperatorInfo {

  trafficflowInfo()
  {
    name      = "trafficflow";
    signature = "stream(tuple(mgpsecunit))->rel(int,int,int,mint)";
    syntax    = "_ trafficflow";
    meaning   = "Rel. trafficflow for sorted input stream.";
  }
};

/*
3.2 trafficflow2

*/

ListExpr OpTrafficFlow2TypeMap(ListExpr in_xArgs)
{
  NList type(in_xArgs);
  if (type.length()==1)
  {
    NList st = type.first();
    NList attr("mgpsecunit");
    if (st.checkStream(attr))
    {
      ListExpr retList;
      nl->ReadFromString(trafficFlowRelationTypeInfo, retList);
      return retList;
    }
  }
  return NList::typeError("Expected a stream of mgpsecunits.");
}

/*
Value Mapping

*/

int OpTrafficFlow2ValueMap(Word* args, Word& result, int message,
                          Word& local, Supplier s)
{
  Word actual;
  GenericRelation* rel = (Relation*)((qp->ResultStorage(s)).addr);
  result.setAddr(rel);
  ListExpr relInfo;
  nl->ReadFromString(trafficFlowRelationTypeInfo, relInfo);
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType(relInfo);
  int actSectId = -1;
  int actPartNo = 0;
  int actDir = None;
  int actFlow = 0;
  Instant actTStart(instanttype);
  Instant actTEnd(instanttype);
  MInt *flow = new MInt(0);
  priority_queue<MGPSecUnit, deque<MGPSecUnit>,
                 greater<MGPSecUnit> > mgpsecUnits;
  priority_queue<Instant, deque<Instant>,
                 greater<Instant> > endInstants;
  if(rel->GetNoTuples() > 0) rel->Clear();
  MGPSecUnit *actMGPSU = 0;
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  while (qp->Received(args[0].addr))
  {
    actMGPSU = (MGPSecUnit*)actual.addr;
    mgpsecUnits.push(*actMGPSU);
    qp->Request(args[0].addr, actual);
    delete actMGPSU;
  }
  qp->Close(args[0].addr);
  bool first = true;
  flow->StartBulkLoad();
  MGPSecUnit curMGPSec;
  while(!mgpsecUnits.empty())
  {
    curMGPSec = mgpsecUnits.top();
    mgpsecUnits.pop();
    if(first && curMGPSec.IsDefined() && curMGPSec.GetDirect() != None)
    {
      actTStart = curMGPSec.GetTimeInterval().start;
      endInstants.push(curMGPSec.GetTimeInterval().end);
      actSectId = curMGPSec.GetSecId();
      actPartNo = curMGPSec.GetPart();
      actDir = curMGPSec.GetDirect();
      first = false;
      actFlow = 1;
    }
    else
    {
      if (curMGPSec.IsDefined() && curMGPSec.GetDirect() != None)
      {
        if (actSectId == curMGPSec.GetSecId() &&
            actPartNo == curMGPSec.GetPart() &&
            actDir == curMGPSec.GetDirect())
        {
          endInstants.push(curMGPSec.GetTimeInterval().end);
          if (curMGPSec.GetTimeInterval().start > actTStart)
          {
            while (!endInstants.empty() &&
                    curMGPSec.GetTimeInterval().start > endInstants.top())
            {
              if (actTStart != endInstants.top())
                flow->Add(UInt(Interval<Instant>(actTStart, endInstants.top(),
                          true, false),
                          CcInt(true, actFlow)));
              actTStart = endInstants.top();
              endInstants.pop();
              actFlow--;
            }
            if (!endInstants.empty() &&
                 curMGPSec.GetTimeInterval().start == endInstants.top())
            {
              endInstants.pop();
              while(!endInstants.empty() &&
                     curMGPSec.GetTimeInterval().start == endInstants.top())
              {
                actFlow--;
                endInstants.pop();
              }
            }
            else
            {
              if (!endInstants.empty() &&
                   curMGPSec.GetTimeInterval().start < endInstants.top())
              {
                if (actTStart != curMGPSec.GetTimeInterval().start)
                {
                  flow->Add(UInt(Interval<Instant> (actTStart,
                            curMGPSec.GetTimeInterval().start,
                                true,false),
                                CcInt(true, actFlow)));
                  actTStart = curMGPSec.GetTimeInterval().start;
                  actFlow++;
                }
                else actFlow++;
              }
            }
          }
          else
          {
            if (curMGPSec.GetTimeInterval().start == actTStart) actFlow++;
            else // shoult not happen stream not well sorted.
            {
              cerr << "Trafficflow stopped computation."
                  << " Stream is not sorted." << endl;
              flow->EndBulkLoad();
              WriteTuple(rel, relNumType, actSectId, actPartNo, actDir, flow);
              delete flow;
              while (!endInstants.empty()) endInstants.pop();
              while (!mgpsecUnits.empty()) mgpsecUnits.pop();
              return -1;
            }
          }
        }
        else //section, partition, or direction changed
        {
          if (!endInstants.empty())
          {
            actTEnd = endInstants.top();
            endInstants.pop();
          }
          if (actTStart != actTEnd)
            flow->Add(UInt(Interval<Instant>(actTStart, actTEnd, true, false),
                      CcInt(true, actFlow)));
          actTStart = actTEnd;
          actFlow--;
          if (!endInstants.empty())
          {
            actTEnd = endInstants.top();
            endInstants.pop();
          }
          while (!endInstants.empty())
          {
            if (actTEnd < endInstants.top())
            {
              if (actTStart != actTEnd)
                flow->Add(UInt(Interval<Instant>(actTStart, actTEnd,
                          true, false),
                          CcInt(true, actFlow)));
              actFlow--;
              actTStart = actTEnd;
              actTEnd = endInstants.top();
              endInstants.pop();
            }
            else
            {
              if(actTEnd == endInstants.top())
              {
                actFlow--;
                endInstants.pop();
              }
              else //should never happen
              {
                cerr << "Trafficflow stopped computation."
                    << " Failure in endInstants queue." << endl;
                flow->EndBulkLoad();
                WriteTuple(rel, relNumType, actSectId, actPartNo, actDir, flow);
                flow->Clear();
                delete flow;
                while (!endInstants.empty()) endInstants.pop();
                while (!mgpsecUnits.empty()) mgpsecUnits.pop();
                return -1;
              }
            }
          }
          if (actTStart != actTEnd)
            flow->Add(UInt(Interval<Instant> (actTStart, actTEnd, true, false),
                      CcInt(true, actFlow)));
          flow->EndBulkLoad();
          WriteTuple(rel, relNumType, actSectId, actPartNo, actDir, flow);
          actTStart = curMGPSec.GetTimeInterval().start;
          endInstants.push(curMGPSec.GetTimeInterval().end);
          actSectId = curMGPSec.GetSecId();
          actPartNo = curMGPSec.GetPart();
          actDir = curMGPSec.GetDirect();
          actFlow = 1;
          first = false;
          flow = new MInt(0);
          flow->StartBulkLoad();
        }
      }
    }
  }
  if (!endInstants.empty())
  {
    actTEnd = endInstants.top();
    endInstants.pop();
  }
  if (actTStart != actTEnd)
    flow->Add(UInt(Interval<Instant>(actTStart, actTEnd, true, false),
              CcInt(true, actFlow)));
  actTStart = actTEnd;
  actFlow--;
  if (!endInstants.empty())
  {
    actTEnd = endInstants.top();
    endInstants.pop();
  }
  if (endInstants.empty() && actTStart != actTEnd)
    flow->Add(UInt(Interval<Instant> (actTStart, actTEnd, true, false),
              CcInt(true,actFlow)));
  else {
    while (!endInstants.empty())
    {
      if (actTEnd < endInstants.top())
      {
        if (actTStart != actTEnd)
          flow->Add(UInt(Interval<Instant>(actTStart, actTEnd,
                    true, false),
                    CcInt(true, actFlow)));
        actFlow--;
        actTStart = actTEnd;
        actTEnd = endInstants.top();
        endInstants.pop();
      }
      else
      {
        if(actTEnd == endInstants.top())
        {
          actFlow--;
          endInstants.pop();
          if (endInstants.empty() && actTStart != actTEnd)
            flow->Add(UInt(Interval<Instant>(actTStart, actTEnd,
                      true, false),
                      CcInt(true, actFlow)));
        }
        else //should never happen
        {
          cerr << "Trafficflow stopped computation."
              << " Failure in endInstants queue." << endl;
          flow->EndBulkLoad();
          WriteTuple(rel, relNumType, actSectId, actPartNo, actDir, flow);
          flow->Clear();
          delete flow;
          while (!endInstants.empty()) endInstants.pop();
          while (!mgpsecUnits.empty()) mgpsecUnits.pop();
          return -1;
        }
      }
    }
  }
  if (actTStart != actTEnd)
    flow->Add(UInt(Interval<Instant> (actTStart, actTEnd, true, false),
              CcInt(true, actFlow)));
  flow->EndBulkLoad();
  WriteTuple(rel, relNumType, actSectId, actPartNo, actDir, flow);
  return 0;
}

/*
Operator Information for the user:

*/

struct trafficflow2Info : OperatorInfo {

  trafficflow2Info()
  {
    name      = "trafficflow2";
    signature = "stream(mgpsecunit)->rel(int,int,int,mint)";
    syntax    = "_ trafficflow2";
    meaning   = "Relation trafficflow from input stream.";
  }
};
/*
4 Creating the ~TrafficAlgebra~

*/

class TrafficAlgebra : public Algebra
{
 public:
  TrafficAlgebra() : Algebra()
  {
    AddOperator(trafficflowInfo(), OpTrafficFlowValueMap, OpTrafficFlowTypeMap);
    AddOperator(trafficflow2Info(), OpTrafficFlow2ValueMap,
                OpTrafficFlow2TypeMap);
  }
  ~TrafficAlgebra() {};
};

/*
5 Initialization

*/

extern "C"
Algebra*
InitializeTrafficAlgebra( NestedList* nlRef,
                          QueryProcessor* qpRef )
{
  return (new TrafficAlgebra());
}
