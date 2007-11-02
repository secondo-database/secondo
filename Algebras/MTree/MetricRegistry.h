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

1 Header file of Metric Registry

November 2007 Mirko Dibbert

[TOC]

1.1 Overview

Every type constructor, which should provide a metric (e.g. to be stored in
m-trees), has to implement ecactly one method of type ~Metric~ for the
respective type constructor in the class ~MetricRegistry~ and register it in
the ~initialize~ method.

Further it should inherit from a class wich inherits from ~Metrical~ e.g.
~MetricalAttribute~, ~StandardMetricalAttrbute~ or 
~IndexableStandardMetricalAttribute~ (defined in ~MetricAttribute.h~) and 
belong to the kind ~METRICAL~.

1.2 Includes and Defines

*/

#ifndef __METRIC_REGISTRY_H__
#define __METRIC_REGISTRY_H__

#include <string>
#include <map>

#include "SecondoInterface.h"

typedef double (*Metric)( const void* attr1, const void* attr2 );

/*
1.3 Class ~MetricRegistry~

This class manages the defined metrics. Bevore a metric may be used, the
~Initialize~ method has to be called to register the defined metrics. The
metrics are stored in the map ~metrics~ which is indexed by the ~algebraId~
and the ~typeId~ of the respective type constructor.

*/
class MetricRegistry
{
 public:
  static void Initialize( SecondoInterface* secondoInterface );
/*
This method stores all defined metrics in the metric map.

*/

  static Metric GetMetric( const int algebraId, const int typeId );
/*
For every type constructor, this method returns the associated metric. The 
respective type construktor is selected through ~algebraId~ and ~typeId~ 
(if no metric has been associatet, the method returns 0).

*/

  static double Distance( const int algebraId, const int typeId,
                          const void* attr1, const void* attr2 );
/*
For every type constructor, this method calls the associated metric with the
parameters ~attr1~ and ~attr2~. The respective type construktor is selected 
through ~algebraId~ and ~typeId~ (if no metric has been associatet, the method
returns 0).

*/

 private:

  static void RegisterMetric( const string& name, Metric mf );
/*
This method mapps the string ~name~ to the metric ~mf~

*/

  static double PictureMetric( const void* attr1, const void* attr2 );
/*
This method implements the metric for the picture type constructor

*/

  static double HistogramMetric( const void* attr1, const void* attr2 );
/*
This method implements the metric for the histogram type constructor

*/

  static bool initialized;
  static SecondoInterface* si;       // reference to secondo interface
  static map<string,Metric> metrics; // contains the registered metrics
};

#endif
