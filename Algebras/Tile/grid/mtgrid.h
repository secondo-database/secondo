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

#ifndef TILEALGEBRA_MTGRID_H
#define TILEALGEBRA_MTGRID_H

/*
SECONDO includes

*/

#include "DateTime.h"

/*
TileAlgebra includes

*/

#include "tgrid.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Class mtgrid represents a 3-dimensional grid definition
with x origin, y origin, length of a grid cell and a duration
for the time dimension.

author: Dirk Zacher

*/

class mtgrid : public tgrid
{
  public:

  /*
  Constructor mtgrid does not initialize any members and
  should only be used in conjunction with Cast method.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  mtgrid();

  /*
  Constructor mtgrid sets defined flag of base class Attribute and
  initializes all members of the class with default values.

  author: Dirk Zacher
  parameters: bDefined - defined flag of base class Attribute
  return value: -
  exceptions: -

  */

  mtgrid(bool bDefined);

  /*
  Constructor mtgrid sets defined flag of base class Attribute to true and
  initializes all members of the class with corresponding parameter values.

  author: Dirk Zacher
  parameters: rX - reference to the x origin of the grid
              rY - reference to the y origin of the grid
              rLength - reference to the length of a grid cell
              rDuration - reference to the duration value of time dimension
  return value: -
  exceptions: -

  */

  mtgrid(const double& rX, const double& rY, const double& rLength,
         const datetime::DateTime& rDuration);

  /*
  Constructor mtgrid sets defined flag of base class Attribute to defined flag
  of rmtgrid object and initializes all members of the class with corresponding
  values of rmtgrid object.

  author: Dirk Zacher
  parameters: rmtgrid - reference to a mtgrid object
  return value: -
  exceptions: -

  */

  mtgrid(const mtgrid& rmtgrid);

  /*
  Destructor ~mtgrid deinitializes a mtgrid object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */
  
  virtual ~mtgrid();

  /*
  Operator= assigns all member values of a given mtgrid object
  to the corresponding member values of this object.

  author: Dirk Zacher
  parameters: rmtgrid - reference to a mtgrid object
  return value: reference to this object
  exceptions: -

  */

  const mtgrid& operator=(const mtgrid& rmtgrid);

  /*
  Operator== compares this object with given mtgrid object.

  author: Dirk Zacher
  parameters: rmtgrid - reference to a mtgrid object
  return value: true, if this object equals rmtgrid, otherwise false
  exceptions: -

  */

  bool operator==(const mtgrid& rmtgrid) const;

  /*
  Method GetDuration returns the duration of the mtgrid.

  author: Dirk Zacher
  parameters: -
  return value: duration of the mtgrid
  exceptions: -

  */

  const datetime::DateTime& GetDuration() const;

  /*
  Method SetDuration sets the duration of the mtgrid.

  author: Dirk Zacher
  parameters: rDuration - reference to duration
  return value: true, if duration successfully set, otherwise false
  exceptions: -

  */

  bool SetDuration(const datetime::DateTime& rDuration);

  /*
  Method IsMatchingGrid checks if this object matches given tgrid object.

  author: Dirk Zacher
  parameters: rtgrid - reference to a tgrid object
  return value: true, if this object matches rtgrid object, otherwise false
  exceptions: -

  */

  bool IsMatchingGrid(const tgrid& rtgrid) const;

  /*
  Method IsMatchingGrid checks if this object matches given mtgrid object.

  author: Dirk Zacher
  parameters: rmtgrid - reference to a mtgrid object
  return value: true, if this object matches rmtgrid object, otherwise false
  exceptions: -

  */

  bool IsMatchingGrid(const mtgrid& rmtgrid) const;

  protected:

  /*
  Method Reset resets all members of the class to the default values.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  void Reset();

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
  Method HashValue returns the hash value of the mtgrid object.

  author: Dirk Zacher
  parameters: -
  return value: hash value of the mtgrid object
  exceptions: -

  */

  virtual size_t HashValue() const;

