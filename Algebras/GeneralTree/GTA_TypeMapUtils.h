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

1 Headerfile "TypeMappingMacros.h"[4]

January-May 2008, Mirko Dibbert

1.1 Overview

This headerfile contains some macros, which are used in the type mappings of the GeneralTreeAlgebra, MTreeAlgebra and XTreeAlgebra to provide uniform error messages in all type mapping functions.

1.1 Includes and defines

*/
#include "Algebra.h"
#include "NestedList.h"
#include "NList.h"
#include <sstream>

#ifndef __GTA_TYPE_MAP_UTILS_H
#define __GTA_TYPE_MAP_UTILS_H

/********************************************************************
1.1 Common macros

********************************************************************/
/*
Macro CHECK[_]LIST[_]LENGTH:

Returns an type error, if the NList "list"[4] has not the length "len"[4].

*/
#define CHECK_LIST_LENGTH(len, list)\
{\
    stringstream err;\
    err << "Expecting " << len << " argument(s)!";\
    CHECK_COND(list.length() == len, err.str());\
}



/*
Marco CHECK[_]REAL:

Returns a type error, if the NList "arg" is not equal to "REAL"[4]. "pos"[4] should be the respective argument position to allow it to refer to it in the error message.

*/
#define CHECK_REAL(arg, pos)\
{\
    stringstream err;\
    err << "Argument " << pos << " must be an \"real\" value!";\
    CHECK_COND(arg.isEqual(CcReal::BasicType()), err.str());\
}



/*
Marco CHECK[_]REAL:

Returns a type error, if the NList "arg" is not equal to "INT"[4]. "pos"[4] should be the respective argument position to allow it to refer to it in the error message.

*/
#define CHECK_INT(arg, pos)\
{\
    stringstream err;\
    err << "Argument " << pos << " must be an \"int\" value!";\
    CHECK_COND(arg.isEqual(CcInt::BasicType()), err.str());\
}



/*
Marco CHECK[_]SYMBOL:

Returns a type error, if the NList "arg" is not a symbol. "pos"[4] should be the respective argument position to allow it to refer to it in the error message.

*/
#define CHECK_SYMBOL(arg, pos)\
{\
    stringstream err;\
    err << "Argument " << pos \
        << " must be a symbol or atomar type!";\
    CHECK_COND(arg.isSymbol(), err.str());\
}



/*
Macro CHECK[_]REL:

Returns an type error, if the NList "arg"[4] is not a relation description. Otherwhise, the attributes are written to the NList "attrs"[4]. "pos"[4] should be the respective argument position to allow it to refer to it in the error message.

*/
#define CHECK_REL(arg, attrs, pos)\
{\
    stringstream err;\
    err << "Argument " << pos << " must be a relation!";\
    CHECK_COND(arg.checkRel(attrs), err.str())\
}



/*
Macro CHECK[_]STREAM:

Returns an type error, if the NList "arg"[4] is not a tuple stream description. Otherwhise, the attributes are written to the NList "attrs"[4]. "pos"[4] should be the respective argument position to allow it to refer to it in the error message.

*/
#define CHECK_STREAM(arg, attrs, pos)\
{\
    stringstream err;\
    err << "Argument " << pos << " must be a tuple stream!";\
    CHECK_COND(arg.checkStreamTuple(attrs), err.str())\
}



/*
Macro CHECK[_]REL[_]OR[_]STREAM:

Returns an type error, if the NList "arg"[4] is not a relation or tuple stream description. Otherwhise, the attributes are written to the NList "attrs"[4]. "pos"[4] should be the respective argument position to allow it to refer to it in the error message.

*/
#define CHECK_REL_OR_STREAM(arg, attrs, pos)\
{\
    stringstream err;\
    err << "Argument " << pos\
        << " must be a relation or tuple stream!";\
    CHECK_COND(arg.checkRel(attrs) ||\
               arg.checkStreamTuple(attrs), err.str())\
}



