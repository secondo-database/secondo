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

1.1 Headerfile "DistDataReg.h"[4]

January-May 2008, Mirko Dibbert

1.1.1 Overview

This headerfile declares the following classes:

  * "DistDataId"[4]\\
    Objects of this class store the id of a distdata-type together with the id of a type constructor (type-constructor id + algebra id).

  * "DistData"[4]\\
    This class is used as parameter type for the distance functions, which are defined in the "DistfunReg"[4] class (headerfile "Distfunreg.h"[4]). Objects of this class contain a char array, which stores all neccesary data for distance computations (usually the memory representation of a value (or an array of values), but it could also contain any other data, e.g. the name of a file, from which the distance function should read the data).

  * "DistDataAttribute"[4]\\
    This class is similar to the "DistData"[4] class, but it implemets the "Attribute"[4] class and the data is stored within a "FLOB"[4] instead of a char array. Additionally it stores a "DistDataId"[4] object, which is needed to select the assigned "DistDataInfo"[4] object from the "DistDataReg"[4] class and to test, if "DistDataAttr"[4] objects belong to the same distdata-type.

  * "DistDataInfo"[4]\\
    Objects of this class store a function pointer to a getdata-function, which transforms "Attribute"[4] objects into "DistData"[4] objects. Each type constructor could provide several distdata-types, which are distinguished by the name of the assigned type-constructor and the name of the distdata-type (used e.g. for the picture type constructor to create several histogram-types from pictures).

  * "DistDataReg"[4]\\
    This class managages all defined "DistDataInfo"[4] objects. Each "DistDataInfo"[4] object could be unambigously selected by the name of the distdata-type and the name of the type-constructor or by a "DistDataId"[4] object.

New distdata-types must be registered in the "DistDataReg::initialize"[4] method.

1.1.1 Includes and defines

*/
#ifndef __DISTDATA_REG_H__
#define __DISTDATA_REG_H__

#include "SecondoInterface.h"
#include <iostream>
#include <string>
#include "StandardTypes.h"
#include "Attribute.h"
#include "Symbols.h"
#include "TypeConstructor.h"
#include "NestedList.h"
#include "ListUtils.h"

extern SecondoInterface* si;
using namespace std;

