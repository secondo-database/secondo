/*
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//characters [1] Type: [] []
//characters [2] Type: [] []
//[ae] [\"{a}]
//[oe] [\"{o}]
//[ue] [\"{u}]
//[ss] [{\ss}]
//[Ae] [\"{A}]
//[Oe] [\"{O}]
//[Ue] [\"{U}]
//[x] [$\times $]
//[->] [$\rightarrow $]
//[toc] [\tableofcontents]

[1] Implementation of the Worker type: Loop

The Worker receives the queries from the Coordinator and tuples from the
stream supplier. After checking all queries with the tuple, the nomo handler
is informed.

[toc]

1 WorkerJoin class implementation
see WorkerJoin.cpp for details.

*/

#ifndef __WORKERJOIN_H__
#define __WORKERJOIN_H__

#include "../Generic/WorkerGen.h"
#include "Algebras/BTree/BTreeAlgebra.h"
#include "NList.h"
#include "ListUtils.h"
#include "SecParser.h"

namespace continuousqueries {

class WorkerJoin: public WorkerGen {

  public:
    // Create
    WorkerJoin(int id, std::string attrliststr, 
      TcpClient* coordinationClient, std::string joinCondition
    );

    ~WorkerJoin();

    struct structQuerysort {
        std::string tname;
        std::string qname;
        std::string comp;
        std::string type;
        bool unused;
        int group;
        int count;
    };

      // EXTENDED FUNCTIONS
      virtual void Initialize();

      // PURE VIRTUAL FUNCTIONS
      void TightLoop();
      void addQuery(int id, std::string function);
      void showStatus();

      // OWN FUNCTIONS
      Word executeQueryString(std::string query);

      void buildQueryString();
      std::string getRelationDescription(NList attrlist);
      std::string getUniqueId(int len=4);

      std::string getFilterStringPart(structQuerysort elem);
      std::string getJoinStringPart();

  private:
    std::map<std::string, int> _queries;
    std::vector<std::pair <std::string, std::string>> _queryparts;

    std::string _joinCondition;
    std::string _querystring;
    
    NList _queryattrlist;
    NList _tupleattrlist;

    TupleType* _tuplett;
    TupleType* _querytt;
    Relation* _tuplerel;
    Relation* _queryrel;
    BTree* _qbtree;

    SecondoCatalog* _sc;
};

}
#endif