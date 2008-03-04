/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% This file belongs to the GeneralTreeAlgebra framework (GTAF)           %
% Class descriptions and usage details could be found in gtaf.pdf        %
%                                                                        %
% (if this file does not exist, use "make docu" in the parent directory) %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\newpage

----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute iter and/or modify
iter under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that iter will be useful,
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

1.1 Implementation file "GTAF[_]Errors.cpp"[4]

January-February 2008, Mirko Dibbert
\\[3ex]
This file implements the "Msg"[4] class.

*/
#include "GTAF_Errors.h"

using namespace gtaf;

const string seperator = "\n" + string(75, '-') + "\n";

void
Msg::Msg::fatalError(string error, string msg)
{
    cmsg.error() << seperator
    << "<GTAF> FATAL ERROR: " << error << endl
    << msg << seperator
    << endl;
    cmsg.send();
    assert(false);
}

void
Msg::showDbgMsg()
{
    cmsg.info() << seperator
    << "<GTAF> Debugging outputs enabled..." << seperator
    << endl;
    cmsg.send();
}

void
Msg::memoryLeak_Warning(unsigned count, string objectName)
{
    cmsg.warning() << seperator
    << "<GTAF> Memory leak warning: "
    << count << " " << objectName << " left open!" << seperator
    << endl;
    cmsg.send();
}

void
Msg::noNodeRecord_Error()
{
    string error = "NO_NODE_RECORD";
    ostringstream msg;
    msg <<
"Tried to put a node to file, but a record for that node has not yet"
<< endl <<
"been created! Call <getNodeId> before deleting the node or calling "
<< endl <<
"the <put> method...";
    fatalError(error, msg.str());
}

void
Msg::wrongEntryType_Error()
{
    string error = "WRONG_ENTRY_TYPE";
    ostringstream msg;
    msg <<
"Tryed to insert or replace an entry, but the entry type does not"
    << endl <<
"correspond to the nodes entry type!";
    fatalError(error, msg.str());
}

void
Msg::undefinedNodeType_Error(int type)
{
    string error = "UNDEFINED_NODE_TYPE";
    string msg = "Tryied to create a node of undefined type!";
    fatalError(error, msg);
}

void
Msg::invalidNodeCast_Error()
{
    string error = "INVALID_NODE_CAST";
    string msg =
        "The node could not be casted to the specified type!";
    fatalError(error, msg);
}

void
Msg::noParentNode_Error()
{
    string error = "NO_PARENT_NODE";
    ostringstream msg;
    msg << "TreeManager tried to read the parent of the root node!";
    fatalError(error, msg.str());
}
