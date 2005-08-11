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

1 The Tree implementation

In this file are  the functions for creating and manipulation 
operator trees 

*/

#include "Tree.h"

enum OPERATORS { OPAND, OPXOR, OPOR, OPNOT, OPCONDITIONAL,
                 OPBICONDITIONAL};


struct tree* createConstant(int value){
   struct tree* t = (struct tree*) malloc(sizeof(struct tree));
   t->defined = 1;
   t->type = CONSTANT;
   if(!value)
      t->value=0;
   else
      t->value=1;
   t->son1=0;
   t->son2=0; 
   return t;
}

struct tree* createVariable(int value){
   struct tree* t = (struct tree*) malloc(sizeof(struct tree));
   t->defined = 1;
   t->type = VARIABLE;
   t->value=value;
   t->son1=0;
   t->son2=0; 
   return t;
}

struct tree* createNonLeaf(int type, int value,
                           struct  tree* son1,struct tree* son2){
   struct tree* t = (struct tree*) malloc(sizeof(struct tree));
   t->type = type;
   t->value=value;
   t->son1=son1;
   t->son2=son2; 
   t->defined = 1;
   if(son1 && !son1->defined)
      t->defined=0;
    if(son2 && !son2->defined)
      t->defined=0; 
   return t;

}

struct tree* createAnd(struct tree* s1, struct tree* s2){
 return createNonLeaf(OP2,OPAND,s1,s2); 
}

struct tree* createOr(struct tree* s1, struct tree* s2){
    return createNonLeaf(OP2,OPOR,s1,s2); 
}

struct tree* createXor(struct tree* s1, struct tree* s2){
    return createNonLeaf(OP2,OPXOR,s1,s2);
}
struct tree* createConditional(struct tree* s1, struct tree* s2){
    return createNonLeaf(OP2,OPCONDITIONAL,s1,s2);
}
struct tree* createBiconditional(struct tree* s1, struct tree* s2){
    return createNonLeaf(OP2,OPBICONDITIONAL,s1,s2);
}

struct tree* createNot(struct tree* s1){
    return createNonLeaf(OP1,OPNOT,s1,0);
}

void destroyTree(struct tree* victim){
    if(victim->son1)
       destroyTree(victim->son1);
    if(victim->son2)
       destroyTree(victim->son2);
    free(victim);
    victim = 0; 
}

int evalTree(struct tree* t, unsigned short number){

    if(!t){
        printf("try to evaluate an empty tree");
        return 0;
    }
    if(!t->defined){
       printf("error: undefined tree detected");
       return 0;
    }
    int s1,s2;

    if(t->type == CONSTANT)
      return t->value != 0;
    if(t->type == VARIABLE){
      return ((t->value) & number)!=0;
    }
    // operator
    switch (t->value){
         case OPNOT: s1 = evalTree(t->son1,number);
                     return !s1;
         case OPAND: s1 = evalTree(t->son1,number);  
                     s2 = evalTree(t->son2,number);
                     return s1 && s2;
         case OPOR: s1 = evalTree(t->son1,number);  
                    s2 = evalTree(t->son2,number);
                    return s1 || s2;
         case OPXOR: s1 = evalTree(t->son1,number);  
                     s2 = evalTree(t->son2,number);
                     return s1 ^ s2;
         case OPCONDITIONAL: s1 = evalTree(t->son1,number);  
                             s2 = evalTree(t->son2,number);
                             return !s1 || s2;
         case OPBICONDITIONAL: s1 = evalTree(t->son1,number);  
                               s2 = evalTree(t->son2,number);
                               return s1==s2;
    }
    printf("unknown operator"); 
    return -1;
}


void printTree(struct tree* t){
  if(t){
     if(t->type==CONSTANT){
        if(t->value)
            printf("true");
        else
            printf("false");
        return;
     }
     if(t->type==VARIABLE){
        switch(t->value){
           case ii : printf("ii");break;
           case ib : printf("ib");break;
           case ie : printf("ie");break;
           case bi : printf("bi");break;
           case bb : printf("bb");break;
           case be : printf("be");break;
           case ei : printf("ei");break;
           case eb : printf("eb");break;
           case ee : printf("ee");break;
           default : printf(" error-unknown variable %d",t->value);
        }
        return;
     }
     if(t->type==OP1){
        if(t->value==OPNOT){
           printf("not ");
           if(!t->son1){
              printf("error in argument of 'not'\n" );
           }else{
              printf("(");
              printTree(t->son1);
              printf(")");
           }
        }else{
           printf(" unknown unary operator "); 
        }
        return;
     }
     if(t->type==OP2){
        printf("("); printTree(t->son1); printf(")");
        switch(t->value){
           case OPAND : printf(" and ");break;
           case OPOR  : printf(" or ");break;
           case OPXOR : printf(" xor ");break;
           default: printf("unknown binary operator");
        }
        printf("(");
        printTree(t->son2);
        printf(")");
        return;
     }
     printf("unknown type for tree");
     
  } else{
     printf("empty tree");
  }

}
