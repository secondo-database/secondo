

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


/*

1 Implementation of ~AttributeIterator~

*/


  #include "AttributeFile.h"
  #include "FileSystem.h"


  #define AF_FILEBUFFERSIZE 1048576


  AttributeIterator::AttributeIterator(const std::string& _filename):
      fileName(_filename){
      buffer = 0;
      in = new std::ifstream(fileName.c_str(), std::ios::in | std::ios::binary);
      if(!in->good()){
         delete in;
         in = 0;
         return;
      }
      in->read((char*)&algId, sizeof(uint32_t));
      in->read((char*)&typeId, sizeof(uint32_t));
      uint32_t l;
      in->read((char*)&l, sizeof(uint32_t));
      char* t = new char[l];
      in->read(t,l);
      bool ok = nl->ReadFromString(std::string(t,l),typeList);
      delete[] t;
      if(!ok){
         delete in;
         in = 0;
      }  
      buffer = new char[AF_FILEBUFFERSIZE];
      in->rdbuf()->pubsetbuf(buffer,AF_FILEBUFFERSIZE);
   }

   AttributeIterator::AttributeIterator(const std::string& _filename,
                     const uint32_t _algId, 
                     const uint32_t _typeId,
                     const ListExpr _typeList): fileName(_filename){
      buffer = 0;
      in = new std::ifstream(fileName.c_str(), std::ios::in | std::ios::binary);
      if(!in->good()){
         delete in;
         in = 0;
         return;
      }
      buffer = new char[AF_FILEBUFFERSIZE];
      in->rdbuf()->pubsetbuf(buffer,AF_FILEBUFFERSIZE);
      in->read((char*)&algId, sizeof(uint32_t));
      in->read((char*)&typeId, sizeof(uint32_t));
      uint32_t l;
      in->read((char*)&l, sizeof(uint32_t));
      char* t = new char[l];
      in->read(t,l);
      typeList = _typeList;
      delete[] t;
      if(algId!=_algId || typeId!=_typeId){
          delete in;
          in = 0;
          delete[] buffer;
          buffer = 0;
      }
      
   }
   

   Attribute* AttributeIterator::retrieveNext(){
      if(!in){
         return 0;
      }
      uint8_t dv;
      in->read((char*) &dv,1);
      if(!in->good()){
        return 0;
      }
      if(dv>2){
         return 0;
      } 
      switch(dv){
         case 0: {  // default with flobs
             uint32_t l;
             in->read((char*) &l, 4);
             char* root = new char[l];
             in->read(root, l);
             if(!in->good()){
                return 0;
             }
             AlgebraManager* am = SecondoSystem::GetAlgebraManager();

             Attribute* attr = (Attribute*)
                           (am->CreateObj(algId,typeId)(typeList)).addr;
             attr = attr->Create(attr, root, l, algId,typeId);
             delete[] root;

             for(int i=0;i< attr->NumOfFLOBs(); i++){
                Flob* flob = attr->GetFLOB(i);
                size_t size = flob->getSize();
                flob->kill();
                char* fb = new char[size];
                in->read(fb,size);
                flob->createFromBlock(*flob,fb, size,false);
                delete[] fb;
             }
             return attr;
           }

         case 1: {  // default without flobs
             uint32_t l;
             in->read((char*) &l, 4);
             char* root = new char[l];
             in->read(root, l);
             if(!in->good()){
                return 0;
             }
             AlgebraManager* am = SecondoSystem::GetAlgebraManager();

             Attribute* attr = (Attribute*)
                                  (am->CreateObj(algId,typeId)(typeList)).addr;
             attr = attr->Create(attr, root, l, algId,typeId);
             delete[] root;
             return attr;
          }

         case 2:{ // serialized 

              uint32_t sz;
              in->read((char*) &sz, 4);
              char* av = new char[sz];
              in->read(av,sz);
              size_t offset =0;
              Attribute* attr = Attribute::Create(av,offset,typeList);
              delete[] av;
              return attr;
         }
      }
      return 0;

   }



/*
2 Implementation of AttributeFile

*/

  AttributeFile::AttributeFile(const std::string& _filename,
                               const uint32_t _algId,
                               const uint32_t _typeId,
                               const ListExpr _typeList,
                               const bool _temp):
     filename(_filename), algId(_algId), typeId(_typeId),
     typeList(_typeList), temp(_temp){

     buffer = 0;
     out = new std::ofstream(filename.c_str(), std::ios::out 
                                               | std::ios::binary);
     if(!out->good()){
         cout << "problem in creating file" << endl;
         delete out;
         out = 0;
         temp = false; 
         return;
     }
     buffer = new char[AF_FILEBUFFERSIZE];
     out->rdbuf()->pubsetbuf(buffer,AF_FILEBUFFERSIZE);

     out->write((char*) &algId, sizeof(uint32_t));
     out->write((char*) &typeId, sizeof(uint32_t));
     std::string tl = nl->ToString(typeList);
     uint32_t l = tl.length();
     out->write((char*) &l, sizeof(uint32_t));
     out->write(tl.c_str(), l);
     nType = SecondoSystem::GetInstance()->GetCatalog()->NumericType(typeList);
  }

  AttributeFile::~AttributeFile(){
     if(out){
        delete out;
     }
     if(temp){
        FileSystem::DeleteFileOrFolder(filename);
     }
     if(buffer){
        delete[] buffer;
     }
  }


  bool AttributeFile::append(Attribute* attr, bool withFlobs) {
      if(!out || !out->good()){
         return false;
      }
      // format is
      // char: 0 Default with flobs
      //       1 Default without flobs 
      //       2 Serialized
      // uint32_t size of root record (or serialized size)
      // root record (serial version)
      // possible flobs
      if(attr->GetStorageType() == Attribute::Default){
         if(withFlobs){
           uint8_t dv = 0;
           out->write((char*) &dv,1);
           uint32_t l = attr->Sizeof();
           out->write((char*)&l, 4); 
           out->write((char*)attr, l); 
           // process FLOBS
           for(int i=0;i<attr->NumOfFLOBs(); i++){
              Flob* flob = attr->GetFLOB(i);
              char* data = flob->getData();
              out->write(data, flob->getSize());
              delete[] data;
           }   
         } else {
           for(int i=0;i<attr->NumOfFLOBs(); i++){
              Flob* flob = attr->GetFLOB(i);
              flob->bringToDisk();
           }   
           attr->Pin();
           uint8_t dv = 1;
           out->write((char*) &dv,1);
           uint32_t l = attr->Sizeof();
           out->write((char*)&l, 4); 
           out->write((char*)attr, l); 
         }
      } else {
         uint8_t dv = 2;
         out->write((char*) &dv,1);
         uint32_t  sz = attr->SerializedSize();
         out->write((char*) &sz, 4); 
         char* vb = new char[sz];
         attr->Serialize(vb,sz,0);
         out->write(vb,sz);
         delete[] vb;
      }
      return true;
  }


