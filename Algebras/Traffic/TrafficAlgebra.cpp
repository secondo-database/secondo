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
2. Implementation of Class ~MGPSecUnit~

*/
MGPSecUnit::MGPSecUnit():Attribute()
{
}

MGPSecUnit::MGPSecUnit(int secId, int part, Side direct, double sp,
                       Interval<Instant> timeInterval):
              m_secId(secId),
              m_part(part),
              m_direct(direct),
              m_speed(sp),
              m_time(timeInterval)
{
  del.refs=1;
  del.isDelete=true;
}

MGPSecUnit::MGPSecUnit( const MGPSecUnit& in_xOther):
                        m_secId(in_xOther.GetSecId()),
                        m_part(in_xOther.GetPart()),
                        m_direct(in_xOther.GetDirect()),
                        m_speed(in_xOther.GetSpeed()),
                        m_time(in_xOther.GetTimeInterval())
{
  del.refs=1;
  del.isDelete=true;
}

MGPSecUnit::~MGPSecUnit() {}

int MGPSecUnit::GetSecId() const
{
  return m_secId;
}

int MGPSecUnit::GetPart() const
{
  return m_part;
}

Side MGPSecUnit::GetDirect() const
{
  return m_direct;
}

double MGPSecUnit::GetSpeed() const
{
  return m_speed;
}

Interval<Instant> MGPSecUnit::GetTimeInterval() const
{
  return m_time;
}

void MGPSecUnit::SetSecId(int secId)
{
  m_secId = secId;
}

void MGPSecUnit::SetPart(int p)
{
  m_part = p;
}

void MGPSecUnit::SetDirect(Side dir)
{
  m_direct = dir;
}

void MGPSecUnit::SetSpeed(double x)
{
  m_speed = x;
}

void MGPSecUnit::SetTimeInterval(Interval<Instant> time)
{
  m_time = time;
}

size_t MGPSecUnit::Sizeof() const
{
  return sizeof(MGPSecUnit);
}

size_t MGPSecUnit::HashValue() const
{
  size_t hash = m_secId + m_part + (int) m_direct + (int) m_speed +
                (int) m_time.start.ToDouble() +
                (int) m_time.end.ToDouble();
  return hash;
}

void MGPSecUnit::CopyFrom( const Attribute* right )
{
  const MGPSecUnit* gp = (const MGPSecUnit*)right;
  *this = *gp;
}

int MGPSecUnit::Compare( const Attribute* arg ) const
{
  const MGPSecUnit *p = (const MGPSecUnit*) arg;
  if (m_secId < p->GetSecId()) return -1;
  else
    if (m_secId > p->GetSecId()) return 1;
    else
      if (m_part < p->GetPart()) return -1;
      else
        if (m_part > p->GetPart()) return 1;
        else
          if (m_direct < p->GetDirect()) return -1;
          else
            if (m_direct > p->GetDirect()) return 1;
            else
              if (m_time.start < p->GetTimeInterval().start) return -1;
              else
                if (m_time.start > p->GetTimeInterval().start) return 1;
                else
                  if (m_time.end < p->GetTimeInterval().end) return -1;
                  else
                    if (m_time.end > p->GetTimeInterval().end) return 1;
                    else
                      if (m_speed < p->GetSpeed()) return -1;
                      else
                        if (m_speed > p->GetSpeed()) return 1;
                        else return 0;
}

bool MGPSecUnit::Adjacent( const Attribute *arg ) const
{
  return false;
}

MGPSecUnit* MGPSecUnit::Clone() const
{
  return new MGPSecUnit( *this );
}

ostream& MGPSecUnit::Print( ostream& os ) const
{
  os << "MGPSecUnit: " << m_secId
      << ", Part: " << m_part
      << ", Side: " << m_direct
      << ", Speed: " << m_speed
      << ", Timeinterval: " << m_time.Print(os) << endl;
  return os;
}


