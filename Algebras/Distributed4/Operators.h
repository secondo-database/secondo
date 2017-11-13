/*
----
This file is part of SECONDO.

Copyright (C) 2017, Faculty of Mathematics and Computer Science, Database
Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

SECONDO is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
SECONDO; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [10] title: [{\Large \bf] [}]
//characters [1] tt: [\texttt{] [}]
//[secondo] [{\sc Secondo}]

[10] Definition of Operators in Algebra Distributed4

2017-11-13: Sebastian J. Bronner $<$sebastian@bronner.name$>$

Please see "Operators.cpp"[1] for documentation on the following.

*/
#ifndef ALGEBRAS_DISTRIBUTED4_OPERATORS_H
#define ALGEBRAS_DISTRIBUTED4_OPERATORS_H

#include "NestedList.h"
#include "AlgebraTypes.h"
#include "Operator.h"

namespace distributed4 {
/*
Type Mapping Functions

*/
  ListExpr lockTM(ListExpr);
  ListExpr unlockTM(ListExpr);
  ListExpr addhostTM(ListExpr);
  ListExpr removehostTM(ListExpr);
  ListExpr moveslotTM(ListExpr);
  ListExpr splitslotTM(ListExpr);
  ListExpr mergeslotsTM(ListExpr);
/*
Selection Functions

*/
  int removehostSel(ListExpr);
  int moveslotSel(ListExpr);
/*
Value Mapping Functions

*/
  int lockVM(Word*, Word&, int, Word&, Supplier);
  int trylockVM(Word*, Word&, int, Word&, Supplier);
  int unlockVM(Word*, Word&, int, Word&, Supplier);
  int addpeerVM(Word*, Word&, int, Word&, Supplier);
  int removepeerVM_Index(Word*, Word&, int, Word&, Supplier);
  int removepeerVM_HostPort(Word*, Word&, int, Word&, Supplier);
  int addworkerVM(Word*, Word&, int, Word&, Supplier);
  int removeworkerVM_Index(Word*, Word&, int, Word&, Supplier);
  int removeworkerVM_HostPort(Word*, Word&, int, Word&, Supplier);
  int moveslotVM_Index(Word*, Word&, int, Word&, Supplier);
  int moveslotVM_HostPort(Word*, Word&, int, Word&, Supplier);
  int splitslotVM(Word*, Word&, int, Word&, Supplier);
  int mergeslotsVM(Word*, Word&, int, Word&, Supplier);
/*
Operator Descriptions

*/
  struct lockInfo: OperatorInfo { lockInfo(); };
  struct trylockInfo: OperatorInfo { trylockInfo(); };
  struct unlockInfo: OperatorInfo { unlockInfo(); };
  struct addpeerInfo: OperatorInfo { addpeerInfo(); };
  struct removepeerInfo: OperatorInfo { removepeerInfo(); };
  struct addworkerInfo: OperatorInfo { addworkerInfo(); };
  struct removeworkerInfo: OperatorInfo { removeworkerInfo(); };
  struct moveslotInfo: OperatorInfo { moveslotInfo(); };
  struct splitslotInfo: OperatorInfo { splitslotInfo(); };
  struct mergeslotsInfo: OperatorInfo { mergeslotsInfo(); };
}

#endif
