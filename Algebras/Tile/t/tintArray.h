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

#ifndef TILEALGEBRA_TINTARRAY_H
#define TILEALGEBRA_TINTARRAY_H

/*
SECONDO includes

*/

#include "Attribute.h"

/*
TileAlgebra includes

*/

#include "../grid/tgrid.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Class tintArray represents a test implementation of a tint datatype
that stores all values in an array of datatype int.

author: Dirk Zacher

*/

class tintArray : public Attribute
{  
  private:

  /*
  Constructor tintArray does not initialize any members and
  should only be used in conjunction with Cast method.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  tintArray();

  public:

  /*
  Constructor tintArray sets defined flag of base class Attribute and
  initializes all members of the class with default values.

  author: Dirk Zacher
  parameters: bDefined - defined flag of base class Attribute
  return value: -
  exceptions: -

  */

  tintArray(bool bDefined);

  /*
  Constructor tintArray sets defined flag of base class Attribute
  to defined flag of rtintArray object and initializes all members of the class
  with corresponding values of rtintArray object.

  author: Dirk Zacher
  parameters: rtintArray - reference to a tintArray object
  return value: -
  exceptions: -

  */

  tintArray(const tintArray& rtintArray);
  
  /*
  Destructor ~tintArray deinitializes a tintArray object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */
  
  virtual ~tintArray();
  
  /*
  Operator= assigns all member values of a given tintArray object
  to the corresponding member values of this object.

  author: Dirk Zacher
  parameters: rtintArray - reference to a tintArray object
  return value: reference to this object
  exceptions: -

  */
  
  tintArray& operator=(const tintArray& rtintArray);

  /*
  Operator== checks if this object equals rtintArray object.

  author: Dirk Zacher
  parameters: rtintArray - reference to a tintArray object
  return value: true, if this object equals rtintArray object, otherwise false
  exceptions: -

  */

  bool operator==(const tintArray& rtintArray) const;

  /*
  TileAlgebra operator load loads all values of the tintArray object.

  author: Dirk Zacher
  parameters: -
  return value: true, if all values of the tintArray object successfully loaded,
                otherwise false
  exceptions: -

  */

  bool load();

  private:

  /*
  Method SetGrid sets the tgrid properties of tintArray object.

  author: Dirk Zacher
  parameters: rX - reference to the x origin of the grid
              rY - reference to the y origin of the grid
              rLength - reference to the length of a grid cell
  return value: true, if tgrid properties of tintArray object successfully set,
                otherwise false
  exceptions: -

  */

  bool SetGrid(const double& rX,
               const double& rY,
               const double& rLength);

  /*
  Method SetValue sets a value of tintArray object at given index.

  author: Dirk Zacher
  parameters: nIndex - index of tintArray value
              nValue - value
  return value: true, if nValue was successfully set at nIndex, otherwise false
  exceptions: -

  */

  bool SetValue(int nIndex,
                int nValue);

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
  Method HashValue returns the hash value of the tintArray object.

  author: Dirk Zacher
  parameters: -
  return value: hash value of the tintArray object
  exceptions: -

  */

  virtual size_t HashValue() const;

  /*
  Method Sizeof returns the size of tintArray datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of tintArray datatype
  exceptions: -

  */

  virtual size_t Sizeof() const;
  
  /*
  Method BasicType returns the typename of tintArray datatype.

  author: Dirk Zacher
  parameters: -
  return value: typename of tintArray datatype
  exceptions: -

  */
  
  static const std::string BasicType();

  /*
  Method Cast casts a void pointer to a new tintArray object.

  author: Dirk Zacher
  parameters: pVoid - a pointer to a memory address
  return value: a pointer to a new tintArray object
  exceptions: -

  */

  static void* Cast(void* pVoid);

  /*
  Method Clone clones an existing tintArray object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing tintArray object
  return value: a Word that references a new tintArray object
  exceptions: -

  */

  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);

  /*
  Method Close closes an existing tintArray object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing tintArray object
  return value: -
  exceptions: -

  */

  static void Close(const ListExpr typeInfo,
                    Word& rWord);

  /*
  Method Create creates a new tintArray object.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of the new tintArray object to create
  return value: a Word that references a new tintArray object
  exceptions: -

  */

  static Word Create(const ListExpr typeInfo);

  /*
  Method Delete deletes an existing tintArray object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing tintArray object
  return value: -
  exceptions: -

  */

  static void Delete(const ListExpr typeInfo,
                     Word& rWord);

  /*
  Method GetTypeConstructor returns the TypeConstructor of class tintArray.

  author: Dirk Zacher
  parameters: -
  return value: TypeConstructor of class tintArray
  exceptions: -

  */

  static TypeConstructor GetTypeConstructor();

  /*
  Method In creates a new tintArray object on the basis of a given ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object to create on the basis of instance
              instance - ListExpr of the tintArray object to create
              errorPos - error position
              rErrorInfo - reference to error information
              rCorrect - flag that indicates if tintArray object
                         correctly created
  return value: a Word that references a new tintArray object
  exceptions: -

  */

  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);

  /*
  Method KindCheck checks if given type is tintArray type.

  author: Dirk Zacher
  parameters: type - ListExpr of type to check
              rErrorInfo - reference to error information
  return value: true, if type is tintArray type, otherwise false
  exceptions: -

  */

  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);

  /*
  Method Open opens a tintArray object from a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord containing tintArray object to open
              rOffset - Offset to the tintArray object in SmiRecord
              typeInfo - TypeInfo of tintArray object to open
              rValue - reference to a Word referencing the opened
                       tintArray object
  return value: true, if tintArray object was successfully opened,
                otherwise false
  exceptions: -

  */

  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method Out writes out an existing tintArray object in the form of a ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of tintArray object to write out
              value - reference to a Word referencing the tintArray object
  return value: ListExpr of tintArray object referenced by value
  exceptions: -

  */

  static ListExpr Out(ListExpr typeInfo,
                      Word value);

  /*
  Method Property returns all properties of tintArray datatype.

  author: Dirk Zacher
  parameters: -
  return value: properties of tintArray datatype in the form of a ListExpr
  exceptions: -

  */

  static ListExpr Property();

  /*
  Method Save saves an existing tintArray object in a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord to save existing tintArray object
              rOffset - Offset to save position of tintArray object in SmiRecord
              typeInfo - TypeInfo of tintArray object to save
              rValue - reference to a Word referencing
                       the tintArray object to save
  return value: true, if tintArray object was successfully saved,
                otherwise false
  exceptions: -

  */

  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method SizeOfObj returns the size of a tintArray object.

  author: Dirk Zacher
  parameters: -
  return value: size of a tintArray object
  exceptions: -

  */

  static int SizeOfObj();
    
  private:

  /*
  Member m_Grid contains the tgrid object of tintArray object.

  */

  tgrid m_Grid;

  /*
  Member m_Array contains all values of tintArray object.

  */

  int m_Array[961];
};

}

#endif // TILEALGEBRA_TINTARRAY_H
