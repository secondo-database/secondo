/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



0 Pointcloud2 createPc2Shapes operator

Creates a Pointcloud2 with planes, spheres, cylinders and cones.
Parameters are:

createPc2Shapes(planeCount, sphereCount, cylinderCount, coneCount,
pointDist, shapeSizeMin, shapeSizeMax, spaceSize,
diffusion, noisePointCount, rndSeed)

  * planeCount etc. is the number of planes etc. to be created by the
    operator

  * pointDist is the regular distance between two points (e.g. 0.1),

  * shapeSizeMin/Max is the extent of the shape in any dimension
    (e.g. setting Min = 5.0 and Max = 8.0 could create a plane of size
    5.5 x 7.4 with 55 * 74 points if pointDist is 0.1)

  * spaceSize is the extent of the Pointcloud2 bbox in every dimension.
    The shapes will be created at random locations within that space
    (e.g. spaceSize = 30.0 for a bbox [0.0 - 30.0, 0.0 - 30.0, 0.0 - 30.0])

  * diffusion will be added to every point in any dimension (e.g. 0.02 to add
    random numbers in [-0.02, 0.02]

  * noisePointCount is the total number of random points added as noise
    anywhere in the given space

  * rndSeed is the seed for the random number generator. With rndSeed = 0,
    the current system time is used as a seed; with positive values, the
    given value itself is used to generate reproducible results.

Example: createPc2Shapes(2, 2, 2, 2, 0.1, 2.0, 3.0, 20.0, 0.01, 50, 1); will
create 2 planes, 2 spheres, 2 cylinders, and 2 cones, each of size 2.0 - 3.0
in any dimension at (reproducible) random locations in the space
[0.0 - 20.0, 0.0 - 20.0, 0.0 - 20.0], diffuse each point by a random number
in [-0.01, 0.01], and then add 50 points of noise at random positions within
that space. Since rndSeed is positive, the exact same objects will be created
if the function is called again with the same parameters.

*/
#include "opCreatePc2Shapes.h"

#include <cmath>
#include <math.h>
#include <iostream>
#include <algorithm>

using namespace std;
using namespace pointcloud2;

extern NestedList *nl;

ListExpr op_createPc2Shapes::createPc2ShapesTM(ListExpr args) {
    const string err("createPc2Shapes(int planeCount, int sphereCount, "
            "int cylinderCount, int coneCount, real pointDist, "
            "real shapeSizeMin, real shapeSizeMax, real spaceSize, "
            "real diffusion, int noisePointCount, int rndSeed) expected");

    const int expectedArgsLength = 11;
    int argsLength = nl->ListLength(args);
    if (argsLength != expectedArgsLength) {
        stringstream st;
        st << "wrong number of arguments: got " << argsLength
           << " but expected " << expectedArgsLength << ": " << err;
        return listutils::typeError(st.str());
    }

    ListExpr rest = args;
    stringstream wrongArgTypes;
    wrongArgTypes << "wrong argument type in argument ";
    size_t wrongCount = 0;
    for (int i = 1; i <= expectedArgsLength; ++i) {
        ListExpr arg = nl->First(rest);
        bool expectedInt = (i <= 4 || i >= 10);
        bool expectedReal = !expectedInt;
        if (       (expectedInt && !CcInt::checkType(arg))
                || (expectedReal && !CcReal::checkType(arg))) {
            if (wrongCount > 0)
                wrongArgTypes << ", ";
            ++wrongCount;
            wrongArgTypes << i;
            if (expectedInt)
                wrongArgTypes << " (int expected)";
            else if (expectedReal)
                wrongArgTypes << " (real expected)";
        }
        rest = nl->Rest(rest);
    }
    if (wrongCount > 0) {
        wrongArgTypes << ": " << err;
        return listutils::typeError(wrongArgTypes.str());
    }

    ListExpr result = Pointcloud2::cloudTypeWithParams(
            nl->SymbolAtom(
            Referencesystem::toString(Referencesystem::Type::EUCLID)));

    return result;
}

int op_createPc2Shapes::createPc2ShapesVM(Word* args, Word& result,
        int message, Word& local, Supplier s) {

    // DEBUG
    // const size_t max_memory = qp->GetMemorySize(s); // test machine: 512 MB
    // cout << "Memory Size for Operator: " << max_memory << " MB" << endl;

    // get args
    int planeCount = (static_cast<CcInt*>(args[0].addr))->GetIntval();
    int sphereCount = (static_cast<CcInt*>(args[1].addr))->GetIntval();
    int cylinderCount = (static_cast<CcInt*>(args[2].addr))->GetIntval();
    int coneCount = (static_cast<CcInt*>(args[3].addr))->GetIntval();
    double pointDist = (static_cast<CcReal*>(args[4].addr))->GetRealval();
    double shapeSizeMin = (static_cast<CcReal*>(args[5].addr))->GetRealval();
    double shapeSizeMax = (static_cast<CcReal*>(args[6].addr))->GetRealval();
    double spaceSize = (static_cast<CcReal*>(args[7].addr))->GetRealval();
    double diffusion = (static_cast<CcReal*>(args[8].addr))->GetRealval();
    int noisePointCount = (static_cast<CcInt*>(args[9].addr))->GetIntval();
    int rndSeed = (static_cast<CcInt*>(args[10].addr))->GetIntval();

    // ensure values are positive or 0
    planeCount = MAX(planeCount , 0);
    sphereCount = MAX(sphereCount, 0);
    cylinderCount = MAX(cylinderCount, 0);
    coneCount = MAX(coneCount, 0);
    pointDist = std::abs(pointDist);
    shapeSizeMin = std::abs(shapeSizeMin);
    shapeSizeMax = std::abs(shapeSizeMax);
    shapeSizeMax = MAX(shapeSizeMin, shapeSizeMax);
    spaceSize = std::abs(spaceSize);
    diffusion = std::abs(diffusion);
    noisePointCount = MAX(noisePointCount, 0);
    rndSeed = (rndSeed < 0) ? -rndSeed : rndSeed;

    // get result Pointcloud2
    result = qp->ResultStorage(s);
    Pointcloud2* pc2 = static_cast<Pointcloud2*>(result.addr);

    ShapeGenerator gen { pc2,
        unsigned(planeCount), unsigned(sphereCount),
        unsigned(cylinderCount), unsigned(coneCount),
        pointDist, shapeSizeMin, shapeSizeMax, spaceSize,
        diffusion, unsigned(noisePointCount), unsigned(rndSeed) };

    gen.generate();

    return 0;
}

std::string op_createPc2Shapes::getOperatorSpec(){
    return OperatorSpec(
        " int^4 x real^5 x int -> pointcloud2",
        " createPc2Shapes(_, _, _, _, _, _, _, _, _, _) ",
        " Creates a Pointcloud2 with planes, spheres etc. ",
        " query createPc2Shapes(1, 1, 1, 1, 0.3, 5.0, 8.0, 30.0, 0.02, 50, 0)"
    ).getStr();
}

std::shared_ptr<Operator> op_createPc2Shapes::getOperator(){
    return std::make_shared<Operator>("createPc2Shapes",
                                    getOperatorSpec(),
                                    &op_createPc2Shapes::createPc2ShapesVM,
                                    Operator::SimpleSelect,
                                    &op_createPc2Shapes::createPc2ShapesTM);
}


