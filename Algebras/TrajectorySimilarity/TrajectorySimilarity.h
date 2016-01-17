/*
----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, Department of Computer Science,
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
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]

[1] Interface of the Trajectory Similarity Algebra

\tableofcontents

\noindent

1 Introduction

This file declares the class ~TrajectorySimilarityAlgebra~. This file is needed
since some methods of the class are implemented in other {\tt .cpp} files than
{\tt TrajectorySimilarity.cpp} to keep the interdependencies of the files low.


2 Includes, Constants

*/

#ifndef __TRAJECTORY_SIMILARITY_H__
#define __TRAJECTORY_SIMILARITY_H__

#include "Algebra.h"


namespace tsa {

/*
3 Declaration of Class ~TrajectorySimilarityAlgebra~

*/

class TrajectorySimilarityAlgebra : public Algebra
{
public:

  TrajectorySimilarityAlgebra();
  ~TrajectorySimilarityAlgebra() { }

/*
The following methods perform the actual registration of type constructors and
operators. They are implemented in the {\tt .cpp} files of the corresponding
type constructors and operators and not in {\tt TrajectorySimilarity.cpp} to
keep the interdependencies of the files low.

*/

  void addPointSeqTC();
  void addTPointSeqTC();

  void addIsEmptyOp();
  void addNoComponentsOp();
  void addToDLineOp();
  void addToPointSeqOp();
  void addToTPointSeqOp();

  void addEndPointDistOp();
  void addEuclideanDistOp();
};

} //-- namespace tsa

#endif  //-- __TRAJECTORY_SIMILARITY_H__
