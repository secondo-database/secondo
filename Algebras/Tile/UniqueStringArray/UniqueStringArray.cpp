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

/*
SECONDO includes

*/

#include "TypeConstructor.h"
#include "Symbols.h"

/*
TileAlgebra includes

*/

#include "UniqueStringArray.h"
#include "../Constants.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Constructor UniqueStringArray does not initialize any members and
should only be used in conjunction with Cast method.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

UniqueStringArray::UniqueStringArray()
                  :Attribute()
{
  
}

/*
Constructor UniqueStringArray sets defined flag
of base class Attribute and initializes all members
of the class with default values.

author: Dirk Zacher
parameters: bDefined - defined flag of base class Attribute
return value: -
exceptions: -

*/
  
UniqueStringArray::UniqueStringArray(bool bDefined)
                  :Attribute(bDefined),
                   m_StringData(0),
                   m_StringFlob(0)
{
  
}

/*
Constructor UniqueStringArray sets defined flag
of base class Attribute to defined flag of rUniqueStringArray object
and initializes all members of the class with corresponding values
of rUniqueStringArray object.

author: Dirk Zacher
parameters: rUniqueStringArray - reference to a UniqueStringArray object
return value: -
exceptions: -

*/

UniqueStringArray::UniqueStringArray
                  (const UniqueStringArray& rUniqueStringArray)
                  :Attribute(rUniqueStringArray.IsDefined()),
                   m_StringData(rUniqueStringArray.m_StringData.getSize()),
                   m_StringFlob(rUniqueStringArray.m_StringFlob.getSize())
{
  m_StringData.copyFrom(rUniqueStringArray.m_StringData);
  m_StringFlob.copyFrom(rUniqueStringArray.m_StringFlob);
}

/*
Destructor deinitializes a UniqueStringArray object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

UniqueStringArray::~UniqueStringArray()
{
  
}

/*
Operator= assigns all member values of a given UniqueStringArray object
to the corresponding member values of this object.

author: Dirk Zacher
parameters: rUniqueStringArray - reference to a UniqueStringArray object
return value: reference to this object
exceptions: -

*/

UniqueStringArray& UniqueStringArray::operator=
                   (const UniqueStringArray& rUniqueStringArray)
{
  if(this != &rUniqueStringArray)
  {
    bool bOK = false;
    
    bOK = m_StringData.clean();
    assert(bOK);
    bOK = m_StringData.Append(rUniqueStringArray.m_StringData);
    assert(bOK);
    bOK = m_StringFlob.clean();
    assert(bOK);
    bOK = m_StringFlob.copyFrom(rUniqueStringArray.m_StringFlob);
    assert(bOK);
  }
  
  return *this;
}

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

bool UniqueStringArray::GetUniqueString(int nIndex,
                                        std::string& rString) const
{
  bool bRetVal = false;
  
  int nStrings = m_StringData.Size();
  
  if(nIndex < nStrings)
  {
    StringData stringData;
    bRetVal = m_StringData.Get(nIndex, stringData);
    
    if(bRetVal == true)
    {
      SmiSize bufferSize = stringData.m_Length + 1;
      char* pBuffer = new char[bufferSize];
      
      if(pBuffer != 0)
      {
        memset(pBuffer, 0, bufferSize * sizeof(char));
        bRetVal = m_StringFlob.read(pBuffer, stringData.m_Length,
                                    stringData.m_Offset);
        
        if(bRetVal == true)
        {
          rString = pBuffer;
        }
        
        delete [] pBuffer;
      }
    }
  }
  
  return bRetVal;
}

/*
Method GetUniqueStringArray returns all unique strings.

author: Dirk Zacher
parameters: rUniqueStringArray - reference to a vector of strings
                                 containing all unique strings
return value: -
exceptions: -

*/

void UniqueStringArray::GetUniqueStringArray
                        (std::vector<std::string>& rUniqueStringArray) const
{
  rUniqueStringArray.clear();
  
  int nStrings = m_StringData.Size();
  bool bOK = false;
  std::string uniqueString;
  
  for(int i = 0; i < nStrings; i++)
  {
    bOK = GetUniqueString(i, uniqueString);
    
    if(bOK == true &&
       uniqueString.empty() == false)
    {
      rUniqueStringArray.push_back(uniqueString);
    }
  }
}

/*
Method GetUniqueStringIndex returns the index of a unique string.

author: Dirk Zacher
parameters: rString - reference to a string
return value: index of unique string if rString exists in UniqueStringArray,
              otherwise -1
exceptions: -

*/

