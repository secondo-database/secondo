 
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
#include "tintFlob.h"
#include "TypeConstructor.h"
#include "Symbols.h"

namespace TileAlgebra
{

tintFlob::tintFlob()
         :Attribute()
{
  
}
  
tintFlob::tintFlob(bool bDefined)
         :Attribute(bDefined),
          m_Grid(false),
          m_Flob(TINTFLOB_SIZE)
{
  for(int i = 0; i < TINTFLOB_ELEMENTS; i++)
  {
    SetValue(i, UNDEFINED_INT);
  }
}

tintFlob::tintFlob(const tintFlob& rtintFlob)
                  :Attribute(rtintFlob.IsDefined()),
                   m_Grid(rtintFlob.m_Grid),
                   m_Flob(rtintFlob.m_Flob)
{
  
}

tintFlob::~tintFlob()
{
  
}

tintFlob& tintFlob::operator=(const tintFlob& rtintFlob)
{
  if(this != &rtintFlob)
  {
    SetDefined(rtintFlob.IsDefined());
    m_Grid=rtintFlob.m_Grid;

    bool bOK = false;

    bOK = m_Flob.clean();
    assert(bOK);
    bOK = m_Flob.copyFrom(rtintFlob.m_Flob);
    assert(bOK);
  }
  
  return *this;
}

bool tintFlob::operator==(const tintFlob& rtintFlob) const
{
  bool bIsEqual = false;

  if(this != &rtintFlob)
  {
    if(m_Grid == rtintFlob.m_Grid &&
       m_Flob == rtintFlob.m_Flob)
    {
      bIsEqual = true;
    }
  }

  return bIsEqual;
}

void tintFlob::Destroy()
{
  m_Flob.destroy();
}

bool tintFlob::Load()
{
  bool bRetVal = true;

  /*
  According to Prof. Dr. Gueting open method of an attribute type loads
  only root record in memory. Flob data will be read immediately before
  an operator tries to access flob object and will be stored in a read cache.

  */

  char buffer[TINTFLOB_SIZE];
  bRetVal = m_Flob.read(buffer, TINTFLOB_SIZE, 0);

  return bRetVal;
}

bool tintFlob::SetGrid(const double& rX,
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

bool tintFlob::SetValue(int nIndex, int nValue)
{
  bool bRetVal = false;

  if(IsDefined() &&
     nIndex >= 0 &&
     nIndex < TINTFLOB_ELEMENTS)
  {
    bRetVal = m_Flob.write(reinterpret_cast<const char*>(&nValue),
                           sizeof(int),
                           nIndex * sizeof(int));
  }

  return bRetVal;
}

bool tintFlob::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

Attribute* tintFlob::Clone() const
{
  Attribute* pAttribute = new tintFlob(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

int tintFlob::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const tintFlob* ptintFlob = dynamic_cast<const tintFlob*>(pAttribute);
    
    if(ptintFlob != 0)
    {
      bool bIsDefined = IsDefined();
      bool btintFlobIsDefined = ptintFlob->IsDefined();
      
      if(bIsDefined == true)
      {
        if(btintFlobIsDefined == true) // defined x defined
        {
          char buffer1[TINTFLOB_SIZE];
          m_Flob.read(buffer1, TINTFLOB_SIZE, 0);

          char buffer2[TINTFLOB_SIZE];
          ptintFlob->m_Flob.read(buffer2, TINTFLOB_SIZE, 0);

          nRetVal = memcmp(buffer1, buffer2, TINTFLOB_SIZE);
        }
        
        else // defined x undefined
        {
          nRetVal = 1;
        }
      }
      
      else
      {
        if(btintFlobIsDefined == true) // undefined x defined
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

void tintFlob::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const tintFlob* ptintFlob = dynamic_cast<const tintFlob*>(pAttribute);
    
    if(ptintFlob != 0)
    {
      *this = *ptintFlob;
    }
  }
}

Flob* tintFlob::GetFLOB(const int i)
{ 
  Flob* pFlob = 0;
  int nFlobs = NumOfFLOBs();
  
  if(i >= 0 &&
     i < nFlobs)
  {
    switch(i)
    {
      case 0:   pFlob = &m_Flob;
                break;
                
      default:  break;
    }
  }
  
  return pFlob;
}

size_t tintFlob::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }
  
  return hashValue;
}

int tintFlob::NumOfFLOBs() const
{ 
  return 1;
}

size_t tintFlob::Sizeof() const
{
  return sizeof(tintFlob);
}

const std::string tintFlob::BasicType()
{
  return "tintFlob";
}

void* tintFlob::Cast(void* pVoid)
{
  return new(pVoid)tintFlob;
}

Word tintFlob::Clone(const ListExpr typeInfo,
                     const Word& rWord)
{
  Word word;
  
  tintFlob* ptintFlob = static_cast<tintFlob*>(rWord.addr);
                     
  if(ptintFlob != 0)
  {
    word.addr = new tintFlob(*ptintFlob);
    assert(word.addr != 0);
  }
  
  return word;
}

void tintFlob::Close(const ListExpr typeInfo,
                           Word& rWord)
{
  tintFlob* ptintFlob = static_cast<tintFlob*>(rWord.addr);
  
  if(ptintFlob != 0)
  {
    delete ptintFlob;
    rWord.addr = 0;
  }
}

Word tintFlob::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new tintFlob(true);
  assert(word.addr != 0);

  return word;
}

void tintFlob::Delete(const ListExpr typeInfo,
                      Word& rWord)
{
  tintFlob* ptintFlob = static_cast<tintFlob*>(rWord.addr);
  
  if(ptintFlob != 0)
  {
    delete ptintFlob;
    rWord.addr = 0;
  }
}

TypeConstructor tintFlob::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    tintFlob::BasicType(), // type name function    
    tintFlob::Property,    // property function describing signature
    tintFlob::Out,         // out function
    tintFlob::In,          // in function
    0,                     // save to list function
    0,                     // restore from list function
    tintFlob::Create,      // create function
    tintFlob::Delete,      // delete function
    tintFlob::Open,        // open function
    tintFlob::Save,        // save function
    tintFlob::Close,       // close function
    tintFlob::Clone,       // clone function
    tintFlob::Cast,        // cast function
    tintFlob::SizeOfObj,   // sizeofobj function
    tintFlob::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());
  
  return typeConstructor;
}

