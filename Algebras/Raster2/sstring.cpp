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

#include <algorithm>

#include "sstring.h"
#include "SecondoCatalog.h"

namespace raster2
{

const char* stype_helper<string>::name = TYPE_NAME_SSTRING;

sstring::sstring()
    : m_bDelete(false),
      m_psint(new sint()),
      m_pUniqueStringArray(new UniqueStringArray(true)),
      m_minimum(UNDEFINED_STRING_INDEX),
      m_maximum(UNDEFINED_STRING_INDEX)
{}

sstring::sstring(sint* psint, UniqueStringArray* pUniqueStringArray,
                 int minimum, int maximum)
    : m_bDelete(false),
      m_psint(psint),
      m_pUniqueStringArray(pUniqueStringArray),
      m_minimum(minimum),
      m_maximum(maximum)
{}

sstring::~sstring()
{ 
    if (m_bDelete) {
        m_psint->destroy();
        m_pUniqueStringArray->Destroy();
    }
    delete m_psint;
    delete m_pUniqueStringArray;
}

void sstring::clear(){
   m_psint->clear();
   m_pUniqueStringArray->clear();
   m_minimum=UNDEFINED_STRING_INDEX;
   m_maximum=UNDEFINED_STRING_INDEX;
}

bool sstring::isDefined() const{
   return m_psint->isDefined();
}

void sstring::setDefined( const bool _defined){
  if(_defined != isDefined()){
     m_psint->setDefined(_defined);
     if(!_defined){
        m_pUniqueStringArray->clear();
        m_minimum=UNDEFINED_STRING_INDEX;
        m_maximum=UNDEFINED_STRING_INDEX;
     }
  } 
}



void sstring::destroy() {
    m_bDelete = true;
}

sint::storage_type& sstring::getStorage()
{
  return m_psint->getStorage();
}

string sstring::atlocation(double x, double y) const
{
  string value = UNDEFINED_STRING;
  
  if(m_psint != 0 &&
     m_pUniqueStringArray != 0)
  {
    int nLocationIndex = m_psint->atlocation(x, y);
    
    if(nLocationIndex >= 0)
    {
      bool bOK = m_pUniqueStringArray->GetUniqueString(nLocationIndex, value);
      assert(bOK);
    }
  }
  
  return value;
}

bool sstring::isUndefined(const std::string& t) {
  return stype_helper<std::string>::isUndefined(t);
}

stype_helper<std::string>::wrapper_type sstring::wrap
    (const std::string& t)
{
  return stype_helper<std::string>::wrap(t);
}

std::string sstring::unwrap(const CcString& w) {
  return stype_helper<std::string>::unwrap(w);
}

sstring* sstring::atrange(const Rect& rRect) const
{
    sstring* result = new sstring();
    result->setGrid(getGrid());

    index_type from = m_psint->getGrid().getIndex(rRect.MinD(0), rRect.MinD(1));
    index_type to = m_psint->getGrid().getIndex(rRect.MaxD(0), rRect.MaxD(1));

    std::string value;

    for (riter_type rit = m_psint->iterate_regions(from, to),
                    re  = m_psint->end_regions();
         rit != re; ++rit)
    {
        index_type from = *rit;
        index_type to = *rit + sint::riter_type::region_size;

        for (index_type i = from; i < to; i.increment(from, to)) {
            m_pUniqueStringArray->GetUniqueString(m_psint->get(i), value);
            result->set(i, value);
        }
    }

    return result;
}

Rect sstring::bbox() const
{
  Rect boundingBox(false);
  
  if((m_psint != 0) && isDefined())
  {
    boundingBox = m_psint->bbox();
  }
  
  return boundingBox;
}

string sstring::getMinimum() const
{
  std::string result = UNDEFINED_STRING;
  if ( (m_minimum != UNDEFINED_STRING_INDEX) && isDefined()) {
    m_pUniqueStringArray->GetUniqueString(m_minimum, result);
  }
  return result;
}

string sstring::getMaximum() const
{
  std::string result = UNDEFINED_STRING;
  if ((m_maximum != UNDEFINED_STRING_INDEX) && isDefined()) {
    m_pUniqueStringArray->GetUniqueString(m_maximum, result);
  }
  return result;
}

grid2 sstring::getGrid() const
{
  grid2 grid2;
  
  if(m_psint != 0)
  {
    grid2 = m_psint->getGrid();
  }
  
  return grid2;
}

void sstring::setGrid(const grid2& rGrid)
{
  m_psint->setGrid(rGrid);
}

sstring::riter_type sstring::begin_regions() const {
    assert(m_psint != 0);
    return m_psint->getStorage().begin_regions();
}

sstring::riter_type sstring::end_regions() const {
    assert(m_psint != 0);
    return m_psint->getStorage().end_regions();
}

sstring::riter_type sstring::iterate_regions(const index_type& from,
                                             const index_type& to) const
{
    assert(m_psint != 0);
    return m_psint->iterate_regions(from, to);
}

void sstring::set(const index_type& i, const std::string& value) {
    assert(m_psint != 0);
    assert(m_pUniqueStringArray != 0);

    if (value == UNDEFINED_STRING) {
        m_psint->getStorage()[i] = UNDEFINED_INT;
        return;
    }

    int index = m_pUniqueStringArray->GetUniqueStringIndex(value);
    if (index == UNDEFINED_STRING_INDEX) {
        std::string minimum = UNDEFINED_STRING;
        if (m_minimum != UNDEFINED_STRING_INDEX) {
          m_pUniqueStringArray->GetUniqueString(m_minimum, minimum);
        }
        std::string maximum = UNDEFINED_STRING;
        if (m_maximum != UNDEFINED_STRING_INDEX) {
          m_pUniqueStringArray->GetUniqueString(m_maximum, maximum);
        }

        index = m_pUniqueStringArray->AddString(value);

        if (m_minimum == UNDEFINED_STRING_INDEX) {
            m_minimum = index;
        } else if (!stype_helper<std::string>::isUndefined(value)) {
            if (value < minimum) {
                m_minimum = index;
            }
        }
        if (m_maximum == UNDEFINED_STRING_INDEX) {
            m_maximum = index;
        } else if (!stype_helper<std::string>::isUndefined(value)) {
            if (value > maximum) {
                m_maximum = index;
            }
        }
    }
    m_psint->getStorage()[i] = index;
}

std::string sstring::get(const index_type& i) const {
    assert(m_psint != 0);
    assert(m_pUniqueStringArray != 0);

    std::string result = stype_helper<std::string>::getUndefined();
    int index = m_psint->getStorage()[i];
    if (index >= 0) {
        m_pUniqueStringArray->GetUniqueString(index, result);
    }
    return result;
}

void sstring::setCacheSize(size_t size) {
    m_psint->setCacheSize(size);
}

void sstring::flushCache() {
    m_psint->flushCache();
}

const string sstring::BasicType()
{ 
  return TYPE_NAME_SSTRING;
}

const bool sstring::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

void* sstring::Cast(void* pVoid)
{ 
  return new (pVoid) sstring;
}

Word sstring::Clone(const ListExpr typeInfo,
                    const Word& w)
{ 
  Word word = SetWord(Address(0));
  
  sstring* psstring = static_cast<sstring*>(w.addr);
  
  if(psstring != 0)
  {
    SecondoCatalog* pSecondoCatalog = SecondoSystem::GetCatalog();
    
    if(pSecondoCatalog != 0)
    {
      ListExpr numericType = pSecondoCatalog->NumericType(
                              NList(sint::BasicType()).listExpr());
      word = SetWord(psstring->m_psint);
      sint* psint = static_cast<sint*>(sint::Clone(numericType, word).addr);
      
      numericType = pSecondoCatalog->NumericType(
                              NList(UniqueStringArray::BasicType()).listExpr());
      word = SetWord(psstring->m_pUniqueStringArray);
      UniqueStringArray* pUniqueStringArray =
        static_cast<UniqueStringArray*>(UniqueStringArray::Clone(numericType,
                                                                 word).addr);
      if(psint != 0 &&
         pUniqueStringArray != 0)
      {    
        word = SetWord(new sstring(psint, pUniqueStringArray,
                       psstring->m_minimum, psstring->m_maximum));
      }
    }
  }
  
  return word;
}

void sstring::Close(const ListExpr typeInfo,
                    Word& w)
{ 
  sstring* psstring = static_cast<sstring*>(w.addr);
  
  if(psstring != 0)
  {
    delete psstring;
    w = SetWord(Address(0));
  }
}

Word sstring::Create(const ListExpr typeInfo)
{
  Word w = SetWord(new sstring());
  return w;
}

void sstring::Delete(const ListExpr typeInfo,
                     Word& w)
{
  sstring* psstring = static_cast<sstring*>(w.addr);
  
  if(psstring != 0)
  {
    delete psstring;
    w = SetWord(Address(0));
  }
}

TypeConstructor sstring::getTypeConstructor()
{ 
  TypeConstructor typeConstructorsstring(
    sstring::BasicType(), // type name function    
    sstring::Property,    // property function describing signature
    sstring::Out,         // out function
    sstring::In,          // in function
    0,                    // save to list function
    0,                    // restore from list function
    sstring::Create,      // create function
    sstring::Delete,      // delete function
    sstring::Open,        // open function
    sstring::Save,        // save function
    sstring::Close,       // close function
    sstring::Clone,       // clone function
    sstring::Cast,        // cast function
    sstring::SizeOfObj,   // sizeofobj function
    sstring::KindCheck);  // kindcheck function

  typeConstructorsstring.AssociateKind(Kind::SIMPLE());
  
  return typeConstructorsstring;
}

Word sstring::In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& errorInfo,
                 bool& correct)
{ 
 
  if(listutils::isSymbolUndefined(instance)){
     correct = true;
     sstring* res = new sstring();
     res->setDefined(false);
     return Word(res);
  }
 
  Word w = SetWord(Address(0));

  NList nlist(instance);
  NList sintList;

  grid2 grid;
  std::pair<int, int> sizes;
  string minimum = UNDEFINED_STRING;
  string maximum = UNDEFINED_STRING;
  UniqueStringArray* pUniqueStringArray = new UniqueStringArray(true);

  try
  {
    if (nlist.isAtom()) {
        throw util::parse_error
          ("Expected list as first element, got an atom.");
    }

    NList grid2list = nlist.elem(1);
    nlist.rest();

    if (grid2list.length() != 3) {
        throw util::parse_error
          ("Type mismatch: list for grid2 is too short or too long.");
    }
    if (!grid2list.isReal(1)
     || !grid2list.isReal(2)
     || !grid2list.isReal(3))
    {
        throw util::parse_error(
          "Type mismatch: expected 3 reals as grid2 sublist.");
    }

    if (grid2list.elem(3).realval() <= 0) {
        throw util::parse_error(
          "The length in a grid2 must be larger than 0.");
    }

    sintList.append(grid2list);

    if (!nlist.isEmpty()) {
        NList sizelist = nlist.elem(1);
        nlist.rest();
        if (sizelist.length() != 2) {
            throw util::parse_error(
              "Type mismatch: list for partial grid sizes is too short.");
        }
        if ( sizelist.isInt(1)
          && sizelist.isInt(2)
          && sizelist.elem(1).intval() > 0
          && sizelist.elem(2).intval() > 0)
        {
            sintList.append(sizelist);

            sizes.first = sizelist.elem(1).intval();
            sizes.second = sizelist.elem(2).intval();
        } else {
            throw util::parse_error("Type mismatch: "
              "partial grid size must contain two positive integers.");
        }
    }

    while(!nlist.isEmpty())
    {
      NList sintPartialList;

      sint::index_type root;
      NList pagelist = nlist.first();
      nlist.rest();

      if(pagelist.length() != 3)
      {
        throw util::parse_error("Type mismatch: "
          "partial grid content must contain three elements.");
      }

      if(pagelist.isInt(1) &&
        pagelist.isInt(2))
      {
        root[0] = pagelist.elem(1).intval();
        root[1] = pagelist.elem(2).intval();
      }

      else
      {
        throw util::parse_error("Type mismatch: "
          "partial grid content must start with two integers.");
      }

      sintPartialList.append(pagelist.elem(1));
      sintPartialList.append(pagelist.elem(2));

      pagelist.rest();
      pagelist.rest();

      NList valuelist = pagelist.first();

      if(valuelist.length() != Cardinal(sizes.first) * Cardinal(sizes.second))
      {
        throw util::parse_error("Type mismatch: "
          "list for partial grid values is too short or too long.");
      }

      NList sintValueList;

      for(int row = 0; row < sizes.first; ++row)
      {
        for(int column = 0; column < sizes.second; ++column)
        {
          string value;
          int i = row * sizes.first + column + 1;

          if(valuelist.elem(i).isSymbol(Symbol::UNDEFINED()))
          {
            sintValueList.append(valuelist.elem(i));
          }

          else if(stype_helper<string>::check(valuelist.elem(i)))
          {
            value = stype_helper<string>::parse(valuelist.elem(i));
            
            if(stype_helper<string>::isUndefined(minimum) ||
               value < minimum)
            {
              minimum = value;
            }
            
            if(stype_helper<string>::isUndefined(maximum) ||
               value > maximum)
            {
              maximum = value;
            }
            
            int nUniqueStringIndex = pUniqueStringArray->AddString(value);
            sintValueList.append(NList(nUniqueStringIndex));
          }

          else
          {
            throw util::parse_error("Type mismatch: "
              "list value in partial grid has wrong type.");
          }
        }
      }

      sintPartialList.append(sintValueList);
      sintList.append(sintPartialList);
    }
  }

  catch(util::parse_error& e)
  {
    delete pUniqueStringArray;
    cmsg.inFunError(e.what());
    correct = false;
    return w;
  }

  w = sint::In(typeInfo, sintList.listExpr(), errorPos, errorInfo, correct);

  if(correct == true)
  {
    sint* psint = static_cast<sint*>(w.addr);
    
    if(psint != 0)
    {
      std::list<std::string> l = pUniqueStringArray->GetUniqueStringArray();
      l.sort();
      int minimum = pUniqueStringArray->GetUniqueStringIndex(l.front());
      int maximum = pUniqueStringArray->GetUniqueStringIndex(l.back());

      sstring* psstring = new sstring(psint, pUniqueStringArray,
                                      minimum, maximum);
      
      correct = true;
      w = SetWord(psstring);
    }
  } else {
      delete pUniqueStringArray;
  }

  return w;
}

