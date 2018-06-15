/*
----
This file is part of SECONDO.

Copyright (C) 2018, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/


/*
Class representing variable length records.

*/

#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

class vlr{
 public:
   uint16_t reserved;
   char userId[16];
   uint16_t recordId;
   uint64_t recordLength;
   char description[32];

   vlr(){
     reserved = 0;
     std::memset(userId,0,16);
     recordId = 0;
     recordLength = 0;
     memset(description,0,32);
   }

   vlr(const vlr& src) {
      reserved = src.reserved;
      memcpy(userId,src.userId,16);
      recordId = src.recordId;
      recordLength = src.recordLength;
      memcpy(description, src.description,32);
   }

   vlr& operator=(const vlr& src){
      reserved = src.reserved;
      memcpy(userId,src.userId,16);
      recordId = src.recordId;
      recordLength = src.recordLength;
      memcpy(description, src.description,32);
      return *this;
   }

   bool check(const std::string& _userid, uint16_t _recordId) const{
      if(recordId!=_recordId) return false;
      return strcmp(userId, _userid.c_str()) == 0;
   }


   virtual ~vlr(){}

   static vlr* read(std::ifstream& in, bool extended);  

   virtual std::ostream& print(std::ostream& out) const{
       out << "userId: " << userId << std::endl;
       out << "recordID: " << recordId << std::endl;
       out << "recordLength: " << recordLength << std::endl; 
       out << "description: " << std::string(description,32) << std::endl;
       return out;
   }

};



/*
Implementation of known vlrs

*/


class LASF_Projection_2111: public vlr{
  // OGC MATH TRANSFORM WKT RECORD
  public:
     static LASF_Projection_2111* create(vlr& s, std::ifstream& in) {
        if(!s.check("LASF_Projection", 2111)) return 0;        
        LASF_Projection_2111* res = new LASF_Projection_2111(s);
        if(res->read(in)){
           return res;
        }
        delete res;
        return 0;
     }
     virtual std::ostream& print(std::ostream& out) const{
       vlr::print(out);
       out << "content " << content << std::endl;
       return out;
     }

     const char* getData() const{
       return content.c_str();
     }

  private:
     LASF_Projection_2111(const vlr& s): vlr(s), content(""){}

     bool read(std::ifstream& in) {
        char* c = new char[recordLength];
        in.read(c,recordLength);
        if(in.good()){
           content = c;
        }
        delete[] c; 
        // TODO extract projection values from WKT
        return in.good();
     }
    

     std::string content; 
};

class LASF_Projection_2112: public vlr{
  // OGC COORDINATE SYSTEM WKT
  public:
     static LASF_Projection_2112* create(vlr& s, std::ifstream& in) {
        if(!s.check("LASF_Projection", 2112)) return 0;        
        LASF_Projection_2112* res = new LASF_Projection_2112(s);
        if(res->read(in)){
           return res;
        }
        delete res;
        return 0;
     }
     virtual std::ostream& print(std::ostream& out) const{
       vlr::print(out);
       out << "content " << content << std::endl;
       return out;
     }

     const char* getData() const{
       return content.c_str();
     }

  private:
     LASF_Projection_2112(const vlr& s): vlr(s), content(""){}

     bool read(std::ifstream& in) {
        char* c = new char[recordLength];
        in.read(c,recordLength);
        if(in.good()){
           content = c;
        }
        delete[] c; 
        // TODO extract projection values from WKT
        return in.good();
     }
     std::string content; 
};



class LASF_Projection_34735: public vlr{

   public:

     static LASF_Projection_34735* create(const vlr& s, std::ifstream& in){
        if(!s.check("LASF_Projection",34735)){
          return 0;
        }
        LASF_Projection_34735* res = new LASF_Projection_34735(s);
        if(res->read(in)){
           return res;
        } 
        delete res;
        return 0;
     }

     virtual std::ostream& print(std::ostream& out) const{
        vlr::print(out);
        out << "wKeyDirectoryVersion = " << wKeyDirectoryVersion << std::endl;
        out << "wKeyRevision = " << wKeyRevision << std::endl;
        out << "wMinorRevision = " << wMinorRevision << std::endl;
        out << "wNumberOfKeys  = " << wNumberOfKeys << std::endl;
        for(size_t i=0;i<pKeys.size();i++){
           out << "pKey["<<i<<"] = ";
           pKeys[i].print(out) << std::endl;
        }
        return out;
     }

