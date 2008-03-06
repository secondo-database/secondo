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

[11] Headerfile "BBox.h"[4]

January-February 2008, Mirko Dibbert

[TOC]
\newpage

1 Overview

This headerfile contain the implementation of bounding boxes, which is e.g. needed for the x-tree algebra. The "BBox"[4] class is the base class for all bounding boxes and contains common members and virtual methods for all bounding boxes. The "GenericBBox"[4] class implements bounding boxes of various domains and should be sufficent for most use cases. Each domain should correspond to one id in the "BBoxType"[4] enumeration, which is used in the "BBoxCreator"[4] class to create a new bounding box of the respective type.

If the "GenericBBox"[4] class is not sufficient, it is also possible to implement individual bouding box classes. In this case, the "BBoxType"[4] enum must contain an id for the new class and a new switch case which handles this id must be added to the "newBBox"[4] method of the "BBoxCreater"[4] class. This is e.g. done for the "HistBBox"[4] class, which implement bounding boxes for histograms, that store the data in a compressed form.

1 includes and defines

*/
#ifndef __BOUNDING_BOX_H
#define __BOUNDING_BOX_H

#include <iostream>
#include "StandardTypes.h"

using namespace std;

namespace generalTree
{

/********************************************************************
1 Class declarations

1.1 Class BBox

This class is the base class for bounding boxes, which is e.g. used in the x-tree algebra. All bounding box classes must be derived from this class. The reason to provide different bounding box classes is, that individual bounding box classes could save some memory (e.g. when char arrays are sufficient instead of double values), which would also save some I/O costs, and increases the flexibility.

TODO:
This class and the derived classes are still in development and curently no further methods except of the read/write methods are implemented

********************************************************************/
class BBox
{
public:
    inline BBox()
    {}

    virtual inline ~BBox()
    {}

    virtual size_t size() = 0;

    virtual void write(char* const buffer, int& offset) const = 0;
    virtual void read(const char* const buffer, int& offset) = 0;

    virtual bool intersects(BBox* rhs) = 0;
    virtual bool contains(BBox* rhs) = 0;
};

/********************************************************************
1.1 Class GenericBBox

This class implements the default bounding boxes.

********************************************************************/

template<class domainT, unsigned dim>
class GenericBBox : public BBox
{
public:
    typedef domainT domain_type;

/*
Default constructor.

*/
    inline GenericBBox()
    {}

/*
Constructor.

*/
    inline GenericBBox(domainT* lb, domainT* ub)
    {
        memcpy(m_lb, lb, dim*sizeof(domainT));
        memcpy(m_ub, ub, dim*sizeof(domainT));
    }

/*
Constructor (reads the bbox from buffer and increases offset).

*/
    inline GenericBBox(const char* buffer, int& offset)
    { read(buffer, offset); }

/*
Destructor.

*/
    inline ~GenericBBox()
    {}

/*
Returns the dimension of the bounding box.

*/
    inline unsigned getDim()
    { return dim; }

/*
TODO enter method description

*/
    virtual bool intersects(BBox* bbox)
    {
        // TODO not yet implemented
        return false;
    }

/*
TODO enter method description

*/
    virtual bool contains(BBox* bbox)
    {
        // TODO not yet implemented
        return false;
    }

/*
TODO enter method description

*/
    inline size_t size()
    { return 2*dim*sizeof(domainT); }

    void write(char* buffer, int& offset) const
    {
        memcpy(buffer+offset, m_lb, dim*sizeof(domainT));
        offset += dim * sizeof(domainT);

        memcpy(buffer+offset, m_ub, dim*sizeof(domainT));
        offset += dim * sizeof(domainT);
    }

/*
TODO enter method description

*/
    void read(const char* buffer, int& offset)
    {
        memcpy(m_lb, buffer+offset, dim*sizeof(domainT));
        offset += dim * sizeof(domainT);

        memcpy(m_ub, buffer+offset, dim*sizeof(domainT));
        offset += dim * sizeof(domainT);
    }

protected:
    domainT m_lb[dim];  // array of lower bounds
    domainT m_ub[dim];  // array of upper bounds
}; // class BBox

} // namespace generalTree
#endif // #ifndef __BOUNDING_BOX_H
