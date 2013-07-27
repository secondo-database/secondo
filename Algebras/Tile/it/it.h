/*
----
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
----

*/

#ifndef TILEALGEBRA_IT_H
#define TILEALGEBRA_IT_H

/*
SECONDO includes

*/

#include "DateTime.h"
#include "TypeConstructor.h"

/*
TileAlgebra includes

*/

#include "itProperties.h"
#include "../t/t.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Template class it represents the base implementation
for datatypes itint, itreal, itbool and itstring.

author: Dirk Zacher

*/

template <typename Type, typename Properties = itProperties<Type> >
class it : public Properties::tType
{
  private:

  /*
  Constructor it does not initialize any members and
  should only be used in conjunction with Cast method.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  it();

  public:

  /*
  Constructor it sets defined flag of base class Attribute and
  initializes all members of the class with default values.

  author: Dirk Zacher
  parameters: bDefined - defined flag of base class Attribute
  return value: -
  exceptions: -

  */

  it(bool bDefined);

  /*
  Constructor it sets defined flag of base class Attribute to true and
  initializes all members of the class with corresponding parameter values.

  author: Dirk Zacher
  parameters: rInstant - reference to an Instant object
              rt - reference to a Properties::tType object
  return value: -
  exceptions: -

  */

  it(const Instant& rInstant,
     const typename Properties::tType& rt);

  /*
  Constructor it sets defined flag of base class Attribute to defined flag
  of rit object and initializes all members of the class with corresponding
  values of rit object.

  author: Dirk Zacher
  parameters: rit - reference to an it object
  return value: -
  exceptions: -

  */

  it(const it& rit);

  /*
  Destructor ~it deinitializes an it object.

  author: Dirk Zacher
  parameters: -
  return value: -
  exceptions: -

  */

  virtual ~it();

  /*
  Operator= assigns all member values of a given it object
  to the corresponding member values of this object.

  author: Dirk Zacher
  parameters: rit - reference to an it object
  return value: reference to this object
  exceptions: -

  */

  it& operator=(const it& rit);

  /*
  TileAlgebra operator inst returns the Instant value of an it object.

  author: Dirk Zacher
  parameters: rInstant - reference to an Instant object
  return value: -
  exceptions: -

  */

  void inst(Instant& rInstant);

  /*
  TileAlgebra operator val returns all values of an it object.

  author: Dirk Zacher
  parameters: rt - reference to a Properties::tType object
  return value: -
  exceptions: -

  */

  void val(typename Properties::tType& rt);

  /*
  Method SetInstant sets the Instant value of an it object.

  author: Dirk Zacher
  parameters: rInstant - reference to an Instant object
  return value: -
  exceptions: -

  */

  void SetInstant(const Instant& rInstant);

  /*
  Method SetValues sets all values of an it object.

  author: Dirk Zacher
  parameters: rt - reference to a Properties::tType object
  return value: -
  exceptions: -

  */

  void SetValues(const typename Properties::tType& rt);

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
  Method HashValue returns the hash value of the it object.

  author: Dirk Zacher
  parameters: -
  return value: hash value of the it object
  exceptions: -

  */

  virtual size_t HashValue() const;

  /*
  Method NumOfFLOBs returns the number of Flobs of an it object.

  author: Dirk Zacher
  parameters: -
  return value: number of Flobs of an it object
  exceptions: -

  */

  virtual int NumOfFLOBs() const;

  /*
  Method Sizeof returns the size of it datatype.

  author: Dirk Zacher
  parameters: -
  return value: size of it datatype
  exceptions: -

  */

  virtual size_t Sizeof() const;

  /*
  Method BasicType returns the typename of it datatype.

  author: Dirk Zacher
  parameters: -
  return value: typename of it datatype
  exceptions: -

  */

  static const std::string BasicType();

  /*
  Method Cast casts a void pointer to a new it object.

  author: Dirk Zacher
  parameters: pVoid - a pointer to a memory address
  return value: a pointer to a new it object
  exceptions: -

  */

  static void* Cast(void* pVoid);

