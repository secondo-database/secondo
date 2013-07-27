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

#include "mtstring.h"
#include "../Constants.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Constructor mtstring does not initialize any members and
should only be used in conjunction with Cast method.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

mtstring::mtstring()
         :mtint()
{

}

/*
Constructor mtstring sets defined flag of base class Attribute and
initializes all members of the class with default values.

author: Dirk Zacher
parameters: bDefined - defined flag of base class Attribute
return value: -
exceptions: -

*/

mtstring::mtstring(bool bDefined)
         :mtint(bDefined),
          m_UniqueStringArray(bDefined)
{
  
}

/*
Constructor mtstring sets defined flag of base class Attribute to defined flag
of rmtint object and initializes all members of the class with corresponding
values of rmtint object and rUniqueStringArray object.

author: Dirk Zacher
parameters: rmtint - reference to a mtint object
            rUniqueStringArray - reference to an UniqueStringArray object
return value: -
exceptions: -

*/

mtstring::mtstring(const mtint& rmtint,
                   const UniqueStringArray& rUniqueStringArray)
         :mtint(rmtint),
          m_UniqueStringArray(rUniqueStringArray)
{
  
}

/*
Constructor mtstring sets defined flag of base class Attribute to defined flag
of rmtstring object and initializes all members of the class with corresponding
values of rmtstring object.

author: Dirk Zacher
parameters: rmtstring - reference to a mtstring object
return value: -
exceptions: -

*/

mtstring::mtstring(const mtstring& rmtstring)
         :mtint(rmtstring),
          m_UniqueStringArray(rmtstring.m_UniqueStringArray)
{
  
}

/*
Destructor deinitializes a mtstring object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

mtstring::~mtstring()
{

}

/*
Operator= assigns all member values of a given mtstring object
to the corresponding member values of this object.

author: Dirk Zacher
parameters: rmtstring - reference to a mtstring object
return value: reference to this object
exceptions: -

*/

mtstring& mtstring::operator=(const mtstring& rmtstring)
{
  if(this != &rmtstring)
  {
    const mtint& rmtint = dynamic_cast<const mtint&>(rmtstring);
    mtint::operator =(rmtint);
    m_UniqueStringArray = rmtstring.m_UniqueStringArray;
  }

  return *this;
}

/*
TileAlgebra operator atlocation returns the time dependent values
of a mtstring object at given location rX and rY.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
            rValues - reference to a MString object containing
            the time dependent values at given location rX and rY
return value: -
exceptions: -

*/

void mtstring::atlocation(const double& rX,
                          const double& rY,
                          MString& rValues) const
{
  rValues.SetDefined(false);

  if(IsValidLocation(rX, rY))
  {
    rValues.SetDefined(true);

    Rectangle<3> boundingBox;
    bbox(boundingBox);
    Instant minimumTime = boundingBox.MinD(2);
    Instant maximumTime = boundingBox.MaxD(2);
    datetime::DateTime duration = m_Grid.GetDuration();

    rValues.StartBulkLoad();

    for(Instant currentTime = minimumTime;
        currentTime < maximumTime;
        currentTime += duration)
    {
      CcString value;
      atlocation(rX, rY, currentTime.ToDouble(), value);

      if(value.IsDefined())
      {
        Interval<Instant> interval(currentTime, currentTime + duration,
                                   true, false);
        rValues.Add(UString(interval, value, value));
      }
    }

    rValues.EndBulkLoad();

    if(rValues.IsEmpty())
    {
      rValues.SetDefined(false);
    }
  }
}

/*
TileAlgebra operator atlocation returns the value of a mtstring object
at given location rX and rY at given Instant value.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
            rInstant - reference to an Instant value of time dimension
            rValue - reference to the value at given location rX and rY
                     at given rInstant value
return value: -
exceptions: -

*/

void mtstring::atlocation(const double& rX,
                          const double& rY,
                          const double& rInstant,
                          CcString& rValue) const
{
  rValue.SetDefined(false);

  if(IsValidLocation(rX, rY, rInstant))
  {
    Index<3> index = GetLocationIndex(rX, rY, rInstant);
    std::string value = GetValue(index);

    if(mtProperties<std::string>::TypeProperties::
       IsUndefinedValue(value) == false)
    {
      rValue = mtProperties<std::string>::TypeProperties::
               GetWrappedValue(value);
    }
  }
}

