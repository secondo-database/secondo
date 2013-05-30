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
#include "grid2.h"
#include "TypeConstructor.h"
#include "Symbols.h"

namespace TileAlgebra
{

grid2::grid2()
      :Attribute()
{
  
}

grid2::grid2(bool bDefined)
      :Attribute(bDefined)
{
  Reset(); 
}

grid2::grid2(const double& rX,
             const double& rY,
             const double& rLength)
      :Attribute(true),
       m_dX(rX),
       m_dY(rY),
       m_dLength(rLength)
{

}

grid2::grid2(const grid2& rgrid2)
      :Attribute(rgrid2.IsDefined())
{
  *this = rgrid2;
}

grid2::~grid2()
{
  
}

const grid2& grid2::operator=(const grid2& rgrid2)
{
  if(this != &rgrid2)
  {
    SetDefined(rgrid2.IsDefined());
    m_dX = rgrid2.m_dX;
    m_dY = rgrid2.m_dY;
    m_dLength = rgrid2.m_dLength;
  }

  return *this;
}

bool grid2::operator==(const grid2& rgrid2) const
{
  bool bIsEqual = false;

  if(this != &rgrid2)
  {
    if(m_dX == rgrid2.m_dX &&
       m_dY == rgrid2.m_dY &&
       m_dLength == rgrid2.m_dLength)
    {
      bIsEqual = true;
    }
  }

  return bIsEqual;
}

const double& grid2::GetX() const
{
  return m_dX;
}

const double& grid2::GetY() const
{
  return m_dY;
}

const double& grid2::GetLength() const
{
  return m_dLength;
}

bool grid2::SetX(const double& rX)
{
  bool bRetVal = true;

  m_dX = rX;

  return bRetVal;
}

bool grid2::SetY(const double& rY)
{
  bool bRetVal = true;

  m_dY = rY;

  return bRetVal;
}

bool grid2::SetLength(const double& rLength)
{
  bool bRetVal = false;

  m_dLength = rLength;

  if(m_dLength > 0.0)
  {
    bRetVal = true;
  }

  return bRetVal;
}

void grid2::Reset()
{
  m_dX = 0.0;
  m_dY = 0.0;
  m_dLength = 1.0;
}

bool grid2::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

Attribute* grid2::Clone() const
{
  Attribute* pAttribute = new grid2(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

int grid2::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const grid2* pgrid2 = dynamic_cast<const grid2*>(pAttribute);
    
    if(pgrid2 != 0)
    {
      bool bIsDefined = IsDefined();
      bool bgrid2IsDefined = pgrid2->IsDefined();
      
      if(bIsDefined == true)
      {
        if(bgrid2IsDefined == true) // defined x defined
        {
          if(m_dX < pgrid2->m_dX)
          {
            nRetVal = -1;
          }

          else
          {
            if(m_dX > pgrid2->m_dX)
            {
              nRetVal = 1;
            }

            else // m_dX == pgrid2->m_dX
            {
              if(m_dY < pgrid2->m_dY)
              {
                nRetVal = -1;
              }

              else
              {
                if(m_dY > pgrid2->m_dY)
                {
                  nRetVal = 1;
                }

                else // m_dY == pgrid2->m_dY
                {
                  if(m_dLength < pgrid2->m_dLength)
                  {
                    nRetVal = -1;
                  }

                  else
                  {
                    if(m_dLength > pgrid2->m_dLength)
                    {
                      nRetVal = 1;
                    }

                    else // m_dLength == pgrid2->m_dLength
                    {
                      nRetVal = 0;
                    }
                  }
                }
              }
            }
          }
        }
        
        else // defined x undefined
        {
          nRetVal = 1;
        }
      }
      
      else
      {
        if(bgrid2IsDefined == true) // undefined x defined
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

void grid2::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const grid2* pgrid2 = dynamic_cast<const grid2*>(pAttribute);
    
    if(pgrid2 != 0)
    {
      *this = *pgrid2;
    }
  }
}

size_t grid2::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = (size_t)&m_dX + (size_t)&m_dY + (size_t)&m_dLength;
  }
  
  return hashValue;
}

size_t grid2::Sizeof() const
{
  return sizeof(grid2);
}

const std::string grid2::BasicType()
{
  return TYPE_NAME_grid2;
}

void* grid2::Cast(void* pVoid)
{
  return new(pVoid)grid2;
}

Word grid2::Clone(const ListExpr typeInfo,
                  const Word& rWord)
{
  Word word;

  grid2* pgrid2 = static_cast<grid2*>(rWord.addr);

  if(pgrid2 != 0)
  {
    word.addr = new grid2(*pgrid2);
    assert(word.addr != 0);
  }

  return word;
}

void grid2::Close(const ListExpr typeInfo,
                  Word& rWord)
{
  grid2* pgrid2 = static_cast<grid2*>(rWord.addr);
  
  if(pgrid2 != 0)
  {
    delete pgrid2;
    rWord.addr = 0;
  }
}

Word grid2::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new grid2(0.0, 0.0, 1.0);
  assert(word.addr != 0);