/*
Macro CHECK[_]ATTRIBUTE:

Returns an type error, if the NList "attrName"[4] is not a symbol or the specified attribute could not be found in "attrs"[4]. Otherwhise the found position and type name is returned in "attrIndex"[4] and "typeName"[4], respectively. "pos"[4] should be the respective argument position to allow it to refer to it in the error message.

*/
#define CHECK_ATTRIBUTE(attrs, attrName, typeName, attrIndex, pos)\
{\
    stringstream err;\
    err << "Attribute name \"" << attrName << "\" is not known.\n"\
             "Known Attribute(s):\n" << attrs.convertToString();\
    ListExpr attrTypeLE;\
    attrIndex = FindAttribute(\
            attrs.listExpr(), attrName, attrTypeLE);\
    CHECK_COND(attrIndex > 0, err.str());\
    --attrIndex;\
    NList attrType (attrTypeLE);\
    typeName = attrType.str();\
}



/*
Macro GET[_]CONFIG[_]NAME:

Returs a type error, if the NList "arg"[4] is not a symbol value. Otherwhise the symbol value is written to "str"[4]. "pos"[4] should be the respective argument position to allow it to refer to it in the error message.

(this macro is used from the MTree- and XTreeAlgebra)

*/
#define GET_CONFIG_NAME(arg, configName , pos)\
{\
    stringstream err;\
    err << "Argument " << pos << " must be the name of an existing "\
        << "config object!";\
    CHECK_COND(arg.isSymbol(), err.str());\
    configName = arg.str();\
}

/********************************************************************
1,1 Macros for "DistFunReg" specific tests

********************************************************************/
/*
Macro GET[_]DISTFUN[_]NAME:

Returs a type error, if the NList "arg"[4] is not a symbol value. Otherwhise the symbol value is written to "str"[4]. "pos"[4] should be the respective argument position to allow it to refer to it in the error message.

*/
#define GET_DISTFUN_NAME(arg, distfunName, pos)\
{\
    stringstream err;\
    err << "Argument " << pos << " must be the name of an existing "\
        << "distance function or \"" + DFUN_DEFAULT + "\"!";\
    CHECK_COND(arg.isSymbol(), err.str());\
    distfunName = arg.str();\
}



/*
Macro CHECK[_]DISTFUN[_]DEFINED:

Returs a type error, if the specified distance function is not defined.

*/
#define CHECK_DISTFUN_DEFINED(distfunName, typeName, dataName)\
{\
    string errmsg;\
    if (distfunName == DFUN_DEFAULT)\
    {\
        distfunName = DistfunReg::defaultName(typeName);\
        errmsg = "No default distance function defined for type \""\
            + typeName + "\"!";\
        CHECK_COND(distfunName != DFUN_UNDEFINED, errmsg);\
    }\
    else\
    {\
        if (!DistfunReg::isDefined(\
                distfunName, typeName, dataName))\
        {\
            errmsg = "Distance function \"" + distfunName +\
                     "\" not defined for type \"" +\
                     typeName + "\" and data type \"" +\
                     dataName + "\"! Defined names: \n\n" +\
                     DistfunReg::definedNames(typeName);\
            CHECK_COND(false, errmsg);\
        }\
    }\
}



/********************************************************************
1.1 Macros for "DistDataReg" specific tests

********************************************************************/
/*
Macro GET[_]DISTDATA[_]NAME:

Returs a type error, if the NList "arg"[4] is not a symbol value. Otherwhise the symbol value is written to "str"[4]. "pos"[4] should be the respective argument position to allow it to refer to it in the error message.

*/
#define GET_DISTDATA_NAME(arg, distdataName, pos)\
{\
    stringstream err;\
    err << "Argument " << pos << " must be the name of an existing "\
        << "distdata type or \"" + DDATA_DEFAULT + "\"!";\
    CHECK_COND(arg.isSymbol(), err.str());\
    distdataName = arg.str();\
}



