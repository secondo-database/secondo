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

The type constructors and operators of this algebra are assigned in this file.
They are implemented in other files as follows:

  * Types in "Distributed4Types.cpp"[1] and corresponding class files
    ("TypeName.h/cpp/ipp"[1]).

  * Operators in "Distributed4Operators.cpp"[1]

*/
#include "Algebra.h"
#include "Symbols.h"

namespace distributed4 {
  extern TypeConstructor dstructTC;
  extern Operator addWorkerOp, inspectDArrayOp;

  class Distributed4Algebra: public Algebra {
    public:
      Distributed4Algebra() {
        AddTypeConstructor(&dstructTC);
        dstructTC.AssociateKind(Kind::SIMPLE());

        AddOperator(&addWorkerOp);
        AddOperator(&inspectDArrayOp);
      }
  };
}

extern "C"
Algebra* InitializeDistributed4Algebra(NestedList*, QueryProcessor*) {
  return new distributed4::Distributed4Algebra;
}
