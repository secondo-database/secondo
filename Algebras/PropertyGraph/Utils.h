/*
----
This file is part of SECONDO.

Copyright (C) 2012, University in Hagen
Faculty of Mathematic and Computer Science,
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

#ifndef PGRAPH_UTILS_H_
#define PGRAPH_UTILS_H_

#include "../MainMemory2/MPointer.h"
#include "../MainMemory2/Mem.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "AlgebraTypes.h"

#include <string>

//------------------------------------------------------------------
namespace pgraph
{


const bool LOG_COLORED=true;
const std:string LOGOP_COL = LOG_COLORED?"\033[32m":"";  
const std:string LOGOP_COL_ERR = LOG_COLORED?"\033[91m":"";  
const std:string LOG_COL_OFF = LOG_COLORED?"\033[0m":"";  

extern int debugLevel;

//------------------------------------------------------------------
inline void LOG_() {};
template<typename Head, typename... Args>
void LOG_(const Head& head, const Args&... args )
{ 
    std::cout << head << " ";
    LOG_(args...);
}

//------------------------------------------------------------------
template<typename... Args>
void LOGERR(const Args&... args )
{ 
    if (LOG_COLORED) std::cout <<LOGOP_COL_ERR;

    LOG_(args...);
    
    if (LOG_COLORED)
        std::cout << LOG_COL_OFF<<"\n";
    else        
        std::cout << "\n";
}
    

//------------------------------------------------------------------
template<typename... Args>
void LOG(int level, const Args&... args )
{ 
    if (level>debugLevel) return;
    //std::cout << "PGRAPH | ";
    LOG_(args...);
    
    if (LOG_COLORED)
        std::cout << LOG_COL_OFF<<"\n";
    else        
        std::cout << "\n";
    

}
//------------------------------------------------------------------
template<typename... Args>
void LOGOP(int level, const std::string source, const Args&... args )
{ 

    if (level>debugLevel) return;
    //std::cout << "PGRAPH | ";

    if (LOG_COLORED) std::cout <<LOGOP_COL;
    std::cout <<"["<<source<<"] ";
    if (LOG_COLORED) std::cout <<LOG_COL_OFF;


    LOG_(args...);
    
    if (LOG_COLORED)
        std::cout << LOG_COL_OFF<<"\n";
    else        
        std::cout << "\n";
    
}
//------------------------------------------------------------------

ListExpr GetArg_FTEXT_AS_LIST(Address a);
std::string GetArg_FTEXT_AS_STRING(Address a);

bool DefOpen(InObject infunc, SmiRecord& valueRecord,
                size_t& offset, const ListExpr typeInfo,
                Word& value );

bool DefSave(OutObject outfunc, SmiRecord& valueRecord,
                size_t& offset, const ListExpr typeInfo,
                Word& value );

std::string getDBname();

int GetNextListIndex(ListExpr list);
bool firstUpper(const string& word);

ListExpr GetTupleDefFromObject(std::string name);
void ReplaceStringInPlace(std::string& subject, const std::string& 
   search, const std::string& replace);

int QueryRelationCount(std::string relname);
Relation* QueryRelation(std::string relname); 
Relation* QueryRelation(std::string relname, ListExpr &relinfo); 
bool queryValueDouble(std::string cmd, double &val);

//------------------------------------------------------------------
class PGraphException : public std::exception {

  public:
  PGraphException() : msgStr("Unknown Error") {}
  PGraphException(const std::string& Msg) : exception(), msgStr(Msg) {}
  PGraphException(const PGraphException& rhs) : 
    std::exception(), msgStr(rhs.msgStr) {}
  virtual ~PGraphException() throw() {}

  virtual const char* what() const throw()
  {
    return ("PGRAPH-Exception: " + msgStr).c_str();
  }
  const std::string msg() { return msgStr; }
  
  protected:
    std::string msgStr;
};
} // namespace

#endif // PGRAPH_UTILS_H_