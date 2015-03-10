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

1.1 Headerfile "HPointReg.h"[4]

January-May 2008, Mirko Dibbert

This file contains all defined gethpoint functions. New functions must be registered in the \\
"HPointReg::initialize"[4] method.

1.1.1 Includes and defines

*/
#ifndef __HPOINT_REG_H__
#define __HPOINT_REG_H__

#include "Symbols.h"
#include "TypeConstructor.h"
#include "GTA_SpatialAttr.h"
#include <string>


namespace gta
{

typedef HPoint* (*GetHPointFun)(const void *data);

/*
Refers the default gethpoint function for the each type constructor.

*/
const string HPOINT_DEFAULT("default");

/*
This value is returned from "HPointReg::defaultName()"[4], if no default gethpoint function has been found for the resp. type constructor.

*/
const string HPOINT_UNDEFINED("n/a");

/////////////////////////////////////////////////////////////////////
// constants for the gethpoint function names:
/////////////////////////////////////////////////////////////////////

// native gethpoint function (e.g. used if only one fun is defined)
const string  HPOINT_NATIVE("native");



/////////////////////////////////////////////////////////////////////
// Flags for the HPointInfo class:
/////////////////////////////////////////////////////////////////////

// this flag is set for all defined info objects
// (will automatically assigned from the constructor)
const char HPOINT_IS_DEFINED = (1 << 0);

// if set, the info object will be used as default for the resp. type
// constructor (if more than one info object for the same type
// constructor is specified as default, the least added one will be
// used)
const char HPOINT_IS_DEFAULT = (1 << 1);



/********************************************************************
1.1.1 Class ~HPointInfo~

This class stores a pointer to a gethpoint function together with the function name.

********************************************************************/
class HPointInfo
{

public:
/*
Default constructor (creates an undefined info object)

*/
    inline HPointInfo()
    : m_name("undef"), m_gethpointFun(0), m_flags(0)
    {}

/*
Constructor (creates a new info object with the given values)

*/
    inline HPointInfo(
            const string &name,
            const string &typeName,
            const GetHPointFun gethpoint_Fun,
            char flags = 0)
        : m_name(name), m_gethpointFun(gethpoint_Fun),
          m_flags(HPOINT_IS_DEFINED | flags)
    {  static SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
      ctlg->GetTypeId(typeName, m_algebraId, m_typeId); }

/*
Returns the name of the assigned gethpoint function.

*/
    inline string name() const
    { return m_name; }

/*
Returns the assigned gethpoint function.

*/
    inline GetHPointFun getHPointFun() const
    { return m_gethpointFun; }

/*
Generates a new "HPoint" object of with the assigned gethpoint function from "attr"[4].

*/
    inline HPoint* getHPoint(const void *attr) const
    { return m_gethpointFun(attr); }

/*
Returns the id of the assigned type constructor.

*/
    inline int typeId() const
    { return m_typeId; }

/*
Returns the name of the assigned type constructor.

*/
    inline string typeName() const
    { return am->GetTC(m_algebraId, m_typeId)->Name(); }

/*
Returns the id of the assigned algebra.

*/
    inline int algebraId() const
    { return m_algebraId; }

/*
Returns the name of the assigned algebra.

*/
    inline string algebraName() const
    { return am->GetAlgebraName(m_algebraId); }

/*
Returns "true"[4], if the "HPointInfo"[4] object is defined (should only be "false"[4], if a requested gethpoint function could not be found in the "HPointReg"[4] class).

*/
    inline bool isDefined() const
    { return (m_flags & HPOINT_IS_DEFINED); }

/*
Returns "true"[4], if the info object is the default info object for the respective type constructor.

*/
    inline bool isDefault() const
    { return (m_flags & HPOINT_IS_DEFAULT); }

private:
    string m_name;   // unique name (used for selection)
    int m_algebraId; // algebra-id of the assigned type constructor
    int m_typeId;    // type-id of the assigned type constructor
    GetHPointFun m_gethpointFun; // assigned gethpoint function
    char m_flags;
}; // class HPointInfo



/********************************************************************
1.1.1 Class ~HPointReg~

This class manages all defined gethpoint functions.

********************************************************************/
class HPointReg
{

public:
/*
Initializes the defined gethpoint functions.

*/
    static void initialize();

/*
Returns "true"[4], if the "initialize"[4] function  has already been called.

*/
    static inline bool isInitialized()
    { return initialized; }

/*
Adds a new "HPointInfo"[4] object.

*/
    static void addInfo(HPointInfo info);

/*
Returns the name of the default gethpoint function for the specified type.

*/
    static string defaultName(const string &typeName);

/*
Returns the specified "HPointInfo"[4] object.

*/
    static HPointInfo &getInfo(
            const string &typeName, const string &gethpoint_FunName);

/*
Returns "true"[4], if the specified "HPointInfo"[4] object does exist.

*/
    static inline bool isDefined(
            const string &typeName, const string &gethpoint_FunName)
    { return getInfo(typeName, gethpoint_FunName).isDefined(); }

/*
Returns a string with a list of all defined gethpoint functions for the specified type (could e.g. be used in the type mapping functions in case of errors to show possible values).

*/
    static string definedNames(const string &typeName);

private:
    typedef map<string, HPointInfo>::iterator hpointIter;

    static bool initialized;
    static HPointInfo defaultInfo;
    static map<string, HPointInfo> hpointInfos;
    static map<string, string> defaultNames;



/********************************************************************
Defined gethpoint functions:

********************************************************************/
/*
Gethpoint function for the "real"[4] type constructor.

*/
    static HPoint *gethpoint_Int(const void *attr);

/*
Gethpoint function for the "real"[4] type constructor.

*/
    static HPoint *gethpoint_Real(const void *attr);

/*
Gethpoint function for the "point"[4] type constructor.

*/
    static HPoint *gethpoint_Point(const void *attr);

/*
Gethpoint function for the "hpoint"[4] type constructor.

*/
    static HPoint *gethpoint_HPoint(const void *attr);
}; // class HPointReg


} // namespace gta
#endif // #ifndef __HPOINT_REG_H__
