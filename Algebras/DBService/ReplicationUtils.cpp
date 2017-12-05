/*

1.1.1 Class Implementation

----
This file is part of SECONDO.

Copyright (C) 2017,
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
#include <algorithm>
#include <cstring>
#include <sstream>

#include "Algebras/DBService/ReplicationUtils.hpp"

using namespace std;

namespace DBService {

const std::string ReplicationUtils::getFileName(
        const std::string& databaseName,
        const std::string& relationName)
{
    stringstream fileName;
    fileName << databaseName << separator << relationName << ".bin";
    return fileName.str();
}


const std::string ReplicationUtils::getRelId(
        const std::string& databaseName,
        const std::string& relationName)
{
    stringstream relId;
    relId << databaseName << separator << relationName;
    return relId.str();
}

const std::string ReplicationUtils::getFileNameOnDBServiceWorker(
        const std::string& databaseName,
        const std::string& relationName)
{
    stringstream fileName;
    fileName << databaseName
             << separator
             << relationName
             << "xRPLCTD" << ".bin";
    return fileName.str();
}

const std::string ReplicationUtils::getRelNameOnDBServiceWorker(
           const string& databaseName,
           const string& relationName){
   stringstream relname;
   relname << databaseName << separator << relationName << "xRPLCTD";
   return relname.str();
}

void ReplicationUtils::parseFileName(const std::string& fileName,
        std::string& databaseName,
        std::string& relationName)
{
    size_t dbNameEndPos = fileName.find(separator, 0);
    databaseName = fileName.substr(0, dbNameEndPos);

    size_t relNameStartPos = dbNameEndPos+separator.length();
    size_t relNameLength = fileName.length() - strlen(".bin") - relNameStartPos;

    relationName = fileName.substr(relNameStartPos, relNameLength);
}


std::string ReplicationUtils::getRelName(const std::string& filename){
    // currently just remove the ".bin"
    if(filename.size()>4){
      return filename.substr(0,filename.size()-4);
    }
    return filename;
}


std::string ReplicationUtils::getDerivedName(
      const std::string& databasename,
      const std::string& relationname,
      const std::string& derivedName)
{
    stringstream r;
    r << databasename << separator << relationname << separator << derivedName;
    return r.str();
}

 bool ReplicationUtils::extractDerivateInfo(
         const std::string& derivedId,
         std::string& database,
         std::string& relation,
         std::string& derivate)
{

      size_t pos = derivedId.find(separator);
      if(pos==string::npos){
          return false;
      }
      database = derivedId.substr(0, pos);
      size_t restPos = pos+separator.length();
      string rest = derivedId.substr(restPos);
      pos = rest.find(separator);
      if(pos==string::npos){
         return false;
      }
      relation = rest.substr(0,pos);
      pos += separator.length();
      derivate = rest.substr(pos);
      return true;
 }



string ReplicationUtils::separator("xDBSx");

} /* namespace DBService */
