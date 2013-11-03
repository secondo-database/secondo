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

/*
TileAlgebra includes

*/

#include "tstring.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Constructor tstring does not initialize any members and
should only be used in conjunction with Cast method.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

tstring::tstring()
        :tint()
{

}

/*
Constructor tstring sets defined flag of base class Attribute and
initializes all members of the class with default values.

author: Dirk Zacher
parameters: bDefined - defined flag of base class Attribute
return value: -
exceptions: -

*/

tstring::tstring(bool bDefined)
        :tint(bDefined),
         m_UniqueStringArray(bDefined)
{
  
}

/*
Constructor tstring sets defined flag of base class Attribute to defined flag
of rtint object and initializes all members of the class with corresponding
values of rtint object and rUniqueStringArray object.

author: Dirk Zacher
parameters: rtint - reference to a tint object
            rUniqueStringArray - reference to an UniqueStringArray object
return value: -
exceptions: -

*/

tstring::tstring(const tint& rtint,
                 const UniqueStringArray& rUniqueStringArray)
        :tint(rtint),
         m_UniqueStringArray(rUniqueStringArray)
{

}

/*
Constructor tstring sets defined flag of base class Attribute to defined flag
of rtstring object and initializes all members of the class with corresponding
values of rtstring object.

author: Dirk Zacher
parameters: rtstring - reference to a tstring object
return value: -
exceptions: -

*/

tstring::tstring(const tstring& rtstring)
        :tint(rtstring),
         m_UniqueStringArray(rtstring.m_UniqueStringArray)
{
  
}

/*
Destructor deinitializes a tstring object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

tstring::~tstring()
{

}

/*
Operator= assigns all member values of a given tstring object
to the corresponding member values of this object.

author: Dirk Zacher
parameters: rtstring - reference to a tstring object
return value: reference to this object
exceptions: -

*/

tstring& tstring::operator=(const tstring& rtstring)
{
  if(this != &rtstring)
  {
    tint::operator =(rtstring);
    m_UniqueStringArray = rtstring.m_UniqueStringArray;
  }

  return *this;
}

/*
TileAlgebra operator atlocation returns the value of a tstring object
at given location rX and rY.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
            rValue - reference to a CcString object containing
                     the value at given location rX and rY
return value: -
exceptions: -

*/

