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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

1 Implementation file "HPointReg.cpp"[4]

January-May 2008, Mirko Dibbert

*/
#include <sstream>
#include "PictureFuns.h"
#include "GeneralTreeAlgebra.h"
#include "SpatialAlgebra.h"

using namespace gta;
/*
Initialize static members:

*/
bool HPointReg::initialized = false;
map<string, string> HPointReg::defaultNames;
HPointInfo HPointReg::defaultInfo;
map<string, HPointInfo> HPointReg::hpointInfos;

/*
Method ~HPointReg::addInfo~:

*/
void HPointReg::addInfo(HPointInfo info)
{
    ostringstream osId;
    osId << info.algebraId() << "#"
         << info.typeId() << "#"
         << info.name();
    hpointInfos [osId.str()] = info;

    if (info.isDefault())
        defaultNames[info.typeName()] = info.name();
}

/*
Method ~HPointReg::defaultName~:

*/
string HPointReg::defaultName(const string& typeName)
{
    if(!initialized)
        initialize();

    map<string, string>::iterator iter =
            defaultNames.find(typeName);
    if (iter == defaultNames.end())
        return HPOINT_UNDEFINED;
    else
        return iter->second;
}

/*
Method  ~HPointReg::getInfo~:

*/
HPointInfo& HPointReg::getInfo(
        const string& typeName, const string& gethpoint_FunName)
{
    if (!initialized)
        initialize();

    int algebraId, typeId;
    
    SecondoSystem::GetCatalog()->GetTypeId(typeName, algebraId, typeId);

    ostringstream osId;
    osId << algebraId << "#" << typeId << "#" << gethpoint_FunName;

    hpointIter iter = hpointInfos.find(osId.str());
    if (iter != hpointInfos.end())
        return iter->second;
    else
        return defaultInfo;
}

/*
Method ~HPointReg::definedNames~:

*/
string HPointReg::definedNames(const string &typeName)
{
    if(!initialized)
        initialize();

    // search first info object for typeName
    hpointIter iter = hpointInfos.begin();
    while ((iter != hpointInfos.end()) &&
            (iter->second.typeName() != typeName))
    {
        ++iter;
    }

    // process all info objects for typeName
    ostringstream result;
    while ((iter != hpointInfos.end()) &&
            (iter->second.typeName() == typeName))
    {
        result << "\"" << iter->second.name() << "\" ";
        ++iter;
    }

    return result.str();
}

/********************************************************************
Implementation of gethpoint functions:

********************************************************************/
/*
Method ~HPointReg::gethpoint[_]Int~:

*/
HPoint *HPointReg::gethpoint_Int(const void *attr)
{
    int value = static_cast<const CcInt*>(attr)->GetValue();
    GTA_SPATIAL_DOM* coords = new GTA_SPATIAL_DOM[1];
    coords[0] = value;
    HPoint* res = new HPoint(1, coords);
    delete[] coords;
    return res;
}

/*
Method ~HPointReg::gethpoint[_]Real~:

*/
HPoint *HPointReg::gethpoint_Real(const void *attr)
{
    SEC_STD_REAL value =
            static_cast<const CcReal*>(attr)->GetValue();
    GTA_SPATIAL_DOM* coords = new GTA_SPATIAL_DOM[1];
    coords[0] = value;
    HPoint* res =  new HPoint(1, coords);
    delete[] coords;
    return res;
}

/*
Method ~HPointReg::gethpoint[_]Point~:

*/
HPoint *HPointReg::gethpoint_Point(const void *attr)
{
    const Point* point = static_cast<const Point*>(attr);
    GTA_SPATIAL_DOM coords[2];
    coords[0] = point->GetX();
    coords[1] = point->GetY();
    HPoint* res =  new HPoint(2, coords);
    return res;
}

/*
Method ~HPointReg::gethpoint[_]HPoint~:

*/
HPoint *HPointReg::gethpoint_HPoint(const void *attr)
{
    const HPointAttr *p = static_cast<const HPointAttr*>(attr);
    return p->hpoint();
}

/********************************************************************
Method ~HPointReg::initialize~:

All avaliable gethpoint functions must be registered in this method by calling the "addInfo"[4] method whith the respective "HPointInfo" object. The parameter of the "HPointInfo"[4] constructor have the following meaning:

  1 name of the bounding box type

  2 name of the assigned type constructor

  3 assigned gethpoint function

  4 flags (optional - currently only the "HPOINT[_]IS[_]DEFAULT"[4] flag is defined)

*/
void HPointReg::initialize()
{
    if (initialized)
        return;

    addInfo(HPointInfo(
        HPOINT_NATIVE,
        CcInt::BasicType(), gethpoint_Int, HPOINT_IS_DEFAULT));

    addInfo(HPointInfo(
        HPOINT_NATIVE,
        CcReal::BasicType(), gethpoint_Real, HPOINT_IS_DEFAULT));

    addInfo(HPointInfo(
        HPOINT_NATIVE,
        Point::BasicType(), gethpoint_Point, HPOINT_IS_DEFAULT));

    addInfo(HPointInfo(
        HPOINT_NATIVE,
        "hpoint", gethpoint_HPoint, HPOINT_IS_DEFAULT));

    PictureFuns::initHPointReg();

    initialized = true;
}
