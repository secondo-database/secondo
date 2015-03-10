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

1.1 Headerfile "BBoxReg.h"[4]

January-May 2008, Mirko Dibbert

This file contains all defined getbbox functions. New functions must be registered in the\\
"BBoxReg::initialize"[4] method.

1.1.1 Includes and defines

*/
#ifndef __BBOX_REG_H__
#define __BBOX_REG_H__

#include "SecondoInterface.h"
#include "Symbols.h"
#include "TypeConstructor.h"
#include "RectangleAlgebra.h"
#include "GTA_SpatialAttr.h"

namespace gta
{

typedef HRect* (*GetBBoxFun)(const void *data);

/*
Refers the default getbbox function for the each type constructor.

*/
const string BBOX_DEFAULT("default");

/*
This value is returned from "BBoxReg::defaultName()"[4], if no default getbbox function has been found for the resp. type constructor.

*/
const string BBOX_UNDEFINED("n/a");

/////////////////////////////////////////////////////////////////////
// constants for the getbbox function names:
/////////////////////////////////////////////////////////////////////

// native getbbox function (e.g. used if only one fun is defined)
const string BBOX_NATIVE("native");



/////////////////////////////////////////////////////////////////////
// Flags for the BBoxInfo class:
/////////////////////////////////////////////////////////////////////

// this flag is set for all defined info objects
// (will automatically assigned from the constructor)
const char BBOX_IS_DEFINED = (1 << 0);

// if set, the info object will be used as default for the resp. type
// constructor (if more than one info object for the same type
// constructor is specified as default, the least added one will be
// used)
const char BBOX_IS_DEFAULT = (1 << 1);



/********************************************************************
1.1.1 Class ~BBoxInfo~

This class stores a pointer to a getbbox function together with the function name.

********************************************************************/
class BBoxInfo
{

public:
/*
Default constructor (creates an undefined info object)

*/
    inline BBoxInfo()
    : m_name("undef"), m_getbboxFun(0), m_flags(0)
    {}

/*
Constructor (creates a new info object with the given values)

*/
    inline BBoxInfo(
            const string &name,
            const string &typeName,
            const GetBBoxFun getbbox_Fun,
            char flags = 0)
        : m_name(name), m_getbboxFun(getbbox_Fun),
          m_flags(BBOX_IS_DEFINED | flags)
    { static SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
      ctlg->GetTypeId(typeName, m_algebraId, m_typeId); }

/*
Returns the name of the assigned getbbox function.

*/
    inline string name() const
    { return m_name; }

/*
Returns the assigned getbbox function.

*/
    inline GetBBoxFun getBBoxFun() const
    { return m_getbboxFun; }

/*
Generates a new "HRect"[4] object with the assigned getbbox function from "attr"[4].

*/
    inline HRect* getBBox(const void *attr) const
    { return m_getbboxFun(attr); }

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
Returns "true"[4], if the "BBoxInfo"[4] object is defined (should only be "false"[4], if a requested getbbox function could not be found in the "BBoxReg"[4] class).

*/
    inline bool isDefined() const
    { return (m_flags & BBOX_IS_DEFINED); }

/*
Returns "true"[4], if the info object is the default info object for the respective type constructor.

*/
    inline bool isDefault() const
    { return (m_flags & BBOX_IS_DEFAULT); }

private:
    string m_name;   // unique name (used for selection)
    int m_algebraId; // algebra-id of the assigned type constructor
    int m_typeId;    // type-id of the assigned type constructor
    GetBBoxFun m_getbboxFun; // assigned getbbox_ function
    char m_flags;
}; // class BBoxInfo



/********************************************************************
1.1.1 Class ~BBoxReg~

This class manages all defined getbbox functions.

********************************************************************/
class BBoxReg
{

public:
/*
Initializes the defined getbbox functions.

*/
    static void initialize();

/*
Returns "true"[4], if the "initialize"[4] function  has already been called.

*/
    static inline bool isInitialized()
    { return initialized; }

/*
Adds a new "BBoxInfo"[4] object.

*/
    static void addInfo(BBoxInfo info);

/*
Returns the name of the default getbbox function for the specified type.

*/
    static string defaultName(const string &typeName);

/*
Returns the specified "BBoxInfo"[4] object.

*/
    static BBoxInfo &getInfo(
            const string &typeName, const string &getbbox_FunName);

/*
Returns "true"[4], if the specified "BBoxInfo"[4] object does exist.

*/
    static inline bool isDefined(
            const string &typeName, const string &getbbox_FunName)
    { return getInfo(typeName, getbbox_FunName).isDefined(); }

/*
Returns a string with a list of all defined getbbox functions for the specified type (could e.g. be used in the type mapping functions in case of errors to show possible values).

*/
    static string definedNames(const string &typeName);

private:
    typedef map<string, BBoxInfo>::iterator bboxIter;

    static bool initialized;
    static BBoxInfo defaultInfo;
    static map<string, BBoxInfo> bboxInfos;
    static map<string, string> defaultNames;



/********************************************************************
Defined getbbox functions:

********************************************************************/
/*
Getbbox function for the "real"[4] type constructor.

*/
    static HRect *getbbox_Int(const void *attr);

/*
Getbbox function for the "real"[4] type constructor.

*/
    static HRect *getbbox_Real(const void *attr);

/*
Getbbox function for spatial attributes.

Need to define seperate functions for each dimension duo to compiler errors under windows:

---- Internal compiler error in c_expand_expr, at c-common.c:3714
----

*/
    static HRect *getbbox_Spatial2(const void *attr);
    static HRect *getbbox_Spatial3(const void *attr);
    static HRect *getbbox_Spatial4(const void *attr);
    static HRect *getbbox_Spatial8(const void *attr);

/*
Getbbox function for the "hpoint" type constructor.

*/
    static HRect *getbbox_HPoint(const void *attr);

/*
Getbbox function for the "bbox" type constructor.

*/
    static HRect *getbbox_HRect(const void *attr);
}; // class BBoxReg

} // namespace gta
#endif // #ifndef __BBOX_REG_H__
