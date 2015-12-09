/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header of the createbtree2 Operator 

[TOC]

0 Overview

*/

#ifndef _BTREE2_OP_CREATEBTREE_H
#define _BTREE2_OP_CREATEBTREE_H

#include "Operator.h"
#include "NestedList.h"
#include "AlgebraTypes.h"

#include "BTree2.h"

namespace BTree2Algebra {
namespace Operators {

class createbtree2 {
  public:
  static std::string Specification1();
  static std::string Specification2();
  static ListExpr TypeMapping(ListExpr args);
  static int Select( ListExpr args );

  static int ValueMapping_Rel(Word* args, Word& result, int message,
                        Word& local, Supplier s);

  static int ValueMapping_Stream_Tid(Word* args, Word& result, int message,
                           Word& local, Supplier s);

  static int ValueMapping_Stream_Attrib(Word* args, Word& result, int message,
                           Word& local, Supplier s);

  static Operator def1;
  static Operator def2;
  static ValueMapping valueMappings[];
  static int numberOfValueMappings;
  static void AppendString(BTree2* inBtree, int attrIndex, 
                   GenericRelationIterator* iter);
  static void AppendInt(BTree2* btree, int attrIndex, 
                   GenericRelationIterator* iter);
  static void AppendDouble(BTree2* btree, int attrIndex, 
                   GenericRelationIterator* iter);
  static void AppendBool(BTree2* btree, int attrIndex, 
                   GenericRelationIterator* iter);
  static void AppendIndexableAttribute(BTree2* inBtree, int attrIndex, 
                   GenericRelationIterator* iter);
};

} // end namespace Operators
} // end namespace BTree2Algebra

#endif
