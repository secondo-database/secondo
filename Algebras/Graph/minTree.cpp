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

*/
#include "GraphAlgebra.h"
#include "minNode.cpp"
#include "vector"

class MinTree {

    public:

        MinTree();

        void put(Edge elem);
        Edge pull();

    private:

        vector<MinNode> dynamic;

        void orderPull(int pos);
        void orderPut(int pos);
        void rochade(int pos);
        int compare(Edge a, Edge b);
};

MinTree::MinTree() { 
}

void MinTree::put(Edge elem) {
    dynamic.push_back(MinNode(elem));
    orderPut(dynamic.size() - 1);
}

Edge MinTree::pull() {
//    cout << "MinTree::pull Zeile 55" << endl;
//  assert (!(dynamic.size() == 0));
    if (dynamic.size() == 0) {
        Edge edge;
        edge.SetDefined(false);
        return edge;
    }
    Edge ret = dynamic.at(0).getElem();
//    cout << "MinTree::pull Zeile 58" << endl;
    orderPull(0);
//    cout << "MinTree::pull Zeile 60" << endl;
    return ret;
}

void MinTree::orderPull(int pos) {
    int left = 2 * pos + 1;
    if (left > (int)(dynamic.size() - 1)) {
        rochade(pos);
        return;
    }
    if (left == (int)(dynamic.size() - 1)) {
        dynamic[pos] = dynamic.at(left);
//        vector<MinNode>::iterator iter_mn; 
        dynamic.pop_back();
        return;
    }
    if (compare(dynamic.at(left).getElem(), 
        dynamic.at(left + 1).getElem()) <= 0) {
        dynamic[pos] = dynamic.at(left);
        orderPull(left);
    } else {
        dynamic[pos] = dynamic.at(left + 1);
        orderPull(left + 1);
    }
    return;
}

void MinTree::orderPut(int pos) {
    if (pos == 0)
        return;
    int father = (pos - 1) / 2;
    if (compare(dynamic.at(father).getElem(), dynamic.at(pos).getElem()) < 0)
        return;
    MinNode change = dynamic.at(father);
    dynamic[father] = dynamic.at(pos);
    dynamic[pos] = change;
    orderPut(father);
}

void MinTree::rochade(int pos) {
    if (pos == (int)dynamic.size() - 1) {
        dynamic.pop_back();
        return;
    }
    dynamic[pos] = dynamic.at(dynamic.size() - 1);
    dynamic.pop_back();
    orderPut(pos);
}

int MinTree::compare(Edge a, Edge b) {
    if (a.GetCost() == b.GetCost())
        return 0;
    if (a.GetCost() > b.GetCost())
        return 1;
    else
        return -1;
}