  /*
  Method Clone clones an existing it object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing it object
  return value: a Word that references a new it object
  exceptions: -

  */

  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);

  /*
  Method Close closes an existing it object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing it object
  return value: -
  exceptions: -

  */

  static void Close(const ListExpr typeInfo,
                    Word& rWord);

  /*
  Method Create creates a new it object.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of the new it object to create
  return value: a Word that references a new it object
  exceptions: -

  */

  static Word Create(const ListExpr typeInfo);

  /*
  Method Delete deletes an existing it object given by a reference to a Word.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object referenced by rWord
              rWord - reference to the address of an existing it object
  return value: -
  exceptions: -

  */

  static void Delete(const ListExpr typeInfo,
                     Word& rWord);

  /*
  Method GetTypeConstructor returns the TypeConstructor of class it.

  author: Dirk Zacher
  parameters: -
  return value: TypeConstructor of class it
  exceptions: -

  */

  static TypeConstructor GetTypeConstructor();

  /*
  Method In creates a new it object on the basis of a given ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of object to create on the basis of instance
              instance - ListExpr of the it object to create
              errorPos - error position
              rErrorInfo - reference to error information
              rCorrect - flag that indicates if it object correctly created
  return value: a Word that references a new it object
  exceptions: -

  */

  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);

  /*
  Method KindCheck checks if given type is it type.

  author: Dirk Zacher
  parameters: type - ListExpr of type to check
              rErrorInfo - reference to error information
  return value: true, if type is it type, otherwise false
  exceptions: -

  */

  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);

  /*
  Method Open opens an it object from a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord containing it object to open
              rOffset - Offset to the it object in SmiRecord
              typeInfo - TypeInfo of it object to open
              rValue - reference to a Word referencing the opened it object
  return value: true, if it object was successfully opened, otherwise false
  exceptions: -

  */

  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method Out writes out an existing it object in the form of a ListExpr.

  author: Dirk Zacher
  parameters: typeInfo - TypeInfo of it object to write out
              value - reference to a Word referencing the it object
  return value: ListExpr of it object referenced by value
  exceptions: -

  */

  static ListExpr Out(ListExpr typeInfo,
                      Word value);

  /*
  Method Property returns all properties of it datatype.

  author: Dirk Zacher
  parameters: -
  return value: properties of it datatype in the form of a ListExpr
  exceptions: -

  */

  static ListExpr Property();

  /*
  Method Save saves an existing it object in a SmiRecord.

  author: Dirk Zacher
  parameters: rValueRecord - SmiRecord to save existing it object
              rOffset - Offset to save position of it object in SmiRecord
              typeInfo - TypeInfo of it object to save
              rValue - reference to a Word referencing the it object to save
  return value: true, if it object was successfully saved, otherwise false
  exceptions: -

  */

  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);

  /*
  Method SizeOfObj returns the size of an it object.

  author: Dirk Zacher
  parameters: -
  return value: size of an it object
  exceptions: -

  */

  static int SizeOfObj();

  private:

  /*
  Member m_Instant contains the Instant value of an it object.

  */

  Instant m_Instant;
};

/*
Constructor it does not initialize any members and
should only be used in conjunction with Cast method.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
it<Type, Properties>::it()
                     :Properties::tType(),
                      m_Instant()
{

}

/*
Constructor it sets defined flag of base class Attribute and
initializes all members of the class with default values.

author: Dirk Zacher
parameters: bDefined - defined flag of base class Attribute
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
it<Type, Properties>::it(bool bDefined)
                     :Properties::tType(bDefined),
                      m_Instant(0.0)
{
  
}

/*
Constructor it sets defined flag of base class Attribute to true and
initializes all members of the class with corresponding parameter values.

author: Dirk Zacher
parameters: rInstant - reference to an Instant object
            rt - reference to a Properties::tType object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
it<Type, Properties>::it(const Instant& rInstant,
                         const typename Properties::tType& rt)
                     :Properties::tType(rt)
{
  m_Instant = rInstant;
}

/*
Constructor it sets defined flag of base class Attribute to defined flag
of rit object and initializes all members of the class with corresponding
values of rit object.

author: Dirk Zacher
parameters: rit - reference to an it object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
it<Type, Properties>::it(const it<Type, Properties>& rit)
                     :Properties::tType(rit),
                      m_Instant(rit.m_Instant)
{
  
}

/*
Destructor deinitializes an it object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
it<Type, Properties>::~it()
{

}

/*
Operator= assigns all member values of a given it object
to the corresponding member values of this object.

author: Dirk Zacher
parameters: rit - reference to an it object
return value: reference to this object
exceptions: -

*/

