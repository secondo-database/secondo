/*
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]

1 Declaration of a class managing networks

Mai-Oktober 2007 Martin Scheppokat


1.1 Overview


This file contains the implementation of ~gline~


2.1 Defines, includes, and constants

*/

#ifndef NETWORKMANAGER_H_
#define NETWORKMANAGER_H_

#ifndef NETWORK_H_
#error Network.h is needed by NetworkManager.h. \
Please include in *.cpp-File.
#endif

/*
"Static" class managing all networks in secondo.

*/
class NetworkManager
{
public:
  
/*
Loads a network with a given id.

*/
  static Network* GetNetwork(int in_iNetworkId);


/*
Closes a network retrived via getNetwork.

*/
  static void CloseNetwork(Network* in_pNetwork);
};

#endif /*NETWORKMANAGER_H_*/
