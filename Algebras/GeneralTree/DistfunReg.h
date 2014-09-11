/*
\newpage

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

1.1 Headerfile "DistfunReg.h"[4]

January-May 2008, Mirko Dibbert

1.1.1 Overview

This headerfile declares the following classes:

  * "DistfunInfo"[4]\\
    Objects of this class store a function pointer to a distance function, which computes the distance between two "DistData"[4] objects. Each "DistfunInfo"[4] object is assigned to a distdata-type, which specifies the expected content of the "DistData"[4] objects.

  * "DistfunReg"[4]\\
    This class managages all defined "DistfunInfo"[4] objects. Each "DistfunInfo"[4] object could be unambigously selected by the name of the distance function and the expected distdata-type (e.g. the name of the distance function, the name of the distdata-type and the name of the type constructor or the name of the distance function and a "DistDataId"[4] object).

New distance functions must be registered in the "DistfunReg::initialize"[4] method.

1.1.1 Includes and defines

*/
#ifndef __DISTFUN_REG_H__
#define __DISTFUN_REG_H__

#include "DistDataReg.h"

namespace gta
{

// typedef for distance functions
typedef void (*Distfun)(
        const DistData *data1, const DistData *data2,
        double &result);

/*
Refers the default distance function for the each type constructor.

*/
const string DFUN_DEFAULT("default");

/*
This value is returned from "DistfunReg::defaultName()"[4], if no default distance function has been found for the resp. type constructor.

*/
const string DFUN_UNDEFINED("n/a");

/////////////////////////////////////////////////////////////////////
// Name and short descriprions for the defined distance functions:
/////////////////////////////////////////////////////////////////////
const string DFUN_EUCLID("euclid");
const string DFUN_EUCLID_DESCR("euclidean distance function");

const string DFUN_SPECIALPOINTS("specialpoints");
const string DFUN_SPECIALPOINTS_DESCR("compute a distance assuming that "
                                      "x coordinates denote a dimension");

const string DFUN_EDIT_DIST("edit");
const string DFUN_EDIT_DIST_DESCR("edit distance function");

const string DFUN_QUADRATIC("quadratic");
const string DFUN_QUADRATIC_DESCR(
    "quadratic distance function using a similarity matrix");

const string DFUN_SYMTRAJ_DIST1("symtraj1");
const string DFUN_SYMTRAJ_DIST1_DESCR("distance function for symbolic "
  "trajectories, normalized to [0,1]");

/////////////////////////////////////////////////////////////////////
// Flags for the "DistfunInfo" class
/////////////////////////////////////////////////////////////////////

// this flag is set for all defined info objects
// (will automatically assigned from the constructor)
const char DFUN_IS_DEFINED = (1 << 0);

// if set, the info object will be used as default for the resp. type
// constructor (if more than one info object for the same type
// constructor is specified as default, the least added one will be
// used)
const char DFUN_IS_DEFAULT = (1 << 1);

// should be set, if the respective distance function is a metric
const char DFUN_IS_METRIC  = (1 << 2);



/********************************************************************
1.1.1 Class ~DistfunInfo~

********************************************************************/
class DistfunInfo
{

public:
/*
Default constructor (creates an undefined info object)

*/
    DistfunInfo();

/*
Constructor (creates a new info object with the given values)

*/
    DistfunInfo(
            const string &name, const string &descr,
            const Distfun distfun, DistDataInfo distdataInfo,
            char flags = 0);

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
Returns the assigned getdata function.

*/
    inline GetDataFun getDataFun() const
    { return m_distdataInfo.getDataFun(); }

/*
Computes the distance between "data1"[4] and "data2"[4] with the assigned distance function.

*/
    inline void dist(const DistData *data1, const DistData *data2,
                     double &result) const
    { m_distfun(data1, data2, result); }

/*
Returns the assigned "DistDataInfo"[4] object.

*/
    inline DistDataInfo data() const
    { return m_distdataInfo; }

/*
Generates a new "DistData"[4] object from "attr"[4] by applying the assigned getdata function.

*/
    inline DistData *getData(const Attribute *attr)
    { return m_distdataInfo.getData(attr); }

/*
Returns "true"[4], if the "DistfunInfo"[4] object is defined (should only return "false"[4] for the default "DistfunInfo"[4] object, which is returned, if a requested distance function could not be found in the "DistfunReg"[4] class.

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
    DistDataInfo m_distdataInfo; // assigned distdata info object
    char m_flags;
}; // class DistfunInfo



/********************************************************************
1.1.1 Class ~DistfunReg~

********************************************************************/
class DistfunReg
{

public:
/*
Initializes the distance functions and distdata types (in particular sets the default distdata types)

*/
    static void initialize();

/*
Returns "true"[4], if the "initialize"[4] function has already been called.

*/
    static inline bool isInitialized()
    { return initialized; }

/*
Adds a new "DistfunInfo"[4] object.

*/
    static void addInfo(DistfunInfo info);

/*
Returns the name of the default distance function for the specified type.

*/
    static string defaultName(const string &typeName);

/*
Returns the specified "DistfunInfo"[4] object.

*/
    static DistfunInfo &getInfo(
            const string &distfunName, DistDataId id);

/*
Returns the specified "DistfunInfo"[4] object.

*/
    static inline DistfunInfo &getInfo(
            const string &distfunName, const string &typeName,
            const string &dataName)
    {
        return getInfo(
                distfunName, DistDataReg::getId(typeName, dataName));

    }

/*
Returns "true"[4], if the specified "DistFunInfo"[4] object does exist.

*/
    static inline bool isDefined(
            const string &distfunName, const string &typeName,
            const string &distdataName)
    {
        return getInfo(
                distfunName, typeName, distdataName).isDefined();
    }

/*
Returns "true"[4], if the specified "DistFunInfo"[4] object does exist.

*/
    static inline bool isDefined(
            const string &distfunName, DistDataId id)
    { return getInfo(distfunName, id).isDefined(); }

/*
Returns a string with a list of all defined distance functions for the specified type (could e.g. be used in the type checking functions in case of errors to show possible values).

*/
    static string definedNames(const string &typeName);

/*
Returns a list with all defined "DistfunInfo"[4] objects.

*/
    static void getInfoList(list<DistfunInfo> &result);

/*
Prints a structured list with all defined distance functions to cmsg.info().

*/
    static string printDistfuns();

private:
    typedef map<string, DistfunInfo>::iterator distfunIter;

