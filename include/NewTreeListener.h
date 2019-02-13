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

The atof functions does not work correcty within a swi prolog environment.
For this reason, here an implementation of this function is provides exploiting a 
stringstream.

1 class NewTreeListener

This class represents a listener that can be registered at a QueryProcessor.

*/

#pragma once

struct OpNode;
typedef OpNode* OpTree;

class NewTreeListener{
  public:
     virtual void handleNewTree(OpTree root, 
                                const ListExpr fromExpression) = 0;
};



