/*

1.1.1 Class Implementation

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
#include "Algebras/DBService2/SecondoNodeAdapter.hpp"

using namespace std;

extern NestedList* nl;
extern boost::mutex nlparsemtx;

namespace DBService
{
  
  shared_ptr<Node> SecondoNodeAdapter::buildObjectFromNestedList(
    string database, ListExpr recordAsNestedList) {

    /*
      TODO This is similar to the type mapping problem in Algebras.
        Converting nested lists into data structures.

        In this example these pieces of information come together:

        - A record attributed such as Node host, port, ...
        - The C++ data type of the attribute e.g. string, int, ..
        - The Secondo data type, e.g. Text, String, Int, ...
        - The position in the nested list, e.g. Nth(1), Nth(2), ...

      Question:
        Is there a way to express this mapping generically based on a 
        mapping? 

        Here in pseudo JSON notation:

          { 
            member_class: Node
            member_name: "host", 
            member_type: "string", 
            secondo_type: "Text",
            position_in_nested_list: 1
          }
    */

    boost::lock_guard<boost::mutex> guard(nlparsemtx);

    // Example of a record as nested list: 
    //   ('localhost' 1245 '' '/home/doesnt_exist/secondo' 9941 9942 1) 
    string host       = nl->TextValue(nl->First(recordAsNestedList));
    int port          = nl->IntValue(nl->Second(recordAsNestedList));
    string config     = nl->TextValue(nl->Third(recordAsNestedList));
    string diskPath   = nl->TextValue(nl->Fourth(recordAsNestedList));
    int comPort       = nl->IntValue(nl->Fifth(recordAsNestedList));
    int transferPort  = nl->IntValue(nl->Sixth(recordAsNestedList));
    string type       = nl->StringValue(nl->Nth(7, recordAsNestedList));

    // Due to the appended "addid" operator, the id is always the 
    //  last item in the list
    int recordId = nl->IntValue(nl->Nth(8, recordAsNestedList));

    //TODO There is no way to set the id in the contructor. Fix it.
    shared_ptr<Node> node = make_shared<Node>(host, port, config, diskPath, 
      comPort, transferPort);
      
    node->setId(recordId);
    node->setType(type);
    node->setDatabase(database);
    node->setClean();
    node->setNotNew();
    
    return node;
  }
}