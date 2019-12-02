/*

1.1 ~DebugOutput~

This is a collection of functions that helps to simplify error analysis by
printing information to the command line.

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
#ifndef ALGEBRAS_DBSERVICE_DEBUGOUTPUT_HPP_
#define ALGEBRAS_DBSERVICE_DEBUGOUTPUT_HPP_

#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include <boost/thread.hpp>

#include "Algebra.h"

#include "Algebras/DBService/LocationInfo.hpp"
#include "Algebras/DBService/RelationInfo.hpp"
#include "Algebras/DBService/DerivateInfo.hpp"

namespace DBService
{

/*

1.1.1 Function Definitions

These functions allow writing information into the tracefile. They are available
for all necessary data types.

*/

void print(std::string& text, std::ostream& out );
void printFunction(const char* text,std::ostream& out);
void printFunction(const boost::thread::id tid, const char* text,
                   std::ostream& out);
void print(const std::string& text,std::ostream& out);
void print(const char* text,std::ostream& out);
void print(ListExpr nestedList,std::ostream& out);
void print(int number,std::ostream& out);
void print(const char* text, int number,std::ostream& out);
void print(const char* text, ListExpr nestedList,std::ostream& out);
void print(const char* text1, std::string& text2,std::ostream& out);
void print(const char* text1, const std::string& text2,std::ostream& out);
void print(const char* text, const std::vector<std::string>& values,
           std::ostream& out);
void print(boost::thread::id tid, const char* text1, const std::string& text2,
           std::ostream& out);
void print(const std::string& text1, const char* text2,std::ostream& out);
void print(const LocationInfo& locationInfo,std::ostream& out);
void print(const RelationInfo& relationInfo,std::ostream& out);
void print(const DerivateInfo& derivateInfo,std::ostream& out);
void printLocationInfo(const LocationInfo& locationInfo, std::ostream& out);
void printRelationInfo(const RelationInfo& relationInfo, std::ostream& out);
void printDerivateInfo(const DerivateInfo& derivateInfo, std::ostream& out);

}



#endif /* ALGEBRAS_DBSERVICE_DEBUGOUTPUT_HPP_ */