template <typename Type, typename Properties>
it<Type, Properties>& it<Type, Properties>::operator=
                                            (const it<Type, Properties>& rit)
{
  if(this != &rit)
  {
    Properties::tType::operator=(rit);
    m_Instant = rit.m_Instant; 
  }

  return *this;
}

/*
TileAlgebra operator inst returns the Instant value of an it object.

author: Dirk Zacher
parameters: rInstant - reference to an Instant object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void it<Type, Properties>::inst(Instant& rInstant)
{
  rInstant = m_Instant;
}

/*
TileAlgebra operator val returns all values of an it object.

author: Dirk Zacher
parameters: rt - reference to a Properties::tType object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void it<Type, Properties>::val(typename Properties::tType& rt)
{
  rt = *this;
}

/*
Method SetInstant sets the Instant value of an it object.

author: Dirk Zacher
parameters: rInstant - reference to an Instant object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void it<Type, Properties>::SetInstant(const Instant& rInstant)
{
  m_Instant = rInstant;
}

/*
Method SetValues sets all values of an it object.

author: Dirk Zacher
parameters: rt - reference to a Properties::tType object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void it<Type, Properties>::SetValues(const typename Properties::tType& rt)
{
  Properties::tType::operator=(rt);
}

/*
Method Adjacent checks if this object is adjacent to given Attribute object.

author: Dirk Zacher
parameters: pAttribute - a pointer to an Attribute object
return value: true, if this object is adjacent to pAttribute, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool it<Type, Properties>::Adjacent(const Attribute* pAttribute) const
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
Attribute* it<Type, Properties>::Clone() const
{
  Attribute* pAttribute = new it<Type, Properties>(*this);
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
int it<Type, Properties>::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;

  if(pAttribute != 0)
  {
    const it<Type, Properties>* pit = dynamic_cast<const it<Type, Properties>*>
                                      (pAttribute);

    if(pit != 0)
    {
      bool bIsDefined = Properties::tType::IsDefined();
      bool bitIsDefined = pit->IsDefined();

      if(bIsDefined == true)
      {
        if(bitIsDefined == true) // defined x defined
        {
          nRetVal = Properties::tType::Compare(pAttribute);

          if(nRetVal == 0)
          {
            nRetVal = m_Instant.Compare(&(pit->m_Instant));
          }
        }

        else // defined x undefined
        {
          nRetVal = 1;
        }
      }

      else
      {
        if(bitIsDefined == true) // undefined x defined
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
void it<Type, Properties>::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const it<Type, Properties>* pit = dynamic_cast<const it<Type, Properties>*>
                                      (pAttribute);

    if(pit != 0)
    {
      *this = *pit;
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
Flob* it<Type, Properties>::GetFLOB(const int i)
{ 
  return Properties::tType::GetFLOB(i);
}

/*
Method HashValue returns the hash value of the it object.

author: Dirk Zacher
parameters: -
return value: hash value of the it object
exceptions: -

*/

template <typename Type, typename Properties>
size_t it<Type, Properties>::HashValue() const
{
  size_t hashValue = 0;

  if(Properties::tType::IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }

  return hashValue;
}

/*
Method NumOfFLOBs returns the number of Flobs of an it object.

author: Dirk Zacher
parameters: -
return value: number of Flobs of an it object
exceptions: -

*/

template <typename Type, typename Properties>
int it<Type, Properties>::NumOfFLOBs() const
{ 
  return Properties::tType::NumOfFLOBs();
}

/*
Method Sizeof returns the size of it datatype.

author: Dirk Zacher
parameters: -
return value: size of it datatype
exceptions: -

*/

template <typename Type, typename Properties>
size_t it<Type, Properties>::Sizeof() const
{
  return sizeof(it<Type, Properties>);
}

/*
Method BasicType returns the typename of it datatype.

author: Dirk Zacher
parameters: -
return value: typename of it datatype
exceptions: -

*/

template <typename Type, typename Properties>
const std::string it<Type, Properties>::BasicType()
{
  return Properties::GetTypeName();
}

/*
Method Cast casts a void pointer to a new it object.

author: Dirk Zacher
parameters: pVoid - a pointer to a memory address
return value: a pointer to a new it object
exceptions: -

*/

