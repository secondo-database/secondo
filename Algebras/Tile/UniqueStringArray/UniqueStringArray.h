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

#ifndef TILEALGEBRA_UNIQUESTRINGARRAY_H
#define TILEALGEBRA_UNIQUESTRINGARRAY_H

/*
SECONDO includes

*/

#include "Attribute.h"
#include "../../Tools/Flob/DbArray.h"
#include "../../Tools/Flob/Flob.h"

/*
TileAlgebra includes

*/

#include "StringData.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Class UniqueStringArray represents an array of unique strings.

author: Dirk Zacher

*/

class UniqueStringArray : public Attribute
{
  public:

  /*
  Constructor UniqueStringArray does not initialize any members and
  should only be used in conjunction with Cast method.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  UniqueStringArray();
  
  /*
  Constructor UniqueStringArray sets defined flag of base class Attribute and
  initializes all members of the class with default values.

  author: Dirk Zacher
  parameters: bDefined - defined flag of base class Attribute
  return value: -
  exceptions: -

  */

  UniqueStringArray(bool bDefined);

  /*
  Constructor UniqueStringArray sets defined flag of base class Attribute
  to defined flag of rUniqueStringArray object and initializes all
  members of the class with corresponding values of rUniqueStringArray object.

  author: Dirk Zacher
  parameters: rUniqueStringArray - reference to a UniqueStringArray object
  return value: -
  exceptions: -

  */
  
  UniqueStringArray(const UniqueStringArray& rUniqueStringArray);
  
  /*
  Destructor ~UniqueStringArray deinitializes a UniqueStringArray object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */
  
  virtual ~UniqueStringArray();
  
  /*
  Operator= assigns all member values of a given UniqueStringArray object
  to the corresponding member values of this object.

  author: Dirk Zacher
  parameters: rUniqueStringArray - reference to a UniqueStringArray object
  return value: reference to this object
  exceptions: -

  */
  
  UniqueStringArray& operator=(const UniqueStringArray& rUniqueStringArray);
  
  /*
  Method GetUniqueString returns the string with given index.

  author: Dirk Zacher
  parameters: nIndex - index of string
              rString - reference to a string containing
                        the string with given index
  return value: true, if nIndex is a valid string index and rString contains
                the string with given index, otherwise false
  exceptions: -

  */

  bool GetUniqueString(int nIndex, std::string& rString) const;
  
  /*
  Method GetUniqueStringArray returns all unique strings.

  author: Dirk Zacher
  parameters: rUniqueStringArray - reference to a vector of strings
                                   containing all unique strings
  return value: -
  exceptions: -

  */
  
  void GetUniqueStringArray(std::vector<std::string>& rUniqueStringArray) const;

  /*
  Method GetUniqueStringIndex returns the index of a unique string.

  author: Dirk Zacher
  parameters: rString - reference to a string
  return value: index of unique string if rString exists in UniqueStringArray,
                otherwise UNDEFINED_STRING_INDEX
  exceptions: -

  */

  int GetUniqueStringIndex(const std::string& rString) const;

  /*
  Method AddString adds given string to UniqueStringArray if given string
  does not exist in UniqueStringArray and returns the index of given string.

  author: Dirk Zacher
  parameters: rString - reference to a string
  return value: index of given string in UniqueStringArray
  exceptions: -

  */

  int AddString(const std::string& rString);

  /*
  Method Destroy destroys UniqueStringArray object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  void Destroy();

  private:

  /*
  Method IsUniqueString checks if given string is an unique string.

  author: Dirk Zacher
  parameters: rString - reference to a string
  return value: true, if given string is an unique string, otherwise false
  exceptions: -

  */
  
  bool IsUniqueString(const std::string& rString) const;

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
  Method HashValue returns the hash value of the UniqueStringArray object.

  author: Dirk Zacher
  parameters: -
  return value: hash value of the UniqueStringArray object
  exceptions: -

  */

  virtual size_t HashValue() const;

  /*
  Method NumOfFLOBs returns the number of Flobs of a UniqueStringArray object.

  author: Dirk Zacher
  parameters: -
  return value: number of Flobs of a UniqueStringArray object
  exceptions: -

  */

  virtual int NumOfFLOBs() const;

  /*
  Method Sizeof returns the size of UniqueStringArray datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of UniqueStringArray datatype
  exceptions: -

  */

