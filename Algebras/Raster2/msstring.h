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

#ifndef RASTER2_MSSTRING_H
#define RASTER2_MSSTRING_H

#include <algorithm>

#include "util/noncopyable.h"
#include "msint.h"
#include "UniqueStringArray.h"
#include "sstring.h"

namespace raster2
{
  class msstring : util::noncopyable
  {
    public:
      typedef msint::index_type index_type;
      typedef msint::riter_type riter_type;
      typedef CcString wrapper_type;
      typedef std::string cell_type;
      typedef MString moving_type;
      typedef UString unit_type;
      typedef msstring this_type;
      typedef sstring spatial_type;
      typedef grid3 grid_type;

    private:
    /*
    constructors
  
    */
  
    msstring();
    
    public:
    /*
    constructors
  
    */
    
    msstring(msint* pmsint, UniqueStringArray* pUniqueStringArray,
             int minimum, int maximum);
  
    /*
    destructor
    
    */
    
    ~msstring();
    
    /*
    operators
  
    */
    
    
    
    /*
    functions
    
    */
    void destroy();
    string atlocation(double x, double y, double t) const;
    MString*  atlocation(double x, double y) const;
    void setatlocation(double x, double y, double t, const string& value);
    // isstring atinstant(Instant instant) const;
    // msstring atperiods(Periods periods) const;
    msstring* atperiods(const Periods& periods);
    msstring* atrange(const Rect& rRect);
    msstring* atrange(const Rect& rRect, const double& instFrom,
                      const double& instTo);
    msstring* atrange(const Rect& rRect, const Instant& start,
                      const Instant& end);
    void getDefinedPeriods(Periods& result) const;
    Rectangle<3> bbox() const;
    string getMinimum() const;
    string getMaximum() const;
    grid3 getGrid() const;
    void setGrid(const grid3& g);
    
    riter_type begin_regions();
    riter_type end_regions();
    riter_type iterate_regions(const index_type& from, const index_type& to);
    void set(const index_type& i, const std::string& value);
    std::string get(const index_type& i) const;

    static bool isUndefined(const std::string& t);
    static CcString wrap(const std::string& t);
    static std::string unwrap(const CcString& w);

    void setCacheSize(size_t size);
    void flushCache();

    void clear();
    bool isDefined() const;
    void setDefined(const bool _defined);


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
    functions

    */
    
    /*
    members
   
    */
    
    friend void swap(msstring&, msstring&);
    bool m_bDelete;
    msint* m_pmsint;
    UniqueStringArray* m_pUniqueStringArray;
    int m_minimum;
    int m_maximum;
  };
  
  template <>
  struct mstype_helper<string>
  {
    typedef msstring implementation_type;
    typedef MString moving_type;
    typedef CcString wrapper_type;
    static const char* name;
    
    static bool check(const NList& nl)
    {
      return nl.isString();
    }
    
    static string parse(const NList& nl)
    {
      return nl.str();
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
    static std::string unwrap(const CcString& rString)
    {
      if (rString.IsDefined()) {
        return rString.GetValue();
      } else {
        return getUndefined();
      }
    }
  };

  void swap(msstring&, msstring&);
}

namespace std {
    template<>
    inline void swap<raster2::msstring>
      (raster2::msstring& a, raster2::msstring&b)
    {
        raster2::swap(a, b);
    }
}

#endif // RASTER2_MSSTRING_H