template <typename Type, typename Properties>
void* it<Type, Properties>::Cast(void* pVoid)
{
  return new(pVoid)it<Type, Properties>;
}

/*
Method Clone clones an existing it object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing it object
return value: a Word that references a new it object
exceptions: -

*/

template <typename Type, typename Properties>
Word it<Type, Properties>::Clone(const ListExpr typeInfo,
                                 const Word& rWord)
{
  Word word;

  it<Type, Properties>* pit = static_cast<it<Type, Properties>*>(rWord.addr);

  if(pit != 0)
  {
    word.addr = new it<Type, Properties>(*pit);
    assert(word.addr != 0);
  }

  return word;
}

/*
Method Close closes an existing it object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing it object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void it<Type, Properties>::Close(const ListExpr typeInfo,
                                 Word& rWord)
{
  it<Type, Properties>* pit = static_cast<it<Type, Properties>*>(rWord.addr);

  if(pit != 0)
  {
    delete pit;
    rWord.addr = 0;
  }
}

/*
Method Create creates a new it object.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of the new it object to create
return value: a Word that references a new it object
exceptions: -

*/

template <typename Type, typename Properties>
Word it<Type, Properties>::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new it<Type, Properties>(true);
  assert(word.addr != 0);

  return word;
}

/*
Method Delete deletes an existing it object given by a reference to a Word.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object referenced by rWord
            rWord - reference to the address of an existing it object
return value: -
exceptions: -

*/

template <typename Type, typename Properties>
void it<Type, Properties>::Delete(const ListExpr typeInfo,
                                  Word& rWord)
{
  it<Type, Properties>* pit = static_cast<it<Type, Properties>*>(rWord.addr);

  if(pit != 0)
  {
    delete pit;
    rWord.addr = 0;
  }
}

/*
Method GetTypeConstructor returns the TypeConstructor of class it.

author: Dirk Zacher
parameters: -
return value: TypeConstructor of class it
exceptions: -

*/

template <typename Type, typename Properties>
TypeConstructor it<Type, Properties>::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    it<Type, Properties>::BasicType(), // type name function
    it<Type, Properties>::Property,    // property function describing signature
    it<Type, Properties>::Out,         // out function
    it<Type, Properties>::In,          // in function
    0,                                 // save to list function
    0,                                 // restore from list function
    it<Type, Properties>::Create,      // create function
    it<Type, Properties>::Delete,      // delete function
    it<Type, Properties>::Open,        // open function
    it<Type, Properties>::Save,        // save function
    it<Type, Properties>::Close,       // close function
    it<Type, Properties>::Clone,       // clone function
    it<Type, Properties>::Cast,        // cast function
    it<Type, Properties>::SizeOfObj,   // sizeofobj function
    it<Type, Properties>::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());

  return typeConstructor;
}

/*
Method In creates a new it object on the basis of a given ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of object to create on the basis of instance
            instance - ListExpr of the it object to create
            errorPos - error position
            rErrorInfo - reference to error information
            rCorrect - flag that indicates if it object correctly created
return value: a Word that references a new it object
exceptions: -

*/

template <typename Type, typename Properties>
Word it<Type, Properties>::In(const ListExpr typeInfo,
                              const ListExpr instance,
                              const int errorPos,
                              ListExpr& rErrorInfo,
                              bool& rCorrect)
{
  Word word;

  rCorrect = false;

  if(listutils::isSymbolUndefined(instance) == false)
  {
    NList instanceList(instance);

    if(instanceList.length() == 2)
    {
      Instant instant(datetime::instanttype);
      bool bOK = instant.ReadFrom(instanceList.elem(1).listExpr(), true);

      if(bOK == true)
      {
        if(instant.IsDefined() == true)
        {
          Word tword = Properties::tType::In(0, instanceList.elem(2).listExpr(),
                                             0, rErrorInfo, bOK);

          if(tword.addr != 0 &&
             bOK == true)
          {
            typename Properties::tType* pt = static_cast
                                             <typename Properties::tType*>
                                             (tword.addr);

            if(pt != 0)
            {
              it<Type, Properties>* pit = new it<Type, Properties>
                                          (instant, *pt);

              if(pit != 0)
              {
                rCorrect = true;
                word.addr = pit;
              }

              delete pt;
              pt = 0;
            }
          }

          else
          {
            cmsg.inFunError("Cannot parse " +
                            tProperties<Type>::GetTypeName() + ".");
          }
        }

        else
        {
          cmsg.inFunError("Instant cannot be undefined.");
        }
      }

      else
      {
        cmsg.inFunError("Cannot parse instant.");
      }

    }

    else
    {
      cmsg.inFunError("Expected (instant " +
                      tProperties<Type>::GetTypeName() + ").");
    }
  }

  else
  {
    it<Type, Properties>* pit = new it<Type, Properties>(false);

    if(pit != 0)
    {
      rCorrect = true;
      word.addr = pit;
    }
  }

  return word;
}

