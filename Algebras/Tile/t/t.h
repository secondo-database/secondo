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

#ifndef TILEALGEBRA_T_H
#define TILEALGEBRA_T_H

/*
SECONDO includes

*/

#include "Attribute.h"
#include "DateTime.h"
#include "RectangleAlgebra.h"
#include "Symbols.h"
#include "TypeConstructor.h"
#include "../../Tools/Flob/Flob.h"

/*
TileAlgebra includes

*/

#include "tProperties.h"
#include "../grid/tgrid.h"
#include "../Index/Index.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Template class t represents the base implementation
for datatypes tint, treal, tbool and tstring.

author: Dirk Zacher

*/

template <typename Type, typename Properties = tProperties<Type> >
class t : public Attribute
{
  protected:

  /*
  Constructor t does not initialize any members and
  should only be used in conjunction with Cast method.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  t();

  public:

  /*
  Constructor t sets defined flag of base class Attribute and
  initializes all members of the class with default values.

  author: Dirk Zacher
  parameters: bDefined - defined flag of base class Attribute
  return value: -
  exceptions: -

  */

  t(bool bDefined);

  /*
  Constructor t sets defined flag of base class Attribute to defined flag
  of rt object and initializes all members of the class with corresponding
  values of rt object.

  author: Dirk Zacher
  parameters: rt - reference to a t object
  return value: -
  exceptions: -

  */

  t(const t& rt);

  /*
  Destructor ~t deinitializes a t object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  virtual ~t();

  /*
  Operator= assigns all member values of a given t object
  to the corresponding member values of this object.

  author: Dirk Zacher
  parameters: rt - reference to a t object
  return value: reference to this object
  exceptions: -

  */

  t& operator=(const t& rt);

  /*
  TileAlgebra operator atlocation returns the value of a t object
  at given location rX and rY.

  author: Dirk Zacher
  parameters: rX - reference to location of dimension x
              rY - reference to location of dimension y
              rValue - reference to a Properties::TypeProperties::WrapperType
                       object containing the value at given location rX and rY
  return value: -
  exceptions: -

  */

  void atlocation(const double& rX,
                  const double& rY,
                  typename Properties::TypeProperties::WrapperType& rValue)
                  const;
 
  /*
  TileAlgebra operator atrange returns all values of a t object
  inside the given rectangle.

  author: Dirk Zacher
  parameters: rRectangle - reference to a Rectangle<2> object
              rt - reference to a t object containing all values
                   of the t object inside the given rectangle
  return value: -
  exceptions: -

  */

  void atrange(const Rectangle<2>& rRectangle,
               typename Properties::PropertiesType& rt) const;

  /*
  TileAlgebra operator bbox returns the bounding box of a t object.

  author: Dirk Zacher
  parameters: rBoundingBox - reference to a Properties::RectangleType object
                             containing the bounding box of the t object.
  return value: -
  exceptions: -

  */

  void bbox(typename Properties::RectangleType& rBoundingBox) const;

  /*
  TileAlgebra operator minimum returns the minimum value of t object.

  author: Dirk Zacher
  parameters: rMinimum - reference to a Type object containing
                         the minimum value of t object
  return value: -
  exceptions: -

  */

  void minimum(Type& rMinimum) const;

  /*
  TileAlgebra operator maximum returns the maximum value of t object.

  author: Dirk Zacher
  parameters: rMaximum - reference to a Type object containing
                         the maximum value of t object
  return value: -
  exceptions: -

  */

  void maximum(Type& rMaximum) const;

  /*
  TileAlgebra operator getgrid returns the tgrid object of t object.

  author: Dirk Zacher
  parameters: rtgrid - reference to a tgrid object containing
                       tgrid object of t object
  return value: -
  exceptions: -

  */

  void getgrid(tgrid& rtgrid) const;

  /*
  Method GetBoundingBoxIndexes returns minimum index and maximum index
  of the bounding box of t object.

  author: Dirk Zacher
  parameters: rMinimumIndex - reference to an Index<2> object containing
                              the minimum index of the bounding box of t object
              rMaximumIndex - reference to an Index<2> object containing
                              the maximum index of the bounding box of t object
  return value: true, if minimum index and maximum index of the bounding box
                of t object successfully calculated, otherwise false
  exceptions: -

  */

