 
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
#include "tintArray.h"
#include "TypeConstructor.h"
#include "Symbols.h"

namespace TileAlgebra
{

tintArray::tintArray()
          :Attribute()
{
  
}
  
tintArray::tintArray(bool bDefined)
          :Attribute(bDefined),
           m_Grid(false)
{
  for(int i = 0; i < TINTARRAY_SIZE; i++)
  {
    m_Array[i] = UNDEFINED_INT;
  }
}

tintArray::tintArray(const tintArray& rtintArray)
          :Attribute(rtintArray.IsDefined())
{
  m_Grid = rtintArray.m_Grid;
  memcpy(m_Array, rtintArray.m_Array, TINTARRAY_SIZE * sizeof(int));
}

tintArray::~tintArray()
{
  
}

tintArray& tintArray::operator=(const tintArray& rtintArray)
{
  if(this != &rtintArray)
  {
    SetDefined(rtintArray.IsDefined());
    m_Grid = rtintArray.m_Grid;
    memcpy(m_Array, rtintArray.m_Array, TINTARRAY_SIZE * sizeof(int));
  }
  
  return *this;
}

bool tintArray::Load()
{
  bool bRetVal = true;

  return bRetVal;
}

bool tintArray::SetGrid(const double& rX,
                        const double& rY,
                        const double& rLength)
{
  bool bRetVal = true;

  m_Grid.SetDefined(true);
  bRetVal &= m_Grid.SetX(rX);
  bRetVal &= m_Grid.SetY(rY);
  bRetVal &= m_Grid.SetLength(rLength);

  return bRetVal;
}

bool tintArray::SetValue(int nIndex, int nValue)
{
  bool bRetVal = false;

  if(IsDefined() &&
     nIndex >= 0 &&
     nIndex < TINTARRAY_SIZE)
  {
    bRetVal = true;
    m_Array[nIndex] = nValue;
  }

  return bRetVal;
}

bool tintArray::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

Attribute* tintArray::Clone() const
{
  Attribute* pAttribute = new tintArray(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

int tintArray::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const tintArray* ptintArray = dynamic_cast<const tintArray*>(pAttribute);
    
    if(ptintArray != 0)
    {
      bool bIsDefined = IsDefined();
      bool btintArrayIsDefined = ptintArray->IsDefined();
      
      if(bIsDefined == true)
      {
        if(btintArrayIsDefined == true) // defined x defined
        {
          nRetVal = memcmp(m_Array, ptintArray->m_Array,
                           TINTARRAY_SIZE * sizeof(int));
        }
        
        else // defined x undefined
        {
          nRetVal = 1;
        }
      }
      
      else
      {
        if(btintArrayIsDefined == true) // undefined x defined
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

void tintArray::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const tintArray* ptintArray = dynamic_cast<const tintArray*>(pAttribute);
    
    if(ptintArray != 0)
    {
      *this = *ptintArray;
    }
  }
}

size_t tintArray::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = (size_t)&m_Array;
  }
  
  return hashValue;
}

size_t tintArray::Sizeof() const
{
  return sizeof(tintArray);
}

const std::string tintArray::BasicType()
{
  return "tintArray";
}

void* tintArray::Cast(void* pVoid)
{
  return new(pVoid)tintArray;
}

Word tintArray::Clone(const ListExpr typeInfo,
                      const Word& rWord)
{
  Word word;
  
  tintArray* ptintArray = static_cast<tintArray*>(rWord.addr);
                     
  if(ptintArray != 0)
  {
    word.addr = new tintArray(*ptintArray);
    assert(word.addr != 0);
  }
  
  return word;
}

void tintArray::Close(const ListExpr typeInfo,
                            Word& rWord)
{
  tintArray* ptintArray = static_cast<tintArray*>(rWord.addr);
  
  if(ptintArray != 0)
  {
    delete ptintArray;
    rWord.addr = 0;
  }
}

Word tintArray::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new tintArray(true);
  assert(word.addr != 0);

  return word;
}

void tintArray::Delete(const ListExpr typeInfo,
                       Word& rWord)
{
  tintArray* ptintArray = static_cast<tintArray*>(rWord.addr);
  
  if(ptintArray != NULL)
  {
    delete ptintArray;
    rWord.addr = 0;
  }
}