Word tintFlob::In(const ListExpr typeInfo,
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
        tintFlob* ptintFlob = new tintFlob(true);

        if(ptintFlob != 0)
        {
          bool bOK = ptintFlob->SetGrid(gridList.elem(1).realval(),
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
                           indexX <= TINTFLOB_DIMENSION_SIZE - sizeX &&
                           indexY >= 0 &&
                           indexY <= TINTFLOB_DIMENSION_SIZE - sizeY)
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
                                int flobIndex = (indexY + row) *
                                                TINTFLOB_DIMENSION_SIZE +
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

                                ptintFlob->SetValue(flobIndex, value);
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
            word.setAddr(ptintFlob);
            rCorrect = true;
          }

          else
          {
            delete ptintFlob;
            ptintFlob = 0;
            rCorrect = false;
          }
        }
      }

      else
      {
        cmsg.inFunError("Type mismatch: expected 3 reals as tgrid sublist.");
      }
    }

    else
    {
      cmsg.inFunError("Type mismatch: list for tgrid is too short "
                      "or too long.");
    }
  }

  else
  {
    cmsg.inFunError("Expected list as first element, got an atom.");
  }

  return word;
}

bool tintFlob::KindCheck(ListExpr type,
                         ListExpr& rErrorInfo)
{
  bool bRetVal = false;
  
  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, tintFlob::BasicType());
  }
  
  return bRetVal;
}

bool tintFlob::Open(SmiRecord& rValueRecord,
                    size_t& rOffset,
                    const ListExpr typeInfo,
                    Word& rValue)
{ 
  bool bRetVal = OpenAttribute<tintFlob>(rValueRecord,
                                         rOffset,
                                         typeInfo,
                                         rValue);

  return bRetVal;
}

ListExpr tintFlob::Out(ListExpr typeInfo,
                       Word value)
{ 
  ListExpr pListExpr = 0;

  if(nl != 0)
  {  
    tintFlob* ptintFlob = static_cast<tintFlob*>(value.addr);
    
    if(ptintFlob != 0)
    {
      if(ptintFlob->IsDefined() == true)
      {
        NList instanceList;

        NList gridList;
        gridList.append(ptintFlob->m_Grid.GetX());
        gridList.append(ptintFlob->m_Grid.GetY());
        gridList.append(ptintFlob->m_Grid.GetLength());
        instanceList.append(gridList);

        NList sizeList;
        sizeList.append(TINTFLOB_DIMENSION_SIZE);
        sizeList.append(TINTFLOB_DIMENSION_SIZE);
        instanceList.append(sizeList);

        NList tintList;
        tintList.append(0);
        tintList.append(0);

        NList valueList;

        for(int i = 0; i < TINTFLOB_ELEMENTS; i++)
        {
          int nValue = UNDEFINED_INT;

          bool bOK = ptintFlob->m_Flob.read(reinterpret_cast<char*>(&nValue),
                                            sizeof(int),
                                            i * sizeof(int));
          assert(bOK);

          if(nValue == UNDEFINED_INT)
          {
            valueList.append(NList(Symbol::UNDEFINED()));
          }

          else
          {
            valueList.append(nValue);
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

ListExpr tintFlob::Property()
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

bool tintFlob::Save(SmiRecord& rValueRecord,
                    size_t& rOffset,
                    const ListExpr typeInfo,
                    Word& rValue)
{ 
  bool bRetVal = SaveAttribute<tintFlob>(rValueRecord,
                                         rOffset,
                                         typeInfo,
                                         rValue);

  return bRetVal;
}

int tintFlob::SizeOfObj()
{
  return sizeof(tintFlob);
}

}