/*
TileAlgebra operator atinstant returns all values of a mtstring object
at given Instant value.

author: Dirk Zacher
parameters: rInstant - reference to an Instant value of time dimension
            ritstring - reference to an itstring object containing all values
                        of the mtstring object at given Instant value
return value: -
exceptions: -

*/

void mtstring::atinstant(const Instant& rInstant,
                         itstring& ritstring) const
{
  ritstring.SetDefined(false);

  tstring tstring(true);
  tstring.SetGrid(m_Grid);

  int xDimensionSize = mtProperties<std::string>::GetXDimensionSize();
  int yDimensionSize = mtProperties<std::string>::GetYDimensionSize();
  double gridDuration = m_Grid.GetDuration().ToDouble();
  int time = static_cast<int>(rInstant.ToDouble() / gridDuration);
  bool bmtstringDefined = false;

  for(int row = 0; row < yDimensionSize; row++)
  {
    for(int column = 0; column < xDimensionSize; column++)
    {
      Index<3> index3 = (int[]){column, row, time};
      std::string value = GetValue(index3);

      if(mtProperties<std::string>::TypeProperties::
         IsUndefinedValue(value) == false)
      {
        bmtstringDefined = true;
        Index<2> index2 = (int[]){column, row};
        tstring.SetValue(index2, value, true);
      }
    }
  }
  
  if(bmtstringDefined == false)
  {
    tstring.SetDefined(false);
  }

  ritstring.SetInstant(rInstant);
  ritstring.SetValues(tstring);
}

/*
TileAlgebra operator atperiods returns all values of a mtstring object
at given periods.

author: Dirk Zacher
parameters: rPeriods - reference to a Periods object
            rmtstring - reference to a mtstring object containing all values
                        of the mtstring object at given periods
return value: -
exceptions: -

*/

