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



0 The foundational Types of the Algebra
- especially Pointcloud2

*/
#pragma once
#include <memory>
#include <string>

#include "NestedList.h"
#include "SecondoSMI.h"
#include "MMRTree.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/RTree/RTreeAlgebra.h"

#include "utility/PointBase.h"
#include "utility/PcPoint.h"
#include "utility/DbScanPoint.h"
#include "utility/DbScan.h"
#include "utility/BitArray.h"
#include "utility/Param.h"

/*
0.1 Constants Defines

*/
constexpr int PAGE_SIZE = 4000;
const std::string CLUSTER_ATTR_NAME = "Cluster";

namespace pointcloud2 {

/*
0.3 Convenience Typedefs

*/
typedef std::shared_ptr<Relation> RelationPtr;
typedef R_Tree<DIMENSIONS, SmiRecordId> RTreeType;
typedef R_TreeNode<DIMENSIONS, SmiRecordId>* RTreeNodePtr;
typedef R_TreeLeafEntry<DIMENSIONS, SmiRecordId> RTreeLeafEntry;
typedef R_TreeInternalEntry<DIMENSIONS> RTreeInternalEntry;
typedef std::unique_ptr<RTreeType> RTreePtr;
typedef std::shared_ptr<PcPoint> PcPointPtr;
typedef std::unique_ptr<SmiRecordFile> RecordFilePtr;
typedef Rectangle<DIMENSIONS> PcBox;

/*
0.4 The geometric Referencesystems supported by the Algebra

*/
struct Referencesystem {
    enum Type {
        EUCLID, WGS84
    };

    Referencesystem() = default;

    static std::string toString(Type enumType) {
        switch (enumType) {
        case EUCLID: return "EUCLID";
        case WGS84: return "WGS84";
        default:
            throw std::invalid_argument("Unsupported Referencesystem Enum");
        }
    }

    static Type toEnum(const std::string& enumString) {
        if (enumString == "EUCLID") return EUCLID;
        else if (enumString == "WGS84") return WGS84;
        throw std::invalid_argument("Unsupported Referencesystem");
    }
};

/*
1 The Pointcloud Class

*/
class Pointcloud2 {
    RecordFilePtr _pointFile = nullptr;
    RTreePtr _rtree = nullptr;
    RelationPtr _relation = nullptr;
    bool _isDefined = true;

    /* this is set to true with the startInsert() method. If afterwards,
     * insert(const PcPoint&) is actually called, the RTree's bulk mode
     * will be activated until finalizeInsert() is called. The bulk mode
     * cannot be activated in advance since activating it without inserting
     * a point will result in a small memory leak.*/
    bool _mayStartBulkMode = false;
    /* true if the RTree is in bulk mode */
    bool _bulkMode = false;

    size_t _pointCount = 0;
    Referencesystem::Type _referencesystem = Referencesystem::EUCLID;

    void setReferencesystem(const ListExpr& expr);
    void setRelationpointer(SmiRecord &valueRecord, size_t &offset,
            const ListExpr& expr);
    void createRelationpointer(const ListExpr& expr);
    ListExpr getRelationTypeInfo(const ListExpr& tupleTypeInfo) const;

public:

    Pointcloud2() = default;
    ~Pointcloud2() = default;
    // Constructor for Open
    Pointcloud2(SmiRecord &valueRecord, size_t &offset, const ListExpr typeInfo,
            Word &value);
    // Constructor for Create
    Pointcloud2(const ListExpr& typeInfo);

/*
1.1 The Secondo-specific part

Secondo Type Information

*/
    static TypeConstructor typeConstructor;
    static ListExpr Property();
/*
Secondo Out function: this is not useful for Pointcloud2.

*/
    static ListExpr Out(ListExpr typeInfo, Word value);
/*
Secondo In function: Pointcloud2s cannot be initialized directly.

*/
    static Word In(ListExpr typeInfo, ListExpr value, int errorPos,
            ListExpr& errorInfo, bool& correct);
/*
Secondo function: used to be called "Create", but it would be
ambigious as it does not create a ready-to-use object (one has
to call CreatePointcloud2 or Open for this).

*/
    static Word Create(const ListExpr typeInfo);
/*
Secondo Close function: called after Pointcloud2 has been used.

*/
    static void Close(const ListExpr typeInfo, Word& w);
/*
Secondo Clone function: duplicate this Pointcloud2.

*/
    static Word Clone(const ListExpr typeInfo, const Word& w);
/*
Secondo Delete function: deletes this Pointcloud2.

*/
    static void Delete(const ListExpr typeInfo, Word& w);
/*
Secondo Cast function: casts a void ptr.

*/
    static void* Cast(void* addr);
/*
Secondo SizeOf function: gives the size of this object.

*/
    static int SizeOf();
/*
Secondo Open function: restores an Pointcloud2 from file.

*/
    static bool Open(SmiRecord& valueRecord, size_t& offset,
                const ListExpr typeInfo, Word &value);
/*
Secondo Save function: saves an Pointcloud2 to file.

*/
    static bool Save(SmiRecord& valueRecord, size_t& offset,
            const ListExpr typeInfo, Word &value);
    static std::string BasicType() {
        return "pointcloud2";
    }
/*
Secondo TypeCheck function: checks if the type identifier is valid.

*/
    static bool TypeCheck(ListExpr type, ListExpr& errorInfo);
/*
1.2 Type Mapping Utility Functions

*/
    static bool TypeCheck(ListExpr type);
    static bool isTupleCloud(ListExpr type);
    static ListExpr getTupleType(ListExpr cloudType);
    static ListExpr appendAttributesToCloud(ListExpr cloudType,
            ListExpr appendage);
    static ListExpr cloudTypeWithParams(ListExpr refSys);
    static ListExpr cloudTypeWithParams(ListExpr refSys, ListExpr tupType);
    static int findAttributeIndexInVM(std::string const name, 
                                      ListExpr const type);
/*
1.3 Getters and Value Mapping Utility Functions

*/
    bool isDefined() const {
        return _isDefined;
    }
    bool hasRelation() const {
        return (_relation != nullptr);

    }

    int getTupleCount() const {
        return hasRelation() ? _relation.get()->GetNoTuples() : 0;
    }

    Rect3 getBoundingBox() const;
    size_t getPointCount() const;
    Referencesystem::Type getReferenceSystem() const {
        return _referencesystem;
    }
    /* pointId is 1-based! */
    PcPointPtr getPoint(const size_t pointId) const;

    /* pointId is 1-based! */
    void getPoint(const size_t pointId, PcPoint* pcPoint) const;

    SmiRecordFileIterator* getFileIterator() const;


    Tuple* getTuple(const TupleId& tupleId) const;

    TupleType* getTupleType() const;

    /* returns a newly created, empty tuple of the relation's tuple type which
     * can then be added using insert(PcPoint&, Tuple*) or insert(Tuple*) */
    Tuple* createEmptyTuple() const {
        assert(_relation != nullptr);
        return new Tuple(_relation->GetTupleType());
    }

    /* for the CcInt attribute with the given index, the maximum attribute
     * value from all tuples in the relation is returned. If the relation is
     * empty or no value is greater than defaultValue, the given default
     * value is returned. For all tuples that differ from the defaultValue,
     * the corresponding bit in bitMask is set to false */
    int getMaxValueOfIntAttr(const int attrIndex,
            const int defaultValue, std::shared_ptr<BitArray> bitMask) const;

/*
1.4 Pointcloud2 Manipulation
- Setters and Mutators

*/
    void setDefined(const bool isDefined) {
        _isDefined = isDefined;
    }

    size_t insert(PcPoint& point, Tuple* const tuple);
    size_t insert(const PcPoint& point);
/*
1.4.1
Used for bulk loading operations on the clouds RTree

*/
    void startInsert();
    void finalizeInsert();
/*
1.5 Operator Utilities

*/
    //  TODO: Aufräumen!
/*
copies points (and the corresponding tuples) from the source pointcloud2
if they are inside the given bbox

*/
    size_t copySelectionFrom(const Pointcloud2* source, const PcBox* bbox);

/*
copies points (and the corresponding tuples) from the source pointcloud2
if for their SmiRecordId, bitMap.get(SmiRecordId) returns true

*/
    size_t copySelectionFrom(const Pointcloud2* source,
            const BitArray& bitMap);

