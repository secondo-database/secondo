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

3.3 Class ~MetricalAttribute~

December 2007, Mirko Dibbert

3.3.1 Class description

This interface class provide a new method, which is needed to obtain a
"DistData"[4] object from an attribute object. These objects will be stored
within m-trees and are used as parameter objects of the respective metric.

3.3.2 Definition part (file: MetricalAttribute.h)

*/
#ifndef __METRICAL_ATTRIBUTE_H
#define __METRICAL_ATTRIBUTE_H

#include "StandardAttribute.h"
#include "DistData.h"

class MetricalAttribute
: public IndexableStandardAttribute
{

public:
  virtual DistData* getDistData( const string& metricName ) = 0;
/*
This method should return a new DistData object, which must correspond with
the DistData object that the respective metric(defined in the class
"MetricRegistry"[4]) excepts.

The "metricName"[4] parameter may be used, if the attribute should return
different strings for different metrics (e.g. one metric, which expects value
vectors, and another, which expects two filenames and restores the value vectors
from these files).

*/
}; // MetricalAtrubite

#endif
