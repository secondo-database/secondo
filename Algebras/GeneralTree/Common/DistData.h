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
//paragraph [11] Title: [{\Large \bf \begin{center}] [\end{center}}]
//[TOC] [\tableofcontents]

[11] Headerfile "DistData.h"[4]

January-February 2008, Mirko Dibbert

[TOC]
\newpage

1 Overview

This headerfile contains the "DistData"[4] datatype, which stores the data for the distance functions defined in "DistFuns.h"[4]. Each "DistData"[4] object contains the memory representation of the respective data structure (e.g. histograms for the "picture"[4] type constructor distance functions), which is stored as "char"[4] array.

Below, two classes are defined for this purpose: The "DistData"[4] class should be used for internal representation and contains only the data itself. The "DistDataAttr"[4] class implements the "distdata"[4] type constructor and additionally stores the algebra- and type-id of the type-constructor and the id of the distance function, that had been used to create the object. Furthermore, the data is stored within a "FLOB"[4] object instead of a char array, since dynamically data is not allowed within attributes.

2 Includes and defines

*/
#ifndef __DISTDATA_H
#define __DISTDATA_H

#include <iostream>
#include <string>
#include "SmartPtr.h"
#include "StandardTypes.h"
#include "StandardAttribute.h"
#include "SecondoInterface.h"
#include "DistDataId.h"

extern SecondoInterface* si;

using namespace std;

/********************************************************************
3 Class declarations

3.1 Class "DistData"[4]

This class provides the following methods:

---- const void* value() const
     size_t size() const
     void write(char* buffer, int& offset) const
     DistData* clone()
----

The "value"[4] method returns a reference to the data array. The "size"[4] method returns the count of chars in the data array. The "write"[4] method writes the object to "buffer"[4] and increases "offset"[4]. Finally, the "clone"[4] method returns a copy of the "DistData"[4] object.

The following constructors are avaliable:

---- DistData(size_t size, const void* value)
     DistData(const string value)
     DistData(const char* buffer, int& offset)
     DistData(const DistData& e)
----
The first constructor copies the "value"[4] array to the char buffer. This constructor is needed in most cases and could e.g. be used to copy the memory representation of any datatype. The only restriction is, that the whole data must be stored in a continuous memory representation and the total size of the respective memory area must be known.

The second constructor initiates the char buffer with a string, which is in particular useful for debugging purposes (the string will be stored as null-terminated string).

The third constructor reads a previously written "DistData"[4] object from "buffer"[4] and increases offset.

The last constructor is the default copy constructor, which returs a deep copy of the object.

********************************************************************/

namespace general_tree
{

class DistData
{

public:
  inline DistData(size_t size, const void* value)
  : m_size(size), m_value(new char[m_size])
  { memcpy(m_value, value , m_size); }
/*
Constructor (copies "value"[4] array).

*/

/*
Constructor (initiates data with "value"[4] string).

*/
  inline DistData(const string value)
  : m_size(value.size()+1), m_value(new char[m_size])
  { memcpy(m_value, value.c_str(), m_size); }

/*
Constructor (reads the object from "buffer"[4] and increases "offset"[4]).

*/
  DistData(const char* buffer, int& offset)
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
  { delete m_value; }