  bool GetBoundingBoxIndexes(Index<2>& rMinimumIndex,
                             Index<2>& rMaximumIndex) const;

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
  Method GetValue returns the value of t object at given 2-dimensional index.

  author: Dirk Zacher
  parameters: rIndex - reference to a 2-dimensional index
  return value: value of t object at given 2-dimensional index
  exceptions: -

  */

  Type GetValue(const Index<2>& rIndex) const;

  /*
  Method IsValidLocation checks if given location rX and rY
  is a valid location inside the t object.

  author: Dirk Zacher
  parameters: rX - reference to location of dimension x
              rY - reference to location of dimension y
  return value: true, if given location rX and rY is a valid location
                inside the t object, otherwise false
  exceptions: -

  */

  bool IsValidLocation(const double& rX,
                       const double& rY) const;

  /*
  Method SetGrid sets the tgrid of t object.

  author: Dirk Zacher
  parameters: rtgrid - reference to a tgrid object
  return value: true, if the tgrid of t object was successfully set,
                otherwise false
  exceptions: -

  */

  bool SetGrid(const tgrid& rtgrid);

  /*
  Method SetGrid sets the tgrid properties of t object.

  author: Dirk Zacher
  parameters: rX - reference to the x origin of the grid
              rY - reference to the y origin of the grid
              rLength - reference to the length of a grid cell
  return value: true, if tgrid properties of t object were successfully set,
                otherwise false
  exceptions: -

  */

  bool SetGrid(const double& rX,
               const double& rY,
               const double& rLength);

  /*
  Method SetValue sets a value of t object at given index and
  recalculates minimum and maximum of t object if bSetExtrema is true.

  author: Dirk Zacher
  parameters: rIndex - reference to a 2-dimensional index
              rValue - reference to a value
              bSetExtrema - flag that indicates if minimum and maximum
                            of t object should be recalculated
  return value: true, if rValue was successfully set at rIndex, otherwise false
  exceptions: -

  */

  bool SetValue(const Index<2>& rIndex,
                const Type& rValue,
                bool bSetExtrema);

  /*
  Method SetValue sets a value of t object at given location rX and rY and
  recalculates minimum and maximum of t object if bSetExtrema is true.

  author: Dirk Zacher
  parameters: rX - reference to location of dimension x
              rY - reference to location of dimension y
              rValue - reference to a value
              bSetExtrema - flag that indicates if minimum and maximum
                            of t object should be recalculated
  return value: true, if rValue was successfully set at given location
                rX and rY, otherwise false
  exceptions: -

  */

  bool SetValue(const double& rX,
                const double& rY,
                const Type& rValue,
                bool bSetExtrema);

  /*
  Method SetValues sets all values of t object and recalculates
  minimum and maximum of t object if bSetExtrema is true.

  author: Dirk Zacher
  parameters: rValue - reference to a value
              bSetExtrema - flag that indicates if minimum and maximum
                            of t object should be recalculated
  return value: true, if all values were successfully set, otherwise false
  exceptions: -

  */

  bool SetValues(const Type& rValue,
                 bool bSetExtrema);

  protected:

  /*
  Method IsValidIndex checks if given index is a valid index
  inside the t object.

  author: Dirk Zacher
  parameters: rIndex - reference to a 2-dimensional index
  return value: true, if given index is a valid index inside the t object,
                otherwise false
  exceptions: -

  */

  bool IsValidIndex(const Index<2>& rIndex) const;

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
  Method HashValue returns the hash value of the t object.

  author: Dirk Zacher
  parameters: -
  return value: hash value of the t object
  exceptions: -

  */

  virtual size_t HashValue() const;

  /*
  Method NumOfFLOBs returns the number of Flobs of a t object.

  author: Dirk Zacher
  parameters: -
  return value: number of Flobs of a t object
  exceptions: -

  */

  virtual int NumOfFLOBs() const;

  /*
  Method Sizeof returns the size of t datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of t datatype
  exceptions: -

  */

  virtual size_t Sizeof() const;

  /*
  Method BasicType returns the typename of t datatype.

  author: Dirk Zacher
  parameters: -
  return value: typename of t datatype
  exceptions: -

  */

  static const std::string BasicType();

  /*
  Method Cast casts a void pointer to a new t object.

  author: Dirk Zacher
  parameters: pVoid - a pointer to a memory address
  return value: a pointer to a new t object
  exceptions: -

  */