    void merge(const Pointcloud2* source,
            const bool hasTuple,
            const std::vector<int> sourceAttrIndex,
            const ListExpr tupleTypeInfo);

    void cluster(const double eps, const size_t minPts,
            size_t minClusterSizeToCopy, Pointcloud2* destPc2,
            int clusterAttrIndex, std::vector<int> destAttrIndices) const;



/*
1.5.1 Parameters for the pc2SetParam and pc2GetParam operators

*/
        // parameters for the analyzeRaster operator
        // ---------------------------------------
        /* for building raster: cell size in m*/
        static double CELL_SIZE_IN_M;
        /* for flooding: maximum altitude difference of neighbor cells
         * for calculating objects*/
        static double DELTA_ALT_IN_M;
        /* for flooding: trying 4 or 8 neighbor cells*/
        static bool NEIGHBOUR_CELLS;
        /* for calculating local maxima: maximum altitude difference
         * of neighboring cells*/
        static double THRESHOLD_MERGE;
        /* for calculating significant maxima: max distance of neighboring
         * local maxima to be merged to a significant maximum*/
        static double DISTANCE_SIG_MAXIMA;
        /* for building raster partition: overlapping 0 up to 0.5*/
        static double OVERLAP_RASTERS;
        /* for calculating ground: minimum cells for ground*/
        static size_t MIN_GROUND_CELLS;
        /* for excluding objects: minimum cells at edge
         *  of an objects to be ignored*/
        static size_t MAX_CELLS_AT_EDGE;
        /* for calculating object splits: switch of calculating of local maxima
         * and splitting of objects*/
        static bool SPLIT;
        /* for calculating objects: minimum cells of an object*/
        static size_t MIN_OBJ_SIZE;
        /* for calculating local maxima: minimum cells of an local maximum*/
        static size_t MIN_LOC_MAX_SIZE;
        /* for dbscan in classify process: coming from one point
         * this value determines the range of search for points nearby*/
        static double RASTER_CLASSIFY_EPSILON;
        /* for dbscan in classify process: minimum number of points
         * one point needs nearby to be part of a cluster*/
        static size_t RASTER_CLASSIFY_MINPTS;
        /* for dbscan in classify process: specifies the number of
         * varying scans to find best value for RASTER_CLASSIFY_EPSILON */
        static size_t RASTER_CLASSIFY_SCANSERIES;
        /* for dbscan in classify process: specifies the value that
         * is added to RASTER_CLASSIFY_EPSILON in each round of this series
         * of scans*/
        static double RASTER_CLASSIFY_SCANSERIES_ADD;
        /*  between 1 and the value of this parameter the standard deviation of
         *  the object properties get adjusted for feature vector for dbscan*/
        static size_t RASTER_CLASSIFY_NORMALIZE;
        /* set true analyzeRaster doesn't return the analyzed points, but the
         * raster. in form of a pointcloud: each point represents one cell of
         * the raster.*/
        static bool GET_RASTER_NOT_PC;
        /* switch features for classify as binary number (as int).*/
        static size_t SWITCH_FEATURES;


        // parameters for the analyzeGeom operator
        // ---------------------------------------
        /* the minimum extent of the objects (geometric primitives) which
         * the scan should be sure to recognize */
        static double MINIMUM_OBJECT_EXTENT;
        /* how many points should typically be in a neighborhood used to
         * create dual points */
        static size_t NEIGHBORHOOD_SIZE;
        /* how far points in a surface may deviate from a regular grid
         * (without sticking out from the surface, i.e. without making the
         * surface rough). Default value is 0.5, so points may deviate by
         * half a typical point distance */
        static double SOFT_DIFFUSION;
        /* how far points may deviate from their ideal positions (given an
         * ideal plane, sphere, cylinder, cone etc.) by error of measurement
         * etc., creating a rough surface. Default value is 0.5;
         * a value of 1.0 means a point can stick out from the surface
         * as far as the typical point distance */
        static double ROUGH_DIFFUSION;