ListExpr MGPSecUnit::Out(ListExpr typeInfo, Word value)
{
  MGPSecUnit* msec = static_cast<MGPSecUnit*> (value.addr);
  if (msec->IsDefined())
    return nl->FiveElemList(nl->IntAtom(msec->GetSecId()),
                            nl->IntAtom(msec->GetPart()),
                            nl->IntAtom(msec->GetDirect()),
                            nl->RealAtom(msec->GetSpeed()),
                            nl->FourElemList(OutDateTime(nl->TheEmptyList(),
                                                 SetWord(&msec->m_time.start)),
                                              OutDateTime(nl->TheEmptyList(),
                                                  SetWord(&msec->m_time.end)),
                                              nl->BoolAtom(msec->m_time.lc),
                                              nl->BoolAtom(msec->m_time.rc)));
  else return nl->SymbolAtom("undef");
}

Word MGPSecUnit::In(const ListExpr typeInfo, const ListExpr instance,
                    const int errorPos, ListExpr& errorInfo, bool& correct)
{
  NList  list(instance);
  if (list.length() == 5)
  {
    NList seclist = list.first();
    NList partlist = list.second();
    NList dirlist = list.third();
    NList speedlist = list.fourth();
    if (seclist.isInt() && dirlist.isInt()
        && partlist.isInt() && speedlist.isReal())
    {
      NList timelist = list.fifth();
      if (timelist.length() == 4)
      {
        NList stinst = timelist.first();
        NList einst = timelist.second();
        NList lclist = timelist.third();
        NList rclist = timelist.fourth();
        if (lclist.isBool() && rclist.isBool())
        {
          correct = true;
          Instant *start = (Instant*)InInstant(nl->TheEmptyList(),
                                                stinst.listExpr(),
                                                errorPos,
                                                errorInfo,
                                                correct).addr;
          if(correct)
          {
            Instant *end = (Instant*)InInstant(nl->TheEmptyList(),
                              einst.listExpr(),
                              errorPos,
                              errorInfo,
                              correct).addr;
            if (correct)
            {
              Word w = new MGPSecUnit(seclist.intval(), partlist.intval(),
                                      (Side) dirlist.intval(),
                                      nl->RealValue(speedlist.listExpr()),
                                      Interval<Instant> (*start, *end,
                                                        lclist.boolval(),
                                                        rclist.boolval()));
              return w;
            }
          }
        }
      }
    }
  }
  errorInfo = nl->Append(errorInfo, nl->StringAtom(
                         "Expected <int><int><int><real><timeinterval>."));
  correct = false;
  return SetWord(Address(0));
}

bool MGPSecUnit::CheckKind( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "mgpsecunit" ));
}

int MGPSecUnit::NumOfFLOBs()const
{
  return 0;
}

FLOB* MGPSecUnit::GetFLOB(const int i)
{
  return 0;
}

/*
3 Type Constructor for ~mgpsecunit~

*/
struct mgpsecFunctions:ConstructorFunctions<MGPSecUnit>
{
  mgpsecFunctions()
  {
  in = MGPSecUnit::In;
  out = MGPSecUnit::Out;
  kindCheck = MGPSecUnit::CheckKind;
  }
};

struct mgpsecInfo:ConstructorInfo
{
  mgpsecInfo()
  {
    name = "mgpsecunit";
    signature = "-> DATA";
    typeExample = "mgpsecunit";
    listRep = "(<secId><part><direction><speed>(<timeinterval>))";
    valueExample = "(15 1 1 3.5 (\"2000-01-01\" \"2000-01-02\" TRUE FALSE))";
    remarks = "direction:down=0,up=1,none=2. Speed: m/s";
  }
};

mgpsecInfo mgpinfo;
mgpsecFunctions mgpfunct;
TypeConstructor mgpsecunitTC(mgpinfo, mgpfunct);

/*
4 Operators of the Traffic Algebra

4.1 ~mgp2mgpsecunit~

The operation ~mgp2mgpsecunit~ gets a network and a maximum section
length and a stream of ~mgpoint~. With this values it computes the

TypeMapping:

*/

