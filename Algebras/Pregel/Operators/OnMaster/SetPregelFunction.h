/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the class of operator ~setPregelFunction~

November 2018, J. Mende


[TOC]

1 Overview

This header file contains definitions of type mapping, vallue mapping and the operator specification.

2 Defines and includes

*/

#ifndef SECONDO_SETPREGELFUNCTION_H
#define SECONDO_SETPREGELFUNCTION_H

#include <NestedList.h>
#include <AlgebraTypes.h>
#include <Operator.h>

namespace pregel {
 /*
  * 3 Class SetPregelFunction
  *
  * */
 class SetPregelFunction {
 public:
  /*
   * 3.1 Type mapping
   *
   * */
  static ListExpr typeMapping(ListExpr args);

  /*
   * 3.2 Value mapping
   *
   * */
  static int valueMapping(Word *args,
                          Word &result,
                          int message,
                          Word &local,
                          void *s);

  /*
   * 3.3 Operator specification
   *
   * */
  static OperatorSpec operatorSpec;

  /*
   * 3.4 Operator instance
   *
   * */
  static Operator setPregelFunction;
 private:
  /*
   * 3.5 Auxiliary functions
   *
   * */
  static bool remoteQueryCall(std::string &function, int index);

  static int findAttribute(const std::string &attributeName,
                           const ListExpr tupleType) noexcept(false);
 };
}


#endif //SECONDO_SETPREGELFUNCTION_H
