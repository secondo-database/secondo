/*
----
This file is part of SECONDO.

Copyright (C) 2018,
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



#ifndef DARRAYELEMENT_H
#define DARRAYELEMENT_H

#include <string>
#include <iostream>
#include "NestedList.h"
#include "SecondoSMI.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"

namespace distributed2{

/*

2 Class ~DArrayElement~

This class represents information about a single worker of a DArray.

*/



class DArrayElement{
  public:
     DArrayElement(const std::string& _server, const int _port,
                    const int _num, const std::string& _config);

     DArrayElement(const DArrayElement& src);

     DArrayElement& operator=(const DArrayElement& src);


     ~DArrayElement();

      inline void setNum(const int num){
         this->num = num;
      }

     void set(const std::string& server, const int port, 
              const int num, const std::string& config);


     bool operator==(const DArrayElement& other) const;
     
     inline bool operator!=(const DArrayElement& other) const{
       return   !((*this) == other);
     }

     bool operator<(const DArrayElement& other) const;
     
     bool operator>(const DArrayElement& other) const;
     
     ListExpr toListExpr() const;

     bool readFrom(SmiRecord& valueRecord, size_t& offset);

     bool saveTo(SmiRecord& valueRecord, size_t& offset);

     void print(std::ostream& out)const;

     inline std::string getHost()const{ return server; }
     inline int getPort() const {return port; }
     inline std::string getConfig() const{ return config; }
     inline int getNum() const{ return num; }


     template<class H, class C>
     static DArrayElement* createFromTuple(Tuple* tuple, int num, 
                                   int hostPos, int portPos, int configPos){

         if(!tuple || (num < 0) ) {
            return 0;
         }

         H* CcHost = (H*) tuple->GetAttribute(hostPos);
         CcInt* CcPort = (CcInt*) tuple->GetAttribute(portPos);
         C* CcConfig = (C*) tuple->GetAttribute(configPos);

         if(!CcHost->IsDefined() || !CcPort->IsDefined() || 
            !CcConfig->IsDefined()){
             return 0;
         }
         std::string host = CcHost->GetValue();
         int port = CcPort->GetValue();
         std::string config = CcConfig->GetValue();
         if(port<=0){
            return 0;
         }
         return new DArrayElement(host,port,num,config);
     }


  private:
     std::string server;
     uint32_t port;
     uint32_t num;
     std::string config;
};

std::ostream& operator<<(std::ostream& out, const DArrayElement& elem);


bool InDArrayElement(ListExpr list, DArrayElement& result);


}

#endif


