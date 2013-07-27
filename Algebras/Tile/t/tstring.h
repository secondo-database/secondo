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

#ifndef TILEALGEBRA_TSTRING_H
#define TILEALGEBRA_TSTRING_H

/*
system includes

*/

#include <string>

/*
SECONDO includes

*/

#include "RectangleAlgebra.h"

/*
TileAlgebra includes

*/

#include "t.h"
#include "tProperties.h"
#include "tint.h"
#include "../grid/tgrid.h"
#include "../Properties/Propertiesstring.h"
#include "../UniqueStringArray/UniqueStringArray.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Class tstring represents the implementation of datatype tstring.

author: Dirk Zacher

*/

class tstring : public tint
{
  protected:

  /*
  Constructor tstring does not initialize any members and
  should only be used in conjunction with Cast method.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  tstring();

  public:

  /*
  Constructor tstring sets defined flag of base class Attribute and
  initializes all members of the class with default values.

  author: Dirk Zacher
  parameters: bDefined - defined flag of base class Attribute
  return value: -
  exceptions: -

  */

  tstring(bool bDefined);

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

  tstring(const tint& rtint,
          const UniqueStringArray& rUniqueStringArray);

  /*
  Constructor tstring sets defined flag of base class Attribute to defined flag
  of rtstring object and initializes all members of the class with corresponding
  values of rtstring object.

  author: Dirk Zacher
  parameters: rtstring - reference to a tstring object
  return value: -
  exceptions: -

  */

  tstring(const tstring& rtstring);

  /*
  Destructor ~tstring deinitializes a tstring object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  virtual ~tstring();

  /*
  Operator= assigns all member values of a given tstring object
  to the corresponding member values of this object.

  author: Dirk Zacher
  parameters: rtstring - reference to a tstring object
  return value: reference to this object
  exceptions: -

  */

  tstring& operator=(const tstring& rtstring);

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

  void atlocation(const double& rX,
                  const double& rY,
                  CcString& rValue) const;

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
  
  void atrange(const Rectangle<2>& rRectangle,
               tstring& rtstring) const;

  /*
  TODO: delete method implementation after refactoring of atrange operator

  TileAlgebra operator atrange returns all values of a tstring object
  inside the given rectangle.

  author: Dirk Zacher
  parameters: rRectangle - reference to a Rectangle<2> object
              rInstant1 - reference to the first Instant value
              rInstant2 - reference to the second Instant value
              rtstring - reference to a tstring object containing all values
                         of the tstring object inside the given rectangle
  return value: -
  exceptions: -

  */

  void atrange(const Rectangle<2>& rRectangle,
               const double& rInstant1,
               const double& rInstant2,
               tstring& rtstring) const;

  /*
  TileAlgebra operator minimum returns the minimum value of tstring object.

  author: Dirk Zacher
  parameters: -
  return value: minimum value of tstring object
  exceptions: -

  */

  std::string minimum() const;

  /*
  TileAlgebra operator maximum returns the maximum value of tstring object.

  author: Dirk Zacher
  parameters: -
  return value: maximum value of tstring object
  exceptions: -

  */

  std::string maximum() const;

  /*
  Method GetValue returns the string value of tstring object
  at given 2-dimensional index.

  author: Dirk Zacher
  parameters: rIndex - reference to a 2-dimensional index
  return value: string value of tstring object at given 2-dimensional index
  exceptions: -

  */

  std::string GetValue(const Index<2>& rIndex) const;

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

  bool SetValue(const Index<2>& rIndex,
                const std::string& rValue,
                bool bSetExtrema);

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

  bool SetValue(const double& rX,
                const double& rY,
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
  Method HashValue returns the hash value of the tstring object.

  author: Dirk Zacher
  parameters: -
  return value: hash value of the tstring object
  exceptions: -

  */

  virtual size_t HashValue() const;

  /*
  Method NumOfFLOBs returns the number of Flobs of a tstring object.

  author: Dirk Zacher
  parameters: -
  return value: number of Flobs of a tstring object
  exceptions: -

  */

  virtual int NumOfFLOBs() const;

  /*
  Method Sizeof returns the size of tstring datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of tstring datatype
  exceptions: -

  */

  virtual size_t Sizeof() const;

  /*
  Method BasicType returns the typename of tstring datatype.

  author: Dirk Zacher
  parameters: -
  return value: typename of tstring datatype
  exceptions: -

  */

  static const std::string BasicType();

  /*
  Method Cast casts a void pointer to a new tstring object.

  author: Dirk Zacher
  parameters: pVoid - a pointer to a memory address
  return value: a pointer to a new tstring object
  exceptions: -

  */

  static void* Cast(void* pVoid);

  /*
  Method Clone clones an existing tstring object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing tstring object
  return value: a Word that references a new tstring object
  exceptions: -

  */

  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);

