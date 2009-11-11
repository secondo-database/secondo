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

//[_][\_]
//[&][\&]
//characters [1] verbatim:   [\verb@]    [@]

""[1]


[1] Header File of the Histogram Algebra

December 2007, S. H[oe]cher, M. H[oe]ger, A. Belz, B. Poneleit


[TOC]

1 Overview

The file "Histogram.h" contains only defines and includes that are common
to all files of the HistogramAlgebra.

2 Defines and includes

*/
#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_


#include "Histogram1d.h"
#include "Histogram2d.h"

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager* am;

namespace hgr
{

  class HistogramAlgebra : public Algebra
  {
    public :
      HistogramAlgebra();
      ~HistogramAlgebra();
  };

} // namespace hgr

// Operators


#endif /*HISTOGRAM_H_*/
