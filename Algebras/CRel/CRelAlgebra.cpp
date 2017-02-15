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

*/

#include "Algebra.h"
#include "NestedList.h"
#include "Operators/And.h"
#include "Operators/ApplyPredicate.h"
#include "Operators/Attr.h"
#include "Operators/BlockCount.h"
#include "Operators/BlockType.h"
#include "Operators/CConsume.h"
#include "Operators/Count.h"
#include "Operators/Feed.h"
#include "Operators/FeedProject.h"
#include "Operators/Filter.h"
#include "Operators/ItHashJoin.h"
#include "Operators/ItSpatialJoin.h"
#include "Operators/Not.h"
#include "Operators/Or.h"
#include "Operators/Project.h"
#include "Operators/Rename.h"
#include "Operators/Repeat.h"
//#include "Operators/Test.h"
#include "QueryProcessor.h"
#include "TypeConstructors/AttrArrayTC.h"
#include "TypeConstructors/TBlockTC.h"
#include "TypeConstructors/CRelTC.h"
#include "TypeConstructors/IndicesTC.h"

extern NestedList *nl;
extern QueryProcessor *qp;

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

extern "C" Algebra *InitializeCRelAlgebra(NestedList *nlRef,
                                          QueryProcessor *qpRef)
{
  class CRelAlgebra : public Algebra
  {
  public:
    CRelAlgebra() :
      Algebra()
    {
      AddTypeConstructor(new CRelTC(), true);

      AddTypeConstructor(new TBlockTC(), true);

      AddTypeConstructor(new AttrArrayTC(), true);

      AddTypeConstructor(new IndicesTC(), true);

      AddOperator(new And(), true);

      AddOperator(new ApplyPredicate(), true);

      AddOperator(new Attr(), true);

      AddOperator(new BlockCount(), true);

      AddOperator(new BlockType(), true);

      AddOperator(new CConsume(), true);

      AddOperator(new Count(), true);

      AddOperator(new Feed(), true);

      AddOperator(new FeedProject(), true);

      AddOperator(new Filter(), true);

      AddOperator(new ItHashJoin(), true);

      AddOperator(new ItSpatialJoin(), true);

      AddOperator(new Not(), true);

      AddOperator(new Or(), true);

      AddOperator(new Project(), true);

      AddOperator(new Rename(), true);

      AddOperator(new Repeat(), true);

      //AddOperator(new Test(), true);
    }
  };

  return new CRelAlgebra();
}