ListExpr OpMgp2mgpsecunitsTypeMap(ListExpr in_xArgs)
{
  NList type(in_xArgs);
  if (type.length() == 4)
  {
    ListExpr rel = type.first().listExpr();
    ListExpr attr = type.second().listExpr();
    NList net = type.third();
    NList length = type.fourth();
    if (net.isEqual("network") && length.isEqual("real")
        && (!nl->IsAtom(attr) && nl->AtomType(attr) != SymbolType)
        && IsRelDescription(rel))
    {
      string attrname = nl->SymbolValue(attr);
      ListExpr attrtype;
      ListExpr tupleDescr = nl->Second(rel);
      int j=listutils::findAttribute(nl->Second(tupleDescr),attrname, attrtype);
      if (j!=0 && nl->IsEqual(attrtype, "mgpoint"))
      {
        return nl->ThreeElemList(nl->SymbolAtom("APPEND"),
                                 nl->OneElemList(nl->IntAtom(j)),
                                 nl->TwoElemList(
                                    nl->SymbolAtom(STREAM),
                                    nl->SymbolAtom("mgpsecunit")));
      }
    }
  }
  return NList::typeError(
      "Expected <rel(tuple(..ai xi..)><ai><network><real> with xi=mgpoint.");
}

/*
Auxilliary Functions

*/

struct OpMgp2mgpsecLocalInfo
  {
    OpMgp2mgpsecLocalInfo()
    {
      m = 0;
      pNetwork = 0;
      rel = 0;
      iterRel = 0;
      attrIndex = 0;
      unitIndex = 0;
      maxSectLength = 0.0;
      curSecId = -1;
      curPart = 0;
      curDir = None;
      curLength = 0.0;
      curTimeSeconds = 0.0;
      curStartTime = Instant(instanttype);
      curEndTime = curStartTime;
      curSecTid = -1;
      lastUnitParted = false;
      curSectStartPos = 0.0;
      curSectLength = 0.0;
      passedSections.clear();
      curPassedSectionsPointer = 0;
      lastUnitDifferentSections = false;
    }

    MGPoint *m; //current mgpoint
    Network *pNetwork; //networkobject
    Relation *rel; //input relation
    GenericRelationIterator *iterRel; //pointer to actual tuple of rel
    int attrIndex; //attribute index of mgpoint attribut in rel
    int unitIndex; //current unit of current mgpoint
    double maxSectLength; //maximum section part length
    int curSecId; //current section id
    int curPart; //identifier number of current section part (starting with 1)
    Side curDir; //moving direction of mgpoint respectively side of section
    double curLength; //distance on the section passed by the current mgpsecunit
                      // in meter
    double curTimeSeconds;//time in seconds used by mgpoint to passe ~curLength~
    Instant curStartTime;//starttime of current mgpsecunit
    Instant curEndTime;//endtime of current mgpsecunit
    TupleId curSecTid;//tupleId of current mgpsecunit
    bool lastUnitParted;//flag for units parted by parted sections
    double curSectStartPos;//reminder of section start value
    double curSectLength; //length of current section
    vector<TupleId> passedSections;//Contains the tuple identifiers of the
                                   //sections passed within a single ugpoint
    size_t curPassedSectionsPointer;//Pointer to current activ element of passed
                                    //sections
    bool lastUnitDifferentSections;//flag for units parted by different sections
  };

void ResetLiNewMGP (OpMgp2mgpsecLocalInfo* li)
{
  li->unitIndex = 0;
  li->curSecId = -1;
  li->curPart = -1;
  li->curDir = None;
  li->curLength = 0.0;
  li->curTimeSeconds = 0.0;
  li->curStartTime = Instant(instanttype);
  li->curEndTime = li->curStartTime;
  li->lastUnitParted = false;
  li->curSectStartPos = 0.0;
  li->curSectLength = 0.0;
  li->passedSections.clear();
  li->curPassedSectionsPointer = 0;
  li->lastUnitDifferentSections = false;
}

