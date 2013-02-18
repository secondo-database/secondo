/*
----
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
----

*/

#ifndef RASTER2_ISTYPE_H
#define RASTER2_ISTYPE_H

#include "Defines.h"
#include "util/noncopyable.h"
#include "util/parse_error.h"
#include "NList.h"
#include "DateTime.h"
#include "TypeConstructor.h"

using namespace datetime;

namespace raster2
{
  template <class T>
  struct istype_helper
  {
    typedef T implementation_type;
    typedef T spatial_type;
    typedef T wrapper_type;
    static const char* name;
    static bool check(const NList& nl);
    static T parse(const NList& nl);
    static bool isUndefined(const T& t);
    static T getUndefined();
    static std::string BasicType();
  };

  template <typename T, typename Helper = istype_helper<T> >
  class istype : util::noncopyable
  {
    private:
    /*
    constructors
  
    */
    
    istype();
  
    public:
    /*
    constructors
  
    */

    istype(DateTime* pInstant, typename Helper::spatial_type* psT);
    
    /*
    destructor
    
    */
    
    virtual ~istype();
    
    /*
    operators
  
    */
    
    
    
    /*
    functions
    
    */

    void assumeData(istype& ristype);
    void setInstant(DateTime* instant);
    void setValues(typename Helper::spatial_type* values);
    const DateTime& inst() const;
    typename Helper::spatial_type* val() const;

    void setCacheSize(size_t);
    void flushCache();

    /*
    The following functions are used to integrate the ~istype~ datatype
    into secondo.

    */
    
    static const string BasicType();
    static const bool checkType(const ListExpr type);
    static void* Cast(void* pVoid);
    static Word Clone(const ListExpr typeInfo,
                      const Word& w);
    static void Close(const ListExpr typeInfo,
                      Word& w);
    static Word Create(const ListExpr typeInfo);
    static void Delete(const ListExpr typeInfo,
                      Word& w);
    static TypeConstructor getTypeConstructor();
    static Word In(const ListExpr typeInfo,
                  const ListExpr instance,
                  const int errorPos,
                  ListExpr& errorInfo,
                  bool& correct);
    static bool KindCheck(ListExpr type,
                          ListExpr& errorInfo);
    static bool Open(SmiRecord& valueRecord,
                    size_t& offset,
                    const ListExpr typeInfo,
                    Word& value);
    static ListExpr Out(ListExpr typeInfo,
                        Word value);
    static ListExpr Property();
    static bool Save(SmiRecord& valueRecord,
                    size_t& offset,
                    const ListExpr typeInfo,
                    Word& value);
    static int SizeOfObj();
  
    private:
    /*
    functions

    */
    
    void Delete();
    
    /*
    members
   
    */
    
    bool m_bDelete;
    DateTime* m_pInstant;
    typename Helper::spatial_type* m_psT;
  };

  template <typename T, typename Helper>
  istype<T, Helper>::istype()
                    :m_bDelete(true),
                     m_pInstant(0),
                     m_psT(0)
  {
    
  }

  template <typename T, typename Helper>
  istype<T, Helper>::istype(DateTime* pInstant,
                            typename Helper::spatial_type* psT)
                    :m_bDelete(true),
                     m_pInstant(pInstant),
                     m_psT(psT)
  {
    
  }

  template <typename T, typename Helper>
  istype<T, Helper>::~istype()
  {
    Delete();
  }

  template <typename T, typename Helper>
  void istype<T, Helper>::assumeData(istype<T, Helper>& ristype)
  { 
    if(this != &ristype)
    {
      Delete();
      
      m_bDelete = true;
      m_pInstant = ristype.m_pInstant;
      m_psT = ristype.m_psT;
      
      ristype.m_bDelete = false;
      ristype.m_pInstant = 0;
      ristype.m_psT = 0;
    }
  }

  template <typename T, typename Helper>
  void istype<T, Helper>::setInstant(DateTime* instant)
  {
    m_pInstant = instant;
  }
  
  template <typename T, typename Helper>
  void istype<T, Helper>::setValues(typename Helper::spatial_type* values)
  {
    m_psT = values;
  }
  
  template <typename T, typename Helper>
  const DateTime& istype<T, Helper>::inst() const
  {
    return *m_pInstant;
  }

  template <typename T, typename Helper>
  typename Helper::spatial_type* istype<T, Helper>::val() const
  {
    typename Helper::spatial_type* pval = 0;
    
    if(m_psT != 0)
    {
      SecondoCatalog* pSecondoCatalog = SecondoSystem::GetCatalog();
      
      if(pSecondoCatalog != 0)
      {
        ListExpr numericType = pSecondoCatalog->NumericType(
                               NList(Helper::spatial_type::BasicType())
                               .listExpr());
        Word word = SetWord(m_psT);
        
        pval = static_cast<typename Helper::spatial_type*>
               (Helper::spatial_type::Clone(numericType, word).addr);
      }
    }
    
    return pval;
  }

