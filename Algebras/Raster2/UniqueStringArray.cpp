 
/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

#include "Defines.h"
#include "UniqueStringArray.h"
#include "WinUnix.h"
#include "TypeConstructor.h"
#include "Symbols.h"

namespace raster2
{

UniqueStringArray::UniqueStringArray()
                  :Attribute()
{
  
}
  
UniqueStringArray::UniqueStringArray(bool bDefined)
                  :Attribute(bDefined),
                   m_StringData(0),
                   m_StringFlob(0)
{
  
}

UniqueStringArray::UniqueStringArray
                  (const UniqueStringArray& rUniqueStringArray)
                  :Attribute(rUniqueStringArray.IsDefined()),
                   m_StringData(rUniqueStringArray.m_StringData),
                   m_StringFlob(rUniqueStringArray.m_StringFlob)
{
  
}

UniqueStringArray::~UniqueStringArray()
{

}

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

bool UniqueStringArray::GetUniqueString(int nIndex, std::string& rString) const
{
  bool bRetVal = false;
  
  int nStrings = m_StringData.Size();
  
  if(nIndex < nStrings)
  {
    StringData stringData;
    bRetVal = m_StringData.Get(nIndex, stringData);
    
    if(bRetVal == true)
    {
      SmiSize bufferSize = stringData.Length + 1;
      char* pBuffer = new char[bufferSize];
      
      if(pBuffer != 0)
      {
        memset(pBuffer, 0, bufferSize * sizeof(char));
        bRetVal = m_StringFlob.read(pBuffer, stringData.Length,
                                    stringData.Offset);
        
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

list<string> UniqueStringArray::GetUniqueStringArray() const
{
  list<string> uniqueStringArray;
  
  int nStrings = m_StringData.Size();
  bool bOK = false;
  string uniqueString;
  
  for(int i = 0; i < nStrings; i++)
  {
    bOK = GetUniqueString(i, uniqueString);
    
    if(bOK == true)
    {
      uniqueStringArray.push_back(uniqueString);
    }
  }
  
  return uniqueStringArray;
}

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
        if(stringLength == stringData.Length)
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

int UniqueStringArray::AddString(const std::string& rString)
{ 
  int nIndex = UNDEFINED_STRING_INDEX;
  
  if(rString.empty() == false)
  {
    if(IsUniqueString(rString) == true)
    {
      StringData stringData;
      stringData.Offset = m_StringFlob.getSize();
      stringData.Length = rString.length();
      SmiSize newStringFlobSize = stringData.Offset + stringData.Length;
      
      bool bOK = m_StringFlob.resize(newStringFlobSize);
      
      if(bOK == true)
      {
        m_StringData.Append(stringData);
        m_StringFlob.write(rString.c_str(), stringData.Length,
                           stringData.Offset);
        
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

void UniqueStringArray::Destroy()
{
  m_StringData.Destroy();
  m_StringFlob.destroy();
}

bool UniqueStringArray::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

Attribute* UniqueStringArray::Clone() const
{
  Attribute* pAttribute = new UniqueStringArray(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

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

size_t UniqueStringArray::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = reinterpret_cast<size_t>(this);
  }
  
  return hashValue;
}

int UniqueStringArray::NumOfFLOBs() const
{ 
  return 2;
}

size_t UniqueStringArray::Sizeof() const
{
  return sizeof(UniqueStringArray);
}

const string UniqueStringArray::BasicType()
{
  return "uniquestringarray";
}

void* UniqueStringArray::Cast(void* pVoid)
{
  return new(pVoid)UniqueStringArray;
}

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

Word UniqueStringArray::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new UniqueStringArray(true);
  assert(word.addr != 0);

  return word;
}

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

TypeConstructor UniqueStringArray::getTypeConstructor()
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
        list<string> uniqueStringArray =
                     pUniqueStringArray->GetUniqueStringArray();
        
        if(uniqueStringArray.size() > 0)
        {
          list<string>::iterator it = uniqueStringArray.begin();
          ListExpr pCurrentListExpr = nl->StringAtom(*it);
          
          if(pCurrentListExpr != 0)
          {
            pListExpr = nl->OneElemList(pCurrentListExpr);
            ListExpr pLastListExpr = pListExpr;
            
            for(it++; it != uniqueStringArray.end(); it++)
            {
              pCurrentListExpr = nl->StringAtom(*it);
              
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

int UniqueStringArray::SizeOfObj()
{
  return sizeof(UniqueStringArray);
}

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
        if(stringLength == stringData.Length)
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

}
