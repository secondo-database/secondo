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

#ifndef TILEALGEBRA_MT_H
#define TILEALGEBRA_MT_H

/*
SECONDO includes

*/

#include "Attribute.h"
#include "RectangleAlgebra.h"
#include "TemporalAlgebra.h"
#include "TypeConstructor.h"
#include "../../Tools/Flob/Flob.h"

/*
TileAlgebra includes

*/

#include "mtProperties.h"
#include "../grid/mtgrid.h"
#include "../Index/Index.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Template class mt represents the base implementation
for datatypes mtint, mtreal, mtbool and mtstring.

author: Dirk Zacher

*/

template <typename Type, typename Properties = mtProperties<Type> >
class mt : public Attribute
{
  protected:

  /*
  Constructor mt does not initialize any members and
  should only be used in conjunction with Cast method.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  mt();

  public:

  /*
  Constructor mt sets defined flag of base class Attribute and
  initializes all members of the class with default values.

  author: Dirk Zacher
  parameters: bDefined - defined flag of base class Attribute
  return value: -
  exceptions: -

  */

  mt(bool bDefined);

  /*
  Constructor mt sets defined flag of base class Attribute to defined flag
  of rmt object and initializes all members of the class with corresponding
  values of rmt object.

  author: Dirk Zacher
  parameters: rmt - reference to a mt object
  return value: -
  exceptions: -

  */

  mt(const mt& rmt);

  /*
  Destructor ~mt deinitializes a mt object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  virtual ~mt();

  /*
  Operator= assigns all member values of a given mt object
  to the corresponding member values of this object.

  author: Dirk Zacher
  parameters: rmt - reference to a mt object
  return value: reference to this object
  exceptions: -

  */

  mt& operator=(const mt& rmt);

  /*
  TileAlgebra operator atlocation returns the time dependent values
  of a mt object at given location rX and rY.

  author: Dirk Zacher
  parameters: rX - reference to location of dimension x
              rY - reference to location of dimension y
              rValues - reference to a moving type object containing
                        the time dependent values at given location rX and rY
  return value: -
  exceptions: -

  */

  void atlocation(const double& rX,
                  const double& rY,
                  typename Properties::TypeProperties::MType& rValues) const;

  /*
  TileAlgebra operator atlocation returns the value of a mt object
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

  void atlocation(const double& rX,
                  const double& rY,
                  const double& rInstant,
                  typename Properties::TypeProperties::WrapperType& rValue)
                  const;

  /*
  TileAlgebra operator atinstant returns all values of a mt object
  at given Instant value.

  author: Dirk Zacher
  parameters: rInstant - reference to an Instant value of time dimension
              rit - reference to an it object containing all values
                    of the mt object at given Instant value
  return value: -
  exceptions: -

  */

  void atinstant(const Instant& rInstant,
                 typename Properties::itType& rit) const;

  /*
  TileAlgebra operator atperiods returns all values of a mt object
  at given periods.

  author: Dirk Zacher
  parameters: rPeriods - reference to a Periods object
              rmt - reference to a mt object containing all values
                    of the mt object at given periods
  return value: -
  exceptions: -

  */

  void atperiods(const Periods& rPeriods,
                 typename Properties::PropertiesType& rmt) const;

  /*
  TileAlgebra operator atrange returns all values of a mt object
  inside the given rectangle.

  author: Dirk Zacher
  parameters: rRectangle - reference to a Rectangle<2> object
              rmt - reference to a mt object containing all values
                    of the mt object inside the given rectangle
  return value: -
  exceptions: -

  */

  void atrange(const Rectangle<2>& rRectangle,
               typename Properties::PropertiesType& rmt) const;

  /*
  TileAlgebra operator atrange returns all values of a mt object
  inside the given rectangle between first given Instant value
  and second given Instant value.

  author: Dirk Zacher
  parameters: rRectangle - reference to a Rectangle<2> object
              rInstant1 - reference to the first Instant value
              rInstant2 - reference to the second Instant value
              rmt - reference to a mt object containing all values
                    of the mt object inside the given rectangle
                    between rInstant1 and rInstant2
  return value: -
  exceptions: -

  */

  void atrange(const Rectangle<2>& rRectangle,
               const double& rInstant1,
               const double& rInstant2,
               typename Properties::PropertiesType& rmt) const;

  /*
  TileAlgebra operator deftime returns the defined periods of a mt object.

  author: Dirk Zacher
  parameters: rPeriods - reference to a Periods object containing
                         all defined Periods of mt object.
  return value: -
  exceptions: -

  */

  void deftime(Periods& rPeriods) const;

  /*
  TileAlgebra operator bbox returns the bounding box of a mt object.

  author: Dirk Zacher
  parameters: rBoundingBox - reference to a Properties::RectangleType object
                             containing the bounding box of the mt object.
  return value: -
  exceptions: -

  */

  void bbox(typename Properties::RectangleType& rBoundingBox) const;

  /*
  TileAlgebra operator minimum returns the minimum value of mt object.

  author: Dirk Zacher
  parameters: rMinimum - reference to a Type object containing
                         the minimum value of mt object
  return value: -
  exceptions: -

  */

  void minimum(Type& rMinimum) const;

  /*
  TileAlgebra operator maximum returns the maximum value of mt object.

  author: Dirk Zacher
  parameters: rMaximum - reference to a Type object containing
                         the maximum value of mt object
  return value: -
  exceptions: -

  */

  void maximum(Type& rMaximum) const;

  /*
  TileAlgebra operator getgrid returns the mtgrid object of mt object.

  author: Dirk Zacher
  parameters: rmtgrid - reference to a mtgrid object containing
                        mtgrid object of mt object
  return value: -
  exceptions: -

  */

  void getgrid(mtgrid& rmtgrid) const;

  /*
  Method GetBoundingBoxIndexes returns minimum index and maximum index
  of the bounding box of mt object.

  author: Dirk Zacher
  parameters: rMinimumIndex - reference to an Index<3> object containing
                              the minimum index of the bounding box of mt object
              rMaximumIndex - reference to an Index<3> object containing
                              the maximum index of the bounding box of mt object
  return value: true, if minimum index and maximum index of the bounding box
                of mt object successfully calculated, otherwise false
  exceptions: -

  */

  bool GetBoundingBoxIndexes(Index<3>& rMinimumIndex,
                             Index<3>& rMaximumIndex) const;

  /*
  Method GetLocationIndex returns a 2-dimensional index
  of given location rX and rY.

  author: Dirk Zacher
  parameters: rX - reference to location of dimension x
              rY - reference to location of dimension y
  return value: 2-dimensional index of given location rX and rY
  exceptions: -

  */

  Index<2> GetLocationIndex(const double& rX,
                            const double& rY) const;

  /*
  Method GetLocationIndex returns a 3-dimensional index
  of given location rX and rY at given Instant value.

  author: Dirk Zacher
  parameters: rX - reference to location of dimension x
              rY - reference to location of dimension y
              rInstant - reference to an Instant value of time dimension
  return value: 3-dimensional index of given location rX and rY at rInstant
  exceptions: -

  */

