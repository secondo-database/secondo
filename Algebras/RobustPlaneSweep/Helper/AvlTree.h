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

#pragma once

#include <stdexcept>

namespace RobustPlaneSweep
{
  template<class TKey,class TValue> 
  class AvlTreeNode
  {
  private:
    TKey _key;
    int _height;
    int _balance;

    static void SwapParentChildNode(
      AvlTreeNode<TKey, TValue>* parent, 
      AvlTreeNode<TKey, TValue>* child)
    {
      int heightTemp = parent->_height; 
      parent->_height = child->_height; 
      child->_height = heightTemp;

      int balanceTemp = parent->_balance; 
      parent->_balance = child->_balance; 
      child->_balance = balanceTemp;

      AvlTreeNode<TKey, TValue>* childLeft = child->Left;
      AvlTreeNode<TKey, TValue>* childRight = child->Right;
      AvlTreeNode<TKey, TValue>* parentLeft = parent->Left;
      AvlTreeNode<TKey, TValue>* parentRight = parent->Right;

      parent->Left = childLeft;
      parent->Right = childRight;
      child->Left = (parentLeft == child ? parent : parentLeft);
      child->Right = (parentRight == child ? parent : parentRight);

      AvlTreeNode<TKey, TValue>* parentTemp = parent->Parent; 
      parent->Parent = child->Parent; 
      child->Parent = parentTemp;

      if (parent->Left != NULL) { parent->Left->Parent = parent; }
      if (parent->Right != NULL) { parent->Right->Parent = parent; }
      if (child->Left != NULL) { child->Left->Parent = child; }
      if (child->Right != NULL) { child->Right->Parent = child; }

      if (child->Parent != NULL) {
        if (child->Parent->Left == parent) { 
          child->Parent->Left = child; 
        } else if (child->Parent->Right == parent) { 
          child->Parent->Right = child; 
        } else { 
          throw new std::invalid_argument("child"); 
        }
      }
    }

  public:
    TValue Value;

    AvlTreeNode<TKey, TValue>* Left;
    AvlTreeNode<TKey, TValue>* Right;
    AvlTreeNode<TKey, TValue>* Parent;

    TKey GetKey() const
    {
      return _key;
    }

    void SetKey(const TKey &key) 
    {
      _key=key;
    }

    int GetHeight() const
    {
      return _height;
    }

    int GetBalance() const
    {
      return _balance;
    }

    AvlTreeNode(const TKey &key) : 
      _key(key),
      _height(1),
      _balance(0),
      Value(NULL),
      Left(NULL),
      Right(NULL),
      Parent(NULL)
    {
    }

    static void SwapNodes(
      AvlTreeNode<TKey, TValue>* x, 
      AvlTreeNode<TKey, TValue>* y)
    {
      if (x->Parent == y) {
        SwapParentChildNode(y, x);
      } else if (y->Parent == x) {
        SwapParentChildNode(x, y);
      } else {
        int heightTemp = x->_height; 
        x->_height = y->_height; 
        y->_height = heightTemp;

        int balanceTemp = x->_balance;
        x->_balance = y->_balance;
        y->_balance = balanceTemp;

        AvlTreeNode<TKey, TValue>* leftTemp = x->Left;
        x->Left = y->Left;
        y->Left = leftTemp;

        AvlTreeNode<TKey, TValue>* rightTemp = x->Right;
        x->Right = y->Right;
        y->Right = rightTemp;

        AvlTreeNode<TKey, TValue>* parentTemp = x->Parent;
        x->Parent = y->Parent;
        y->Parent = parentTemp;

        if (x->Left != NULL) {
          x->Left->Parent = x; 
        }

        if (x->Right != NULL) { 
          x->Right->Parent = x; 
        }

        if (y->Left != NULL) { 
          y->Left->Parent = y; 
        }

        if (y->Right != NULL) { 
          y->Right->Parent = y; 
        }

        if (x->Parent != NULL) {
          if (x->Parent->Left == y) { 
            x->Parent->Left = x; 
          } else if (x->Parent->Right == y) { 
            x->Parent->Right = x; 
          } else { 
            throw new std::logic_error("tree inconsistent"); 
          }
        }
        if (y->Parent != NULL) {
          if (y->Parent->Left == x) { 
            y->Parent->Left = y; 
          } else if (y->Parent->Right == x) { 
            y->Parent->Right = y; 
          } else { 
            throw new std::logic_error("tree inconsistent"); 
          }
        }
      }
    }

    static void CalculateHeightAndBalance(AvlTreeNode<TKey, TValue> *node)
    {
      while (node != NULL) {
        int height, balance;

        if (node->Left != NULL) {
          height = node->Left->GetHeight();
          if (node->Right != NULL) {
            int temp = node->Right->GetHeight();
            balance = height - temp;
            if (temp > height) {
              height = temp;
            }
          } else {
            balance = height;
          }
        } else if (node->Right != NULL) {
          height = node->Right->GetHeight();
          balance = -height;
        } else {
          height = balance = 0;
        }

        ++height;

        if (node->GetHeight() != height ||
          node->GetBalance() != balance) {
            node->_height = height;
            node->_balance = balance;
            node = node->Parent;
        } else {
          node = NULL;
        }
      }
    }

