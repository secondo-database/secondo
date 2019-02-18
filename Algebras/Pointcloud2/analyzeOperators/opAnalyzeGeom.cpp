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



1 Analyze Geometries Operator

*/
#include "opAnalyzeGeom.h"

#include "../utility/DbScan.h"
#include "../utility/Timer.h"
#include "../utility/MathUtils.h"

#include "GpPlane.h"
#include "GpSphere.h"
#include "GpCylinder.h"
#include "GpCone.h"

using namespace pointcloud2;

extern NestedList *nl;

bool op_analyzeGeom::REPORT_TO_CONSOLE = true;

/*
1.1 Type Mapping

*/
ListExpr op_analyzeGeom::analyzeGeomTM(ListExpr args) {
    if (!(nl->HasLength(args, 1) && Pointcloud2::TypeCheck(nl->First(args)))) {
        return listutils::typeError("wrong input");
    }

    // get the attribute indices of ObjID and CatID, if existing.
    // Ensure these attributes are of type CcInt
    ListExpr cloudType = nl->First(args);
    int attrIndexObjID = -1;
    int attrIndexCatID = -1;
    bool hasTuples = Pointcloud2::isTupleCloud(cloudType);
    ListExpr attrList;
    if (hasTuples) {
        attrList = nl->Second(Pointcloud2::getTupleType(cloudType));
        ListExpr type;
        attrIndexObjID = listutils::findAttribute(attrList, OBJ_ID, type) - 1;
        if (attrIndexObjID >= 0 &&
                !listutils::isSymbol(type, CcInt::BasicType())) {
            return listutils::typeError("wrong type of attribute ObjID");
        }
        attrIndexCatID = listutils::findAttribute(attrList, CAT_ID, type) - 1;
        if (attrIndexCatID >= 0 &&
                !listutils::isSymbol(type, CcInt::BasicType())) {
            return listutils::typeError("wrong type of attribute CatID");
        }
        // ensure that either both attributes are present or both are missing
        if ((attrIndexObjID >= 0) != (attrIndexCatID >= 0)) {
            return listutils::typeError("one of the attributes "
                    "ObjID and CatID is defined, the other is missing.");
        }
    }

    // remember whether the source cloud has ObjIDs from a previous analysis.
    // If so, we will have to read through the tuples in Value Mapping to
    // exclude points which have already been assigned to an object.
    bool hasOldObjIDs = (attrIndexObjID >= 0);

    // append the attributes ObjID and CatID to the result type, if missing
    ListExpr newCloudType;
    if (!hasOldObjIDs) {
        // calculate the appendage for the missing attributes
        ListExpr attrObjID = nl->TwoElemList(
                nl->SymbolAtom(OBJ_ID), listutils::basicSymbol<CcInt>());
        ListExpr attrCatID = nl->TwoElemList(
                nl->SymbolAtom(CAT_ID), listutils::basicSymbol<CcInt>());
        ListExpr appendage = nl->TwoElemList(attrObjID, attrCatID);

        // determine the new type with the attributes
        newCloudType = Pointcloud2::appendAttributesToCloud(cloudType,
                appendage);

        // determine the attribute indices in the new cloud type
        attrIndexObjID = hasTuples ? nl->ListLength(attrList) : 0;
        attrIndexCatID = attrIndexObjID + 1;
    } else {
        newCloudType = cloudType;
        // attrIndexObj/CatId will be identical in the new cloud
    }

    // use the append mechanism (see GuideAlgebra.pdf, p.48, section 11.1)
    // to inform Value Mapping about hasOldObjIDs and attrIndexes
    return nl->ThreeElemList(
            nl->SymbolAtom(Symbols::APPEND()),
            nl->ThreeElemList(
                    nl->BoolAtom(hasOldObjIDs),
                    nl->IntAtom(attrIndexObjID),
                    nl->IntAtom(attrIndexCatID)),
            newCloudType);
}

