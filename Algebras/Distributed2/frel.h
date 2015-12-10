/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
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


//[$][\$]

*/

#ifndef FREL_H
#define FREL_H

#include <string>
#include "NestedList.h"
#include "SecondoSMI.h"

union Word;

namespace distributed2{

class frel{

public:
  frel();
  frel(const frel& src);
  explicit frel(const std::string& name);

  void set(const std::string& value);
 
  // secondo support 
  static inline std::string BasicType(){return "frel";}
  static bool checkType(const ListExpr list);
  static ListExpr Property();
  static Word In(const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct);
  static ListExpr Out(ListExpr typeInfo, Word value);
  static Word Create(const ListExpr typeInfo);
  static void Delete(const ListExpr typeInfo, Word& w);
  static bool Open(SmiRecord& valueRecord,
                   size_t& offset, const ListExpr typeInfo,
                   Word& value);
  static bool Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value);
  static void Close(const ListExpr typeInfo, Word& w);
  static Word Clone(const ListExpr typeInfo, const Word& v);
  static void* Cast(void* addr);
  static bool TypeCheck(ListExpr type, ListExpr& errorInfo);
  static int SizeOf();
  inline bool IsDefined()const{ return defined;}
  inline std::string GetValue()const{ return value;}
  inline void SetDefined(const bool _def){ defined = _def;}
  

private:
  explicit frel(const int dummy);
  std::string value;
  bool defined;

};

} // end of namespace distributed2

#endif
