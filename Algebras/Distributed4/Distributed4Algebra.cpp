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

  * Types in "Distributed4Types.cpp"[1] and corresponding class files
    ("TypeName.cpp"[1] and "TypeName.h)"[1]).

  * Operators in "Distributed4Operators.cpp"[1]

*/
#include "Algebra.h"
#include "Symbols.h"

namespace distributed4 {
/*
Types and operators are instantiated as objects in the accompanying files.
These objects are declared here as "extern"[1] so that the linker will find
them.

*/
  extern TypeConstructor dtableTC;
  extern TypeConstructor dpartitionTC;
  extern Operator addWorkerOp, inspectDArrayOp;
/*
"Distributed4Algebra"[1] is derived from "Algebra"[1], just like every other
algebra here. The default constructor is redefined to add this algebra's types
and operators.

It is not possible simply to create an object of type Algebra and add the types
and objects to that object because the methods "AddTypeConstructor"[1] and
"AddOperator"[1] are "protected"[1] in "Algebra"[1].

*/
  class Distributed4Algebra: public Algebra {
    public:
      Distributed4Algebra() {
/*
Type Constructors

*/
        AddTypeConstructor(&dtableTC);
        dtableTC.AssociateKind(Kind::SIMPLE());
        AddTypeConstructor(&dpartitionTC);
        dpartitionTC.AssociateKind(Kind::SIMPLE());
/*
Operators

*/
        AddOperator(&addWorkerOp);
        AddOperator(&inspectDArrayOp);
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
