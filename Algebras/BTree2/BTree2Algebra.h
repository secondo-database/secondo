/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header of the BTree2 Algebra

[TOC]

0 Overview

*/

#ifndef _BTREE2_ALGEBRA_H
#define _BTREE2_ALGEBRA_H

#include "Algebra.h"

namespace BTree2Algebra {

class Algebra : public ::Algebra
{
 public:
  Algebra();
  ~Algebra() {};
};

extern "C"
Algebra*
InitializeAlgebra( NestedList* nlRef,
                        QueryProcessor* qpRef,
                        AlgebraManager* amRef );
}

#endif
