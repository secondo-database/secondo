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

1 Implementation file "BBoxReg.cpp"[4]

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
bool BBoxReg::initialized = false;
map<string, string> BBoxReg::defaultNames;
BBoxInfo BBoxReg::defaultInfo;
map<string, BBoxInfo> BBoxReg::bboxInfos;

/*
Method ~BBoxReg::addInfo~:

*/
void BBoxReg::addInfo(BBoxInfo info)
{
    ostringstream osId;
    osId << info.algebraId() << "#"
         << info.typeId() << "#"
         << info.name();
    bboxInfos [osId.str()] = info;

    if (info.isDefault())
        defaultNames[info.typeName()] = info.name();
}

/*
Method ~BBoxReg::defaultName~:

*/
string BBoxReg::defaultName(const string &typeName)
{
    if(!initialized)
        initialize();

    map<string, string>::iterator iter =
            defaultNames.find(typeName);
    if (iter == defaultNames.end())
        return BBOX_UNDEFINED;
    else
        return iter->second;
}

/*
Method  ~BBoxReg::getInfo~:

*/
BBoxInfo& BBoxReg::getInfo(
        const string& typeName, const string &getbbox_FunName)
{
    if (!initialized)
        initialize();

    int algebraId, typeId;
    SecondoSystem::GetCatalog()->GetTypeId(typeName, algebraId, typeId);

    ostringstream osId;
    osId << algebraId << "#" << typeId << "#" << getbbox_FunName;

    bboxIter iter = bboxInfos.find(osId.str());
    if (iter != bboxInfos.end())
        return iter->second;
    else
        return defaultInfo;
}

/*
Method ~BBoxReg::definedNames~:

*/
string BBoxReg::definedNames(const string &typeName)
{
    if(!initialized)
        initialize();

    // search first info object for typeName
    bboxIter iter = bboxInfos.begin();
    while ((iter != bboxInfos.end()) &&
            (iter->second.typeName() != typeName))
    {
        ++iter;
    }

    // process all info objects for typeName
    ostringstream result;
    while ((iter != bboxInfos.end()) &&
            (iter->second.typeName() == typeName))
    {
        result << "\"" << iter->second.name() << "\" ";
        ++iter;
    }

    return result.str();
}




/********************************************************************
Implementation of getbbox functions:

********************************************************************/
/*
Method ~BBoxReg::getbbox[_]Int~:

*/
HRect *BBoxReg::getbbox_Int(const void *attr)
{
    int value = static_cast<const CcInt*>(attr)->GetValue();
    GTA_SPATIAL_DOM* lb = new GTA_SPATIAL_DOM[1];
    GTA_SPATIAL_DOM* ub = new GTA_SPATIAL_DOM[1];
    lb[0] = value;
    ub[0] = value;
    HRect* res = new HRect(1, lb, ub);
    delete[] lb;
    delete[] ub;
    return res;
}

/*
Method ~BBoxReg::getbbox[_]Real~:

*/
HRect *BBoxReg::getbbox_Real(const void *attr)
{
    SEC_STD_REAL value = static_cast<const CcReal*>(attr)->GetValue();
    GTA_SPATIAL_DOM* lb = new GTA_SPATIAL_DOM[1];
    GTA_SPATIAL_DOM* ub = new GTA_SPATIAL_DOM[1];
    lb[0] = value;
    ub[0] = value;
    HRect* res =  new HRect(1, lb, ub);
    delete[] lb;
    delete[] ub;
    return res;
}

/*
Method ~BBoxReg::getbbox[_]Point~

*/
HRect *BBoxReg::getbbox_HPoint(const void *attr)
{
    HPoint *p = static_cast<const HPointAttr*>(attr)->hpoint();
    HRect *result = p->bbox();
    delete p;
    return result;
}

/*
Method ~BBoxReg::getbbox[_]HRect~

*/
HRect *BBoxReg::getbbox_HRect(const void *attr)
{
    const HRectAttr *r = static_cast<const HRectAttr*>(attr);
    return r->hrect();
}

/*
Method ~BBoxReg::getbbox[_]Spatial2~

*/
HRect *BBoxReg::getbbox_Spatial2(const void *attr)
{
    Rectangle<2> rect = static_cast<
            const StandardSpatialAttribute<2>*>(attr)->
            BoundingBox();

    GTA_SPATIAL_DOM lb[2];
    GTA_SPATIAL_DOM ub[2];
    for (int i=0; i<2; ++i)
    {
        lb[i] = rect.MinD(i);
        ub[i] = rect.MaxD(i);
    }

    return new HRect(2, lb, ub);
}

