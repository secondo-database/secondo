
/*
----
This file is part of SECONDO.

Copyright (C) 2016, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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


#ifndef ATTRIBUTEFILE_H
#define ATTRIBUTEFILE_H

#include <string>
#include "NestedList.h"
#include "Attribute.h"



/*
1 AttributeIterator

This iterator class can be used for iterating over files
constructed by the AttributeFile class.


*/

class AttributeIterator{


 public:

   AttributeIterator(const std::string& _filename);
   AttributeIterator(const std::string& _filename,
                     const uint32_t _algId, 
                     const uint32_t _typeId,
                     const ListExpr _typeList);

   ~AttributeIterator(){
      if(in){
        delete in;
      }
      if(buffer){
         delete[] buffer;
      }
    }

   inline Attribute* next(){
     return in?retrieveNext():0;
   }

   std::streampos getPos(){
      if(!in){
        return -1;
      }
      return in->tellg();
   }

 private:
   std::string fileName;
   uint32_t algId;
   uint32_t typeId;
   ListExpr typeList;
   std::ifstream* in;

   Attribute* retrieveNext();
   char* buffer;
   

};

/*
2 AttributeFile

This class can be used to store attributes of the same type within a file.

*/

class AttributeFile{

  public:
     AttributeFile(const std::string& _filename,
                   const uint32_t _algId,
                   const uint32_t _typeId,
                   const ListExpr _typeList,
                   const ListExpr _numTypeList,
                   const bool temp);

     ~AttributeFile();


     bool append(Attribute* attr, bool withFlobs);


     AttributeIterator* makeScan( bool close=true){
        if(close && out ){
           out->flush();
           out->close();
           if(buffer){
             delete[] buffer;
             buffer = 0;
           }
           delete out;
           out = 0;
        }
        return new AttributeIterator(filename, algId, typeId, nType);
     }

     void close(){
        if(out){
           out->flush();
           out->close();
           if(buffer){
             delete[] buffer;
             buffer = 0;
           }
           delete out;
           out = 0;
        }
     }


     std::streampos getPos(){
        if(!out){
          return -1;
        }
        return out->tellp();
     }

  private:
     std::string filename;
     uint32_t algId;
     uint32_t typeId;
     ListExpr typeList;
     ListExpr nType;
     bool temp;
     std::ofstream* out;
     char* buffer;
};

#endif

