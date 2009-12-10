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
#include "NetworkManager.h"
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
3 Auxiliary Functions and structs

3.1 struct ~speed Instant~

Helper struct for average speed computing in traffic jam estimation.

*/

class speedInstant
{
  public:
    speedInstant(){};

  speedInstant(const double s, const Instant& i):
      inst(i)
      {
        speed = s;
      };

  speedInstant(const speedInstant& sI):
      inst(sI.inst)
      {
        speed = sI.speed;
      };

      ~speedInstant(){};

      speedInstant& operator=(const speedInstant sI)
      {
        speed = sI.speed;
        inst = sI.inst;
        return *this;
      };

      int Compare (const speedInstant *sI) const
      {
        return inst.Compare(&sI->inst);
      };

      bool operator<(const speedInstant sI) const
      {
        return Compare(&sI) < 0;
      };

      bool operator>(const speedInstant sI) const
      {
        return Compare(&sI) > 0;
      };

      bool operator==(const speedInstant sI)const
      {
        return Compare(&sI)==0;
      };

      bool operator!=(const speedInstant sI)const
      {
        return Compare(&sI)!=0;
      };

      bool operator<=(const speedInstant sI)const
      {
        return Compare(&sI)<=0;
      };

      bool operator>=(const speedInstant sI)const
      {
        return Compare(&sI)>=0;
      };

      double speed;
      Instant inst;
};

/*
3.1 ~InitializeValues~

3.1.1 Traffic

*/

void InitializeValues(Instant &actTStart,
                      priority_queue<speedInstant, deque<speedInstant>,
                                    greater<speedInstant> > &endInstants,
                      int &actSectId,
                      int &actPartNo,
                      int &actDir,
                      int &actFlow,
                      double &actSpeed,
                      MGPSecUnit &curMGPSec)
{
    actTStart = curMGPSec.GetTimeInterval().start;
    while (!endInstants.empty()) endInstants.pop();
    endInstants.push(speedInstant(curMGPSec.GetSpeed(),
                     curMGPSec.GetTimeInterval().end));
    actSectId = curMGPSec.GetSecId();
    actPartNo = curMGPSec.GetPart();
    actDir = curMGPSec.GetDirect();
    actFlow = 1;
    actSpeed = curMGPSec.GetSpeed();
};

/*
3.1.2 TrafficFlow

*/

void InitializeValues(Instant &actTStart,
                      priority_queue<Instant, deque<Instant>,
                      greater<Instant> > &endInstants,
                      int &actSectId,
                      int &actPartNo,
                      int &actDir,
                      int &actFlow,
                      MGPSecUnit &curMGPSec)
{
  actTStart = curMGPSec.GetTimeInterval().start;
  endInstants.push(curMGPSec.GetTimeInterval().end);
  actSectId = curMGPSec.GetSecId();
  actPartNo = curMGPSec.GetPart();
  actDir = curMGPSec.GetDirect();
  actFlow = 1;
};

/*
3.2 ~Write Tuple~

Writes extends relation ~rel~ by a tuple for the traffic flow value within a
section.

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

void WriteTupleSpeed(GenericRelation* rel, ListExpr relNumType, int actSectId,
                     int actPartNo, int actDir, MReal* avgSpeed, MInt* flow)
{
  Tuple *actTuple = new Tuple(nl->Second(relNumType));
  actTuple->PutAttribute(TRAFFICJAM_SECID,
                         new CcInt(true,actSectId));
  actTuple->PutAttribute(TRAFFICJAM_PARTNO,
                         new CcInt(true,actPartNo));
  actTuple->PutAttribute(TRAFFICJAM_DIR, new CcInt(true,actDir));
  actTuple->PutAttribute(TRAFFICJAM_SPEED, avgSpeed);
  actTuple->PutAttribute(TRAFFICJAM_FLOW, flow);
  rel->AppendTuple(actTuple);
  actTuple->DeleteIfAllowed();
};

/*
3.4 Write Traffic Relation

*/

