/*
---- 
This file is part of SECONDO.

Copyright (C) 2019, 
University in Hagen, 
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

#include <functional>
#include <string>
#include <iostream>


/*
~getCommand~

This function extracts the next command from some input stream.

The arguments of this function are:

in:   the stream containing the next command
isPD: the input stream is a file containing comments in PD style
cmd:  the return value
showPrompt: some function showing a prompt for interactive input
isInternalCommand: returns true if the command should handled 
                   by the userinterface directly, w.g., help or quit
isStdInput: input comes from the keyboard instead of a file
prompt: the prompt that is used for readline library

*/

bool getCommand(std::istream& in, 
                const bool isPD, 
                std::string& cmd,
                std::function<void(const bool)> showPrompt,
                std::function<bool(const std::string&)> isInternalCommand,
                const bool isStdInput,
                const std::string& prompt);