  static void* Cast(void* pVoid);

  /*
  Method Clone clones an existing t object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing t object
  return value: a Word that references a new t object
  exceptions: -

  */

  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);

  /*
  Method Close closes an existing t object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing t object
  return value: -
  exceptions: -

  */

  static void Close(const ListExpr typeInfo,
                    Word& rWord);

  /*
  Method Create creates a new t object.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of the new t object to create
  return value: a Word that references a new t object
  exceptions: -

  */

  static Word Create(const ListExpr typeInfo);

  /*
  Method Delete deletes an existing t object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing t object
  return value: -
  exceptions: -

  */

  static void Delete(const ListExpr typeInfo,
                     Word& rWord);

  /*
  Method GetTypeConstructor returns the TypeConstructor of class t.

  author: Dirk Zacher
  parameters: -
  return value: TypeConstructor of class t
  exceptions: -

  */

  static TypeConstructor GetTypeConstructor();

  /*
  Method In creates a new t object on the basis of a given ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object to create on the basis of instance
              instance - ListExpr of the t object to create
              errorPos - error position
              rErrorInfo - reference to error information
              rCorrect - flag that indicates if t object correctly created
  return value: a Word that references a new t object
  exceptions: -

  */

  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);

  /*
  Method KindCheck checks if given type is t type.

  author: Dirk Zacher
  parameters: type - ListExpr of type to check
              rErrorInfo - reference to error information
  return value: true, if type is t type, otherwise false
  exceptions: -

  */

  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);

  /*
  Method Open opens a t object from a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord containing t object to open
              rOffset - Offset to the t object in SmiRecord
              typeInfo - TypeInfo of t object to open
              rValue - reference to a Word referencing the opened t object
  return value: true, if t object was successfully opened, otherwise false
  exceptions: -

  */

  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method Out writes out an existing t object in the form of a ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of t object to write out
              value - reference to a Word referencing the t object
  return value: ListExpr of t object referenced by value
  exceptions: -

  */

  static ListExpr Out(ListExpr typeInfo,
                      Word value);

  /*
  Method Property returns all properties of t datatype.

  author: Dirk Zacher
  parameters: -
  return value: properties of t datatype in the form of a ListExpr
  exceptions: -

  */

  static ListExpr Property();

  /*
  Method Save saves an existing t object in a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord to save existing t object
              rOffset - Offset to save position of t object in SmiRecord
              typeInfo - TypeInfo of t object to save
              rValue - reference to a Word referencing the t object to save
  return value: true, if t object was successfully saved, otherwise false
  exceptions: -

  */

  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method SizeOfObj returns the size of a t object.

  author: Dirk Zacher
  parameters: -
  return value: size of a t object
  exceptions: -

  */

  static int SizeOfObj();

  protected:

  /*
  Member m_Grid contains the tgrid object of t object.

  */

  tgrid m_Grid;

  /*
  Member m_Minimum contains the minimum value of all values of t object.

  */

  Type m_Minimum;

  /*
  Member m_Maximum contains the maximum value of all values of t object.

  */

  Type m_Maximum;

  /*
  Member m_Flob contains the Flob to store all values of t object.

  */

  Flob m_Flob;
};

/*
Constructor t does not initialize any members and
should only be used in conjunction with Cast method.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
t<Type, Properties>::t()
                    :Attribute()
{

}

/*
Constructor t sets defined flag of base class Attribute and
initializes all members of the class with default values.

author: Dirk Zacher
parameters: bDefined - defined flag of base class Attribute
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
t<Type, Properties>::t(bool bDefined)
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
Constructor t sets defined flag of base class Attribute to defined flag
of rt object and initializes all members of the class with corresponding
values of rt object.

author: Dirk Zacher
parameters: rt - reference to a t object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
t<Type, Properties>::t(const t<Type, Properties>& rt)
                    :Attribute(rt.IsDefined()),
                     m_Grid(rt.m_Grid),
                     m_Minimum(rt.m_Minimum),
                     m_Maximum(rt.m_Maximum),
                     m_Flob(rt.m_Flob)
{
  
}

/*
Destructor deinitializes a t object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
t<Type, Properties>::~t()
{

}

/*
Operator= assigns all member values of a given t object
to the corresponding member values of this object.

author: Dirk Zacher
parameters: rt - reference to a t object
return value: reference to this object
exceptions: -

*/