     int numOfKeys() const{
         return wNumberOfKeys;
     }
     
     struct sKeyEntry{
       uint16_t wKeyID;
       uint16_t wTIFFTagLocation;
       uint16_t wCount;
       uint16_t wValue_Offset;
       std::ostream& print(std::ostream& out) const{
        out << "keyID: " << wKeyID 
            << ", tiffLocation : " << wTIFFTagLocation
            << ", count        : " << wCount
            << ", valueOffset : " << wValue_Offset;
        return out;
       }
       void read(std::ifstream& in, size_t& offset){
          in.read(reinterpret_cast<char*>(&wKeyID),2);
          offset +=2;
          in.read(reinterpret_cast<char*>(&wTIFFTagLocation),2);
          offset +=2;
          in.read(reinterpret_cast<char*>(&wCount),2);
          offset +=2;
          in.read(reinterpret_cast<char*>(&wValue_Offset),2);
          offset +=2;
       }
     };
     sKeyEntry getEntry(int i){
        return pKeys[i];
     }
   private:
     LASF_Projection_34735(const vlr& super):vlr(super){}

     bool read(std::ifstream& in);

     uint16_t wKeyDirectoryVersion;
     uint16_t wKeyRevision;
     uint16_t wMinorRevision;
     uint16_t wNumberOfKeys;
     std::vector<sKeyEntry> pKeys;
};


class LASF_Projection_34736: public vlr{
  // GeoDoubleParamsTag Record
  public:
     static LASF_Projection_34736* create(vlr& s, std::ifstream& in) {
        if(!s.check("LASF_Projection", 34736)) return 0;        
        LASF_Projection_34736* res = new LASF_Projection_34736(s);
        if(res->read(in)){
           return res;
        }
        delete res;
        return 0;
     }
     virtual std::ostream& print(std::ostream& out) const{
       vlr::print(out);
       for(size_t i=0;i<content.size(); i++){
         out  << "content[" << i << "] = " << content[i] << std::endl;
       }
       return out;
     }

     double* getData(){
       char* res = new char[content.size()*8];
       size_t offset = 0;
       for(size_t i=0;i<content.size();i++){
          double v = content[i];
          memcpy(res+offset,reinterpret_cast<char*>(&v),8);
          offset += 8;
       }
       return reinterpret_cast<double*>(res);
     }


  private:
     LASF_Projection_34736(const vlr& s): vlr(s), content(){}

     bool read(std::ifstream& in) {
        content.clear();
        int numvalues = recordLength/8;
        size_t ignore = recordLength % 8;
        double value;
        for(int i=0;i<numvalues;i++){
          in.read(reinterpret_cast<char*>(&value),8);
          content.push_back(value); 
        }
        if(ignore>0){
           in.seekg(ignore,std::ios::cur);
        }
        return in.good();
     }
     std::vector<double> content;     
};



class LASF_Projection_34737: public vlr{
  // GeoAsciiParamsTag Record  
  public:
     static LASF_Projection_34737* create(vlr& s, std::ifstream& in) {
        if(!s.check("LASF_Projection", 34737)) return 0;        
        LASF_Projection_34737* res = new LASF_Projection_34737(s);
        if(res->read(in)){
           return res;
        }
        delete res;
        return 0;
     }

     ~LASF_Projection_34737(){
        if(content){
           delete[] content;
        }
     }

     virtual std::ostream& print(std::ostream& out) const{
       vlr::print(out);
       size_t i=0;
       while(i<recordLength){
          std::stringstream ss;
          if(content[i]==0){
              i++;
          } else {
             ss << content[i];
             i++;
             while(i<recordLength && content[i]!=0){
               ss << content[i];
               i++;
             }
             out << ss.str();
          }
       }
       return out;
     }

     char* getData(){
       char* res  = new char[recordLength];
       memcpy(res,content,recordLength); 
       return res;
     }


  private:
     LASF_Projection_34737(const vlr& s): vlr(s), content(0){}

     bool read(std::ifstream& in) {
        content = new char[recordLength];
        in.read(content,recordLength);
        return in.good();
     }
     char* content;     
};

// TODO: implement further known vlr 






