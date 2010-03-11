/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header of the keyrange and keyrange2 Operator 

[TOC]

0 Overview

*/

#ifndef _BTREE2_OP_KEYRANGE_H
#define _BTREE2_OP_KEYRANGE_H

#include "Operator.h"
#include "NestedList.h"
#include "AlgebraTypes.h"

#include "BTree2.h"
#include "BTree2Impl.h"

namespace BTree2Algebra {
namespace Operators {

class keyrange {
  public:
  static ListExpr TypeMapping(ListExpr args);
  
  static ListExpr TypeMapping2(ListExpr args);  

  template<typename valuetype, int argNo>
  static int 
  ValueMappingAttr(Word* args, Word& result, int message, 
                                  Word& local, Supplier s);

  template<typename valuetype, int argNo>
  static int 
  ValueMappingInt(Word* args, Word& result, int message, 
                                  Word& local, Supplier s);

  template<typename valuetype, int argNo>
  static int 
  ValueMappingReal(Word* args, Word& result, int message, 
                                  Word& local, Supplier s);

  template<typename valuetype, int argNo>
  static int 
  ValueMappingString(Word* args, Word& result, int message, 
                                  Word& local, Supplier s);

  template<typename keytype, typename valuetype>
  static void 
  getKeyrangeValues (keytype key, double& less, double& equal, 
         double& greater, BTree2Impl<keytype, valuetype>* btree);

  template<typename valuetype, int argNo>
  static int 
  ValueMappingBool(Word* args, Word& result, int message, 
                                  Word& local, Supplier s);


  static ValueMapping valueMappings[];
  static ValueMapping valueMappings2[];
  
  static int Select( ListExpr args );
  static int Select2( ListExpr args );
  
  static Operator keyrange1;
  static Operator keyrange2;
 

};

} // end namespace Operators
} // end namespace BTree2Algebra

#endif