int UniqueStringArray::GetUniqueStringIndex(const std::string& rString) const
{
  int nUniqueStringIndex = UNDEFINED_STRING_INDEX;
  
  if(rString.empty() == false)
  {
    SmiSize stringLength = rString.length();
    int nStrings = m_StringData.Size();
    bool bOK = false;
    StringData stringData;
    string uniqueString;
    
    for(int i = 0; i < nStrings; i++)
    {
      bOK = m_StringData.Get(i, stringData);
      
      if(bOK == true)
      {
        if(stringLength == stringData.m_Length)
        {
          bOK = GetUniqueString(i, uniqueString);
          
          if(bOK == true &&
             rString == uniqueString)
          {
            nUniqueStringIndex = i;
            break;
          }
        }
      }
    }
  }
  
  return nUniqueStringIndex;
}

/*
Method AddString adds given string to UniqueStringArray if given string
does not exist in UniqueStringArray and returns the index of given string.

author: Dirk Zacher
parameters: rString - reference to a string
return value: index of given string in UniqueStringArray
exceptions: -

*/

int UniqueStringArray::AddString(const std::string& rString)
{ 
  int nIndex = UNDEFINED_STRING_INDEX;
  
  if(rString.empty() == false)
  {
    if(IsUniqueString(rString) == true)
    {
      StringData stringData;
      stringData.m_Offset = m_StringFlob.getSize();
      stringData.m_Length = rString.length();
      SmiSize newStringFlobSize = stringData.m_Offset + stringData.m_Length;
      
      bool bOK = m_StringFlob.resize(newStringFlobSize);
      
      if(bOK == true)
      {
        m_StringData.Append(stringData);
        m_StringFlob.write(rString.c_str(), stringData.m_Length,
                           stringData.m_Offset);
        
        nIndex = m_StringData.Size() - 1;
      }
    }
    
    else
    {
      nIndex = GetUniqueStringIndex(rString);
    }
  }
  
  return nIndex;
}

/*
Method Destroy destroys UniqueStringArray object.

author: Dirk Zacher
parameters: -
return value: -
exceptions: -

*/

void UniqueStringArray::Destroy()
{
  m_StringData.Destroy();
  m_StringFlob.destroy();
}

/*
Method IsUniqueString checks if given string is an unique string.

author: Dirk Zacher
parameters: rString - reference to a string
return value: true, if given string is an unique string, otherwise false
exceptions: -

*/

bool UniqueStringArray::IsUniqueString(const std::string& rString) const
{
  bool bIsUniqueString = false;
  
  if(rString.empty() == false)
  {
    bIsUniqueString = true;
    SmiSize stringLength = rString.length();
    int nStrings = m_StringData.Size();
    bool bOK = false;
    StringData stringData;
    string uniqueString;
    
    for(int i = 0; i < nStrings; i++)
    {
      bOK = m_StringData.Get(i, stringData);
      
      if(bOK == true)
      {
        if(stringLength == stringData.m_Length)
        {
          bOK = GetUniqueString(i, uniqueString);
          
          if(bOK == true &&
             rString == uniqueString)
          {
            bIsUniqueString = false;
            break;
          }
        }
      }
    }
  }
  
  return bIsUniqueString;
}

/*
Method Adjacent checks if this object is adjacent to given Attribute object.

author: Dirk Zacher
parameters: pAttribute - a pointer to an Attribute object
return value: true, if this object is adjacent to pAttribute, otherwise false
exceptions: -

*/

bool UniqueStringArray::Adjacent(const Attribute* pAttribute) const
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