  virtual size_t Sizeof() const;
  
  /*
  Method BasicType returns the typename of UniqueStringArray datatype.

  author: Dirk Zacher
  parameters: -
  return value: typename of UniqueStringArray datatype
  exceptions: -

  */
  
  static const std::string BasicType();

  /*
  Method Cast casts a void pointer to a new UniqueStringArray object.

  author: Dirk Zacher
  parameters: pVoid - a pointer to a memory address
  return value: a pointer to a new UniqueStringArray object
  exceptions: -

  */

  static void* Cast(void* pVoid);

  /*
  Method Clone clones an existing UniqueStringArray object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing
                      UniqueStringArray object
  return value: a Word that references a new UniqueStringArray object
  exceptions: -

  */

  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);

  /*
  Method Close closes an existing UniqueStringArray object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing
                      UniqueStringArray object
  return value: -
  exceptions: -

  */

  static void Close(const ListExpr typeInfo,
                    Word& rWord);

  /*
  Method Create creates a new UniqueStringArray object.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of the new UniqueStringArray object to create
  return value: a Word that references a new UniqueStringArray object
  exceptions: -

  */

  static Word Create(const ListExpr typeInfo);

  /*
  Method Delete deletes an existing UniqueStringArray object
  given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing
                      UniqueStringArray object
  return value: -
  exceptions: -

  */

  static void Delete(const ListExpr typeInfo,
                     Word& rWord);

  /*
  Method GetTypeConstructor returns the TypeConstructor
  of class UniqueStringArray.

  author: Dirk Zacher
  parameters: -
  return value: TypeConstructor of class UniqueStringArray
  exceptions: -

  */

  static TypeConstructor GetTypeConstructor();

  /*
  Method In creates a new UniqueStringArray object
  on the basis of a given ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object to create on the basis of instance
              instance - ListExpr of the UniqueStringArray object to create
              errorPos - error position
              rErrorInfo - reference to error information
              rCorrect - flag that indicates if UniqueStringArray object
                         correctly created
  return value: a Word that references a new UniqueStringArray object
  exceptions: -

  */

  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);

  /*
  Method KindCheck checks if given type is UniqueStringArray type.

  author: Dirk Zacher
  parameters: type - ListExpr of type to check
              rErrorInfo - reference to error information
  return value: true, if type is UniqueStringArray type, otherwise false
  exceptions: -

  */

  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);

  /*
  Method Open opens a UniqueStringArray object from a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord containing UniqueStringArray object
                             to open
              rOffset - Offset to the UniqueStringArray object in SmiRecord
              typeInfo - TypeInfo of UniqueStringArray object to open
              rValue - reference to a Word referencing the opened
                       UniqueStringArray object
  return value: true, if UniqueStringArray object was successfully opened,
                otherwise false
  exceptions: -

  */

  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method Out writes out an existing UniqueStringArray object
  in the form of a ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of UniqueStringArray object to write out
              value - reference to a Word referencing
                      the UniqueStringArray object
  return value: ListExpr of UniqueStringArray object referenced by value
  exceptions: -

  */

  static ListExpr Out(ListExpr typeInfo,
                      Word value);

  /*
  Method Property returns all properties of UniqueStringArray datatype.

  author: Dirk Zacher
  parameters: -
  return value: properties of UniqueStringArray datatype
                in the form of a ListExpr
  exceptions: -

  */

  static ListExpr Property();

  /*
  Method Save saves an existing UniqueStringArray object in a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord to save existing UniqueStringArray object
              rOffset - Offset to save position of UniqueStringArray object
                        in SmiRecord
              typeInfo - TypeInfo of UniqueStringArray object to save
              rValue - reference to a Word referencing
                       the UniqueStringArray object to save
  return value: true, if UniqueStringArray object was successfully saved,
                otherwise false
  exceptions: -

  */

  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method SizeOfObj returns the size of a UniqueStringArray object.

  author: Dirk Zacher
  parameters: -
  return value: size of a UniqueStringArray object
  exceptions: -

  */

  static int SizeOfObj();

  private:

  /*
  Member m_StringData contains string informations of all unique strings.

  */
  
  DbArray<StringData> m_StringData;

  /*
  Member m_StringFlob contains the Flob to store all unique strings.

  */

  Flob m_StringFlob;
};

}

#endif // TILEALGEBRA_UNIQUESTRINGARRAY_H