/*
1.2 Value Mapping

*/
int op_analyzeGeom::analyzeGeomVM(Word* args, Word& result, int message,
        Word& local, Supplier s) {

    if (REPORT_TO_CONSOLE)
        cout << endl << "starting analyzeGeom" << endl;
    result = qp->ResultStorage(s);

    // get source and result Pointcloud2. These variables must NOT be wrapped
    // in a shared_ptr, otherwise both clouds will be deleted when leaving
    // this method!
    Pointcloud2* sourceCloud = static_cast<Pointcloud2*>(args[0].addr);
    Pointcloud2* resultCloud = static_cast<Pointcloud2*>(result.addr);

    // get further parameters passed to Value Mapping by Type Mapping
    bool hasOldObjIDs = ((CcBool*)args[1].addr)->GetValue();
    int attrIndexObjID = ((CcInt*)args[2].addr)->GetValue();
    int attrIndexCatID = ((CcInt*)args[3].addr)->GetValue();

    // create a Timer instance for performance measurement
    Timer timer { Timer::UNIT::millis };
    size_t pointCount = sourceCloud->getPointCount();
    timer.getReportStream() << endl << "Timer for analyzeGeom, pointCount = "
            << formatInt(pointCount) << ":" << endl;

    // first we'll clone our cloud, adding "ObjID" and "CatID" attributes
    // if they were missing from the sourceCloud
    timer.startTask("copy cloud");
    if (REPORT_TO_CONSOLE) {
        cout << "creating copy of source cloud (" << formatInt(pointCount)
                << " points) with attributes " << OBJ_ID << " and " << CAT_ID
                << endl;
    }
    std::vector<std::unique_ptr<Attribute>> attributes;
    if (!hasOldObjIDs) {
        // add the new attributes
        attributes.push_back(std::unique_ptr<Attribute>(new CcInt(true, 0)));
        attributes.push_back(std::unique_ptr<Attribute>(new CcInt(true, 0)));
    } // otherwise, attributes remains empty which allows for simpler copying
    resultCloud->copyAllWithAdditionalAttributes(sourceCloud,
            attributes);

    // ensure there are points to work with
    if (!resultCloud->isDefined()) {
        cout << "Pointcloud2 is undefined." << endl;
        return 0;
    } else if (resultCloud->getPointCount() == 0) {
        cout << "Pointcloud2 is empty." << endl;
        return 0;
    }

    // the mmrCloud constructs an MMRTree which stores the respective first
    // point of a point sequence. The size of a sequence is
    // determined by the number of points stored in each RTree node,
    // (theoretically, max. 76 points, usually about 70% of that).
    // Depending on params._maxMMRCloudSequenceLength, these sequences are
    // further refined into several sequences */
    timer.startTask("create MMRCloud");
    if (REPORT_TO_CONSOLE) {
        cout << "reading all " << formatInt(pointCount) << " points "
                "to main memory" << endl;
    }
    std::shared_ptr<MMRCloud> mmrCloud = std::make_shared<MMRCloud>(resultCloud,
            ParamsAnalyzeGeom::_maxMMRCloudSequenceLength);
    if (REPORT_TO_CONSOLE) {
        mmrCloud->_tree->printStats(pointCount,
                sizeof(DbScanPoint<DIMENSIONS>));
    }

    // "conceptual parameters" that relate to how the
    // analysis is done
    double typicalPointDist = 0.0; // = Pointcloud2::TYPICAL_POINT_DISTANCE;
    if (typicalPointDist <= 0.0) {
        size_t sampleSize = pointCount / 40;
        sampleSize = std::max(sampleSize, size_t(1000));
        typicalPointDist = mmrCloud->estimateTypicalPointDistance(
                sampleSize);
    }
    const ParamsAnalyzeGeom params = ParamsAnalyzeGeom(
            resultCloud->getPointCount(), typicalPointDist);

    // if the sourceCloud already contained ObjIDs, the points that were
    // assigned to ObjIDs are excluded by setting their bit to "false"
    // in the bitMask; maxObjID is set to the previous maximum ObjID:
    size_t totalPointCount = resultCloud->getPointCount();
    size_t unassignedPointCount = totalPointCount;
    int maxObjID = 0;
    if (hasOldObjIDs) {
        timer.startTask("get previous maximum ObjID");
        maxObjID = resultCloud->getMaxValueOfIntAttr(attrIndexObjID, 0,
                mmrCloud->_bitMask);
        unassignedPointCount = mmrCloud->_bitMask->getTrueCount();
    }

    // ensure there are points to work with
    bool overwriteObjIDs = false;
    if (unassignedPointCount == 0) {
        cout << "All " << formatInt(totalPointCount) << " points in this "
                "cloud have been assigned to an ObjID in a "
                "previous analysis.";
        cout << endl << "ObjIDs are reset for a new analysis." << endl;
        unassignedPointCount = totalPointCount;
        maxObjID = 0;
        mmrCloud->_bitMask->initializeAll(true);
        overwriteObjIDs = true;
    } else if (hasOldObjIDs) {
        cout << formatInt(totalPointCount - unassignedPointCount) << " point "
                "nave been assigned to " << maxObjID << " objects in a "
                "previous analysis. " << endl;
        cout << "Restricting analysis to the remaining " <<
                formatInt(unassignedPointCount) << " points." << endl;
    }

    // the ObjectIdManager will provide us with new object ids as needed,
    // and temporarily store the assignment of each point to an object id
    timer.startTask("create ObjectIdManager");
    std::shared_ptr<ObjectIdManager> objectManager 
                      = std::make_shared<ObjectIdManager>(
            resultCloud->getPointCount(), maxObjID, overwriteObjIDs,
            attrIndexObjID, attrIndexCatID);

    // we are doing a randomized Hough-transformation, so this sample
    // is random - the _bitMask excludes points already assigned to objects
    // in a previous analysis
    std::stringstream task;
    task << "take sample of " << formatInt(params._sampleSize )<< " points";
    if (REPORT_TO_CONSOLE) {
        cout << "taking sample of " << formatInt(params._sampleSize)
                << " points" << endl;
    }
    timer.startTask(task.str());
    std::vector<SamplePoint> sample = op_analyzeGeom::takeSample(
            mmrCloud->_points, params._sampleSize, mmrCloud->_bitMask);

    // now we'll get the primitives in the order in which they shall be
    // identified (e.g. 1. Planes, 2. Spheres, 3. Cylinders, 4. Scones)
    // and iterate over them
    if (REPORT_TO_CONSOLE) {
        cout << endl << "starting analysis with parameters:" << endl
            << "- minimum object size             : "
            << params._minimumObjectExtent << endl
            << "- typical point distance in cloud : "
            << params._typicalPointDistance << endl
            << "- typical point distance in sample: "
            << params._typicalSampleDistance << endl
            << "- neighborhood diameter           : "
            << params._neighborhoodDiameter << endl;
    }

    // GpSphere will be analyzed first as currently it is the only finite
    // shape and it is less likely for points to get wrongly assigned to a
    // finite sphere than to an infinite plane, cylinder or cone)
    std::vector<std::shared_ptr<AbstractGeomPrimitive>> primitives = 
                                    op_analyzeGeom::getAllPrimitives();
    

    for (std::shared_ptr<AbstractGeomPrimitive> primitive : primitives) {

        // find primitives of the current type in the sample and match
        // the _points to the identified shapes by setting their
        // _idOfBiggestObject in the objectManager to a new object id
        // unless they were already assigned to a bigger object
        primitive->projectClusterSetObjID(sample, mmrCloud, params,
                objectManager, timer);

        // tell the objectManager this type of primitives is done with
        objectManager->categoryFinished();
    }

    // make results (ObjID and CatID) persistent
    timer.startTask("save results");
    if (REPORT_TO_CONSOLE) {
        cout << endl << "saving results to cloud" << endl;
    }
    objectManager->appointFinalObjIDs(params._requiredObjSize);
    op_analyzeGeom::saveResultsToPointcloud(resultCloud, 
                                            mmrCloud, 
                                            objectManager);

    // print summary
    cout << endl << "analyzeGeom summary:" << endl;
    size_t categoryId = 0;
    size_t unassignedLeft = unassignedPointCount;
    for (std::shared_ptr<AbstractGeomPrimitive> primitive : primitives) {
        ++categoryId;
        size_t assigned = objectManager->getAssignedPointCount(categoryId);
        cout << " * " << objectManager->getObjectCount(categoryId) << " "
                << primitive->getCaption(true) << " with "
                << formatInt(assigned) << " points found" << endl;
        unassignedLeft -= assigned;
    }
    if (unassignedLeft == 0) {
        cout << "All " << formatInt(unassignedPointCount) << " points were "
                "assigned." << endl;
    } else {
        cout << formatInt(unassignedLeft) << " of "
                << formatInt(unassignedPointCount) << " points unassigned."
                << endl;
    }
    // print runtime summary
    std::cout << timer.getReportForAllTasks();

    return 0;
}