namespace gta
{

class DistData; // forward declaration

/*
Refers the default distdata-type for the each type constructor.

*/
const string DDATA_DEFAULT("default");

/*
This value is returned from "DistDataReg::defaultName()"[4], if no default distdata-type has been found for the resp. type constructor.

*/
const string DDATA_UNDEFINED("n/a");

/////////////////////////////////////////////////////////////////////
// Name and short descriprions for the defined distdata-types:
/////////////////////////////////////////////////////////////////////

// native data representation, e.g. an int value for CcInt attributes
// (this type usually used, if only one distdata-type is defined for
// the respective type constructor).
const string DDATA_NATIVE("native");
const string DDATA_NATIVE_DESCR(
        "native representation of the respective datatype");

/////////////////////////////////////////////////////////////////////
// Constants for the distdata-type id's
//
// For each distdata-type name above one unique integer constant
// should be defined below. These constants are stored within
// "DistDataAttribute"[4] objects instead of the whole name to save
// some memory (by the way this allows to change the name of the
// distdata-types without affecting the "DistDataAttribute"[4]
// objects, as long as the assigned id will not be changed).
/////////////////////////////////////////////////////////////////////
const int DDATA_NATIVE_ID = 0;

// id's for the picture algebra types (defined in PictureFuns.h)
const int DDATA_HSV8_ID         = 1000;
const int DDATA_HSV16_ID         = 1001;
const int DDATA_HSV32_ID         = 1002;
const int DDATA_HSV64_ID         = 1003;
const int DDATA_HSV128_ID        = 1004;
const int DDATA_HSV256_ID        = 1005;
const int DDATA_LAB256_ID        = 1011;
const int DDATA_HSV8_NCOMPR_ID   = 1020;
const int DDATA_HSV16_NCOMPR_ID  = 1021;
const int DDATA_HSV32_NCOMPR_ID  = 1022;
const int DDATA_HSV64_NCOMPR_ID  = 1023;
const int DDATA_HSV128_NCOMPR_ID = 1024;
const int DDATA_HSV256_NCOMPR_ID = 1025;
const int DDATA_LAB256_NCOMPR_ID = 1031;

typedef DistData* (*GetDataFun)(const void* attr);

/////////////////////////////////////////////////////////////////////
// Flags for the DistDataInfo class
/////////////////////////////////////////////////////////////////////

// this flag is set for all defined info objects
// (automatically assigned from the constructor)
const char DDATA_IS_DEFINED = (1 << 0);



/********************************************************************
1.1.1 Class ~DistDataId~

********************************************************************/
class DistDataId
{

public:
/*
Default constructor (should not be used).

*/
    inline DistDataId()
    {}

/*
Constructor (creates an undefined id - the "defined"[4] parameter is only needed to distinguish this constructor from the default constructor).

*/
    inline DistDataId(bool defined)
        : m_algebraId(-1)
    {}

/*
Constructor.

*/
    inline DistDataId(int algebraId, int typeId, int distdataId)
    : m_algebraId(algebraId), m_typeId(typeId),
      m_distdataId(distdataId)
    {}

/*
Returns "true"[4], if the id's are equal.

*/
    bool inline operator == (const DistDataId& rhs) const
    {
        return (m_algebraId == rhs.m_algebraId) &&
               (m_typeId == rhs.m_typeId) &&
               (m_distdataId == rhs.m_distdataId);
    }

/*
Returns "true"[4], if the id's are not equal.

*/
    bool inline operator != (const DistDataId& rhs) const
    { return !operator == (rhs); }

/*
Returns "true"[4], if the assigned distdata type is defined (in this case "m[_]algebraId"[4] must be positive).

*/
    inline bool isDefined() const
    { return (m_algebraId >= 0); }

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
Returns the id of the assigned distdata type.

*/
    inline int distdataId() const
    { return m_distdataId; }

private:
    int m_algebraId;  // algebra-id of the assigned type constructor
    int m_typeId;     // type-id of the assigned type constructor
    int m_distdataId; // id of the distdata type
}; // class DistDataId



/********************************************************************
1.1.1 Class ~DistDataId~

********************************************************************/
class DistData
{

public:
/*
Constructor (initiates the object with a copy of the given "char"[4] array).

*/
    inline DistData(size_t size, const void* value)
        : m_size(size), m_value(new char[m_size])
    { memcpy(m_value, value, m_size); }

/*
Constructor (initiates the object with the given string).

*/
    inline DistData(const string value)
        : m_size(value.size()+1), m_value(new char[m_size])
    { memcpy(m_value, value.c_str(), m_size); }

/*
Constructor (reads the object from "buffer"[4] and increases "offset"[4]).

*/
    DistData(const char *buffer, int& offset)
    {
        // read m_size
        memcpy(&m_size, buffer+offset, sizeof(size_t));
        offset += sizeof(size_t);

        // read m_value
        m_value = new char[m_size];
        memcpy(m_value, buffer+offset, m_size);
        offset += m_size;
    }

/*
Default copy constructor.

*/
    inline DistData(const DistData& e)
    : m_size (e.m_size), m_value(new char[e.m_size])
    { memcpy(m_value, e.m_value, e.m_size); }

/*
Destructor.

*/
    inline ~DistData()
    { delete [] m_value; }

/*
Returns a copy of "this"[4].

*/
    inline DistData *clone()
    { return new DistData(*this); }

/*
Returns a reference to the stored data array.

*/
    inline const void *value() const
    { return m_value; }

/*
Returns the size of the data array

*/
    inline size_t size() const
    { return m_size; }

/*
Writes the object to "buffer"[4] and increases "offset"[4].

*/
    void write(char *buffer, int& offset) const
    {
        // write m_size
        memcpy(buffer+offset, &m_size, sizeof(size_t));
        offset += sizeof(size_t);

        // write m_value
        memcpy(buffer+offset, m_value, m_size);
        offset += m_size;
    }

private:
    size_t m_size;  // length of the data array
    char * m_value; // the data array
};



/********************************************************************
1.1.1 Class ~DistDataAttribute~

********************************************************************/
class DistDataAttribute : public Attribute
{

public:
/*
Default constructor (should not be used, except for the cast method).

*/
    inline DistDataAttribute() {}

/*
Constructor (creates an undefined distdata object)

*/
    inline DistDataAttribute(size_t size)
    : Attribute(false), m_data(0)
    {}

/*
Default copy constructor.

*/
    DistDataAttribute(const DistDataAttribute& ddAttr)
    : Attribute(ddAttr.IsDefined()),
      m_data(ddAttr.m_data.getSize()),
      m_distdataId(ddAttr.m_distdataId)
    {
        if(IsDefined()){
            m_data.copyFrom(ddAttr.m_data);
        }
    }

/*
Destructor.

*/
    inline ~DistDataAttribute()
    {}

/*
Returns a copy of the object.

*/
    inline DistDataAttribute *clone() const
    { return new DistDataAttribute(*this); }

/*
Sets the attribute values to the given values.

*/
    void set(
            bool defined, const char *data, size_t size,
            DistDataId distdataId);

/*
Sets the attribute values to the given values.

*/
    void set(bool defined, DistData *data, DistDataId distdataId);

/*
Returns id of the assigned distance function.

*/
    inline DistDataId distdataId() const
    { return m_distdataId; }


/*
Returns the size of the data object in bytes.

*/
    inline size_t size() const
    { return m_data.getSize(); }

/*
Removes the disc representation of the data FLOB.

*/
    inline void deleteFLOB()
    { m_data.destroy(); }

/////////////////////////////////////////////////////////////////////
// virtual methods from the Attribute class:
/////////////////////////////////////////////////////////////////////

    inline virtual size_t Sizeof() const
    { return sizeof(*this); }

    inline virtual bool Adjacent(const Attribute *attr) const
    { return false; }

    inline virtual Attribute *Clone() const
    { return clone(); }

    inline virtual int NumOfFLOBs() const
    { return 1; }

