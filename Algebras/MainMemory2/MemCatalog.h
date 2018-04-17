/*

----
This file is part of SECONDO.

Copyright (C) 2018, 
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


#ifndef MEMCATALOG_H
#define MEMCATALOG_H

#include <stdint.h>
#include <map>
#include <string>
#include "NestedList.h"

namespace mm2algebra{

class MemoryObject;
class MPointer;

class MemCatalog {

    public:

        void setMemSizeTotal(size_t size);

        size_t getMemSizeTotal();

        unsigned long getUsedMemSize(); //in Byte

        void addToUsedMemSize(int i){
            usedMemSize = usedMemSize + i;
        }

        unsigned long getAvailableMemSize();  //in Byte

        std::map<std::string,MemoryObject*>* getMemContent();

        bool insert (const std::string& name, MemoryObject* obj);
        bool insert (const std::string& name, MPointer* obj);

        bool modify(const std::string& name, MemoryObject* obj);

        bool deleteObject (const std::string& name);

        void clear ();

        bool isMMOnlyObject(const std::string& objectName);

        bool isObject(const std::string& objectName);

        //*Precondition*: "isMMObject( objectName ) == true"
        MemoryObject* getMMObject(const std::string& objectName)const;

        ListExpr getMMObjectTypeExpr(const std::string& oN);

        bool isAccessible(const std::string& name);

        static MemCatalog* getInstance(); 

        static void destroyInstance();

        bool clone(const std::string& name, MemoryObject* value);

        bool isReserved(const std::string& name) const;
        
        bool isNull(const std::string& name) const;

        size_t getNoReferences(std::string name) const;

    private:
        unsigned long usedMemSize;  //in Byte
        size_t memSizeTotal; //in MB
        std::map<std::string,MemoryObject*> memContents;
       
        static MemCatalog* instance;
 
        MemCatalog (){
          memSizeTotal=256;  //main memory size in MB
          usedMemSize=0;     //used main memory size in B
        };
        ~MemCatalog();
};


}

#endif
