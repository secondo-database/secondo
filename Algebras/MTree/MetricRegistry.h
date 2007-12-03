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

3 Managing Metrics

December 2007, Mirko Dibbert

3.1 Overview

Every type constructor, which needs a metric (e.g. to be indexed by m-trees),
has to implement at least one method of the type "TMetric"[4] for the respective
type constructor in the class "MetricRegistry"[4] (see below). The metrics
should except DistData objects, which are created with the "getDistData"[4]
method of the respective attribute class, which must inherrit from
"MetricalAttribute"[4] (extends "IndexableStandardAttribute"[4] to provide this
method.

3.2 Class ~MetricRegistry~

3.2.1 Class description

TODO enter class description

3.2.2 Definition part (file: MetricRegistry.h)

*/
#ifndef __METRIC_REGISTRY_H
#define __METRIC_REGISTRY_H

#define DEBUG_METRIC_REGISTRY

#include <string>
#include <map>
#include "SecondoInterface.h"

const string MF_DEFAULT = "default";
/*
The name for default metrics. Each type constructor which need a metric,
should define one of the provided metrics as default metric, which is used
if no metric is specified.

*/

typedef void ( *TMetric )( const void* data1, const void* data2,
               double& result );
/*
Type definition for metrics.

*/

class MetricRegistry
{
  struct MetricData
  {
    string tcName;
    TMetric metric;
    string descr;

    MetricData()
    {}

    inline MetricData( const string& tcName_,
               const TMetric metric_,
               const string& descr_ )
    : tcName ( tcName_ ), metric ( metric_ ), descr ( descr_ )
    {}
  }; // MetricData

  static map< string, MetricData > metric_map;
  static bool initialized;

  static void registerMetric( const string& metricName,
                const MetricData& data );
/*
This method is used to register a new metric.

*/

  static void initialize();
/*
This method registeres all defined distance functions.

*/

public:
  static TMetric getMetric( const string& tcName,
                            const string& metricName );
/*
This method returns the associated distance function (0, if no distance function
was found).

*/

  static ListExpr listMetrics();
/*
This method returns all registered metrics in a list, wich has the following
fomrat:

\begin{center}
      ((tcName metricName metricType metricDescr)...(...))
\end{center}

This list is used in the "DisplayTTY"[4] class to print the registered metrics
in a formated manner, which is used by the "list metrics"[4] command.

*/

private:
/********************************************************************
Below, all avaliable metrics will be defined:

********************************************************************/
  static void EuclideanInt(
      const void* data1, const void* data2, double& result );
/*
Euclidean distance function for the "int"[4] type constructor.

*/
  static void EuclideanReal(
      const void* data1, const void* data2, double& result );
/*
Euclidean distance function for the "real"[4] type constructor.

*/

  static void EditDistance(
      const void* data1, const void* data2, double& result );
/*
Edit distance function for the "string"[4] type constructor.

*/

  static void HistogramMetric(
      const void* data1, const void* data2, double& result );
/*
Metric for the "histogram"[4] type constructor.

*/

  static void PictureMetric(
      const void* data1, const void* data2, double& result );
/*
Metric for the "picture"[4] type constructor.

*/
};

#endif
