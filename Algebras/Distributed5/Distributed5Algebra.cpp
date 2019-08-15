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

#include "Algebras/Distributed5/schedule.h"
#include "Algebras/Distributed5/Task.h"
#include "Algebras/Distributed5/dmapS.h"
#include "Algebras/Distributed5/ARRAYORTASKSFUNARG.h"
#include <boost/bind.hpp>
#include <boost/ref.hpp>

using namespace std;
using namespace distributed2;

namespace distributed5{

class Distributed5Algebra : public Algebra
{
public:
    Distributed5Algebra() : Algebra()
    {
        AddOperator(&dmapSOp);
        dmapSOp.SetUsesArgsInTypeMapping();
        AddOperator(&scheduleOp);
        AddTypeConstructor(&TaskTC);
        TaskTC.AssociateKind(Kind::SIMPLE());

        AddOperator(&ARRAYORTASKSFUNARG1Op);
    }
};

} // namespace distributed5

extern "C" Algebra *
InitializeDistributed5Algebra(NestedList *nlRef,
                              QueryProcessor *qpRef,
                              AlgebraManager *amRef)
{
    return new distributed5::Distributed5Algebra();
}
