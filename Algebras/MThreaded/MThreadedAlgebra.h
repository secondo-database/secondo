/*

This file is part of SECONDO.

Copyright (C) 2004-2012, University in Hagen, Faculty of Mathematics and
        Computer Science, Database Systems for New Applications.

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


//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]
//[_] [\_]
//[x] [\ensuremath{\times}]
//[->] [\ensuremath{\rightarrow}]
//[>] [\ensuremath{>}]
//[<] [\ensuremath{<}]

\newpage

[1] Implementation of Multithreaded Algebra

        June 2020. Ingo Bader

[TOC]

\newpage


1 The MThreadedAlgebra class

        Nothing special to see here really.

*/
#pragma once
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include <Algebras/MThreaded/MThreadedAux.h>
#include <thread>

namespace mthreaded {


class MThreadedAlgebra;
}

extern "C"
Algebra* InitializeMThreadedAlgebra(NestedList* nlRef, QueryProcessor* qpRef);