  template <typename T, typename Helper>
  void istype<T, Helper>::setCacheSize(size_t size)
  {
      assert(m_psT != 0);
      m_psT->setCacheSize(size);
  }

  template <typename T, typename Helper>
  void istype<T, Helper>::flushCache()
  {
      assert(m_psT != 0);
      m_psT->flushCache();
  }

  template <typename T, typename Helper>
  const string istype<T, Helper>::BasicType()
  { 
    return Helper::name;
  }

  template <typename T, typename Helper>
  const bool istype<T, Helper>::checkType(const ListExpr type)
  {
    return listutils::isSymbol(type, BasicType());
  }

  template <typename T, typename Helper>
  void* istype<T, Helper>::Cast(void* pVoid)
  { 
    return new (pVoid) istype<T, Helper>();
  }

  template <typename T, typename Helper>
  Word istype<T, Helper>::Clone(const ListExpr typeInfo,
                                const Word& w)
  { 
    Word word = SetWord(Address(0));
    
    istype<T, Helper>* pistype = static_cast<istype<T, Helper>*>(w.addr);
    
    if(pistype != 0)
    {
      SecondoCatalog* pSecondoCatalog = SecondoSystem::GetCatalog();
      
      if(pSecondoCatalog != 0)
      {
        DateTime* pInstant = static_cast<DateTime*>
                             (pistype->m_pInstant->Clone());
        
        ListExpr numericType = pSecondoCatalog->NumericType(
                               NList(Helper::spatial_type::BasicType())
                               .listExpr());
        word = SetWord(pistype->m_psT);
        typename Helper::spatial_type* psT =
          static_cast<typename Helper::spatial_type*>
          (Helper::spatial_type::Clone(numericType, word).addr);
        
        if(pInstant != 0 &&
           psT != 0)
        {    
          word = SetWord(new istype<T, Helper>(pInstant, psT));
        }
      }
    }
    
    return word;
  }

  template <typename T, typename Helper>
  void istype<T, Helper>::Close(const ListExpr typeInfo,
                                Word& w)
  { 
    istype<T, Helper>* pistype = static_cast<istype<T, Helper>*>(w.addr);
    
    if(pistype != 0)
    {
      delete pistype;
      w = SetWord(Address(0));
    }
  }

  template <typename T, typename Helper>
  Word istype<T, Helper>::Create(const ListExpr typeInfo)
  {
    Word w = SetWord(new istype<T, Helper>());
    return w;
  }

  template <typename T, typename Helper>
  void istype<T, Helper>::Delete(const ListExpr typeInfo,
                                 Word& w)
  {
    istype<T, Helper>* pistype = static_cast<istype<T, Helper>*>(w.addr);
    
    if(pistype != 0)
    {
      delete pistype;
      w = SetWord(Address(0));
    }
  }

  template <typename T, typename Helper>
  TypeConstructor istype<T, Helper>::getTypeConstructor()
  { 
    TypeConstructor typeConstructoristype(
      istype<T, Helper>::BasicType(), // type name function    
      istype<T, Helper>::Property,    // property function describing signature
      istype<T, Helper>::Out,         // out function
      istype<T, Helper>::In,          // in function
      0,                              // save to list function
      0,                              // restore from list function
      istype<T, Helper>::Create,      // create function
      istype<T, Helper>::Delete,      // delete function
      istype<T, Helper>::Open,        // open function
      istype<T, Helper>::Save,        // save function
      istype<T, Helper>::Close,       // close function
      istype<T, Helper>::Clone,       // clone function
      istype<T, Helper>::Cast,        // cast function
      istype<T, Helper>::SizeOfObj,   // sizeofobj function
      istype<T, Helper>::KindCheck);  // kindcheck function

    typeConstructoristype.AssociateKind(Kind::SIMPLE());
    
    return typeConstructoristype;
  }

  template <typename T, typename Helper>
  Word istype<T, Helper>::In(const ListExpr typeInfo,
                             const ListExpr instance,
                             const int errorPos,
                             ListExpr& errorInfo,
                             bool& correct)
  { 
    NList nlist(instance);

    typename Helper::spatial_type* psT = 0;
    DateTime* pInstant = new DateTime(instanttype);

    try {
        if (nlist.length() != 2) {
            throw util::parse_error(
               "Expected (instant " + Helper::spatial_type::BasicType() + ").");
        }

        correct = pInstant->ReadFrom(nlist.elem(1).listExpr(), true);
        if (!correct) {
            throw util::parse_error("Cannot parse instant.");
        }

        if (!pInstant->IsDefined()) {
            throw util::parse_error("Instant cannot be undefined.");
        }

        ListExpr serrorInfo;
        Word w = Helper::spatial_type::In(0, nlist.elem(2).listExpr(),
                                          0, serrorInfo, correct);
        if (correct) {
            assert(w.addr != 0);
            psT = static_cast<typename Helper::spatial_type*>(w.addr);
        } else {
            assert(w.addr == 0);
            throw util::parse_error(
                    "Cannot parse " + Helper::spatial_type::BasicType() + ".");
        }

    } catch (util::parse_error& e) {
        delete pInstant;
        correct = false;
        cmsg.inFunError(e.what());
        return Word();
    }
    
    istype<T, Helper>* is = new istype<T, Helper>(pInstant, psT);
    return SetWord(Address(is));
  }

