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
//[_][\_]

*/

#include "CommandLog.h"

#include <boost/thread.hpp>
#include <boost/date_time.hpp>

#include "Algebras/Standard-C++/LongInt.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "ConnectionInfo.h"


namespace distributed2
{


/*
  Class CommandLog Implementation

*/

CommandLog::CommandLog()
{
   fileout=0;
   memlog = true;
}

CommandLog::~CommandLog(){
   if(fileout){
      fileout->close();
      delete fileout;
   }
   clear();
}

bool CommandLog::logToFile(const std::string& filename){
    boost::lock_guard < boost::mutex > gurad(mtx);
    std::ofstream* s = new std::ofstream(filename.c_str(), std::ios::trunc);
    if(s->good()){
      if(fileout){
         fileout->close();
         delete fileout;
      }
      fileout=s;
      return true;      
    } else {
      delete s;
      return false;
    }
}

void CommandLog::stopFileLogging(){
   boost::lock_guard < boost::mutex > gurad(mtx);
   if(fileout){
      fileout->close();
      delete fileout;
      fileout=0;
   }
}

void CommandLog::setMemLog(const bool _mlog){
   boost::lock_guard < boost::mutex > gurad(mtx);
   memlog = _mlog;
}

void CommandLog::clear()
{
    entries.clear();
}

void CommandLog::insert(ConnectionInfo* ci,
        const std::string& server,
        const std::string& home,
        const std::string& query,
        const double& runtime,
        const int errorCode)
{
    boost::lock_guard < boost::mutex > gurad(mtx);
    if(memlog){
      entries.push_back(
            LogEntry((void*) ci, server, home, query, runtime, errorCode));
    }
    if(fileout){
       std::stringstream ss;
       ss <<"[" << endl;
       ss << ci->getHost() << "@" << ci->getPort() << ci->serverPid() << endl;
       ss << query;
       ss << endl;
       ss << "--------------------" << endl;
       ss << runtime << endl;
       ss << errorCode << endl;
       ss << "]" << endl;
       (*fileout) << ss.str();
    }
}

ListExpr CommandLog::getTupleDescription()
{
    ListExpr attrList = nl->SixElemList(
            nl->TwoElemList(nl->SymbolAtom("ConnectionId"),
                            listutils::basicSymbol<LongInt>()),
            nl->TwoElemList(nl->SymbolAtom("Server"),
                            listutils::basicSymbol<FText>()),
            nl->TwoElemList(nl->SymbolAtom("Home"),
                            listutils::basicSymbol<FText>()),
            nl->TwoElemList(nl->SymbolAtom("Command"),
                            listutils::basicSymbol<FText>()),
            nl->TwoElemList(nl->SymbolAtom("RunTime"),
                            listutils::basicSymbol<CcReal>()),
            nl->TwoElemList(nl->SymbolAtom("ErrorCode"),
                            listutils::basicSymbol<CcInt>()));
    return nl->TwoElemList(listutils::basicSymbol<Tuple>(), attrList);
}

} // namespace distributed2

