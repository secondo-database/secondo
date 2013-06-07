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
#include "tgrid.h"
#include "TypeConstructor.h"
#include "Symbols.h"

namespace TileAlgebra
{

tgrid::tgrid()
      :Attribute()
{
  
}

tgrid::tgrid(bool bDefined)
      :Attribute(bDefined)
{
  Reset(); 
}

tgrid::tgrid(const double& rX,
             const double& rY,
             const double& rLength)
      :Attribute(true),
       m_dX(rX),
       m_dY(rY),
       m_dLength(rLength)
{

}

tgrid::tgrid(const tgrid& rtgrid)
      :Attribute(rtgrid.IsDefined())
{
  *this = rtgrid;
}

tgrid::~tgrid()
{
  
}

const tgrid& tgrid::operator=(const tgrid& rtgrid)
{
  if(this != &rtgrid)
  {
    Attribute::operator=(rtgrid);
    m_dX = rtgrid.m_dX;
    m_dY = rtgrid.m_dY;
    m_dLength = rtgrid.m_dLength;
  }

  return *this;
}

bool tgrid::operator==(const tgrid& rtgrid) const
{
  bool bIsEqual = false;

  if(this != &rtgrid)
  {
    if(m_dX == rtgrid.m_dX &&
       m_dY == rtgrid.m_dY &&
       m_dLength == rtgrid.m_dLength)
    {
      bIsEqual = true;
    }
  }

  return bIsEqual;
}

const double& tgrid::GetX() const
{
  return m_dX;
}

const double& tgrid::GetY() const
{
  return m_dY;
}

const double& tgrid::GetLength() const
{
  return m_dLength;
}

bool tgrid::SetX(const double& rX)
{
  bool bRetVal = true;

  m_dX = rX;

  return bRetVal;
}

bool tgrid::SetY(const double& rY)
{
  bool bRetVal = true;

  m_dY = rY;

  return bRetVal;
}

bool tgrid::SetLength(const double& rLength)
{
  bool bRetVal = false;

  m_dLength = rLength;

  if(m_dLength > 0.0)
  {
    bRetVal = true;
  }

  return bRetVal;
}

void tgrid::Reset()
{
  m_dX = 0.0;
  m_dY = 0.0;
  m_dLength = 1.0;
}

bool tgrid::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

Attribute* tgrid::Clone() const
{
  Attribute* pAttribute = new tgrid(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

int tgrid::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const tgrid* ptgrid = dynamic_cast<const tgrid*>(pAttribute);
    
    if(ptgrid != 0)
    {
      bool bIsDefined = IsDefined();
      bool btgridIsDefined = ptgrid->IsDefined();
      
      if(bIsDefined == true)
      {
        if(btgridIsDefined == true) // defined x defined
        {
          if(m_dX < ptgrid->m_dX)
          {
            nRetVal = -1;
          }

          else
          {
            if(m_dX > ptgrid->m_dX)
            {
              nRetVal = 1;
            }

            else // m_dX == ptgrid->m_dX
            {
              if(m_dY < ptgrid->m_dY)
              {
                nRetVal = -1;
              }

              else
              {
                if(m_dY > ptgrid->m_dY)
                {
                  nRetVal = 1;
                }

                else // m_dY == ptgrid->m_dY
                {
                  if(m_dLength < ptgrid->m_dLength)
                  {
                    nRetVal = -1;
                  }

                  else
                  {
                    if(m_dLength > ptgrid->m_dLength)
                    {
                      nRetVal = 1;
                    }

                    else // m_dLength == ptgrid->m_dLength
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
        if(btgridIsDefined == true) // undefined x defined
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

void tgrid::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const tgrid* ptgrid = dynamic_cast<const tgrid*>(pAttribute);
    
    if(ptgrid != 0)
    {
      *this = *ptgrid;
    }
  }
}

size_t tgrid::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }
  
  return hashValue;
}

size_t tgrid::Sizeof() const
{
  return sizeof(tgrid);
}

const std::string tgrid::BasicType()
{
  return TYPE_NAME_TGRID;
}

void* tgrid::Cast(void* pVoid)
{
  return new(pVoid)tgrid;
}

Word tgrid::Clone(const ListExpr typeInfo,
                  const Word& rWord)
{
  Word word;

  tgrid* ptgrid = static_cast<tgrid*>(rWord.addr);

  if(ptgrid != 0)
  {
    word.addr = new tgrid(*ptgrid);
    assert(word.addr != 0);
  }

  return word;
}

void tgrid::Close(const ListExpr typeInfo,
                  Word& rWord)
{
  tgrid* ptgrid = static_cast<tgrid*>(rWord.addr);
  
  if(ptgrid != 0)
  {
    delete ptgrid;
    rWord.addr = 0;
  }
}

Word tgrid::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new tgrid(0.0, 0.0, 1.0);
  assert(word.addr != 0);

  return word;
}

void tgrid::Delete(const ListExpr typeInfo,
                   Word& rWord)
{
  tgrid* ptgrid = static_cast<tgrid*>(rWord.addr);

  if(ptgrid != 0)
  {
    delete ptgrid;
    rWord.addr = 0;
  }
}

TypeConstructor tgrid::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    tgrid::BasicType(), // type name function
    tgrid::Property,    // property function describing signature
    tgrid::Out,         // out function
    tgrid::In,          // in function
    0,                  // save to list function
    0,                  // restore from list function
    tgrid::Create,      // create function
    tgrid::Delete,      // delete function
    tgrid::Open,        // open function
    tgrid::Save,        // save function
    tgrid::Close,       // close function
    tgrid::Clone,       // clone function
    tgrid::Cast,        // cast function
    tgrid::SizeOfObj,   // sizeofobj function
    tgrid::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

Word tgrid::In(const ListExpr typeInfo,
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
      tgrid* ptgrid = new tgrid(instanceList.elem(1).realval(),
                                instanceList.elem(2).realval(),
                                instanceList.elem(3).realval());
      
      if(ptgrid != 0)
      { 
        if(ptgrid->m_dLength <= 0)
        {
          cmsg.inFunError("Length of tgrid object must be larger than zero.");
          delete ptgrid;
          ptgrid = 0;
          rCorrect = false;
        }

        else
        {
          word.addr = ptgrid;
          rCorrect = true;
        }
      }
    }

    else
    {
      cmsg.inFunError("Types of nested list expression elements"
                      "of tgrid object must be reals.");
    }
  }

  else
  {
    cmsg.inFunError("Length of nested list expression of tgrid object"
                    "must be 3.");
  }

  return word;
}

bool tgrid::KindCheck(ListExpr type,
                      ListExpr& rErrorInfo)
{
  return NList(type).isSymbol(tgrid::BasicType());
}

bool tgrid::Open(SmiRecord& rValueRecord,
                 size_t& rOffset,
                 const ListExpr typeInfo,
                 Word& rValue)
{
  bool bRetVal = OpenAttribute<tgrid>(rValueRecord,
                                      rOffset,
                                      typeInfo,
                                      rValue);

  return bRetVal;
}


ListExpr tgrid::Out(ListExpr typeInfo,
                    Word value)
{
  NList nlist;

  tgrid* ptgrid = static_cast<tgrid*>(value.addr);

  if(ptgrid != 0)
  {
    nlist.append(NList(ptgrid->m_dX));
    nlist.append(NList(ptgrid->m_dY));
    nlist.append(NList(ptgrid->m_dLength));
  }

  return nlist.listExpr();
}

ListExpr tgrid::Property()
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

bool tgrid::Save(SmiRecord& rValueRecord,
                 size_t& rOffset,
                 const ListExpr typeInfo,
                 Word& rValue)
{
  bool bRetVal = SaveAttribute<tgrid>(rValueRecord,
                                      rOffset,
                                      typeInfo,
                                      rValue);

  return bRetVal;
}

int tgrid::SizeOfObj()
{
  return sizeof(tgrid);
}

}
