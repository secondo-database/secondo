/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute iter and/or modify
iter under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that iter will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

1 Implementation file "BBoxReg.cpp"[4]

January-March 2008, Mirko Dibbert
\\[3ex]
This file implements the "BBoxReg"[4] class.

*/
#include "BBoxReg.h"

using namespace general_tree;

/*
Initialize static members :

*/
bool BBoxReg::initialized = false;
BBoxInfo BBoxReg::defaultInfo;
map<string, BBoxInfo> BBoxReg::bboxInfos;

/*
Method ~addInfo~:

*/
void
BBoxReg::addInfo(BBoxInfo info)
{
    ostringstream osId;
    osId << info.data().algebraId() << "#"
         << info.data().typeId() << "#"
         << info.data().id();
    bboxInfos [osId.str()] = info;
}

/*
Method  ~getInfo~:

*/
BBoxInfo&
BBoxReg::getInfo(DistDataId id)
{
    if (!initialized)
        initialize();

    ostringstream osId;
    if (!id.defined)
        return defaultInfo;

    osId << id.algebraId << "#" << id.typeId << "#" << id.distdataId;

    bboxIter iter = bboxInfos.find(osId.str());
    if (iter != bboxInfos.end())
        return iter->second;
    else
        return defaultInfo;
}

/********************************************************************
Below, the avaliable distance functions are implemented:

********************************************************************/
/*
Method ~getBBoxInt~:

*/
BBox*
BBoxReg::getBBoxInt(const DistData* data)
{
    int value = *static_cast<const int*>(data->value());
    int* lb = new int[1];
    int* ub = new int[1];
    lb[0] = value;
    ub[0] = value;
    return new GenericBBox<int, 1>(lb, ub);
}

/*
Method ~getBBoxReal~:

*/
BBox*
BBoxReg::getBBoxReal(const DistData* data)
{
    SEC_STD_REAL value =
        *static_cast<const SEC_STD_REAL*>(data->value());
    SEC_STD_REAL* lb = new SEC_STD_REAL[1];
    SEC_STD_REAL* ub = new SEC_STD_REAL[1];
    lb[0] = value;
    ub[0] = value;
    return new GenericBBox<SEC_STD_REAL, 1>(lb, ub);
}



/*
Method ~initialize~:

Registeres all defined bounding box types. The parameters of the BBoxInfo constructor have the following meaning:

  1 assigned GetBBox function

  2 assigned CreateBBox function

  3 id of the assigned distdata type

  4 flags (currently no individual flags defined)

*/
void
BBoxReg::initialize()
{
    if (initialized)
        return;

    addInfo(BBoxInfo(
        getBBoxInt, createBBoxInt,
        DistDataReg::getDataId(INT, DDATA_NATIVE)));

    addInfo(BBoxInfo(
        getBBoxReal, createBBoxReal,
        DistDataReg::getDataId(REAL, DDATA_NATIVE)));

    initialized = true;
}
