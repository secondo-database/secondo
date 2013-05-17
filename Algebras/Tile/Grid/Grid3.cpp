 
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

#include "Grid3.h"
#include "TypeConstructor.h"
#include "Symbols.h"

namespace TileAlgebra
{

Grid3::Grid3()
      :Grid2()
{
  
}

Grid3::Grid3(bool bDefined)
      :Grid2(bDefined)
{
  Reset(); 
}

Grid3::Grid3(const double& rX,
             const double& rY,
             const double& rLength,
             const datetime::DateTime& rDuration)
      :Grid2(rX, rY, rLength),
       m_Duration(rDuration)
{
  m_Duration.SetType(datetime::durationtype);
}

Grid3::Grid3(const Grid3& rGrid3)
      :Grid2(rGrid3.IsDefined())
{
  *this = rGrid3;
}

Grid3::~Grid3()
{
  
}

const Grid3& Grid3::operator=(const Grid3& rGrid3)
{
  if(this != &rGrid3)
  {
    SetDefined(rGrid3.IsDefined());
    Grid2::operator=(rGrid3);
    m_Duration = rGrid3.m_Duration;
  }

  return *this;
}

const datetime::DateTime& Grid3::GetDuration() const
{
  return m_Duration;
}

void Grid3::Reset()
{
  Grid2::Reset();
  m_Duration.SetToZero();
}

bool Grid3::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

Attribute* Grid3::Clone() const
{
  Attribute* pAttribute = new Grid3(*this);
  assert(pAttribute != NULL);

  return pAttribute;
}

int Grid3::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const Grid3* pGrid3 = dynamic_cast<const Grid3*>(pAttribute);
    
    if(pGrid3 != 0)
    {
      bool bIsDefined = IsDefined();
      bool bGrid3IsDefined = pGrid3->IsDefined();
      
      if(bIsDefined == true)
      {
        if(bGrid3IsDefined == true) // defined x defined
        {
          nRetVal = Grid2::Compare(pGrid3);

          if(nRetVal == 0)
          {
            nRetVal = m_Duration.Compare(&(pGrid3->m_Duration));
          }
        }
        
        else // defined x undefined
        {
          nRetVal = 1;
        }
      }
      
      else
      {
        if(bGrid3IsDefined == true) // undefined x defined
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

void Grid3::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const Grid3* pGrid3 = dynamic_cast<const Grid3*>(pAttribute);
    
    if(pGrid3 != 0)
    {
      *this = *pGrid3;
    }
  }
}

size_t Grid3::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = Grid2::HashValue() + (size_t)&m_Duration;
  }
  
  return hashValue;
}

size_t Grid3::Sizeof() const
{
  return sizeof(Grid3);
}

const std::string Grid3::BasicType()
{
  return "Grid3";
}

void* Grid3::Cast(void* pVoid)
{
  return new(pVoid)Grid3;
}

Word Grid3::Clone(const ListExpr typeInfo,
                  const Word& rWord)
{
  Word word;

  Grid3* pGrid3 = static_cast<Grid3*>(rWord.addr);

  if(pGrid3 != 0)
  {
    word.addr = new Grid3(*pGrid3);
    assert(word.addr != NULL);
  }

  return word;
}

void Grid3::Close(const ListExpr typeInfo,
                  Word& rWord)
{
  Grid3* pGrid3 = static_cast<Grid3*>(rWord.addr);
  
  if(pGrid3 != 0)
  {
    delete pGrid3;
    pGrid3 = 0;
    rWord.addr = 0;
  }
}

Word Grid3::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new Grid3(0.0, 0.0, 1.0, 1.0);
  assert(word.addr != NULL);

  return word;
}

void Grid3::Delete(const ListExpr typeInfo,
                   Word& rWord)
{
  Grid3* pGrid3 = static_cast<Grid3*>(rWord.addr);

  if(pGrid3 != 0)
  {
    delete pGrid3;
    pGrid3 = 0;
    rWord.addr = 0;
  }
}

TypeConstructor Grid3::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    Grid3::BasicType(), // type name function
    Grid3::Property,    // property function describing signature
    Grid3::Out,         // out function
    Grid3::In,          // in function
    0,                  // save to list function
    0,                  // restore from list function
    Grid3::Create,      // create function
    Grid3::Delete,      // delete function
    Grid3::Open,        // open function
    Grid3::Save,        // save function
    Grid3::Close,       // close function
    Grid3::Clone,       // clone function
    Grid3::Cast,        // cast function
    Grid3::SizeOfObj,   // sizeofobj function
    Grid3::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

Word Grid3::In(const ListExpr typeInfo,
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

      Grid3* pGrid3 = new Grid3(nlist.elem(1).realval(),
                                nlist.elem(2).realval(),
                                nlist.elem(3).realval(),
                                duration);
      
      if(pGrid3 != 0)
      { 
        if(pGrid3->GetLength() <= 0.0)
        {
          cmsg.inFunError("Length of Grid3 object must be larger than zero.");
          delete pGrid3;
          pGrid3 = 0;
          rCorrect = false;
        }

        else
        {
          bool bOK = pGrid3->m_Duration.ReadFrom(nlist.elem(4).listExpr(),
                                                 true);

          if(bOK == true &&
             pGrid3->m_Duration.IsDefined())
          {
            word.addr = pGrid3;
            rCorrect = true;
          }

          else
          {
            cmsg.inFunError("Duration of Grid3 object must be "
                            "(duration (days milliseconds)).");
            delete pGrid3;
            pGrid3 = 0;
            rCorrect = false;
          }
        }
      }
    }

    else
    {
      cmsg.inFunError("Types of nested list expression elements "
                      "of Grid3 object must be 3 reals and 1 list.");
    }
  }
  
  else
  {
    cmsg.inFunError("Length of nested list expression of Grid3 object "
                    "must be 4.");
  }
  
  return word;
}

bool Grid3::KindCheck(ListExpr type,
                      ListExpr& rErrorInfo)
{
  return NList(type).isSymbol(Grid3::BasicType());
}

bool Grid3::Open(SmiRecord& rValueRecord,
                 size_t& rOffset,
                 const ListExpr typeInfo,
                 Word& rValue)
{
  bool bRetVal = false;

  Grid3* pGrid3 = static_cast<Grid3*>(Attribute::Open(rValueRecord, rOffset,
                                                      typeInfo));

  if(pGrid3 != 0)
  {
    rValue = SetWord(pGrid3);
    bRetVal = true;
  }

  return bRetVal;
}

ListExpr Grid3::Out(ListExpr typeInfo,
                    Word value)
{
  NList nlist;

  Grid3* pGrid3 = static_cast<Grid3*>(value.addr);

  if(pGrid3 != NULL)
  {
    nlist.append(NList(pGrid3->GetX()));
    nlist.append(NList(pGrid3->GetY()));
    nlist.append(NList(pGrid3->GetLength()));
    nlist.append(NList(pGrid3->m_Duration.ToListExpr(true)));
  }

  return nlist.listExpr();
}

ListExpr Grid3::Property()
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

bool Grid3::Save(SmiRecord& rValueRecord,
                 size_t& rOffset,
                 const ListExpr typeInfo,
                 Word& rValue)
{
  bool bRetVal = false;

  Grid3* pGrid3 = static_cast<Grid3*>(rValue.addr);

  if(pGrid3 != 0)
  {
    Attribute::Save(rValueRecord, rOffset, typeInfo, pGrid3);
    bRetVal = true;
  }

  return bRetVal;
}

int Grid3::SizeOfObj()
{
  return sizeof(Grid3);
}

}
