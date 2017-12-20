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

December 07, 2017

Author: Nicolas Napp

\tableofcontents

*/

#ifndef CCPARTHASHJOIN_H_
#define CCPARTHASHJOIN_H_

#include "Operator.h"

/*
1 Operator ~ccPartHashJoin~

The ~ccPartHashJoin~ operator is a cache-conscious equi-join operator, which performs a partitioned hash join on two streams of tuple blocks. As arguments, it expects two streams of tuple blocks and the name of the join attribute for each argument relation. The operator is part of SECONDO's CRel-Algebra.

*/

namespace CRel2Algebra {

class ccPartHashJoin: public Operator
{
public:
  // constructor
  ccPartHashJoin();

  // destructor
  virtual ~ccPartHashJoin();

private:
  class Info;

  static ListExpr TypeMapping(ListExpr args);

  static ValueMapping valueMappings[];

  static int SelectValueMapping(ListExpr args);
};

} /* namespace CRel2Algebra */

#endif /* CCPARTHASHJOIN_H_ */