Attribute* UniqueStringArray::Clone() const
{
  Attribute* pAttribute = new UniqueStringArray(*this);
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

int UniqueStringArray::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const UniqueStringArray* pUniqueStringArray =
                             dynamic_cast<const UniqueStringArray*>(pAttribute);
    
    if(pUniqueStringArray != 0)
    {
      bool bIsDefined = IsDefined();
      bool bUniqueStringArrayIsDefined = pUniqueStringArray->IsDefined();
      
      if(bIsDefined == true)
      {
        if(bUniqueStringArrayIsDefined == true) // defined x defined
        {
          if(m_StringData == pUniqueStringArray->m_StringData &&
             m_StringFlob == pUniqueStringArray->m_StringFlob)
          {
            nRetVal = 0;
          }
        }
        
        else // defined x undefined
        {
          nRetVal = 1;
        }
      }
      
      else
      {
        if(bUniqueStringArrayIsDefined == true) // undefined x defined
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

void UniqueStringArray::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const UniqueStringArray* pUniqueStringArray =
                             dynamic_cast<const UniqueStringArray*>(pAttribute);
    
    if(pUniqueStringArray != 0)
    {
      *this = *pUniqueStringArray;
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

Flob* UniqueStringArray::GetFLOB(const int i)
{ 
  Flob* pFlob = 0;
  int nFlobs = NumOfFLOBs();
  
  if(i >= 0 &&
     i < nFlobs)
  {
    switch(i)
    {
      case 0:   pFlob = &m_StringData;
                break;
                
      case 1:   pFlob = &m_StringFlob;
                break;
                
      default:  break;
    }
  }
  
  return pFlob;
}

/*
Method HashValue returns the hash value of the UniqueStringArray object.

author: Dirk Zacher
parameters: -
return value: hash value of the UniqueStringArray object
exceptions: -

*/

size_t UniqueStringArray::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }
  
  return hashValue;
}

/*
Method NumOfFLOBs returns the number of Flobs of a UniqueStringArray object.

author: Dirk Zacher
parameters: -
return value: number of Flobs of a UniqueStringArray object
exceptions: -

*/

int UniqueStringArray::NumOfFLOBs() const
{ 
  return 2;
}

/*
Method Sizeof returns the size of UniqueStringArray datatype.

author: Dirk Zacher
parameters: -
return value: size of UniqueStringArray datatype
exceptions: -

*/

size_t UniqueStringArray::Sizeof() const
{
  return sizeof(UniqueStringArray);
}

/*
Method BasicType returns the typename of UniqueStringArray datatype.

author: Dirk Zacher
parameters: -
return value: typename of UniqueStringArray datatype
exceptions: -

*/

const string UniqueStringArray::BasicType()
{
  return TYPE_NAME_UNIQUESTRINGARRAY;
}

/*
Method Cast casts a void pointer to a new UniqueStringArray object.

author: Dirk Zacher
parameters: pVoid - a pointer to a memory address
return value: a pointer to a new UniqueStringArray object
exceptions: -

*/

void* UniqueStringArray::Cast(void* pVoid)
{
  return new(pVoid)UniqueStringArray;
}

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

Word UniqueStringArray::Clone(const ListExpr typeInfo,
                              const Word& rWord)
{
  Word word;
  
  UniqueStringArray* pUniqueStringArray =
                     static_cast<UniqueStringArray*>(rWord.addr);
                     
  if(pUniqueStringArray != 0)
  {
    word.addr = new UniqueStringArray(*pUniqueStringArray);
    assert(word.addr != 0);
  }
  
  return word;
}

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

void UniqueStringArray::Close(const ListExpr typeInfo,
                              Word& rWord)
{
  UniqueStringArray* pUniqueStringArray =
                     static_cast<UniqueStringArray*>(rWord.addr);
  
  if(pUniqueStringArray != 0)
  {
    delete pUniqueStringArray;
    rWord.addr = 0;
  }
}

/*
Method Create creates a new UniqueStringArray object.

author: Dirk Zacher
parameters: typeInfo - TypeInfo of the new UniqueStringArray object to create
return value: a Word that references a new UniqueStringArray object
exceptions: -

*/

Word UniqueStringArray::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new UniqueStringArray(true);
  assert(word.addr != 0);

  return word;
}

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

void UniqueStringArray::Delete(const ListExpr typeInfo,
                               Word& rWord)
{
  UniqueStringArray* pUniqueStringArray =
                     static_cast<UniqueStringArray*>(rWord.addr);
  
  if(pUniqueStringArray != 0)
  {
    delete pUniqueStringArray;
    rWord.addr = 0;
  }
}

/*
Method GetTypeConstructor returns the TypeConstructor
of class UniqueStringArray.

author: Dirk Zacher
parameters: -
return value: TypeConstructor of class UniqueStringArray
exceptions: -

*/

TypeConstructor UniqueStringArray::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    UniqueStringArray::BasicType(), // type name function    
    UniqueStringArray::Property,    // property function describing signature
    UniqueStringArray::Out,         // out function
    UniqueStringArray::In,          // in function
    0,                              // save to list function
    0,                              // restore from list function
    UniqueStringArray::Create,      // create function
    UniqueStringArray::Delete,      // delete function
    UniqueStringArray::Open,        // open function
    UniqueStringArray::Save,        // save function
    UniqueStringArray::Close,       // close function
    UniqueStringArray::Clone,       // clone function
    UniqueStringArray::Cast,        // cast function
    UniqueStringArray::SizeOfObj,   // sizeofobj function
    UniqueStringArray::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());
  
  return typeConstructor;
}

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

