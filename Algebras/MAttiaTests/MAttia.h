/*
  MAttia.h

  Created on: Jan 13, 2009
      Author: m.attia
 
*/

#ifndef MATTIA_H_
#define MATTIA_H_

#include "Algebra.h"

#include "NestedList.h"

#include "QueryProcessor.h"

#include "StandardTypes.h"
#include "LogMsg.h"
#include "NList.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/Temporal/TemporalAlgebra.h"


class MAttiaAlgebra:public Algebra {
public:
	MAttiaAlgebra();
	virtual ~MAttiaAlgebra();
};

class Page
{
public:
 char ar[4000];
 Page()
 {
 }
 Page(char dummy)
 {
   for(int i=0; i<4000; ++i)
     ar[i]= 'M';
 }
};

#endif 