  inline DistData* clone()
  { return new DistData(*this); }
/*
Returns a copy of the actual object.

*/

/*
Returns a void pointer to the data array stored in m[_]value.

*/
  inline const void* value() const
  { return m_value; }

/*
Returns the size of the data array

*/
  inline size_t size() const
  { return m_size; }

/*
Writes the object to "buffer"[4] and increases "offset"[4].

*/
  void write(char* buffer, int& offset) const
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
  char*  m_value; // the data array
};

/******************************************************************************
3.2 Class "DistDataAttr"[4]

This class implementes the "distdata"[4] type constructor of the "GeneralTree"[4] algebra. Additionally to the methods of the "DistData"[4] class and the "StandardAttribute"[4] methods, the class provides the following methods:

---- void set(bool defined, const char* data, size_t size,
              string& typeName, int distfunId)
----
This method should be used to assign new values to the method. The "distfunId"[4] parameter could be obtained from the respective method in the "DistFuns"[4] class, which expects the algebra-id, type-id and distance function name as parameter.

---- int algebraId() const
     int typeId() const
     int distfunId() const
----
These methods return the respective id's, which is needed to check if a given "distdata" attribute has the right type.

---- void deleteFLOB()
----
This method is only needed in the "DeleteDistData"[4] method of the "distdata"[4] type constructor to remove the flob.
\\[3ex]
Beyond the copy constructor, only the following constructor is avaliable (the standard constructor should not be used):

---- DistDataAttr(size_t size)
----
This constructor should be called with the assumed flob size or with 0 (if necessary, the flob will be resized in the set method).

******************************************************************************/
class DistDataAttr : public StandardAttribute
{

public:
/*
Default constructor (should not be used, except for the cast method).

*/
  inline DistDataAttr()
  {}

/*
Constructor (use this constructor to create new objects, if size is unknown use 0)

*/
  inline DistDataAttr(size_t size)
  : m_data(size), m_defined(false)
  {}

/*
Default copy constructor.

*/
  DistDataAttr(const DistDataAttr& ddAttr)
  : m_data(ddAttr.m_data.Size()),
    m_defined(ddAttr.m_defined),
    m_distdataId(ddAttr.m_distdataId)
  {
    if(IsDefined())
    {
      m_data.Put(0, ddAttr.size(), ddAttr.value());
    }
  }

/*
Destructor.

*/
  inline ~DistDataAttr()
  {}

/*
Returns a copy of the object.

*/
  inline DistDataAttr* clone() const
  { return new DistDataAttr(*this); }

/*
Sets the attribute values to the given values.

*/
  void set(bool defined, const char* data, size_t size,
           DistDataId distdataId)
  {
    m_defined = defined;
    if(defined)
    {
      m_data.Clean();
      m_data.Resize(size);
      m_data.Put(0, size, data);

      m_distdataId = distdataId;
    }
  }

/*
Sets the attribute values to the given values.

*/
  void set(bool defined, DistData* data, DistDataId distdataId)
  {
    m_defined = defined;
    if(defined)
    {
      m_data.Clean();
      m_data.Resize(data->size());
      m_data.Put(0, data->size(), data->value());

      m_distdataId = distdataId;
    }
  }

/*
Returns id of the assigned distance function.

*/
  inline DistDataId distdataId() const
  { return m_distdataId; }

/*
Returns a reference to the data representation.

*/
  inline const char* value() const
  {
    const char* data;
    m_data.Get(0, &data);
    return data;
  }

/*
Returns the size of the data object in bytes.

*/
  inline size_t size() const
  { return m_data.Size(); }

/*
Removes the disc representation of the data FLOB.

*/
  inline void deleteFLOB()
  { m_data.Destroy(); }

/*
Implementation of virtual methods from the StandardAttribute class:

*/
  inline virtual bool IsDefined() const
  { return m_defined; }

  inline virtual void SetDefined(bool defined)
  { m_defined = defined; }

  inline virtual size_t Sizeof() const
  { return sizeof(*this); }

  inline virtual bool Adjacent(const Attribute* attr) const
  { return false; }

  inline virtual Attribute* Clone() const
  { return clone(); }

  inline virtual int NumOfFLOBs() const
  { return 1; }

  inline virtual FLOB* GetFLOB(const int i)
  { return &m_data; }

  inline virtual int Compare(const Attribute* rhs) const
  { return 0; }

  virtual size_t HashValue() const
  { return 0; }

  virtual void CopyFrom(const StandardAttribute* rhs)
  {
    const DistDataAttr* ddAttr =
        static_cast<const DistDataAttr*>(rhs);

    if(ddAttr->IsDefined())
    {
      m_data.Clean();
      m_data.Resize(ddAttr->size());
      m_data.Put(0, ddAttr->size(), ddAttr->value());

      m_defined = true;
      m_distdataId = ddAttr->distdataId();
    }
  }

private:
  FLOB m_data;     // contains the data of the DistData object
  bool m_defined;  // true, if the attribute is defined
  DistDataId m_distdataId;
}; // class DistDataAttr

} // namespace general_tree
#endif // #ifndef __DISTDATA_H
