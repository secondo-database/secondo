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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Header File of the class ~PregelAlgebra~

November 2018, J. Mende


[TOC]

1 Overview

This header file contains declarations of the Algebra constructor and some auxiliary functions.

2 Defines and includes

*/

#ifndef SECONDO_PREGELALGEBRA_H
#define SECONDO_PREGELALGEBRA_H

#include <Algebra.h>
#include "PregelContext.h"
#include "typedefs.h"
#include <boost/log/trivial.hpp>
#include "Helpers/WorkerConfig.h"
#include "Helpers/Commander.h"
#include "Helpers/PregelStatus2Helper.h"

#include <regex>

namespace pregel {
 /*
  * 3 Pregel Algebra
  *
  * */
 class PregelAlgebra : public Algebra {
 public:
  /*
  * 3.1 Constructors and Destructor
  *
  * */
  PregelAlgebra();

  ~PregelAlgebra();

  /*
  * 3.2 Singleton Accessor
  *
  * */
  static PregelAlgebra *getAlgebra();

  static void setAlgebra(PregelAlgebra *algebra);

  /*
  * 3.3 Auxiliary functions
  *
  * */
  bool amITheMaster();

  void reset();

  void healthReport(std::stringstream &sstream);
  void healthReport(PregelStatus2Helper& ps2h);

 private:
  /*
  * 3.4 Singleton instance
  *
  * */
  static PregelAlgebra *algebra;
 };
}

#endif //SECONDO_PREGELALGEBRA_H
