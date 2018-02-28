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

[1] Implementation of Tree functions.

[toc]

1 Node class definition
2 Tree class definition

*/
#ifndef __VTREE_H__
#define __VTREE_H__

#include <string>
#include <vector>
#include <set>
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"

namespace cstream {

typedef enum {NAME, TYPE} NodeType;

/*
1. Node class definition.

*/
class Node {
public:    
    Node(const std::string& n, const std::string& v, Node* p);
    void AddChild(Node* p);
    
    Node* _parent;
    std::string _name;
    std::string _type;    
    std::vector<Node*> _children;
};

/*
2. Tree class definition.

*/
class VTree {
public:
    static VTree& Inst() {
        static VTree _svtree;
        return _svtree;
    }

    /*Node* Create(
        const std::string& s, 
        std::string& error,
        int& max_depth,
        int& num_nodes
    );*/

    Node* Create(
        ListExpr list,
        std::string& error,
        int& max_depth,
        int& num_nodes
    );

    void Delete(Node*& root);    
    static void Print(Node* n, const std::string& ident);    

    int _max_depth;
    int _num_nodes;
protected:    
    bool IsLetterDigit(char ch);
    bool EatSpace(const std::string& s, size_t& pos);
    size_t GetNextSpace(const std::string& s, size_t pos, bool& nospace);
    size_t NextCloseBracket(const std::string& s, size_t pos);
    size_t NextOpenBracket(const std::string& s, size_t pos);
    void RemoveSpace(std::string& s);    

    void CreateRec(
        Node* parent,
        const ListExpr& list,
        int depth,
        bool isvector,
        std::string& error
    );

    void DeleteRec(Node* n);
    //bool CheckAttrName(const std::string& name, std::string& serror);
    //bool CheckAttrType(const std::string& type);
};
    
}

#endif
