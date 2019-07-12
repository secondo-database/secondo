/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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

#include <string>
#include "Messages.h"

/*
1 Class ProgressListener extends ~MessageHandler~

Handles progress messages from workers

*/
class ProgressListener : public MessageHandler
{
public:
    /*
    1.1 Constructor

    */
    ProgressListener(std::shared_ptr<WorkerStatus> _ws);

    /*
    1.2 Destructor

    */
    ~ProgressListener();

    /*
    ~handleMsg~

    Callback method to handle a message from worker. 
    Handles only progress messages
     */
    bool handleMsg(NestedList *nl, ListExpr list, int source);

    /*
    ~calculateProgressInPercent~

    Returns the progress in percent
     */
    int calculateProgressInPercent(std::string progressMessage);

private:
    std::shared_ptr<WorkerStatus> ws;
};