void mtstring::atperiods(const Periods& rPeriods,
                         mtstring& rmtstring)
                         const
{
  rmtstring.SetDefined(false);

  if(rPeriods.IsDefined())
  {
    rmtstring.SetDefined(true);
    bool bOK = rmtstring.SetGrid(m_Grid);

    if(bOK == true)
    {
      int xDimensionSize = mtProperties<std::string>::GetXDimensionSize();
      int yDimensionSize = mtProperties<std::string>::GetYDimensionSize();
      int tDimensionSize = mtProperties<std::string>::GetTDimensionSize();
      datetime::DateTime gridDuration = m_Grid.GetDuration();
      double duration = gridDuration.ToDouble();

      for(int time = 0; time < tDimensionSize; time++)
      {
        for(int row = 0; row < yDimensionSize; row++)
        {
          for(int column = 0; column < xDimensionSize; column++)
          {
            Index<3> index = (int[]){column, row, time};
            std::string value = GetValue(index);

            if(mtProperties<std::string>::TypeProperties::
               IsUndefinedValue(value) == false)
            {
              datetime::DateTime startTime = time * duration;
              datetime::DateTime endTime = (time + 1) * duration;

              Interval<DateTime> timeInterval(startTime, endTime, true, false);

              if(rPeriods.Contains(timeInterval))
              {
                bOK = rmtstring.SetValue(index, value, true);
              }

              else
              {
                if(rPeriods.Intersects(timeInterval) ||
                   rPeriods.Inside(timeInterval))
                {
                  Range<datetime::DateTime> range(2);
                  rPeriods.Intersection(timeInterval, range);

                  Interval<DateTime> rangeValue(startTime, endTime,
                                                true, false);
                  range.Get(0, rangeValue);
                  datetime::DateTime rangeLength = rangeValue.end -
                                                   rangeValue.start;

                  if(rangeLength >= (gridDuration * 0.5))
                  {
                    bOK = rmtstring.SetValue(index, value, true);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

/*
TileAlgebra operator atrange returns all values of a mtstring object
inside the given rectangle.

author: Dirk Zacher
parameters: rRectangle - reference to a Rectangle<2> object
            rmtstring - reference to a mtstring object containing all values
                        of the mtstring object inside the given rectangle
return value: -
exceptions: -

*/

void mtstring::atrange(const Rectangle<2>& rRectangle,
                       mtstring& rmtstring) const
{
  rmtstring.SetDefined(false);

  if(rRectangle.IsDefined())
  {
    double instant1 = 0;
    double instant2 = (mtProperties<std::string>::GetTDimensionSize() - 1) *
                       m_Grid.GetDuration().ToDouble();

    atrange(rRectangle, instant1, instant2, rmtstring);
  }
}

/*
TileAlgebra operator atrange returns all values of a mtstring object
inside the given rectangle between first given Instant value
and second given Instant value.

author: Dirk Zacher
parameters: rRectangle - reference to a Rectangle<2> object
            rInstant1 - reference to the first Instant value
            rInstant2 - reference to the second Instant value
            rmtstring - reference to a mtstring object containing all values
                        of the mtstring object inside the given rectangle
                        between rInstant1 and rInstant2
return value: -
exceptions: -

*/

void mtstring::atrange(const Rectangle<2>& rRectangle,
                       const double& rInstant1,
                       const double& rInstant2,
                       mtstring& rmtstring) const
{
  rmtstring.SetDefined(false);

  if(rRectangle.IsDefined())
  {
    if(IsValidLocation(rRectangle.MinD(0), rRectangle.MinD(1), rInstant1) &&
       IsValidLocation(rRectangle.MaxD(0), rRectangle.MaxD(1), rInstant2))
    {
      rmtstring.SetDefined(true);

      double x = m_Grid.GetX();
      double y = m_Grid.GetY();
      double length = m_Grid.GetLength();
      datetime::DateTime gridDuration = m_Grid.GetDuration();
      double duration = gridDuration.ToDouble();
      rmtstring.SetGrid(m_Grid);

      Index<3> startIndex = GetLocationIndex(rRectangle.MinD(0),
                                             rRectangle.MinD(1),
                                             rInstant1);
      Index<3> endIndex = GetLocationIndex(rRectangle.MaxD(0),
                                           rRectangle.MaxD(1),
                                           rInstant2);

      for(int time = startIndex[2]; time <= endIndex[2]; time++)
      {
        for(int row = startIndex[1]; row <= endIndex[1]; row++)
        {
          for(int column = startIndex[0]; column <= endIndex[0]; column++)
          {
            if(rRectangle.MinD(0) <= (x + column * length) &&
               rRectangle.MaxD(0) >= (x + column * length) &&
               rRectangle.MinD(1) <= (y + row * length) &&
               rRectangle.MaxD(1) >= (y + row * length)&&
               rInstant1 <= (time * duration) &&
               rInstant2 >= (time * duration))
            {
              Index<3> index = (int[]){column, row, time};
              std::string value = GetValue(index);

              if(mtProperties<std::string>::TypeProperties::
                 IsUndefinedValue(value) == false)
              {
                rmtstring.SetValue(index, value, true);
              }
            }
          }
        }
      }
    }
  }
}

/*
TileAlgebra operator minimum returns the minimum value of mtstring object.

author: Dirk Zacher
parameters: -
return value: minimum value of mtstring object
exceptions: -

*/

std::string mtstring::minimum() const
{
  std::string minimum;

  if(mtProperties<int>::TypeProperties::IsUndefinedValue(m_Minimum) == false)
  {
    bool bOK = m_UniqueStringArray.GetUniqueString(m_Minimum, minimum);
    assert(bOK);
  }

  return minimum;
}

/*
TileAlgebra operator maximum returns the maximum value of mtstring object.

author: Dirk Zacher
parameters: -
return value: maximum value of mtstring object
exceptions: -

*/

std::string mtstring::maximum() const
{
  std::string maximum;

  if(mtProperties<int>::TypeProperties::IsUndefinedValue(m_Maximum) == false)
  {
    bool bOK = m_UniqueStringArray.GetUniqueString(m_Maximum, maximum);
    assert(bOK);
  }

  return maximum;
}

/*
Method GetValue returns the string value of mtstring object
at given 3-dimensional index.

author: Dirk Zacher
parameters: rIndex - reference to a 3-dimensional index
return value: string value of mtstring object at given 3-dimensional index
exceptions: -

*/

std::string mtstring::GetValue(const Index<3>& rIndex) const
{
  std::string value = mtProperties<std::string>::TypeProperties::
                      GetUndefinedValue();

  int intValue = mtint::GetValue(rIndex);

  if(mtProperties<int>::TypeProperties::IsUndefinedValue(intValue) == false)
  {
    bool bOK = m_UniqueStringArray.GetUniqueString(intValue, value);
    assert(bOK);
  }

  return value;
}

/*
Method SetValue sets a string value of mtstring object at given index and
recalculates minimum and maximum of mtstring object if bSetExtrema is true.

author: Dirk Zacher
parameters: rIndex - reference to a 3-dimensional index
            rValue - reference to a string value
            bSetExtrema - flag that indicates if minimum and maximum
                          of mtstring object should be recalculated
return value: true, if rValue was successfully set at rIndex, otherwise false
exceptions: -

*/

bool mtstring::SetValue(const Index<3>& rIndex,
                        const std::string& rValue,
                        bool bSetExtrema)
{
  bool bRetVal = false;

  int xDimensionSize = mtProperties<std::string>::GetXDimensionSize();
  int yDimensionSize = mtProperties<std::string>::GetYDimensionSize();
  int tDimensionSize = mtProperties<std::string>::GetTDimensionSize();

  if(IsDefined() &&
     rIndex[0] >= 0 &&
     rIndex[0] < xDimensionSize &&
     rIndex[1] >= 0 &&
     rIndex[1] < yDimensionSize &&
     rIndex[2] >= 0 &&
     rIndex[2] < tDimensionSize)
  {
    int stringIndex = m_UniqueStringArray.AddString(rValue);

    if(stringIndex != UNDEFINED_STRING_INDEX)
    {
      bRetVal = mtint::SetValue(rIndex, stringIndex, false);
    }

    if(bSetExtrema == true)
    {
      if(mtProperties<int>::TypeProperties::IsUndefinedValue(m_Minimum) ||
         rValue < minimum())
      {
        m_Minimum = stringIndex;
      }

      if(mtProperties<int>::TypeProperties::IsUndefinedValue(m_Maximum) ||
         rValue > maximum())
      {
        m_Maximum = stringIndex;
      }
    }
  }

  return bRetVal;
}

/*
Method SetValue sets a string value of mtstring object at given location
rX and rY at given Instant value and recalculates minimum and maximum
of mtstring object if bSetExtrema is true.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
            rInstant - reference to an Instant value of time dimension
            rValue - reference to a string value
            bSetExtrema - flag that indicates if minimum and maximum
                          of mtstring object should be recalculated
return value: true, if rValue was successfully set at given location
              rX and rY at given Instant value, otherwise false
exceptions: -

*/

bool mtstring::SetValue(const double& rX,
                        const double& rY,
                        const double& rInstant,
                        const std::string& rValue,
                        bool bSetExtrema)
{
  bool bRetVal = false;

  if(IsDefined() &&
     IsValidLocation(rX, rY, rInstant))
  {
    Index<3> index = GetLocationIndex(rX, rY, rInstant);
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

bool mtstring::Adjacent(const Attribute* pAttribute) const
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

Attribute* mtstring::Clone() const
{
  Attribute* pAttribute = new mtstring(*this);
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

int mtstring::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;

  if(pAttribute != 0)
  {
    const mtstring* pmtstring = dynamic_cast<const mtstring*>(pAttribute);

    if(pmtstring != 0)
    {
      bool bIsDefined = IsDefined();
      bool bmtstringIsDefined = pmtstring->IsDefined();

      if(bIsDefined == true)
      {
        if(bmtstringIsDefined == true) // defined x defined
        {
          nRetVal = mtint::Compare(pmtstring);

          if(nRetVal == 0)
          {
            nRetVal = m_UniqueStringArray.Compare(
                      &(pmtstring->m_UniqueStringArray));
          }
        }

        else // defined x undefined
        {
          nRetVal = 1;
        }
      }

      else
      {
        if(bmtstringIsDefined == true) // undefined x defined
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

void mtstring::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const mtstring* pmtstring = dynamic_cast<const mtstring*>(pAttribute);

    if(pmtstring != 0)
    {
      *this = *pmtstring;
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

Flob* mtstring::GetFLOB(const int i)
{ 
  Flob* pFlob = 0;
  int nFlobs = NumOfFLOBs();
  
  if(i >= 0 &&
     i < nFlobs)
  {
    switch(i)
    {
      case 0:   pFlob = mtint::GetFLOB(i);
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
Method HashValue returns the hash value of the mtstring object.

author: Dirk Zacher
parameters: -
return value: hash value of the mtstring object
exceptions: -

*/

size_t mtstring::HashValue() const
{
  size_t hashValue = 0;

  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }

  return hashValue;
}

/*
Method NumOfFLOBs returns the number of Flobs of a mtstring object.

author: Dirk Zacher
parameters: -
return value: number of Flobs of a mtstring object
exceptions: -

*/

int mtstring::NumOfFLOBs() const
{ 
  int nNumberOfFLOBs = mtint::NumOfFLOBs() +
                       m_UniqueStringArray.NumOfFLOBs();

  return nNumberOfFLOBs;
}

/*
Method Sizeof returns the size of mtstring datatype.

author: Dirk Zacher
parameters: -
return value: size of mtstring datatype
exceptions: -

*/

size_t mtstring::Sizeof() const
{
  return sizeof(mtstring);
}

/*
Method BasicType returns the typename of mtstring datatype.

author: Dirk Zacher
parameters: -
return value: typename of mtstring datatype
exceptions: -

*/

const std::string mtstring::BasicType()
{
  return mtProperties<std::string>::GetTypeName();
}

/*
Method Cast casts a void pointer to a new mtstring object.

author: Dirk Zacher
parameters: pVoid - a pointer to a memory address
return value: a pointer to a new mtstring object
exceptions: -

*/

void* mtstring::Cast(void* pVoid)
{
  return new(pVoid)mtstring;
}

/*
Method Clone clones an existing mtstring object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing mtstring object
return value: a Word that references a new mtstring object
exceptions: -

*/

Word mtstring::Clone(const ListExpr typeInfo,
                     const Word& rWord)
{
  Word word;

  mtstring* pmtstring = static_cast<mtstring*>(rWord.addr);

  if(pmtstring != 0)
  {
    word.addr = new mtstring(*pmtstring);
    assert(word.addr != 0);
  }

  return word;
}

/*
Method Close closes an existing mtstring object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing mtstring object
return value: -
exceptions: -

*/

void mtstring::Close(const ListExpr typeInfo,
                     Word& rWord)
{
  mtstring* pmtstring = static_cast<mtstring*>(rWord.addr);

  if(pmtstring != 0)
  {
    delete pmtstring;
    rWord.addr = 0;
  }
}

/*
Method Create creates a new mtstring object.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of the new mtstring object to create
return value: a Word that references a new mtstring object
exceptions: -

*/

Word mtstring::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new mtstring(true);
  assert(word.addr != 0);

  return word;
}

/*
Method Delete deletes an existing mtstring object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing mtstring object
return value: -
exceptions: -

*/

void mtstring::Delete(const ListExpr typeInfo,
                     Word& rWord)
{
  mtstring* pmtstring = static_cast<mtstring*>(rWord.addr);

  if(pmtstring != 0)
  {
    delete pmtstring;
    rWord.addr = 0;
  }
}

/*
Method GetTypeConstructor returns the TypeConstructor of class mtstring.

author: Dirk Zacher
parameters: -
return value: TypeConstructor of class mtstring
exceptions: -

*/

TypeConstructor mtstring::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    mtstring::BasicType(), // type name function
    mtstring::Property,    // property function describing signature
    mtstring::Out,         // out function
    mtstring::In,          // in function
    0,                     // save to list function
    0,                     // restore from list function
    mtstring::Create,      // create function
    mtstring::Delete,      // delete function
    mtstring::Open,        // open function
    mtstring::Save,        // save function
    mtstring::Close,       // close function
    mtstring::Clone,       // clone function
    mtstring::Cast,        // cast function
    mtstring::SizeOfObj,   // sizeofobj function
    mtstring::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

/*
Method In creates a new mtstring object on the basis of a given ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object to create on the basis of instance
            instance - ListExpr of the mtstring object to create
            errorPos - error position
            rErrorInfo - reference to error information
            rCorrect - flag that indicates if mtstring object correctly created
return value: a Word that references a new mtstring object
exceptions: -

*/

Word mtstring::In(const ListExpr typeInfo,
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

    if(gridList.length() == 4)
    {
      if(gridList.isReal(1) &&
         gridList.isReal(2) &&
         gridList.isReal(3) &&
         gridList.isReal(4))
      {
        mtstring* pmtstring = new mtstring(true);

        if(pmtstring != 0)
        {
          datetime::DateTime duration(gridList.elem(4).realval());
          duration.SetType(datetime::durationtype);

          bool bOK = pmtstring->SetGrid(gridList.elem(1).realval(),
                                        gridList.elem(2).realval(),
                                        gridList.elem(3).realval(),
                                        duration);

          if(bOK == true)
          {
            instanceList.rest();

            if(instanceList.isEmpty() == false)
            {
              NList sizeList = instanceList.elem(1);

              if(sizeList.length() == 3)
              {
                if(sizeList.isInt(1) &&
                   sizeList.isInt(2) &&
                   sizeList.isInt(3) &&
                   sizeList.elem(1).intval() > 0 &&
                   sizeList.elem(2).intval() > 0 &&
                   sizeList.elem(3).intval() > 0)
                {
                  int sizeX = sizeList.elem(1).intval();
                  int sizeY = sizeList.elem(2).intval();
                  int sizeT = sizeList.elem(3).intval();
                  Cardinal valueListLength = static_cast<Cardinal>
                                             (sizeX * sizeY * sizeT);

                  instanceList.rest();

                  while(bOK &&
                        instanceList.isEmpty() == false)
                  {
                    NList pageList = instanceList.first();

                    if(pageList.length() == 4)
                    {
                      if(pageList.isInt(1) &&
                         pageList.isInt(2) &&
                         pageList.isInt(3))
                      {
                        int indexX = pageList.elem(1).intval();
                        int indexY = pageList.elem(2).intval();
                        int indexT = pageList.elem(3).intval();
                        int xDimensionSize = mtProperties<std::string>::
                                             GetXDimensionSize();
                        int yDimensionSize = mtProperties<std::string>::
                                             GetYDimensionSize();
                        int tDimensionSize = mtProperties<std::string>::
                                             GetTDimensionSize();

                        if(indexX >= 0 &&
                           indexX <= xDimensionSize - sizeX &&
                           indexY >= 0 &&
                           indexY <= yDimensionSize - sizeY &&
                           indexT >= 0 &&
                           indexT <= tDimensionSize - sizeT)
                        {
                          pageList.rest();
                          pageList.rest();
                          pageList.rest();

                          NList valueList = pageList.first();

                          if(valueList.length() == valueListLength)
                          {
                            int listIndex = 0;

                            for(int time = 0; time < sizeT; time++)
                            {
                              for(int row = 0; row < sizeY; row++)
                              {
                                for(int column = 0; column < sizeX; column++)
                                {
                                  listIndex++;

                                  Index<3> index = (int[]){(indexX + column),
                                                           (indexY + row),
                                                           (indexT + time)};
                                  std::string stringValue = mtProperties
                                                            <std::string>::
                                                            TypeProperties::
                                                            GetUndefinedValue();

                                  if(valueList.elem(listIndex).
                                     isSymbol(Symbol::UNDEFINED()) == false)
                                  {
                                    if(mtProperties<std::string>::
                                       TypeProperties::IsValidValueType
                                      (valueList.elem(listIndex)))
                                    {
                                      stringValue = mtProperties<std::string>::
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

                                  if(mtProperties<std::string>::TypeProperties::
                                     IsUndefinedValue(stringValue) == false)
                                  {
                                    bOK = pmtstring->SetValue(index,
                                                              stringValue,
                                                              true);
                                  }
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
                                        "with three integers.");
                      }
                    }

                    else
                    {
                      bOK = false;
                      cmsg.inFunError("Type mismatch: "
                                      "partial grid content must contain "
                                      "four elements.");
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
                cmsg.inFunError("Size list must have a length of 3.");
              }
            }
          }

          if(bOK)
          {
            word.addr = pmtstring;
            rCorrect = true;
          }

          else
          {
            delete pmtstring;
            pmtstring = 0;
            rCorrect = false;
          }
        }
      }

      else
      {
        cmsg.inFunError("Type mismatch: expected 4 reals as mtgrid sublist.");
      }
    }

    else
    {
      cmsg.inFunError("Type mismatch: list for mtgrid is too short "
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
Method KindCheck checks if given type is mtstring type.

author: Dirk Zacher
parameters: type - ListExpr of type to check
            rErrorInfo - reference to error information
return value: true, if type is mtstring type, otherwise false
exceptions: -

*/

bool mtstring::KindCheck(ListExpr type,
                         ListExpr& rErrorInfo)
{
  bool bRetVal = false;

  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, mtstring::BasicType());
  }

  return bRetVal;
}

/*
Method Open opens an mtstring object from a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord containing mtstring object to open
            rOffset - Offset to the mtstring object in SmiRecord
            typeInfo - TypeInfo of mtstring object to open
            rValue - reference to a Word referencing the opened mtstring object
return value: true, if mtstring object was successfully opened, otherwise false
exceptions: -

*/

bool mtstring::Open(SmiRecord& rValueRecord,
                    size_t& rOffset,
                    const ListExpr typeInfo,
                    Word& rValue)
{
  bool bRetVal = OpenAttribute<mtstring>(rValueRecord,
                                         rOffset,
                                         typeInfo,
                                         rValue);

  return bRetVal;
}

/*
Method Out writes out an existing mtstring object in the form of a ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of mtstring object to write out
            value - reference to a Word referencing the mtstring object
return value: ListExpr of mtstring object referenced by value
exceptions: -

*/

ListExpr mtstring::Out(ListExpr typeInfo,
                       Word value)
{
  ListExpr pListExpr = 0;

  if(nl != 0)
  {
    mtstring* pmtstring = static_cast<mtstring*>(value.addr);

    if(pmtstring != 0)
    {
      if(pmtstring->IsDefined() == true)
      {
        NList instanceList;

        NList gridList;
        gridList.append(pmtstring->m_Grid.GetX());
        gridList.append(pmtstring->m_Grid.GetY());
        gridList.append(pmtstring->m_Grid.GetLength());
        gridList.append(pmtstring->m_Grid.GetDuration().ToDouble());
        instanceList.append(gridList);

        NList sizeList;
        sizeList.append(mtProperties<std::string>::GetXDimensionSize());
        sizeList.append(mtProperties<std::string>::GetYDimensionSize());
        sizeList.append(mtProperties<std::string>::GetTDimensionSize());
        instanceList.append(sizeList);

        NList tintList;
        tintList.append(0);
        tintList.append(0);
        tintList.append(0);

        std::string undefinedStringValue = mtProperties<std::string>::
                                           TypeProperties::GetUndefinedValue();
        NList valueList;

        for(int i = 0; i < mtProperties<std::string>::GetFlobElements(); i++)
        {
          int stringIndex = UNDEFINED_STRING_INDEX;
          std::string stringValue = undefinedStringValue;

          bool bOK = pmtstring->m_Flob.read(reinterpret_cast<char*>
                                            (&stringIndex),
                                            sizeof(int),
                                            i * sizeof(int));
          
          if(bOK &&
             stringIndex != UNDEFINED_INT)
          {
            bOK = pmtstring->m_UniqueStringArray.GetUniqueString(stringIndex,
                                                                 stringValue);
          }

          valueList.append(mtProperties<std::string>::TypeProperties::
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
Method Property returns all properties of mtstring datatype.

author: Dirk Zacher
parameters: -
return value: properties of mtstring datatype in the form of a ListExpr
exceptions: -

*/

ListExpr mtstring::Property()
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
               (std::string("((x y l t) (szx szy szt) ((ix iy it (v*)))*)"),
                true));
  values.append(NList
               (std::string("((0.0 0.0 1.0 1.0) (1 1 1) ((0 0 0 (\"A\"))))"),
                true));
  values.append(NList(std::string(""), true));

  propertyList = NList(names, values);

  return propertyList.listExpr();
}

/*
Method Save saves an existing mtstring object in a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord to save existing mtstring object
            rOffset - Offset to save position of mtstring object in SmiRecord
            typeInfo - TypeInfo of mtstring object to save
            rValue - reference to a Word referencing the mtstring object to save
return value: true, if mtstring object was successfully saved, otherwise false
exceptions: -

*/

bool mtstring::Save(SmiRecord& rValueRecord,
                    size_t& rOffset,
                    const ListExpr typeInfo,
                    Word& rValue)
{
  bool bRetVal = SaveAttribute<mtstring>(rValueRecord,
                                         rOffset,
                                         typeInfo,
                                         rValue);

  return bRetVal;
}

/*
Method SizeOfObj returns the size of a mtstring object.

author: Dirk Zacher
parameters: -
return value: size of a mtstring object
exceptions: -

*/

int mtstring::SizeOfObj()
{
  return sizeof(mtstring);
}

/*
Method GetXDimensionSize returns the size of x dimension of datatype mtstring.

author: Dirk Zacher
parameters: -
return value: size of x dimension of datatype mtstring
exceptions: -

*/

int mtProperties<std::string>::GetXDimensionSize()
{
  int xDimensionSize = static_cast<unsigned int>
                       (std::pow((WinUnix::getPageSize() -
                                  sizeof(mtgrid) -
                                  2 * sizeof(int)) /
                                  sizeof(int),
                                  0.5)
                       );

  return xDimensionSize;
}

/*
Method GetYDimensionSize returns the size of y dimension of datatype mtstring.

author: Dirk Zacher
parameters: -
return value: size of y dimension of datatype mtstring
exceptions: -

*/

int mtProperties<std::string>::GetYDimensionSize()
{
  int yDimensionSize = static_cast<unsigned int>
                       (std::pow((WinUnix::getPageSize() -
                                  sizeof(mtgrid) -
                                  2 * sizeof(int)) /
                                  sizeof(int),
                                  0.5)
                       );

  return yDimensionSize;
}

/*
Method GetTDimensionSize returns the size of time dimension of datatype mtstring.

author: Dirk Zacher
parameters: -
return value: size of time dimension of datatype mtstring
exceptions: -

*/

int mtProperties<std::string>::GetTDimensionSize()
{
  return TIME_DIMENSION_SIZE;
}

/*
Method GetFlobElements returns the number of flob elements of datatype mtstring.

author: Dirk Zacher
parameters: -
return value: number of flob elements of datatype mtstring
exceptions: -

*/

int mtProperties<std::string>::GetFlobElements()
{
  int flobElements = GetXDimensionSize() *
                     GetYDimensionSize() *
                     GetTDimensionSize();

  return flobElements;
}

/*
Method GetFlobSize returns the size of the flob of datatype mtstring.

author: Dirk Zacher
parameters: -
return value: size of the flob of datatype mtstring
exceptions: -

*/

SmiSize mtProperties<std::string>::GetFlobSize()
{
  SmiSize flobSize = GetFlobElements() * sizeof(int);

  return flobSize;
}

/*
Method GetTypeName returns the typename of datatype mtstring.

author: Dirk Zacher
parameters: -
return value: typename of datatype mtstring
exceptions: -

*/

std::string mtProperties<std::string>::GetTypeName()
{
  return TYPE_NAME_MTSTRING;
}

}
