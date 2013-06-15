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

#include "tstring.h"

namespace TileAlgebra
{

/*
implementation of class tstring

*/

tstring::tstring()
        :tint()
{

}

tstring::tstring(bool bDefined)
        :tint(bDefined),
         m_UniqueStringArray(bDefined)
{
  
}

tstring::tstring(const tint& rtint,
                 const UniqueStringArray& rUniqueStringArray)
        :tint(rtint),
         m_UniqueStringArray(rUniqueStringArray)
{

}

tstring::tstring(const tstring& rtstring)
        :tint(rtstring),
         m_UniqueStringArray(rtstring.m_UniqueStringArray)
{
  
}

tstring::~tstring()
{

}

tstring& tstring::operator=(const tstring& rtstring)
{
  if(this != &rtstring)
  {
    tint::operator =(rtstring);
    m_UniqueStringArray = rtstring.m_UniqueStringArray;
  }

  return *this;
}

void tstring::atlocation(const double& rX,
                         const double& rY,
                         CcString& rValue) const
{
  rValue.SetDefined(false);

  CcInt intValue;
  tint::atlocation(rX, rY, intValue);

  if(intValue.IsDefined())
  {
    std::string stringValue;
    bool bOK = m_UniqueStringArray.GetUniqueString(intValue.GetValue(),
                                                   stringValue);

    if(bOK == true)
    {
      rValue = tProperties<std::string>::TypeProperties::
               GetWrappedValue(stringValue);
    }
  }
}

void tstring::atlocation(const double& rX,
                         const double& rY,
                         const double& rInstant,
                         CcString& rValue) const
{
  /*
  instant value is not relevant for tstring type.

  */
  
  atlocation(rX, rY, rValue);
}

std::string tstring::minimum() const
{
  std::string minimum;

  if(tProperties<int>::TypeProperties::IsUndefinedValue(m_Minimum) == false)
  {
    bool bOK = m_UniqueStringArray.GetUniqueString(m_Minimum, minimum);
    assert(bOK);
  }

  return minimum;
}

std::string tstring::maximum() const
{
  std::string maximum;

  if(tProperties<int>::TypeProperties::IsUndefinedValue(m_Maximum) == false)
  {
    bool bOK = m_UniqueStringArray.GetUniqueString(m_Maximum, maximum);
    assert(bOK);
  }

  return maximum;
}

std::string tstring::GetValue(const Index<2>& rIndex) const
{
  std::string value = tProperties<std::string>::TypeProperties::
                      GetUndefinedValue();

  if(IsDefined() &&
     IsValidIndex(rIndex))
  {
    int intValue = tint::GetValue(rIndex);

    if(tProperties<int>::TypeProperties::IsUndefinedValue(intValue) == false)
    {
      bool bOK = m_UniqueStringArray.GetUniqueString(intValue, value);
      assert(bOK);
    }
  }

  return value;
}

bool tstring::SetValue(const Index<2>& rIndex,
                       const std::string& rValue)
{
  bool bRetVal = false;

  if(IsDefined() &&
     IsValidIndex(rIndex))
  {
    int stringIndex = m_UniqueStringArray.AddString(rValue);

    if(stringIndex != UNDEFINED_STRING_INDEX)
    {
      bRetVal = tint::SetValue(rIndex, stringIndex, false);
    }

    if(tProperties<int>::TypeProperties::IsUndefinedValue(m_Minimum) ||
       rValue < minimum())
    {
      m_Minimum = stringIndex;
    }
   
    if(tProperties<int>::TypeProperties::IsUndefinedValue(m_Maximum) ||
       rValue > maximum())
    {
      m_Maximum = stringIndex;
    }
  }

  return bRetVal;
}

bool tstring::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

Attribute* tstring::Clone() const
{
  Attribute* pAttribute = new tstring(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

int tstring::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;

  if(pAttribute != 0)
  {
    const tstring* ptstring = dynamic_cast<const tstring*>(pAttribute);

    if(ptstring != 0)
    {
      bool bIsDefined = IsDefined();
      bool btstringIsDefined = ptstring->IsDefined();

      if(bIsDefined == true)
      {
        if(btstringIsDefined == true) // defined x defined
        {
          nRetVal = tint::Compare(ptstring);

          if(nRetVal == 0)
          {
            nRetVal = m_UniqueStringArray.Compare(
                      &(ptstring->m_UniqueStringArray));
          }
        }

        else // defined x undefined
        {
          nRetVal = 1;
        }
      }

      else
      {
        if(btstringIsDefined == true) // undefined x defined
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

void tstring::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const tstring* ptstring = dynamic_cast<const tstring*>(pAttribute);

    if(ptstring != 0)
    {
      *this = *ptstring;
    }
  }
}

Flob* tstring::GetFLOB(const int i)
{ 
  Flob* pFlob = 0;
  int nFlobs = NumOfFLOBs();
  
  if(i >= 0 &&
     i < nFlobs)
  {
    switch(i)
    {
      case 0:   pFlob = tint::GetFLOB(i);
                break;
      case 1:   pFlob = m_UniqueStringArray.GetFLOB(i - 1);
                break;
      case 2:   pFlob = m_UniqueStringArray.GetFLOB(i - 1);
                break;
      default:  break;
    }
  }
  
  return pFlob;
}

size_t tstring::HashValue() const
{
  size_t hashValue = 0;

  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }

  return hashValue;
}

int tstring::NumOfFLOBs() const
{ 
  int nNumberOfFLOBs = tint::NumOfFLOBs() +
                       m_UniqueStringArray.NumOfFLOBs();

  return nNumberOfFLOBs;
}

size_t tstring::Sizeof() const
{
  return sizeof(tstring);
}

const std::string tstring::BasicType()
{
  return tProperties<std::string>::GetTypeName();
}

void* tstring::Cast(void* pVoid)
{
  return new(pVoid)tstring;
}

Word tstring::Clone(const ListExpr typeInfo,
                    const Word& rWord)
{
  Word word;

  tstring* ptstring = static_cast<tstring*>(rWord.addr);

  if(ptstring != 0)
  {
    word.addr = new tstring(*ptstring);
    assert(word.addr != 0);
  }

  return word;
}

void tstring::Close(const ListExpr typeInfo,
                    Word& rWord)
{
  tstring* ptstring = static_cast<tstring*>(rWord.addr);

  if(ptstring != 0)
  {
    delete ptstring;
    rWord.addr = 0;
  }
}

Word tstring::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new tstring(true);
  assert(word.addr != 0);

  return word;
}

void tstring::Delete(const ListExpr typeInfo,
                     Word& rWord)
{
  tstring* ptstring = static_cast<tstring*>(rWord.addr);

  if(ptstring != 0)
  {
    delete ptstring;
    rWord.addr = 0;
  }
}

TypeConstructor tstring::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    tstring::BasicType(), // type name function
    tstring::Property,    // property function describing signature
    tstring::Out,         // out function
    tstring::In,          // in function
    0,                    // save to list function
    0,                    // restore from list function
    tstring::Create,      // create function
    tstring::Delete,      // delete function
    tstring::Open,        // open function
    tstring::Save,        // save function
    tstring::Close,       // close function
    tstring::Clone,       // clone function
    tstring::Cast,        // cast function
    tstring::SizeOfObj,   // sizeofobj function
    tstring::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

Word tstring::In(const ListExpr typeInfo,
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
        tstring* ptstring = new tstring(true);

        if(ptstring != 0)
        {
          bool bOK = ptstring->SetGrid(gridList.elem(1).realval(),
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
                        int dimensionSize = tProperties<std::string>::
                                            GetDimensionSize();

                        if(indexX >= 0 &&
                           indexX <= dimensionSize - sizeX &&
                           indexY >= 0 &&
                           indexY <= dimensionSize - sizeY)
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
                                Index<2> index = (int[]){(indexX + column),
                                                         (indexY + row)};
                                std::string stringValue = tProperties
                                                          <std::string>::
                                                          TypeProperties::
                                                          GetUndefinedValue();

                                if(valueList.elem(listIndex).
                                   isSymbol(Symbol::UNDEFINED()) == false)
                                {
                                  if(tProperties<std::string>::TypeProperties::
                                     IsValidValueType
                                     (valueList.elem(listIndex)))
                                  {
                                    stringValue = tProperties<std::string>::
                                                  TypeProperties::GetValue(
                                                  valueList.elem(listIndex));
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

                                if(tProperties<std::string>::TypeProperties::
                                   IsUndefinedValue(stringValue) == false)
                                {
                                  bOK = ptstring->SetValue(index, stringValue);
                                }
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
          }

          if(bOK)
          {
            word.addr = ptstring;
            rCorrect = true;
          }

          else
          {
            delete ptstring;
            ptstring = 0;
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

bool tstring::KindCheck(ListExpr type,
                        ListExpr& rErrorInfo)
{
  bool bRetVal = false;

  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, tstring::BasicType());
  }

  return bRetVal;
}

bool tstring::Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue)
{
  bool bRetVal = OpenAttribute<tstring>(rValueRecord,
                                        rOffset,
                                        typeInfo,
                                        rValue);

  return bRetVal;
}

ListExpr tstring::Out(ListExpr typeInfo,
                      Word value)
{
  ListExpr pListExpr = 0;

  if(nl != 0)
  {
    tstring* ptstring = static_cast<tstring*>(value.addr);

    if(ptstring != 0)
    {
      if(ptstring->IsDefined() == true)
      {
        NList instanceList;

        NList gridList;
        gridList.append(ptstring->m_Grid.GetX());
        gridList.append(ptstring->m_Grid.GetY());
        gridList.append(ptstring->m_Grid.GetLength());
        instanceList.append(gridList);

        NList sizeList;
        sizeList.append(tProperties<std::string>::GetDimensionSize());
        sizeList.append(tProperties<std::string>::GetDimensionSize());
        instanceList.append(sizeList);

        NList tintList;
        tintList.append(0);
        tintList.append(0);

        std::string undefinedStringValue = tProperties<std::string>::
                                           TypeProperties::GetUndefinedValue();
        NList valueList;

        for(int i = 0; i < tProperties<std::string>::GetFlobElements(); i++)
        {
          int stringIndex = UNDEFINED_STRING_INDEX;
          std::string stringValue = undefinedStringValue;

          bool bOK = ptstring->m_Flob.read(reinterpret_cast<char*>
                                           (&stringIndex),
                                           sizeof(int),
                                           i * sizeof(int));
          
          if(bOK &&
             stringIndex != UNDEFINED_INT)
          {
            bOK = ptstring->m_UniqueStringArray.GetUniqueString(stringIndex,
                                                                stringValue);
          }

          valueList.append(tProperties<std::string>::TypeProperties::
                           ToNList(stringValue));
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

ListExpr tstring::Property()
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
               (std::string("((0.0 0.0 1.0) (2 2) "
                            "((0 0 (\"A\" \"B\" \"A\" \"C\"))"),
                true));
  values.append(NList(std::string(""), true));

  propertyList = NList(names, values);

  return propertyList.listExpr();
}

bool tstring::Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue)
{
  bool bRetVal = SaveAttribute<tstring>(rValueRecord,
                                        rOffset,
                                        typeInfo,
                                        rValue);

  return bRetVal;
}

int tstring::SizeOfObj()
{
  return sizeof(tstring);
}

/*
implementation of template class tProperties<std::string>

*/

int tProperties<std::string>::GetDimensionSize()
{
  int dimensionSize = static_cast<unsigned int>
                      (std::pow((WinUnix::getPageSize() -
                                 sizeof(tgrid) -
                                 2 * sizeof(int)) /
                                 sizeof(int),
                                 0.5)
                      );

  return dimensionSize;
}

int tProperties<std::string>::GetFlobElements()
{
  int nFlobElements = static_cast<unsigned int>
                      (std::pow(GetDimensionSize(), 2));

  return nFlobElements;
}

SmiSize tProperties<std::string>::GetFlobSize()
{
  SmiSize flobSize = GetFlobElements() * sizeof(int);

  return flobSize;
}

std::string tProperties<std::string>::GetTypeName()
{
  return TYPE_NAME_TSTRING;
}

}
