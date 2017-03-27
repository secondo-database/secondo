

/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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


//[$][\$]

*/

#include "FileAttribute.h"
#include "Attribute.h"
#include <iostream>

#include "Algebras/FText/FTextAlgebra.h"


using namespace std;


   bool FileAttribute::saveAttribute(ListExpr type, Attribute* value, 
                             const string& fileName){

       // format is batt<tl><t><al><data> with
       // tl : length of the type description
       // t  : type description as a nested list
       // al : lengths of the root record
       // <data> : attribute data: root record + flobs

      string t = nl->ToString(type);
      return saveAttribute(t, value, fileName);
  }



   bool FileAttribute::saveAttribute(const string&  type, Attribute* value, 
                             const string& fileName){
  
      ofstream out(fileName.c_str(),ios::out|ios::binary);
      size_t bs = 1024*1024;
      char* buffer = new char[bs];
      memset(buffer,0,bs);
      out.rdbuf()->pubsetbuf(buffer, bs);
      if(!out.good()){
         delete[] buffer;
         return false;
      }
      out.write("batt",4);
      uint32_t l = type.size();
      out.write((char*)&l, 4);
      out.write(type.c_str(),l);
      if(value->GetStorageType() == Attribute::Default){
         uint8_t dv = 1;
         out.write((char*) &dv,1);
         l = value->Sizeof();


         out.write((char*)&l, 4);
         out.write((char*)value, l);
         // process FLOBS
         for(int i=0;i<value->NumOfFLOBs(); i++){
            Flob* flob = value->GetFLOB(i);
            char* data = flob->getData();
            out.write(data, flob->getSize());
            delete[] data;
         }
      } else {
         uint8_t dv = 0;
         out.write((char*) &dv,1);
         uint32_t  sz = value->SerializedSize();
         out.write((char*) &sz, 4);
         char* vb = new char[sz];
         value->Serialize(vb,sz,0);
         out.write(vb,sz);
         delete[] vb;
      }
 
      out.flush();
      bool res = out.good();
      out.close();
      delete[] buffer;
      return res;
   }  


   ListExpr FileAttribute::getType(const string& fileName){
       ifstream in(fileName.c_str(), ios::in|ios::binary);
       if(!in){
           return 0;
       }
       size_t bs = 1024*1024;
       char* buffer = new char[bs];
       in.rdbuf()->pubsetbuf(buffer, bs);
       char b1[4]; 
       in.read(b1,4);
       string magic(b1,4);
       if(magic!="batt"){
         in.close();
         delete[] buffer;  // not a binary attribute file
         return nl->Empty(); 
       }
       uint32_t l;
       in.read((char*) &l, 4);
       char* tb = new char[l];
       in.read(tb,l);
       string typestr(tb,l);
       delete[] tb;
       ListExpr type;
       if(!nl->ReadFromString(typestr,type) ){
          in.close();
          delete[] buffer;
          return nl->Empty();
       }
       in.close();
       delete[] buffer;
       return type; 
    }

   Attribute* FileAttribute::restoreAttribute(ListExpr& type, 
                                              const string& fileName){
       ifstream in(fileName.c_str(), ios::in|ios::binary);
       if(!in){
           return 0;
       }
       size_t bs = 1024*1024;
       char* buffer = new char[bs];
       in.rdbuf()->pubsetbuf(buffer, bs);
       char b1[4]; 
       in.read(b1,4);
       string magic(b1,4);
       if(magic!="batt"){
         in.close();
         delete[] buffer;  // not a binary attribute file
         return 0; 
       }
       // read length of the list
       uint32_t l;
       in.read((char*) &l, 4);
       char* tb = new char[l];
       in.read(tb,l);
       string typestr(tb,l);
       delete[] tb;

       if(!nl->ReadFromString(typestr,type) ){
          in.close();
          delete[] buffer;
          return 0;
       }

       if(!listutils::isDATA(type)){
          in.close();
          delete[] buffer;
          return 0;
       }

       // extract algid and type idA
       int algId;
       int typeId;
       string typeName;
       if(!SecondoSystem::GetCatalog()->LookUpTypeExpr(type,
                                            typeName, algId, typeId)){
          // type currently not availlable
          in.close();
          delete[]  buffer;
          return 0;
       }

       // get size of root record
       uint8_t dv;
       in.read((char*) &dv, 1);
       Attribute* attr = 0;
       ListExpr ntype = SecondoSystem::GetCatalog()->NumericType(type);

       if(dv){
           in.read((char*) &l, 4);
           char* root = new char[l];
           in.read(root, l);
           if(!in.good()){
              in.close();
              delete[] buffer;
              return 0;
           }  
           AlgebraManager* am = SecondoSystem::GetAlgebraManager();

           attr = (Attribute*)(am->CreateObj(algId,typeId)(ntype)).addr;


           attr = attr->Create(attr, root, l, algId,typeId);
           delete[] root;

           for(int i=0;i< attr->NumOfFLOBs(); i++){
             Flob* flob = attr->GetFLOB(i);
             size_t size = flob->getSize();
             flob->kill();
             char* fb = new char[size];
             in.read(fb,size);
             flob->createFromBlock(*flob,fb, size,false);
             delete[] fb;
           }


       } else {
         uint32_t sz; 
         in.read((char*) &sz, 4);
         char* av = new char[sz];
         in.read(av,sz);
         size_t offset =0;
         attr = Attribute::Create(av,offset,ntype);
         delete[] av;
       }
       delete[] buffer; 
       return attr;

   }