bool GetNextMGPoint(OpMgp2mgpsecLocalInfo* li)
{
  Tuple *tup = li->iterRel->GetNextTuple();
  if (!tup) return false;
  delete li->m;
  li->m = (MGPoint*) tup->GetAttribute(li->attrIndex);
  tup->DeleteIfAllowed();
  ResetLiNewMGP(li);
  return true;
}

/*
Value Mapping

*/

int OpMgp2mgpsecunitsValueMap(Word* args, Word& result, int message,
                              Word& local, Supplier s)
{
  OpMgp2mgpsecLocalInfo* li = 0;
  switch( message )
  {
    case OPEN:
    {
      li = new OpMgp2mgpsecLocalInfo();
      li->rel = (Relation*) args[0].addr;
      li->pNetwork = (Network*) args[2].addr;
      li->maxSectLength = ((CcReal*) args[3].addr)->GetRealval();
      li->attrIndex = ((CcInt*)args[4].addr)->GetIntval()-1;
      li->iterRel = li->rel->MakeScan();
      local.addr = li;
      return 0;
    }

    case REQUEST:
    {
      MGPSecUnit* res = (MGPSecUnit*) ((qp->ResultStorage(s)).addr);
      result = SetWord(res);
      if (local.addr)
        li = (OpMgp2mgpsecLocalInfo*) local.addr;
      else return CANCEL;
      const UGPoint *unit;
      //Check first if we have a rest unit from the last unit.
      if (li->lastUnitParted)
      {
        //if this is the case it must have been a moving unit
        li->m->Get(li->unitIndex, unit);
        if(li->curDir == Up)
        {
          //moving up
          //look if the end point is in the actual part or not
          if (unit->p1.GetPosition() <=
              li->curSectStartPos + li->curPart * li->maxSectLength)
          {
            //unit endpoint on actual section part
            //conclude working of this unit. And continue with next unit
            li->curLength = unit->p1.GetPosition() - li->curSectStartPos
                           + (li->curPart-1) * li->maxSectLength;
            li->unitIndex++;
            li->lastUnitParted = false;
            if (li->unitIndex >= li->m->GetNoComponents())
            {
              //end of mgpoint reached write last mgpsecunit delete mgpoint
              //and reset values
              *res = MGPSecUnit(li->curSecId, li->curPart, li->curDir,
                                li->curLength / li->curTimeSeconds,
                                Interval<Instant> (li->curStartTime,
                                                   li->curEndTime,
                                                   true,false));
              delete li->m;
              ResetLiNewMGP(li);
              return YIELD;
            } //else continue with next unit
          }
          else
          {
            //unit endpoint not on actual section part
            //part the rest of the unit in the part on this section part and
            //the part not on this section part and write the actual mgpsecunit
            //to the result.
            li->curLength = li->maxSectLength;
            li->curEndTime = unit->TimeAtPos(li->curSectStartPos
                                              + li->curPart *li->maxSectLength);
            li->curTimeSeconds = (li->curEndTime - li->curStartTime).ToDouble()
                                  / 0.00001157;
            *res = MGPSecUnit(li->curSecId, li->curPart, li->curDir,
                              li->curLength / li->curTimeSeconds,
                              Interval<Instant> (li->curStartTime,
                                                 li->curEndTime,
                                                 true,false));
            //save intermediate results and continue working on this unit at
            //the next system call.
            li->curStartTime = li->curEndTime;
            li->curEndTime = unit->timeInterval.end;
            li->curPart++;
            li->curLength = 0.0;
            li->curTimeSeconds =
                (li->curEndTime - li->curStartTime).ToDouble() /
                0.00001157;
            return YIELD;
          }
        }
        else
        {
          //moving down
          //almost equal to moving up but with looking for the parts downwards
          //instead of upwards.
          if (unit->p1.GetPosition() >= li->curSectStartPos +
                                        (li->curPart - 1) * li->maxSectLength)
          {
            //unit endpoint on actual section part
            li->curLength = li->curSectStartPos +
                      li->curPart * li->maxSectLength - unit->p1.GetPosition();
            li->unitIndex++;
            li->lastUnitParted = false;
            if (li->unitIndex >= li->m->GetNoComponents())
            {
              //end of mgpoint reached write last mgpsecunit delete mgpoint
              //and reset values
              *res = MGPSecUnit(li->curSecId, li->curPart, li->curDir,
                                li->curLength / li->curTimeSeconds,
                                Interval<Instant> (li->curStartTime,
                                                   li->curEndTime,
                                                   true,false));
              delete li->m;
              ResetLiNewMGP(li);
              return YIELD;
            } //else continue with next unit
          }
          else
          {
            //unit endpoint not on actual section part
            li->curLength = li->maxSectLength;
            li->curEndTime = unit->TimeAtPos(li->curSectStartPos +
                                          (li->curPart-1) * li->maxSectLength);
            li->curTimeSeconds = (li->curEndTime - li->curStartTime).ToDouble()
                                  / 0.00001157;
            *res = MGPSecUnit(li->curSecId, li->curPart, li->curDir,
                              li->curLength / li->curTimeSeconds,
                              Interval<Instant> (li->curStartTime,
                                                 li->curEndTime,
                                                 true,false));
            li->curStartTime = li->curEndTime;
            li->curEndTime = unit->timeInterval.end;
            li->curPart--;
            li->curLength = 0.0;
            li->curTimeSeconds =
                (li->curEndTime - li->curStartTime).ToDouble() /
                0.00001157;
            return YIELD;
          }
        }
      }
      //Check if the actual mgpoint is well defined and not transformed
      //completely. If this is not the case get the next well defined mgpoint
      //if possible.
      while (!li->m || !li->m->IsDefined()
              || li->unitIndex >= li->m->GetNoComponents())
        if (!GetNextMGPoint(li)) return CANCEL;
      while (li->unitIndex < li->m->GetNoComponents())
      {
        //get next unit of actual mgpoint if defined and compute the passed
        //sections within this unit.
        li->m->Get(li->unitIndex, unit);
        li->passedSections.clear();
        unit->GetPassedSections(li->pNetwork, li->passedSections);
        if (li->passedSections.size() > 1)
        {
          // unit must be parted to the passed sections.
          li->lastUnitDifferentSections = true;
          li->curPassedSectionsPointer = 0;
          TupleId unitSecTid = li->passedSections[li->curPassedSectionsPointer];
//to be continued



        }
        else
        {
          // whole movement in one section
          TupleId unitSecTid = li->passedSections[0];
          li->passedSections.clear();
          if (li->curSecTid == unitSecTid )
          {
            //mgpoint stays the on same section it has been before
            if (li->curSectLength <= li->maxSectLength)
            {
              //section is not parted
              if (li->curDir == unit->MovingDirection() ||
                  unit->MovingDirection() == None ||
                  li->curDir == None)
              {
                //MovingDirection moves in the same direction than before
                //or stops respectively comes from a stop. We can expand the
                //actual mgpsecunit with the values of this unit and get the
                //next unit to continue computing the current mgpsecunit
                if (li->curDir == None) li->curDir = unit->MovingDirection();
                li->curLength += unit->Length();
                li->curTimeSeconds += unit->DurationSeconds();
                li->curEndTime = unit->GetUnitEndTime();
                li->unitIndex++;
              }
              else
              {
                //change of moving direction
                //write the current mgpsecunit
                //and initialize a new mgpsecunit with values of the actual
                //ugpoint
                //Return mgpsecunit
                *res = MGPSecUnit(li->curSecId, li->curPart, li->curDir,
                                  li->curLength / li->curTimeSeconds,
                                  Interval<Instant> (li->curStartTime,
                                      li->curEndTime,
                                      true,false));
                li->curDir = unit->MovingDirection();
                li->curLength = unit->Length();
                li->curTimeSeconds = unit->DurationSeconds();
                li->curStartTime = unit->timeInterval.start;
                li->curEndTime = unit->timeInterval.end;
                return YIELD;
              }
            }
            else
            {
              //section is parted
              //check actual part for unit start
              if(!(li->curPart-1)*li->maxSectLength <= unit->p0.GetPosition()
                && unit->p0.GetPosition() <= li->curPart * li->maxSectLength)
              {
                //unit start is not in actual part.
                //write old mgpsecunit and initalize vakues for new one.
                *res = MGPSecUnit(li->curSecId, li->curPart, li->curDir,
                                  li->curLength / li->curTimeSeconds,
                                  Interval<Instant> (li->curStartTime,
                                      li->curEndTime,
                                      true,false));
                //find section part where the unit starts.
                int i = 0;
                while (unit->p0.GetPosition() > li->curSectStartPos +
                        i * li->maxSectLength)
                  i++;
                li->curPart = i;
                li->curDir = unit->MovingDirection();
                li->curLength = unit->Length();
                li->curTimeSeconds = unit->DurationSeconds();
                li->curStartTime = unit->timeInterval.start;
                li->curEndTime = unit->timeInterval.end;
                li->lastUnitParted = true;
                return YIELD;
              }
              else
              {
                //unit starts in actual section part.
                //check end of unit to be in the same section part
                if (li->curSectStartPos +
                     (li->curPart - 1) * li->maxSectLength
                    <= unit->p1.GetPosition() &&
                    unit->p1.GetPosition() <= li->curSectStartPos +
                    li->curPart * li->maxSectLength)
                {
                  //unit end in same section part
                  if (li->curDir == unit->MovingDirection() ||
                      unit->MovingDirection() == None ||
                      li->curDir == None)
                  {
                    //MovingDirection moves in the same direction than before
                    //or stops respectively comes from a stop. We can expand the
                    //actual mgpsecunit with the values of this unit and get the
                    //next unit to continue computing the current mgpsecunit
                    if (li->curDir == None) li->curDir =unit->MovingDirection();
                    li->curLength += unit->Length();
                    li->curTimeSeconds += unit->DurationSeconds();
                    li->curEndTime = unit->GetUnitEndTime();
                    li->unitIndex++;
                  }
                  else
                  {
                    //change of moving direction
                    //write the current mgpsecunit
                    //and initialize a new mgpsecunit with values of the actual
                    //ugpoint
                    //Return mgpsecunit
                    *res = MGPSecUnit(li->curSecId, li->curPart, li->curDir,
                                      li->curLength / li->curTimeSeconds,
                                      Interval<Instant> (li->curStartTime,
                                          li->curEndTime,
                                          true,false));
                    li->curDir = unit->MovingDirection();
                    li->curLength = unit->Length();
                    li->curTimeSeconds = unit->DurationSeconds();
                    li->curStartTime = unit->timeInterval.start;
                    li->curEndTime = unit->timeInterval.end;
                    return YIELD;
                  }
                }
                else
                {
                  //unit end in other section part
                  //split unit into more than one mgpsecunit.
                  //first mgpsecunit must be returned
                  //and the values for the next mgpsecunit must be initialized
                  li->lastUnitParted = true;
                  if (li->curDir == unit->MovingDirection() ||
                      unit->MovingDirection() == None ||
                      li->curDir == None)
                  {
                    //moving direction stays same
                    if (li->curDir == None) li->curDir =unit->MovingDirection();
                    if(li->curDir == Up)
                    {

                    }
                    else
                    {

                    }
                  }
                  else
                  {
                    //moving direction changes
                  }

                  return YIELD;
                }
              }
            }
          }
          else
          {
            //mgpoint changed section or first unit of mgpoint
            if (li->curSecTid == -1)
            {
              //first unit of mgpoint
              //set initial mgpsectunit values
              li->curTimeSeconds = unit->DurationSeconds();
              li->curStartTime = unit->GetUnitStartTime();
              li->curEndTime = unit->GetUnitEndTime();
              li->curSecTid = unitSecTid;
              li->curDir = unit->MovingDirection();
              Tuple *pSect = li->pNetwork->GetSection(li->curSecTid);
              double secMeas1 = ((CcReal*)
                    pSect->GetAttribute(SECTION_MEAS1))->GetRealval();
              double secMeas2 = ((CcReal*)
                    pSect->GetAttribute(SECTION_MEAS2))->GetRealval();
              li->curSectLength = fabs(secMeas2 -secMeas1);
              li->curSectStartPos = secMeas1;
              li->curSecId =
                  ((CcInt*)pSect->GetAttribute(SECTION_SID))->GetIntval();
              pSect->DeleteIfAllowed();
              if (li->curSectLength > li->maxSectLength)
              {
                //section is parted
                // find section part of start point
                li->curSectStartPos = secMeas1;
                int i = 1;
                while (unit->p0.GetPosition() >
                            (secMeas1 + i * li->maxSectLength))
                  i++;
                li->curPart = i;
                if (li->curDir == None)
                {
                    //mgpoint not moving
                  li->curLength = unit->Length();
                  li->unitIndex++;
                }
                else
                {
                  if(li->curDir == Up)
                  {
                    //mgpoint moves upwards
                    if (unit->p1.GetPosition() <=
                          secMeas1 + i*li->maxSectLength)
                    {
                      //unit endpoint on same section part
                      li->curLength = unit->Length();
                      li->unitIndex++;
                    }
                    else
                    {
                      //unit endpoint not on same section part
                      //unit must be parted into different mgpsecunits
                      //first mgpsecunit must be returned
                      //and the values for the next must be initialized
                      li->lastUnitParted = true;
                      li->curLength = secMeas1 + i*li->maxSectLength -
                                          unit->p0.GetPosition();
                      li->curEndTime =
                          unit->TimeAtPos(secMeas1 + i * li->maxSectLength);
                      li->curTimeSeconds =
                          (li->curEndTime - li->curStartTime).ToDouble() /
                              0.00001157;
                      *res = MGPSecUnit(li->curSecId, li->curPart,li->curDir,
                                        li->curLength / li->curTimeSeconds,
                                        Interval<Instant> (li->curStartTime,
                                            li->curEndTime,
                                            true,false));
                      li->curPart++;
                      li->curStartTime = li->curEndTime;
                      li->curEndTime = unit->timeInterval.end;
                      li->curLength = 0.0;
                      li->curTimeSeconds =
                          (li->curEndTime - li->curStartTime).ToDouble() /
                              0.00001157;
                      return YIELD;
                    }
                  }
                  else
                  {
                    //mgpoint moves downwards
                    if (unit->p1.GetPosition() >=
                        secMeas1 + (i-1)*li->maxSectLength)
                    {
                      //unit endpoint on same section part
                      li->curLength = unit->Length();
                      li->unitIndex++;
                    }
                    else
                    {
                      //unit endpoint not on same section part
                      //unit must be parted into different mgpsecunits
                      //first mgpsecunit is to be returned and the values must
                      //be saved for further computation.
                      li->lastUnitParted = true;
                      li->curLength = unit->p0.GetPosition() -
                                            secMeas1 + (i-1)*li->maxSectLength;
                      li->curEndTime =
                          unit->TimeAtPos(secMeas1 + (i-1) * li->maxSectLength);
                      li->curTimeSeconds =
                          (li->curEndTime - li->curStartTime).ToDouble() /
                          0.00001157;
                      *res = MGPSecUnit(li->curSecId, li->curPart,li->curDir,
                                        li->curLength / li->curTimeSeconds,
                                        Interval<Instant> (li->curStartTime,
                                            li->curEndTime,
                                            true,false));
                      li->curPart--;
                      li->curStartTime = li->curEndTime;
                      li->curEndTime = unit->timeInterval.end;
                      li->curLength = 0.0;
                      li->curTimeSeconds =
                          (li->curEndTime - li->curStartTime).ToDouble() /
                          0.00001157;
                      return YIELD;
                    }
                  }
                }
              }
              else
              {
                //section not parted
                li->curPart = 1;
                li->curLength = unit->Length();
                li->unitIndex++;
              }
            }
            else
            {
              //mgpoint changed section
              //write actual mgpsecunit and initialize new one with the unit
              //values
              *res = MGPSecUnit(li->curSecId, li->curPart,li->curDir,
                                li->curLength / li->curTimeSeconds,
                                Interval<Instant> (li->curStartTime,
                                    li->curEndTime,
                                    true,false));
              li->curTimeSeconds = unit->DurationSeconds();
              li->curStartTime = unit->GetUnitStartTime();
              li->curEndTime = unit->GetUnitEndTime();
              li->curSecTid = unitSecTid;
              li->curDir = unit->MovingDirection();
              Tuple *pSect = li->pNetwork->GetSection(li->curSecTid);
              double secMeas1 = ((CcReal*)
                    pSect->GetAttribute(SECTION_MEAS1))->GetRealval();
              double secMeas2 = ((CcReal*)
                    pSect->GetAttribute(SECTION_MEAS2))->GetRealval();
              li->curSectLength = fabs(secMeas2 -secMeas1);
              li->curSecId =
                  ((CcInt*)pSect->GetAttribute(SECTION_SID))->GetIntval();
              pSect->DeleteIfAllowed();
              if (li->curSectLength > li->maxSectLength)
              {
                //section is parted
                // find section part of start point
                li->curSectStartPos = secMeas1;
                int i = 1;
                while (unit->p0.GetPosition() >
                       (secMeas1 + i * li->maxSectLength))
                  i++;
                li->curPart = i;
              }
              else
                li->curPart = 1;
              return YIELD;
            }
          }
        }
      }
      //end of mgpoint write last ~mgpsecunit~ and get new ~mgpoint~
      if (li->curStartTime != li->curEndTime)
        *res = MGPSecUnit(li->curSecId, li->curPart,li->curDir,
                        li->curLength / li->curTimeSeconds,
                        Interval<Instant> (li->curStartTime,
                                            li->curEndTime,
                                            true,false));
      else
        *res = MGPSecUnit(li->curSecId, li->curPart, li->curDir,
                        li->curLength / li->curTimeSeconds,
                        Interval<Instant> (li->curStartTime,
                                            li->curEndTime,
                                            true,true));
      delete li->m;
      ResetLiNewMGP(li);
      return YIELD;
    }

    case CLOSE:
    {
      if (local.addr)
      {
        li = (OpMgp2mgpsecLocalInfo*) local.addr;
        if (li->m) delete li->m;
        li->m = 0;
        li->pNetwork = 0; //network
        li->rel = 0; //relation
        li->iterRel = 0;
        delete li;
        local.addr = 0;
      }
      return 0;
    }
    default:
    {
      // should not happen
      return -1;
    }
  }
}


struct mgp2mgpsecunitsInfo : OperatorInfo {

  mgp2mgpsecunitsInfo()
  {
    name      = "mgp2mgpsecunits";
    signature = "rel(tuple(a1x1...anxn))xaixnetworkxreal->stream(mgpsecunit)";
    syntax    = "_ mgp2mgpsecunits[_,_,_]";
    meaning   = "Returns the mgpoints of rel as stream of mgpsecunits.";
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
    AddTypeConstructor( &mgpsecunitTC);

    mgpsecunitTC.AssociateKind( "DATA" );

    AddOperator(mgp2mgpsecunitsInfo(), OpMgp2mgpsecunitsValueMap,
                OpMgp2mgpsecunitsTypeMap);

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
