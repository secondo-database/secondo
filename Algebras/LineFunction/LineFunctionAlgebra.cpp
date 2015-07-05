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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[bl] [\\]

[1] Implementation of Module

April 2015 Rene Steinbrueck
[TOC]

1 Overview

2 Inclusion of the Header File

*/

#include "LineFunctionAlgebra.h"


/*
4 Operators

*/

Operator lcompose
(
    lcomposeInfo(),
    lcomposeFuns,
    lcomposeSelectFun,
    lcomposeTypeMap
);

Operator heightatposition
(
    heightatpositionInfo(),
    heightatpositionFuns,
    heightatpositionSelectFun,
    heightatpositionTypeMap
);

Operator lfdistance
(
    lfdistanceInfo(),
    lfdistanceFuns,
    lfdistanceSelectFun,
    lfdistanceTypeMap
);

Operator lfdistanceparam
(
    lfdistanceparamInfo(),
    lfdistanceparamFuns,
    lfdistanceparamSelectFun,
    lfdistanceparamTypeMap
);

/*
5 Creating the Algebra

*/
class LineFunctionAlgebra : public Algebra
{
 public:
  LineFunctionAlgebra() : Algebra()
  {
    AddTypeConstructor( &lunitbool );
    AddTypeConstructor( &lunitint );
    AddTypeConstructor( &lunitstring );
    AddTypeConstructor( &lunitreal );

    AddTypeConstructor( &lengthbool );
    AddTypeConstructor( &lengthint );
    AddTypeConstructor( &lengthstring );
    AddTypeConstructor( &lengthreal );

    lunitbool.AssociateKind( Kind::DATA() );
    lunitint.AssociateKind( Kind::DATA() );
    lunitstring.AssociateKind( Kind::DATA() );
    lunitreal.AssociateKind( Kind::DATA() );

    lengthbool.AssociateKind( Kind::DATA() );
    lengthint.AssociateKind( Kind::DATA() );
    lengthstring.AssociateKind( Kind::DATA() );
    lengthreal.AssociateKind( Kind::DATA() );

    AddOperator( &lcompose );
    AddOperator( &heightatposition );
    AddOperator( &lfdistance );
    AddOperator( &lfdistanceparam);
  }
  ~LineFunctionAlgebra() {};
};

/*
6 Initialization

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

extern "C"
Algebra*
InitializeLineFunctionAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new LineFunctionAlgebra());
}
