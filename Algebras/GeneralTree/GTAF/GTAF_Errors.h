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

1.1 Headerfile "GTAF[_]Errors.h"[4]

January-February 2008, Mirko Dibbert

*/
#ifndef __GTAF_ERRORS_H
#define __GTAF_ERRORS_H

#include <assert.h>
#include <string>
#include <sstream>
#include "GTAF_Config.h"
#include "LogMsg.h"

namespace gtaf
{
/********************************************************************
1.1 Class "Msg"[4]

This struct is used to show some error messages and debugging outputs.

********************************************************************/
class Msg
{
public:
/*
Shows an error message and calls "assert(false)"[4]. "Error"[4] should be the name of the error, whereas "msg"[4] is the message to show.

*/
    static void fatalError(string error, string msg);

/*
Prints a 'Debug mode enabled' message.

*/
    static void showDbgMsg();

/*
Used to show a warning message, if not all nodes/entris had beed deleted.

*/
    static void memoryLeak_Warning(
            unsigned count, string objectName);

/*
The following methods call the "fatalError"[4] method to show the respective error messages.

*/
    static void noNodeRecord_Error();
    static void wrongEntryType_Error();
    static void undefinedNodeType_Error(int type);
    static void invalidNodeCast_Error();
    static void noParentNode_Error();
};

}; // namespace gtaf
#endif // #ifndef __GTAF_ERRORS_H