  Index<3> GetLocationIndex(const double& rX, const double& rY,
                            const double& rInstant) const;

  /*
  Method GetValue returns the value of mt object at given 3-dimensional index.

  author: Dirk Zacher
  parameters: rIndex - reference to a 3-dimensional index
  return value: value of mt object at given 3-dimensional index
  exceptions: -

  */

  Type GetValue(const Index<3>& rIndex) const;

  /*
  Method IsValidLocation checks if given location rX and rY
  is a valid location inside the mt object.

  author: Dirk Zacher
  parameters: rX - reference to location of dimension x
              rY - reference to location of dimension y
  return value: true, if given location rX and rY is a valid location
                inside the mt object, otherwise false
  exceptions: -

  */

  bool IsValidLocation(const double& rX,
                       const double& rY) const;

  /*
  Method IsValidLocation checks if given location rX and rY
  at given Instant value is a valid location inside the mt object.

  author: Dirk Zacher
  parameters: rX - reference to location of dimension x
              rY - reference to location of dimension y
              rInstant - reference to an Instant value of time dimension
  return value: true, if given location rX and rY at given Instant value
                is a valid location inside the mt object, otherwise false
  exceptions: -

  */

  bool IsValidLocation(const double& rX,
                       const double& rY,
                       const double& rInstant) const;

  /*
  Method SetGrid sets the mtgrid of mt object.

  author: Dirk Zacher
  parameters: rmtgrid - reference to a mtgrid object
  return value: true, if the mtgrid of mt object was successfully set,
                otherwise false
  exceptions: -

  */

  bool SetGrid(const mtgrid& rmtgrid);

  /*
  Method SetGrid sets the mtgrid properties of mt object.

  author: Dirk Zacher
  parameters: rX - reference to the x origin of the grid
              rY - reference to the y origin of the grid
              rLength - reference to the length of a grid cell
              rDuration - reference to duration of the grid
  return value: true, if mtgrid properties of mt object were successfully set,
                otherwise false
  exceptions: -

  */

  bool SetGrid(const double& rX,
               const double& rY,
               const double& rLength,
               const datetime::DateTime& rDuration);

  /*
  Method SetValue sets a value of mt object at given index and
  recalculates minimum and maximum of mt object if bSetExtrema is true.

  author: Dirk Zacher
  parameters: rIndex - reference to a 3-dimensional index
              rValue - reference to a value
              bSetExtrema - flag that indicates if minimum and maximum
                            of mt object should be recalculated
  return value: true, if rValue was successfully set at rIndex, otherwise false
  exceptions: -

  */

  bool SetValue(const Index<3>& rIndex,
                const Type& rValue,
                bool bSetExtrema);

  /*
  Method SetValue sets a value of mt object at given location rX and rY
  at given Instant value and recalculates minimum and maximum of mt object
  if bSetExtrema is true.

  author: Dirk Zacher
  parameters: rX - reference to location of dimension x
              rY - reference to location of dimension y
              rInstant - reference to an Instant value of time dimension
              rValue - reference to a value
              bSetExtrema - flag that indicates if minimum and maximum
                            of mt object should be recalculated
  return value: true, if rValue was successfully set at given location
                rX and rY at given Instant value, otherwise false
  exceptions: -

  */

  bool SetValue(const double& rX,
                const double& rY,
                const double& rInstant,
                const Type& rValue,
                bool bSetExtrema);

  /*
  Method SetValues sets all values of mt object and recalculates
  minimum and maximum of mt object if bSetExtrema is true.

  author: Dirk Zacher
  parameters: rValue - reference to a value
              bSetExtrema - flag that indicates if minimum and maximum
                            of mt object should be recalculated
  return value: true, if all values were successfully set, otherwise false
  exceptions: -

  */

  bool SetValues(const Type& rValue,
                 bool bSetExtrema);

  protected:

  /*
  Method IsValidIndex checks if given index is a valid index
  inside the mt object.

  author: Dirk Zacher
  parameters: rIndex - reference to a 3-dimensional index
  return value: true, if given index is a valid index inside the mt object,
                otherwise false
  exceptions: -

  */

  bool IsValidIndex(const Index<3>& rIndex) const;
  

  public:

  /*
  Method Adjacent checks if this object is adjacent to given Attribute object.

  author: Dirk Zacher
  parameters: pAttribute - a pointer to an Attribute object
  return value: true, if this object is adjacent to pAttribute, otherwise false
  exceptions: -

  */

  virtual bool Adjacent(const Attribute* pAttribute) const;

  /*
  Method Clone returns a copy of this object.

  author: Dirk Zacher
  parameters: -
  return value: a pointer to a copy of this object
  exceptions: -

  */

  virtual Attribute* Clone() const;

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

  virtual int Compare(const Attribute* pAttribute) const;

  /*
  Method CopyFrom assigns all member values of pAttribute object
  to the corresponding member values of this object.

  author: Dirk Zacher
  parameters: pAttribute - a pointer to an Attribute object
  return value: -
  exceptions: -

  */

  virtual void CopyFrom(const Attribute* pAttribute);

  /*
  Method GetFLOB returns a pointer to the Flob with given index.

  author: Dirk Zacher
  parameters: i - index of Flob
  return value: a pointer to the Flob with given index
  exceptions: -

  */

  virtual Flob* GetFLOB(const int i);

  /*
  Method HashValue returns the hash value of the mt object.

  author: Dirk Zacher
  parameters: -
  return value: hash value of the mt object
  exceptions: -

  */

  virtual size_t HashValue() const;

  /*
  Method NumOfFLOBs returns the number of Flobs of a mt object.

  author: Dirk Zacher
  parameters: -
  return value: number of Flobs of a mt object
  exceptions: -

  */

  virtual int NumOfFLOBs() const;

  /*
  Method Sizeof returns the size of mt datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of mt datatype
  exceptions: -

  */

  virtual size_t Sizeof() const;

  /*
  Method BasicType returns the typename of mt datatype.

  author: Dirk Zacher
  parameters: -
  return value: typename of mt datatype
  exceptions: -

  */

  static const std::string BasicType();

  /*
  Method Cast casts a void pointer to a new mt object.

  author: Dirk Zacher
  parameters: pVoid - a pointer to a memory address
  return value: a pointer to a new mt object
  exceptions: -

  */

  static void* Cast(void* pVoid);