/*
Method ~BBoxReg::getbbox[_]Spatial3~

*/
HRect *BBoxReg::getbbox_Spatial3(const void *attr)
{
    Rectangle<3> rect = static_cast<
            const StandardSpatialAttribute<3>*>(attr)->
            BoundingBox();

    GTA_SPATIAL_DOM lb[3];
    GTA_SPATIAL_DOM ub[3];
    for (int i=0; i<3; ++i)
    {
        lb[i] = rect.MinD(i);
        ub[i] = rect.MaxD(i);
    }

    return new HRect(3, lb, ub);
}

/*
Method ~BBoxReg::getbbox[_]Spatial4~

*/
HRect *BBoxReg::getbbox_Spatial4(const void *attr)
{
    Rectangle<4> rect = static_cast<
            const StandardSpatialAttribute<4>*>(attr)->
            BoundingBox();

    GTA_SPATIAL_DOM lb[4];
    GTA_SPATIAL_DOM ub[4];
    for (int i=0; i<4; ++i)
    {
        lb[i] = rect.MinD(i);
        ub[i] = rect.MaxD(i);
    }

    return new HRect(4, lb, ub);
}

/*
Method ~BBoxReg::getbbox[_]Spatial8~

*/
HRect *BBoxReg::getbbox_Spatial8(const void *attr)
{
    Rectangle<8> rect = static_cast<
            const StandardSpatialAttribute<8>*>(attr)->
            BoundingBox();

    GTA_SPATIAL_DOM lb[8];
    GTA_SPATIAL_DOM ub[8];
    for (int i=0; i<8; ++i)
    {
        lb[i] = rect.MinD(i);
        ub[i] = rect.MaxD(i);
    }

    return new HRect(8, lb, ub);
}

/********************************************************************
Method ~BBoxReg::initialize~:

All avaliable getbbox functions must be registered in this method by calling the "addInfo"[4] method whith the respective "BBoxInfo" object. The parameter of the "BBoxInfo"[4] constructor have the following meaning:

  1 name of the bounding box type

  2 name of the assigned type constructor

  3 assigned getbbox function

  4 flags (optional - currently only the "BBOX[_]IS[_]DEFAULT"[4] flag is defined)

*/
void BBoxReg::initialize()
{
    if (initialized)
        return;

    addInfo(BBoxInfo(
        BBOX_NATIVE, CcInt::BasicType(), getbbox_Int, BBOX_IS_DEFAULT));

    addInfo(BBoxInfo(
        BBOX_NATIVE, CcReal::BasicType(), getbbox_Real, BBOX_IS_DEFAULT));

    addInfo(BBoxInfo(
        BBOX_NATIVE, Rectangle<2>::BasicType(),
                     getbbox_Spatial2, BBOX_IS_DEFAULT));

    addInfo(BBoxInfo(
        BBOX_NATIVE, Rectangle<3>::BasicType(),
                     getbbox_Spatial3, BBOX_IS_DEFAULT));

    addInfo(BBoxInfo(
        BBOX_NATIVE, Rectangle<4>::BasicType(),
                     getbbox_Spatial4, BBOX_IS_DEFAULT));

    addInfo(BBoxInfo(
        BBOX_NATIVE, Rectangle<8>::BasicType(),
                     getbbox_Spatial8, BBOX_IS_DEFAULT));

    addInfo(BBoxInfo(
        BBOX_NATIVE, Line::BasicType(), getbbox_Spatial2, BBOX_IS_DEFAULT));

    addInfo(BBoxInfo(
        BBOX_NATIVE, Points::BasicType(), getbbox_Spatial2, BBOX_IS_DEFAULT));

    addInfo(BBoxInfo(
        BBOX_NATIVE, Points::BasicType(), getbbox_Spatial2, BBOX_IS_DEFAULT));

    addInfo(BBoxInfo(
        BBOX_NATIVE, Region::BasicType(), getbbox_Spatial2, BBOX_IS_DEFAULT));

    addInfo(BBoxInfo(
        BBOX_NATIVE, "hpoint", getbbox_HPoint, BBOX_IS_DEFAULT));

    addInfo(BBoxInfo(
        BBOX_NATIVE, "hrect", getbbox_HRect, BBOX_IS_DEFAULT));

    initialized = true;
}
