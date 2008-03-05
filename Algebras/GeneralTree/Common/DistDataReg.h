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

1 Headerfile "DistDataReg.h"[4]

January-February 2008, Mirko Dibbert

1.1 Overview

This headerfile contains the "DistDataReg" class, which provides the distdata types for the distance functions in the "DistFunReg"[4] class. Constants for the names, descriptions and id's of the distdata types are defined in the "DistDataNames.h"[4] headerfile.

*/
#ifndef __DISTDATA_REG_H
#define __DISTDATA_REG_H

#include "AlgebraManager.h"
#include "SecondoInterface.h"
#include "DistData.h"
#include "DistDataNames.h"

extern AlgebraManager* am;
extern SecondoInterface* si;

namespace general_tree
{

// type for getData functions
typedef DistData* (*GetDataFun)(const void* attr);

// flags for the DistDataInfo class
// (DDATA_IS_DEFINED will be automatically set from the constructor)
const char DDATA_IS_DEFINED = (1 << 0);
const char DDATA_IS_DEFAULT = (1 << 1);

/********************************************************************
1.1 Class "DistDataInfo"[4]

This class stores the name, short description and id of a defined distdata type (constants for these members should be defined in the "DistDataNames.h"[4] header). Furtherore it contains a pointer to the respective getdata function, which creates distdata objects of the assigned type.

********************************************************************/
class DistDataInfo
{

public:
/*
Default constructor (creates an undefined info object)

*/
    inline DistDataInfo() :
            m_name("undef"),
            m_descr("undef"),
            m_id(-1),
            m_getDataFun(0),
            m_flags(0)
    {}

/*
Constructor (creates a new info object with the given values)

*/
    inline DistDataInfo(const string& name, const string& descr,
                        int id, const string& typeName,
                        const GetDataFun getDataFun, char flags) :
            m_name(name), m_descr(descr), m_id(id),
            m_getDataFun(getDataFun),
            m_flags(DDATA_IS_DEFINED | flags)
    {
        si->GetTypeId(typeName, m_algebraId, m_typeId);
    }

/*
Returns the name of the distdata type.

*/
    inline string name() const
    { return m_name; }

/*
Returns the description of the distdata type.

*/
    inline string descr() const
    { return m_descr; }

/*
Returns the id of the distdata type.

*/
    inline int id() const
    { return m_id; }

/*
Returns the assigned getdata function, which creates distdata objects of the respective type.

*/
    inline GetDataFun getDataFun() const
    { return m_getDataFun; }

/*
Calls the assigned getdata function with parameter "attr"[4].

*/
    inline DistData* getData(const void* attr)
    { return m_getDataFun(attr); }

/*
Returns the name of the assigned type constructor.

*/
    inline string typeName() const
    { return am->Constrs(m_algebraId, m_typeId); }

/*
Returns the algebra-id of the assigned type constructor.

*/
    inline int algebraId() const
    { return m_algebraId; }

/*
Returns the type-id of the assigned type constructor.

*/
    inline int typeId() const
    { return m_typeId; }

/*
Returns "true"[4], if the "DistDataInfo"[4] object is defined, and "false"[4] otherwhise. This method should only return "false"[4] for the default "getDistfunInfo"[4] object, which is returned, if a requested distance function could not be found in the "DistDataReg"[4] class.

*/
    inline bool isDefined() const
    { return (m_flags & DDATA_IS_DEFINED); }

/*
Returns "true"[4], if the info object is the default info object for the respective type constructor.

*/
    inline bool isDefault() const
    { return (m_flags & DDATA_IS_DEFAULT); }

private:
    string m_name;             // unique name (used for selection)
    string m_descr;            // short description
    int m_id;                  // unique id
    GetDataFun m_getDataFun;   // assigned getdata function
    int m_algebraId, m_typeId; // id's of the resp. type constructor
    char m_flags;
};

/********************************************************************
1.1 Class DistDataReg

This class contains all defined distdata types.

********************************************************************/
class DistDataReg
{
public:
/*
Registeres a new "DistdataInfo"[4] object.

*/
    static void addInfo(DistDataInfo info);

/*
Returns the "DistDataId"[4] of the specified "DistDataInfo"[4] object.

*/
    static DistDataId getDataId(
        const string& typeName,
        const string& distdataName);

/*
Returns the "DistDataInfo"[4] object for the specified id.

*/
    static DistDataInfo& getInfo(DistDataId id);

/*
Returns the specified "DistDataInfo"[4] object.

*/
    static inline DistDataInfo& getInfo(
        const string& typeName, const string& dataName)
    {
        return getInfo(getDataId(typeName, dataName));
    }

/*
Returns "true"[4], if the specified "DistDataInfo"[4] object exists.

*/
    static inline bool isDefined(
        const string& typeName,
        const string& distdataName)
    {
        return getInfo(typeName, distdataName).isDefined();
    }

/*
Returns the name of the default distdata type for the specified type.

*/
    static string defaultName(const string& typeName);

/*
Returns a string with a list of all defined distdata types for the specified type (could be used in the type checking functions, if an undefined distdata type has been requested).

*/
    static string definedNames(const string& typeName);

private:
    typedef map<string, DistDataInfo>::iterator distdataIter;

    static void initialize();
    static bool initialized;

    static map<string, int> dataIds;

    static DistDataInfo defaultInfo;
    static map<string, DistDataInfo> distdataInfos;

    static map<string, string> defaultNames;

/********************************************************************
Below, all avaliable getdata functions are defined:

********************************************************************/
    static DistData* getDataInt(const void* attr);
    static DistData* getDataReal(const void* attr);
    static DistData* getDataString(const void* attr);
};

} //namespace distfuns
#endif // #ifndef __DISTDATA_REG_H
