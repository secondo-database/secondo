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

#ifndef RASTER2_UNIQUESTRINGARRAY_H
#define RASTER2_UNIQUESTRINGARRAY_H

#include "Attribute.h"
#include "../../Tools/Flob/Flob.h"
#include "../../Tools/Flob/DbArray.h"
#include "StringData.h"

using namespace std;

namespace raster2
{

class UniqueStringArray : public Attribute
{  
  public:
  /*
  constructors
  
  */
  
  UniqueStringArray();
  UniqueStringArray(bool bDefined);
  UniqueStringArray(const UniqueStringArray& rUniqueStringArray);
  
  /*
  destructor
  
  */
  
  virtual ~UniqueStringArray();
  
  /*
  operators
  
  */
  
  UniqueStringArray& operator = (const UniqueStringArray& rUniqueStringArray);
  
  /*
  functions
  
  */

  void clear(); 
  
  bool GetUniqueString(int nIndex, string& rString) const;
  list<string> GetUniqueStringArray() const;
  int GetUniqueStringIndex(const string& rString) const;
  int AddString(const string& rString);
  void Destroy();

  /*
  override functions from base class Attribute
  
  */
  
  virtual bool Adjacent(const Attribute* attrib) const;
  virtual Attribute* Clone() const;
  virtual int Compare(const Attribute* rhs) const;
  virtual void CopyFrom(const Attribute* right);
  virtual Flob* GetFLOB(const int i);
  virtual StorageType GetStorageType() const;
  virtual size_t HashValue() const;
  virtual int NumOfFLOBs() const;
  virtual ostream& Print( ostream& os ) const;
  virtual size_t Sizeof() const;
  
  /*
  The following functions are used to integrate the ~UniqueStringArray~
  datatype into secondo.

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
  
  bool IsUniqueString(const string& rString) const;
  
  /*
  members
   
  */
  
  DbArray<StringData> m_StringData;
  Flob m_StringFlob;
};

}

#endif // RASTER2_UNIQUESTRINGARRAY_H
 