/*
Macro CHECK[_]DISTDATA[_]DEFINED:

Returs a type error, if the specified distdata type is not defined.

*/
#define CHECK_DISTDATA_DEFINED(dataName)\
{\
    string errmsg;\
    if (dataName == DDATA_DEFAULT)\
    {\
        errmsg = "No default distdata type defined for type "\
                 "constructor \"" + typeName + "\"!";\
        dataName = DistDataReg::defaultName(typeName);\
        CHECK_COND(dataName != DDATA_UNDEFINED, errmsg);\
    }\
    else if(!DistDataReg::isDefined(typeName, dataName))\
    {\
        errmsg = "Distdata type \"" + dataName + "\" for "\
                 "type constructor \"" + typeName + \
                 "\" is not defined! Defined names: \n\n" +\
                 DistDataReg::definedNames(typeName);\
        CHECK_COND(false, errmsg);\
    }\
}



/********************************************************************
1.1 Macros for "HPointReg" specific tests

********************************************************************/
/*
Macro GET[_]HPOINT[_]NAME:

Returs a type error, if the NList "arg"[4] is not a symbol value. Otherwhise the symbol value is written to "str"[4]. "pos"[4] should be the respective argument position to allow it to refer to it in the error message.

*/
#define GET_HPOINT_NAME(arg, funName, pos)\
{\
    stringstream err;\
    err << "Argument " << pos << " must be the name of an existing "\
        << "gethpoint function or \"" + HPOINT_DEFAULT + "\"!";\
    CHECK_COND(arg.isSymbol(), err.str());\
    funName = arg.str();\
}



/*
Macro CHECK[_]HPOINT[_]DEFINED:

Returs a type error, if the specified distdata type is not defined.

*/
#define CHECK_HPOINT_DEFINED(funName)\
{\
    string errmsg;\
    if (funName == HPOINT_DEFAULT)\
    {\
        errmsg = "No default gethpoint function defined for type "\
                 "constructor \"" + typeName + "\"!";\
        funName = HPointReg::defaultName(typeName);\
        CHECK_COND(funName != HPOINT_UNDEFINED, errmsg);\
    }\
    else if(!HPointReg::isDefined(typeName, funName))\
    {\
        errmsg = "gethpoint function  \"" + funName +\
                 "\" for type constructor \"" + typeName +\
                 "\" is not defined! Use \"" + HPOINT_DEFAULT +\
                 "\" or one of the following names: \n\n" +\
                 HPointReg::definedNames(typeName);\
        CHECK_COND(false, errmsg);\
    }\
}

/********************************************************************
1.1 Macros for "BBoxReg" specific tests

********************************************************************/
/*
Macro GET[_]HRECT[_]NAME:

Returs a type error, if the NList "arg"[4] is not a symbol value. Otherwhise the symbol value is written to "str"[4]. "pos"[4] should be the respective argument position to allow it to refer to it in the error message.

*/
#define GET_BBOX_NAME(arg, funName, pos)\
{\
    stringstream err;\
    err << "Argument " << pos << " must be the name of an existing "\
        << "gethrect function or \"" + BBOX_DEFAULT + "\"!";\
    CHECK_COND(arg.isSymbol(), err.str());\
    funName = arg.str();\
}



/*
Macro CHECK[_]HRECT[_]DEFINED:

Returs a type error, if the specified distdata type is not defined.

*/
#define CHECK_BBOX_DEFINED(funName)\
{\
    string errmsg;\
    if (funName == BBOX_DEFAULT)\
    {\
        errmsg = "No default gethrect function defined for type "\
                 "constructor \"" + typeName + "\"!";\
        funName = BBoxReg::defaultName(typeName);\
        CHECK_COND(funName != BBOX_UNDEFINED, errmsg);\
    }\
    else if(!BBoxReg::isDefined(typeName, funName))\
    {\
        errmsg = "gethrect function  \"" + funName +\
                 "\" for type constructor \"" + typeName +\
                 "\" is not defined! Use \"" + BBOX_DEFAULT +\
                 "\" or one of the following names: \n\n" +\
                 BBoxReg::definedNames(typeName);\
        CHECK_COND(false, errmsg);\
    }\
}


#endif // __GTA_TYPE_MAP_UTILS_H
