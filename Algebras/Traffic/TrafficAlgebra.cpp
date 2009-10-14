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

1 Implementation of TrafficEsitmation for Traffic Networks

October 2009 Simone Jandt

1.1 Defines, includes, and constants

*/

#include "StandardTypes.h"
#include "DateTime.h"
#include "Algebra.h"
#include "TrafficAlgebra.h"
#include "NetworkAlgebra.h"
#include "TupleIdentifier.h"
#include "TemporalAlgebra.h"
#include "TemporalNetAlgebra.h"
#include "QueryProcessor.h"
#include "NestedList.h"
#include "NList.h"
#include "ConstructorTemplates.h"
#include "Symbols.h"

using namespace symbols;

extern NestedList* nl;
extern QueryProcessor* qp;

MGPSecUnit::MGPSecUnit():StandardAttribute()
{
}

MGPSecUnit::MGPSecUnit(int secId, int direct, double sp,
                       Interval<Instant> timeInterval):
              m_secId(secId),
              m_direct(direct),
              m_speed(sp),
              m_time(timeInterval)
{
  del.refs=1;
  del.isDelete=true;
}

MGPSecUnit::MGPSecUnit( const MGPSecUnit& in_xOther):
                        m_secId(in_xOther.GetSecId()),
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

int MGPSecUnit::GetDirect() const
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

void MGPSecUnit::SetDirect(int dir)
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
  size_t hash = m_secId + m_direct + (int) m_speed +
                (int) m_time.start.ToDouble() +
                (int) m_time.end.ToDouble();
  return hash;
}

void MGPSecUnit::CopyFrom( const StandardAttribute* right )
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
      << " Side: " << m_direct
      << " Timeinterval: " << m_time.Print(os) << endl;
  return os;
}


ListExpr MGPSecUnit::Out(ListExpr typeInfo, Word value)
{
  MGPSecUnit* msec = static_cast<MGPSecUnit*> (value.addr);
  if (msec->IsDefined())
    return nl->FourElemList(nl->IntAtom(msec->GetSecId()),
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
  if (list.length() == 4)
  {
    NList seclist = list.first();
    NList dirlist = list.second();
    NList speedlist = list.third();
    if (seclist.isInt() && dirlist.isInt() && speedlist.isReal())
    {
      NList timelist = list.fourth();
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
              Word w = new MGPSecUnit(seclist.intval(), dirlist.intval(),
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
                         "Expected 2 int a real and a timeInterval."));
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
Type Constructor for ~mgpsecunit~

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
    listRep = "(<secId> <direction> <speed>(<timeinterval>))";
    valueExample = "(15 1 3.5 (\"2000-01-01\" \"2000-01-02\" TRUE FALSE))";
    remarks = "direction:down=0,up=1,none=2. Speed: m/s";
  }
};

mgpsecInfo mgpinfo;
mgpsecFunctions mgpfunct;
TypeConstructor mgpsecunitTC(mgpinfo, mgpfunct);
/*
7 Creating the ~TrafficAlgebra~

*/

class TrafficAlgebra : public Algebra
{
 public:
  TrafficAlgebra() : Algebra()
  {
    AddTypeConstructor( &mgpsecunitTC);

    mgpsecunitTC.AssociateKind( "DATA" );

  }
  ~TrafficAlgebra() {};
};

/*
Initialization

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
Algebra*
InitializeTrafficAlgebra( NestedList* nlRef,
                          QueryProcessor* qpRef )
{
  return (new TrafficAlgebra());
}
