 
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

#include "tintFlob.h"
#include "TypeConstructor.h"
#include "Symbols.h"

namespace TileAlgebra
{

tintFlob::tintFlob()
         :Attribute()
{
  
}
  
tintFlob::tintFlob(bool bDefined)
         :Attribute(bDefined),
          m_Flob(0)
{
  
}

tintFlob::tintFlob(const tintFlob& rtintFlob)
                  :Attribute(rtintFlob.IsDefined()),
                   m_Flob(rtintFlob.m_Flob)
{
  
}

tintFlob::~tintFlob()
{
  
}

tintFlob& tintFlob::operator=(const tintFlob& rtintFlob)
{
  if(this != &rtintFlob)
  {
    bool bOK = false;

    bOK = m_Flob.clean();
    assert(bOK);
    bOK = m_Flob.copyFrom(rtintFlob.m_Flob);
    assert(bOK);
  }
  
  return *this;
}

void tintFlob::Destroy()
{
  m_Flob.destroy();
}

bool tintFlob::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

Attribute* tintFlob::Clone() const
{
  Attribute* pAttribute = new tintFlob(*this);
  assert(pAttribute != NULL);

  return pAttribute;
}

int tintFlob::Compare(const Attribute* pAttribute) const
{
  int nRetVal = -1;
  
  if(pAttribute != 0)
  {
    const tintFlob* ptintFlob = dynamic_cast<const tintFlob*>(pAttribute);
    
    if(ptintFlob != 0)
    {
      bool bIsDefined = IsDefined();
      bool btintFlobIsDefined = ptintFlob->IsDefined();
      
      if(bIsDefined == true)
      {
        if(btintFlobIsDefined == true) // defined x defined
        {
          if(m_Flob == ptintFlob->m_Flob)
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
        if(btintFlobIsDefined == true) // undefined x defined
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

void tintFlob::CopyFrom(const Attribute* pAttribute)
{
  if(pAttribute != 0)
  {
    const tintFlob* ptintFlob = dynamic_cast<const tintFlob*>(pAttribute);
    
    if(ptintFlob != 0)
    {
      *this = *ptintFlob;
    }
  }
}

Flob* tintFlob::GetFLOB(const int i)
{ 
  Flob* pFlob = 0;
  int nFlobs = NumOfFLOBs();
  
  if(i >= 0 &&
     i < nFlobs)
  {
    switch(i)
    {
      case 0:   pFlob = &m_Flob;
                break;
                
      default:  break;
    }
  }
  
  return pFlob;
}

size_t tintFlob::HashValue() const
{
  size_t hashValue = 0;
  
  if(IsDefined())
  {
    hashValue = (size_t)&m_Flob;
  }
  
  return hashValue;
}

int tintFlob::NumOfFLOBs() const
{ 
  return 1;
}

size_t tintFlob::Sizeof() const
{
  return sizeof(tintFlob);
}

const std::string tintFlob::BasicType()
{
  return "tintFlob";
}

void* tintFlob::Cast(void* pVoid)
{
  return new(pVoid)tintFlob;
}

Word tintFlob::Clone(const ListExpr typeInfo,
                     const Word& rWord)
{
  Word word;
  
  tintFlob* ptintFlob = static_cast<tintFlob*>(rWord.addr);
                     
  if(ptintFlob != 0)
  {
    word.addr = new tintFlob(*ptintFlob);
    assert(word.addr != 0);
  }
  
  return word;
}

void tintFlob::Close(const ListExpr typeInfo,
                           Word& rWord)
{
  tintFlob* ptintFlob = static_cast<tintFlob*>(rWord.addr);
  
  if(ptintFlob != 0)
  {
    delete ptintFlob;
    rWord.addr = 0;
  }
}

Word tintFlob::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new tintFlob(true);
  assert(word.addr != 0);

  return word;
}

void tintFlob::Delete(const ListExpr typeInfo,
                      Word& rWord)
{
  tintFlob* ptintFlob = static_cast<tintFlob*>(rWord.addr);
  
  if(ptintFlob != 0)
  {
    delete ptintFlob;
    rWord.addr = 0;
  }
}

TypeConstructor tintFlob::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    tintFlob::BasicType(), // type name function    
    tintFlob::Property,    // property function describing signature
    tintFlob::Out,         // out function
    tintFlob::In,          // in function
    0,                     // save to list function
    0,                     // restore from list function
    tintFlob::Create,      // create function
    tintFlob::Delete,      // delete function
    tintFlob::Open,        // open function
    tintFlob::Save,        // save function
    tintFlob::Close,       // close function
    tintFlob::Clone,       // clone function
    tintFlob::Cast,        // cast function
    tintFlob::SizeOfObj,   // sizeofobj function
    tintFlob::KindCheck    // kindcheck function
  );

  typeConstructor.AssociateKind(Kind::DATA());
  
  return typeConstructor;
}

Word tintFlob::In(const ListExpr typeInfo,
                  const ListExpr instance,
                  const int errorPos,
                  ListExpr& rErrorInfo,
                  bool& rCorrect)
{ 
  Word word;

  // TODO: add implementation
  
  return word;
}

bool tintFlob::KindCheck(ListExpr type,
                         ListExpr& rErrorInfo)
{
  bool bRetVal = false;
  
  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, tintFlob::BasicType());
  }
  
  return bRetVal;
}

bool tintFlob::Open(SmiRecord& rValueRecord,
                    size_t& rOffset,
                    const ListExpr typeInfo,
                    Word& rValue)
{ 
  bool bRetVal = false;
  
  tintFlob* ptintFlob = static_cast<tintFlob*>(Attribute::Open(rValueRecord,
                                                               rOffset,
                                                               typeInfo));
  
  if(ptintFlob != 0)
  { 
    rValue = SetWord(ptintFlob);
    bRetVal = true;
  }
  
  return bRetVal;
}

ListExpr tintFlob::Out(ListExpr typeInfo,
                       Word value)
{ 
  ListExpr pListExpr = 0;
  
  if(nl != 0)
  {  
    tintFlob* ptintFlob = static_cast<tintFlob*>(value.addr);
    
    if(ptintFlob != 0)
    {
      if(ptintFlob->IsDefined() == true)
      {
        
      }
      
      else
      {
        pListExpr = nl->SymbolAtom(Symbol::UNDEFINED());
      }
    }
  }
  
  return pListExpr;
}

ListExpr tintFlob::Property()
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
  values.append(NList
               (std::string("((x y l) (szx szy) ((ix iy (v*)))*)"),
                true));
  values.append(NList
               (std::string("((0.0 0.0 1.0) (2 2) ((-32 -32 (1 2 3 4))))"),
                true));
  values.append(NList(std::string(""), true));

  propertyList = NList(names, values);

  return propertyList.listExpr();
}

bool tintFlob::Save(SmiRecord& rValueRecord,
                    size_t& rOffset,
                    const ListExpr typeInfo,
                    Word& rValue)
{ 
  bool bRetVal = false;
  
  tintFlob* ptintFlob = static_cast<tintFlob*>(rValue.addr);
  
  if(ptintFlob != 0)
  {
    Attribute::Save(rValueRecord, rOffset, typeInfo, ptintFlob);
    bRetVal = true;
  }
  
  return bRetVal;
}

int tintFlob::SizeOfObj()
{
  return sizeof(tintFlob);
}

}
