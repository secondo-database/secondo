
/*
----
This file is part of SECONDO.

Copyright (C) 2010, University in Hagen,
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]


[1]  A simple stack implementation

[TOC]

*/

#ifndef STACK_H
#define STACK_H

#include <assert.h>


template<class T>
class stackElem{
  public:
   stackElem(const T& t, stackElem* root): elem(t), next(root){}

   T elem;
   stackElem* next;
};

template<class T>
class Stack{
  public:
    Stack(): root(0), size(0) {}

    void push(const T& elem){
       root = new stackElem<T>(elem, root);
       size++;
    }
    T pop(){
      assert(root);
      stackElem<T>* s = root;
      root = root->next;
      T res(s->elem);
      delete s;
      size--;
      return res;
    }
    T top() const{
      assert(root);
      return root->elem;
    }
    bool isEmpty() const{
      return (root==0);
    }
    size_t getSize(){
      return size;
    }
    void makeEmpty(){
      while(root){
        stackElem<T>* victim = root;
        root = root->next;
        delete victim;
      }
      size = 0;
    }
  
  private:
    stackElem<T>* root;
    size_t size;
};


#endif