  /*
  Method Clone clones an existing mt object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing mt object
  return value: a Word that references a new mt object
  exceptions: -

  */

  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);

  /*
  Method Close closes an existing mt object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing mt object
  return value: -
  exceptions: -

  */

  static void Close(const ListExpr typeInfo,
                    Word& rWord);

  /*
  Method Create creates a new mt object.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of the new mt object to create
  return value: a Word that references a new mt object
  exceptions: -

  */

  static Word Create(const ListExpr typeInfo);

  /*
  Method Delete deletes an existing mt object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing mt object
  return value: -
  exceptions: -

  */

  static void Delete(const ListExpr typeInfo,
                     Word& rWord);

  /*
  Method GetTypeConstructor returns the TypeConstructor of class mt.

  author: Dirk Zacher
  parameters: -
  return value: TypeConstructor of class mt
  exceptions: -

  */

  static TypeConstructor GetTypeConstructor();

  /*
  Method In creates a new mt object on the basis of a given ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object to create on the basis of instance
              instance - ListExpr of the mt object to create
              errorPos - error position
              rErrorInfo - reference to error information
              rCorrect - flag that indicates if mt object correctly created
  return value: a Word that references a new mt object
  exceptions: -

  */

  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);

  /*
  Method KindCheck checks if given type is mt type.

  author: Dirk Zacher
  parameters: type - ListExpr of type to check
              rErrorInfo - reference to error information
  return value: true, if type is mt type, otherwise false
  exceptions: -

  */

  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);

  /*
  Method Open opens an mt object from a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord containing mt object to open
              rOffset - Offset to the mt object in SmiRecord
              typeInfo - TypeInfo of mt object to open
              rValue - reference to a Word referencing the opened mt object
  return value: true, if mt object was successfully opened, otherwise false
  exceptions: -

  */

  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method Out writes out an existing mt object in the form of a ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of mt object to write out
              value - reference to a Word referencing the mt object
  return value: ListExpr of mt object referenced by value
  exceptions: -

  */

  static ListExpr Out(ListExpr typeInfo,
                      Word value);

  /*
  Method Property returns all properties of mt datatype.

  author: Dirk Zacher
  parameters: -
  return value: properties of mt datatype in the form of a ListExpr
  exceptions: -

  */

  static ListExpr Property();

  /*
  Method Save saves an existing mt object in a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord to save existing mt object
              rOffset - Offset to save position of mt object in SmiRecord
              typeInfo - TypeInfo of mt object to save
              rValue - reference to a Word referencing the mt object to save
  return value: true, if mt object was successfully saved, otherwise false
  exceptions: -

  */

  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method SizeOfObj returns the size of a mt object.

  author: Dirk Zacher
  parameters: -
  return value: size of a mt object
  exceptions: -

  */

  static int SizeOfObj();

  protected:

  /*
  Member m_Grid contains the mtgrid object of mt object.

  */

  mtgrid m_Grid;

  /*
  Member m_Minimum contains the minimum value of all values of mt object.

  */

  Type m_Minimum;

  /*
  Member m_Maximum contains the maximum value of all values of mt object.

  */

  Type m_Maximum;

  /*
  Member m_Flob contains the Flob to store all values of mt object.

  */

  Flob m_Flob;
};

/*
Constructor mt does not initialize any members and
should only be used in conjunction with Cast method.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
mt<Type, Properties>::mt()
                     :Attribute()
{

}

/*
Constructor mt sets defined flag of base class Attribute and
initializes all members of the class with default values.

author: Dirk Zacher
parameters: bDefined - defined flag of base class Attribute
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
mt<Type, Properties>::mt(bool bDefined)
                     :Attribute(bDefined),
                      m_Grid(false),
                      m_Minimum(Properties::TypeProperties::
                                GetUndefinedValue()),
                      m_Maximum(Properties::TypeProperties::
                                GetUndefinedValue()),
                      m_Flob(Properties::GetFlobSize())
{
  Type undefinedValue = Properties::TypeProperties::GetUndefinedValue();
  bool bOK = SetValues(undefinedValue, true);
  assert(bOK);
}

/*
Constructor mt sets defined flag of base class Attribute to defined flag
of rmt object and initializes all members of the class with corresponding
values of rmt object.

author: Dirk Zacher
parameters: rmt - reference to a mt object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
mt<Type, Properties>::mt(const mt<Type, Properties>& rmt)
                     :Attribute(rmt.IsDefined()),
                      m_Grid(rmt.m_Grid),
                      m_Minimum(rmt.m_Minimum),
                      m_Maximum(rmt.m_Maximum),
                      m_Flob(rmt.m_Flob.getSize())
{
  m_Flob.copyFrom(rmt.m_Flob);
}

/*
Destructor deinitializes an mt object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
mt<Type, Properties>::~mt()
{

}

/*
Operator= assigns all member values of a given mt object
to the corresponding member values of this object.

author: Dirk Zacher
parameters: rmt - reference to a mt object
return value: reference to this object
exceptions: -

*/

template <typename Type, typename Properties>
mt<Type, Properties>& mt<Type, Properties>::operator=
                                            (const mt<Type, Properties>& rmt)
{
  if(this != &rmt)
  {
    Attribute::operator=(rmt);
    m_Grid = rmt.m_Grid;
    m_Minimum = rmt.m_Minimum;
    m_Maximum = rmt.m_Maximum;

    bool bOK = false;
    bOK = m_Flob.clean();
    assert(bOK);
    bOK = m_Flob.copyFrom(rmt.m_Flob);
    assert(bOK);
  }

  return *this;
}

