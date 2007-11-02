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

November 2007 Mirko Dibbert

[TOC]

1.1 Includes and Defines

*/

using namespace std;

#include <string>
#include <sstream>

#include "MetricRegistry.h"
#include "SecondoInterface.h"
#include "PictureAlgebra.h"

/*
1.2 Class ~MetricRegistry~

1.2.1 Initialize static members

*/
bool MetricRegistry::initialized = false;
SecondoInterface* MetricRegistry::si = 0;
map<string,Metric> MetricRegistry::metrics;

/*
1.2.2 Method ~GetMetric~

*/
Metric MetricRegistry::GetMetric( const int algebraId, const int typeId )
{
  ostringstream osId;
  osId << "[" << algebraId << "|" << typeId << "]";
  map<string,Metric>::iterator mfPos = metrics.find( osId.str() );

  if ( mfPos != metrics.end() )
  {
    return mfPos->second;
  }
  else
  { // no metric registered for this type
    return 0;
  }
}

/*
1.2.3 Method ~Distance~

*/
double MetricRegistry::Distance( const int algebraId, const int typeId,
                                 const void* attr1, const void* attr2)
{
  ostringstream osId;
  osId << "[" << algebraId << "|" << typeId << "]";
  map<string,Metric>::iterator mfPos = metrics.find( osId.str() );

  if ( mfPos != metrics.end() )
  {
    return (*mfPos->second)( attr1, attr2 );
  }
  else
  { // no metric registered for this type
    return 0;
  }
}

/*
1.2.4 Method ~RegisterMetric~

*/
void MetricRegistry::RegisterMetric( const string& name, Metric mf )
{
   int algebraId, typeId;
   si->GetTypeId( name, algebraId, typeId );
   ostringstream osId;
   osId << "[" << algebraId << "|" << typeId << "]";
   metrics[osId.str()] = mf;
}

/*
1.2.5 Method ~PictureMetric~

*/
double MetricRegistry::PictureMetric( const void* attr1, const void* attr2 )
{
  Picture* p1 = (Picture*)attr1;
  Picture* p2 = (Picture*)attr2;

  double result = 0;

  // TODO compute result value

  return result;
}

/*
1.2.6 Method ~HistogramMetric~

*/
double MetricRegistry::HistogramMetric( const void* attr1, const void* attr2 )
{
  Histogram* h1 = (Histogram*)attr1;
  Histogram* h2 = (Histogram*)attr2;

  double result = 0;

  // TODO compute result value

  return result;
}

/*
1.2.7 Method ~Initialize~

*/
void MetricRegistry::Initialize( SecondoInterface* secondoInterface )
{
  if (!initialized)
  {
    si = secondoInterface;

    // register defined metrics
    RegisterMetric( "picture", &PictureMetric );
    RegisterMetric( "histogram", &HistogramMetric );

    initialized = true;
  }
}