TypeConstructor tintArray::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    tintArray::BasicType(), // type name function    
    tintArray::Property,    // property function describing signature
    tintArray::Out,         // out function
    tintArray::In,          // in function
    0,                      // save to list function
    0,                      // restore from list function
    tintArray::Create,      // create function
    tintArray::Delete,      // delete function
    tintArray::Open,        // open function
    tintArray::Save,        // save function
    tintArray::Close,       // close function
    tintArray::Clone,       // clone function
    tintArray::Cast,        // cast function
    tintArray::SizeOfObj,   // sizeofobj function
    tintArray::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());
  
  return typeConstructor;
}

Word tintArray::In(const ListExpr typeInfo,
                   const ListExpr instance,
                   const int errorPos,
                   ListExpr& rErrorInfo,
                   bool& rCorrect)
{ 
  Word word;
  
  NList instanceList(instance);
  rCorrect = false;

  if(instanceList.isAtom() == false)
  {
    NList gridList = instanceList.elem(1);

    if(gridList.length() == 3)
    {
      if(gridList.isReal(1) &&
         gridList.isReal(2) &&
         gridList.isReal(3))
      {
        tintArray* ptintArray = new tintArray(true);

        if(ptintArray != 0)
        {
          bool bOK = ptintArray->SetGrid(gridList.elem(1).realval(),
                                         gridList.elem(2).realval(),
                                         gridList.elem(3).realval());

          if(bOK == true)
          {
            instanceList.rest();

            if(instanceList.isEmpty() == false)
            {
              NList sizeList = instanceList.elem(1);

              if(sizeList.length() == 2)
              {
                if(sizeList.isInt(1) &&
                   sizeList.isInt(2) &&
                   sizeList.elem(1).intval() > 0 &&
                   sizeList.elem(2).intval() > 0)
                {
                  int sizeX = sizeList.elem(1).intval();
                  int sizeY = sizeList.elem(2).intval();
                  Cardinal valueListLength = static_cast<Cardinal>
                                             (sizeX * sizeY);

                  instanceList.rest();

                  while(bOK &&
                        instanceList.isEmpty() == false)
                  {
                    NList pageList = instanceList.first();

                    if(pageList.length() == 3)
                    {
                      if(pageList.isInt(1) &&
                         pageList.isInt(2))
                      {
                        int indexX = pageList.elem(1).intval();
                        int indexY = pageList.elem(2).intval();

                        if(indexX >= 0 &&
                           indexX <= TINTARRAY_DIMENSION_SIZE - sizeX &&
                           indexY >= 0 &&
                           indexY <= TINTARRAY_DIMENSION_SIZE - sizeY)
                        {
                          pageList.rest();
                          pageList.rest();

                          NList valueList = pageList.first();

                          if(valueList.length() == valueListLength)
                          {
                            for(int row = 0; row < sizeY; row++)
                            {
                              for(int column = 0; column < sizeX; column++)
                              {
                                int listIndex = row * sizeX + column + 1;
                                int arrayIndex = (indexY + row) *
                                                 TINTARRAY_DIMENSION_SIZE +
                                                 (indexX + column);
                                int value = UNDEFINED_INT;

                                if(valueList.elem(listIndex).
                                   isSymbol(Symbol::UNDEFINED()) == false)
                                {
                                  if(valueList.elem(listIndex).isInt())
                                  {
                                    value = valueList.elem(listIndex).intval();
                                  }

                                  else
                                  {
                                    bOK = false;
                                    cmsg.inFunError("Type mismatch: "
                                                    "list value in "
                                                    "partial grid has "
                                                    "wrong type.");
                                  }
                                }

                                ptintArray->SetValue(arrayIndex, value);
                              }
                            }

                            instanceList.rest();
                          }

                          else
                          {
                            bOK = false;
                            cmsg.inFunError("Type mismatch: "
                                            "list for partial grid values "
                                            "is too short or too long.");
                          }
                        }

                        else
                        {
                          bOK = false;
                          cmsg.inFunError("Type mismatch: "
                                          "page list index is "
                                          "out of valid range.");
                        }
                      }

                      else
                      {
                        bOK = false;
                        cmsg.inFunError("Type mismatch: "
                                        "partial grid content must start "
                                        "with two integers.");
                      }
                    }

                    else
                    {
                      bOK = false;
                      cmsg.inFunError("Type mismatch: "
                                      "partial grid content must contain "
                                      "three elements.");
                    }
                  }

                }

                else
                {
                  bOK = false;
                  cmsg.inFunError("Type mismatch: "
                                  "partial grid size must contain "
                                  "two positive integers.");
                }
              }

              else
              {
                bOK = false;
                cmsg.inFunError("Size list must have a length of 2.");
              }
            }

            else
            {
              bOK = false;
              cmsg.inFunError("Expected list as second element, "
                              "got an empty list.");
            }
          }

          if(bOK)
          {
            word.setAddr(ptintArray);
            rCorrect = true;
          }

          else
          {
            delete ptintArray;
            ptintArray = 0;
            rCorrect = false;
          }
        }
      }

      else
      {
        cmsg.inFunError("Type mismatch: expected 3 reals as grid2 sublist.");
      }
    }

    else
    {
      cmsg.inFunError("Type mismatch: list for grid2 is too short "
                      "or too long.");
    }
  }

  else
  {
    cmsg.inFunError("Expected list as first element, got an atom.");
  }

  return word;
}