  template <typename T, typename Helper>
  bool istype<T, Helper>::KindCheck(ListExpr type,
                                    ListExpr& errorInfo)
  { 
    bool bRetVal = false;
    
    if(nl != 0)
    {
      bRetVal = nl->IsEqual(type, istype<T, Helper>::BasicType());
    }
    
    return bRetVal;
  }

  template <typename T, typename Helper>
  bool istype<T, Helper>::Open(SmiRecord& valueRecord,
                               size_t& offset,
                               const ListExpr typeInfo,
                               Word& value)
  { 
    bool bRetVal = true;
    
    SecondoCatalog* pSecondoCatalog = SecondoSystem::GetCatalog();
      
    if(pSecondoCatalog != 0)
    { 
      ListExpr numericType = pSecondoCatalog->NumericType(NList(
                                              DateTime::BasicType()).
                                              listExpr());
      
      DateTime* pInstant = static_cast<DateTime*>(DateTime::Open(valueRecord,
                                                  offset, numericType));
      
      numericType = pSecondoCatalog->NumericType(
                                     NList(Helper::spatial_type::BasicType()).
                                     listExpr());
      Word w = SetWord(Address(0));
      bRetVal &= Helper::spatial_type::Open(valueRecord, offset,
                                            numericType, w);
      
      typename Helper::spatial_type* psT =
        static_cast<typename Helper::spatial_type*>(w.addr);
      
      if(pInstant != 0 &&
         psT != 0)
      {
        istype<T, Helper>* pistype = new istype<T, Helper>(pInstant, psT);
        
        if(pistype != 0)
        {
          value = SetWord(pistype);
        }
      }
    }
    
    return bRetVal;
  }

  template <typename T, typename Helper>
  ListExpr istype<T, Helper>::Out(ListExpr typeInfo,
                                  Word value)
  { 
    istype<T, Helper>* pistype = static_cast<istype<T, Helper>*>(value.addr);

    NList InstantList;
    if (pistype->m_pInstant->IsDefined()) {
        InstantList = NList(pistype->m_pInstant->ToListExpr(true));
    } else {
        InstantList = NList(NList(Instant::BasicType()),
                            NList(Symbol::UNDEFINED()));
    }

    NList sTList = NList(Helper::spatial_type::Out(0, SetWord(pistype->m_psT)));

    return NList(InstantList, sTList).listExpr();
  }

  template <typename T, typename Helper>
  ListExpr istype<T, Helper>::Property()
  {
    NList property;

    if (property.isEmpty())
    {
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

        property = NList(names, values);
    }

    return property.listExpr();
  }

  template <typename T, typename Helper>
  bool istype<T, Helper>::Save(SmiRecord& valueRecord,
                               size_t& offset,
                               const ListExpr typeInfo,
                               Word& value)
  {
    bool bRetVal = true;
    
    istype<T, Helper>* pistype = static_cast<istype<T, Helper>*>(value.addr);
    
    if(pistype != 0)
    {
      SecondoCatalog* pSecondoCatalog = SecondoSystem::GetCatalog();
      
      if(pSecondoCatalog != 0)
      { 
        ListExpr numericType = pSecondoCatalog->NumericType(
                                                NList(DateTime::BasicType()).
                                                      listExpr());
        DateTime::Save(valueRecord, offset, numericType, pistype->m_pInstant);
        
        numericType = pSecondoCatalog->NumericType(
                                       NList(Helper::spatial_type::BasicType()).
                                       listExpr());
        Word w = SetWord(pistype->m_psT);
        bRetVal = Helper::spatial_type::Save(valueRecord, offset,
                                             numericType, w);
      }
    }
    
    return bRetVal;
  }

  template <typename T, typename Helper>
  int istype<T, Helper>::SizeOfObj()
  { 
    return sizeof(istype<T, Helper>);
  }

  template <typename T, typename Helper>
  void istype<T, Helper>::Delete()
  {
    if(m_bDelete == true)
    {
      if(m_psT != 0)
      {
        delete m_psT;
        m_psT = 0;
      }
      
      if(m_pInstant != 0)
      {
        delete m_pInstant;
        m_pInstant = 0;
      }
    }
  }
}

#endif /* #ifndef RASTER2_ISTYPE_H */