  return word;
}

void grid2::Delete(const ListExpr typeInfo,
                   Word& rWord)
{
  grid2* pgrid2 = static_cast<grid2*>(rWord.addr);

  if(pgrid2 != 0)
  {
    delete pgrid2;
    rWord.addr = 0;
  }
}

TypeConstructor grid2::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    grid2::BasicType(), // type name function
    grid2::Property,    // property function describing signature
    grid2::Out,         // out function
    grid2::In,          // in function
    0,                  // save to list function
    0,                  // restore from list function
    grid2::Create,      // create function
    grid2::Delete,      // delete function
    grid2::Open,        // open function
    grid2::Save,        // save function
    grid2::Close,       // close function
    grid2::Clone,       // clone function
    grid2::Cast,        // cast function
    grid2::SizeOfObj,   // sizeofobj function
    grid2::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

Word grid2::In(const ListExpr typeInfo,
               const ListExpr instance,
               const int errorPos,
               ListExpr& rErrorInfo,
               bool& rCorrect)
{
  Word word;

  rCorrect = false;
  NList instanceList(instance);
  
  if(instanceList.length() == 3)
  {
    if(instanceList.isReal(1) &&
       instanceList.isReal(2) &&
       instanceList.isReal(3))
    {
      grid2* pgrid2 = new grid2(instanceList.elem(1).realval(),
                                instanceList.elem(2).realval(),
                                instanceList.elem(3).realval());
      
      if(pgrid2 != 0)
      { 
        if(pgrid2->m_dLength <= 0)
        {
          cmsg.inFunError("Length of grid2 object must be larger than zero.");
          delete pgrid2;
          pgrid2 = 0;
          rCorrect = false;
        }

        else
        {
          word.addr = pgrid2;
          rCorrect = true;
        }
      }
    }

    else
    {
      cmsg.inFunError("Types of nested list expression elements"
                      "of grid2 object must be reals.");
    }
  }

  else
  {
    cmsg.inFunError("Length of nested list expression of grid2 object"
                    "must be 3.");
  }

  return word;
}

bool grid2::KindCheck(ListExpr type,
                      ListExpr& rErrorInfo)
{
  return NList(type).isSymbol(grid2::BasicType());
}

bool grid2::Open(SmiRecord& rValueRecord,
                 size_t& rOffset,
                 const ListExpr typeInfo,
                 Word& rValue)
{
  bool bRetVal = OpenAttribute<grid2>(rValueRecord,
                                      rOffset,
                                      typeInfo,
                                      rValue);

  return bRetVal;
}


ListExpr grid2::Out(ListExpr typeInfo,
                    Word value)
{
  NList nlist;

  grid2* pgrid2 = static_cast<grid2*>(value.addr);

  if(pgrid2 != 0)
  {
    nlist.append(NList(pgrid2->m_dX));
    nlist.append(NList(pgrid2->m_dY));
    nlist.append(NList(pgrid2->m_dLength));
  }

  return nlist.listExpr();
}

ListExpr grid2::Property()
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
  values.append(NList(std::string("(<X> <Y> <Length>)"), true));
  values.append(NList(std::string("(3.1415 2.718 12.0)"), true));
  values.append(NList(std::string("length must be positive"), true));

  nlist = NList(names, values);

  return nlist.listExpr();
}

bool grid2::Save(SmiRecord& rValueRecord,
                 size_t& rOffset,
                 const ListExpr typeInfo,
                 Word& rValue)
{
  bool bRetVal = SaveAttribute<grid2>(rValueRecord,
                                      rOffset,
                                      typeInfo,
                                      rValue);

  return bRetVal;
}

int grid2::SizeOfObj()
{
  return sizeof(grid2);
}

}