template <typename Type, typename Properties>
t<Type, Properties>& t<Type, Properties>::operator=
                                          (const t<Type, Properties>& rt)
{
  if(this != &rt)
  {
    Attribute::operator=(rt);
    m_Grid = rt.m_Grid;
    m_Minimum = rt.m_Minimum;
    m_Maximum = rt.m_Maximum;

    bool bOK = false;
    bOK = m_Flob.clean();
    assert(bOK);
    bOK = m_Flob.copyFrom(rt.m_Flob);
    assert(bOK);
  }

  return *this;
}

/*
TileAlgebra operator atlocation returns the value of a t object
at given location rX and rY.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
            rValue - reference to a Properties::TypeProperties::WrapperType
                     object containing the value at given location rX and rY
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void t<Type, Properties>::atlocation(const double& rX,
                                     const double& rY,
                                     typename Properties::TypeProperties::
                                     WrapperType& rValue) const
{
  rValue.SetDefined(false);

  if(IsDefined() &&
     IsValidLocation(rX, rY))
  {
    Index<2> index = GetLocationIndex(rX, rY);
    Type value = GetValue(index);

    if(Properties::TypeProperties::IsUndefinedValue(value) == false)
    {
      rValue = Properties::TypeProperties::GetWrappedValue(value);
    }
  }
}

/*
TileAlgebra operator atrange returns all values of a t object
inside the given rectangle.

author: Dirk Zacher
parameters: rRectangle - reference to a Rectangle<2> object
            rt - reference to a t object containing all values
                 of the t object inside the given rectangle
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void t<Type, Properties>::atrange(const Rectangle<2>& rRectangle,
                                  typename Properties::PropertiesType& rt)
                                  const
{
  rt.SetDefined(false);

  if(IsDefined() &&
     rRectangle.IsDefined())
  {
    if(IsValidLocation(rRectangle.MinD(0), rRectangle.MinD(1)) &&
       IsValidLocation(rRectangle.MaxD(0), rRectangle.MaxD(1)))
    {
      rt.SetDefined(true);

      double x = m_Grid.GetX();
      double y = m_Grid.GetY();
      double length = m_Grid.GetLength();
      rt.SetGrid(m_Grid);

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
            Type value = GetValue(index);

            if(Properties::TypeProperties::IsUndefinedValue(value) == false)
            {
              rt.SetValue(index, value, true);
            }
          }
        }
      }
    }
  }
}

/*
TileAlgebra operator bbox returns the bounding box of a t object.

author: Dirk Zacher
parameters: rBoundingBox - reference to a Properties::RectangleType object
                           containing the bounding box of the t object.
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void t<Type, Properties>::bbox(typename Properties::RectangleType&
                               rBoundingBox) const
{
  rBoundingBox.SetDefined(false);

  if(IsDefined())
  {
    double minima[2] = { 0.0, 0.0 };
    double maxima[2] = { 0.0, 0.0 };

    int xDimensionSize = Properties::GetXDimensionSize();
    int yDimensionSize = Properties::GetYDimensionSize();
    Type value = Properties::TypeProperties::GetUndefinedValue();

    /*
    calculation of x dimension minimum

    */

    for(int column = 0; column < xDimensionSize; column++)
    {
      bool bbreak = false;

      for(int row = 0; row < yDimensionSize; row++)
      {
        Index<2> index((int[]){column, row});
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

    /*
    calculation of x dimension maximum

    */

    for(int column = xDimensionSize - 1; column >= 0; column--)
    {
      bool bbreak = false;

      for(int row = 0; row < yDimensionSize; row++)
      {
        Index<2> index((int[]){column, row});
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

    /*
    calculation of y dimension minimum

    */

    for(int row = 0; row < yDimensionSize; row++)
    {
      bool bbreak = false;

      for(int column = 0; column < xDimensionSize; column++)
      {
        Index<2> index((int[]){column, row});
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

    /*
    calculation of y dimension maximum

    */

    for(int row = yDimensionSize - 1; row >= 0; row--)
    {
      bool bbreak = false;

      for(int column = 0; column < xDimensionSize; column++)
      {
        Index<2> index((int[]){column, row});
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

    rBoundingBox.Set(true, minima, maxima);
  }
}

/*
TileAlgebra operator minimum returns the minimum value of t object.

author: Dirk Zacher
parameters: rMinimum - reference to a Type object containing
                       the minimum value of t object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void t<Type, Properties>::minimum(Type& rMinimum) const
{
  rMinimum = Properties::TypeProperties::GetUndefinedValue();

  if(IsDefined())
  {
    rMinimum = m_Minimum;
  }
}

/*
TileAlgebra operator maximum returns the maximum value of t object.

author: Dirk Zacher
parameters: rMaximum - reference to a Type object containing
                       the maximum value of t object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void t<Type, Properties>::maximum(Type& rMaximum) const
{
  rMaximum = Properties::TypeProperties::GetUndefinedValue();

  if(IsDefined())
  {
    rMaximum = m_Maximum;
  }
}

/*
TileAlgebra operator getgrid returns the tgrid object of t object.

author: Dirk Zacher
parameters: rtgrid - reference to a tgrid object containing
                     tgrid object of t object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void t<Type, Properties>::getgrid(tgrid& rtgrid) const
{
  rtgrid.SetDefined(false);

  if(IsDefined())
  {
    rtgrid = m_Grid;
  }
}

/*
Method GetBoundingBoxIndexes returns minimum index and maximum index
of the bounding box of t object.

author: Dirk Zacher
parameters: rMinimumIndex - reference to an Index<2> object containing
                            the minimum index of the bounding box of t object
            rMaximumIndex - reference to an Index<2> object containing
                            the maximum index of the bounding box of t object
return value: true, if minimum index and maximum index of the bounding box
              of t object successfully calculated, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool t<Type, Properties>::GetBoundingBoxIndexes(Index<2>& rMinimumIndex,
                                                Index<2>& rMaximumIndex) const
{
  bool bRetVal = false;

  if(IsDefined())
  {
    typename Properties::RectangleType boundingBox;
    bbox(boundingBox);

    if(boundingBox.IsDefined())
    {
      rMinimumIndex = GetLocationIndex(boundingBox.MinD(0),
                                       boundingBox.MinD(1));
      rMaximumIndex = GetLocationIndex(boundingBox.MaxD(0),
                                       boundingBox.MaxD(1));
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
Index<2> t<Type, Properties>::GetLocationIndex(const double& rX,
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
Method GetValue returns the value of t object at given 2-dimensional index.

author: Dirk Zacher
parameters: rIndex - reference to a 2-dimensional index
return value: value of t object at given 2-dimensional index
exceptions: -

*/

template <typename Type, typename Properties>
Type t<Type, Properties>::GetValue(const Index<2>& rIndex) const
{
  Type value = Properties::TypeProperties::GetUndefinedValue();

  if(IsDefined() &&
     IsValidIndex(rIndex))
  {
    int yDimensionSize = Properties::GetYDimensionSize();
    int flobIndex = rIndex[1] * yDimensionSize + rIndex[0];

    bool bOK = m_Flob.read(reinterpret_cast<char*>(&value),
                           sizeof(Type),
                           flobIndex * sizeof(Type));
    assert(bOK);
  }

  return value;
}

/*
Method IsValidLocation checks if given location rX and rY
is a valid location inside the t object.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
return value: true, if given location rX and rY is a valid location
              inside the t object, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool t<Type, Properties>::IsValidLocation(const double& rX,
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
Method SetGrid sets the tgrid of t object.

author: Dirk Zacher
parameters: rtgrid - reference to a tgrid object
return value: true, if the tgrid of t object was successfully set,
              otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool t<Type, Properties>::SetGrid(const tgrid& rtgrid)
{
  bool bRetVal = false;

  if(IsDefined() &&
     rtgrid.IsDefined())
  {
    m_Grid = rtgrid;
    bRetVal = true;
  }

  return bRetVal;
}

/*
Method SetGrid sets the tgrid properties of t object.

author: Dirk Zacher
parameters: rX - reference to the x origin of the grid
            rY - reference to the y origin of the grid
            rLength - reference to the length of a grid cell
return value: true, if tgrid properties of t object were successfully set,
              otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool t<Type, Properties>::SetGrid(const double& rX,
                                  const double& rY,
                                  const double& rLength)
{
  bool bRetVal = false;

  if(IsDefined())
  {
    m_Grid.SetDefined(true);
    bRetVal  = m_Grid.SetX(rX);
    bRetVal &= m_Grid.SetY(rY);
    bRetVal &= m_Grid.SetLength(rLength);
  }

  return bRetVal;
}

/*
Method SetValue sets a value of t object at given index and
recalculates minimum and maximum of t object if bSetExtrema is true.

author: Dirk Zacher
parameters: rIndex - reference to a 2-dimensional index
            rValue - reference to a value
            bSetExtrema - flag that indicates if minimum and maximum
                          of t object should be recalculated
return value: true, if rValue was successfully set at rIndex, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool t<Type, Properties>::SetValue(const Index<2>& rIndex,
                                   const Type& rValue,
                                   bool bSetExtrema)
{
  bool bRetVal = false;

  if(IsDefined() &&
     IsValidIndex(rIndex))
  {
    int yDimensionSize = Properties::GetYDimensionSize();
    int flobIndex = rIndex[1] * yDimensionSize + rIndex[0];

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
Method SetValue sets a value of t object at given location rX and rY and
recalculates minimum and maximum of t object if bSetExtrema is true.

author: Dirk Zacher
parameters: rX - reference to location of dimension x
            rY - reference to location of dimension y
            rValue - reference to a value
            bSetExtrema - flag that indicates if minimum and maximum
                          of t object should be recalculated
return value: true, if rValue was successfully set at given location
              rX and rY, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool t<Type, Properties>::SetValue(const double& rX,
                                   const double& rY,
                                   const Type& rValue,
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
Method SetValues sets all values of t object and recalculates
minimum and maximum of t object if bSetExtrema is true.

author: Dirk Zacher
parameters: rValue - reference to a value
            bSetExtrema - flag that indicates if minimum and maximum
                          of t object should be recalculated
return value: true, if all values were successfully set, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool t<Type, Properties>::SetValues(const Type& rValue,
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
inside the t object.

author: Dirk Zacher
parameters: rIndex - reference to a 2-dimensional index
return value: true, if given index is a valid index inside the t object,
              otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool t<Type, Properties>::IsValidIndex(const Index<2>& rIndex) const
{
  bool bIsValidIndex = false;

  int xDimensionSize = Properties::GetXDimensionSize();
  int yDimensionSize = Properties::GetYDimensionSize();

  if(rIndex[0] >= 0 &&
     rIndex[0] < xDimensionSize &&
     rIndex[1] >= 0 &&
     rIndex[1] < yDimensionSize)
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
bool t<Type, Properties>::Adjacent(const Attribute* pAttribute) const
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
Attribute* t<Type, Properties>::Clone() const
{
  Attribute* pAttribute = new t<Type, Properties>(*this);
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
int t<Type, Properties>::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;

  if(pAttribute != 0)
  {
    const t<Type, Properties>* pt = dynamic_cast<const t<Type, Properties>*>
                                    (pAttribute);

    if(pt != 0)
    {
      bool bIsDefined = IsDefined();
      bool btIsDefined = pt->IsDefined();

      if(bIsDefined == true)
      {
        if(btIsDefined == true) // defined x defined
        {
          nRetVal = m_Grid.Compare(&(pt->m_Grid));

          if(nRetVal == 0)
          {
            SmiSize flobSize = Properties::GetFlobSize();
            
            char buffer1[flobSize];
            m_Flob.read(buffer1, flobSize, 0);

            char buffer2[flobSize];
            pt->m_Flob.read(buffer2, flobSize, 0);

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
void t<Type, Properties>::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const t<Type, Properties>* pt = dynamic_cast<const t<Type, Properties>*>
                                    (pAttribute);

    if(pt != 0)
    {
      *this = *pt;
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
Flob* t<Type, Properties>::GetFLOB(const int i)
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
Method HashValue returns the hash value of the t object.

author: Dirk Zacher
parameters: -
return value: hash value of the t object
exceptions: -

*/

template <typename Type, typename Properties>
size_t t<Type, Properties>::HashValue() const
{
  size_t hashValue = 0;

  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }

  return hashValue;
}

/*
Method NumOfFLOBs returns the number of Flobs of a t object.

author: Dirk Zacher
parameters: -
return value: number of Flobs of a t object
exceptions: -

*/

template <typename Type, typename Properties>
int t<Type, Properties>::NumOfFLOBs() const
{ 
  return 1;
}

/*
Method Sizeof returns the size of t datatype.

author: Dirk Zacher
parameters: -
return value: size of t datatype
exceptions: -

*/

template <typename Type, typename Properties>
size_t t<Type, Properties>::Sizeof() const
{
  return sizeof(t<Type, Properties>);
}

/*
Method BasicType returns the typename of t datatype.

author: Dirk Zacher
parameters: -
return value: typename of t datatype
exceptions: -

*/

template <typename Type, typename Properties>
const std::string t<Type, Properties>::BasicType()
{
  return Properties::GetTypeName();
}

/*
Method Cast casts a void pointer to a new t object.

author: Dirk Zacher
parameters: pVoid - a pointer to a memory address
return value: a pointer to a new t object
exceptions: -

*/

template <typename Type, typename Properties>
void* t<Type, Properties>::Cast(void* pVoid)
{
  return new(pVoid)t<Type, Properties>;
}

/*
Method Clone clones an existing t object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing t object
return value: a Word that references a new t object
exceptions: -

*/

template <typename Type, typename Properties>
Word t<Type, Properties>::Clone(const ListExpr typeInfo,
                                const Word& rWord)
{
  Word word;

  t<Type, Properties>* pt = static_cast<t<Type, Properties>*>(rWord.addr);

  if(pt != 0)
  {
    word.addr = new t<Type, Properties>(*pt);
    assert(word.addr != 0);
  }

  return word;
}

/*
Method Close closes an existing t object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing t object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void t<Type, Properties>::Close(const ListExpr typeInfo,
                                Word& rWord)
{
  t<Type, Properties>* pt = static_cast<t<Type, Properties>*>(rWord.addr);

  if(pt != 0)
  {
    delete pt;
    rWord.addr = 0;
  }
}

/*
Method Create creates a new t object.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of the new t object to create
return value: a Word that references a new t object
exceptions: -

*/

template <typename Type, typename Properties>
Word t<Type, Properties>::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new t<Type, Properties>(true);
  assert(word.addr != 0);

  return word;
}

/*
Method Delete deletes an existing t object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing t object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void t<Type, Properties>::Delete(const ListExpr typeInfo,
                                 Word& rWord)
{
  t<Type, Properties>* pt = static_cast<t<Type, Properties>*>(rWord.addr);

  if(pt != 0)
  {
    delete pt;
    rWord.addr = 0;
  }
}

/*
Method GetTypeConstructor returns the TypeConstructor of class t.

author: Dirk Zacher
parameters: -
return value: TypeConstructor of class t
exceptions: -

*/

template <typename Type, typename Properties>
TypeConstructor t<Type, Properties>::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    t<Type, Properties>::BasicType(), // type name function
    t<Type, Properties>::Property,    // property function describing signature
    t<Type, Properties>::Out,         // out function
    t<Type, Properties>::In,          // in function
    0,                                // save to list function
    0,                                // restore from list function
    t<Type, Properties>::Create,      // create function
    t<Type, Properties>::Delete,      // delete function
    t<Type, Properties>::Open,        // open function
    t<Type, Properties>::Save,        // save function
    t<Type, Properties>::Close,       // close function
    t<Type, Properties>::Clone,       // clone function
    t<Type, Properties>::Cast,        // cast function
    t<Type, Properties>::SizeOfObj,   // sizeofobj function
    t<Type, Properties>::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

/*
Method In creates a new t object on the basis of a given ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object to create on the basis of instance
            instance - ListExpr of the t object to create
            errorPos - error position
            rErrorInfo - reference to error information
            rCorrect - flag that indicates if t object correctly created
return value: a Word that references a new t object
exceptions: -

*/

template <typename Type, typename Properties>
Word t<Type, Properties>::In(const ListExpr typeInfo,
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
        t<Type, Properties>* pt = new t<Type, Properties>(true);

        if(pt != 0)
        {
          bool bOK = pt->SetGrid(gridList.elem(1).realval(),
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
                        int xDimensionSize = Properties::GetXDimensionSize();
                        int yDimensionSize = Properties::GetYDimensionSize();

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
                                Type value = Properties::TypeProperties::
                                             GetUndefinedValue();

                                if(valueList.elem(listIndex).
                                   isSymbol(Symbol::UNDEFINED()) == false)
                                {
                                  if(Properties::TypeProperties::
                                     IsValidValueType
                                     (valueList.elem
                                     (listIndex)))
                                  {
                                    value = Properties::TypeProperties::
                                            GetValue
                                            (valueList.elem(listIndex));
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

                                pt->SetValue(index, value, true);
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
            word.addr = pt;
            rCorrect = true;
          }

          else
          {
            delete pt;
            pt = 0;
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
Method KindCheck checks if given type is t type.

author: Dirk Zacher
parameters: type - ListExpr of type to check
            rErrorInfo - reference to error information
return value: true, if type is t type, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool t<Type, Properties>::KindCheck(ListExpr type,
                                    ListExpr& rErrorInfo)
{
  bool bRetVal = false;

  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, t<Type, Properties>::BasicType());
  }

  return bRetVal;
}

/*
Method Open opens an t object from a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord containing t object to open
            rOffset - Offset to the t object in SmiRecord
            typeInfo - TypeInfo of t object to open
            rValue - reference to a Word referencing the opened t object
return value: true, if t object was successfully opened, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool t<Type, Properties>::Open(SmiRecord& rValueRecord,
                               size_t& rOffset,
                               const ListExpr typeInfo,
                               Word& rValue)
{
  bool bRetVal = OpenAttribute<t<Type, Properties> >(rValueRecord,
                                                     rOffset,
                                                     typeInfo,
                                                     rValue);

  return bRetVal;
}

/*
Method Out writes out an existing t object in the form of a ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of t object to write out
            value - reference to a Word referencing the t object
return value: ListExpr of t object referenced by value
exceptions: -

*/

template <typename Type, typename Properties>
ListExpr t<Type, Properties>::Out(ListExpr typeInfo,
                                  Word value)
{
  ListExpr pListExpr = 0;

  if(nl != 0)
  {
    t<Type, Properties>* pt = static_cast<t<Type, Properties>*>(value.addr);

    if(pt != 0)
    {
      if(pt->IsDefined() == true)
      {
        NList instanceList;

        NList gridList;
        gridList.append(pt->m_Grid.GetX());
        gridList.append(pt->m_Grid.GetY());
        gridList.append(pt->m_Grid.GetLength());
        instanceList.append(gridList);

        NList sizeList;
        sizeList.append(Properties::GetXDimensionSize());
        sizeList.append(Properties::GetYDimensionSize());
        instanceList.append(sizeList);

        NList tintList;
        tintList.append(0);
        tintList.append(0);

        Type undefinedValue = Properties::TypeProperties::GetUndefinedValue();
        NList valueList;

        for(int i = 0; i < Properties::GetFlobElements(); i++)
        {
          Type value = undefinedValue;

          bool bOK = pt->m_Flob.read(reinterpret_cast<char*>(&value),
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
Method Property returns all properties of t datatype.

author: Dirk Zacher
parameters: -
return value: properties of t datatype in the form of a ListExpr
exceptions: -

*/

template <typename Type, typename Properties>
ListExpr t<Type, Properties>::Property()
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
               (std::string("((0.0 0.0 1.0) (2 2) ((0 0 (0 1 2 3))))"),
                true));
  values.append(NList(std::string(""), true));

  propertyList = NList(names, values);

  return propertyList.listExpr();
}

/*
Method Save saves an existing t object in a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord to save existing t object
            rOffset - Offset to save position of t object in SmiRecord
            typeInfo - TypeInfo of t object to save
            rValue - reference to a Word referencing the t object to save
return value: true, if t object was successfully saved, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool t<Type, Properties>::Save(SmiRecord& rValueRecord,
                               size_t& rOffset,
                               const ListExpr typeInfo,
                               Word& rValue)
{
  bool bRetVal = SaveAttribute<t<Type, Properties> >(rValueRecord,
                                                     rOffset,
                                                     typeInfo,
                                                     rValue);

  return bRetVal;
}

/*
Method SizeOfObj returns the size of a t object.

author: Dirk Zacher
parameters: -
return value: size of a t object
exceptions: -

*/

template <typename Type, typename Properties>
int t<Type, Properties>::SizeOfObj()
{
  return sizeof(t<Type, Properties>);
}

}

#endif // TILEALGEBRA_T_H
