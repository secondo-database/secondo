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

#ifndef TILEALGEBRA_TINTFLOB_H
#define TILEALGEBRA_TINTFLOB_H

/*
SECONDO includes

*/

#include "Attribute.h"
#include "../../Tools/Flob/Flob.h"

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
Class tintFlob represents a test implementation of a tint datatype
that stores all values in a Flob.

author: Dirk Zacher

*/


class tintFlob : public Attribute
{  
  private:

  /*
  Constructor tintFlob does not initialize any members and
  should only be used in conjunction with Cast method.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */
  
  tintFlob();

  public:

  /*
  Constructor tintFlob sets defined flag of base class Attribute and
  initializes all members of the class with default values.

  author: Dirk Zacher
  parameters: bDefined - defined flag of base class Attribute
  return value: -
  exceptions: -

  */

  tintFlob(bool bDefined);

  /*
  Constructor tintFlob sets defined flag of base class Attribute to defined flag
  of rtintFlob object and initializes all members of the class
  with corresponding values of rtintFlob object.

  author: Dirk Zacher
  parameters: rtintFlob - reference to a tintFlob object
  return value: -
  exceptions: -

  */

  tintFlob(const tintFlob& rtintFlob);
  
  /*
  Destructor ~tintFlob deinitializes a tintFlob object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */
  
  virtual ~tintFlob();
  
  /*
  Operator= assigns all member values of a given tintFlob object
  to the corresponding member values of this object.

  author: Dirk Zacher
  parameters: rtintFlob - reference to a tintFlob object
  return value: reference to this object
  exceptions: -

  */
  
  tintFlob& operator=(const tintFlob& rtintFlob);

  /*
  Operator== checks if this object equals rtintFlob object.

  author: Dirk Zacher
  parameters: rtintFlob - reference to a tintFlob object
  return value: true, if this object equals rtintFlob object, otherwise false
  exceptions: -

  */

  bool operator==(const tintFlob& rtintFlob) const;
  
  /*
  TileAlgebra operator load loads all values of the tintFlob object.

  author: Dirk Zacher
  parameters: -
  return value: true, if all values of the tintFlob object successfully loaded,
                otherwise false
  exceptions: -

  */

  bool load();
  
  /*
  Method Destroy destroys tintFlob object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */
  
  void Destroy();

  /*
  Method SetGrid sets the tgrid properties of tintFlob object.

  author: Dirk Zacher
  parameters: rX - reference to the x origin of the grid
              rY - reference to the y origin of the grid
              rLength - reference to the length of a grid cell
  return value: true, if tgrid properties of tintFlob object successfully set,
                otherwise false
  exceptions: -

  */

  bool SetGrid(const double& rX,
               const double& rY,
               const double& rLength);

  /*
  Method SetValue sets a value of tintFlob object at given index.

  author: Dirk Zacher
  parameters: nIndex - index of tintFlob value
              nValue - value
  return value: true, if nValue was successfully set at nIndex, otherwise false
  exceptions: -

  */

  bool SetValue(int nIndex,
                int nValue);

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
  Method HashValue returns the hash value of the tintFlob object.

  author: Dirk Zacher
  parameters: -
  return value: hash value of the tintFlob object
  exceptions: -

  */

  virtual size_t HashValue() const;

  /*
  Method NumOfFLOBs returns the number of Flobs of a tintFlob object.

  author: Dirk Zacher
  parameters: -
  return value: number of Flobs of a tintFlob object
  exceptions: -

  */

  virtual int NumOfFLOBs() const;

  /*
  Method Sizeof returns the size of tintFlob datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of tintFlob datatype
  exceptions: -

  */

  virtual size_t Sizeof() const;
  
  /*
  Method BasicType returns the typename of tintFlob datatype.

  author: Dirk Zacher
  parameters: -
  return value: typename of tintFlob datatype
  exceptions: -

  */
  
  static const string BasicType();

  /*
  Method Cast casts a void pointer to a new tintFlob object.

  author: Dirk Zacher
  parameters: pVoid - a pointer to a memory address
  return value: a pointer to a new tintFlob object
  exceptions: -

  */

  static void* Cast(void* pVoid);

  /*
  Method Clone clones an existing tintFlob object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing tintFlob object
  return value: a Word that references a new tintFlob object
  exceptions: -

  */

  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);

  /*
  Method Close closes an existing tintFlob object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing tintFlob object
  return value: -
  exceptions: -

  */

  static void Close(const ListExpr typeInfo,
                    Word& rWord);

  /*
  Method Create creates a new tintFlob object.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of the new tintFlob object to create
  return value: a Word that references a new tintFlob object
  exceptions: -

  */

  static Word Create(const ListExpr typeInfo);

  /*
  Method Delete deletes an existing tintFlob object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing tintFlob object
  return value: -
  exceptions: -

  */

  static void Delete(const ListExpr typeInfo,
                     Word& rWord);

  /*
  Method GetTypeConstructor returns the TypeConstructor of class tintFlob.

  author: Dirk Zacher
  parameters: -
  return value: TypeConstructor of class tintFlob
  exceptions: -

  */

  static TypeConstructor GetTypeConstructor();

  /*
  Method In creates a new tintFlob object on the basis of a given ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object to create on the basis of instance
              instance - ListExpr of the tintFlob object to create
              errorPos - error position
              rErrorInfo - reference to error information
              rCorrect - flag that indicates if tintFlob object
                         correctly created
  return value: a Word that references a new tintFlob object
  exceptions: -

  */

  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);

  /*
  Method KindCheck checks if given type is tintFlob type.

  author: Dirk Zacher
  parameters: type - ListExpr of type to check
              rErrorInfo - reference to error information
  return value: true, if type is tintFlob type, otherwise false
  exceptions: -

  */

  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);

  /*
  Method Open opens a tintFlob object from a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord containing tintFlob object to open
              rOffset - Offset to the tintFlob object in SmiRecord
              typeInfo - TypeInfo of tintFlob object to open
              rValue - reference to a Word referencing the opened
                       tintFlob object
  return value: true, if tintFlob object was successfully opened,
                otherwise false
  exceptions: -

  */

  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method Out writes out an existing tintFlob object in the form of a ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of tintFlob object to write out
              value - reference to a Word referencing the tintFlob object
  return value: ListExpr of tintFlob object referenced by value
  exceptions: -

  */

  static ListExpr Out(ListExpr typeInfo,
                      Word value);

  /*
  Method Property returns all properties of tintFlob datatype.

  author: Dirk Zacher
  parameters: -
  return value: properties of tintFlob datatype in the form of a ListExpr
  exceptions: -

  */

  static ListExpr Property();

  /*
  Method Save saves an existing tintFlob object in a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord to save existing tintFlob object
              rOffset - Offset to save position of tintFlob object in SmiRecord
              typeInfo - TypeInfo of tintFlob object to save
              rValue - reference to a Word referencing
                       the tintFlob object to save
  return value: true, if tintFlob object was successfully saved,
                otherwise false
  exceptions: -

  */

  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method SizeOfObj returns the size of a tintFlob object.

  author: Dirk Zacher
  parameters: -
  return value: size of a tintFlob object
  exceptions: -

  */

  static int SizeOfObj();
    
  private:

  /*
  Member m_Grid contains the tgrid object of tintFlob object.

  */

  tgrid m_Grid;

  /*
  Member m_Flob contains the Flob to store all values of tintFlob object.

  */

  Flob m_Flob;
};

}

#endif // TILEALGEBRA_TINTFLOB_H