/*
1.3 Operator specification

*/
std::string op_analyzeGeom::getOperatorSpec(){
    return OperatorSpec(
            " pointcloud2 -> pointcloud2(...,ObjID,CatID)",
            " pc2 analyzeGeom",
            " Returns classified pc2 ",
            " query pc2 analyzeGeom"
    ).getStr();
}

/*
1.4 Operator methods

*/
std::shared_ptr<Operator> op_analyzeGeom::getOperator(){
    return std::make_shared<Operator>("analyzeGeom",
                                    getOperatorSpec(),
                                    &op_analyzeGeom::analyzeGeomVM,
                                    Operator::SimpleSelect,
                                    &op_analyzeGeom::analyzeGeomTM);
}

/*
Does what the signature suggests: returns a random
selection of parameter points of size size\_of\_sample,
but none that are flipped in the corresponding bit-mask.

*/
std::vector<PointBase<DIMENSIONS>> op_analyzeGeom::takeSample(
        const std::shared_ptr<std::vector<DbScanPoint<DIMENSIONS>>>& points,
        const size_t size_of_sample,
        const std::shared_ptr<BitArray> bit_mask) {

    std::vector<PointBase<DIMENSIONS>> result;
    result.reserve(size_of_sample);

    // get an iterator that returns a random selection of (size_of_sample)
    // ids of the points that are still "true" in the bitArray (i.e. a
    // random selection of the points that have not yet been assigned to a
    // geometrical object by opAnalyzeGeom)
    auto iter = bit_mask->getIterator(size_of_sample);
    int id;
    while ((id = iter->next()) >= 0) {
        PointBase<DIMENSIONS> point = points->at(id).toPointBase();
        result.push_back(point);
    }
    return result;
}


