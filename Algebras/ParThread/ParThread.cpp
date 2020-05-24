/*
----
This file is part of SECONDO.

Copyright (C) 2019, 
University in Hagen, 
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

*/

#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "SecondoSystem.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Stream.h"

#include "ParOperator.h"

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

namespace parThread
{

/*
1.1 Operator ~par~

This operator devides the query plan in execution contexts which can be
evaluated independend with different threads.

*/

OperatorSpec parSpec(
    "stream(A) x int x string -> stream(A)",
    "_ par [ numInstances, partitionAttribute ]",
    "This operator devides the query plan in execution contexts which can be"
    "evaluated independently in different threads. The degree of parallelism"
    "for the par-operators related context is defined with the first parameter."
    "The second parameter is optional and used to feed a hash function for "
    "data partitioning. It must be an attribute of the incoming stream."
    "The following example is divided in three contexts. The first context "
    "contains just one operator 'count' and is per default executed serial. The"
    "second context uses up to three instances of the containing 'filter'-"
    "operator partitioned by the attribute value 'Vorwahl'. The last context"
    "is executed serial again and feeds the stream with tuples from the 'Orte'"
    "relation.",
    "query Orte feed par[1, Vorwahl] filter[.BevT > 500] par[3] count");

Operator parOp(
    "par",                  // name
    parSpec.getStr(),       // specification
    ParOperator::ParVM,     // value mapping
    Operator::SimpleSelect, // trivial selection function
    ParOperator::ParTM      // type mapping
);

/*
6 Creating the Algebra

*/

class ParThreadAlgebra : public Algebra
{
public:
  ParThreadAlgebra() : Algebra()
  {
    AddOperator(&parOp);
    parOp.enableInitFinishSupport();
    //the par operator uses memory, but the memory is allocated 
    //after static parallel optimization
  }

  ~ParThreadAlgebra(){};
};

} // namespace parThread

/*
7 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C" Algebra *
InitializeParThreadAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (new parThread::ParThreadAlgebra());
}
