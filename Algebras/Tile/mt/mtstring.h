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

#ifndef TILEALGEBRA_MTSTRING_H
#define TILEALGEBRA_MTSTRING_H

/*
system includes

*/

#include <string>

/*
SECONDO includes

*/

#include "RectangleAlgebra.h"
#include "TemporalAlgebra.h"

/*
TileAlgebra includes

*/

#include "mt.h"
#include "mtProperties.h"
#include "mtint.h"
#include "../it/itstring.h"
#include "../Properties/Propertiesstring.h"
#include "../UniqueStringArray/UniqueStringArray.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Class mtstring represents the implementation of datatype mtstring.

author: Dirk Zacher

*/

class mtstring : public mtint
{
  private:

  /*
  Constructor mtstring does not initialize any members and
  should only be used in conjunction with Cast method.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  mtstring();

  public:

  /*
  Constructor mtstring sets defined flag of base class Attribute and
  initializes all members of the class with default values.

  author: Dirk Zacher
  parameters: bDefined - defined flag of base class Attribute
  return value: -
  exceptions: -

  */

  mtstring(bool bDefined);

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

  mtstring(const mtint& rmtint, const UniqueStringArray& rUniqueStringArray);

  /*
  Constructor mtstring sets defined flag of base class Attribute to defined flag
  of rmtstring object and initializes all members of the class
  with corresponding values of rmtstring object.

  author: Dirk Zacher
  parameters: rmtstring - reference to a mtstring object
  return value: -
  exceptions: -

  */

  mtstring(const mtstring& rmtstring);

  /*
  Destructor ~mtstring deinitializes a mtstring object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  virtual ~mtstring();

  /*
  Operator= assigns all member values of a given mtstring object
  to the corresponding member values of this object.

  author: Dirk Zacher
  parameters: rmtstring - reference to a mtstring object
  return value: reference to this object
  exceptions: -

  */

  mtstring& operator=(const mtstring& rmtstring);

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

  void atlocation(const double& rX,
                  const double& rY,
                  MString& rValues) const;

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

  void atlocation(const double& rX,
                  const double& rY,
                  const double& rInstant,
                  CcString& rValue) const;

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

  void atinstant(const Instant& rInstant,
                 itstring& ritstring) const;

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

  void atperiods(const Periods& rPeriods,
                 mtstring& rmtstring) const;

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

  void atrange(const Rectangle<2>& rRectangle,
               mtstring& rmtstring) const;

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

  void atrange(const Rectangle<2>& rRectangle,
               const double& rInstant1,
               const double& rInstant2,
               mtstring& rmtstring) const;

  /*
  TileAlgebra operator minimum returns the minimum value of mtstring object.

  author: Dirk Zacher
  parameters: -
  return value: minimum value of mtstring object
  exceptions: -

  */

  std::string minimum() const;

  /*
  TileAlgebra operator maximum returns the maximum value of mtstring object.

  author: Dirk Zacher
  parameters: -
  return value: maximum value of mtstring object
  exceptions: -

  */

  std::string maximum() const;

  /*
  Method GetValue returns the string value of mtstring object
  at given 3-dimensional index.

  author: Dirk Zacher
  parameters: rIndex - reference to a 3-dimensional index
  return value: string value of mtstring object at given 3-dimensional index
  exceptions: -

  */

  std::string GetValue(const Index<3>& rIndex) const;

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

  bool SetValue(const Index<3>& rIndex,
                const std::string& rValue,
                bool bSetExtrema);

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

  bool SetValue(const double& rX,
                const double& rY,
                const double& rInstant,
                const std::string& rValue,
                bool bSetExtrema);

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
  Method HashValue returns the hash value of the mtstring object.

  author: Dirk Zacher
  parameters: -
  return value: hash value of the mtstring object
  exceptions: -

  */

  virtual size_t HashValue() const;

  /*
  Method NumOfFLOBs returns the number of Flobs of a mtstring object.

  author: Dirk Zacher
  parameters: -
  return value: number of Flobs of a mtstring object
  exceptions: -

  */

  virtual int NumOfFLOBs() const;

  /*
  Method Sizeof returns the size of mtstring datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of mtstring datatype
  exceptions: -

  */

  virtual size_t Sizeof() const;

  /*
  Method BasicType returns the typename of mtstring datatype.

  author: Dirk Zacher
  parameters: -
  return value: typename of mtstring datatype
  exceptions: -

  */

  static const std::string BasicType();

  /*
  Method Cast casts a void pointer to a new mtstring object.

  author: Dirk Zacher
  parameters: pVoid - a pointer to a memory address
  return value: a pointer to a new mtstring object
  exceptions: -

  */

  static void* Cast(void* pVoid);