    inline virtual Flob *GetFLOB(const int i)
    { return &m_data; }

    inline virtual int Compare(const Attribute *rhs) const
    { return 0; }

    inline virtual size_t HashValue() const
    { return 0; }

    virtual void CopyFrom(const Attribute *rhs);

    virtual char* getData(){
         return m_data.getData();
    }

    static const string BasicType() { return "distdata"; }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }

private:
    Flob m_data; // contains the data array
    DistDataId m_distdataId;
}; // class DistDataAttribute



/********************************************************************
1.1.1 Class ~DistDataInfo~

********************************************************************/
class DistDataInfo
{

public:
/*
Default constructor (creates an undefined info object)

*/
    DistDataInfo();

/*
Constructor (creates a new info object with the given values)

*/
    DistDataInfo(const string& name, const string& descr,
                 int distdataId, const string& typeName,
                 const GetDataFun getDataFun, char flags = 0);

/*
Returns the name of the distdata-type.

*/
    inline string name() const
    { return m_name; }

/*
Returns the description of the distdata-type.

*/
    inline string descr() const
    { return m_descr; }

/*
Returns the assigned getdata function.

*/
    inline GetDataFun getDataFun() const
    { return m_getDataFun; }

/*
Generates a new "DistData" object of the assigned type from "attr"[4].

*/
    inline DistData *getData(const void *attr)
    { return m_getDataFun(attr); }

/*
Returns the id of the "DistDataInfo"[4] object.

*/
    inline DistDataId id() const
    { return m_id; }

/*
Returns the id of the assigned type constructor.

*/
    inline int typeId() const
    { return m_id.typeId(); }

/*
Returns the name of the assigned type constructor.

*/
    inline string typeName() const
    { return m_id.typeName(); }

/*
Returns the id of the assigned algebra.

*/
    inline int algebraId() const
    { return m_id.algebraId(); }

/*
Returns the name of the assigned algebra.

*/
    inline string algebraName() const
    { return m_id.algebraName(); }

/*
Returns the id of the distdata-type.

*/
    inline int distdataId() const
    { return m_id.distdataId(); }

/*
Returns "true"[4], if the "DistDataInfo"[4] object is defined (should only be "false"[4], if a requested distdata-type could not be found in the "DistDataReg"[4] class).

*/
    inline bool isDefined() const
    { return (m_flags & DDATA_IS_DEFINED); }

private:
    string m_name;  // unique name (used for selection)
    string m_descr; // short description
    GetDataFun m_getDataFun; // assigned getdata function
    DistDataId m_id;
    char m_flags;
}; // class DistDataInfo


/********************************************************************
1.1.1 Class ~DistDataReg~

********************************************************************/
class DistDataReg
{

friend class DistfunReg;

public:
/*
Initializes the defined distdata types.

*/
    static void initialize();

/*
Returns "true"[4], if the "initialize"[4] function  has already been called.

*/
    static inline bool isInitialized()
    { return initialized; }

/*
Adds a new "DistDataInfo"[4] object.

*/
    static void addInfo(DistDataInfo info);

/*
Returns the name of the default distdata type for the specified type.

*/
    static string defaultName(const string& typeName);

/*
Returns the "DistDataId"[4] of the specified "DistDataInfo"[4] object.

*/
    static DistDataId getId(
            const string& typeName, const string& distdataName);

/*
Returns the "DistDataInfo"[4] object for the specified id.

*/
    static DistDataInfo& getInfo(DistDataId id);

/*
Returns the specified "DistDataInfo"[4] object.

*/
    static inline DistDataInfo& getInfo(
            const string& typeName, const string& distdataName)
    { return getInfo(getId(typeName, distdataName)); }

/*
Returns "true"[4], if the specified "DistDataInfo"[4] object does exist.

*/
    static inline bool isDefined(
            const string& typeName, const string& distdataName)
    { return getInfo(typeName, distdataName).isDefined(); }

/*
Returns a string with a list of all defined distdata types for the specified type (could e.g. be used in the type checking functions in case of errors to show possible values).

*/
    static string definedNames(const string& typeName);

private:
    typedef map<string, DistDataInfo>::iterator distdataIter;

    static bool initialized;
    static map<string, int> distdataIds;
    static DistDataInfo defaultInfo;
    static map<string, DistDataInfo> distdataInfos;
    static map<string, string> defaultNames;

/////////////////////////////////////////////////////////////////////
// Defined getdata functions:
/////////////////////////////////////////////////////////////////////
/*
Getdata function for the "int"[4] type constructor.

*/
    static DistData *getDataInt(const void *attr);

/*
Getdata function for the "real"[4] type constructor.

*/
    static DistData *getDataReal(const void *attr);


/*
Getdata function for the "points"[4] type constructor.

*/
    static DistData *getDataPoints(const void *attr);

/*
Getdata function for the "hpoint"[4] type constructor.

*/
    static DistData *getDataHPoint(const void *attr);

/*
Getdata function for the "string"[4] type constructor.

*/
    static DistData *getDataString(const void *attr);
}; // class DistDataReg

} //namespace gta

#endif // #ifndef __DISTDATA_REG_H__
