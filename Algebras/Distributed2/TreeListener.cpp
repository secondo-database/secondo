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

#include "ProgressObserver.h"
#include "TreeListener.h"

TreeListener::TreeListener(QueryProcessor *_qp, ProgressObserver *_po)
{
    qp = _qp;
    po = _po;
}

TreeListener::~TreeListener() {}

void TreeListener::handleNewTree(OpTree root, const ListExpr fromExpression)
{

    if (observableNodes.size() > 0)
    {
        observableNodes.clear();
    }
    if (observableNodesWithNames.size() > 0)
    {
        observableNodesWithNames.clear();
    }

    if (observableNodesWithArguments.size() > 0)
    {
        observableNodesWithArguments.clear();
    }

    if (sortedObservableNodeIds.size() > 0)
    {
        sortedObservableNodeIds.clear();
    }

    if (argsCollectedFor.size() > 0)
    {
        argsCollectedFor.clear();
    }

    collectNodesToObserve(root);
    std::sort(sortedObservableNodeIds.begin(), sortedObservableNodeIds.end());
    tree = "";
    constructTreeString(root);
    collectArgumentsForNodes(fromExpression, 0);
    po->observableNodesReadyCallback(observableNodes,
                                     qp->getInterfaceCommandText(),
                                     tree,
                                     nl->ToString(fromExpression),
                                     observableNodesWithNames,
                                     observableNodesWithArguments);
}

bool TreeListener::isObservable(std::string _op)
{
    for (auto it = observableNodesWithNames.begin();
     it != observableNodesWithNames.end(); ++it)
    {
        auto &w = *it;
        std::string op = w.second;
        if (op == _op)
        {
            return true;
        }
    }
    return false;
}

void TreeListener::pairOperatorWithArguments
(std::string _op, int occurence, std::string args)
{
    int opCount = 0;
    for (auto &i : sortedObservableNodeIds)
    {
        std::string opForId = observableNodesWithNames.at(i);
        if (_op == opForId)
        {
            if (opCount == occurence)
            {
                observableNodesWithArguments.insert(std::make_pair(i, args));
                break;
            }
            else
            {
                opCount = opCount + 1;
            }
        }
    }
}

void TreeListener::collectArgumentsForNodes(ListExpr expr, int count)
{
    const int l = nl->ListLength(expr);
    if (l == 0)
    {
        return;
    }
    for (int l0 = 1; l0 <= l; l0++)
    {
        std::string listOp = nl->ToString(nl->Nth(l0, expr));
        if (isObservable(listOp))
        {
            std::string arguments = "";
            for (int l1 = 2; l1 <= l; l1++)
            {
                if (l1 == 2)
                {
                    arguments = nl->ToString(nl->Nth(l1, expr));
                }
                else
                {
                    arguments = 
                    arguments + " : " + nl->ToString(nl->Nth(l1, expr));
                }
            }
            pairOperatorWithArguments(listOp,
             std::count(argsCollectedFor.begin(),
              argsCollectedFor.end(), listOp), arguments);
            argsCollectedFor.push_back(listOp);
        }
        collectArgumentsForNodes(nl->Nth(l0, expr), count++);
    }
}

void TreeListener::collectNodesToObserve(OpTree root)
{
    std::vector<Supplier> sons;

    if (qp->IsOperatorNode(root))
    {
        Operator *op = qp->GetOperator(root);

        if (op != nullptr)
        {
            observeOrNot(op, root);
        }

        int noSons = qp->GetNoSons(root);

        for (int i = 0; i < noSons; i++)
        {
            Supplier s = qp->GetSon(root, i);
            sons.push_back(s);
        }
    }
    for (Supplier son : sons)
    {
        collectNodesToObserve((OpTree)son);
    }
}

void TreeListener::observeOrNot(Operator *op, OpTree node)
{
    std::string type = nl->ToString(qp->GetType(node));
    if (type == "drel") {
        return;
    }
    std::string operatorName = op->GetName();
    if (operatorName == "dmap" 
    || operatorName == "dmap2" 
    || operatorName == "dmap3" 
    || operatorName == "partitionF" 
    || operatorName == "collect2")
    {
        observableNodes.insert(std::make_pair(qp->GetId(node), node));
        sortedObservableNodeIds.push_back(qp->GetId(node));
        observableNodesWithNames
        .insert(std::make_pair(qp->GetId(node), operatorName));
    }
}

void TreeListener::constructTreeString(OpTree root)
{
    tree.append("{ ");
    parseToJSONString(root);
    tree.pop_back();
    tree.append(" }");
}

 /*
  Building following structure:
   "id": {
    "result_type": "int",
    "operator": "tie",
    "operators_num_of_functions": 1,
    "sons": [1, 11],
    "constant": false,
    "object": false,
    "function": false,
    "info": "",
    "no_of_sons": 2

  }

 */
void TreeListener::parseToJSONString(OpTree root){
    std::vector<Supplier> sons;

    // id
    tree.append("\"");
    tree.append(std::to_string(qp->GetId(root)));
    tree.append("\": {");

    // result_type
    tree.append("\"result_type\": \"");
    tree.append(nl->ToString(qp->GetType(root)));
    tree.append("\",");

    // constant
    if (IsConstantObject(root))
    {
        tree.append("\"constant\": true,");
    }
    else
    {
        tree.append("\"constant\": false,");
    }
    // object
    if (qp->IsObjectNode(root))
    {
        tree.append("\"object\": true,");
    }
    else
    {
        tree.append("\"object\": false,");
    }
    // function
    if (qp->IsFunctionNode(root))
    {
        tree.append("\"function\": true");
    }
    else
    {
        tree.append("\"function\": false");
    }

    if (qp->IsOperatorNode(root))
    {
        // for function
        tree.append(",");
        Operator *op = qp->GetOperator(root);
        if (op != nullptr)
        {
            // operator
            tree.append("\"operator\": \"");
            tree.append(op->GetName());
            tree.append("\",");

            // operators_num_of_functions
            tree.append("\"operators_num_of_functions\": \"");
            tree.append(std::to_string(op->GetNumOfFun()));
            tree.append("\",");
        }
        else
        {
            // info
tree
.append("\"info\": \"operator node without operator (named function)\",");
        }

        int noSons = qp->GetNoSons(root);
        // no_of_sons
        tree.append("\"no_of_sons\": \"");
        tree.append(std::to_string(noSons));
        tree.append("\",");

        // sons
        tree.append("\"sons\": [");
        for (int i = 0; i < noSons; i++)
        {
            Supplier s = qp->GetSon(root, i);
            tree.append(std::to_string(qp->GetId(s)));
            if (i != (noSons - 1))
            {
                tree.append(", ");
            }
            sons.push_back(s);
        }
        tree.append("]");
    }
    tree.append("},");

    for (Supplier son : sons)
    {
        parseToJSONString((OpTree)son);
    }
}