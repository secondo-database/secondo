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



1 The ObjectIdManager class

During analysis of a Pointcloud2, this class provides free object ids
and the corresponding category ids. It also stores the object id of the
biggest object which a point was assigned to so far. If alter in the
analysis an even bigger object is found for the same point, that object
will be preferred.

*/
#pragma once
#include <vector>
#include <cstddef>

namespace pointcloud2 {

class ObjectIdManager {
    /* This struct stores the size and category id of each object found
     * during the analysis. At the end of the analysis, small objects
     * are eliminated and the final, consecutive object ids are
     * determined */
    struct ObjInfo {
        /* the number of points assigned to this object */
        unsigned _size;
        /* the total number of neighborhood relationships found by DBSCAN
         * in this object, divided by the perimeter of the object's bbox */
        double _density;
        /* the category (i.e. the type of geometric primitive) this object
         * belongs to*/
        int _catId;
        /* the object id that will ultimately be used for this object
         * (objects are eliminated if they are too small) */
        int _finalObjId;

        ObjInfo(unsigned size, double density, int catId) {
            _size = size;
            _density = density;
            _catId = catId;
            _finalObjId = -1;
        }
    };

    struct CatInfo {
        /* the number of objects assigned to this category */
        size_t _objCount;
        /* the total number of points assigned to an object of this category */
        size_t _sizeSum;

        CatInfo(size_t objCount, size_t sizeSum) {
            _objCount = objCount;
            _sizeSum = sizeSum;
        }
    };

    /* for every point, this vector stores the object id of the biggest
     * object which this point can be assigned to. A point is identified
     * by its 1-based index as used in the MMRCloud.\_points. If
     * setNewObjIdIfObjectBigger() finds that the point can be assigned to an
     * even bigger object, that object is used. */
    std::vector<int> _idOfBestObject;

    /* stores information on the objects. The 1-based object id is used as
     * the index, so \_objects[0] is unused. Note that this object id is
     * temporary, as objects can still be eliminated if most of their
     * points are assigned to even bigger objects. */
    std::vector<ObjInfo> _objects;

    /* the maximum ObjId used when the analysis was started. Usually, this
     * is 0 unless the source Pointcloud2 is itself the result
     * of an analysis performed previously */
    size_t _maxObjIdAtStart;

    /* true if at the end of the analysis, all ObjIDs (even those that are 0)
     * must be written to the Pointcloud2, overwriting existing entries. */
    const bool _overwriteOldObjIDs;

    /* the category (i.e. the type of geometric primitive) currently
     * treated during the analysis. */
    int _currentCategory;

    /* statistical information on each category, assembled in
     * appointFinalObjIds() */
    std::vector<CatInfo> _catInfos;

public:
    /* the attribute index in which the ObjId is stored in the Pointcloud2's
     * tuples to make the result of the analysis persistent*/
    const int _attrIndexObjID;
    /* the attribute index in which the CatId is stored in the Pointcloud2's
     * tuples to make the result of the analysis persistent*/
    const int _attrIndexCatID;

    ObjectIdManager(const size_t pointCount, const int maxObjIdInCloud,
            const bool overwriteOldObjIDs, const int attrIndexObjID,
            const int attrIndexCatID);

    ~ObjectIdManager() = default;

    /* adds an object with the given size and cohesion (which belongs to the
     * current category) and returns its object id */
    size_t addObject(const unsigned objectSize, const unsigned objectCohesion,
            const double bboxPerimeter);

    /* updates the (temporary) object id for the given point to the
     * newObjId unless the point was already assigned to an object
     * bigger than the new object. pointIndex is the 1-based index as used
     * in the MMRCloud.\_points. Returns true if the newObjId was used. */
    bool setNewObjIdIfObjectBigger(const int pointIndex, const int newObjId);

    /* Must be called when the analysis for a primitive type is completed. */
    void categoryFinished();

    /* Returns true if getFreeObjectId() was called at least one time during
     * the analysis */
    bool wereObjectIdsCreated() const {
        return (_objects.size() > 1); // _objects[0] is empty
    }

    /* true if at the end of the analysis, all ObjIDs (even those that are 0)
     * must be written to the Pointcloud2, overwriting existing entries. */
    bool mustOverwriteOldObjIDs() const {
        return _overwriteOldObjIDs;
    }

    /* calculates the ObjIds used to make the result of the analysis
     * persistent and returns the maximum ObjId. Objects which were found
     * during the analysis but later became too small (as their points were
     * assigned to even bigger objects later in the analysis) will be
     * eliminated, i.e. the remaining points assigned to those objects get
     * the final object id 0 */
    int appointFinalObjIDs(const size_t requiredObjectSize);

    /* returns the final, persistent object id for the given point.
     * pointIndex is the 1-based index as used in the MMRCloud.\_points. */
    int getFinalObjID(const size_t pointIndex) const;

    /* returns the final, persistent category id for the given point.
     * pointIndex is the 1-based index as used in the MMRCloud.\_points. */
    size_t getCategoryID(const size_t pointIndex) const;

    /* the number of objects assigned to the given category.
     * This can only be called after appointFinalObjIds() was called. */
    size_t getObjectCount(const size_t categoryId) const;

    /* returns the total number of points assigned to any object of the
     * given category. This can only be called after appointFinalObjIds()
     * was called. */
    size_t getAssignedPointCount(const size_t categoryId) const;

private:
    size_t getMaxObjId() const {
        return _objects.size() - 1;  // _objects is 1-based
    }
};
}
