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
#include "RelationAlgebra.h"
#include "TemporalAlgebra.h"


class MAttiaAlgebra:public Algebra {
public:
	MAttiaAlgebra();
	virtual ~MAttiaAlgebra();
};

#endif 
