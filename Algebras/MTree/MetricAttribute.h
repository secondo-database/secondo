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

//[TOC] [\tableofcontents]

1 Implementation of Metric Registry

November 2000 Mirko Dibbert

[TOC]

1.1 Overview


There must be exactly one metric of type ~Metric~ for every type constructor
provided by any of the algebra modules, for which a metric is needed, e.g.
metric trees like m-trees

which should be indexable with metric
trees, e.g. m-trees.

To add a new metric, the respective static funtion has to be registered in the
initialize function.

1.2 Includes and Defines

*/

#ifndef __METRIC_H__
#define __METRIC_H__

#include <string>
#include <map>

#include "StandardAttribute.h"

/*
1.3 Class ~MetricalAttribute~

This class provides some functions, which are required, when the type 
constructor should be indexed by metrical datastructures like m-trees.

These functions make it possible to store a data string representation of this
object in the datastucture instead of the whole object, but unlike the ~WriteTo~
method of ~IndexableStandardAttribute~ only the parts of the object have to be
stored, which are necessary for distance computations.

If a type constructor should provide such capabilities, it must be of the kind
~METRICAL~ and the respective class has to inherit from on of the defined
~MetricaAttribute~ classes below. Furthermore the metric for this type
constructor has to be added to the class ~MetricRegistry~ (~MetricRegistry.h~).

*/
class StandardMetricalAttribute : public IndexableStandardAttribute
{
public:
  virtual char *GetMData() const = 0;
/*
This method should return the data string for distance computations.

*/
  virtual size_t GetMDataSize() const = 0;
/*
This method should return the length of the data string. If 0 is returned,
the datatype object itself will be used for distance computations.

WARNING: currently only fixed data length supportet, thus ensure that the data
length will be independent from objects.

*/
};

#endif
