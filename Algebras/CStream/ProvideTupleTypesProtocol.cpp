/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of datatype ProvideTupleTypesProtocol and operators.

[toc]

1 ProvideTupleTypesProtocol class implementation

*/
#include "ProvideTupleTypesProtocol.h"
#include <boost/lexical_cast.hpp>

using namespace std;

namespace cstream {

string ProvideTupleTypesProtocol::requestTupleTypes() {
    return "RequestTupleTypes";
}

string ProvideTupleTypesProtocol::requestDone() {
    return "RequestDone()";
}

bool ProvideTupleTypesProtocol::requestTupleTypes
        (string request, int& numOfTuples) {

    size_t pos1;
    size_t pos2;

    if(request.length() == 0)
        return false;

    pos1 = request.find("(");
    if(pos1 == std::string::npos)
        return false;
    if(request.substr(0, pos1) != requestTupleTypes())
        return false;

    request.erase(0, pos1 + 1);

    pos2 = request.find(")");
    if(pos2 == std::string::npos)
        return false;
    
    request = request.substr(0, pos2);
    
    try {
        numOfTuples = boost::lexical_cast<int>(request);
    }
    catch( boost::bad_lexical_cast const& ) {
        return false;
    }

    return true;
}

} /* namespace cstream */