/*
TileAlgebra operator atlocation returns the time dependent values
of a mt object at given location rX and rY.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
            rValues - reference to a moving type object containing
                      the time dependent values at given location rX and rY
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::atlocation(const double& rX,
                                      const double& rY,
                                      typename Properties::TypeProperties::
                                      MType& rValues) const
{
  rValues.SetDefined(false);

  if(IsDefined() &&
     IsValidLocation(rX, rY))
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
      typename Properties::TypeProperties::WrapperType value;
      atlocation(rX, rY, currentTime.ToDouble(), value);

      if(value.IsDefined())
      {
        Interval<Instant> interval(currentTime, currentTime + duration,
                                   true, false);
        rValues.Add(typename Properties::TypeProperties::UnitType(interval,
                                                                  value,
                                                                  value));
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
TileAlgebra operator atlocation returns the value of a mt object
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

template <typename Type, typename Properties>
void mt<Type, Properties>::atlocation(const double& rX,
                                      const double& rY,
                                      const double& rInstant,
                                      typename Properties::TypeProperties::
                                      WrapperType& rValue) const
{
  rValue.SetDefined(false);

  if(IsDefined() &&
     IsValidLocation(rX, rY, rInstant))
  {
    Index<3> index = GetLocationIndex(rX, rY, rInstant);
    Type value = GetValue(index);

    if(Properties::TypeProperties::IsUndefinedValue(value) == false)
    {
      rValue = Properties::TypeProperties::GetWrappedValue(value);
    }
  }
}

/*
TileAlgebra operator atinstant returns all values of a mt object
at given Instant value.

author: Dirk Zacher
parameters: rInstant - reference to an Instant value of time dimension
            rit - reference to an it object containing all values
                  of the mt object at given Instant value
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::atinstant(const Instant& rInstant,
                                     typename Properties::itType& rit) const
{
  rit.SetDefined(false);
  
  if(IsDefined())
  {
    rit.SetDefined(true);

    typename Properties::tType t(true);
    t.SetGrid(m_Grid);

    int time = static_cast<int>(rInstant.ToDouble() /
                                m_Grid.GetDuration().ToDouble());
    int tDimensionSize = Properties::GetTDimensionSize();
    bool btDefined = false;

    if(time < tDimensionSize)
    {
      Index<3> minimumIndex;
      Index<3> maximumIndex;
      bool bOK = GetBoundingBoxIndexes(minimumIndex, maximumIndex);

      if(bOK == true)
      {
        for(int row = minimumIndex[1]; row < maximumIndex[1]; row++)
        {
          for(int column = minimumIndex[0]; column < maximumIndex[0]; column++)
          {
            Index<3> index3((int[]){column, row, time});
            Type value = GetValue(index3);

            if(Properties::TypeProperties::IsUndefinedValue(value) == false)
            {
              btDefined = true;
              Index<2> index2((int[]){column, row});
              t.SetValue(index2, value, true);
            }
          }
        }
      }
    }

    if(btDefined == false)
    {
      t.SetDefined(false);
    }

    rit.SetInstant(rInstant);
    rit.SetValues(t);
  }
}

/*
TileAlgebra operator atperiods returns all values of a mt object
at given periods.

author: Dirk Zacher
parameters: rPeriods - reference to a Periods object
            rmt - reference to a mt object containing all values
                  of the mt object at given periods
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::atperiods(const Periods& rPeriods,
                                     typename Properties::PropertiesType& rmt)
                                     const
{
  rmt.SetDefined(false);

  if(IsDefined() &&
     rPeriods.IsDefined())
  {
    rmt.SetDefined(true);
    bool bOK = rmt.SetGrid(m_Grid);

    if(bOK == true)
    {
      int tDimensionSize = Properties::GetTDimensionSize();
      datetime::DateTime gridDuration = m_Grid.GetDuration();
      double duration = gridDuration.ToDouble();

      Index<3> minimumIndex;
      Index<3> maximumIndex;
      bOK = GetBoundingBoxIndexes(minimumIndex, maximumIndex);

      if(bOK == true)
      {
        for(int time = 0; time < tDimensionSize; time++)
        {
          for(int row = minimumIndex[1]; row < maximumIndex[1]; row++)
          {
            for(int column = minimumIndex[0]; column < maximumIndex[0];
                column++)
            {
              Index<3> index((int[]){column, row, time});
              Type value = GetValue(index);

              if(Properties::TypeProperties::IsUndefinedValue(value) == false)
              {
                datetime::DateTime startTime = time * duration;
                datetime::DateTime endTime = (time + 1) * duration;

                Interval<DateTime> timeInterval(startTime, endTime,
                                                true, false);

                if(rPeriods.Contains(timeInterval))
                {
                  bOK = rmt.SetValue(index, value, true);
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
                      bOK = rmt.SetValue(index, value, true);
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
}

/*
TileAlgebra operator atrange returns all values of a mt object
inside the given rectangle.

author: Dirk Zacher
parameters: rRectangle - reference to a Rectangle<2> object
            rmt - reference to a mt object containing all values
                  of the mt object inside the given rectangle
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::atrange(const Rectangle<2>& rRectangle,
                                   typename Properties::PropertiesType& rmt)
                                   const
{
  rmt.SetDefined(false);

  if(IsDefined() &&
     rRectangle.IsDefined())
  {
    double instant1 = 0;
    double instant2 = (Properties::GetTDimensionSize() - 1) *
                       m_Grid.GetDuration().ToDouble();

    atrange(rRectangle, instant1, instant2, rmt);
  }
}

/*
TileAlgebra operator atrange returns all values of a mt object
inside the given rectangle between first given Instant value
and second given Instant value.

author: Dirk Zacher
parameters: rRectangle - reference to a Rectangle<2> object
            rInstant1 - reference to the first Instant value
            rInstant2 - reference to the second Instant value
            rmt - reference to a mt object containing all values
                  of the mt object inside the given rectangle
                  between rInstant1 and rInstant2
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::atrange(const Rectangle<2>& rRectangle,
                                   const double& rInstant1,
                                   const double& rInstant2,
                                   typename Properties::PropertiesType& rmt)
                                   const
{
  rmt.SetDefined(false);

  if(IsDefined() &&
     rRectangle.IsDefined())
  {
    if(IsValidLocation(rRectangle.MinD(0), rRectangle.MinD(1), rInstant1) &&
       IsValidLocation(rRectangle.MaxD(0), rRectangle.MaxD(1), rInstant2))
    {
      rmt.SetDefined(true);

      double x = m_Grid.GetX();
      double y = m_Grid.GetY();
      double length = m_Grid.GetLength();
      datetime::DateTime gridDuration = m_Grid.GetDuration();
      double duration = gridDuration.ToDouble();
      rmt.SetGrid(m_Grid);

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
              Index<3> index((int[]){column, row, time});
              Type value = GetValue(index);

              if(Properties::TypeProperties::IsUndefinedValue(value) == false)
              {
                rmt.SetValue(index, value, true);
              }
            }
          }
        }
      }
    }
  }
}

/*
TileAlgebra operator deftime returns the defined periods of a mt object.

author: Dirk Zacher
parameters: rPeriods - reference to a Periods object containing
                       all defined Periods of mt object.
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::deftime(Periods& rPeriods) const
{
  rPeriods.SetDefined(false);

  if(IsDefined())
  {
    Index<3> minimumIndex;
    Index<3> maximumIndex;
    bool bOK = GetBoundingBoxIndexes(minimumIndex, maximumIndex);

    if(bOK == true)
    {
      double duration = m_Grid.GetDuration().ToDouble();
      Periods periods(true);

      periods.StartBulkLoad();

      for(int time = minimumIndex[2]; time < maximumIndex[2]; time++)
      {
        bool bDefined = false;

        for(int row = minimumIndex[1]; row < maximumIndex[1]; row++)
        {
          for(int column = minimumIndex[0]; column < maximumIndex[0]; column++)
          {
            Index<3> index((int[]){column, row, time});
            Type value = GetValue(index);

            if(Properties::TypeProperties::IsUndefinedValue(value) == false)
            {
              bDefined = true;
              break;
            }
          }
          
          if(bDefined == true)
          {
            break;
          }
        }

        if(bDefined == true)
        {
          Instant startTime(time * duration);
          Instant endTime((time + 1) * duration);
          periods.Add(Interval<DateTime>(startTime, endTime, true, false));
        }
      }

      periods.EndBulkLoad();
      periods.Merge(rPeriods);
    }
  }
}

/*
TileAlgebra operator bbox returns the bounding box of a mt object.

author: Dirk Zacher
parameters: rBoundingBox - reference to a Properties::RectangleType object
                           containing the bounding box of the mt object.
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::bbox(typename Properties::RectangleType&
                                rBoundingBox) const
{
  rBoundingBox.SetDefined(false);

  if(IsDefined())
  {
    double minima[3] = { 0.0, 0.0, 0.0 };
    double maxima[3] = { 0.0, 0.0, 0.0 };

    int xDimensionSize = Properties::GetXDimensionSize();
    int yDimensionSize = Properties::GetYDimensionSize();
    int tDimensionSize = Properties::GetTDimensionSize();
    Type value = Properties::TypeProperties::GetUndefinedValue();

    /*
    calculation of x dimension minimum

    */

    for(int column = 0; column < xDimensionSize; column++)
    {
      bool bbreak = false;

      for(int row = 0; row < yDimensionSize; row++)
      {
        for(int time = 0; time < tDimensionSize; time++)
        {
          Index<3> index((int[]){column, row, time});
          value = GetValue(index);

          if(Properties::TypeProperties::IsUndefinedValue(value) == false)
          {
            minima[0] = m_Grid.GetX() + column * m_Grid.GetLength();
            bbreak = true;
            break;
          }
        }

        if(bbreak == true)
        {
          break;
        }
      }

      if(bbreak == true)
      {
        break;
      }
    }

    /*
    calculation of x dimension maximum

    */

    for(int column = xDimensionSize - 1; column >= 0; column--)
    {
      bool bbreak = false;

      for(int row = 0; row < yDimensionSize; row++)
      {
        for(int time = 0; time < tDimensionSize; time++)
        {
          Index<3> index((int[]){column, row, time});
          value = GetValue(index);

          if(Properties::TypeProperties::IsUndefinedValue(value) == false)
          {
            maxima[0] = m_Grid.GetX() + (column + 1) * m_Grid.GetLength();
            bbreak = true;
            break;
          }
        }

        if(bbreak == true)
        {
          break;
        }
      }

      if(bbreak == true)
      {
        break;
      }
    }

    /*
    calculation of y dimension minimum

    */

    for(int row = 0; row < yDimensionSize; row++)
    {
      bool bbreak = false;

      for(int column = 0; column < xDimensionSize; column++)
      {
        for(int time = 0; time < tDimensionSize; time++)
        {
          Index<3> index((int[]){column, row, time});
          value = GetValue(index);

          if(Properties::TypeProperties::IsUndefinedValue(value) == false)
          {
            minima[1] = m_Grid.GetY() + row * m_Grid.GetLength();
            bbreak = true;
            break;
          }
        }

        if(bbreak == true)
        {
          break;
        }
      }

      if(bbreak == true)
      {
        break;
      }
    }

    /*
    calculation of y dimension maximum

    */

    for(int row = yDimensionSize - 1; row >= 0; row--)
    {
      bool bbreak = false;

      for(int column = 0; column < xDimensionSize; column++)
      {
        for(int time = 0; time < tDimensionSize; time++)
        {
          Index<3> index((int[]){column, row, time});
          value = GetValue(index);

          if(Properties::TypeProperties::IsUndefinedValue(value) == false)
          {
            maxima[1] = m_Grid.GetY() + (row + 1) * m_Grid.GetLength();
            bbreak = true;
            break;
          }
        }

        if(bbreak == true)
        {
          break;
        }
      }

      if(bbreak == true)
      {
        break;
      }
    }

    /*
    calculation of time dimension minimum

    */

    for(int time = 0; time < tDimensionSize; time++)
    {
      bool bbreak = false;

      for(int column = 0; column < xDimensionSize; column++)
      {
        for(int row = 0; row < yDimensionSize; row++)
        {
          Index<3> index((int[]){column, row, time});
          value = GetValue(index);

          if(Properties::TypeProperties::IsUndefinedValue(value) == false)
          {
            minima[2] = time * m_Grid.GetDuration().ToDouble();
            bbreak = true;
            break;
          }
        }

        if(bbreak == true)
        {
          break;
        }
      }

      if(bbreak == true)
      {
        break;
      }
    }

    /*
    calculation of time dimension maximum

    */

    for(int time = tDimensionSize - 1; time >= 0; time--)
    {
      bool bbreak = false;

      for(int column = 0; column < xDimensionSize; column++)
      {
        for(int row = 0; row < yDimensionSize; row++)
        {
          Index<3> index((int[]){column, row, time});
          value = GetValue(index);

          if(Properties::TypeProperties::IsUndefinedValue(value) == false)
          {
            maxima[2] = (time + 1) * m_Grid.GetDuration().ToDouble();
            bbreak = true;
            break;
          }
        }

        if(bbreak == true)
        {
          break;
        }
      }

      if(bbreak == true)
      {
        break;
      }
    }

    rBoundingBox.Set(true, minima, maxima);
  }
}