void tstring::atlocation(const double& rX,
                         const double& rY,
                         CcString& rValue) const
{
  rValue.SetDefined(false);

  if(IsDefined())
  {
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
}

/*
TileAlgebra operator atrange returns all values of a tstring object
inside the given rectangle.

author: Dirk Zacher
parameters: rRectangle - reference to a Rectangle<2> object
            rtstring - reference to a tstring object containing all values
                       of the tstring object inside the given rectangle
return value: -
exceptions: -

*/

void tstring::atrange(const Rectangle<2>& rRectangle,
                      tstring& rtstring) const
{
  rtstring.SetDefined(false);

  if(IsDefined() &&
     rRectangle.IsDefined())
  {
    if(IsValidLocation(rRectangle.MinD(0), rRectangle.MinD(1)) &&
       IsValidLocation(rRectangle.MaxD(0), rRectangle.MaxD(1)))
    {
      rtstring.SetDefined(true);

      double x = m_Grid.GetX();
      double y = m_Grid.GetY();
      double length = m_Grid.GetLength();
      rtstring.SetGrid(m_Grid);

      Index<2> startIndex = GetLocationIndex(rRectangle.MinD(0),
                                             rRectangle.MinD(1));
      Index<2> endIndex = GetLocationIndex(rRectangle.MaxD(0),
                                           rRectangle.MaxD(1));

      for(int row = startIndex[1]; row <= endIndex[1]; row++)
      {
        for(int column = startIndex[0]; column <= endIndex[0]; column++)
        {
          if(rRectangle.MinD(0) <= (x + column * length) &&
             rRectangle.MaxD(0) >= (x + column * length) &&
             rRectangle.MinD(1) <= (y + row * length) &&
             rRectangle.MaxD(1) >= (y + row * length))
          {
            Index<2> index((int[]){column, row});
            std::string value = GetValue(index);

            if(tProperties<std::string>::TypeProperties::
               IsUndefinedValue(value) == false)
            {
              rtstring.SetValue(index, value, true);
            }
          }
        }
      }
    }
  }
}

/*
TileAlgebra operator minimum returns the minimum value of tstring object.

author: Dirk Zacher
parameters: rMinimum - reference to a std::string object containing
                       the minimum value of tstring object
return value: -
exceptions: -

*/

void tstring::minimum(std::string& rMinimum) const
{
  rMinimum.clear();

  if(IsDefined() &&
     tProperties<int>::TypeProperties::IsUndefinedValue(m_Minimum) == false)
  {
    bool bOK = m_UniqueStringArray.GetUniqueString(m_Minimum, rMinimum);
    assert(bOK);
  }
}

/*
TileAlgebra operator maximum returns the maximum value of tstring object.

author: Dirk Zacher
parameters: rMaximum - reference to a std::string object containing
                       the maximum value of tstring object
return value: -
exceptions: -

*/

void tstring::maximum(std::string& rMaximum) const
{
  rMaximum.clear();

  if(IsDefined() &&
     tProperties<int>::TypeProperties::IsUndefinedValue(m_Maximum) == false)
  {
    bool bOK = m_UniqueStringArray.GetUniqueString(m_Maximum, rMaximum);
    assert(bOK);
  }
}

/*
Method GetValue returns the string value of tstring object
at given 2-dimensional index.

author: Dirk Zacher
parameters: rIndex - reference to a 2-dimensional index
return value: string value of tstring object at given 2-dimensional index
exceptions: -

*/

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

/*
Method SetValue sets a string value of tstring object at given index and
recalculates minimum and maximum of tstring object if bSetExtrema is true.

author: Dirk Zacher
parameters: rIndex - reference to a 2-dimensional index
            rValue - reference to a string value
            bSetExtrema - flag that indicates if minimum and maximum
                          of tstring object should be recalculated
return value: true, if rValue was successfully set at rIndex, otherwise false
exceptions: -

*/

bool tstring::SetValue(const Index<2>& rIndex,
                       const std::string& rValue,
                       bool bSetExtrema)
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

    if(bSetExtrema == true)
    {
      std::string minimumValue;
      minimum(minimumValue);

      if(tProperties<int>::TypeProperties::IsUndefinedValue(m_Minimum) ||
         rValue < minimumValue)
      {
        m_Minimum = stringIndex;
      }

      std::string maximumValue;
      maximum(maximumValue);

      if(tProperties<int>::TypeProperties::IsUndefinedValue(m_Maximum) ||
         rValue > maximumValue)
      {
        m_Maximum = stringIndex;
      }
    }
  }

  return bRetVal;
}

/*
Method SetValue sets a string value of tstring object at given location
rX and rY and recalculates minimum and maximum of tstring object
if bSetExtrema is true.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
            rValue - reference to a value
            bSetExtrema - flag that indicates if minimum and maximum
                          of tstring object should be recalculated
return value: true, if rValue was successfully set at given location
              rX and rY, otherwise false
exceptions: -

*/

bool tstring::SetValue(const double& rX,
                       const double& rY,
                       const std::string& rValue,
                       bool bSetExtrema)
{
  bool bRetVal = false;

  if(IsDefined() &&
     IsValidLocation(rX, rY))
  {
    Index<2> index = GetLocationIndex(rX, rY);
    bRetVal = SetValue(index, rValue, bSetExtrema);
  }

  return bRetVal;
}

/*
Method Adjacent checks if this object is adjacent to given Attribute object.

author: Dirk Zacher
parameters: pAttribute - a pointer to an Attribute object
return value: true, if this object is adjacent to pAttribute, otherwise false
exceptions: -

*/

bool tstring::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

/*
Method Clone returns a copy of this object.

author: Dirk Zacher
parameters: -
return value: a pointer to a copy of this object
exceptions: -

*/

