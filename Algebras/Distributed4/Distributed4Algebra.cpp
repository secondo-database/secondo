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

[10] Algebra Distributed4

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

The type constructors and operators of this algebra are assigned to the algebra
in this file. However, they are implemented in other files as follows:

  * Types in corresponding class files ("TypeName.cpp"[1] and
    "TypeName.h)"[1])

  * Operators in "Operators.cpp"[1] and "Operators.h"[1]

*/
#include "DPartition.h"
#include "Operators.h"
#include "Algebra.h"
#include "Symbols.h"

namespace distributed4 {
/*
"Distributed4Algebra"[1] is derived from "Algebra"[1], just like every other
algebra here. The default constructor is redefined to add this algebra's types
and operators.

It is not possible simply to create an instance of the class "Algebra"[1] and
add the type constructors and operators to that object because the member
functions "AddTypeConstructor"[1] and "AddOperator"[1] are "protected"[1] in
"Algebra"[1].

*/
  class Distributed4Algebra: public Algebra {
/*
Define Type Constructors

*/
    protected:
      TypeConstructor tc{DPartition::Info{}, DPartition::Functions{}};

    public:
      Distributed4Algebra() {
/*
Add Type Constructors

*/
        tc.AssociateKind(Kind::SIMPLE());
        AddTypeConstructor(&tc);
/*
Add Operators

The "Operator"[1] class takes care of putting together operator instances given
values and functions for an operator. The constructor expects the following
elements: name of operator, specification of operator, value mapping function
(for overloaded operators, the number of available value mapping functions and
an array of those functions are supplied instead), selection function (simple
operators use "Operator::SimpleSelect"[1]), type mapping function.

The "OperatorSpec"[1] class formats human-readable specification for the
operators in a consistent way. The constructor expects four or five strings in
the following order: signature, syntax, meaning, example, remark (optional).

*/
        AddOperator(lockInfo(), lockVM, lockTM);
        AddOperator(trylockInfo(), trylockVM, lockTM);
        AddOperator(unlockInfo(), unlockVM, unlockTM);
        AddOperator(addworkerInfo(), addworkerVM, addworkerTM);
        AddOperator(removeworkerInfo(), removeworkerVM, removeworkerTM);
        AddOperator(moveslotInfo(), moveslotVM, moveslotTM);
      }
  };
}
/*
This function is called by [secondo] during initialization. The algebra is
instantiated and passed to [secondo].

*/
extern "C"
Algebra* InitializeDistributed4Algebra(NestedList*, QueryProcessor*) {
  return new distributed4::Distributed4Algebra;
}