/*
TileAlgebra operator minimum returns the minimum value of mt object.

author: Dirk Zacher
parameters: rMinimum - reference to a Type object containing
                       the minimum value of mt object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::minimum(Type& rMinimum) const
{
  rMinimum = Properties::TypeProperties::GetUndefinedValue();

  if(IsDefined())
  {
    rMinimum = m_Minimum;
  }
}

/*
TileAlgebra operator maximum returns the maximum value of mt object.

author: Dirk Zacher
parameters: rMaximum - reference to a Type object containing
                       the maximum value of mt object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::maximum(Type& rMaximum) const
{
  rMaximum = Properties::TypeProperties::GetUndefinedValue();

  if(IsDefined())
  {
    rMaximum = m_Maximum;
  }
}

/*
TileAlgebra operator getgrid returns the mtgrid object of mt object.

author: Dirk Zacher
parameters: rmtgrid - reference to a mtgrid object containing
                      mtgrid object of mt object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::getgrid(mtgrid& rmtgrid) const
{
  rmtgrid.SetDefined(false);

  if(IsDefined())
  {
    rmtgrid = m_Grid;
  }
}

/*
Method GetBoundingBoxIndexes returns minimum index and maximum index
of the bounding box of mt object.

author: Dirk Zacher
parameters: rMinimumIndex - reference to an Index<3> object containing
                            the minimum index of the bounding box of mt object
            rMaximumIndex - reference to an Index<3> object containing
                            the maximum index of the bounding box of mt object
return value: true, if minimum index and maximum index of the bounding box
              of mt object successfully calculated, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::GetBoundingBoxIndexes(Index<3>& rMinimumIndex,
                                                 Index<3>& rMaximumIndex) const
{
  bool bRetVal = false;

  if(IsDefined())
  {
    typename Properties::RectangleType boundingBox;
    bbox(boundingBox);

    if(boundingBox.IsDefined())
    {
      rMinimumIndex = GetLocationIndex(boundingBox.MinD(0),
                                       boundingBox.MinD(1),
                                       boundingBox.MinD(2));
      rMaximumIndex = GetLocationIndex(boundingBox.MaxD(0),
                                       boundingBox.MaxD(1),
                                       boundingBox.MaxD(2));
      bRetVal = true;
    }
  }

  return bRetVal;
}

/*
Method GetLocationIndex returns a 2-dimensional index
of given location rX and rY.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
return value: 2-dimensional index of given location rX and rY
exceptions: -

*/

