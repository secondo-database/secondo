 
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

*/

#include "../Constants.h"
#include "grid3.h"
#include "TypeConstructor.h"
#include "Symbols.h"

namespace TileAlgebra
{

grid3::grid3()
      :grid2()
{
  
}

grid3::grid3(bool bDefined)
      :grid2(bDefined)
{
  Reset(); 
}

grid3::grid3(const double& rX,
             const double& rY,
             const double& rLength,
             const datetime::DateTime& rDuration)
      :grid2(rX, rY, rLength),
       m_Duration(rDuration)
{
  m_Duration.SetType(datetime::durationtype);
}

grid3::grid3(const grid3& rgrid3)
      :grid2(rgrid3.IsDefined())
{
  *this = rgrid3;
}

grid3::~grid3()
{
  
}

const grid3& grid3::operator=(const grid3& rgrid3)
{
  if(this != &rgrid3)
  {
    SetDefined(rgrid3.IsDefined());
    grid2::operator=(rgrid3);
    m_Duration = rgrid3.m_Duration;
  }

  return *this;
}

bool grid3::operator==(const grid3& rgrid3) const
{
  bool bIsEqual = false;

  if(this != &rgrid3)
  {
    if(grid2::operator==(rgrid3) &&
       m_Duration == rgrid3.m_Duration)
    {
      bIsEqual = true;
    }
  }

  return bIsEqual;
}

const datetime::DateTime& grid3::GetDuration() const
{
  return m_Duration;
}

void grid3::Reset()
{
  grid2::Reset();
  m_Duration.SetToZero();
}

bool grid3::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

Attribute* grid3::Clone() const
{
  Attribute* pAttribute = new grid3(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

int grid3::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const grid3* pgrid3 = dynamic_cast<const grid3*>(pAttribute);
    
    if(pgrid3 != 0)
    {
      bool bIsDefined = IsDefined();
      bool bgrid3IsDefined = pgrid3->IsDefined();
      
      if(bIsDefined == true)
      {
        if(bgrid3IsDefined == true) // defined x defined
        {
          nRetVal = grid2::Compare(pgrid3);

          if(nRetVal == 0)
          {
            nRetVal = m_Duration.Compare(&(pgrid3->m_Duration));
          }
        }
        
        else // defined x undefined
        {
          nRetVal = 1;
        }
      }
      
      else
      {
        if(bgrid3IsDefined == true) // undefined x defined
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

void grid3::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const grid3* pgrid3 = dynamic_cast<const grid3*>(pAttribute);
    
    if(pgrid3 != 0)
    {
      *this = *pgrid3;
    }
  }
}

size_t grid3::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = grid2::HashValue() + (size_t)&m_Duration;
  }
  
  return hashValue;
}

size_t grid3::Sizeof() const
{
  return sizeof(grid3);
}

const std::string grid3::BasicType()
{
  return TYPE_NAME_GRID3;
}

void* grid3::Cast(void* pVoid)
{
  return new(pVoid)grid3;
}

Word grid3::Clone(const ListExpr typeInfo,
                  const Word& rWord)
{
  Word word;

  grid3* pgrid3 = static_cast<grid3*>(rWord.addr);

  if(pgrid3 != 0)
  {
    word.addr = new grid3(*pgrid3);
    assert(word.addr != 0);
  }

  return word;
}

void grid3::Close(const ListExpr typeInfo,
                  Word& rWord)
{
  grid3* pgrid3 = static_cast<grid3*>(rWord.addr);
  
  if(pgrid3 != 0)
  {
    delete pgrid3;
    pgrid3 = 0;
    rWord.addr = 0;
  }
}

Word grid3::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new grid3(0.0, 0.0, 1.0, 1.0);
  assert(word.addr != 0);

  return word;
}

void grid3::Delete(const ListExpr typeInfo,
                   Word& rWord)
{
  grid3* pgrid3 = static_cast<grid3*>(rWord.addr);

  if(pgrid3 != 0)
  {
    delete pgrid3;
    pgrid3 = 0;
    rWord.addr = 0;
  }
}

TypeConstructor grid3::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    grid3::BasicType(), // type name function
    grid3::Property,    // property function describing signature
    grid3::Out,         // out function
    grid3::In,          // in function
    0,                  // save to list function
    0,                  // restore from list function
    grid3::Create,      // create function
    grid3::Delete,      // delete function
    grid3::Open,        // open function
    grid3::Save,        // save function
    grid3::Close,       // close function
    grid3::Clone,       // clone function
    grid3::Cast,        // cast function
    grid3::SizeOfObj,   // sizeofobj function
    grid3::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

Word grid3::In(const ListExpr typeInfo,
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

      grid3* pgrid3 = new grid3(nlist.elem(1).realval(),
                                nlist.elem(2).realval(),
                                nlist.elem(3).realval(),
                                duration);
      
      if(pgrid3 != 0)
      { 
        if(pgrid3->GetLength() <= 0.0)
        {
          cmsg.inFunError("Length of grid3 object must be larger than zero.");
          delete pgrid3;
          pgrid3 = 0;
          rCorrect = false;
        }

        else
        {
          bool bOK = pgrid3->m_Duration.ReadFrom(nlist.elem(4).listExpr(),
                                                 true);

          if(bOK == true &&
             pgrid3->m_Duration.IsDefined())
          {
            word.addr = pgrid3;
            rCorrect = true;
          }

          else
          {
            cmsg.inFunError("Duration of grid3 object must be "
                            "(duration (days milliseconds)).");
            delete pgrid3;
            pgrid3 = 0;
            rCorrect = false;
          }
        }
      }
    }

    else
    {
      cmsg.inFunError("Types of nested list expression elements "
                      "of grid3 object must be 3 reals and 1 list.");
    }
  }
  
  else
  {
    cmsg.inFunError("Length of nested list expression of grid3 object "
                    "must be 4.");
  }
  
  return word;
}

bool grid3::KindCheck(ListExpr type,
                      ListExpr& rErrorInfo)
{
  return NList(type).isSymbol(grid3::BasicType());
}

bool grid3::Open(SmiRecord& rValueRecord,
                 size_t& rOffset,
                 const ListExpr typeInfo,
                 Word& rValue)
{
  bool bRetVal = OpenAttribute<grid3>(rValueRecord,
                                      rOffset,
                                      typeInfo,
                                      rValue);

  return bRetVal;
}

ListExpr grid3::Out(ListExpr typeInfo,
                    Word value)
{
  NList nlist;

  grid3* pgrid3 = static_cast<grid3*>(value.addr);

  if(pgrid3 != 0)
  {
    nlist.append(NList(pgrid3->GetX()));
    nlist.append(NList(pgrid3->GetY()));
    nlist.append(NList(pgrid3->GetLength()));
    nlist.append(NList(pgrid3->m_Duration.ToListExpr(true)));
  }

  return nlist.listExpr();
}

ListExpr grid3::Property()
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

bool grid3::Save(SmiRecord& rValueRecord,
                 size_t& rOffset,
                 const ListExpr typeInfo,
                 Word& rValue)
{
  bool bRetVal = SaveAttribute<grid3>(rValueRecord,
                                      rOffset,
                                      typeInfo,
                                      rValue);

  return bRetVal;
}

int grid3::SizeOfObj()
{
  return sizeof(grid3);
}

}
