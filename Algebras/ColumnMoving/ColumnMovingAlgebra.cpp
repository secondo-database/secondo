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
#include "DisplayAttrArray.h"

#include "Types.h"
#include "Operators.h"
#include "ExtendOperator.h"

extern NestedList *nl;
extern QueryProcessor *qp;

using namespace ColumnMovingAlgebra;

/*
Registrierung der Typkonstruktoren und Operatoren

*/


extern "C" Algebra *InitializeColumnMovingAlgebra(NestedList *nlRef,
                                          QueryProcessor *qpRef)
{
  class CustomAlgebra : public Algebra
  {
  public:
    CustomAlgebra() :
      Algebra()
    {
      AddTypeConstructor(new IIntsType::TC(), true);
      AddTypeConstructor(new MIntsType::TC(), true);
      AddTypeConstructor(new IPointsType::TC(), true);
      AddTypeConstructor(new MPointsType::TC(), true);
      AddTypeConstructor(new IRegionsType::TC(), true);
      AddTypeConstructor(new MRegionsType::TC(), true);
      AddTypeConstructor(new IRealsType::TC(), true);
      AddTypeConstructor(new MRealsType::TC(), true);
      AddTypeConstructor(new IBoolsType::TC(), true);
      AddTypeConstructor(new MBoolsType::TC(), true);
      AddTypeConstructor(new IStringsType::TC(), true);
      AddTypeConstructor(new MStringsType::TC(), true);
      AddOperator(new PresentOperator(), true);
      AddOperator(new AtInstantOperator(), true);
      AddOperator(new AtPeriodsOperator(), true);
      AddOperator(new PassesOperator(), true);
      AddOperator(new AtOperator(), true);
      AddOperator(new AddRandomOperator(), true);
      AddOperator(new IndexOperator(), true);
      AddOperator(new IntersectionOperator(), true);
      AddOperator(new InsideOperator(), true);

      AddOperator(new CRelAlgebra::Operators::ExtendOperator(), true);
    }
  };

  DisplayTTY &display = DisplayTTY::GetInstance();

  display.Insert(IIntsType::TC::name, new CRelAlgebra::DisplayAttrArray());
  display.Insert(MIntsType::TC::name, new CRelAlgebra::DisplayAttrArray());
  display.Insert(IPointsType::TC::name, new CRelAlgebra::DisplayAttrArray());
  display.Insert(MPointsType::TC::name, new CRelAlgebra::DisplayAttrArray());
  display.Insert(IRegionsType::TC::name, new CRelAlgebra::DisplayAttrArray());
  display.Insert(MRegionsType::TC::name, new CRelAlgebra::DisplayAttrArray());
  display.Insert(IRealsType::TC::name, new CRelAlgebra::DisplayAttrArray());
  display.Insert(MRealsType::TC::name, new CRelAlgebra::DisplayAttrArray());
  display.Insert(IBoolsType::TC::name, new CRelAlgebra::DisplayAttrArray());
  display.Insert(MBoolsType::TC::name, new CRelAlgebra::DisplayAttrArray());
  display.Insert(IStringsType::TC::name, new CRelAlgebra::DisplayAttrArray());
  display.Insert(MStringsType::TC::name, new CRelAlgebra::DisplayAttrArray());

  return new CustomAlgebra();
}
