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

[1] Implementation of VTree.

[toc]

1 Node class implementation
2 VTree class implementation

*/
#include "VTree.h"
#include "VTHelpers.h"
#include <iostream>

namespace cstream {

static std::string cerror =  "TupleDescr: wrong Format at pos: ";

/*
1.1 Constructor

*/
Node::Node( const std::string& n, 
            const std::string& t, 
            Node* p) : _parent(p) {
    _name = n;
    _type = t;
}

/*
1.2 AddChild

*/
void Node:: AddChild(Node* p) {
    _children.push_back(p);
}

/*
2.1 IsLetterDigit
Checks if a character is letter or digit

*/
bool VTree::IsLetterDigit(char ch) {
    if (ch >= 'a' && ch <= 'z')
        return true;
    if (ch >= 'A' && ch <= 'Z')
        return true;
    if (ch >= '0' && ch <= '9')
        return true;
    return false;
}

/*
2.2 EatSpace
Removes spaces from starting at position pos of a string.

*/
bool VTree::EatSpace(const std::string& s, size_t& pos) {
    while (pos < s.length() && s[pos] == ' ') {
        pos++;
        if (pos >= s.length())
            return false;
    }    
    return true;
}

/*
2.3 GetNextSpace
Finds the 1st position of space from the beginning in a string.

*/
size_t VTree::GetNextSpace(const std::string& s, size_t pos, 
    bool& nospace) {
    do {
        if (pos >= s.size())
            return -1;
        if (s[pos] == '(' || s[pos] == ')') {
            nospace = true;
            return pos;
        }            
    } while (s[pos++] != ' ');
    nospace = false;
    return pos-1;
}

/*
2.4 NextCloseBracket
Finds the next close bracket. Allowed chars before the ')'
are letters, digits or spaces.

*/
size_t VTree::NextCloseBracket(const std::string& s, size_t pos) {
    do {
        if (pos >= s.size())
            return -1;
        if (s[pos] == ')')
            return pos;
        if (!IsLetterDigit(s[pos]) && s[pos] != ' ')
            return -1;
        pos++;
    } while (true);
    return -1;
}

/*
2.5 NextOpenBracket
Finds the next open bracket. Allowed chars before the '('
are letters, digits or spaces.

*/
size_t VTree::NextOpenBracket(const std::string& s, size_t pos) {
    do {
        if (pos >= s.size())
            return -1;
        if (s[pos] == '(')
            return pos;
        if (!IsLetterDigit(s[pos]) && s[pos] != ' ')
            return -1;
        pos++;
    } while (true);
    return -1;
}

/*
2.6 RemoveSpace
Removes spaces at the beginning and end of a string.

*/
void VTree::RemoveSpace(std::string& s) {
    while (s.length() > 0 && s[0] == ' ') {
        s = s.substr(1, s.size() - 1);
    }

    while (s.length() > 0 && s[s.length()-1] == ' ') {
        s = s.substr(0, s.size() - 1);
    }
}

/*
2.7 CreateRec
Constructs the expression tree, recursive part.

*/
void VTree::CreateRec(
        Node* parent,
        const ListExpr& list,
        int depth,
        bool isvector,
        std::string& error
    ) {
    if (!error.empty())
        return;    
    std::set<std::string> attr_names;
    ListExpr rest = list;
    while (!nl->IsEmpty(rest)) {
        std::string sattrname;
        
        ListExpr attrtype;
        if (!isvector) {
            ListExpr attr = nl->First(rest);
            rest = nl->Rest(rest);

            if (nl->ListLength(attr) < 2) {
                error = "TupleDescr: invalid attr pair: " + 
                        nl->ToString(attr);
                return;
            }
            ListExpr attrname = nl->First(attr);
            attrtype = nl->Rest(attr);

            // check attr name here!
            std::string error_name;
            if (!listutils::isValidAttributeName(attrname, error_name)) {
                error = "TupleDescr: " + error_name;
                return;
            }

            // check for duplicate names
            sattrname = nl->SymbolValue(attrname);
            if(attr_names.find(sattrname) != attr_names.end()) {
                error = "TupleDescr: duplicate attr name: " + sattrname;
                return;
            }
            attr_names.insert(sattrname);
            attrtype = nl->First(attrtype);
        } else {
            attrtype = list;
            rest = nl->TheEmptyList();
        }

        int typelen = nl->ListLength(attrtype);
        if (typelen <= 0) {
            std::string sattrtype = nl->ToString(attrtype);
            ListExpr errorinfo = listutils::emptyErrorInfo();
            if (!am->CheckKind(Kind::DATA(), attrtype, errorinfo)) {
                error = "TupleDescr: attr type error: " + 
                        nl->ToString(errorinfo);
                return;
            }
            Node* node = new Node(sattrname, sattrtype, parent);
            _num_nodes++;
            parent->AddChild(node);
        } else { // complex type, record or vector
            ListExpr attrtype1 = nl->First(attrtype);
            std::string sattrtype = nl->ToString(attrtype1);
            ListExpr errorinfo = listutils::emptyErrorInfo();
            if (!am->CheckKind(Kind::DATA(), attrtype, errorinfo)) {
                error = "TupleDescr: attr type error: " + 
                        nl->ToString(errorinfo);
                return;
            }

            Node* node = new Node(sattrname, sattrtype, parent);
            _num_nodes++;
            parent->AddChild(node);
            ListExpr attrlist = nl->Rest(attrtype);
            if (sattrtype == "vector") 
            { 
                attrtype = nl->First(attrlist);
                if (!am->CheckKind(Kind::DATA(), attrtype, errorinfo)) {
                    error = "TupleDescr: attr type error: " + 
                            nl->ToString(errorinfo);
                    return;
                }
                // recursive descend here for vector
                CreateRec(node, attrtype, depth+1, true, error);
            } else {
                // recursive descend here
                CreateRec(node, attrlist, depth+1, false, error);
            }
        }

    }
}

/*
2.8 Create
Constructs the expression tree.

*/
Node* VTree::Create(
        ListExpr list,
        std::string& error,
        int& max_depth,
        int& num_nodes
    ) {
    _max_depth = 0;
    Node* root = new Node("ROOT", "ROOT", NULL);
    _num_nodes = 1;
    error = "";
    CreateRec(root, list, 0, false, error);
    max_depth = _max_depth;
    num_nodes = _num_nodes;
    return root;
}

/*
2.9 DeleteRec
Delete the tree, recursive part.

*/
void VTree::DeleteRec(Node* n) {
    for (std::vector<Node*>::iterator it = n->_children.begin(); 
        it != n->_children.end(); ++it) {
        DeleteRec(*it);
    }
    delete n;
    n = NULL;
}

/*
2.10 Delete
Deletes the tree.

*/
void VTree::Delete(Node*& root) {
    DeleteRec(root);
    root = NULL;
}

/*
2.11 Print
Prints the tree to cout.

*/
void VTree::Print(Node* n, const std::string& ident) {
    std::cout << ident << "(" << n->_name << 
    " " << n->_type << ")" << std::endl;
    for (std::vector<Node*>::iterator it = n->_children.begin(); 
        it != n->_children.end(); ++it) {
        Print(*it, ident + "____");
    }
}

} /* namespace cstream */