  /*
  Method Sizeof returns the size of mtgrid datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of mtgrid datatype
  exceptions: -

  */

  virtual size_t Sizeof() const;

  /*
  Method BasicType returns the typename of mtgrid datatype.

  author: Dirk Zacher
  parameters: -
  return value: typename of mtgrid datatype
  exceptions: -

  */
  
  static const std::string BasicType();

  /*
  Method Cast casts a void pointer to a new mtgrid object.

  author: Dirk Zacher
  parameters: pVoid - a pointer to a memory address
  return value: a pointer to a new mtgrid object
  exceptions: -

  */

  static void* Cast(void* pVoid);

  /*
  Method Clone clones an existing mtgrid object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing mtgrid object
  return value: a Word that references a new mtgrid object
  exceptions: -

  */

  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);

  /*
  Method Close closes an existing mtgrid object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing mtgrid object
  return value: -
  exceptions: -

  */

  static void Close(const ListExpr typeInfo,
                    Word& rWord);

  /*
  Method Create creates a new mtgrid object.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of the new mtgrid object to create
  return value: a Word that references a new mtgrid object
  exceptions: -

  */

  static Word Create(const ListExpr typeInfo);

  /*
  Method Delete deletes an existing mtgrid object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing mtgrid object
  return value: -
  exceptions: -

  */

  static void Delete(const ListExpr typeInfo,
                     Word& rWord);

  /*
  Method GetTypeConstructor returns the TypeConstructor of class mtgrid.

  author: Dirk Zacher
  parameters: -
  return value: TypeConstructor of class mtgrid
  exceptions: -

  */

  static TypeConstructor GetTypeConstructor();

  /*
  Method In creates a new mtgrid object on the basis of a given ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object to create on the basis of instance
              instance - ListExpr of the mtgrid object to create
              errorPos - error position
              rErrorInfo - reference to error information
              rCorrect - flag that indicates if mtgrid object correctly created
  return value: a Word that references a new mtgrid object
  exceptions: -

  */

  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);

  /*
  Method KindCheck checks if given type is mtgrid type.

  author: Dirk Zacher
  parameters: type - ListExpr of type to check
              rErrorInfo - reference to error information
  return value: true, if type is mtgrid type, otherwise false
  exceptions: -

  */

  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);

  /*
  Method Open opens a mtgrid object from a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord containing mtgrid object to open
              rOffset - Offset to the mtgrid object in SmiRecord
              typeInfo - TypeInfo of mtgrid object to open
              rValue - reference to a Word referencing the opened mtgrid object
  return value: true, if mtgrid object was successfully opened, otherwise false
  exceptions: -

  */

  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method Out writes out an existing mtgrid object in the form of a ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of mtgrid object to write out
              value - reference to a Word referencing the mtgrid object
  return value: ListExpr of mtgrid object referenced by value
  exceptions: -

  */

  static ListExpr Out(ListExpr typeInfo,
                      Word value);

  /*
  Method Property returns all properties of mtgrid datatype.

  author: Dirk Zacher
  parameters: -
  return value: properties of mtgrid datatype in the form of a ListExpr
  exceptions: -

  */

  static ListExpr Property();

  /*
  Method Save saves an existing mtgrid object in a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord to save existing mtgrid object
              rOffset - Offset to save position of mtgrid object in SmiRecord
              typeInfo - TypeInfo of mtgrid object to save
              rValue - reference to a Word referencing the mtgrid object to save
  return value: true, if mtgrid object was successfully saved, otherwise false
  exceptions: -

  */

  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method SizeOfObj returns the size of a mtgrid object.

  author: Dirk Zacher
  parameters: -
  return value: size of a mtgrid object
  exceptions: -

  */
  
  static int SizeOfObj();

  private:

  /*
  Member m_Duration contains the duration for the time dimension.

  */ 
  
  datetime::DateTime m_Duration;
};

}

#endif // TILEALGEBRA_MTGRID_H