    static bool initialized;
    static bool distfunsShown;
    static map<string, string> defaultNames;
    static DistfunInfo defaultInfo;
    static map<string, DistfunInfo> distfunInfos;

/////////////////////////////////////////////////////////////////////
// Defined distance functions:
/////////////////////////////////////////////////////////////////////
/*
Euclidean distance function for the "int"[4] type constructor.

*/
    static void EuclideanInt(
            const DistData *data1, const DistData *data2,
            double &result);

/*
Euclidean distance function for the "real"[4] type constructor.

*/
    static void EuclideanReal(
            const DistData *data1, const DistData *data2,
            double &result);


/*
euclidean distance for usual point values

*/
static void euclidPoint(
            const DistData *data1, const DistData *data2,
            double &result);


/*
Distance function for the "Points"[4] type constructor.

This function is used for computing the distacce between two points values.
The first dimension is used a dimension identifier and the second dimension is
the value for this dimensioni.

*/
    static void specialPoints(
            const DistData *data1, const DistData *data2,
            double &result);

/*
Euclidean distance function for the "hpoint"[4] type constructor.

*/
    static void EuclideanHPoint(
            const DistData *data1, const DistData *data2,
            double &result);

/*
Edit distance function for the "string"[4] type constructor.

*/
    static void EditDistance(
            const DistData *data1, const DistData *data2,
            double &result);

/*
Distance function between symbolic trajectories; normalized to [0,1].

*/
    template<class M>
    static void symTrajDistance1(
            const DistData *data1, const DistData *data2, double &result);

}; // class DistfunReg

} //namespace gta
#endif // #ifndef __DISTFUN_REG_H__
