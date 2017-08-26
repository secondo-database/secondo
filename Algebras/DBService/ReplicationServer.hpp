/*

1.1 ~ReplicationServer~

The ~ReplicationServer~ sends a file containing the replica of a relation to a
~ReplicationClient~ on request.

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
#ifndef ALGEBRAS_DBSERVICE_REPLICATIONSERVER_HPP_
#define ALGEBRAS_DBSERVICE_REPLICATIONSERVER_HPP_

#include <iostream>

#include "QueryProcessor.h"

#include "Algebras/Relation-C++/RelationAlgebra.h"

#include "Algebras/Distributed2/FileTransferServer.h"

#include "Algebras/DBService/MultiClientServer.hpp"

namespace DBService {

/*

1.1.1 Class Implementation

*/

class ReplicationServer: public MultiClientServer,
                                distributed2::FileTransferServer {

/*

1.1.1.1 Constructor

*/
public:
    explicit ReplicationServer(int port);

/*

1.1.1.1 Destructor

*/
    ~ReplicationServer();

/*

1.1.1.1 ~start~

This function starts the ~ReplicationServer~ and leaves it waiting for incoming
client connections.

*/
int start();

/*

1.1.1.1 ~communicate~

This function is called as soon as a ~ReplicationClient~ establishes a
connection to the ~ReplicationServer~. It reacts on the incoming messages from
the client.

*/
protected:
    int communicate(std::iostream& io);

/*

1.1.1.1 ~sendFileToClient~

This function is called in order to send a file to a client.

*/
private:
    void sendFileToClient(
            std::iostream& io,
            bool fileCreated,
            const boost::thread::id tid);

/*

1.1.1.1 ~applyFunctionAndCreateNewFile~

This function applies a function to a tuple stream read from a file and stores
the resulting tuple stream in a new file.

*/
    void applyFunctionAndCreateNewFile(
            std::iostream& io,
            const std::string& function,
            const std::string& oldFileName,
            const std::string& newFileName,
            const boost::thread::id tid);

/*

1.1.1.1 ~applyFunctionAndCreateNewFile~

This function applies a function to a tuple stream.

*/
    void applyFunction(
            QueryProcessor* qp,
            Tuple* input,
            Word function,
            Word funResult,
            Tuple* output);
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_REPLICATIONSERVER_HPP_ */
