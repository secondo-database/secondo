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

1 Headerfile "BBoxReg.h"[4]

January-March 2008, Mirko Dibbert

1.1 Overview

This headerfile contains the "BBoxReg" class, which provides the avaliable bounding box types. Each bounding box is assigned to a specific distdata type, which could be used to create a new bounding box, which type depends on the distdata type.

*/
#ifndef __BBOX_REG_H
#define __BBOX_REG_H

#include "DistDataReg.h"
#include "BBox.h"

using general_tree::DistDataInfo;

namespace general_tree
{

// type for getBBox functions
typedef BBox* (*GetBBoxFun)(const DistData* data);

// type for createBBox functions
typedef BBox* (*CreateBBoxFun)();

// flags for the BBoxInfo class
// (BBOX_IS_DEFINED will be automatically set from the constructor)
const char BBOX_IS_DEFINED = (1 << 0);

/********************************************************************
1.1 Class BBoxInfo

********************************************************************/
class BBoxInfo
{
public:
/*
Default constructor (creates an undefined info object)

*/
    inline BBoxInfo() :
        m_getBBox(0), m_createBBox(0), m_distdataId(false),
        m_distdataInfo(), m_flags(0)
    {}

/*
Constructor (creates a new info object with the given values)

*/
    BBoxInfo(const GetBBoxFun getBBoxFun,
             const CreateBBoxFun createBBoxFun,
             DistDataId distdataId, char flags = 0) :
        m_getBBox(getBBoxFun), m_createBBox(createBBoxFun),
        m_distdataId(distdataId),
        m_distdataInfo(DistDataReg::getInfo(m_distdataId)),
        m_flags(BBOX_IS_DEFINED | flags)
    {}

/*
Returns the assigned getBBox function, which creates bounding box from a distdata object.

*/
    inline GetBBoxFun getBBoxFun() const
    { return m_getBBox; }

/*
Returns the bounding box for "data"[4].

*/
    inline BBox* getBBox(const DistData* data) const
    { return m_getBBox(data); }

/*
Returns the assigned createBBox function, which creates a new bounding box of the assigned type.

*/
    inline CreateBBoxFun createBBoxFun() const
    { return m_createBBox; }

/*
Creates a new bouning box.

*/
    inline BBox* createBBox() const
    { return m_createBBox(); }

/*
Returns the assigned "DistDataInfo"[4] object.

*/
    inline DistDataInfo data() const
    { return m_distdataInfo; }

/*
Returns the assigned getdata function, which creates distdata objects of the distdata type.

*/
    inline GetDataFun getDataFun() const
    { return m_distdataInfo.getDataFun(); }

/*
Calls the assigned getdata function with parameter "attr"[4].

*/
    inline DistData* getData(const void* attr)
    { return m_distdataInfo.getData(attr); }

/*
Returns "true"[4], if the "BBoxInfo"[4] object is defined, and "false"[4] otherwhise. This method should only return "false"[4] for the default "BBoxInfo"[4] object, which is returned, if a requested distance function could not be found in the "BBoxReg"[4] class.

*/
    inline bool isDefined() const
    { return (m_flags & BBOX_IS_DEFINED); }

private:
    GetBBoxFun    m_getBBox;      // assigned GetBBox function
    CreateBBoxFun m_createBBox;   // assigned CreateBBox function
    DistDataId    m_distdataId;   // assigned distdata type
    DistDataInfo  m_distdataInfo; // assigned distdata info object
    char m_flags;

};

/********************************************************************
1.1 Class BBoxReg

********************************************************************/
class BBoxReg
{
public:
/*
Registeres a new "BBoxInfo"[4] object.

*/
    static void addInfo(BBoxInfo info);

/*
Returns the specified "BBoxInfo"[4] object.

*/
    static BBoxInfo& getInfo(DistDataId id);

/*
Returns "true"[4], if the specified "BBoxInfo"[4] object exists.

*/
    static inline bool isDefined(DistDataId id)
    { return getInfo(id).isDefined(); }

private:
    typedef map<string, BBoxInfo>::iterator bboxIter;

    static void initialize();
    static bool initialized;

    static BBoxInfo defaultInfo;
    static map<string, BBoxInfo> bboxInfos;


/********************************************************************
Below, the avaliable distance functions are implemented:

********************************************************************/
    static BBox* getBBoxInt(const DistData* data);
    static BBox* getBBoxReal(const DistData* data);

    static inline BBox* createBBoxInt()
    { return new GenericBBox<int, 1>; }

    static inline BBox* createBBoxReal()
    { return new GenericBBox<SEC_STD_REAL, 1>; }
};

} // namespace general_tree
#endif // ifndef __BBOX_REG_H
//