template <typename Type, typename Properties>
Index<2> mt<Type, Properties>::GetLocationIndex(const double& rX,
                                                const double& rY) const
{
  double gridX = m_Grid.GetX();
  double gridY = m_Grid.GetY();
  double gridLength = m_Grid.GetLength();
  int indexX = static_cast<int>((rX - gridX) / gridLength);
  int indexY = static_cast<int>((rY - gridY) / gridLength);
  Index<2> locationIndex((int[]){indexX, indexY});

  return locationIndex;
}

/*
Method GetLocationIndex returns a 3-dimensional index
of given location rX and rY at given Instant value.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
            rInstant - reference to an Instant value of time dimension
return value: 3-dimensional index of given location rX and rY at rInstant
exceptions: -

*/

template <typename Type, typename Properties>
Index<3> mt<Type, Properties>::GetLocationIndex(const double& rX,
                                                const double& rY,
                                                const double& rInstant) const
{
  Index<2> index2 = GetLocationIndex(rX, rY);
  double gridDuration = m_Grid.GetDuration().ToDouble();
  int indexT = static_cast<int>(rInstant / gridDuration);
  Index<3> locationIndex((int[]){index2[0], index2[1], indexT});

  return locationIndex;
}

/*
Method GetValue returns the value of mt object at given 3-dimensional index.

author: Dirk Zacher
parameters: rIndex - reference to a 3-dimensional index
return value: value of mt object at given 3-dimensional index
exceptions: -

*/

template <typename Type, typename Properties>
Type mt<Type, Properties>::GetValue(const Index<3>& rIndex) const
{
  Type value = Properties::TypeProperties::GetUndefinedValue();

  if(IsDefined() &&
     IsValidIndex(rIndex))
  {
    int xDimensionSize = Properties::GetXDimensionSize();
    int yDimensionSize = Properties::GetYDimensionSize();
    int flobIndex = rIndex[2] * xDimensionSize * yDimensionSize +
                    rIndex[1] * yDimensionSize + rIndex[0];

    bool bOK = m_Flob.read(reinterpret_cast<char*>(&value),
                           sizeof(Type),
                           flobIndex * sizeof(Type));
    assert(bOK);
  }

  return value;
}

/*
Method IsValidLocation checks if given location rX and rY
is a valid location inside the mt object.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
return value: true, if given location rX and rY is a valid location
              inside the mt object, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::IsValidLocation(const double& rX,
                                           const double& rY) const
{
  bool bIsValidLocation = false;

  int xDimensionSize = Properties::GetXDimensionSize();
  int yDimensionSize = Properties::GetYDimensionSize();
  double gridX = m_Grid.GetX();
  double gridY = m_Grid.GetY();
  double gridLength = m_Grid.GetLength();

  if(rX >= gridX &&
     rX < (gridX + xDimensionSize * gridLength) &&
     rY >= gridY &&
     rY < (gridY + yDimensionSize * gridLength))
  {
    bIsValidLocation = true;
  }

  return bIsValidLocation;
}

/*
Method IsValidLocation checks if given location rX and rY
at given Instant value is a valid location inside the mt object.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
            rInstant - reference to an Instant value of time dimension
return value: true, if given location rX and rY at given Instant value
              is a valid location inside the mt object, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::IsValidLocation(const double& rX,
                                           const double& rY,
                                           const double& rInstant) const
{
  bool bIsValidLocation = IsValidLocation(rX, rY);

  if(bIsValidLocation == true)
  {
    int tDimensionSize = Properties::GetTDimensionSize();
    double gridDuration = m_Grid.GetDuration().ToDouble();

    if(rInstant > 0.0 &&
       rInstant < (tDimensionSize * gridDuration))
   {
      bIsValidLocation = true;
   }
  }

  return bIsValidLocation;
}

/*
Method SetGrid sets the mtgrid of mt object.

author: Dirk Zacher
parameters: rmtgrid - reference to a mtgrid object
return value: true, if the mtgrid of mt object was successfully set,
              otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::SetGrid(const mtgrid& rmtgrid)
{
  bool bRetVal = false;

  if(IsDefined() &&
     rmtgrid.IsDefined())
  {
    m_Grid = rmtgrid;
    bRetVal = true;
  }

  return bRetVal;
}

/*
Method SetGrid sets the mtgrid properties of mt object.

author: Dirk Zacher
parameters: rX - reference to the x origin of the grid
            rY - reference to the y origin of the grid
            rLength - reference to the length of a grid cell
            rDuration - reference to duration of the grid
return value: true, if mtgrid properties of mt object were successfully set,
              otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::SetGrid(const double& rX,
                                   const double& rY,
                                   const double& rLength,
                                   const datetime::DateTime& rDuration)
{
  bool bRetVal = false;

  if(IsDefined())
  {
    m_Grid.SetDefined(true);
    bRetVal  = m_Grid.SetX(rX);
    bRetVal &= m_Grid.SetY(rY);
    bRetVal &= m_Grid.SetLength(rLength);
    bRetVal &= m_Grid.SetDuration(rDuration);
  }

  return bRetVal;
}

/*
Method SetValue sets a value of mt object at given index and
recalculates minimum and maximum of mt object if bSetExtrema is true.

author: Dirk Zacher
parameters: rIndex - reference to a 3-dimensional index
            rValue - reference to a value
            bSetExtrema - flag that indicates if minimum and maximum
                          of mt object should be recalculated
return value: true, if rValue was successfully set at rIndex, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::SetValue(const Index<3>& rIndex,
                                    const Type& rValue,
                                    bool bSetExtrema)
{
  bool bRetVal = false;

  if(IsDefined() &&
     IsValidIndex(rIndex))
  {
    int xDimensionSize = Properties::GetXDimensionSize();
    int yDimensionSize = Properties::GetYDimensionSize();
    int flobIndex = rIndex[2] * xDimensionSize * yDimensionSize +
                    rIndex[1] * yDimensionSize + rIndex[0];

    bRetVal = m_Flob.write(reinterpret_cast<const char*>(&rValue),
                           sizeof(Type),
                           flobIndex * sizeof(Type));

    if(bSetExtrema == true)
    {
      if(Properties::TypeProperties::IsUndefinedValue(m_Minimum) ||
         rValue < m_Minimum)
      {
        m_Minimum = rValue;
      }

      if(Properties::TypeProperties::IsUndefinedValue(m_Maximum) ||
         rValue > m_Maximum)
      {
        m_Maximum = rValue;
      }
    }
  }

  return bRetVal;
}

/*
Method SetValue sets a value of mt object at given location rX and rY
at given Instant value and recalculates minimum and maximum of mt object
if bSetExtrema is true.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
            rInstant - reference to an Instant value of time dimension
            rValue - reference to a value
            bSetExtrema - flag that indicates if minimum and maximum
                          of mt object should be recalculated
return value: true, if rValue was successfully set at given location
              rX and rY at given Instant value, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::SetValue(const double& rX,
                                    const double& rY,
                                    const double& rInstant,
                                    const Type& rValue,
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
Method SetValues sets all values of mt object and recalculates
minimum and maximum of mt object if bSetExtrema is true.

author: Dirk Zacher
parameters: rValue - reference to a value
            bSetExtrema - flag that indicates if minimum and maximum
                          of mt object should be recalculated
return value: true, if all values were successfully set, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::SetValues(const Type& rValue,
                                     bool bSetExtrema)
{
  bool bRetVal = false;
  
  if(IsDefined())
  {
    bRetVal = true;
    
    int flobElements = Properties::GetFlobElements();

    for(int i = 0; i < flobElements; i++)
    {
      bRetVal &= m_Flob.write(reinterpret_cast<const char*>(&rValue),
                              sizeof(Type),
                              i * sizeof(Type));
    }

    if(bSetExtrema == true)
    {
      m_Minimum = rValue;
      m_Maximum = rValue;
    }
  }

  return bRetVal;
}

/*
Method IsValidIndex checks if given index is a valid index
inside the mt object.

author: Dirk Zacher
parameters: rIndex - reference to a 3-dimensional index
return value: true, if given index is a valid index inside the mt object,
              otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::IsValidIndex(const Index<3>& rIndex) const
{
  bool bIsValidIndex = false;

  int xDimensionSize = Properties::GetXDimensionSize();
  int yDimensionSize = Properties::GetYDimensionSize();
  int tDimensionSize = Properties::GetTDimensionSize();

  if(rIndex[0] >= 0 &&
     rIndex[0] < xDimensionSize &&
     rIndex[1] >= 0 &&
     rIndex[1] < yDimensionSize &&
     rIndex[2] >= 0 &&
     rIndex[2] < tDimensionSize)
  {
    bIsValidIndex = true;
  }

  return bIsValidIndex;
}

/*
Method Adjacent checks if this object is adjacent to given Attribute object.

author: Dirk Zacher
parameters: pAttribute - a pointer to an Attribute object
return value: true, if this object is adjacent to pAttribute, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::Adjacent(const Attribute* pAttribute) const
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

template <typename Type, typename Properties>
Attribute* mt<Type, Properties>::Clone() const
{
  Attribute* pAttribute = new mt<Type, Properties>(*this);
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

template <typename Type, typename Properties>
int mt<Type, Properties>::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;

  if(pAttribute != 0)
  {
    const mt<Type, Properties>* pmt = dynamic_cast<const mt<Type, Properties>*>
                                      (pAttribute);

    if(pmt != 0)
    {
      bool bIsDefined = IsDefined();
      bool btIsDefined = pmt->IsDefined();

      if(bIsDefined == true)
      {
        if(btIsDefined == true) // defined x defined
        {
          nRetVal = m_Grid.Compare(&(pmt->m_Grid));

          if(nRetVal == 0)
          {
            SmiSize flobSize = Properties::GetFlobSize();
            
            char buffer1[flobSize];
            m_Flob.read(buffer1, flobSize, 0);

            char buffer2[flobSize];
            pmt->m_Flob.read(buffer2, flobSize, 0);

            nRetVal = memcmp(buffer1, buffer2, flobSize);
          }
        }

        else // defined x undefined
        {
          nRetVal = 1;
        }
      }

      else
      {
        if(btIsDefined == true) // undefined x defined
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

template <typename Type, typename Properties>
void mt<Type, Properties>::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const mt<Type, Properties>* pmt = dynamic_cast<const mt<Type, Properties>*>
                                      (pAttribute);

    if(pmt != 0)
    {
      *this = *pmt;
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

template <typename Type, typename Properties>
Flob* mt<Type, Properties>::GetFLOB(const int i)
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

/*
Method HashValue returns the hash value of the mt object.

author: Dirk Zacher
parameters: -
return value: hash value of the mt object
exceptions: -

*/