bool tintArray::KindCheck(ListExpr type,
                          ListExpr& rErrorInfo)
{
  bool bRetVal = false;
  
  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, tintArray::BasicType());
  }
  
  return bRetVal;
}

bool tintArray::Open(SmiRecord& rValueRecord,
                     size_t& rOffset,
                     const ListExpr typeInfo,
                     Word& rValue)
{ 
  bool bRetVal = false;

  tintArray* ptintArray = static_cast<tintArray*>(Attribute::Open(rValueRecord,
                                                                  rOffset,
                                                                  typeInfo));
  
  if(ptintArray != 0)
  { 
    rValue = SetWord(ptintArray);
    bRetVal = true;
  }

  return bRetVal;
}

ListExpr tintArray::Out(ListExpr typeInfo,
                        Word value)
{ 
  ListExpr pListExpr = 0;
  
  if(nl != 0)
  {  
    tintArray* ptintArray = static_cast<tintArray*>(value.addr);
    
    if(ptintArray != 0)
    {
      if(ptintArray->IsDefined() == true)
      {
        NList instanceList;

        NList gridList;
        gridList.append(ptintArray->m_Grid.GetX());
        gridList.append(ptintArray->m_Grid.GetY());
        gridList.append(ptintArray->m_Grid.GetLength());
        instanceList.append(gridList);

        NList sizeList;
        sizeList.append(TINTARRAY_DIMENSION_SIZE);
        sizeList.append(TINTARRAY_DIMENSION_SIZE);
        instanceList.append(sizeList);

        NList tintList;
        tintList.append(0);
        tintList.append(0);

        NList valueList;

        for(int i = 0; i < TINTARRAY_SIZE; i++)
        {
          if(ptintArray->m_Array[i] == UNDEFINED_INT)
          {
            valueList.append(NList(Symbol::UNDEFINED()));
          }

          else
          {
            valueList.append(ptintArray->m_Array[i]);
          }
        }

        tintList.append(valueList);
        instanceList.append(tintList);

        pListExpr = instanceList.listExpr();
      }
      
      else
      {
        pListExpr = nl->SymbolAtom(Symbol::UNDEFINED());
      }
    }
  }
  
  return pListExpr;
}

ListExpr tintArray::Property()
{
  NList propertyList;

  NList names;
  names.append(NList(std::string("Signature"), true));
  names.append(NList(std::string("Example Type List"), true));
  names.append(NList(std::string("ListRep"), true));
  names.append(NList(std::string("Example List"), true));
  names.append(NList(std::string("Remarks"), true));

  NList values;
  values.append(NList(std::string("-> DATA"), true));
  values.append(NList(BasicType(), true));
  values.append(NList
               (std::string("((x y l) (szx szy) ((ix iy (v*)))*)"),
                true));
  values.append(NList
               (std::string("((0.0 0.0 1.0) (2 2) ((-32 -32 (1 2 3 4))))"),
                true));
  values.append(NList(std::string(""), true));

  propertyList = NList(names, values);

  return propertyList.listExpr();
}

bool tintArray::Save(SmiRecord& rValueRecord,
                     size_t& rOffset,
                     const ListExpr typeInfo,
                     Word& rValue)
{ 
  bool bRetVal = false;
  
  tintArray* ptintArray = static_cast<tintArray*>(rValue.addr);
  
  if(ptintArray != 0)
  {
    Attribute::Save(rValueRecord, rOffset, typeInfo, ptintArray);
    bRetVal = true;
  }
  
  return bRetVal;
}

int tintArray::SizeOfObj()
{
  return sizeof(tintArray);
}

}
