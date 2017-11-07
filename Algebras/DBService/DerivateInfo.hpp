
/*

1 Information about a derived object. 

1.1 ~DerivateInfo~

This class stores information about a derived object.
We store only such information that cannot be derived 
from the relation from that this objects depends on. 
For this reason, we ommit database name and original 
location. 

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

#ifndef DERIVATEINFO_HPP
#define DERIVATEINFO_HPP

#include "Algebras/DBService/MetadataObject.hpp"
#include "Algebras/DBService/ReplicaLocations.hpp"
#include <string>

namespace DBService
{

class DerivateInfo: public MetadataObject
{

 public:

    DerivateInfo(const std::string& _objectName,
                 const std::string& _dependsOn,
                 const std::string& _fundef): 
            objectName(_objectName), dependsOn(_dependsOn),
            fundef(_fundef) {}

/*
1.1.1 ~getName~

Returns the name of the derivated object.

*/
    const std::string& getName() const;

/*
1.1.2 ~getSource~

Returns the id of the relation from that this objects
depends on.

*/
   const std::string& getSource() const;

/*
1.1.3 ~getFun~

Returns the function that is used to create the object from
the source relation.

*/
   const std::string& getFun() const;

/*
1.1.4 ~addNode~

*/
   void addNode(ConnectionID id);

   void addNode(ConnectionID id, bool replicated);

/*
1.1.5 ~addNodes~

Adds several nodes at once.

*/
   void addNodes(std::vector<ConnectionID>& nodesToAdd);

/*
1.1.6 ~nodesBegin~

Returns an iterator to the begin of the nodes.

*/
    const ReplicaLocations::const_iterator nodesBegin() const;

/*
1.1.7 ~nodesEnd~

This function returns an iterator to the end of the nodes.    

*/
   const ReplicaLocations::const_iterator nodesEnd() const;


/*
1.1.8 ~toString~

*/
   std::string toString() const;


/*
1.1.9 ~getNodeCount~

Returns the number of nodes.

*/
   size_t getNodeCount() const;

/*
1.1.10 ~updateReplicationStatus~

*/
  void updateReplicationStatus(ConnectionID id, bool replicated);

 private:
    std::string objectName;
    std::string dependsOn;
    std::string fundef;

    ReplicaLocations nodes;

};

} // end of namespace DBService

#endif
