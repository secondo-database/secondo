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
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[->] [$\rightarrow$]
 //[TOC] [\tableofcontents]
 //[_] [\_]

*/

#ifndef STREAMSOURCE_H
#define STREAMSOURCE_H



#include "Attribute.h"          // implementation of attribute types
#include "Algebra.h"            // definition of the algebra
#include "NestedList.h"         // required at many places
#include "QueryProcessor.h"     // needed for implementing value mappings
#include "AlgebraManager.h"     // e.g., check for a certain kind
#include "Operator.h"           // for operator creation
#include "StandardTypes.h"      // priovides int, real, string, bool type
#include "Algebras/FText/FTextAlgebra.h"
#include "Symbols.h"            // predefined strings
#include "ListUtils.h"          // useful functions for nested lists
#include "Algebras/Stream/Stream.h"             // wrapper for secondo streams


#include "LogMsg.h"             // send error messages

#include "Algebras/Relation-C++/RelationAlgebra.h"    // use of tuples

#include "Algebras/Distributed2/ConnectionInfo.h"     //use of ConnectionInfo
#include "Algebras/Distributed2/ErrorWriter.h"
#include "SocketIO.h"
#include "FakedStream.h"


namespace sharedstream {

   /* This class will represent a source for an neverending stream.
    * Other instances of Secondo (StreamProcessors) can connect to the
    * StreamSource and receive a copy of each tuple arriving.*/
    class StreamSource {
    public:
        /*constructor*/
        StreamSource(int port);

        /*constructor with a faked Stream*/

        StreamSource(FakedStream* fakestr, int port);
        /*destructor*/
        ~StreamSource();

        void addProcessor(std::string ip, std::string port);

        void deleteProcessor(std::string ip, std::string port);

        void sendTuple(Tuple tuple);


        struct ProcessorEntry {
            std::string ipAdress;
            std::string port;

            ProcessorEntry(std::string ipAdress, std::string port);
        };

    private:
        //vector for the registered StreamProcessors
        std::vector <ProcessorEntry> processors;

        //Object for a faked stream
        FakedStream *fakedStream;

    };//end streamsource

}//end namespace


#endif // STREAMSOURCE_H

