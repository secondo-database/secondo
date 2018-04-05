/*

----
This file is part of SECONDO.

Copyright (C) 2016, 
University in Hagen, 
Faculty of Mathematics and Computer Science,
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

*/

#ifndef MEMORYOBJECT_H
#define MEMORYOBJECT_H

#include <string>
#include "NestedList.h"

namespace mm2algebra{


class MemoryObject {
    public:
    MemoryObject(){
        flob = false;
        database="";
        memSize=0;
        objectTypeExpr="";
    }

    MemoryObject(bool _flob, const std::string& _database,
                 const std::string& _type) : memSize(0), 
                 objectTypeExpr(_type), flob(_flob),
                 database(_database) {}

    virtual ~MemoryObject();

    unsigned long getMemSize ();

    std::string getObjectTypeExpr();
    void setObjectTypeExpr(std::string _oTE);
    std::string getDatabase();
    bool hasflob();
    ListExpr getType() const;

    protected:
        unsigned long memSize;      // object size in main memory in byte
        std::string objectTypeExpr;  // the tuple description for relations,
                                     // or the attribute description

        bool flob;
        std::string database;

};

}

#endif