template <typename Type, typename Properties>
size_t mt<Type, Properties>::HashValue() const
{
  size_t hashValue = 0;

  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }

  return hashValue;
}

/*
Method NumOfFLOBs returns the number of Flobs of a mt object.

author: Dirk Zacher
parameters: -
return value: number of Flobs of a mt object
exceptions: -

*/

template <typename Type, typename Properties>
int mt<Type, Properties>::NumOfFLOBs() const
{ 
  return 1;
}

/*
Method Sizeof returns the size of mt datatype.

author: Dirk Zacher
parameters: -
return value: size of mt datatype
exceptions: -

*/

template <typename Type, typename Properties>
size_t mt<Type, Properties>::Sizeof() const
{
  return sizeof(mt<Type, Properties>);
}

/*
Method BasicType returns the typename of mt datatype.

author: Dirk Zacher
parameters: -
return value: typename of mt datatype
exceptions: -

*/

template <typename Type, typename Properties>
const std::string mt<Type, Properties>::BasicType()
{
  return Properties::GetTypeName();
}

/*
Method Cast casts a void pointer to a new mt object.

author: Dirk Zacher
parameters: pVoid - a pointer to a memory address
return value: a pointer to a new mt object
exceptions: -

*/

template <typename Type, typename Properties>
void* mt<Type, Properties>::Cast(void* pVoid)
{
  return new(pVoid)mt<Type, Properties>;
}

/*
Method Clone clones an existing mt object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing mt object
return value: a Word that references a new mt object
exceptions: -

*/

template <typename Type, typename Properties>
Word mt<Type, Properties>::Clone(const ListExpr typeInfo,
                                 const Word& rWord)
{
  Word word;

  mt<Type, Properties>* pmt = static_cast<mt<Type, Properties>*>(rWord.addr);

  if(pmt != 0)
  {
    word.addr = new mt<Type, Properties>(*pmt);
    assert(word.addr != 0);
  }

  return word;
}

/*
Method Close closes an existing mt object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing mt object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::Close(const ListExpr typeInfo,
                                Word& rWord)
{
  mt<Type, Properties>* pmt = static_cast<mt<Type, Properties>*>(rWord.addr);

  if(pmt != 0)
  {
    delete pmt;
    rWord.addr = 0;
  }
}

/*
Method Create creates a new mt object.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of the new mt object to create
return value: a Word that references a new mt object
exceptions: -

*/

template <typename Type, typename Properties>
Word mt<Type, Properties>::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new mt<Type, Properties>(true);
  assert(word.addr != 0);

  return word;
}

