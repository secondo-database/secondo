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

[1] Implementation of datatype DistributeStreamProtocol and operators.

[toc]

1 DistributeStreamProtocol class implementation

*/
#include "DistributeStreamProtocol.h"
#include <iostream>

using namespace std;

namespace cstream {

string DistributeStreamProtocol::requestStream() {
    return "RequestStream";
}

string DistributeStreamProtocol::confirmStream() {
    return "ConfirmStream";
}

string DistributeStreamProtocol::tupleMessage() {
    return "TupleMessage";
}

string DistributeStreamProtocol::requestSupportedTypes() {
    return "RequestSupportedTypes()";
}

string DistributeStreamProtocol::sendSupportedTypes(bool binary) {
    if(binary)
        return "ConfirmType(1)";
    return "ConfirmType(0)";
}

bool DistributeStreamProtocol::requestStream(string request,
    string& tupledesc, ListExpr& funList, bool& format){
    unsigned short pos;
    bool descrOK = false;
    bool filterOK = false;
    bool listvariantOK = false;

    if( request.length() == 0){
        return false;
    }

    int brackets = 0;
    for( size_t i = 0; i < request.size(); i++ ) {
        if( request.at(i) == '(' )
            brackets++;
        else if( request.at(i) == ')' )
            brackets--;
    }    
    if( brackets != 0 ) 
        return false;

    pos = request.find("(");
    if( pos == std::string::npos ){
        return false;
    }

    if ( request.substr(0, pos) != requestStream() ){
        return false;
    }

    request.erase(0 ,pos + 1);

    /* Extract the tupledescription */
    unsigned short bracket = 0;
    for(unsigned short i = 0; i < request.length(); i++){
        if ( request.at(i) == '(' ){
            bracket++;
        } else if ( request.at(i) == ')' ){
            bracket--;
            if( bracket == 0) {
                pos = i;
                break;
            } else {
                continue;
            }
        }
    }

    tupledesc = request.substr(0, pos + 1);
    request.erase(0, pos + 1);

    pos = request.find(",");
    if( request.at(0) != ',' ){
        return false;
    }
    descrOK = true;

    if( pos == std::string::npos ){
        return false;
    }

    request.erase(0, pos + 1);

    /* Extract the filtercondition */
    pos = request.find(",");
    string funtxt = request.substr(0, pos);
    request.erase(0, pos);

    pos = request.find(",");
    if ( request.at(0) != ',' ){
        return false;
    }
    if( !nl->ReadFromString(funtxt, funList)) {
        filterOK = false;
    }
    filterOK = true;

    if( pos == std::string::npos ){
        return false;
    }

    request.erase(0, pos + 1);

    pos = request.find(")");
    if ( request.at(pos) != ')' ){
        return false;
    }
    if( pos == string::npos ){
        return false;
    }

    string formattxt = request.substr(0, pos);
    if ( (formattxt == "true" || formattxt == "false" ||
          formattxt == "TRUE" || formattxt == "FALSE") &&
        (request.at(pos) == ')' )){
        listvariantOK = true;
        if(formattxt == "true" || formattxt == "TRUE")
            format = true;
        else 
            format = false;
    }

    //nestedlist = request.substr(0, pos);

    if( descrOK && filterOK && listvariantOK ){
        return true;
    }
    return false;
}

string DistributeStreamProtocol::confirmStream(string confirmMsg){
    return confirmStream() + "(" +confirmMsg + ")";
}

string DistributeStreamProtocol::confirmStreamOK(){
    return confirmStream("OK");
}

string DistributeStreamProtocol::tupleMessage(string tupleMsg){
    return tupleMessage() + "(" +tupleMsg + ")";
}

string DistributeStreamProtocol::streamDone() {
    return "StreamDone()";
}

} /* namespace cstream */
