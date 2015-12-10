/*
\newpage

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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

1.1 Headerfile "GTree[_]Msg.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_MSG_H__
#define __GTREE_MSG_H__

#include <assert.h>
#include <string>
#include <sstream>
#include "LogMsg.h"


namespace gtree
{

/********************************************************************
Class ~Msg~

This class provide some static methods to show error- or warning messages for the framework.

********************************************************************/
class Msg
{

public:
/*
Shows an error message and calls "assert(false)"[4]. "Error"[4] should be the name of the error, whereas "msg"[4] is the message to show.

*/
    static void fatalError(std::string error, std::string msg)
    {
        const std::string seperator = "\n" + std::string(75, '-') + "\n";
        cmsg.error() << seperator
        << "<GTree> FATAL ERROR: " << error << endl
        << msg << seperator
        << endl;
        cmsg.send();
        assert(false);
    }


/*
Prints a 'Debug mode enabled' message.

*/
    static void showDbgMsg()
    {
        const std::string seperator = "\n" + std::string(75, '-') + "\n";
        cmsg.info() << seperator
        << "<GTree> debug mode enabled" << seperator
        << endl;
        cmsg.send();
    }

/*
Used to show a warning message, if not all nodes/entries had beed deleted.

*/
    static void memoryLeak_Warning(
            unsigned count, std::string objectName)
    {
        const std::string seperator = "\n" + std::string(75, '-') + "\n";
        cmsg.warning() << seperator
        << "<GTree> Memory leak warning: "
        << count << " " << objectName << " left open!" << seperator
        << endl;
        cmsg.send();
    }

/*
The following methods call the "fatalError"[4] method to show the respective error messages.

*/
    static void noNodeRecord_Error()
    {
        std::string error = "NO_NODE_RECORD";
        std::ostringstream msg;
        msg << "Tried to put a node to file, "
            << "but a record for that node has not yet"
            << endl
            << "been created! Call <getNodeId> "
            << "before deleting the node or calling "
            << endl << "the <put> method...";
        fatalError(error, msg.str());
    }

    static void wrongEntryType_Error()
    {
        std::string error = "WRONG_ENTRY_TYPE";
        std::ostringstream msg;
        msg << "Tryed to insert or replace an entry, "
            << "but the entry type does not"
            << endl << "correspond to the nodes entry type!";
        fatalError(error, msg.str());
    }
    static void undefinedNodeType_Error(int type)
    {
        std::string error = "UNDEFINED_NODE_TYPE";
        std::string msg = "Tryied to create a node of undefined type!";
        fatalError(error, msg);
    }

    static void invalidNodeCast_Error()
    {
        std::string error = "INVALID_NODE_CAST";
        std::string msg =
            "The node could not be casted to the specified type!";
        fatalError(error, msg);
    }

    static void noParentNode_Error()
    {
        std::string error = "NO_PARENT_NODE";
        std::ostringstream msg;
        msg <<
            "TreeManager tried to read the parent of the root node!";
        fatalError(error, msg.str());
    }
}; // class Msg

} // namespace gtree
#endif // #define __GTREE_FILE_NODE_H__
