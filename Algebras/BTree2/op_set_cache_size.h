/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header of the set\_cache\_size Operator

[TOC]

0 Overview

*/

#ifndef _BTREE2_OP_SET_CACHE_SIZE_H
#define _BTREE2_OP_SET_CACHE_SIZE_H

#include "Operator.h"
#include "NestedList.h"
#include "AlgebraTypes.h"

#include "BTree2.h"

namespace BTree2Algebra {
namespace Operators {

class set_cache_size {
  public:
  
  static ListExpr TypeMapping(ListExpr args);
    
  static int ValueMapping(Word* args, Word& result, int message,
                        Word& local, Supplier s);

  static Operator def;
};

} // end namespace Operators
} // end namespace BTree2Algebra

#endif