Attribute* tstring::Clone() const
{
  Attribute* pAttribute = new tstring(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

/*
Method Compare compares this object with given Attribute object.

author: Dirk Zacher
parameters: pAttribute - a pointer to an Attribute object
return value: -1 if this object < pAttribute object or
                 this object is undefined and pAttribute object is defined,
               0 if this object equals pAttribute object or
                 this object and pAttribute object are undefined,
               1 if this object > pAttribute object or
                 this object is defined and pAttribute object is undefined
exceptions: -

*/

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

/*
Method CopyFrom assigns all member values of pAttribute object
to the corresponding member values of this object.

author: Dirk Zacher
parameters: pAttribute - a pointer to an Attribute object
return value: -
exceptions: -

*/

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

/*
Method GetFLOB returns a pointer to the Flob with given index.

author: Dirk Zacher
parameters: i - index of Flob
return value: a pointer to the Flob with given index
exceptions: -

*/

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

/*
Method HashValue returns the hash value of the tstring object.

author: Dirk Zacher
parameters: -
return value: hash value of the tstring object
exceptions: -

*/

size_t tstring::HashValue() const
{
  size_t hashValue = 0;

  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }

  return hashValue;
}

/*
Method NumOfFLOBs returns the number of Flobs of a tstring object.

author: Dirk Zacher
parameters: -
return value: number of Flobs of a tstring object
exceptions: -

*/

int tstring::NumOfFLOBs() const
{ 
  int nNumberOfFLOBs = tint::NumOfFLOBs() +
                       m_UniqueStringArray.NumOfFLOBs();

  return nNumberOfFLOBs;
}

/*
Method Sizeof returns the size of tstring datatype.

author: Dirk Zacher
parameters: -
return value: size of tstring datatype
exceptions: -

*/

size_t tstring::Sizeof() const
{
  return sizeof(tstring);
}

/*
Method BasicType returns the typename of tstring datatype.

author: Dirk Zacher
parameters: -
return value: typename of tstring datatype
exceptions: -

*/

const std::string tstring::BasicType()
{
  return tProperties<std::string>::GetTypeName();
}

/*
Method Cast casts a void pointer to a new tstring object.

author: Dirk Zacher
parameters: pVoid - a pointer to a memory address
return value: a pointer to a new tstring object
exceptions: -

*/

void* tstring::Cast(void* pVoid)
{
  return new(pVoid)tstring;
}

/*
Method Clone clones an existing tstring object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing tstring object
return value: a Word that references a new tstring object
exceptions: -

*/

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

/*
Method Close closes an existing tstring object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing tstring object
return value: -
exceptions: -

*/

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

/*
Method Create creates a new tstring object.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of the new tstring object to create
return value: a Word that references a new tstring object
exceptions: -

*/

Word tstring::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new tstring(true);
  assert(word.addr != 0);

  return word;
}

/*
Method Delete deletes an existing tstring object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing tstring object
return value: -
exceptions: -

*/

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

/*
Method GetTypeConstructor returns the TypeConstructor of class tstring.

author: Dirk Zacher
parameters: -
return value: TypeConstructor of class tstring
exceptions: -

*/

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

/*
Method In creates a new tstring object on the basis of a given ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object to create on the basis of instance
            instance - ListExpr of the tstring object to create
            errorPos - error position
            rErrorInfo - reference to error information
            rCorrect - flag that indicates if tstring object correctly created
return value: a Word that references a new tstring object
exceptions: -

*/

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
                        int xDimensionSize = tProperties<std::string>::
                                             GetXDimensionSize();
                        int yDimensionSize = tProperties<std::string>::
                                             GetYDimensionSize();

                        if(indexX >= 0 &&
                           indexX <= xDimensionSize - sizeX &&
                           indexY >= 0 &&
                           indexY <= yDimensionSize - sizeY)
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
                                Index<2> index((int[]){(indexX + column),
                                                       (indexY + row)});
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
                                  bOK = ptstring->SetValue(index,
                                                           stringValue,
                                                           true);
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

