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
#include "DisplayTTY.h"
#include "NestedList.h"
#include "SimpleAttrArray.h"
#include "Operators/And.h"
#include "Operators/ApplyPredicate.h"
#include "Operators/Attr.h"
#include "Operators/AttributeType.h"
#include "Operators/BlockCount.h"
#include "Operators/Consume.h"
#include "Operators/CConsume.h"
#include "Operators/Compare.h"
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
#include "Operators/TransformStream.h"
#include "QueryProcessor.h"
#include "TypeConstructors/CRelTC.h"
#include "TypeConstructors/DisplayLongInts.h"
#include "TypeConstructors/DisplayTBlock.h"
#include "TypeConstructors/DisplayCRel.h"
#include "TypeConstructors/GAttrArrayTC.h"
#include "TypeConstructors/GSpatialAttrArrayTC.h"
#include "TypeConstructors/IntsTC.h"
#include "TypeConstructors/LongIntsTC.h"
#include "TypeConstructors/TBlockTC.h"

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

      AddTypeConstructor(new GAttrArrayTC(), true);

      AddTypeConstructor(new GSpatialAttrArrayTC<1>(), true);

      AddTypeConstructor(new GSpatialAttrArrayTC<2>(), true);

      AddTypeConstructor(new GSpatialAttrArrayTC<3>(), true);

      AddTypeConstructor(new GSpatialAttrArrayTC<4>(), true);

      AddTypeConstructor(new GSpatialAttrArrayTC<8>(), true);

      AddTypeConstructor(new IntsTC(), true);

      AddTypeConstructor(new Ints2TC(), true);

      AddTypeConstructor(new LongIntsTC(), true);

      AddOperator(new And(), true);

      AddOperator(new ApplyPredicate(), true);

      AddOperator(new Attr(), true);

      AddOperator(new Operators::AttributeType(), true);

      AddOperator(new BlockCount(), true);

      AddOperator(new Consume(), true);

      AddOperator(new CConsume(), true);

      AddOperator(new Compare<CompareMode::Less>(), true);

      AddOperator(new Compare<CompareMode::LessOrEqual>(), true);

      AddOperator(new Compare<CompareMode::Equal>(), true);

      AddOperator(new Compare<CompareMode::NotEqual>(), true);

      AddOperator(new Compare<CompareMode::GreaterOrEqual>(), true);

      AddOperator(new Compare<CompareMode::Greater>(), true);

      AddOperator(new Count(), true);

      AddOperator(new Feed(), true);

      AddOperator(new FeedProject(), true);

      AddOperator(new Filter<false>(), true);

      AddOperator(new Filter<true>(), true);

      AddOperator(new ItHashJoin(), true);

      AddOperator(new ItSpatialJoin(), true);

      AddOperator(new Not(), true);

      AddOperator(new Or(), true);

      AddOperator(new Project(), true);

      AddOperator(new Rename(), true);

      AddOperator(new Repeat(), true);

      AddOperator(new TransformStream(), true);

      //AddOperator(new Test(), true);
    }
  };

  DisplayTTY &display = DisplayTTY::GetInstance();

  display.Insert(LongIntsTC::name, new DisplayLongInts());
  display.Insert(TBlockTC::name, new DisplayTBlock());
  display.Insert(CRelTC::name, new DisplayCRel());

  return new CRelAlgebra();
}