Word UniqueStringArray::In(const ListExpr typeInfo,
                           const ListExpr instance,
                           const int errorPos,
                           ListExpr& rErrorInfo,
                           bool& rCorrect)
{ 
  Word word;
  rCorrect = false;
  
  if(nl != 0)
  {
    int nListLength = nl->ListLength(instance);
    
    if(nListLength > 0)
    {
      if(nListLength == 1 &&
         nl->IsAtom(nl->First(instance)) &&
         nl->AtomType(nl->First(instance)) == SymbolType &&
         listutils::isSymbolUndefined(nl->First(instance)))
      {
        rCorrect = true;
        word.addr = new UniqueStringArray(false);
      }
      
      else
      {
        UniqueStringArray* pUniqueStringArray = new UniqueStringArray(true);
        
        if(pUniqueStringArray != 0)
        {
          ListExpr first = nl->Empty();
          ListExpr rest = instance;
          
          while(nl->IsEmpty(rest) == false)
          {
            first = nl->First(rest);
            rest = nl->Rest(rest);
            
            if(nl->IsAtom(first) &&
               nl->AtomType(first) == StringType)
            {
              string stringValue = nl->StringValue(first);
              pUniqueStringArray->AddString(stringValue);
            }
          }
          
          rCorrect = true;
          word.addr = pUniqueStringArray;
        }
      }
    }
    
    else
    {
      rCorrect = true;
      word.addr = new UniqueStringArray(true);
    }
  }
  
  return word;
}

/*
Method KindCheck checks if given type is UniqueStringArray type.

author: Dirk Zacher
parameters: type - ListExpr of type to check
            rErrorInfo - reference to error information
return value: true, if type is UniqueStringArray type, otherwise false
exceptions: -

*/

bool UniqueStringArray::KindCheck(ListExpr type,
                                  ListExpr& rErrorInfo)
{
  bool bRetVal = false;
  
  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, UniqueStringArray::BasicType());
  }
  
  return bRetVal;
}

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

bool UniqueStringArray::Open(SmiRecord& rValueRecord,
                             size_t& rOffset,
                             const ListExpr typeInfo,
                             Word& rValue)
{ 
  bool bRetVal = OpenAttribute<UniqueStringArray>(rValueRecord,
                                                  rOffset,
                                                  typeInfo,
                                                  rValue);

  return bRetVal;
}

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

ListExpr UniqueStringArray::Out(ListExpr typeInfo,
                                Word value)
{ 
  ListExpr pListExpr = 0;
  
  if(nl != 0)
  {  
    UniqueStringArray* pUniqueStringArray =
                       static_cast<UniqueStringArray*>(value.addr);
    
    if(pUniqueStringArray != 0)
    {
      if(pUniqueStringArray->IsDefined() == true)
      {
        std::vector<std::string> uniqueStringArray;
        pUniqueStringArray->GetUniqueStringArray(uniqueStringArray);
        
        if(uniqueStringArray.size() > 0)
        {
          ListExpr pCurrentListExpr = nl->StringAtom(uniqueStringArray[0]);
          
          if(pCurrentListExpr != 0)
          {
            pListExpr = nl->OneElemList(pCurrentListExpr);
            ListExpr pLastListExpr = pListExpr;
            
            for(size_t i = 1; i < uniqueStringArray.size(); i++)
            {
              pCurrentListExpr = nl->StringAtom(uniqueStringArray[i]);
              
              if(pCurrentListExpr != 0)
              {
                pLastListExpr = nl->Append(pLastListExpr, pCurrentListExpr);
              }
            }
          }
        }
        
        else
        {
          pListExpr = nl->TheEmptyList();
        }
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
Method Property returns all properties of UniqueStringArray datatype.

author: Dirk Zacher
parameters: -
return value: properties of UniqueStringArray datatype
              in the form of a ListExpr
exceptions: -

*/

ListExpr UniqueStringArray::Property()
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
  values.append(NList(UniqueStringArray::BasicType(), true));
  values.append(NList
               (std::string("(\"String1\" \"String2\" \"...\")"),
                true));
  values.append(NList
               (std::string("(\"Raster\" \"Daten\")"),
                true));
  values.append(NList(std::string(""), true));

  propertyList = NList(names, values);

  return propertyList.listExpr();
}

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

bool UniqueStringArray::Save(SmiRecord& rValueRecord,
                             size_t& rOffset,
                             const ListExpr typeInfo,
                             Word& rValue)
{ 
  bool bRetVal = SaveAttribute<UniqueStringArray>(rValueRecord,
                                                  rOffset,
                                                  typeInfo,
                                                  rValue);

  return bRetVal;
}

/*
Method SizeOfObj returns the size of a UniqueStringArray object.

author: Dirk Zacher
parameters: -
return value: size of a UniqueStringArray object
exceptions: -

*/

int UniqueStringArray::SizeOfObj()
{
  return sizeof(UniqueStringArray);
}

}
