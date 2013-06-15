/*
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

*/

#ifndef TILEALGEBRA_TSTRING_H
#define TILEALGEBRA_TSTRING_H

#include <string>
#include "t.h"
#include "tProperties.h"
#include "tint.h"
#include "../Properties/Propertiesstring.h"
#include "../UniqueStringArray/UniqueStringArray.h"
#include "RectangleAlgebra.h"
#include "../grid/tgrid.h"

namespace TileAlgebra
{

/*
declaration of class tstring

*/

class tstring : public tint
{
  /*
  constructors

  */

  protected:

  tstring();

  public:

  tstring(bool bDefined);
  tstring(const tint& rtint, const UniqueStringArray& rUniqueStringArray);
  tstring(const tstring& rtstring);

  /*
  destructor

  */

  virtual ~tstring();

  /*
  operators

  */

  tstring& operator=(const tstring& rtstring);

  /*
  TileAlgebra operator methods

  */

  void atlocation(const double& rX,
                  const double& rY,
                  CcString& rValue) const;
  void atlocation(const double& rX,
                  const double& rY,
                  const double& rInstant,
                  CcString& rValue) const;
  std::string minimum() const;
  std::string maximum() const;

  /*
  internal methods

  */

  std::string GetValue(const Index<2>& rIndex) const;
  bool SetValue(const Index<2>& rIndex, const std::string& rValue);

  /*
  override functions from base class tint

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
  The following functions are used to integrate the ~tstring~
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

  UniqueStringArray m_UniqueStringArray;
};

/*
declaration of template class tProperties<std::string>

*/

template <>
class tProperties<std::string>
{
  public:

  typedef tstring PropertiesType;
  typedef Properties<std::string> TypeProperties;
  typedef CcString atlocationType;
  typedef Rectangle<2> bboxType;
  typedef tgrid gridType;
  static int GetXDimensionSize();
  static int GetYDimensionSize();
  static int GetFlobElements();
  static SmiSize GetFlobSize();
  static std::string GetTypeName();
};

}

#endif // TILEALGEBRA_TSTRING_H
