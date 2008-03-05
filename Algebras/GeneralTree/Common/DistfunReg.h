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

1 Headerfile "DistfunReg.h"[4]

January-February 2008, Mirko Dibbert

1.1 Overview

This headerfile contains the "DistfunReg" class, which contains some distance functions, that are e.g. used in the m- and x-tree algebra. These distance functions expect two "DistData"[4] objects as parameter, that could be created with the respective getdata function, wich could be received from the "DistfunInfo"[4] object for the distance function. The possible distdata types are defined in the "DistDataReg"[4] class. Constants for the names and descriptions of the distance functions are defined in the "DistDataNames.h"[4] headerfile.

*/
#ifndef __DISTFUN_REG_H
#define __DISTFUN_REG_H

#include "DistDataReg.h"
#include "DistfunNames.h"

extern AlgebraManager* am;
extern SecondoInterface* si;

namespace general_tree
{

// result type of distance functions
// (should be a floating point type, e.g. float or double)
typedef double DFUN_RESULT_TYPE;

// type for distance functions
typedef void (*Distfun)(const DistData* data1, const DistData* data2,
                        DFUN_RESULT_TYPE& result);

// flags for the DistfunInfo class
// (DFUN_IS_DEFINED will be automatically set from the constructor)
const char DFUN_IS_DEFINED = (1 << 0);
const char DFUN_IS_DEFAULT = (1 << 1);
const char DFUN_IS_METRIC  = (1 << 2);

/********************************************************************
1.1 Class "DistfunInfo"[4]

This class stores the name and short description of a defined distance function (constants for these members should be defined in the "DistfunSymbols.h"[4] header). Furtherore it contains a pointer to the respective distance function and a distdata object for the expected distdata type.

********************************************************************/
class DistfunInfo
{
public:
/*
Default constructor (creates an undefined info object)

*/
    inline DistfunInfo() :
            m_name("undef"),
            m_descr("undef"),
            m_distfun(0),
            m_distdataInfo(),
            m_flags(0)
    {}

/*
Constructor (creates a new info object with the given values)

*/
    DistfunInfo(const string& name, const string& descr,
                const Distfun distfun, DistDataId distdataId,
                char flags) :
            m_name(name), m_descr(descr),
            m_distfun(distfun), m_distdataId(distdataId),
            m_distdataInfo(DistDataReg::getInfo(m_distdataId)),
            m_flags(DFUN_IS_DEFINED | flags)
    {}

/*
Returns the name of the distance function.

*/
    inline string name() const
    { return m_name; }

/*
Returns the description of the distance function.

*/
    inline string descr() const
    { return m_descr; }

/*
Returns the assigned distance funcion.

*/
    inline Distfun distfun() const
    { return m_distfun; }

/*
Calls the assigned distance funcion with the given distdata objects.

*/
    inline void dist(const DistData* data1, const DistData* data2,
                     DFUN_RESULT_TYPE& result) const
    { m_distfun(data1, data2, result); }

/*
Calls the assigned distance funcion with the given attribute objects (the respective distdata obejcts will be computed with the "getData"[4] method).

*/
    inline void dist(const Attribute* attr1, const Attribute* attr2,
                     DFUN_RESULT_TYPE& result)
    { m_distfun(getData(attr1), getData(attr2), result); }

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
Returns "true"[4], if the "DistfunInfo"[4] object is defined, and "false"[4] otherwhise. This method should only return "false"[4] for the default "getInfo"[4] object, which is returned, if a requested distance function could not be found in the "DistfunReg"[4] class.

*/
    inline bool isDefined() const
    { return (m_flags & DFUN_IS_DEFINED); }

/*
Returns "true"[4], if the info object is the default info object for the respective type constructor.

*/
    inline bool isDefault() const
    { return (m_flags & DFUN_IS_DEFAULT); }

/*
Returns "true"[4], if the assigned distance function is a metric.

*/
    inline bool isMetric() const
    { return (m_flags & DFUN_IS_METRIC); }

private:
    string m_name;               // unique name (used for selection)
    string m_descr;              // short description
    Distfun m_distfun;           // assigned distance function
    DistDataId m_distdataId;     // assigned distdata type
    DistDataInfo m_distdataInfo; // assigned distdata info object
    char m_flags;
};

/********************************************************************
1.1 Class DistfunReg

This class contains all defined distdata types, distance functions and bounding box types.

********************************************************************/
class DistfunReg
{
public:
/*
Registeres a new "DistfunInfo"[4] object.

*/
    static void addInfo(DistfunInfo info);

/*
Returns the specified "DistfunInfo"[4] object.

*/
    static DistfunInfo& getInfo(
        const string& distfunName, const string& typeName,
        const string& dataName);

/*
Returns the specified "DistfunInfo"[4] object.

*/
    static inline DistfunInfo& getInfo(
        const string& distfunName, DistDataId id)
    {
        DistDataInfo datainfo = DistDataReg::getInfo(id);
        return getInfo(
            distfunName, datainfo.typeName(), datainfo.name());
    }

/*
Returns "true"[4], if the specified "DistFunInfo"[4] object exists.

*/
    static inline bool isDefined(
        const string& typeName,
        const string& distfunName,
        const string& distdataName)
    {
        return getInfo(
                distfunName, typeName, distdataName).isDefined();
    }

/*
Returns the name of the default distance function for the specified type.

*/
    static string defaultName(const string& typeName);

/*
Returns a string with a list of all defined distance functions for the specified type (could be used in the type checking functions, if an undefined distance function has been requested).

*/
    static string definedNames(const string& typeName);

/*
Returns a list with all defined "DistfunInfo"[4] objects.

*/
    static void getInfoList(list<DistfunInfo>& result);

/*
Prints a structured list with all defined distance functions to cmsg.info().

*/
    static void printDistfuns();

private:
    typedef map<string, DistfunInfo>::iterator distfunIter;

    static void initialize();
    static bool initialized;

    static bool distfunsShown;

    static map<string, string> defaultNames;

    static DistfunInfo defaultInfo;
    static map<string, DistfunInfo> distfunInfos;

/********************************************************************
Below, all avaliable distance functions are defined:

********************************************************************/
/*
Euclidean distance function for the "int"[4] type constructor.

*/
    static void EuclideanInt(
        const DistData* data1, const DistData* data2,
        double& result);

/*
Euclidean distance function for the "real"[4] type constructor.

*/
    static void EuclideanReal(
        const DistData* data1, const DistData* data2,
        double& result);

/*
Edit distance function for the "string"[4] type constructor.

*/
    static void EditDistance(
        const DistData* data1, const DistData* data2,
        double& result);
};

} //namespace distfuns
#endif // #ifndef __DISTUN_REG_H
