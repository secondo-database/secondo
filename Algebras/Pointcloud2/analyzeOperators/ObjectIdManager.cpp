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
#include "ObjectIdManager.h"
#include <cassert>

using namespace pointcloud2;

ObjectIdManager::ObjectIdManager(const size_t pointCount,
        const int maxObjIdInCloud, const bool overwriteOldObjIDs,
        const int attrIndexObjID, const int attrIndexCatID) :
            _idOfBestObject(1 + pointCount, 0),
            _maxObjIdAtStart(maxObjIdInCloud),
            _overwriteOldObjIDs(overwriteOldObjIDs),
            _currentCategory(1),
            _attrIndexObjID(attrIndexObjID),
            _attrIndexCatID(attrIndexCatID)
{
    _objects.push_back(ObjInfo(0, 0, 0)); // element [0] is never used
}

size_t ObjectIdManager::addObject(const unsigned objectSize,
        const unsigned objectCohesion, const double bboxPerimeter) {
    size_t objId = _objects.size(); // the index of the new _objets entry
    double objectDensity = objectCohesion; // rather not: ... / bboxPerimeter;
    _objects.push_back(
        ObjInfo(objectSize, objectDensity, _currentCategory));
    return objId;
}

void ObjectIdManager::categoryFinished() {
    ++_currentCategory;
}

bool ObjectIdManager::setNewObjIdIfObjectBigger(
        const int pointIndex, const int newObjID) {
    // if one of the objIds is SCAN_NOISE or SCAN_UNCLASSIFIED,
    // use the other object
    assert (pointIndex < int(_idOfBestObject.size()));
    assert (newObjID > 0);

    int oldObjID = _idOfBestObject[pointIndex];
    if (oldObjID <= 0) { // this point was not assigned to an object before
        _idOfBestObject[pointIndex] = newObjID;
        return true;
    }

    assert (oldObjID < int(_objects.size()) &&
            newObjID < int(_objects.size()));
    int oldObjDensity = _objects[oldObjID]._density;
    int newObjDensity = _objects[newObjID]._density;

    // TODO: Kommentar anpassen
    // as the point could be assigned to two objects, choose the bigger
    // object (i.e. bigger by count of matching points); the other objects
    // "loses" this point, so its size is decreased
    if (oldObjDensity < newObjDensity) {
        _idOfBestObject[pointIndex] = newObjID;
        --_objects[oldObjID]._size;
        return true;
    } else {
        --_objects[newObjID]._size;
        return false;
    }
}

int ObjectIdManager::appointFinalObjIDs(const size_t requiredObjectSize) {
    int finalObjId = _maxObjIdAtStart;
    _catInfos.resize(_currentCategory, CatInfo(0, 0));
    for (ObjInfo& obj : _objects) {
        obj._finalObjId =
                (obj._size < requiredObjectSize) ? 0 : ++finalObjId;
        // add this object to the category info statistics
        if (obj._finalObjId > 0) {
            size_t catId = obj._catId;
            ++_catInfos[catId]._objCount;
            _catInfos[catId]._sizeSum += obj._size;
        }
    }
    return finalObjId;
}

int ObjectIdManager::getFinalObjID(const size_t pointIndex) const {
    int objId = _idOfBestObject[pointIndex];
    return (objId < 0) ? 0 : _objects[objId]._finalObjId;
}

size_t ObjectIdManager::getCategoryID(const size_t pointIndex) const {
    int objId = _idOfBestObject[pointIndex];
    return (objId < 0) ? 0 : _objects[objId]._catId;
}

/* the number of objects assigned to the given category.
 * This can only be called after appointFinalObjIds() was called. */
size_t ObjectIdManager::getObjectCount(const size_t categoryId) const {
    assert (categoryId < _catInfos.size());
    return _catInfos[categoryId]._objCount;
}

/* returns the total number of points assigned to any object of the
 * given category. This can only be called after appointFinalObjIds()
 * was called. */
size_t ObjectIdManager::getAssignedPointCount(const size_t categoryId) const {
    assert (categoryId < _catInfos.size());
    return _catInfos[categoryId]._sizeSum;
}