/*
Makes the ObjIDs and CatIDs which were calculated in the analysis
persistent by storing them in the tuples associated with the points of
the Pointcloud2.

*/
void op_analyzeGeom::saveResultsToPointcloud (
        Pointcloud2* _cloud,
        std::shared_ptr<MMRCloud> mmr_cloud,
        std::shared_ptr<ObjectIdManager> objManager) {

    bool writeAll = objManager->mustOverwriteOldObjIDs();
    if (!writeAll && !objManager->wereObjectIdsCreated())
        return;

    const int attrIndexObjID = objManager->_attrIndexObjID;
    const int attrIndexCatID = objManager->_attrIndexCatID;
    std::vector<int> attrIndices;
    attrIndices.push_back(attrIndexObjID);
    attrIndices.push_back(attrIndexCatID);
    for (size_t smiId = 1; smiId < mmr_cloud->_indexOfSmiId.size(); ++smiId) {
        size_t index = mmr_cloud->_indexOfSmiId[smiId];
        if (index == 0)
            continue;

        // get the object id and the corresponding category id
        // (1 = planes, 2 = spheres etc)
        int objID = objManager->getFinalObjID(index);
        if (!writeAll && objID <= 0)
            continue;

        // continue even if objID == 0 in case all points had been assigned
        // previously and are reassigned now
        int catID = objManager->getCategoryID(index);

        PcPoint point;
        _cloud->getPoint(smiId, &point);
        if (point._tupleId == 0) {
            assert (false); // we expect the operator to create the tuples
                            // before calling this method.
            // create a tuple to store properties
            Tuple* tuple = _cloud->createEmptyTuple();
            tuple->PutAttribute(attrIndexObjID, new CcInt(objID));
            tuple->PutAttribute(attrIndexCatID, new CcInt(catID));
            _cloud->insertTupleForExistingPoint(tuple, point, smiId);
            // do NOT call tuple->DeleteIfAllowed(); here!
        } else {
            // change an existing tuple
            Tuple* tuple = _cloud->getTuple(point._tupleId);
            std::vector<Attribute*> attrs;
            attrs.push_back(new CcInt(true, objID));
            attrs.push_back(new CcInt(true, catID));
            _cloud->updateTuple(tuple, attrIndices, attrs);
            tuple->DeleteIfAllowed();
        }
    }
}

std::vector<std::shared_ptr<AbstractGeomPrimitive>>
op_analyzeGeom::getAllPrimitives(){
    std::vector<std::shared_ptr<AbstractGeomPrimitive>> result;
    result.push_back(std::make_shared<GpPlane>());
    result.push_back(std::make_shared<GpSphere>());
    result.push_back(std::make_shared<GpCylinder>());
    result.push_back(std::make_shared<GpCone>());
    return result;
}