    AvlTreeNode<TKey, TValue>* GetNext()
    {
      AvlTreeNode<TKey, TValue>* node;

      if (Right != NULL) {
        node = Right;
        while (node->Left != NULL) {
          node = node->Left;
        }
        return node;
      }

      AvlTreeNode<TKey, TValue>* lastNode = this;
      node = Parent;
      while (node != NULL && node->Right == lastNode) {
        lastNode = node;
        node = node->Parent;
      }

      return node;
    }

    AvlTreeNode<TKey, TValue>* GetPrevious()
    {
      AvlTreeNode<TKey, TValue>* node;

      if (Left != NULL) {
        node = Left;
        while (node->Right != NULL) {
          node = node->Right;
        }
        return node;
      }


      AvlTreeNode<TKey, TValue>* lastNode = this;
      node = Parent;
      while (node != NULL && node->Left == lastNode) {
        lastNode = node;
        node = node->Parent;
      }

      return node;
    }
  };

  template<class TKey,class TValue, typename Comparision> 
  class AvlTree
  {
  private:
    enum Direction
    {
      Undefined,
      Left,
      Right
    };

    AvlTreeNode<TKey, TValue>* _root;
    int _count;
    Comparision* _comparisonObject;

    bool FindNode(
      const TKey& key, 
      AvlTreeNode<TKey, TValue>*& node, 
      AvlTreeNode<TKey, TValue>*& parentNode,
      Direction& parentNodeDirection)
    {
      AvlTreeNode<TKey, TValue>* currentNode = _root;
      AvlTreeNode<TKey, TValue>* lastNode = NULL;
      Direction lastNodeDirection = Undefined;

      for(;;) {
        if (currentNode == NULL) {
          node = NULL;
          parentNode = lastNode;
          parentNodeDirection = lastNodeDirection;
          return false;
        }

        int compareResult = 
          _comparisonObject->Compare(key, currentNode->GetKey());
        if (compareResult < 0) {
          lastNode = currentNode;
          lastNodeDirection = Left;
          currentNode = currentNode->Left;
        } else if (compareResult > 0) {
          lastNode = currentNode;
          lastNodeDirection = Right;
          currentNode = currentNode->Right;
        } else {
          node = currentNode;
          parentNode = lastNode;
          parentNodeDirection = lastNodeDirection;
          return true;
        }
      }
    }

    void Rebalance(AvlTreeNode<TKey, TValue>* node)
    {
      AvlTreeNode<TKey, TValue>* currentNode = node;

      while (currentNode != NULL) {
        AvlTreeNode<TKey, TValue>* parentNode = currentNode->Parent;
        if (currentNode->GetBalance() < -1) {
          if (currentNode->Right->GetBalance() <= 0) {
            /* Einfache Linksrotation 
            *  A
            *   \           B
            *    B    ->   / \
            *     \       A   C
            *      C
            * 
            */
            Rotate(
              parentNode, 
              currentNode, 
              currentNode->Right, 
              Left);
          } else {
            /* Doppelte Rotation (erst rechts, dann links)
            *   A          A
            *    \          \           B
            *     C  ->      B    ->   / \
            *    /            \       A   C
            *   B              C
            *
            */
            Rotate(
              currentNode, 
              currentNode->Right, 
              currentNode->Right->Left, 
              Right);

            Rotate(
              parentNode, 
              currentNode, 
              currentNode->Right, 
              Left);
          }
        } else if (currentNode->GetBalance() > 1) {
          if (currentNode->Left->GetBalance() >= 0) {
            /* Einfache Rechtsrotation 
            *      C
            *     /         B
            *    B    ->   / \
            *   /         A   C
            *  A   
            *
            */
            Rotate(
              parentNode, 
              currentNode, 
              currentNode->Left, 
              Right);
          } else {
            /* Doppelte Rotation (erst links, dann rechts)
            *   C          C
            *  /          /         B
            * A    ->    B    ->   / \
            *  \        /         A   C
            *   B      A
            * 
            */
            Rotate(
              currentNode, 
              currentNode->Left, 
              currentNode->Left->Right, 
              Left);
            
            Rotate(
              parentNode, 
              currentNode, 
              currentNode->Left, 
              Right);
          }
        }

        currentNode = parentNode;
      }
    }

    void Rotate(
      AvlTreeNode<TKey, TValue>* parentNode, 
      AvlTreeNode<TKey, TValue>* rootNode, 
      AvlTreeNode<TKey, TValue>* pivotNode, 
      Direction direction)
    {
      AvlTreeNode<TKey, TValue>* temp;

      pivotNode->Parent = parentNode;
      if (direction == Right) {
        temp = pivotNode->Right;
        pivotNode->Right = rootNode;
        rootNode->Left = temp;
      } else {
        temp = pivotNode->Left;
        pivotNode->Left = rootNode;
        rootNode->Right = temp;
      }

      rootNode->Parent = pivotNode;
      if (temp != NULL)
        temp->Parent = rootNode;

      if (parentNode != NULL) {
        if (parentNode->Left == rootNode) {
          parentNode->Left = pivotNode;
        } else if (parentNode->Right == rootNode) {
          parentNode->Right = pivotNode;
        } else {
          throw new std::logic_error("rotation error");
        }
      } else {
        _root = pivotNode;
      }

      AvlTreeNode<TKey, TValue>::CalculateHeightAndBalance(rootNode);
    }

