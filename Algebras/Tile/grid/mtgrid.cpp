 
/*
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, Department of Computer Science,
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

*/

#include "../Constants.h"
#include "mtgrid.h"
#include "TypeConstructor.h"
#include "Symbols.h"

namespace TileAlgebra
{

mtgrid::mtgrid()
       :tgrid()
{
  
}

mtgrid::mtgrid(bool bDefined)
       :tgrid(bDefined),
        m_Duration(datetime::durationtype)
{

}

mtgrid::mtgrid(const double& rX,
               const double& rY,
               const double& rLength,
               const datetime::DateTime& rDuration)
       :tgrid(rX, rY, rLength),
        m_Duration(rDuration)
{
  m_Duration.SetType(datetime::durationtype);
}

mtgrid::mtgrid(const mtgrid& rmtgrid)
       :tgrid(rmtgrid.IsDefined())
{
  *this = rmtgrid;
}

mtgrid::~mtgrid()
{
  
}

const mtgrid& mtgrid::operator=(const mtgrid& rmtgrid)
{
  if(this != &rmtgrid)
  {
    tgrid::operator=(rmtgrid);
    m_Duration = rmtgrid.m_Duration;
  }

  return *this;
}

bool mtgrid::operator==(const mtgrid& rmtgrid) const
{
  bool bIsEqual = false;

  if(this != &rmtgrid)
  {
    if(tgrid::operator==(rmtgrid) &&
       m_Duration == rmtgrid.m_Duration)
    {
      bIsEqual = true;
    }
  }

  return bIsEqual;
}

const datetime::DateTime& mtgrid::GetDuration() const
{
  return m_Duration;
}

bool mtgrid::SetDuration(const datetime::DateTime& rDuration)
{
  bool bRetVal = false;

  m_Duration = rDuration;

  if(m_Duration.LessThanZero() == false &&
     m_Duration.IsZero() == false)
  {
    bRetVal = true;
  }

  return bRetVal;
}

bool mtgrid::IsMatchingGrid(const tgrid& rtgrid) const
{
  bool bIsMatchingGrid = tgrid::IsMatchingGrid(rtgrid);

  return bIsMatchingGrid;
}

bool mtgrid::IsMatchingGrid(const mtgrid& rmtgrid) const
{
  bool bIsMatchingGrid = tgrid::IsMatchingGrid(rmtgrid);

  if(bIsMatchingGrid == true &&
     m_Duration == rmtgrid.m_Duration)
  {
    bIsMatchingGrid = true;
  }

  return bIsMatchingGrid;
}

void mtgrid::Reset()
{
  tgrid::Reset();
  m_Duration.SetToZero();
}

bool mtgrid::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

Attribute* mtgrid::Clone() const
{
  Attribute* pAttribute = new mtgrid(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

int mtgrid::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const mtgrid* pmtgrid = dynamic_cast<const mtgrid*>(pAttribute);
    
    if(pmtgrid != 0)
    {
      bool bIsDefined = IsDefined();
      bool bmtgridIsDefined = pmtgrid->IsDefined();
      
      if(bIsDefined == true)
      {
        if(bmtgridIsDefined == true) // defined x defined
        {
          nRetVal = tgrid::Compare(pmtgrid);

          if(nRetVal == 0)
          {
            nRetVal = m_Duration.Compare(&(pmtgrid->m_Duration));
          }
        }
        
        else // defined x undefined
        {
          nRetVal = 1;
        }
      }
      
      else
      {
        if(bmtgridIsDefined == true) // undefined x defined
        {
          nRetVal = -1;
        }
        
        else // undefined x undefined
        {
          nRetVal = 0;
        }
      }
    }
  }
  
  return nRetVal;
}

void mtgrid::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const mtgrid* pmtgrid = dynamic_cast<const mtgrid*>(pAttribute);
    
    if(pmtgrid != 0)
    {
      *this = *pmtgrid;
    }
  }
}

size_t mtgrid::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }
  
  return hashValue;
}

size_t mtgrid::Sizeof() const
{
  return sizeof(mtgrid);
}

const std::string mtgrid::BasicType()
{
  return TYPE_NAME_MTGRID;
}

void* mtgrid::Cast(void* pVoid)
{
  return new(pVoid)mtgrid;
}

Word mtgrid::Clone(const ListExpr typeInfo,
                   const Word& rWord)
{
  Word word;

  mtgrid* pmtgrid = static_cast<mtgrid*>(rWord.addr);

  if(pmtgrid != 0)
  {
    word.addr = new mtgrid(*pmtgrid);
    assert(word.addr != 0);
  }

  return word;
}

void mtgrid::Close(const ListExpr typeInfo,
                   Word& rWord)
{
  mtgrid* pmtgrid = static_cast<mtgrid*>(rWord.addr);
  
  if(pmtgrid != 0)
  {
    delete pmtgrid;
    pmtgrid = 0;
    rWord.addr = 0;
  }
}

Word mtgrid::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new mtgrid(0.0, 0.0, 1.0, 1.0);
  assert(word.addr != 0);

  return word;
}

