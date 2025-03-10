/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header of the updatebtree and updatebtree2 Operator 

[TOC]

0 Overview

*/

#ifndef _BTREE2_OP_UPDATEBTREE_H
#define _BTREE2_OP_UPDATEBTREE_H

#include "Operator.h"
#include "NestedList.h"
#include "AlgebraTypes.h"

#include "BTree2.h"

namespace BTree2Algebra {
namespace Operators {

class updatebtree {
  public:
  static ListExpr TypeMappingAll(ListExpr args, bool wrapper = true); 
  
  static ListExpr TypeMapping1(ListExpr args);
  
  static ListExpr TypeMapping2(ListExpr args);  

  static int 
  ValueMapping1(Word* args, Word& result, int message, 
                                  Word& local, Supplier s);

  static int 
  ValueMapping2(Word* args, Word& result, int message, 
                                  Word& local, Supplier s);
  
  static Operator updatebtree1;
  static Operator updatebtree2;

};

} // end namespace Operators
} // end namespace BTree2Algebra

#endif
