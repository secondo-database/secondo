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

#include "Dist2Helper.h"
#include "DArray.h"


using namespace std;

namespace distributed2{


/*

1.1 Implementation of  Class ~DArrayElement~

This class represents information about a single worker of a DArray.

*/



DArrayElement::DArrayElement(const string& _server, const int _port,
               const int _num, const string& _config):
   server(_server), port(_port), num(_num),config(_config) {}

DArrayElement::DArrayElement(const DArrayElement& src):
 server(src.server), port(src.port), num(src.num),
 config(src.config) {}

DArrayElement& DArrayElement::operator=(const DArrayElement& src){
   this->server = src.server;
   this->port = src.port;
   this->num = src.num;
   this->config = src.config;
   return *this;
}     

DArrayElement::~DArrayElement() {}

void DArrayElement::set(const string& server, const int port, 
         const int num, const string& config){
   this->server = server;
   this->port = port;
   this->num = num;
   this->config = config;
}


bool DArrayElement::operator==(const DArrayElement& other) const{
  return    (this->port == other.port)
         && (this->num == other.num)
         && (this->server == other.server)
         && (this->config == other.config); 
}

bool DArrayElement::operator<(const DArrayElement& other) const {
   if(this->port < other.port) return true;
   if(this->port > other.port) return false;
   if(this->num < other.num) return true;
   if(this->num > other.num) return false;
   if(this->server < other.server) return true;
   if(this->server > other.server) return false;
   if(this->config < other.config) return true;
   // equal or greater
   return false;
}

bool DArrayElement::operator>(const DArrayElement& other) const {
   if(this->port > other.port) return true;
   if(this->port < other.port) return false;
   if(this->num > other.num) return true;
   if(this->num < other.num) return false;
   if(this->server > other.server) return true;
   if(this->server < other.server) return false;
   if(this->config > other.config) return true;
   // equal or greater
   return false;
}

ListExpr DArrayElement::toListExpr(){
   return nl->ThreeElemList(
              nl->TextAtom(server),
              nl->IntAtom(port),
              nl->TextAtom(config));   
}

bool DArrayElement::readFrom(SmiRecord& valueRecord, size_t& offset){
   string s;
   if(!readVar(s,valueRecord,offset)){
       return false;
   }
   uint32_t p;
   if(!readVar(p,valueRecord,offset)){
      return false;
   }
   string c;
   if(!readVar(c,valueRecord,offset)){
      return false;
   }

   server = s;
   port = p;
   num = -1;
   config = c;
   return true;
}

bool DArrayElement::saveTo(SmiRecord& valueRecord, size_t& offset){
   if(!writeVar(server,valueRecord,offset)){
      return false;
   }
   if(!writeVar(port,valueRecord,offset)){
       return false;
   }
   if(!writeVar(config,valueRecord,offset)){
      return false;
   }
   return true;
}

void DArrayElement::print(ostream& out)const{
    out << "( S: " << server << ", P : " << port 
        << "Num : " << num
        << ", C : " << config << ")";
}


bool InDArrayElement(ListExpr list, DArrayElement& result){
   if(!nl->HasLength(list,3)){
     return false;
   }
   ListExpr e1 = nl->First(list);
   ListExpr e2 = nl->Second(list);
   ListExpr e4 = nl->Third(list);
   string server;
   int port; 
   int num;
   string config;

   if(nl->AtomType(e1) == StringType){
      server = nl->StringValue(e1);     
   } else if(nl->AtomType(e1) == TextType){
      server = nl->Text2String(e1);
   } else {
      return false;
   }
   stringutils::trim(server);
   if(server.empty()){
     return false;
   }
   if(nl->AtomType(e2) != IntType){
     return false;
   }
   port = nl->IntValue(e2);
   if(port <=0){
     return false;
   }

   if(nl->AtomType(e4) == StringType){
      config = nl->StringValue(e4);
   } else if(nl->AtomType(e4) == TextType){
      config = nl->Text2String(e4);
   } else {
      return false;
   }
   stringutils::trim(config);
   if(config.empty()){
      return false;
   }
   num = -1;
   result.set(server,port,num,config);
   return true;
}




ostream& operator<<(ostream& out, const DArrayElement& elem){
  elem.print(out);
  return out;
}


} // end of namespace distributed2




