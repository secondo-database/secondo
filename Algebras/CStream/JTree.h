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

[1] Implementation of tree functions for JSON objects.

[toc]

1 JNode class definition

*/
#ifndef __JTREE_H__
#define __JTREE_H__

#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "RTuple.h"
#include "Symbols.h"
#include "ListUtils.h"
#include "Stream.h"

#include "TupleDescr.h"
#include "VTuple.h"
#include "VTHelpers.h"
#include "JTree.h"

#include <string>
#include <vector>

namespace cstream {

typedef enum {jROOT, jOBJECT, jARRAY, jSTRING,
              jNUMBER, jBOOL, jNULL, jERROR} JsonType;
// typedef enum {nlRECORD, nlVECTOR, nlTEXT, nlINT, nlREAL, nlBOOL} NLType;

/*
1. Node class definition.

*/
class JNode {
    public:
        JNode(const JsonType _type, const std::string _key,
              const std::string _value, JNode* _parent);

        void addChild(JNode* p);
        void print(int level=0);
        void buildTree();
        void deleteRec();
        
        std::string createTupleDescrString(bool forceReal=false);
        std::string getTupleType(bool forceReal=false);
        
        Tuple* buildTupleTree();
        Attribute* getAttribute(bool forceReal=false);

        JsonType getValueType(std::string jsonString, int* index, int* max);
        std::string getValue(JsonType valueType, std::string jsonString,
                             int* index, int* max);
        std::string getString(bool keySafe, std::string jsonString,
                              int* index, int* max);
        std::string getNumber(std::string jsonString, int* index, int* max);
        std::string getBool(std::string jsonString, int* index, int* max);
        std::string getNull(std::string jsonString, int* index, int* max);
        std::string getObject(std::string jsonString, int* index, int* max);
        std::string getArray(std::string jsonString, int* index, int* max);

        void toSeperator(std::string jsonString, int* index, int* max);
        
    private:
        std::string _key;
        std::string _value;
        JsonType _type;
        JNode* _parent;
        std::vector<JNode*> _children;
};

} /* namespace cstream */

#endif
