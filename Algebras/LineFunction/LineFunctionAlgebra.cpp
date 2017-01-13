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
//#include <iostream>
#include "Geoid.h"


/*
4 Operators

*/

Operator distanceWithGradient
(
    distanceWithGradientInfo(),
    distanceWithGradientFun,
    distanceWithGradientTypeMap
);

Operator lfResult
(
    lfResultInfo(),
    lfResultFun,
    lfResultTypeMap
);

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
    AddOperator( &distanceWithGradient);
    AddOperator( &lfResult);
  }
  ~LineFunctionAlgebra() {};
};

double DistanceWithHeight(const Point& pointSource, const Point& pointTarget,
 CcReal& weight, LReal& heightfunction, const Geoid* geoid )
{
  assert( pointSource.IsDefined() );
  assert( pointTarget.IsDefined() );
  assert( !geoid || geoid->IsDefined() );
  assert( weight.IsDefined() );

  const Geoid* mygeoid = new Geoid(Geoid::WGS1984);

    bool ok = false;
    double bearInitial = 0, bearFinal = 0;
//    std::cout << "LineFunctionA.cpp mygeoid: \t Address: " << mygeoid ;
//      std::cout << "\t Content: " << *mygeoid << "\n";
    double distance = pointSource.DistanceOrthodromePrecise(pointTarget,
        *mygeoid,ok,bearInitial,bearFinal);
    /*hier muesste das Gewicht pro Steigung uebergeben werden und die 
    HeightDifference Methode angepasst werden, wenn es diese Tabellenfunktion
     geben soll*/
    double height= HeightDifference(heightfunction);
    //distance und height sind hier in koordinateneinheiten, nicht in metern
    double distanceWithHeigt= distance + (height * weight.GetValue());
    return distanceWithHeigt;
}


double HeightDifference(LReal heightfunction)
{
    double heightDiff=0;

    if(!heightfunction.IsDefined()){
        return heightDiff;
    }
    heightfunction.Print(cout);
//    cout << heightfunction.GetNoComponents() << "\n";
    for (int i = 0; i < heightfunction.GetNoComponents(); i++) {
        LUReal* unit = new LUReal();
//        cout << "LFA.cpp Line " << __LINE__ << ", i: " << i << "\n";
        heightfunction.Get(i,*unit);
        //Steigung des Intervals
        double steigung=unit->m;

        double intervStart=unit->getLengthInterval().start.GetRealval();
        double intervEnd=unit->getLengthInterval().end.GetRealval();

        //Laenge des Intervals
        double intervalLength= intervEnd-intervStart;

        if(steigung>0){
            heightDiff=heightDiff + steigung*intervalLength;
        }
        //cout<< "m:" << steigung <<"\t istart:"<<intervStart
//        cout<<"\t iende:"<<intervEnd<<"\t heightDiff:"<<heightDiff;
    }
    //cout<<"\n------------------\n";

return heightDiff;
}


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