int WriteTrafficRelation(GenericRelation *rel,
                         ListExpr relNumType,
                         priority_queue<MGPSecUnit, deque<MGPSecUnit>,
                                     greater<MGPSecUnit> > &mgpsecUnits)
{
  int actSectId = -1;
  int actPartNo = 0;
  int actDir = None;
  int actFlow = 0;
  double actSpeed = 0.0;
  double speed = 0.0;
  Instant actTStart(instanttype);
  Instant actTEnd(instanttype);
  MReal *avgSpeed = new MReal(0);
  MInt *flow = new MInt(0);
  priority_queue<speedInstant, deque<speedInstant>,
  greater<speedInstant> > endInstants;
  if(rel->GetNoTuples() > 0) rel->Clear();
  bool first = true;
  flow->StartBulkLoad();
  avgSpeed->StartBulkLoad();
  MGPSecUnit curMGPSec;
  while(!mgpsecUnits.empty())
  {
    curMGPSec = mgpsecUnits.top();
    mgpsecUnits.pop();
    if(first && curMGPSec.IsDefined() && curMGPSec.GetDirect() != None)
    {
      first = false;
      InitializeValues(actTStart, endInstants, actSectId, actPartNo, actDir,
                      actFlow, actSpeed, curMGPSec);
    }
    else
    {
      if (curMGPSec.IsDefined() && curMGPSec.GetDirect() != None)
      {
        if (actSectId == curMGPSec.GetSecId() &&
            actPartNo == curMGPSec.GetPart() &&
            actDir == curMGPSec.GetDirect())
        {
          endInstants.push(speedInstant(curMGPSec.GetSpeed(),
                          curMGPSec.GetTimeInterval().end));
          if (curMGPSec.GetTimeInterval().start > actTStart)
          {
            while (!endInstants.empty() &&
                    curMGPSec.GetTimeInterval().start > endInstants.top().inst)
            {
              if (actTStart != endInstants.top().inst)
              {
                flow->Add(UInt(Interval<Instant>(actTStart,
                          endInstants.top().inst,
                                          true, false),
                                          CcInt(true, actFlow)));
                avgSpeed->Add(UReal(Interval<Instant> (actTStart,
                              endInstants.top().inst,
                                              true, false),
                                              actSpeed,
                                              actSpeed));
              }
              actTStart = endInstants.top().inst;
              if ((actFlow - 1) > 0)
                actSpeed = (actSpeed * actFlow - endInstants.top().speed)/
                    (actFlow - 1);
              else
                actSpeed = 0.0;
              endInstants.pop();
              actFlow--;
            }
            if (!endInstants.empty() &&
                curMGPSec.GetTimeInterval().start == endInstants.top().inst)
            {
              if (actTStart != endInstants.top().inst)
              {
                avgSpeed->Add(UReal(Interval<Instant> (actTStart,
                              endInstants.top().inst,
                                              true, false),
                                              actSpeed,
                                              actSpeed));
                flow->Add(UInt(Interval<Instant>(actTStart,
                          endInstants.top().inst,
                                          true, false),
                                          CcInt(true, actFlow)));
              }
              if (actFlow > 0)
                actSpeed = (actSpeed * actFlow - endInstants.top().speed)/
                    actFlow;
              else
                actSpeed = 0.0;
              actTStart = endInstants.top().inst;
              endInstants.pop();
              while(!endInstants.empty() &&
                    curMGPSec.GetTimeInterval().start == endInstants.top().inst)
              {
                if ((actFlow - 1) > 0)
                  actSpeed = (actSpeed * actFlow - endInstants.top().speed)/
                      (actFlow - 1);
                else
                  actSpeed = 0.0;
                actFlow--;
                endInstants.pop();
              }
            }
            else
            {
              if (!endInstants.empty() &&
                  curMGPSec.GetTimeInterval().start < endInstants.top().inst)
              {
                if (actTStart != curMGPSec.GetTimeInterval().start)
                {
                  flow->Add(UInt(Interval<Instant> (actTStart,
                            curMGPSec.GetTimeInterval().start,
                                true,false),
                                CcInt(true, actFlow)));
                  avgSpeed->Add(UReal(Interval<Instant> (actTStart,
                                curMGPSec.GetTimeInterval().start,
                                    true,false),
                                    actSpeed,
                                    actSpeed));
                  actTStart = curMGPSec.GetTimeInterval().start;
                  actSpeed = (actSpeed * actFlow + curMGPSec.GetSpeed()) /
                      (actFlow + 1);
                  actFlow++;
                }
                else
                {
                  actSpeed = (actSpeed * actFlow + curMGPSec.GetSpeed()) /
                      (actFlow + 1);
                  actFlow++;
                }
              }
            }
          }
          else
          {
            if (curMGPSec.GetTimeInterval().start == actTStart)
            {
              actSpeed = (actSpeed * actFlow + curMGPSec.GetSpeed())/
                  (actFlow + 1);
              actFlow++;
            }
            else // shoult not happen stream not well sorted.
            {
              cerr << "Traffic stopped computation."
                  << " Stream is not sorted." << endl;
              flow->EndBulkLoad();
              avgSpeed->EndBulkLoad();
              WriteTupleSpeed(rel, relNumType, actSectId, actPartNo, actDir,
                              avgSpeed, flow);
              delete flow;
              delete avgSpeed;
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
            actTEnd = endInstants.top().inst;
            speed = endInstants.top().speed;
            endInstants.pop();
          }
          else
          {
            actTEnd == actTStart;
          }
          if (actTStart != actTEnd)
          {
            flow->Add(UInt(Interval<Instant>(actTStart, actTEnd, true, false),
                      CcInt(true, actFlow)));
            avgSpeed->Add(UReal(Interval<Instant>(actTStart, actTEnd,
                          true, false),
                          actSpeed,
                          actSpeed));
          }
          actTStart = actTEnd;
          if (actFlow - 1 > 0)
          {
            actSpeed = (actSpeed * actFlow - speed)/
                (actFlow - 1);
          }
          else
            actSpeed = 0.0;
          actFlow--;
          if (!endInstants.empty())
          {
            actTEnd = endInstants.top().inst;
            speed = endInstants.top().speed;
            endInstants.pop();
          }
          while (!endInstants.empty())
          {
            if (actTEnd < endInstants.top().inst)
            {
              if (actTStart != actTEnd)
              {
                flow->Add(UInt(Interval<Instant>(actTStart, actTEnd,
                          true, false),
                          CcInt(true, actFlow)));
                avgSpeed->Add(UReal(Interval<Instant>(actTStart, actTEnd,
                              true, false),
                              actSpeed,
                              actSpeed));
              }
              if (actFlow - 1 > 0)
              {
                actSpeed = (actSpeed * actFlow - speed)/
                    (actFlow - 1);
              }
              else
                actSpeed = 0.0;
              actFlow--;
              actTStart = actTEnd;
              actTEnd = endInstants.top().inst;
              speed = endInstants.top().speed;
              endInstants.pop();
            }
            else
            {
              if(actTEnd == endInstants.top().inst)
              {
                if (actFlow - 1 > 0)
                {
                  actSpeed = (actSpeed * actFlow - speed)/
                      (actFlow - 1);
                }
                else
                  actSpeed = 0.0;
                actFlow--;
                speed = endInstants.top().speed;
                endInstants.pop();
              }
              else //should never happen
              {
                cerr << "Traffic stopped computation."
                    << " Failure in endInstants queue." << endl;
                flow->EndBulkLoad();
                avgSpeed->EndBulkLoad();
                WriteTupleSpeed (rel, relNumType, actSectId, actPartNo, actDir,
                                avgSpeed, flow);
                delete avgSpeed;
                delete flow;
                while (!endInstants.empty()) endInstants.pop();
                while (!mgpsecUnits.empty()) mgpsecUnits.pop();
                return -1;
              }
            }
          }
          if (actTStart != actTEnd)
          {
            flow->Add(UInt(Interval<Instant> (actTStart, actTEnd, true, false),
                      CcInt(true, actFlow)));
            avgSpeed->Add(UReal(Interval<Instant>(actTStart, actTEnd,
                          true, false),
                          actSpeed,
                          actSpeed));
          }
          flow->EndBulkLoad();
          avgSpeed->EndBulkLoad();
          WriteTupleSpeed(rel, relNumType, actSectId, actPartNo, actDir,
                          avgSpeed, flow);
          InitializeValues(actTStart, endInstants, actSectId, actPartNo, actDir,
                          actFlow, actSpeed, curMGPSec);
          flow = new MInt(0);
          avgSpeed = new MReal(0);
          flow->StartBulkLoad();
          avgSpeed->StartBulkLoad();
        }
      }
    }
  }
  if (!endInstants.empty())
  {
    actTEnd = endInstants.top().inst;
    speed = endInstants.top().speed;
    endInstants.pop();
  }
  if (actTStart != actTEnd)
  {
    flow->Add(UInt(Interval<Instant>(actTStart, actTEnd, true, false),
              CcInt(true, actFlow)));
    avgSpeed->Add(UReal(Interval<Instant>(actTStart, actTEnd, true, false),
                  actSpeed,
                  actSpeed));
  }
  actTStart = actTEnd;
  if (actFlow - 1 > 0)
  {
    actSpeed = (actSpeed * actFlow - speed)/
        (actFlow - 1);
  }
  else
    actSpeed = 0.0;
  actFlow--;
  if (!endInstants.empty())
  {
    actTEnd = endInstants.top().inst;
    speed = endInstants.top().speed;
    endInstants.pop();
  }
  if (endInstants.empty() && actTStart != actTEnd)
  {
    flow->Add(UInt(Interval<Instant> (actTStart, actTEnd, true, false),
              CcInt(true,actFlow)));
    avgSpeed->Add(UReal(Interval<Instant> (actTStart, actTEnd, true, false),
                  actSpeed, actSpeed));
  }
  else
  {
    while (!endInstants.empty())
    {
      if (actTEnd < endInstants.top().inst)
      {
        if (actTStart != actTEnd)
        {
          flow->Add(UInt(Interval<Instant>(actTStart, actTEnd,
                    true, false),
                    CcInt(true, actFlow)));
          avgSpeed->Add(UReal(Interval<Instant> (actTStart, actTEnd,
                        true, false),
                        actSpeed, actSpeed));
        }
        if (actFlow - 1 > 0)
        {
          actSpeed = (actSpeed * actFlow - speed)/
              (actFlow - 1);
        }
        else
          actSpeed = 0.0;
        actFlow--;
        actTStart = actTEnd;
        actTEnd = endInstants.top().inst;
        speed = endInstants.top().speed;
        endInstants.pop();
      }
      else
      {
        if(actTEnd == endInstants.top().inst)
        {
          if (actFlow - 1 > 0)
          {
            actSpeed = (actSpeed * actFlow - speed)/
                (actFlow - 1);
          }
          else
            actSpeed = 0.0;
          actFlow--;
          speed = endInstants.top().speed;
          endInstants.pop();
          if (endInstants.empty() && actTStart != actTEnd)
          {
            flow->Add(UInt(Interval<Instant>(actTStart, actTEnd,
                      true, false),
                      CcInt(true, actFlow)));
            avgSpeed->Add(UReal(Interval<Instant> (actTStart, actTEnd,
                          true, false),
                          actSpeed, actSpeed));
          }
        }
        else //should never happen
        {
          cerr << "Traffic stopped computation."
              << " Failure in endInstants queue." << endl;
          flow->EndBulkLoad();
          avgSpeed->EndBulkLoad();
          WriteTupleSpeed(rel, relNumType, actSectId, actPartNo, actDir,
                          avgSpeed, flow);
          delete flow;
          delete avgSpeed;
          while (!endInstants.empty()) endInstants.pop();
          while (!mgpsecUnits.empty()) mgpsecUnits.pop();
          return -1;
        }
      }
    }
  }
  if (actTStart != actTEnd)
  {
    flow->Add(UInt(Interval<Instant> (actTStart, actTEnd, true, false),
              CcInt(true, actFlow)));
    avgSpeed->Add(UReal(Interval<Instant> (actTStart, actTEnd,
                  true, false),
                  actSpeed, actSpeed));
  }
  flow->EndBulkLoad();
  avgSpeed->EndBulkLoad();
  WriteTupleSpeed(rel, relNumType, actSectId, actPartNo, actDir, avgSpeed,
                  flow);
  return 0;
}

/*
3.5 Write Traffic Flow Relation

*/

int WriteTrafficFlowRelation(GenericRelation *rel,
                             ListExpr relNumType,
                             priority_queue<MGPSecUnit, deque<MGPSecUnit>,
                             greater<MGPSecUnit> > &mgpsecUnits)
{
  bool first = true;
  int actSectId = -1;
  int actPartNo = 0;
  int actDir = None;
  int actFlow = 0;
  Instant actTStart(instanttype);
  Instant actTEnd(instanttype);
  priority_queue<Instant, deque<Instant>,
  greater<Instant> > endInstants;
  MInt *flow = new MInt(0);
  flow->StartBulkLoad();
  MGPSecUnit curMGPSec;
  while(!mgpsecUnits.empty())
  {
    curMGPSec = mgpsecUnits.top();
    mgpsecUnits.pop();
    if(first && curMGPSec.IsDefined() && curMGPSec.GetDirect() != None)
    {
      first = false;
      InitializeValues(actTStart, endInstants, actSectId, actPartNo, actDir,
                          actFlow, curMGPSec);
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
          InitializeValues(actTStart, endInstants, actSectId, actPartNo,
                              actDir, actFlow, curMGPSec);
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
4 Operators

4.1 Traffic Flow estimation

4.1.1 ~trafficflow~

The operation gets a sorted stream of ~mgpsecunit~s and computes a corresponding
relation of each tuple containing a section number (~int~), a part number of the
section (~int), a direction (~int) and the flow value (number of cars per
time interval) for this before defined network part (~mint~).

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
      InitializeValues(actTStart, endInstants, actSectId, actPartNo, actDir,
                       actFlow, *actMGPSU);
      first = false;
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
          InitializeValues(actTStart, endInstants, actSectId, actPartNo,
                               actDir, actFlow, *actMGPSU);
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
4.1.2 trafficflow2

The operation gets a stream of ~mgpsecunit~s and computes a corresponding
relation of each tuple containing a section number (~int~), a part number of the
section (~int), a direction (~int) and the flow value (number of cars per time
interval) for this before defined network part (~mint~).

Type Mapping

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
  priority_queue<MGPSecUnit, deque<MGPSecUnit>,
                 greater<MGPSecUnit> > mgpsecUnits;
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
  return WriteTrafficFlowRelation(rel, relNumType, mgpsecUnits);
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
4.2 Traffic Jam

Type Mapping

*/

static string trafficRelationTypeInfo =
    "(rel (tuple ((secid int) (part int) (dir int) (speed mreal) "
                 "(flow mint))))";

ListExpr OpTrafficTypeMap(ListExpr in_xArgs)
{
  NList type(in_xArgs);
  if (type.length()==1)
  {
    NList st = type.first();
    NList attr("mgpsecunit");
    if (st.checkStream(attr))
    {
      ListExpr retList;
      nl->ReadFromString(trafficRelationTypeInfo, retList);
      return retList;
    }
  }
  return NList::typeError("Expected a stream of mgpsecunits.");
}

/*
Value Mapping

*/

int OpTrafficValueMap(Word* args, Word& result, int message,
                           Word& local, Supplier s)
{
  Word actual;
  GenericRelation* rel = (Relation*)((qp->ResultStorage(s)).addr);
  result.setAddr(rel);
  ListExpr relInfo;
  nl->ReadFromString(trafficRelationTypeInfo, relInfo);
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType(relInfo);
  priority_queue<MGPSecUnit, deque<MGPSecUnit>,
  greater<MGPSecUnit> > mgpsecUnits;
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
  return WriteTrafficRelation(rel, relNumType, mgpsecUnits);
}

/*
Operator Information for the user:

*/

struct trafficInfo : OperatorInfo {

  trafficInfo()
  {
    name      = "traffic";
    signature = "stream(mgpsecunit)->rel(int,int,int,mreal,mint)";
    syntax    = "_ traffic";
    meaning   = "Relation traffic from input stream.";
  }
};

/*
4.2.2 Traffic2

Type Mapping

*/

ListExpr OpTraffic2TypeMap(ListExpr in_xArgs)
{
  NList type(in_xArgs);
  if (type.length() == 2)
  {
    NList stream = type.first();
    NList mgp("mgpoint");
    NList partlength = type.second();
    if (stream.length() == 2 && stream.checkStream(mgp) &&
        partlength.isEqual("real"))
    {
      ListExpr retList;
      nl->ReadFromString(trafficRelationTypeInfo, retList);
      return retList;
    }
  }
  return NList::typeError( "Expected ((stream mgpoint) real).");
}

/*
Value Mapping

*/

int OpTraffic2ValueMap(Word* args, Word& result, int message,
                      Word& local, Supplier s)
{
  Word actual;
  GenericRelation* rel = (Relation*)((qp->ResultStorage(s)).addr);
  result.setAddr(rel);
  ListExpr relInfo;
  nl->ReadFromString(trafficRelationTypeInfo, relInfo);
  ListExpr relNumType = SecondoSystem::GetCatalog()->NumericType(relInfo);
  priority_queue<MGPSecUnit, deque<MGPSecUnit>,
  greater<MGPSecUnit> > mgpsecUnits;
  MGPoint *actMGP = 0;
  double maxLength = ((CcReal*)args[1].addr)->GetRealval();
  bool first = true;
  Network *pNetwork = 0;
  vector<MGPSecUnit> vmgpsecunit;
  vmgpsecunit.clear();
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  while (qp->Received(args[0].addr))
  {
    actMGP = (MGPoint*)actual.addr;
    if (first && actMGP->IsDefined())
    {
      pNetwork = actMGP->GetNetwork();
      first = false;
    }
    if (actMGP->IsDefined())
    {
      actMGP->GetMGPSecUnits(vmgpsecunit, maxLength, pNetwork);
      for (size_t i = 0 ; i < vmgpsecunit.size(); i++)
      {
        mgpsecUnits.push(MGPSecUnit(vmgpsecunit[i]));
      }
    }
    vmgpsecunit.clear();
    delete actMGP;
    qp->Request(args[0].addr, actual);
  }
  qp->Close(args[0].addr);
  NetworkManager::CloseNetwork(pNetwork);
  pNetwork = 0;
  return WriteTrafficRelation(rel, relNumType, mgpsecUnits);
}

/*
Operator Information for the user:

*/

struct traffic2Info : OperatorInfo {

  traffic2Info()
  {
    name      = "traffic2";
    signature = "stream(mgpoint)xreal->rel(int,int,int,mreal,mint)";
    syntax    = "_ traffic2[<maxsectionlength>]";
    meaning   = "Relation traffic from input relation or stream.";
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
    AddOperator(trafficflow2Info(), OpTrafficFlow2ValueMap,
                OpTrafficFlow2TypeMap);
    AddOperator(trafficInfo(), OpTrafficValueMap,
                OpTrafficTypeMap);
    AddOperator(traffic2Info(), OpTraffic2ValueMap, OpTraffic2TypeMap);
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
