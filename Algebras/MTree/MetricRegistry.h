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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

3 Managing Metrics

November/December 2007, Mirko Dibbert

3.1 Overview

Every type constructor, which needs a metric (e.g. to be indexed by m-trees),
has to implement at least one method of the type "TMetric"[4] for the respective
type constructor in the class "MetricRegistry"[4] (see below). The metrics
should except DistData objects, which are created with the "getData"[4]
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
#include "DistData.h"

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

typedef DistData* ( *TGetDataFun )( void* attr );
/*
Type definition for getData methods.

*/

class MetricRegistry
{
  struct MetricData
  {
    string tcName;
    TMetric metric;
    TGetDataFun getDataFun;
    string descr;

    MetricData()
    {}

    inline MetricData( const string& tcName_,
               const TMetric metric_, const TGetDataFun getDataFun_,
               const string& descr_ )
    : tcName ( tcName_ ), metric ( metric_ ),
      getDataFun( getDataFun_ ), descr ( descr_ )
    {}
  }; // MetricData

  static map< string, MetricData > metric_map;
  static map< string, string > defaults;
  static bool initialized;

  static void registerMetric( const string& metricName,
                const MetricData& data, bool isDefault = false );
/*
This method is used to register a new metric.

*/

  static void initialize();
/*
This method registeres all defined distance functions.

*/

public:
  static TMetric getMetric( const string& tcName,
      const string& metricName = "default" );
/*
This method returns the associated distance function (0, if no distance function
was found).

*/

  static TGetDataFun getDataFun( const string& tcName,
      const string& metricName = "default" );
/*
TODO

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
struct Lab
{
  signed char L, a, b;

  Lab( signed char L_, signed char a_, signed char b_ )
  : L(L_), a(a_), b(b_)
  {}

  Lab( unsigned char r_, unsigned char g_, unsigned char b_ )
  {
    double R, G, B;
    double rd = (double)r_/255;
    double gd = (double)g_/255;
    double bd = (double)b_/255;

    // compute R
    if ( r_ <= 10 )
      R = rd / 12.92;
    else
      R = pow( (rd+0.055)/1.055, 2.2 );

    // compute G
    if ( g_ <= 10 )
      G = gd / 12.92;
    else
      G = pow( (gd+0.055)/1.055, 2.2 );

    // compute B
    if ( b_ <= 10 )
      B = bd / 12.92;
    else
      B = pow( (bd+0.055)/1.055, 2.2 );

    // compute X,Y,Z coordinates of r,g,b
    double X = 0.4124240 * R + 0.357579 * G + 0.1804640 * B;
    double Y = 0.2126560 * R + 0.715158 * G + 0.0721856 * B;
    double Z = 0.0193324 * R + 0.119193 * G + 0.9504440 * B;

    /* used chromacity coordinates of whitepoint D65:
      x = 0.312713, y = 0.329016

      the respective XYZ coordinates are
      Y = 1,
      X = Y * x / y       = 0.9504492183, and
      Z = Y * (1-x-y) / y = 1.0889166480
    */

    double eps = 0.008856; // = 216 / 24389

    double x = X / 0.95045;
    double y = Y;
    double z = Z / 1.08892;

    long double fx, fy, fz;

    if (x > eps)
      fx = pow( x, 0.333333 );
    else
      fx = 7.787 * x + 0.137931;

    if (y > eps)
      fy = pow( y, 0.333333 );
    else
      fy = 7.787 * y + 0.137931;

    if (z > eps)
      fz = pow( z, 0.333333 );
    else
      fz = 7.787 * z + 0.137931;

    // compute Lab coordinates
    double Lab_Ld = ((116  * fy) - 16);
    double Lab_ad = (500 * ( fx - fy ));
    double Lab_bd = (200 * ( fy - fz ));

    L = (signed char)Lab_Ld;
    a = (signed char)Lab_ad;
    b = (signed char)Lab_bd;
  }

  Lab()
  {}

};

/********************************************************************
Below, all avaliable getData methods will be defined:

********************************************************************/
static DistData* getDataInt( void* attr );
static DistData* getDataReal( void* attr );
static DistData* getDataString( void* attr );
static DistData* getDataHistogram( void* attr );
static DistData* getDataPicture( void* attr );
static DistData* getDataPicture2( void* attr );
static DistData* getDataPicture3( void* attr );

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
  static void PictureMetric2(
      const void* data1, const void* data2, double& result );
  static void PictureMetric3(
      const void* data1, const void* data2, double& result );
/*
Metric for the "picture"[4] type constructor.

*/

  static void InitPictureMetric();

  static double pictureSimMatrix[128][128];
  static double pictureSimMatrix2[256][256];
  static double pictureSimMatrix3[256][256];
};

#endif
