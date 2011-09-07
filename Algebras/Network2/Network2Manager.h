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

1.1 Declaration of a Class Managing Networks

Mai-Oktober 2007 Martin Scheppokat


Defines, includes, and constants

*/

#ifndef NETWORK2MANAGER_H_
#define NETWORK2MANAGER_H_

#ifndef __NETWORK2_ALGEBRA_H__
#error Network2Algebra.h is needed by NetworkManager.h. \
Please include in *.cpp-File.
#endif


namespace network2 {

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
  static Network* GetNetworkNew(int in_iNetworkId, map<int,string> *netList);


/*
Closes a network retrived via getNetwork.

*/
  static void CloseNetwork(Network* in_pNetwork);
};

} // end of namespace network2

#endif /*NETWORKMANAGER_H_*/