  /*
  Method Close closes an existing tstring object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing tstring object
  return value: -
  exceptions: -

  */

  static void Close(const ListExpr typeInfo,
                    Word& rWord);

  /*
  Method Create creates a new tstring object.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of the new tstring object to create
  return value: a Word that references a new tstring object
  exceptions: -

  */

  static Word Create(const ListExpr typeInfo);

  /*
  Method Delete deletes an existing tstring object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing tstring object
  return value: -
  exceptions: -

  */

  static void Delete(const ListExpr typeInfo,
                     Word& rWord);

  /*
  Method GetTypeConstructor returns the TypeConstructor of class tstring.

  author: Dirk Zacher
  parameters: -
  return value: TypeConstructor of class tstring
  exceptions: -

  */

  static TypeConstructor GetTypeConstructor();

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

  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);

  /*
  Method KindCheck checks if given type is tstring type.

  author: Dirk Zacher
  parameters: type - ListExpr of type to check
              rErrorInfo - reference to error information
  return value: true, if type is tstring type, otherwise false
  exceptions: -

  */

  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);

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

  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method Out writes out an existing tstring object in the form of a ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of tstring object to write out
              value - reference to a Word referencing the tstring object
  return value: ListExpr of tstring object referenced by value
  exceptions: -

  */

  static ListExpr Out(ListExpr typeInfo,
                      Word value);

  /*
  Method Property returns all properties of tstring datatype.

  author: Dirk Zacher
  parameters: -
  return value: properties of tstring datatype in the form of a ListExpr
  exceptions: -

  */

  static ListExpr Property();

  /*
  Method Save saves an existing tstring object in a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord to save existing tstring object
              rOffset - Offset to save position of tstring object in SmiRecord
              typeInfo - TypeInfo of tstring object to save
              rValue - reference to a Word referencing
                       the tstring object to save
  return value: true, if tstring object was successfully saved, otherwise false
  exceptions: -

  */

  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method SizeOfObj returns the size of a tstring object.

  author: Dirk Zacher
  parameters: -
  return value: size of a tstring object
  exceptions: -

  */

  static int SizeOfObj();

  private:

  /*
  Member m_UniqueStringArray contains all unique strings used by tstring object.

  */

  UniqueStringArray m_UniqueStringArray;
};

/*
Class tProperties<std::string> represents the properties of datatype tstring.

author: Dirk Zacher

*/

template <>
class tProperties<std::string>
{
  public:

  /*
  typedef of PropertiesType

  */

  typedef tstring PropertiesType;

  /*
  typedef of TypeProperties

  */

  typedef Properties<std::string> TypeProperties;

  /*
  typedef of GridType

  */

  typedef tgrid GridType;

  /*
  typedef of RectangleType

  */

  typedef Rectangle<2> RectangleType;

  /*
  Method GetXDimensionSize returns the size of x dimension of datatype tstring.

  author: Dirk Zacher
  parameters: -
  return value: size of x dimension of datatype tstring
  exceptions: -

  */

  static int GetXDimensionSize();

  /*
  Method GetYDimensionSize returns the size of y dimension of datatype tstring.

  author: Dirk Zacher
  parameters: -
  return value: size of y dimension of datatype tstring
  exceptions: -

  */

  static int GetYDimensionSize();

  /*
  Method GetFlobElements returns the number of flob elements
  of datatype tstring.

  author: Dirk Zacher
  parameters: -
  return value: number of flob elements of datatype tstring
  exceptions: -

  */

  static int GetFlobElements();

  /*
  Method GetFlobSize returns the size of the flob of datatype tstring.

  author: Dirk Zacher
  parameters: -
  return value: size of the flob of datatype tstring
  exceptions: -

  */

  static SmiSize GetFlobSize();

  /*
  Method GetTypeName returns the typename of datatype tstring.

  author: Dirk Zacher
  parameters: -
  return value: typename of datatype tstring
  exceptions: -

  */

  static std::string GetTypeName();
};

}

#endif // TILEALGEBRA_TSTRING_H
