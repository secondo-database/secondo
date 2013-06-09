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

#include "../t/t.h"
#include "itProperties.h"
#include "DateTime.h"
#include "TypeConstructor.h"

namespace TileAlgebra
{

/*
declaration of template class it

*/

template <typename Type, typename Properties = itProperties<Type> >
class it : public Properties::tType
{
  /*
  constructors

  */

  private:

  it();

  public:

  it(bool bDefined);
  it(const datetime::DateTime& rInstant,
     const typename Properties::tType& rt);
  it(const it& rit);

  /*
  destructor

  */

  virtual ~it();

  /*
  operators

  */

  it& operator=(const it& rit);

  /*
  TileAlgebra operator methods

  */

  datetime::DateTime inst();
  typename Properties::tType val();

  /*
  override functions from base class Attribute

  */

  virtual bool Adjacent(const Attribute* pAttribute) const;
  virtual Attribute* Clone() const;
  virtual int Compare(const Attribute* pAttribute) const;
  virtual void CopyFrom(const Attribute* pAttribute);
  virtual Flob* GetFLOB(const int i);
  virtual size_t HashValue() const;
  virtual int NumOfFLOBs() const;
  virtual size_t Sizeof() const;

  /*
  The following functions are used to integrate the ~it~
  datatype into secondo.

  */

  static const std::string BasicType();
  static void* Cast(void* pVoid);
  static Word Clone(const ListExpr typeInfo,
                    const Word& rWord);
  static void Close(const ListExpr typeInfo,
                    Word& rWord);
  static Word Create(const ListExpr typeInfo);
  static void Delete(const ListExpr typeInfo,
                     Word& rWord);
  static TypeConstructor GetTypeConstructor();
  static Word In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& rErrorInfo,
                 bool& rCorrect);
  static bool KindCheck(ListExpr type,
                        ListExpr& rErrorInfo);
  static bool Open(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);
  static ListExpr Out(ListExpr typeInfo,
                      Word value);
  static ListExpr Property();
  static bool Save(SmiRecord& rValueRecord,
                   size_t& rOffset,
                   const ListExpr typeInfo,
                   Word& rValue);
  static int SizeOfObj();

  private:

  /*
  members

  */

  datetime::DateTime m_Instant;
};

/*
implementation of template class it

*/

template <typename Type, typename Properties>
it<Type, Properties>::it()
                     :Properties::tType(),
                      m_Instant()
{

}

template <typename Type, typename Properties>
it<Type, Properties>::it(bool bDefined)
                     :Properties::tType(bDefined),
                      m_Instant(0.0)
{
  
}

template <typename Type, typename Properties>
it<Type, Properties>::it(const datetime::DateTime& rInstant,
                         const typename Properties::tType& rt)
                     :Properties::tType(rt)
{
  m_Instant = rInstant;
}

template <typename Type, typename Properties>
it<Type, Properties>::it(const it<Type, Properties>& rit)
                     :Properties::tType(rit),
                      m_Instant(rit.m_Instant)
{
  
}

template <typename Type, typename Properties>
it<Type, Properties>::~it()
{

}

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

template <typename Type, typename Properties>
datetime::DateTime it<Type, Properties>::inst()
{
  return m_Instant;
}

template <typename Type, typename Properties>
typename Properties::tType it<Type, Properties>::val()
{
  return *this;
}

template <typename Type, typename Properties>
bool it<Type, Properties>::Adjacent(const Attribute* pAttribute) const
{
  return false;
}

template <typename Type, typename Properties>
Attribute* it<Type, Properties>::Clone() const
{
  Attribute* pAttribute = new it<Type, Properties>(*this);
  assert(pAttribute != 0);

  return pAttribute;
}

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

template <typename Type, typename Properties>
Flob* it<Type, Properties>::GetFLOB(const int i)
{ 
  return Properties::tType::GetFLOB(i);
}

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

template <typename Type, typename Properties>
int it<Type, Properties>::NumOfFLOBs() const
{ 
  return Properties::tType::NumOfFLOBs();
}

template <typename Type, typename Properties>
size_t it<Type, Properties>::Sizeof() const
{
  return sizeof(it<Type, Properties>);
}

template <typename Type, typename Properties>
const std::string it<Type, Properties>::BasicType()
{
  return Properties::GetTypeName();
}

template <typename Type, typename Properties>
void* it<Type, Properties>::Cast(void* pVoid)
{
  return new(pVoid)it<Type, Properties>;
}

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

template <typename Type, typename Properties>
Word it<Type, Properties>::Create(const ListExpr typeInfo)
{
  Word word;

  word.addr = new it<Type, Properties>(true);
  assert(word.addr != 0);

  return word;
}

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

template <typename Type, typename Properties>
TypeConstructor it<Type, Properties>::GetTypeConstructor()
{
  TypeConstructor typeConstructor
  (
    it<Type, Properties>::BasicType(), // type name function
    it<Type, Properties>::Property,    // property function describing signature
    it<Type, Properties>::Out,         // out function
    it<Type, Properties>::In,          // in function
    0,                                // save to list function
    0,                                // restore from list function
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
      datetime::DateTime instant(datetime::instanttype);
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

template <typename Type, typename Properties>
int it<Type, Properties>::SizeOfObj()
{
  return sizeof(it<Type, Properties>);
}

}

#endif // TILEALGEBRA_IT_H
