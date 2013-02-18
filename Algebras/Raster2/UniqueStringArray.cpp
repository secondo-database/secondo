 
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

UniqueStringArray& UniqueStringArray::operator =
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

bool UniqueStringArray::GetUniqueString(int nIndex, string& rString) const
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

int UniqueStringArray::GetUniqueStringIndex(const string& rString) const
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

int UniqueStringArray::AddString(const string& rString)
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

bool UniqueStringArray::Adjacent(const Attribute* attrib) const
{
  return false;
}

Attribute* UniqueStringArray::Clone() const
{
  Attribute* pAttribute = new UniqueStringArray(*this);
  return pAttribute;
}

int UniqueStringArray::Compare(const Attribute* rhs) const
{
  int nRetVal = -1;
  
  if(rhs != 0)
  {
    const UniqueStringArray* pUniqueStringArray =
                             dynamic_cast<const UniqueStringArray*>(rhs);
    
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

void UniqueStringArray::CopyFrom(const Attribute* right)
{
  if(right != 0)
  {
    const UniqueStringArray* pUniqueStringArray =
                             dynamic_cast<const UniqueStringArray*>(right);
    
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

Attribute::StorageType UniqueStringArray::GetStorageType() const
{
  return Default;
}

size_t UniqueStringArray::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = (size_t)&m_StringData + (size_t)&m_StringFlob;
  }
  
  return hashValue;
}

int UniqueStringArray::NumOfFLOBs() const
{ 
  return 2;
}

ostream& UniqueStringArray::Print(ostream& os) const
{
  os << "UniqueStringArray: ";
  
  if(IsDefined())
  {
    os << "DEFINED, size of m_StringData = " << m_StringData.getSize()
       << ", size of m_StringFlob = " << m_StringFlob.getSize() << endl;
  }
  
  else
  {
    os << "UNDEFINED." << endl;
  }
  
  return os;
}

size_t UniqueStringArray::Sizeof() const
{
  return sizeof(UniqueStringArray);
}

const string UniqueStringArray::BasicType()
{
  return "uniquestringarray";
}

const bool UniqueStringArray::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

void* UniqueStringArray::Cast(void* pVoid)
{
  return new (pVoid) UniqueStringArray;
}

Word UniqueStringArray::Clone(const ListExpr typeInfo,
                              const Word& w)
{
  Word word = SetWord(Address(0));
  
  UniqueStringArray* pUniqueStringArray =
                     static_cast<UniqueStringArray*>(w.addr);
                     
  if(pUniqueStringArray != 0)
  {
    word = SetWord(new UniqueStringArray(*pUniqueStringArray));
  }
  
  return word;
}

void UniqueStringArray::Close(const ListExpr typeInfo,
                              Word& w)
{
  UniqueStringArray* pUniqueStringArray =
                     static_cast<UniqueStringArray*>(w.addr);
  
  if(pUniqueStringArray != 0)
  {
    delete pUniqueStringArray;
    w = SetWord(Address(0));
  }
}

Word UniqueStringArray::Create(const ListExpr typeInfo)
{
  Word w = SetWord(new UniqueStringArray(true));
  return w;
}

void UniqueStringArray::Delete(const ListExpr typeInfo,
                               Word& w)
{
  UniqueStringArray* pUniqueStringArray =
                     static_cast<UniqueStringArray*>(w.addr);
  
  if(pUniqueStringArray != 0)
  {
    delete pUniqueStringArray;
    w = SetWord(Address(0));
  }
}

TypeConstructor UniqueStringArray::getTypeConstructor()
{
  TypeConstructor typeConstructorUniqueStringArray(
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
    UniqueStringArray::KindCheck);  // kindcheck function

  typeConstructorUniqueStringArray.AssociateKind(Kind::DATA());
  
  return typeConstructorUniqueStringArray;
}

Word UniqueStringArray::In(const ListExpr typeInfo,
                           const ListExpr instance,
                           const int errorPos,
                           ListExpr& errorInfo,
                           bool& correct)
{ 
  Word w = SetWord(Address(0));
  
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
        correct = true;
        w = SetWord(new UniqueStringArray(false));
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
          
          correct = true;
          w = SetWord(pUniqueStringArray);
        }
      }
    }
    
    else
    {
      correct = true;
      w = SetWord(new UniqueStringArray(true));
    }
  }
  
  return w;
}

bool UniqueStringArray::KindCheck(ListExpr type,
                                  ListExpr& errorInfo)
{
  bool bRetVal = false;
  
  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, UniqueStringArray::BasicType());
  }
  
  return bRetVal;
}

bool UniqueStringArray::Open(SmiRecord& valueRecord,
                             size_t& offset,
                             const ListExpr typeInfo,
                             Word& value)
{ 
  bool bRetVal = false;
  
  UniqueStringArray* pUniqueStringArray =
                     static_cast<UniqueStringArray*>
                     (Attribute::Open(valueRecord, offset, typeInfo));
  
  if(pUniqueStringArray != 0)
  { 
    /*
    list<string> uniqueStringArray = pUniqueStringArray->GetUniqueStringArray();
        
    if(uniqueStringArray.size() > 0)
    {
      for(list<string>::iterator it = uniqueStringArray.begin();
          it != uniqueStringArray.end(); it++)
      {
        cout << "uniqueStringArray: " << *it << endl;
      }
    }
    
    */
    
    value = SetWord(pUniqueStringArray);
    bRetVal = true;
  }
  
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
  ListExpr pListExpr = 0;
  
  if(nl != 0)
  {
    pListExpr = nl->TwoElemList(
                  nl->FiveElemList(
                    nl->StringAtom("Signature"),
                    nl->StringAtom("Example Type List"),
                    nl->StringAtom("List Rep"),
                    nl->StringAtom("Example List"),
                    nl->StringAtom("Remarks")),
                  nl->FiveElemList(
                    nl->StringAtom("-> DATA"),
                    nl->StringAtom(UniqueStringArray::BasicType()),
                    nl->StringAtom("(\"String1\" \"String2\" \"...\")"),
                    nl->StringAtom("(\"Raster\" \"Daten\")"),
                    nl->StringAtom(""))
                );
  }
  
  return pListExpr;
}

bool UniqueStringArray::Save(SmiRecord& valueRecord,
                             size_t& offset,
                             const ListExpr typeInfo,
                             Word& value)
{ 
  bool bRetVal = false;
  
  UniqueStringArray* pUniqueStringArray =
                     static_cast<UniqueStringArray*>(value.addr);
  
  if(pUniqueStringArray != 0)
  {
    Attribute::Save(valueRecord, offset, typeInfo, pUniqueStringArray);
    bRetVal = true;
  }
  
  return bRetVal;
}

int UniqueStringArray::SizeOfObj()
{
  return sizeof(UniqueStringArray);
}

void UniqueStringArray::Destroy()
{
  m_StringData.Destroy();
  m_StringFlob.destroy();
}

bool UniqueStringArray::IsUniqueString(const string& rString) const
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