/*
Method KindCheck checks if given type is tstring type.

author: Dirk Zacher
parameters: type - ListExpr of type to check
            rErrorInfo - reference to error information
return value: true, if type is tstring type, otherwise false
exceptions: -

*/

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

/*
Method Open opens an tstring object from a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord containing tstring object to open
            rOffset - Offset to the tstring object in SmiRecord
            typeInfo - TypeInfo of tstring object to open
            rValue - reference to a Word referencing the opened tstring object
return value: true, if tstring object was successfully opened, otherwise false
exceptions: -

*/

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

/*
Method Out writes out an existing tstring object in the form of a ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of tstring object to write out
            value - reference to a Word referencing the tstring object
return value: ListExpr of tstring object referenced by value
exceptions: -

*/

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
        sizeList.append(tProperties<std::string>::GetXDimensionSize());
        sizeList.append(tProperties<std::string>::GetYDimensionSize());
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

/*
Method Property returns all properties of tstring datatype.

author: Dirk Zacher
parameters: -
return value: properties of tstring datatype in the form of a ListExpr
exceptions: -

*/

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

/*
Method Save saves an existing tstring object in a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord to save existing tstring object
            rOffset - Offset to save position of tstring object in SmiRecord
            typeInfo - TypeInfo of tstring object to save
            rValue - reference to a Word referencing the tstring object to save
return value: true, if tstring object was successfully saved, otherwise false
exceptions: -

*/

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

/*
Method SizeOfObj returns the size of a tstring object.

author: Dirk Zacher
parameters: -
return value: size of a tstring object
exceptions: -

*/

int tstring::SizeOfObj()
{
  return sizeof(tstring);
}

/*
Method GetXDimensionSize returns the size of x dimension of datatype tstring.

author: Dirk Zacher
parameters: -
return value: size of x dimension of datatype tstring
exceptions: -

*/

int tProperties<std::string>::GetXDimensionSize()
{
  /*
  According to Prof. Dr. Güting all Tile Algebra data types should have
  an identical size of x dimension, optimized for data type tint.

  */

  int xDimensionSize = static_cast<unsigned int>
                       (std::pow((WinUnix::getPageSize() -
                                  sizeof(tgrid) -
                                  2 * sizeof(int)) /
                                  sizeof(int),
                                  0.5)
                       );

  return xDimensionSize;
}

/*
Method GetYDimensionSize returns the size of y dimension of datatype tstring.

author: Dirk Zacher
parameters: -
return value: size of y dimension of datatype tstring
exceptions: -

*/

int tProperties<std::string>::GetYDimensionSize()
{
  /*
  According to Prof. Dr. Güting all Tile Algebra data types should have
  an identical size of y dimension, optimized for data type tint.

  */

  int yDimensionSize = static_cast<unsigned int>
                       (std::pow((WinUnix::getPageSize() -
                                  sizeof(tgrid) -
                                  2 * sizeof(int)) /
                                  sizeof(int),
                                  0.5)
                       );

  return yDimensionSize;
}

/*
Method GetFlobElements returns the number of flob elements of datatype tstring.

author: Dirk Zacher
parameters: -
return value: number of flob elements of datatype tstring
exceptions: -

*/

int tProperties<std::string>::GetFlobElements()
{
  int flobElements = GetXDimensionSize() * GetYDimensionSize();

  return flobElements;
}

/*
Method GetFlobSize returns the size of the flob of datatype tstring.

author: Dirk Zacher
parameters: -
return value: size of the flob of datatype tstring
exceptions: -

*/

SmiSize tProperties<std::string>::GetFlobSize()
{
  SmiSize flobSize = GetFlobElements() * sizeof(int);

  return flobSize;
}

/*
Method GetTypeName returns the typename of datatype tstring.

author: Dirk Zacher
parameters: -
return value: typename of datatype tstring
exceptions: -

*/

std::string tProperties<std::string>::GetTypeName()
{
  return TYPE_NAME_TSTRING;
}

}
