/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[$][\$]

*/

#include "QueryProcessor.h"
#include "NewTreeListener.h"
#include "AlgebraTypes.h"
#include <iostream>
#include "NestedList.h"

extern NestedList *nl;

class ProgressObserver;

/*
1 Class TreeListener extends ~NewTreeListener~

Listens for a new tree and calls the ~ProgressObserver~

*/
class TreeListener : public NewTreeListener
{

public:
    /*
    1.1 Constructor

    */
    TreeListener(QueryProcessor *_qp, ProgressObserver *_po);

    /*
    1.2 Destructor

    */
    ~TreeListener();

    /*
    ~handleNewTree~

    Callback is called after a new tree is created, 
    Coordinates further processing and other methods of this class
     */
    void handleNewTree(OpTree root, const ListExpr fromExpression);

    /*
    ~collectNodesToObserve~

    Collects observalbe nodes
    */
    void collectNodesToObserve(OpTree root);

    /*
    ~observeOrNot~

    If ~node~ should be oberved it will be added to the list of observable nodes
    */
    void observeOrNot(Operator *op, OpTree node);

    /*
    ~constructTreeString~
    
    Creates a JSON object from ~root~ and saves it as a string. 
    Calls ~constructTreeString~
     */
    void constructTreeString(OpTree root);

    /*
    ~constructTreeString~

    Creates a JSON object from ~root~ 
    */
    void parseToJSONString(OpTree root);

    /*
    ~collectArgumentsForNodes~

    Parses arguments for nodes using ~expr~
    */
    void collectArgumentsForNodes(ListExpr expr, int count);

    /*
    ~isObservable~
    
    Returns true if ~op~ should be observed, false else
     */
    bool isObservable(std::string op);

    /*
    ~pairOperatorWithArguments~

    Pairs ~_op~ with corresponding ~args~ 
    taking into account it's occurence in the nested list and tree
    */
    void pairOperatorWithArguments
    (std::string _op, int occurence, std::string args);

private:
    QueryProcessor *qp;
    ProgressObserver *po;
    std::unordered_map<int, Supplier> observableNodes;
    std::unordered_map<int, std::string> observableNodesWithNames;
    std::unordered_map<int, std::string> observableNodesWithArguments;
    std::string tree;
    std::vector<int> sortedObservableNodeIds;
    std::vector<std::string> argsCollectedFor;
};