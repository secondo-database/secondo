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

#include "Grid2.h"
#include "TypeConstructor.h"
#include "Symbols.h"

namespace TileAlgebra
{

Grid2::Grid2()
      :Attribute()
{
  
}

Grid2::Grid2(bool bDefined)
      :Attribute(bDefined)
{
  Reset(); 
}

Grid2::Grid2(const double& rX,
             const double& rY,
             const double& rLength)
      :Attribute(true),
       m_dX(rX),
       m_dY(rY),
       m_dLength(rLength)
{
  assert(m_dLength > 0);
}

Grid2::Grid2(const Grid2& rGrid2)
      :Attribute(rGrid2.IsDefined())
{
  *this = rGrid2;
}

Grid2::~Grid2()
{
  
}

const Grid2& Grid2::operator=(const Grid2& rGrid2)
{
  if(this != &rGrid2)
  {
    SetDefined(rGrid2.IsDefined());
    m_dX = rGrid2.m_dX;
    m_dY = rGrid2.m_dY;
    m_dLength = rGrid2.m_dLength;
  }

  return *this;
}

const double& Grid2::GetX() const
{
  return m_dX;
}

const double& Grid2::GetY() const
{
  return m_dY;
}

const double& Grid2::GetLength() const
{
  return m_dLength;
}

bool Grid2::SetX(const double& rX)
{
  bool bRetVal = true;

  m_dX = rX;

  return bRetVal;
}

bool Grid2::SetY(const double& rY)
{
  bool bRetVal = true;

  m_dY = rY;

  return bRetVal;
}

bool Grid2::SetLength(const double& rLength)
{
  bool bRetVal = false;

  m_dLength = rLength;

  if(m_dLength > 0.0)
  {
    bRetVal = true;
  }

  return bRetVal;
}

void Grid2::Reset()
{
  m_dX = 0.0;
  m_dY = 0.0;
  m_dLength = 1.0;
}

bool Grid2::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

Attribute* Grid2::Clone() const
{
  Attribute* pAttribute = new Grid2(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

int Grid2::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const Grid2* pGrid2 = dynamic_cast<const Grid2*>(pAttribute);
    
    if(pGrid2 != 0)
    {
      bool bIsDefined = IsDefined();
      bool bGrid2IsDefined = pGrid2->IsDefined();
      
      if(bIsDefined == true)
      {
        if(bGrid2IsDefined == true) // defined x defined
        {
          if(m_dX < pGrid2->m_dX)
          {
            nRetVal = -1;
          }

          else
          {
            if(m_dX > pGrid2->m_dX)
            {
              nRetVal = 1;
            }

            else // m_dX == pGrid2->m_dX
            {
              if(m_dY < pGrid2->m_dY)
              {
                nRetVal = -1;
              }

              else
              {
                if(m_dY > pGrid2->m_dY)
                {
                  nRetVal = 1;
                }

                else // m_dY == pGrid2->m_dY
                {
                  if(m_dLength < pGrid2->m_dLength)
                  {
                    nRetVal = -1;
                  }

                  else
                  {
                    if(m_dLength > pGrid2->m_dLength)
                    {
                      nRetVal = 1;
                    }

                    else // m_dLength == pGrid2->m_dLength
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
        if(bGrid2IsDefined == true) // undefined x defined
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

void Grid2::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const Grid2* pGrid2 = dynamic_cast<const Grid2*>(pAttribute);
    
    if(pGrid2 != 0)
    {
      *this = *pGrid2;
    }
  }
}

size_t Grid2::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = (size_t)&m_dX + (size_t)&m_dY + (size_t)&m_dLength;
  }
  
  return hashValue;
}

size_t Grid2::Sizeof() const
{
  return sizeof(Grid2);
}

const std::string Grid2::BasicType()
{
  return "Grid2";
}

void* Grid2::Cast(void* pVoid)
{
  return new(pVoid)Grid2;
}

Word Grid2::Clone(const ListExpr typeInfo,
                  const Word& rWord)
{
  Word word;

  Grid2* pGrid2 = static_cast<Grid2*>(rWord.addr);

  if(pGrid2 != 0)
  {
    word.addr = new Grid2(*pGrid2);
    assert(word.addr != 0);
  }

  return word;
}

void Grid2::Close(const ListExpr typeInfo,
                  Word& rWord)
{
  Grid2* pGrid2 = static_cast<Grid2*>(rWord.addr);
  
  if(pGrid2 != 0)
  {
    delete pGrid2;
    rWord.addr = 0;
  }
}

Word Grid2::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new Grid2(0.0, 0.0, 1.0);
  assert(word.addr != 0);

  return word;
}

void Grid2::Delete(const ListExpr typeInfo,
                   Word& rWord)
{
  Grid2* pGrid2 = static_cast<Grid2*>(rWord.addr);

  if(pGrid2 != 0)
  {
    delete pGrid2;
    rWord.addr = 0;
  }
}

TypeConstructor Grid2::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    Grid2::BasicType(), // type name function
    Grid2::Property,    // property function describing signature
    Grid2::Out,         // out function
    Grid2::In,          // in function
    0,                  // save to list function
    0,                  // restore from list function
    Grid2::Create,      // create function
    Grid2::Delete,      // delete function
    Grid2::Open,        // open function
    Grid2::Save,        // save function
    Grid2::Close,       // close function
    Grid2::Clone,       // clone function
    Grid2::Cast,        // cast function
    Grid2::SizeOfObj,   // sizeofobj function
    Grid2::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

Word Grid2::In(const ListExpr typeInfo,
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
      Grid2* pGrid2 = new Grid2(instanceList.elem(1).realval(),
                                instanceList.elem(2).realval(),
                                instanceList.elem(3).realval());
      
      if(pGrid2 != 0)
      { 
        if(pGrid2->m_dLength <= 0)
        {
          cmsg.inFunError("Length of Grid2 object must be larger than zero.");
          delete pGrid2;
          pGrid2 = 0;
          rCorrect = false;
        }

        else
        {
          word.addr = pGrid2;
          rCorrect = true;
        }
      }
    }

    else
    {
      cmsg.inFunError("Types of nested list expression elements"
                      "of Grid2 object must be reals.");
    }
  }

  else
  {
    cmsg.inFunError("Length of nested list expression of Grid2 object"
                    "must be 3.");
  }

  return word;
}

bool Grid2::KindCheck(ListExpr type,
                      ListExpr& rErrorInfo)
{
  return NList(type).isSymbol(Grid2::BasicType());
}

bool Grid2::Open(SmiRecord& rValueRecord,
                 size_t& rOffset,
                 const ListExpr typeInfo,
                 Word& rValue)
{
  bool bRetVal = false;

  Grid2* pGrid2 = static_cast<Grid2*>(Attribute::Open(rValueRecord, rOffset,
                                                      typeInfo));

  if(pGrid2 != 0)
  {
    rValue = SetWord(pGrid2);
    bRetVal = true;
  }

  return bRetVal;
}


ListExpr Grid2::Out(ListExpr typeInfo,
                    Word value)
{
  NList nlist;

  Grid2* pGrid2 = static_cast<Grid2*>(value.addr);

  if(pGrid2 != 0)
  {
    nlist.append(NList(pGrid2->m_dX));
    nlist.append(NList(pGrid2->m_dY));
    nlist.append(NList(pGrid2->m_dLength));
  }

  return nlist.listExpr();
}

ListExpr Grid2::Property()
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

bool Grid2::Save(SmiRecord& rValueRecord,
                 size_t& rOffset,
                 const ListExpr typeInfo,
                 Word& rValue)
{
  bool bRetVal = false;

  Grid2* pGrid2 = static_cast<Grid2*>(rValue.addr);

  if(pGrid2 != 0)
  {
    Attribute::Save(rValueRecord, rOffset, typeInfo, pGrid2);
    bRetVal = true;
  }

  return bRetVal;
}

int Grid2::SizeOfObj()
{
  return sizeof(Grid2);
}

}