/*
Method KindCheck checks if given type is it type.

author: Dirk Zacher
parameters: type - ListExpr of type to check
            rErrorInfo - reference to error information
return value: true, if type is it type, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool it<Type, Properties>::KindCheck(ListExpr type,
                                     ListExpr& rErrorInfo)
{
  bool bRetVal = false;

  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, it<Type, Properties>::BasicType());
  }

  return bRetVal;
}

/*
Method Open opens an it object from a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord containing it object to open
            rOffset - Offset to the it object in SmiRecord
            typeInfo - TypeInfo of it object to open
            rValue - reference to a Word referencing the opened it object
return value: true, if it object was successfully opened, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool it<Type, Properties>::Open(SmiRecord& rValueRecord,
                                size_t& rOffset,
                                const ListExpr typeInfo,
                                Word& rValue)
{
  bool bRetVal = OpenAttribute<it<Type, Properties> >(rValueRecord,
                                                      rOffset,
                                                      typeInfo,
                                                      rValue);

  return bRetVal;
}

/*
Method Out writes out an existing it object in the form of a ListExpr.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of it object to write out
            value - reference to a Word referencing the it object
return value: ListExpr of it object referenced by value
exceptions: -

*/

template <typename Type, typename Properties>
ListExpr it<Type, Properties>::Out(ListExpr typeInfo,
                                   Word value)
{
  ListExpr pListExpr = 0;

  it<Type, Properties>* pit = static_cast<it<Type, Properties>*>(value.addr);

  if(pit != 0)
  {
    if(pit->IsDefined())
    {
      NList instantList;

      if(pit->m_Instant.IsDefined())
      {
        instantList = NList(pit->m_Instant.ToListExpr(true));
      }

      else
      {
        instantList = NList(NList(Instant::BasicType()),
                            NList(Symbol::UNDEFINED()));
      }

      NList tList;
      Word word(pit);

      tList = NList(Properties::tType::Out(0, word));

      pListExpr = NList(instantList, tList).listExpr();
    }

    else
    {
      pListExpr = nl->SymbolAtom(Symbol::UNDEFINED());
    }
  }

  return pListExpr;
}

/*
Method Property returns all properties of it datatype.

author: Dirk Zacher
parameters: -
return value: properties of it datatype in the form of a ListExpr
exceptions: -

*/

template <typename Type, typename Properties>
ListExpr it<Type, Properties>::Property()
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
  values.append(NList(std::string("(instant sType)"), true));
  values.append(NList(std::string("(instant1 sstring1)"), true));
  values.append(NList(std::string(""), true));

  propertyList = NList(names, values);

  return propertyList.listExpr();
}

/*
Method Save saves an existing it object in a SmiRecord.

author: Dirk Zacher
parameters: rValueRecord - SmiRecord to save existing it object
            rOffset - Offset to save position of it object in SmiRecord
            typeInfo - TypeInfo of it object to save
            rValue - reference to a Word referencing the it object to save
return value: true, if it object was successfully saved, otherwise false
exceptions: -

*/

template <typename Type, typename Properties>
bool it<Type, Properties>::Save(SmiRecord& rValueRecord,
                                size_t& rOffset,
                                const ListExpr typeInfo,
                                Word& rValue)
{
  bool bRetVal = SaveAttribute<it<Type, Properties> >(rValueRecord,
                                                      rOffset,
                                                      typeInfo,
                                                      rValue);

  return bRetVal;
}

/*
Method SizeOfObj returns the size of an it object.

author: Dirk Zacher
parameters: -
return value: size of an it object
exceptions: -

*/

template <typename Type, typename Properties>
int it<Type, Properties>::SizeOfObj()
{
  return sizeof(it<Type, Properties>);
}

}

#endif // TILEALGEBRA_IT_H
