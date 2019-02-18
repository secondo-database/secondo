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



1 The Pointcloud2Algebra class

Our Pointcloud2-Algebra has only one Type and a bunch of Operators.
The type consists of three parts:
	a) a SMIRecordFile that contains the actual set of points
	b) a RTree for indexing the record file
	c) a Relation to store optional properties of point records

The operators contained allow for basic analysis of pointclouds:
they reduce errors, partition clouds and identify geometric objects.
Of course there are also operators that allow for things like these:
get basic information on properties of a cloud, to import clouds,
to feed them into streams, and collect them again,
to merge them and to restrict them to parts.

*/
#include "Pointcloud2Algebra.h"

#include "AlgebraManager.h"
#include "ListUtils.h"

#include "basicOperators/opBasicOperators.h"
#include "basicOperators/opMerge.h"
#include "basicOperators/opStreamingOperators.h"
#include "basicOperators/opRestrictOperators.h"

#include "importOperators/opImportOperators.h"

#include "utilityOperators/opParamOperator.h"
#include "utilityOperators/opCreatePc2Shapes.h"
#include "utility/ShapeGenerator.h"

#include "analyzeOperators/opAnalyzeRaster.h"
#include "analyzeOperators/opClusterPc2.h"
#include "analyzeOperators/opProject.h"
#include "analyzeOperators/opAnalyzeGeom.h"

#include "opPc2RasterTestOperators.h"


extern NestedList *nl;
extern QueryProcessor *qp;

namespace pointcloud2 {

    class Pointcloud2Algebra : public Algebra {
/*
1.1 Operators for the algebra.
The shared-pointers are alive as long as the algebra-object lives in Secondo.
(C++11-feature)

1.1.1 Basic Operators

*/
    std::shared_ptr<Operator> opBbox = op_bbox().getOperator();
    std::shared_ptr<Operator> opBbox2d = op_bbox2d().getOperator();
    std::shared_ptr<Operator> opSize = op_size().getOperator();
    std::shared_ptr<Operator> opMinZ = op_minz().getOperator();
    std::shared_ptr<Operator> opMaxZ = op_maxz().getOperator();
    std::shared_ptr<Operator> opMerge = op_merge().getOperator();
    std::shared_ptr<Operator> opFeed = op_feed().getOperator();
    std::shared_ptr<Operator> opCollectPc2 = OPCollectPc2().getOperator();
/*
1.1.2 Import Operators

*/
    std::shared_ptr<Operator> opImportXyz = op_importxyz().getOperator();
    std::shared_ptr<Operator> opImportPc2FromLas =
                                    op_importPc2FromLas().getOperator();
    std::shared_ptr<Operator> opImportPc2FromStl =
                                    op_importPc2FromStl().getOperator();

/*
1.1.3 Restrict Operators

*/
    std::shared_ptr<Operator> opRestrict = op_restrict().getOperator();
    std::shared_ptr<Operator> opRestrictXY = op_restrictXY().getOperator();
    std::shared_ptr<Operator> opRestrictZ = op_restrictZ().getOperator();
    std::shared_ptr<Operator> opRestrictAttr = op_restrictAttr()
                        .getOperator();
    std::shared_ptr<Operator> opRestrictRnd = op_restrictRnd()
                        .getOperator();


/*
1.1.4 Analyze Operators

*/
    std::shared_ptr<Operator> opPc2SetParam = op_setParam().getOperator();
    std::shared_ptr<Operator> opPc2GetParams = op_getParams().getOperator();
    std::shared_ptr<Operator> opAnalyzeRaster =
           op_analyzeRaster().getOperator();
    std::shared_ptr<Operator> opPc2RasterTest =
        op_Pc2RasterTest().getOperator();
    std::shared_ptr<Operator> opProjectUTM = op_projectUTM().getOperator();
    std::shared_ptr<Operator> opProjectWGS84 = op_projectWGS84().getOperator();
    std::shared_ptr<Operator> opUTMZone = op_UTMZone().getOperator();
    std::shared_ptr<Operator> opUTMSouth = op_UTMSouth().getOperator();
    std::shared_ptr<Operator> opClusterPc2 = op_cluster().getOperator();
    std::shared_ptr<Operator> opRemoveNoise = op_removeNoise().getOperator();
    std::shared_ptr<Operator> opCreatePc2Shapes =
            op_createPc2Shapes().getOperator();
    std::shared_ptr<Operator> opAnalyzeGeom =
            op_analyzeGeom().getOperator();

    public:
        Pointcloud2Algebra() : Algebra() {
            AddTypeConstructor( &Pointcloud2::typeConstructor );
            
            // TODO: nach welchem Prinzip wrd hier SetUsesArgsInTypeMapping()
            // verwendet?

            // Register operator pointers.

            // basic operators
            AddOperator( opBbox.get() );
            AddOperator( opBbox2d.get() );
            AddOperator( opSize.get() );
            AddOperator( opMinZ.get() );
            AddOperator( opMaxZ.get() );
            AddOperator( opMerge.get() );

            // stream operators
            AddOperator( opFeed.get() );
            AddOperator( opCollectPc2.get() );

            // import operators
            opImportXyz.get()->SetUsesArgsInTypeMapping();
            AddOperator( opImportXyz.get() );

            opImportPc2FromLas.get()->SetUsesArgsInTypeMapping();
            AddOperator( opImportPc2FromLas.get() );

            opImportPc2FromStl.get()->SetUsesArgsInTypeMapping();
            AddOperator( opImportPc2FromStl.get() );

            // restrict operators
            AddOperator(opRestrict.get() );

            AddOperator(opRestrictXY.get() );

            AddOperator(opRestrictZ.get() );

            AddOperator(opRestrictAttr.get() );

            AddOperator(opRestrictRnd.get() );

            opPc2SetParam.get()->SetUsesArgsInTypeMapping();
            AddOperator(opPc2SetParam.get() );

            AddOperator(opPc2GetParams.get() );

            opAnalyzeRaster.get()->SetUsesMemory();
            AddOperator(opAnalyzeRaster.get() );

            AddOperator(opPc2RasterTest.get() );

            AddOperator(opProjectUTM.get() );

            AddOperator(opProjectWGS84.get() );

            AddOperator(opUTMZone.get() );

            AddOperator(opUTMSouth.get() );

            opClusterPc2.get()->SetUsesMemory();
            AddOperator(opClusterPc2.get() );

            opRemoveNoise.get()->SetUsesMemory();
            AddOperator(opRemoveNoise.get() );

            if (SHAPE_GEN_INSERT_ORDER != ShapeGenOrder::byShape)
                opCreatePc2Shapes.get()->SetUsesMemory();
            AddOperator(opCreatePc2Shapes.get() );

            opAnalyzeGeom.get()->SetUsesMemory();
            AddOperator(opAnalyzeGeom.get() );
        }
    };
}

extern "C"
Algebra* InitializePointcloud2Algebra(NestedList* nlRef, QueryProcessor* qpRef){
    return new pointcloud2::Pointcloud2Algebra;
}
