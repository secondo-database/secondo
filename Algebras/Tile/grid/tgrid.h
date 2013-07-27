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

#ifndef TILEALGEBRA_TGRID_H
#define TILEALGEBRA_TGRID_H

/*
SECONDO includes

*/

#include "Attribute.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Class tgrid represents a 2-dimensional grid definition
with x origin, y origin and length of a grid cell.

author: Dirk Zacher

*/

class tgrid : public Attribute
{
  public:

  /*
  Constructor tgrid does not initialize any members and
  should only be used in conjunction with Cast method.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  tgrid();

  /*
  Constructor tgrid sets defined flag of base class Attribute and
  initializes all members of the class with default values.

  author: Dirk Zacher
  parameters: bDefined - defined flag of base class Attribute
  return value: -
  exceptions: -

  */

  tgrid(bool bDefined);

  /*
  Constructor tgrid sets defined flag of base class Attribute to true and
  initializes all members of the class with corresponding parameter values.

  author: Dirk Zacher
  parameters: rX - reference to the x origin of the grid
              rY - reference to the y origin of the grid
              rLength - reference to the length of a grid cell
  return value: -
  exceptions: -

  */

  tgrid(const double& rX, const double& rY, const double& rLength);

  /*
  Constructor tgrid sets defined flag of base class Attribute to defined flag
  of rtgrid object and initializes all members of the class with corresponding
  values of rtgrid object.

  author: Dirk Zacher
  parameters: rtgrid - reference to a tgrid object
  return value: -
  exceptions: -

  */

  tgrid(const tgrid& rtgrid);

  /*
  Destructor ~tgrid deinitializes a tgrid object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  virtual ~tgrid();

  /*
  Operator= assigns all member values of a given tgrid object
  to the corresponding member values of this object.

  author: Dirk Zacher
  parameters: rtgrid - reference to a tgrid object
  return value: reference to this object
  exceptions: -

  */

  const tgrid& operator=(const tgrid& rtgrid);

  /*
  Operator== compares this object with given tgrid object.

  author: Dirk Zacher
  parameters: rtgrid - reference to a tgrid object
  return value: true, if this object equals rtgrid, otherwise false
  exceptions: -

  */

  bool operator==(const tgrid& rtgrid) const;

  /*
  Method GetX returns the x origin of the tgrid.

  author: Dirk Zacher
  parameters: -
  return value: x origin of the tgrid
  exceptions: -

  */

  const double& GetX() const;

  /*
  Method GetY returns the y origin of the tgrid.

  author: Dirk Zacher
  parameters: -
  return value: y origin of the tgrid
  exceptions: -

  */

  const double& GetY() const ;

  /*
  Method GetLength returns the length of the a tgrid cell.

  author: Dirk Zacher
  parameters: -
  return value: length of the a tgrid cell
  exceptions: -

  */

  const double& GetLength() const;

  /*
  Method SetX sets the x origin of the tgrid.

  author: Dirk Zacher
  parameters: rX - reference to x origin
  return value: true, if x origin successfully set, otherwise false
  exceptions: -

  */

  bool SetX(const double& rX);

  /*
  Method SetY sets the y origin of the tgrid.

  author: Dirk Zacher
  parameters: rY - reference to y origin
  return value: true, if y origin successfully set, otherwise false
  exceptions: -

  */

  bool SetY(const double& rY);

  /*
  Method SetLength sets the length of a grid cell.

  author: Dirk Zacher
  parameters: rLength - reference to length of grid cell
  return value: true, if length of grid cell successfully set, otherwise false
  exceptions: -

  */

  bool SetLength(const double& rLength);

  /*
  Method IsMatchingGrid checks if this object matches given tgrid object.

  author: Dirk Zacher
  parameters: rtgrid - reference to a tgrid object
  return value: true, if this object matches rtgrid object, otherwise false
  exceptions: -

  */

  bool IsMatchingGrid(const tgrid& rtgrid) const;

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
  Method HashValue returns the hash value of the tgrid object.

  author: Dirk Zacher
  parameters: -
  return value: hash value of the tgrid object
  exceptions: -