void mtgrid::Delete(const ListExpr typeInfo,
                    Word& rWord)
{
  mtgrid* pmtgrid = static_cast<mtgrid*>(rWord.addr);

  if(pmtgrid != 0)
  {
    delete pmtgrid;
    pmtgrid = 0;
    rWord.addr = 0;
  }
}

TypeConstructor mtgrid::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    mtgrid::BasicType(), // type name function
    mtgrid::Property,    // property function describing signature
    mtgrid::Out,         // out function
    mtgrid::In,          // in function
    0,                   // save to list function
    0,                   // restore from list function
    mtgrid::Create,      // create function
    mtgrid::Delete,      // delete function
    mtgrid::Open,        // open function
    mtgrid::Save,        // save function
    mtgrid::Close,       // close function
    mtgrid::Clone,       // clone function
    mtgrid::Cast,        // cast function
    mtgrid::SizeOfObj,   // sizeofobj function
    mtgrid::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

Word mtgrid::In(const ListExpr typeInfo,
                const ListExpr instance,
                const int errorPos,
                ListExpr& rErrorInfo,
                bool& rCorrect)
{
  Word word;

  rCorrect = false;
  NList nlist(instance);

  if(nlist.length() == 4)
  {
    if(nlist.isReal(1) &&
       nlist.isReal(2) &&
       nlist.isReal(3) &&
       nlist.isList(4))
    {
      datetime::DateTime duration;

      mtgrid* pmtgrid = new mtgrid(nlist.elem(1).realval(),
                                   nlist.elem(2).realval(),
                                   nlist.elem(3).realval(),
                                   duration);
      
      if(pmtgrid != 0)
      { 
        if(pmtgrid->GetLength() <= 0.0)
        {
          cmsg.inFunError("Length of mtgrid object must be larger than zero.");
          delete pmtgrid;
          pmtgrid = 0;
          rCorrect = false;
        }

        else
        {
          bool bOK = pmtgrid->m_Duration.ReadFrom(nlist.elem(4).listExpr(),
                                                  true);

          if(bOK == true &&
             pmtgrid->m_Duration.IsDefined())
          {
            word.addr = pmtgrid;
            rCorrect = true;
          }

          else
          {
            cmsg.inFunError("Duration of mtgrid object must be "
                            "(duration (days milliseconds)).");
            delete pmtgrid;
            pmtgrid = 0;
            rCorrect = false;
          }
        }
      }
    }

    else
    {
      cmsg.inFunError("Types of nested list expression elements "
                      "of mtgrid object must be 3 reals and 1 list.");
    }
  }
  
  else
  {
    cmsg.inFunError("Length of nested list expression of mtgrid object "
                    "must be 4.");
  }
  
  return word;
}

bool mtgrid::KindCheck(ListExpr type,
                       ListExpr& rErrorInfo)
{
  return NList(type).isSymbol(mtgrid::BasicType());
}

bool mtgrid::Open(SmiRecord& rValueRecord,
                  size_t& rOffset,
                  const ListExpr typeInfo,
                  Word& rValue)
{
  bool bRetVal = OpenAttribute<mtgrid>(rValueRecord,
                                       rOffset,
                                       typeInfo,
                                       rValue);

  return bRetVal;
}

ListExpr mtgrid::Out(ListExpr typeInfo,
                     Word value)
{
  NList nlist;

  mtgrid* pmtgrid = static_cast<mtgrid*>(value.addr);

  if(pmtgrid != 0)
  {
    nlist.append(NList(pmtgrid->GetX()));
    nlist.append(NList(pmtgrid->GetY()));
    nlist.append(NList(pmtgrid->GetLength()));
    nlist.append(NList(pmtgrid->m_Duration.ToListExpr(true)));
  }

  return nlist.listExpr();
}

ListExpr mtgrid::Property()
{
  NList nlist;

  NList names;
  names.append(NList(std::string("Signature"), true));
  names.append(NList(std::string("Example Type List"), true));
  names.append(NList(std::string("ListRep"), true));
  names.append(NList(std::string("Example List"), true));
  names.append(NList(std::string("Remarks"), true));

  NList values;
  values.append(NList(std::string("-> DATA"), true));
  values.append(NList(BasicType(), true));
  values.append(NList(std::string("(<X> <Y> <Length> <Duration>)"), true));
  values.append(NList(std::string("(3.1415 2.718 12.0 (duration (0 60000))"),
                true));
  values.append(NList(std::string("length must be positive"), true));

  nlist = NList(names, values);

  return nlist.listExpr();
}

bool mtgrid::Save(SmiRecord& rValueRecord,
                  size_t& rOffset,
                  const ListExpr typeInfo,
                  Word& rValue)
{
  bool bRetVal = SaveAttribute<mtgrid>(rValueRecord,
                                       rOffset,
                                       typeInfo,
                                       rValue);

  return bRetVal;
}

int mtgrid::SizeOfObj()
{
  return sizeof(mtgrid);
}

}