  /*
  Method Clone clones an existing mtstring object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing mtstring object
  return value: a Word that references a new mtstring object
  exceptions: -

  */

  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);

  /*
  Method Close closes an existing mtstring object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing mtstring object
  return value: -
  exceptions: -

  */

  static void Close(const ListExpr typeInfo,
                    Word& rWord);

  /*
  Method Create creates a new mtstring object.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of the new mtstring object to create
  return value: a Word that references a new mtstring object
  exceptions: -

  */

  static Word Create(const ListExpr typeInfo);

  /*
  Method Delete deletes an existing mtstring object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing mtstring object
  return value: -
  exceptions: -

  */

  static void Delete(const ListExpr typeInfo,
                     Word& rWord);

  /*
  Method GetTypeConstructor returns the TypeConstructor of class mtstring.

  author: Dirk Zacher
  parameters: -
  return value: TypeConstructor of class mtstring
  exceptions: -

  */

  static TypeConstructor GetTypeConstructor();

  /*
  Method In creates a new mtstring object on the basis of a given ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object to create on the basis of instance
              instance - ListExpr of the mtstring object to create
              errorPos - error position
              rErrorInfo - reference to error information
              rCorrect - flag that indicates if mtstring object
                         correctly created
  return value: a Word that references a new mtstring object
  exceptions: -

  */

  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);

  /*
  Method KindCheck checks if given type is mtstring type.

  author: Dirk Zacher
  parameters: type - ListExpr of type to check
              rErrorInfo - reference to error information
  return value: true, if type is mtstring type, otherwise false
  exceptions: -

  */

  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);

  /*
  Method Open opens an mtstring object from a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord containing mtstring object to open
              rOffset - Offset to the mtstring object in SmiRecord
              typeInfo - TypeInfo of mtstring object to open
              rValue - reference to a Word referencing
                       the opened mtstring object
  return value: true, if mtstring object was successfully opened,
                otherwise false
  exceptions: -

  */

  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method Out writes out an existing mtstring object in the form of a ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of mtstring object to write out
              value - reference to a Word referencing the mtstring object
  return value: ListExpr of mtstring object referenced by value
  exceptions: -

  */

  static ListExpr Out(ListExpr typeInfo,
                      Word value);

  /*
  Method Property returns all properties of mtstring datatype.

  author: Dirk Zacher
  parameters: -
  return value: properties of mtstring datatype in the form of a ListExpr
  exceptions: -

  */

  static ListExpr Property();

  /*
  Method Save saves an existing mtstring object in a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord to save existing mtstring object
              rOffset - Offset to save position of mtstring object in SmiRecord
              typeInfo - TypeInfo of mtstring object to save
              rValue - reference to a Word referencing
                       the mtstring object to save
  return value: true, if mtstring object was successfully saved, otherwise false
  exceptions: -

  */

  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method SizeOfObj returns the size of a mtstring object.

  author: Dirk Zacher
  parameters: -
  return value: size of a mtstring object
  exceptions: -

  */

  static int SizeOfObj();

  private:

  /*
  Member m_UniqueStringArray contains all unique strings
  used by mtstring object.

  */

  UniqueStringArray m_UniqueStringArray;
};

/*
Class mtProperties<std::string> represents the properties of datatype mtstring.

author: Dirk Zacher

*/

template <>
class mtProperties<std::string>
{
  public:

  /*
  typedef of PropertiesType

  */

  typedef mtstring PropertiesType;

  /*
  typedef of TypeProperties

  */

  typedef Properties<std::string> TypeProperties;

  /*
  typedef of GridType

  */

  typedef mtgrid GridType;

  /*
  typedef of RectangleType

  */

  typedef Rectangle<3> RectangleType;

  /*
  typedef of itType

  */

  typedef itstring itType;

  /*
  typedef of tType

  */

  typedef tstring tType;

  /*
  Method GetXDimensionSize returns the size of x dimension of datatype mtstring.

  author: Dirk Zacher
  parameters: -
  return value: size of x dimension of datatype mtstring
  exceptions: -

  */

  static int GetXDimensionSize();

  /*
  Method GetYDimensionSize returns the size of y dimension of datatype mtstring.

  author: Dirk Zacher
  parameters: -
  return value: size of y dimension of datatype mtstring
  exceptions: -

  */

  static int GetYDimensionSize();

  /*
  Method GetTDimensionSize returns the size of time dimension
  of datatype mtstring.

  author: Dirk Zacher
  parameters: -
  return value: size of time dimension of datatype mtstring
  exceptions: -

  */

  static int GetTDimensionSize();

  /*
  Method GetFlobElements returns the number of flob elements
  of datatype mtstring.

  author: Dirk Zacher
  parameters: -
  return value: number of flob elements of datatype mtstring
  exceptions: -

  */

  static int GetFlobElements();

  /*
  Method GetFlobSize returns the size of the flob of datatype mtstring.

  author: Dirk Zacher
  parameters: -
  return value: size of the flob of datatype mtstring
  exceptions: -

  */

  static SmiSize GetFlobSize();

  /*
  Method GetTypeName returns the typename of datatype mtstring.

  author: Dirk Zacher
  parameters: -
  return value: typename of datatype mtstring
  exceptions: -

  */

  static std::string GetTypeName();
};

}

#endif // TILEALGEBRA_MTSTRING_H