  */

  virtual size_t HashValue() const;

  /*
  Method Sizeof returns the size of tgrid datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of tgrid datatype
  exceptions: -

  */

  virtual size_t Sizeof() const;

  /*
  Method BasicType returns the typename of tgrid datatype.

  author: Dirk Zacher
  parameters: -
  return value: typename of tgrid datatype
  exceptions: -

  */
  
  static const std::string BasicType();

  /*
  Method Cast casts a void pointer to a new tgrid object.

  author: Dirk Zacher
  parameters: pVoid - a pointer to a memory address
  return value: a pointer to a new tgrid object
  exceptions: -

  */

  static void* Cast(void* pVoid);

  /*
  Method Clone clones an existing tgrid object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing tgrid object
  return value: a Word that references a new tgrid object
  exceptions: -

  */

  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);

  /*
  Method Close closes an existing tgrid object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing tgrid object
  return value: -
  exceptions: -

  */

  static void Close(const ListExpr typeInfo,
                    Word& rWord);

  /*
  Method Create creates a new tgrid object.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of the new tgrid object to create
  return value: a Word that references a new tgrid object
  exceptions: -

  */

  static Word Create(const ListExpr typeInfo);

  /*
  Method Delete deletes an existing tgrid object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing tgrid object
  return value: -
  exceptions: -

  */

  static void Delete(const ListExpr typeInfo,
                     Word& rWord);

  /*
  Method GetTypeConstructor returns the TypeConstructor of class tgrid.

  author: Dirk Zacher
  parameters: -
  return value: TypeConstructor of class tgrid
  exceptions: -

  */

  static TypeConstructor GetTypeConstructor();

  /*
  Method In creates a new tgrid object on the basis of a given ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object to create on the basis of instance
              instance - ListExpr of the tgrid object to create
              errorPos - error position
              rErrorInfo - reference to error information
              rCorrect - flag that indicates if tgrid object correctly created
  return value: a Word that references a new tgrid object
  exceptions: -

  */

  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);

  /*
  Method KindCheck checks if given type is tgrid type.

  author: Dirk Zacher
  parameters: type - ListExpr of type to check
              rErrorInfo - reference to error information
  return value: true, if type is tgrid type, otherwise false
  exceptions: -

  */

  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);

  /*
  Method Open opens a tgrid object from a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord containing tgrid object to open
              rOffset - Offset to the tgrid object in SmiRecord
              typeInfo - TypeInfo of tgrid object to open
              rValue - reference to a Word referencing the opened tgrid object
  return value: true, if tgrid object was successfully opened, otherwise false
  exceptions: -

  */

  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method Out writes out an existing tgrid object in the form of a ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of tgrid object to write out
              value - reference to a Word referencing the tgrid object
  return value: ListExpr of tgrid object referenced by value
  exceptions: -

  */

  static ListExpr Out(ListExpr typeInfo,
                      Word value);

  /*
  Method Property returns all properties of tgrid datatype.

  author: Dirk Zacher
  parameters: -
  return value: properties of tgrid datatype in the form of a ListExpr
  exceptions: -

  */

  static ListExpr Property();

  /*
  Method Save saves an existing tgrid object in a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord to save existing tgrid object
              rOffset - Offset to save position of tgrid object in SmiRecord
              typeInfo - TypeInfo of tgrid object to save
              rValue - reference to a Word referencing the tgrid object to save
  return value: true, if tgrid object was successfully saved, otherwise false
  exceptions: -

  */

  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method SizeOfObj returns the size of a tgrid object.

  author: Dirk Zacher
  parameters: -
  return value: size of a tgrid object
  exceptions: -

  */

  static int SizeOfObj();

  private:

  /*
  Member m_dX contains the x origin of the grid.

  */

  double m_dX;

  /*
  Member m_dY contains the y origin of the grid.

  */

  double m_dY;

  /*
  Member m_dLength contains the length of a grid cell.

  */

  double m_dLength;
};

}

#endif // TILEALGEBRA_TGRID_H
