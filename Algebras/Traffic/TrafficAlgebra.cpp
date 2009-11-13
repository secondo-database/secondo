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

 double MGPSecUnit::GetDurationInSeconds() const
{
  return (m_time.end - m_time.start).ToDouble()/0.00001157;
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

 MGPSecUnit& MGPSecUnit::operator=( const MGPSecUnit& in_xOther )
{
  m_secId = in_xOther.GetSecId();
  m_part = in_xOther.GetPart();
  m_direct = in_xOther.GetDirect();
  m_speed = in_xOther.GetSpeed();
  m_time = in_xOther.GetTimeInterval();
  return *this;
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
  if (!IsDefined() && !p->IsDefined()) return 0;
  if (!IsDefined() && p->IsDefined()) return -1;
  if (IsDefined() && !p->IsDefined()) return 1;
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
      << ", Timeinterval: " ;
      m_time.Print(os);
  os << endl;
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

void* MGPSecUnit::Cast(void* addr)
{
  return new (addr) MGPSecUnit;
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
  cast = MGPSecUnit::Cast;
  }
};

struct mgpsecInfo:ConstructorInfo
{
  mgpsecInfo()
  {
    name = "mgpsecunit";
    signature = "-> DATA";
    typeExample = "mgpsecunit";
    listRep = "(<secId><part><dir><speed>(<timeinterval>))";
    valueExample = "(15 1 1 3.5 <timeinterval>)";
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
        && (!(nl->IsAtom(attr) && nl->AtomType(attr) != SymbolType))
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
      vmgpsecunit.clear();
      pos = 0;
      pNetwork = 0;
      iterRel = 0;
      attrIndex = 0;
      maxSectLength = numeric_limits<double>::max();
    }

    vector<MGPSecUnit> vmgpsecunit; // vector mit mgpsecunits
    size_t pos; //position im Vector
    Network *pNetwork; //networkobject
    GenericRelationIterator *iterRel; //pointer to actual tuple of rel
    int attrIndex; //attribute index of mgpoint attribut in rel
    double maxSectLength; //maximum section part length
  };

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
      GenericRelation *rel = (GenericRelation*) args[0].addr;
      li->pNetwork = (Network*) args[2].addr;
      li->maxSectLength = ((CcReal*) args[3].addr)->GetRealval();
      li->attrIndex = ((CcInt*)args[4].addr)->GetIntval()-1;
      li->iterRel = rel->MakeScan();
      local.addr = li;
      return 0;
    }

    case REQUEST:
    {
      result = qp->ResultStorage(s);
      if (local.addr)
        li = (OpMgp2mgpsecLocalInfo*) local.addr;
      else return CANCEL;
      if (!li->vmgpsecunit.empty() && li->pos < li->vmgpsecunit.size())
      {
        result = SetWord(new MGPSecUnit(li->vmgpsecunit[li->pos++]));
        return YIELD;
      }
      else
      {
        li->vmgpsecunit.clear();
        Tuple *actTuple = li->iterRel->GetNextTuple() ;
        if (actTuple != 0)
        {
          MGPoint *m = (MGPoint*) actTuple->GetAttribute(li->attrIndex);
          if (m != 0)
          {
            m->GetMGPSecUnits(li->vmgpsecunit, li->maxSectLength, li->pNetwork);
            if (li->vmgpsecunit.size() > 0)
            {
              li->pos = 0;
              result = SetWord(new MGPSecUnit(li->vmgpsecunit[li->pos++]));
              actTuple->DeleteIfAllowed();
              return YIELD;
            }
          }
          actTuple->DeleteIfAllowed();
        }
      }
      return CANCEL;
    }

    case CLOSE:
    {
      if (local.addr)
      {
        li = (OpMgp2mgpsecLocalInfo*) local.addr;
        li->pNetwork = 0;
        if (li->iterRel)
          delete li->iterRel;
        li->iterRel = 0;
        li->vmgpsecunit.clear();
        delete li;
        li = 0;
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
    signature = "rel x attr x net x real->stream(mgpsecunit)";
    syntax    = "_ mgp2mgpsecunits[_,_,_]";
    meaning   = "Builds a stream of mgpsecunits from mgpoint.";
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
