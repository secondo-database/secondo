/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

*/

#pragma once

#include "AlgebraTypes.h"
#include <cstddef>
#include "Algebras/CRel/Ints.h"
#include "NestedList.h"
#include "Operator.h"
#include "SecondoSMI.h"
#include "Stream.h"
#include "Algebras/CRel/TBlock.h"
#include "Algebras/CRel/TypeConstructors/TBlockTC.h"
#include <vector>

/*
1 ExtendProjectOperator.h

we are going to use the CRelAlgebra namespace here as we are mostly operating
with types of this algebra and this operator is also logically closer to
the CRel algebra than the ColumnMoving algebra

*/

namespace CRelAlgebra
{
  namespace Operators
  {
  
/*
1.1 Declaration of the cextendproject Operator for CRel
 
*/

    class ExtendProjectOperator : public Operator
    {
    public:
      ExtendProjectOperator();

    private:
    
/*
The value mapping will be done by the function ~StreamValueMapping~ of the
CRel algebra. This function expects a template parameter, which represents
the status of an open stream. 

*/
      class State
      {
      public:
        State(ArgVector args, const TBlockTI &blockType);
        ~State();

        TBlock *Request();

      private:     
/*
represents the open stream

*/
        Stream<TBlock> m_stream;

        const SharedArray<uint64_t> m_projectionIndices;
/*
the second entry of the parameter array of the value mapping call 
and contains the pointers to the mapping functions for the new columns

*/
        Supplier m_extendParameter;

/*
The types of the extension columns

*/
        ListExpr m_extensionTypes;
      };

      static const OperatorInfo info;
      static ListExpr TypeMapping(ListExpr args);
      static State *CreateState(ArgVector args, Supplier s);
    };
  }
}