  public:
    AvlTree(Comparision* comparisonObject)
    {
      _root = NULL;
      _count=0;
      _comparisonObject=comparisonObject;
    }

    bool TryAddNode(TKey& key, AvlTreeNode<TKey, TValue>*& node)
    {
      AvlTreeNode<TKey, TValue>* parentNode;

      Direction parentNodeDirection;
      if (!FindNode(key, node, parentNode, parentNodeDirection)) {
        node = new AvlTreeNode<TKey, TValue>(key);
        ++_count;

        if (parentNode != NULL) {
          if (parentNodeDirection == Left) {
            parentNode->Left = node;
            node->Parent = parentNode;
          } else {
            parentNode->Right = node;
            node->Parent = parentNode;
          }

          AvlTreeNode<TKey, TValue>::CalculateHeightAndBalance(parentNode);

          if (parentNode->Parent != NULL) {
            Rebalance(parentNode->Parent);
          }

        } else {
          _root = node;
        }
        return true;
      } else {
        return false;
      }
    }

    void Remove(AvlTreeNode<TKey, TValue>* node)
    {
      AvlTreeNode<TKey, TValue>* parentNode = node->Parent;

      if (node->Left == NULL || node->Right == NULL) {
        AvlTreeNode<TKey, TValue>* newChild=
          (node->Left == NULL ? node->Right : node->Left);

        if (newChild != NULL)
          newChild->Parent = parentNode;

        if (node->Parent != NULL) {
          if (parentNode->Left == node) {
            parentNode->Left = newChild;
          } else if (parentNode->Right == node) {
            parentNode->Right = newChild;
          } else {
            throw new std::logic_error ("tree inconsistent");
          }

          AvlTreeNode<TKey, TValue>::CalculateHeightAndBalance(parentNode);
          Rebalance(parentNode);
        } else {
          _root = newChild;
        }
      } else {

        if (node->Right->Left != NULL) {
          AvlTreeNode<TKey, TValue>* nextGreaterNode = node->Right->Left;
          while (nextGreaterNode->Left != NULL) {
            nextGreaterNode = nextGreaterNode->Left;
          }
          AvlTreeNode<TKey, TValue>* tempParent = nextGreaterNode->Parent;
          tempParent->Left = nextGreaterNode->Right;
          if (tempParent->Left != NULL) {
            tempParent->Left->Parent = tempParent;
          }

          if (parentNode != NULL) {
            if (parentNode->Left == node) {
              parentNode->Left = nextGreaterNode;
            } else if (parentNode->Right == node) {
              parentNode->Right = nextGreaterNode;
            } else {
              throw new std::logic_error("tree inconsistent");
            }
          } else {
            _root = nextGreaterNode;
          }

          nextGreaterNode->Parent = node->Parent;
          nextGreaterNode->Left = node->Left;
          nextGreaterNode->Left->Parent = nextGreaterNode;
          nextGreaterNode->Right = node->Right;
          nextGreaterNode->Right->Parent = nextGreaterNode;

          AvlTreeNode<TKey, TValue>::CalculateHeightAndBalance(tempParent);
          Rebalance(tempParent);
          AvlTreeNode<TKey, TValue>::CalculateHeightAndBalance(nextGreaterNode);
          Rebalance(nextGreaterNode);


        } else {
          AvlTreeNode<TKey, TValue>* nextGreaterNode = node->Right;

          nextGreaterNode->Left = node->Left;
          nextGreaterNode->Left->Parent = nextGreaterNode;

          if (parentNode != NULL) {
            if (parentNode->Left == node) {
              parentNode->Left = nextGreaterNode;
            } else if (parentNode->Right == node) {
              parentNode->Right = nextGreaterNode;
            } else {
              throw new std::logic_error("tree inconsistent");
            }
          } else {
            _root = nextGreaterNode;
          }

          nextGreaterNode->Parent = node->Parent;

          AvlTreeNode<TKey, TValue>::CalculateHeightAndBalance(nextGreaterNode);
          Rebalance(nextGreaterNode);
        }
      }

      delete node;
      --_count;
    }

    AvlTreeNode<TKey, TValue>* GetRoot()
    {
      return _root;
    }

    void SwapNodes(AvlTreeNode<TKey, TValue>* x, AvlTreeNode<TKey, TValue>* y)
    {
      AvlTreeNode<TKey, TValue>::SwapNodes(x, y);
      if (x == _root) {
        _root = y;
      } else if (y == _root) {
        _root = x;
      }
    }
  };
}