        // the Params instance to access the parameters by the string
        // representation of their name from the pc2Get/SetParam operators
        static std::unique_ptr<Params> params;
private:
        /* initialize the parameters used by analyzeRaster and analyzeGeom */
        static std::unique_ptr<Params> initParams();

public:
/*
1.5.2 Utilities for Analyze Geom Operator

*/
/*
Returns a pointer to the main memory head of a deep copy
(i.e. cloned SMIFile, RTree and Relation) of this cloud.
It takes a tuple-extension to add uninitialized to the relation.

*/
    void copyAllWithAdditionalAttributes(
            const Pointcloud2* source,
            const std::vector<std::unique_ptr<Attribute>>& attributes);

/*
Returns a random sample from the cloud - the bitmap is used to exclude points
that should not be in the sample. How, do not ask me.

When taking the sample we do not yet know which points in it
will actually be categorized into a cluster, so the bitmap is only
a predicative mapping - it will not(!) be mutated here.

*/
    std::vector<PointBase<DIMENSIONS>> takeSample(
            const size_t size_of_sample,
            const std::shared_ptr<BitArray> bitArray) const;

/*
Returns a random sample from the given point vector - the bitmap is used to
exclude points that should not be in the sample. If the caller already holds
a points vector of a Pointcloud2, this method is much faster than the other
takeSample overload which retrieves the points from external memory.

*/
    static std::vector<PointBase<DIMENSIONS>> takeSample(
            const std::shared_ptr<
            std::vector<DbScanPoint<DIMENSIONS>>>& points,
            const size_t size_of_sample,
            const std::shared_ptr<BitArray> bitArray);

private:
    bool hasValidPointFile() const {
        return _pointFile != nullptr;
    }

    SmiFileId GetPointFileId() const {
        return _pointFile.get()->GetFileId();
    }

    /* inserts the given tuple into the relation, returning a TupleId (which
     * then has to be stored in the PcPoint._tupleId filed) */
    TupleId insert(Tuple* const tuple);

    /* inserts the given point and copies the corresponding tuple from the
     * source pointcloud2 if applicable */
    void insert(PcPoint& point, const Pointcloud2* source);

    void copySelectionFromRTree(const Pointcloud2* source, PcBox* bbox,
            SmiRecordId adr, bool isInsideBbox);

public:
    /* copies all points (and the corresponding tuples) from the source
     * pointcloud2. */
    void copyAllFrom(const Pointcloud2* source);

    void updateTuple(Tuple *tuple, const std::vector<int>& changedIndices,
            const std::vector<Attribute*>& newAttrs);

    /* is used to create a Pointcloud2 with tuples from a source Pointcloud2
     * without tuples. Adds the given, newly created tuple to the relation
     * and updates the reference in the given point which existed */
    void insertTupleForExistingPoint(Tuple* tuple, PcPoint& point,
            SmiRecordId smiRecordId);

/*
1.5 DBSCAN Utilities

*/
    /* Parameter pointSpace is explained in the member variables of DbScan.h.
     * maxSequenceLength determines the maximum number of points represented
     * by a single MMTree entry (must be 2 or higher, recommended: 8). A
     * small value increases the memory size required by the MMRTree,
     * a large value increases the number of distance calculations in DBSCAN.
     * Typically, MMTree entries will represent approx. 0.7 * maxSequenceLength
     * points. If maxSequenceLength is -1, the content of each RTree node
     * will be used as a point sequences without further refinement. */
    std::vector<PointIndex> getAllPoints(
            std::vector<DbScanPoint<DIMENSIONS>>& pointSpace,
            const int maxSequenceLength) const;

private:
    void getRTreeOrder(SmiRecordId adr,
            std::vector<PointIndex>& indexOfSmiId, PointIndex& index,
            std::vector<DbScanPoint<DIMENSIONS>>& pointSpace,
            const int maxSequenceLength) const;
    void copy_file(const Pointcloud2* source);
}; // end of class


/*
2 Stuff that should be removed but probably won't.

*/
template<unsigned dim>
bool UnbrokenOpenRTree(SmiRecord& valueRecord, size_t& offset,
        const ListExpr typeInfo, Word& value);

} // end of namespace pointcloud2
