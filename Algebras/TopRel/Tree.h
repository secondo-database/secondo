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

3 The Tree 

This pure c file provides a struct tree representing an operator tree 
for special boolean expressions related to the 9 intersection model.
The reason for not using c++ is that this code should be used in a
environment created by bison. Unfortunately, bison has no support 
for c++.

*/



#ifndef TREE_H
#define TREE_H

/*
1.1 The Data Structure

*/
struct tree{
          int type;
          int value;
          int defined;
          struct tree* son1;
          struct tree* son2;
       };

/*
1.2 Some definitions

Here some variables are defined to make this code
easier to handle.

*/

enum VARIABLES {ii=256,ib=128,ie=64,
                bi=32, bb=16, be=8,
                ei=4,  eb=2,  ee=1};

enum TYPES {CONSTANT, VARIABLE, OP1, OP2};


/*
1.3 Creation of Types

The following functions create a new tree from the arguments.
It should be clear from the names, what the functions do. 
Note that each of the functions allocates memory. You have to
ensure, that this memory is deallocates after use.

*/

struct tree* createConstant(int value);
struct tree* createVariable(int value);

struct tree* createAnd(struct tree* s1, struct tree* s2);
struct tree* createOr(struct tree* s1, struct tree* s2);
struct tree* createXor(struct tree* s1, struct tree* s2);
struct tree* createConditional(struct tree* s1, struct tree* s2);
struct tree* createBiconditional(struct tree* s1, struct tree* s2);
struct tree* createNot(struct tree* s1);


/*

1.4 Deallocating Memory

This function deallocates the memory occupied by the tree and all
subtrees of it.

*/

void destroyTree(struct tree* victim);

/*
1.5 Eval Functions

This functions checks wether the given 9 intersection matrix
described by its matrix number fulfils the conditions given in
the tree.

*/
int evalTree(struct tree* t, unsigned short number);

/*
1.6 Print Function

This function can be used for debugging purposes. It prints out the
tree.

*/
void printTree(struct tree* t);


/*
1.7 Parse function

This function parses the given string. If the parsing is succsessful, 
the tree is returned as the second argument. If not, the tree remains
unchanged and the result will be 0.

Note that this function is defined in the TreeParser file instead in
the Tree.c file.

*/
int parseString(const char* S,struct tree** t);


/*
1.8 GetLastMessage

This function returns the last occured error message if available.
If not, NULL is returned. Don't forget to deallocate the memory
used by the result.

*/

char* GetLastMessage();

#endif