bool sstring::KindCheck(ListExpr type,
                        ListExpr& errorInfo)
{ 
  bool bRetVal = false;
  
  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, sstring::BasicType());
  }
  
  return bRetVal;
}

bool sstring::Open(SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value)
{
  int minimum, maximum;

  valueRecord.SetPos(offset);
  valueRecord.Read(minimum);
  valueRecord.Read(maximum);
  offset = valueRecord.GetPos();

  bool bRetVal = true;
  
  SecondoCatalog* pSecondoCatalog = SecondoSystem::GetCatalog();
    
  if(pSecondoCatalog != 0)
  {
    ListExpr numericType = pSecondoCatalog->NumericType(
                            NList(sint::BasicType()).listExpr());
    Word w = SetWord(Address(0));
    bRetVal &= sint::Open(valueRecord, offset, numericType, w);
    sint* psint = static_cast<sint*>(w.addr);
    
    numericType = pSecondoCatalog->NumericType(NList(
                                    UniqueStringArray::BasicType()).listExpr());
    w = SetWord(Address(0));
    bRetVal &= UniqueStringArray::Open(valueRecord, offset, numericType, w);
    UniqueStringArray* pUniqueStringArray = static_cast<UniqueStringArray*>
                                                      (w.addr);
    
    if(psint != 0 &&
      pUniqueStringArray != 0)
    {
      sstring* psstring = new sstring(psint, pUniqueStringArray,
                                      minimum, maximum);
      
      if(psstring != 0)
      {
        value = SetWord(psstring);
      }
    }
  }
  
  return bRetVal;
}

