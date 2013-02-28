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

#include "msstring.h"
#include "SecondoCatalog.h"

namespace raster2
{

const char* mstype_helper<string>::name = TYPE_NAME_MSSTRING;

msstring::msstring()
  : m_bDelete(false),
    m_pmsint(new msint()),
    m_pUniqueStringArray(new UniqueStringArray(true)),
    m_minimum(UNDEFINED_STRING_INDEX),
    m_maximum(UNDEFINED_STRING_INDEX)
{ }

msstring::msstring(msint* pmsint, UniqueStringArray* pUniqueStringArray,
                   int minimum, int maximum)
  : m_bDelete(false),
    m_pmsint(pmsint),
    m_pUniqueStringArray(pUniqueStringArray),
    m_minimum(minimum),
    m_maximum(maximum)
{ }

msstring::~msstring()
{
    if (m_bDelete) {
        m_pmsint->destroy();
        m_pUniqueStringArray->Destroy();
    }
    delete m_pmsint;
    delete m_pUniqueStringArray;
}

void msstring::destroy()
{
    m_bDelete = true;
}

void msstring::getDefinedPeriods(Periods& result) const
{
  if(m_pmsint != 0)
  {
      m_pmsint->getDefinedPeriods(result);
  }
}
    
msstring* msstring::atperiods(const Periods& periods)
{
    msstring* result = new msstring();

    grid3 copy(m_pmsint->getGrid().getOriginX(),
               m_pmsint->getGrid().getOriginY(),
               m_pmsint->getGrid().getLength(),
               m_pmsint->getGrid().getDuration());
    result->m_pmsint->setGrid(copy);

    double duration = copy.getDuration().ToDouble();

    const index_type& size = riter_type::region_size;
    index_type index;
    std::string element;

    for (riter_type rit = this->begin_regions(),
                    re  = this->end_regions();
         rit != re; ++rit)
    {
        for (int r = 0; r < size[0]; ++r) {
            for (int c = 0; c < size[1]; ++c) {
                for (int t = 0; t < size[2]; ++t) {
                    index = *rit + index_type((int[]){r, c, t});
                    element = this->get(index);
                    if (!mstype_helper<std::string>::isUndefined(element)) {
                        DateTime start = DateTime(t*duration);
                        DateTime end = DateTime((t+1)*duration);

                        Interval<DateTime> value(start, end, true, false);
                        if (periods.Intersects(value))
                        {
                            RasterIndex<3> ri = (int[]){r, c, t};
                            result->set(ri, element);
                        }
                    }
                }
            }
        }
    }

    return result;
}

msstring* msstring::atrange(const Rect& pRect, const Instant& start,
                            const Instant& end)
{
    msstring* result = new msstring();

    Interval<DateTime> lookupinterval(start, end, true, false);

    grid3 copy(m_pmsint->getGrid().getOriginX(),
               m_pmsint->getGrid().getOriginY(),
               m_pmsint->getGrid().getLength(),
               m_pmsint->getGrid().getDuration());
    result->m_pmsint->setGrid(copy);

    const index_type& size = riter_type::region_size;
    index_type index;
    std::string element;

    for (riter_type rit = this->begin_regions(),
                    re  = this->end_regions();
                    rit != re; ++rit)
    {
        index = *rit + index_type((int[]){size[0]%2,
                                          size[1]%2,
                                          size[2]%2});
        double duration = this->getGrid().getDuration().ToDouble();
        DateTime actstart(index[2] * duration);
        DateTime actend((index[2] + 1) * duration);
        Interval<DateTime> interval(start, end, true, false);
        if (lookupinterval.Contains(interval)) {
            if ((pRect.MinD(0) < index[0] &&
                 pRect.MaxD(0) > index[0]) &&
                (pRect.MinD(1) < index[1] &&
                 pRect.MaxD(1) > index[1])) {
                for (int r = 0; r < size[0]; ++r) {
                    for (int c = 0; c < size[1]; ++c) {
                        for (int t = 0; t < size[2]; ++t) {
                            index = *rit + index_type((int[]){r, c, t});
                            element = this->get(index);
                    if (!mstype_helper<std::string>::isUndefined(element)) {
                                RasterIndex<3> ri = (int[]){r, c, t};
                                result->set(ri, element);
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}

    
string msstring::atlocation(double x, double y, double t) const
{
  string value = UNDEFINED_STRING;
  
  if(m_pmsint != 0 &&
     m_pUniqueStringArray != 0)
  {
    int nLocationIndex = m_pmsint->atlocation(x, y, t);
    
    if(nLocationIndex >= 0)
    {
      bool bOK = m_pUniqueStringArray->GetUniqueString(nLocationIndex, value);
      assert(bOK);
    }
  }
  
  return value;
}

string msstring::atlocation(double x, double y) const
{
  string value = UNDEFINED_STRING;
  
  if(m_pmsint != 0 &&
     m_pUniqueStringArray != 0)
  {
    /*
      * * TODO:
      */
    // MInt movingInt = m_pmsint->atlocation(x, y);
    int nLocationIndex = 0;

    if(nLocationIndex >= 0)
    {
      bool bOK = m_pUniqueStringArray->GetUniqueString(nLocationIndex, value);
      assert(bOK);
    }
  }
  
  return value;
}

Rectangle<3> msstring::bbox() const
{
  Rectangle<3> boundingBox(false);
  
  if(m_pmsint != 0)
  {
    boundingBox = m_pmsint->bbox();
  }
  
  return boundingBox;
}

string msstring::getMinimum() const
{
  std::string result = UNDEFINED_STRING;
  if (m_minimum != UNDEFINED_STRING_INDEX) {
    m_pUniqueStringArray->GetUniqueString(m_minimum, result);
  }
  return result;
}

string msstring::getMaximum() const
{
  std::string result = UNDEFINED_STRING;
  if (m_maximum != UNDEFINED_STRING_INDEX) {
    m_pUniqueStringArray->GetUniqueString(m_maximum, result);
  }
  return result;
}

grid3 msstring::getGrid() const
{
  assert(m_pmsint != 0);

  return m_pmsint->getGrid();
}

void msstring::setGrid(const grid3& g) {
  assert(m_pmsint != 0);

  m_pmsint->setGrid(g);
}

msstring::riter_type msstring::begin_regions() {
    assert(m_pmsint != 0);
    return m_pmsint->getStorage().begin_regions();
}

msstring::riter_type msstring::end_regions() {
    assert(m_pmsint != 0);
    return m_pmsint->getStorage().end_regions();
}

msstring::riter_type msstring::iterate_regions(const index_type& from,
                                               const index_type& to)
{
    assert(m_pmsint != 0);
    return m_pmsint->iterate_regions(from, to);
}

void msstring::set(const index_type& i, const std::string& value) {
    assert(m_pmsint != 0);
    assert(m_pUniqueStringArray != 0);

    if (value == UNDEFINED_STRING) {
        m_pmsint->getStorage()[i] = UNDEFINED_INT;
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
        } else if (!mstype_helper<std::string>::isUndefined(value)) {
            if (value < minimum) {
                m_minimum = index;
            }
        }
        if (m_maximum == UNDEFINED_STRING_INDEX) {
            m_maximum = index;
        } else if (!mstype_helper<std::string>::isUndefined(value)) {
            if (value > maximum) {
                m_maximum = index;
            }
        }
    }
    m_pmsint->getStorage()[i] = index;
}

std::string msstring::get(const index_type& i) const {
    assert(m_pmsint != 0);
    assert(m_pUniqueStringArray != 0);

    std::string result = mstype_helper<std::string>::getUndefined();
    int index = m_pmsint->getStorage()[i];
    if (index >= 0) {
        m_pUniqueStringArray->GetUniqueString(index, result);
    }
    return result;
}

bool msstring::isUndefined(const std::string& t) {
  return mstype_helper<std::string>::isUndefined(t);
}

mstype_helper<std::string>::wrapper_type msstring::wrap
    (const std::string& t)
{
  return mstype_helper<std::string>::wrap(t);
}

std::string msstring::unwrap(const CcString& w) {
  return mstype_helper<std::string>::unwrap(w);
}

void msstring::setCacheSize(size_t size) {
    m_pmsint->setCacheSize(size);
}

void msstring::flushCache() {
    m_pmsint->flushCache();
}

void msstring::clear(){
   m_pmsint->clear();
   m_pUniqueStringArray->clear();
   m_minimum = m_maximum = UNDEFINED_STRING_INDEX;
}

bool msstring::isDefined()const{
   return m_pmsint->isDefined();
}

void msstring::setDefined(const bool _defined){
  if(_defined != isDefined()){
    m_pmsint->setDefined(_defined);
    if(!_defined){
      m_pUniqueStringArray->clear();
      m_minimum = m_maximum = UNDEFINED_STRING_INDEX;
    }
  }
}




const string msstring::BasicType()
{ 
  return TYPE_NAME_MSSTRING;
}

const bool msstring::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

void* msstring::Cast(void* pVoid)
{ 
  return new (pVoid) msstring;
}

Word msstring::Clone(const ListExpr typeInfo,
                     const Word& w)
{ 
  Word word = SetWord(Address(0));
  
  msstring* pmsstring = static_cast<msstring*>(w.addr);
  
  if(pmsstring != 0)
  {
    SecondoCatalog* pSecondoCatalog = SecondoSystem::GetCatalog();
    
    if(pSecondoCatalog != 0)
    {
      ListExpr numericType = pSecondoCatalog->NumericType(
                              NList(msint::BasicType()).listExpr());
      word = SetWord(pmsstring->m_pmsint);
      msint* pmsint = static_cast<msint*>(msint::Clone(numericType, word).addr);
      
      numericType = pSecondoCatalog->NumericType(
                              NList(UniqueStringArray::BasicType()).listExpr());
      word = SetWord(pmsstring->m_pUniqueStringArray);
      UniqueStringArray* pUniqueStringArray =
        static_cast<UniqueStringArray*>(UniqueStringArray::Clone(numericType,
                                                                 word).addr);
      if(pmsint != 0 &&
         pUniqueStringArray != 0)
      {    
        word = SetWord(new msstring(pmsint, pUniqueStringArray,
                       pmsstring->m_minimum, pmsstring->m_maximum));
      }
    }
  }
  
  return word;
}

void msstring::Close(const ListExpr typeInfo,
                    Word& w)
{ 
  msstring* pmsstring = static_cast<msstring*>(w.addr);
  
  if(pmsstring != 0)
  {
    delete pmsstring;
    w = SetWord(Address(0));
  }
}

Word msstring::Create(const ListExpr typeInfo)
{
  Word w = SetWord(new msstring());
  return w;
}

void msstring::Delete(const ListExpr typeInfo,
                      Word& w)
{
  msstring* pmsstring = static_cast<msstring*>(w.addr);

  pmsstring->destroy();
  delete pmsstring;

  w.addr = 0;
}

TypeConstructor msstring::getTypeConstructor()
{ 
  TypeConstructor typeConstructorsstring(
    msstring::BasicType(), // type name function    
    msstring::Property,    // property function describing signature
    msstring::Out,         // out function
    msstring::In,          // in function
    0,                    // save to list function
    0,                    // restore from list function
    msstring::Create,      // create function
    msstring::Delete,      // delete function
    msstring::Open,        // open function
    msstring::Save,        // save function
    msstring::Close,       // close function
    msstring::Clone,       // clone function
    msstring::Cast,        // cast function
    msstring::SizeOfObj,   // sizeofobj function
    msstring::KindCheck);  // kindcheck function

  typeConstructorsstring.AssociateKind(Kind::SIMPLE());
  
  return typeConstructorsstring;
}

Word msstring::In(const ListExpr typeInfo,
                  const ListExpr instance,
                  const int errorPos,
                  ListExpr& errorInfo,
                  bool& correct)
{ 

  if(listutils::isSymbolUndefined(instance)){
    msstring* res = new msstring();
    res->setDefined(false);
    correct = true;
    return Word(res);
  }

  Word w = SetWord(Address(0));

  NList nlist(instance);
  NList sintList;

  grid3 grid;
  index_type sizes((int[]){0, 0, 0});
  string minimum = UNDEFINED_STRING;
  string maximum = UNDEFINED_STRING;
  UniqueStringArray* pUniqueStringArray = new UniqueStringArray(true);

  try
  {
    if (nlist.isAtom()) {
      throw util::parse_error
        ("Expected list as first element, got an atom.");
    }

    NList grid3list = nlist.elem(1);
    nlist.rest();

    if(grid3list.length() != 4)
    {
      throw util::parse_error("Type mismatch: list for grid3 is too short.");
    }

    if (!grid3list.isReal(1)
     || !grid3list.isReal(2)
     || !grid3list.isReal(3)
     || !grid3list.isReal(4))
    {
        throw util::parse_error(
                "Type mismatch: expected 4 reals as grid3 sublist.");
    }

    if (grid3list.elem(3).realval() <= 0.0
     || grid3list.elem(4).realval() <= 0.0)
    {
        throw util::parse_error(
          "Length and duration in grid3 must be larger than 0.");
    }

    grid = grid3(grid3list.elem(1).realval(),
                 grid3list.elem(2).realval(),
                 grid3list.elem(3).realval(),
                 DateTime(grid3list.elem(4).realval()));

    sintList.append(grid3list);

    if (!nlist.isEmpty()) {
        NList sizelist = nlist.elem(1);
        nlist.rest();
        if (sizelist.length() != 3) {
            throw util::parse_error(
              "Type mismatch: list for partial grid sizes is too short.");
        }
        if ( sizelist.isInt(1)
          && sizelist.isInt(2)
          && sizelist.isInt(3)
          && sizelist.elem(1).intval() > 0
          && sizelist.elem(2).intval() > 0
          && sizelist.elem(3).intval() > 0)
        {
            sintList.append(sizelist);

            sizes[0] = sizelist.elem(1).intval();
            sizes[1] = sizelist.elem(2).intval();
            sizes[2] = sizelist.elem(3).intval();
        } else {
            throw util::parse_error("Type mismatch: "
              "partial grid size must contain two positive integers.");
        }
    }

    while(!nlist.isEmpty())
    {
      NList sintPartialList;
      
      msint::index_type root;
      NList pagelist = nlist.first();
      nlist.rest();
      
      if(pagelist.length() != 4)
      {
          throw util::parse_error("Type mismatch: "
            "partial grid content must contain four elements.");
      }
      
      if(pagelist.isInt(1) &&
         pagelist.isInt(2) &&
         pagelist.isInt(3))
      {
        root[0] = pagelist.elem(1).intval();
        root[1] = pagelist.elem(2).intval();
        root[2] = pagelist.elem(3).intval();
      }
      
      else
      {
        throw util::parse_error("Type mismatch: "
          "partial grid content must start with three integers.");
      }
      
      sintPartialList.append(pagelist.elem(1));
      sintPartialList.append(pagelist.elem(2));
      sintPartialList.append(pagelist.elem(3));
      
      pagelist.rest();
      pagelist.rest();
      pagelist.rest();
      
      NList valuelist = pagelist.first();
      
      if(valuelist.length() != (Cardinal(sizes[0]) * Cardinal(sizes[1]) *
                                Cardinal(sizes[2])))
      {
        throw util::parse_error("Type mismatch: "
          "list for partial grid values is too short or too long.");
      }
      
      NList sintValueList;
      int i = 0;

      for(int row = 0; row < sizes[0]; ++row)
      {
        for(int column = 0; column < sizes[1]; ++column)
        {
          for(int time = 0; time < sizes[2]; ++time)
          {
            string value;
            i = i + 1;

            if(valuelist.elem(i).isSymbol(Symbol::UNDEFINED()))
            {
              sintValueList.append(valuelist.elem(i));
            }

            else if(mstype_helper<string>::check(valuelist.elem(i)))
            {
              value = mstype_helper<string>::parse(valuelist.elem(i));
              
              if(mstype_helper<string>::isUndefined(minimum) ||
                 value < minimum)
              {
                minimum = value;
              }
              
              if(mstype_helper<string>::isUndefined(maximum) ||
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
      }

      sintPartialList.append(sintValueList);
      sintList.append(sintPartialList);
    }
  }

  catch(util::parse_error& e)
  {
    correct = false;
    return w;
  }

  w = msint::In(typeInfo, sintList.listExpr(), errorPos, errorInfo, correct);

  if(correct == true)
  {
    msint* pmsint = static_cast<msint*>(w.addr);
    
    if(pmsint != 0)
    {
      int nMinimumIndex = pUniqueStringArray->GetUniqueStringIndex(minimum);
      int nMaximumIndex = pUniqueStringArray->GetUniqueStringIndex(maximum);

      msstring* pmsstring = new msstring(pmsint, pUniqueStringArray,
                                         nMinimumIndex, nMaximumIndex);
      
      correct = true;
      w = SetWord(pmsstring);
    }
  }
  
  return w;
}

bool msstring::KindCheck(ListExpr type,
                         ListExpr& errorInfo)
{ 
  bool bRetVal = false;
  
  if(nl != 0)
  {
    bRetVal = nl->IsEqual(type, msstring::BasicType());
  }
  
  return bRetVal;
}

bool msstring::Open(SmiRecord& valueRecord,
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
                            NList(msint::BasicType()).listExpr());
    Word w = SetWord(Address(0));
    bRetVal &= msint::Open(valueRecord, offset, numericType, w);
    msint* pmsint = static_cast<msint*>(w.addr);
    
    numericType = pSecondoCatalog->NumericType(NList(
                                    UniqueStringArray::BasicType()).listExpr());
    w = SetWord(Address(0));
    bRetVal &= UniqueStringArray::Open(valueRecord, offset, numericType, w);
    UniqueStringArray* pUniqueStringArray = static_cast<UniqueStringArray*>
                                                      (w.addr);
    
    if(pmsint != 0 &&
       pUniqueStringArray != 0)
    {
      msstring* pmsstring = new msstring(pmsint, pUniqueStringArray,
                                         minimum, maximum);
      
      if(pmsstring != 0)
      {
        value = SetWord(pmsstring);
      }
    }
  }
  
  return bRetVal;
}

bool msstring::Save(SmiRecord& valueRecord,
                    size_t& offset,
                    const ListExpr typeInfo,
                    Word& value)
{
  bool bRetVal = true;

  msstring* pmsstring = static_cast<msstring*>(value.addr);

  if(pmsstring != 0)
  {
    valueRecord.SetPos(offset);
    valueRecord.Write(pmsstring->m_minimum);
    valueRecord.Write(pmsstring->m_maximum);
    offset = valueRecord.GetPos();

    SecondoCatalog* pSecondoCatalog = SecondoSystem::GetCatalog();

    if(pSecondoCatalog != 0)
    {
      ListExpr numericType = pSecondoCatalog->NumericType(
                             NList(msint::BasicType()).listExpr());
      Word w = SetWord(pmsstring->m_pmsint);
      bRetVal &= msint::Save(valueRecord, offset, numericType, w);

      numericType = pSecondoCatalog->NumericType(
                              NList(UniqueStringArray::BasicType()).listExpr());
      w = SetWord(pmsstring->m_pUniqueStringArray);
      bRetVal &= UniqueStringArray::Save(valueRecord, offset, numericType, w);
    }
  }

  return bRetVal;
}

ListExpr msstring::Out(ListExpr typeInfo,
                       Word value)
{ 


  msstring* pmsstring = static_cast<msstring*>(value.addr);

  if(!pmsstring->isDefined()){
     return nl->SymbolAtom(Symbol::UNDEFINED());  
  }

  ListExpr pListExpr = 0;
  
  if(nl != 0)
  {  
    
    if(pmsstring != 0)
    {
      NList result;

      NList gridlist;
      const grid3& rGrid = pmsstring->m_pmsint->getGrid(); 
      gridlist.append(rGrid.getOriginX());
      gridlist.append(rGrid.getOriginY());
      gridlist.append(rGrid.getLength());
      gridlist.append(rGrid.getDuration().ToDouble());
      result.append(gridlist);

      NList tilesizelist;
      const msint::index_type& size = msint::riter_type::region_size;
      msint::index_type index;
      int element;

      msint::storage_type& storage = pmsstring->m_pmsint->getStorage();
      RasterRegion<3> bb = storage.bbox();
      msint::index_type sz = bb.Max - bb.Min;

      if (sz[0] <= size[0] && sz[1] <= size[1] && sz[2] <= size[2]) {
          tilesizelist.append(1);
          tilesizelist.append(1);
          tilesizelist.append(1);
          result.append(tilesizelist);

          for (msint::iter_type it = storage.begin(),
                                 e = storage.end();
               it != e; ++it)
          {
              element = *it;
              NList partiallist;
              partiallist.append(it.getIndex()[0]);
              partiallist.append(it.getIndex()[1]);
              partiallist.append(it.getIndex()[2]);

              NList valuelist;
              if(mstype_helper<int>::isUndefined(element))
              {
                valuelist.append(NList(Symbol::UNDEFINED()));
              }
              
              else
              {
                bool bOK = false;
                string uniqueString;
                
                bOK = pmsstring
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
          tilesizelist.append(size[2]);
          result.append(tilesizelist);

          msint::storage_type& rStorage = pmsstring->m_pmsint->getStorage();
          msint::index_type index;
          int element;

          for(msint::riter_type rit = rStorage.begin_regions(),
              re  = rStorage.end_regions();
              rit != re;
              ++rit)
          {
            NList partiallist;
            partiallist.append((*rit)[0]);
            partiallist.append((*rit)[1]);
            partiallist.append((*rit)[2]);

            NList valuelist;

            for(int time = 0; time < size[2]; ++time)
            {
              for(int row = 0; row < size[1]; ++row)
              {
                for(int column = 0; column < size[2]; ++column)
                {
                  index = *rit + msint::index_type((int[]){column, row, time});
                  element = rStorage[index];

                  if(mstype_helper<int>::isUndefined(element))
                  {
                    valuelist.append(NList(Symbol::UNDEFINED()));
                  }

                  else
                  {
                    bool bOK = false;
                    string uniqueString;

                    bOK = pmsstring
                            ->m_pUniqueStringArray
                            ->GetUniqueString(element, uniqueString);

                    if(bOK == true)
                    {
                      valuelist.append(NList(uniqueString, true, false));
                    }
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

ListExpr msstring::Property()
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
                "((x y l t) (szx szy szt) ((ix iy it (v*)))*)"),
              true));
      values.append(NList(
         std::string("((0 0 1 1) (2 1 2) ((1 1 1 (\"A\" \"B\" \"A\" \"C\"))))"),
         true));
      values.append(NList(std::string(""), true));

      property = NList(names, values);
  }

  return property.listExpr();
}

int msstring::SizeOfObj()
{ 
  return sizeof(msstring);
}

void swap(raster2::msstring& a, raster2::msstring&b)
{
    std::swap(a.m_bDelete, b.m_bDelete);
    std::swap(a.m_pmsint, b.m_pmsint);
    std::swap(a.m_pUniqueStringArray, b.m_pUniqueStringArray);
}

}