/*
Method Delete deletes an existing mt object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing mt object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void mt<Type, Properties>::Delete(const ListExpr typeInfo,
                                  Word& rWord)
{
  mt<Type, Properties>* pmt = static_cast<mt<Type, Properties>*>(rWord.addr);

  if(pmt != 0)
  {
    delete pmt;
    rWord.addr = 0;
  }
}

/*
Method GetTypeConstructor returns the TypeConstructor of class mt.

author: Dirk Zacher
parameters: -
return value: TypeConstructor of class mt
exceptions: -

*/

template <typename Type, typename Properties>
TypeConstructor mt<Type, Properties>::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    mt<Type, Properties>::BasicType(), // type name function
    mt<Type, Properties>::Property,    // property function describing signature
    mt<Type, Properties>::Out,         // out function
    mt<Type, Properties>::In,          // in function
    0,                                // save to list function
    0,                                // restore from list function
    mt<Type, Properties>::Create,      // create function
    mt<Type, Properties>::Delete,      // delete function
    mt<Type, Properties>::Open,        // open function
    mt<Type, Properties>::Save,        // save function
    mt<Type, Properties>::Close,       // close function
    mt<Type, Properties>::Clone,       // clone function
    mt<Type, Properties>::Cast,        // cast function
    mt<Type, Properties>::SizeOfObj,   // sizeofobj function
    mt<Type, Properties>::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

/*
Method In creates a new mt object on the basis of a given ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object to create on the basis of instance
            instance - ListExpr of the mt object to create
            errorPos - error position
            rErrorInfo - reference to error information
            rCorrect - flag that indicates if mt object correctly created
return value: a Word that references a new mt object
exceptions: -

*/

template <typename Type, typename Properties>
Word mt<Type, Properties>::In(const ListExpr typeInfo,
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
        mt<Type, Properties>* pmt = new mt<Type, Properties>(true);

        if(pmt != 0)
        {
          datetime::DateTime duration(gridList.elem(4).realval());
          duration.SetType(datetime::durationtype);

          bool bOK = pmt->SetGrid(gridList.elem(1).realval(),
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
                        int xDimensionSize = Properties::GetXDimensionSize();
                        int yDimensionSize = Properties::GetYDimensionSize();
                        int tDimensionSize = Properties::GetTDimensionSize();

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

                                  Index<3> index((int[]){(indexX + column),
                                                         (indexY + row),
                                                         (indexT + time)});
                                  Type value = Properties::TypeProperties::
                                               GetUndefinedValue();

                                  if(valueList.elem(listIndex).
                                     isSymbol(Symbol::UNDEFINED()) == false)
                                  {
                                    if(Properties::TypeProperties::
                                       IsValidValueType
                                       (valueList.elem(listIndex)))
                                    {
                                      value = Properties::TypeProperties::
                                              GetValue(
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

                                  pmt->SetValue(index, value, true);
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
            word.addr = pmt;
            rCorrect = true;
          }

          else
          {
            delete pmt;
            pmt = 0;
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
Method KindCheck checks if given type is mt type.

author: Dirk Zacher
parameters: type - ListExpr of type to check
            rErrorInfo - reference to error information
return value: true, if type is mt type, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::KindCheck(ListExpr type,
                                     ListExpr& rErrorInfo)
{
  bool bRetVal = false;

  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, mt<Type, Properties>::BasicType());
  }

  return bRetVal;
}

/*
Method Open opens an mt object from a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord containing mt object to open
            rOffset - Offset to the mt object in SmiRecord
            typeInfo - TypeInfo of mt object to open
            rValue - reference to a Word referencing the opened mt object
return value: true, if mt object was successfully opened, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::Open(SmiRecord& rValueRecord,
                                size_t& rOffset,
                                const ListExpr typeInfo,
                                Word& rValue)
{
  bool bRetVal = OpenAttribute<mt<Type, Properties> >(rValueRecord,
                                                      rOffset,
                                                      typeInfo,
                                                      rValue);

  return bRetVal;
}

/*
Method Out writes out an existing mt object in the form of a ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of mt object to write out
            value - reference to a Word referencing the mt object
return value: ListExpr of mt object referenced by value
exceptions: -

*/

template <typename Type, typename Properties>
ListExpr mt<Type, Properties>::Out(ListExpr typeInfo,
                                   Word value)
{
  ListExpr pListExpr = 0;

  if(nl != 0)
  {
    mt<Type, Properties>* pmt = static_cast<mt<Type, Properties>*>(value.addr);

    if(pmt != 0)
    {
      if(pmt->IsDefined() == true)
      {
        NList instanceList;

        NList gridList;
        gridList.append(pmt->m_Grid.GetX());
        gridList.append(pmt->m_Grid.GetY());
        gridList.append(pmt->m_Grid.GetLength());
        gridList.append(pmt->m_Grid.GetDuration().ToDouble());
        instanceList.append(gridList);

        NList sizeList;
        sizeList.append(Properties::GetXDimensionSize());
        sizeList.append(Properties::GetYDimensionSize());
        sizeList.append(Properties::GetTDimensionSize());
        instanceList.append(sizeList);

        NList tintList;
        tintList.append(0);
        tintList.append(0);
        tintList.append(0);

        Type undefinedValue = Properties::TypeProperties::GetUndefinedValue();
        NList valueList;

        for(int i = 0; i < Properties::GetFlobElements(); i++)
        {
          Type value = undefinedValue;

          bool bOK = pmt->m_Flob.read(reinterpret_cast<char*>(&value),
                                      sizeof(Type),
                                      i * sizeof(Type));
          assert(bOK);

          valueList.append(Properties::TypeProperties::ToNList(value));
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
Method Property returns all properties of mt datatype.

author: Dirk Zacher
parameters: -
return value: properties of mt datatype in the form of a ListExpr
exceptions: -

*/

template <typename Type, typename Properties>
ListExpr mt<Type, Properties>::Property()
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
               (std::string("((0.0 0.0 1.0 1.0) (1 1 1) ((0 0 0 (0))))"),
                true));
  values.append(NList(std::string(""), true));

  propertyList = NList(names, values);

  return propertyList.listExpr();
}

/*
Method Save saves an existing mt object in a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord to save existing mt object
            rOffset - Offset to save position of mt object in SmiRecord
            typeInfo - TypeInfo of mt object to save
            rValue - reference to a Word referencing the mt object to save
return value: true, if mt object was successfully saved, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool mt<Type, Properties>::Save(SmiRecord& rValueRecord,
                                size_t& rOffset,
                                const ListExpr typeInfo,
                                Word& rValue)
{
  bool bRetVal = SaveAttribute<mt<Type, Properties> >(rValueRecord,
                                                      rOffset,
                                                      typeInfo,
                                                      rValue);

  return bRetVal;
}

/*
Method SizeOfObj returns the size of a mt object.

author: Dirk Zacher
parameters: -
return value: size of a mt object
exceptions: -

*/

template <typename Type, typename Properties>
int mt<Type, Properties>::SizeOfObj()
{
  return sizeof(mt<Type, Properties>);
}

}

#endif // TILEALGEBRA_MT_H