bool sstring::Save(SmiRecord& valueRecord,
                   size_t& offset,
                   const ListExpr typeInfo,
                   Word& value)
{
  bool bRetVal = true;

  sstring* psstring = static_cast<sstring*>(value.addr);

  if(psstring != 0)
  {
    valueRecord.SetPos(offset);
    valueRecord.Write(psstring->m_minimum);
    valueRecord.Write(psstring->m_maximum);
    offset = valueRecord.GetPos();

    SecondoCatalog* pSecondoCatalog = SecondoSystem::GetCatalog();

    if(pSecondoCatalog != 0)
    {
      ListExpr numericType = pSecondoCatalog->NumericType(
                              NList(sint::BasicType()).listExpr());
      Word w = SetWord(psstring->m_psint);
      bRetVal &= sint::Save(valueRecord, offset, numericType, w);

      numericType = pSecondoCatalog->NumericType(
                              NList(UniqueStringArray::BasicType()).listExpr());
      w = SetWord(psstring->m_pUniqueStringArray);
      bRetVal &= UniqueStringArray::Save(valueRecord, offset, numericType, w);
    }
  }

  return bRetVal;
}

ListExpr sstring::Out(ListExpr typeInfo,
                      Word value)
{ 
  sstring* psstring = static_cast<sstring*>(value.addr);

  if(!psstring->isDefined()){
     return nl->SymbolAtom(Symbol::UNDEFINED());
  }

  ListExpr pListExpr = 0;
  
  if(nl != 0)
  {  
    
    if(psstring != 0)
    {
      NList result;

      NList gridlist;
      const grid2& rGrid = psstring->m_psint->getGrid(); 
      gridlist.append(rGrid.getOriginX());
      gridlist.append(rGrid.getOriginY());
      gridlist.append(rGrid.getLength());
      result.append(gridlist);

      NList tilesizelist;
      const sint::index_type& size = sint::riter_type::region_size;
      sint::index_type index;
      int element;

      sint::storage_type& storage = psstring->m_psint->getStorage();
      RasterRegion<2> bb = storage.bbox();
      sint::index_type sz = bb.Max - bb.Min;

      if (sz[0] <= size[0] && sz[1] <= size[1]) {
          tilesizelist.append(1);
          tilesizelist.append(1);
          result.append(tilesizelist);

          for (sint::iter_type it = storage.begin(),
                                 e = storage.end();
               it != e; ++it)
          {
              element = *it;
              NList partiallist;
              partiallist.append(it.getIndex()[0]);
              partiallist.append(it.getIndex()[1]);

              NList valuelist;
              if(stype_helper<int>::isUndefined(element))
              {
                valuelist.append(NList(Symbol::UNDEFINED()));
              }

              else
              {
                bool bOK = false;
                string uniqueString;

                bOK = psstring
                        ->m_pUniqueStringArray
                        ->GetUniqueString(element, uniqueString);

                if(bOK == true)
                {
                  valuelist.append(NList(uniqueString, true, false));
                }
              }

              partiallist.append(valuelist);
              result.append(partiallist);
          }
      } else {
          tilesizelist.append(size[0]);
          tilesizelist.append(size[1]);
          result.append(tilesizelist);

          for(sint::riter_type rit = storage.begin_regions(),
              re  = storage.end_regions();
              rit != re;
              ++rit)
          {
            NList partiallist;
            partiallist.append((*rit)[0]);
            partiallist.append((*rit)[1]);
            
            NList valuelist;
            
            for(int row = 0; row < size[0]; ++row)
            {
              for(int column = 0; column < size[1]; ++column)
              {
                index = *rit + sint::index_type((int[]){row, column});
                element = storage[index];

                if(stype_helper<int>::isUndefined(element))
                {
                  valuelist.append(NList(Symbol::UNDEFINED()));
                }

                else
                {
                  bool bOK = false;
                  string uniqueString;

                  bOK = psstring
                          ->m_pUniqueStringArray
                          ->GetUniqueString(element, uniqueString);

                  if(bOK == true)
                  {
                    valuelist.append(NList(uniqueString, true, false));
                  }
                }
              }
            }

            partiallist.append(valuelist);
            result.append(partiallist);
          }
      }

      pListExpr = result.listExpr();
    }
  }
  
  return pListExpr;
}

ListExpr sstring::Property()
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
      values.append(NList(
              std::string(
                "((x y l) (szx szy) ((ix iy (v*)))*)"),
              true));
      values.append(NList(
              std::string("((0 0 1) (2 2) ((5 5 (\"A\" \"B\" \"A\" \"C\"))))"),
              true));
      values.append(NList(std::string(""), true));

      property = NList(names, values);
  }

  return property.listExpr();
}

int sstring::SizeOfObj()
{ 
  return sizeof(sstring);
}


void swap(raster2::sstring& a, raster2::sstring& b)
{
    std::swap(a.m_bDelete, b.m_bDelete);
    std::swap(a.m_psint, b.m_psint);
    std::swap(a.m_pUniqueStringArray, b.m_pUniqueStringArray);
}

}
