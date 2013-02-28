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

#ifndef RASTER2_SSTRING_H
#define RASTER2_SSTRING_H

#include "util/noncopyable.h"
#include "sint.h"
#include "UniqueStringArray.h"

namespace raster2
{
  class sstring : util::noncopyable
  {
    public:
      typedef sint::index_type index_type;
      typedef sint::riter_type riter_type;
      typedef CcString wrapper_type;
      typedef std::string cell_type;
      typedef sstring this_type;
      typedef MString moving_type;
      typedef UString unit_type;
      typedef sint::storage_type storage_type;

    public:
    /*
    constructors
  
    */
    
    sstring();

    sstring(sint* psint, UniqueStringArray* pUniqueStringArray,
            int minimum, int maximum);
  
    /*
    destructor
    
    */
    
    virtual ~sstring();
    
    /*
    operators
  
    */
    
    /*
    functions
    
    */

    void clear();
    void setDefined(const bool _defined);
    bool isDefined() const;

    void destroy();
    sint::storage_type& getStorage();
    string atlocation(double x, double y) const;
    MString compose(const MPoint& m) const;
    sstring* atrange(const Rect& rRect) const;
    Rect bbox() const;
    string getMinimum() const;
    string getMaximum() const;
    grid2 getGrid() const;
    void setGrid(const grid2&);

    riter_type begin_regions() const;
    riter_type end_regions() const;
    riter_type iterate_regions(const index_type& from,
                               const index_type& to) const;
    void set(const index_type& i, const std::string& value);
    std::string get(const index_type& i) const;

    static bool isUndefined(const std::string& t);
    static CcString wrap(const std::string& t);
    static std::string unwrap(const CcString& w);

    void setCacheSize(size_t size);
    void flushCache();

    /*
    The following functions are used to integrate the ~sstring~ datatype
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
    members
   
    */

    friend void swap(sstring&, sstring&);
    bool m_bDelete;
    sint* m_psint;
    UniqueStringArray* m_pUniqueStringArray;
    int m_minimum;
    int m_maximum;
  };
  
  template <>
  struct stype_helper<string>
  {
    typedef sstring implementation_type;
    typedef CcString wrapper_type;
    typedef MString moving_type;
    typedef UString unit_type;
    static const char* name;
    
    static bool check(const NList& nl)
    {
      return nl.isString();
    }
    
    static string parse(const NList& nl)
    {
      return nl.str();
    }
    
    static NList print(const std::string& s)
    {
      return isUndefined(s) ? NList(Symbol::UNDEFINED()) : NList(s, true);
    }

    static bool isUndefined(const string& rString)
    {
      return rString == UNDEFINED_STRING;
    }
    
    static string getUndefined()
    {
      return UNDEFINED_STRING;
    }
    
    static std::string BasicType()
    {
      return CcString::BasicType();
    }
    
    static CcString wrap(const string& rString)
    {
      return CcString(!isUndefined(rString), rString);
    }

    static std::string unwrap(const CcString& i) {
        if (i.IsDefined()) {
          return i.GetValue();
        } else {
          return getUndefined();
        }
    }
  };

  void swap(sstring& a, sstring& b);
}

namespace std {
    template<>
    inline void swap<raster2::sstring>(raster2::sstring& a, raster2::sstring&b)
    {
        raster2::swap(a, b);
    }
}

#endif // RASTER2_SSTRING_H
