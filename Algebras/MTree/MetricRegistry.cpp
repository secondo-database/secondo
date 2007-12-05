/*
//[_] [\_]

3.2.3 Implementation Part (file: MetricRegistry.cpp)

*/

using namespace std;

#include <math.h>
#include <sstream>
#include "NList.h"
#include "StandardTypes.h"
#include "MetricRegistry.h"
#include "StandardTypes.h"
#include "PictureAlgebra.h"
#include "JPEGPicture.h"

extern SecondoInterface* si;

/*
Initialize static members :

*/
bool MetricRegistry::initialized = false;
map< string, MetricRegistry::MetricData > MetricRegistry::metric_map;

/*
Method ~registerMetric~ :

The default metric will be stored without a name to ensure that this metric is
allways the first one for each type constructor, which "listMetrics"[4] put into
the result list. The algebra- and type-id are only used to order the output of
"listMetrics"[4] by these id's.

*/
void
MetricRegistry::registerMetric( const string& metricName,
                const MetricData& data )
{
  int algebraId, typeId;
  si->GetTypeId( data.tcName, algebraId, typeId );

  ostringstream osId;
  osId << algebraId << "#" << typeId << ".";
  if ( metricName != MF_DEFAULT )
    osId << metricName;

  metric_map[osId.str()] = data;
}

/*
Method ~getMetric~ :

*/
TMetric
MetricRegistry::getMetric( const string& tcName,
                           const string& metricName )
{
  if (!initialized)
    initialize();

  int algebraId, typeId;
  si->GetTypeId( tcName, algebraId, typeId );

  ostringstream osId;
  osId << algebraId << "#" << typeId << ".";
  if ( metricName != MF_DEFAULT )
    osId << metricName;

  map< string, MetricData >::iterator pos =
      metric_map.find( osId.str() );

  if ( pos != metric_map.end() )
  {
    return pos->second.metric;
  }
  else return 0;
}

/*
Method ~getData~ :

*/
TGetDataFun MetricRegistry::getDataFun( const string& tcName,
                                        const string& metricName )
{
  if (!initialized)
    initialize();

  int algebraId, typeId;
  si->GetTypeId( tcName, algebraId, typeId );

  ostringstream osId;
  osId << algebraId << "#" << typeId << ".";
  if ( metricName != MF_DEFAULT )
    osId << metricName;

  map< string, MetricData >::iterator pos =
      metric_map.find( osId.str() );

  if ( pos != metric_map.end() )
  {
    return pos->second.getDataFun;
  }
  else return 0;
}

/*
Method ~listMetrics~ :

*/
ListExpr
MetricRegistry::listMetrics()
{
  if (!initialized)
    initialize();

  NList list;
  NList elem;
  ostringstream os;

  map< string, MetricData >::iterator pos = metric_map.begin();
  while ( pos != metric_map.end() )
  {
    string key = pos->first;

    // get metricName
    string metricName = key.substr( key.find( '.' ) + 1 );
    if ( metricName == "" )
      metricName = MF_DEFAULT;

    // get tcName
    string tcName = pos->second.tcName;

    // append item list to the output list
    NList e1( tcName );
    NList e2( metricName );
    NList e3 = e3.textAtom(pos->second.descr);
    list.append( NList( e1, e2, e3 ) );
    pos++;
  };

  return list.listExpr();
}

/*******************************************************************************
Below, the avaliable getData methods will be implemented:

*******************************************************************************/
DistData* MetricRegistry::GetDataInt( void* attr )
{
  int value = static_cast<CcInt*>(attr)->GetValue();
  char buffer[sizeof(int)];
  memcpy( buffer, &value, sizeof(int) );
  return new DistData( sizeof(int), buffer );
}

DistData* MetricRegistry::GetDataReal( void* attr )
{
  SEC_STD_REAL value =
      static_cast<CcReal*>(attr)-> GetValue();
  char buffer[sizeof(SEC_STD_REAL)];
  memcpy( buffer, &value, sizeof(SEC_STD_REAL) );
  return new DistData( sizeof(SEC_STD_REAL), buffer );
}

DistData* MetricRegistry::GetDataString( void* attr )
{
  string value = static_cast<CcString*>( attr )-> GetValue();
  return new DistData( value );
}

DistData* MetricRegistry::GetDataHistogram( void* attr )
{
  return new DistData("");
}

/*
Method GetDataPicture64Cols

*/
DistData* MetricRegistry::GetDataPicture( void* attr )
{
  unsigned long size;
  const char* imgdata = static_cast<Picture*>( attr )->
      GetJPEGData(size);

  JPEGPicture rgb( (unsigned char *) imgdata, size );

  unsigned long int rgbSize;
  unsigned char* rgbData = rgb.GetImageData( rgbSize );
  const unsigned int numOfPixels = rgbSize/3;

  unsigned long colorhist_abs[64];
  double colorhist[64];
  for (int i=0; i<64; i++)
  {
    colorhist_abs[i] = 0;
    colorhist[i] = 0;
  }

  for (unsigned long pos=0; pos<(numOfPixels); pos++)
  {
    char r = rgbData[(3*pos)] / 64;
    char g = rgbData[(3*pos)+1] / 64;
    char b = rgbData[(3*pos)+2] / 64;
    colorhist_abs[(16*r)+(4*g)+b]++;
  }

  for (int i=0; i<64; i++)
  {
    colorhist[i] = (double)colorhist_abs[i] / numOfPixels;
  }
  return new DistData(64*sizeof(double), &colorhist);
}

/*******************************************************************************
Below, the avaliable metrics will be implemented:

*******************************************************************************/
/*
Method ~EuclideanInt~ :

*/
void MetricRegistry::EuclideanInt(
    const void* data1, const void* data2, double& result )
{
  int val1 = *static_cast<const int*>
      ( static_cast<const DistData*>( data1 )->value() );

  int val2 = *static_cast<const int*>
      ( static_cast<const DistData*>( data2 )->value() );

  result = abs( val1 - val2 );
}

/*
Method ~EuclideanReal~ :

*/
void MetricRegistry::EuclideanReal(
    const void* data1, const void* data2, double& result )
{
  SEC_STD_REAL val1 = *static_cast<const SEC_STD_REAL*>
      ( static_cast<const DistData*>( data1 )->value() );

  SEC_STD_REAL val2 = *static_cast<const SEC_STD_REAL*>
      ( static_cast<const DistData*>( data2 )->value() );

  result = abs( val1 - val2 );
}

/*
Method ~EditDistance~ :

*/
void MetricRegistry::EditDistance(
    const void* data1, const void* data2, double& result )
{
  const char* str1 = static_cast<const char*>
      ( static_cast<const DistData*>( data1 )->value() );
  const char* str2 = static_cast<const char*>
      ( static_cast<const DistData*>( data2 )->value() );

  int len1 = static_cast<const DistData*>( data1 )->size();
  int len2 = static_cast<const DistData*>( data2 )->size();

  int d[len1 + 1][len2 + 1];
  int dist;

  // init row 1 with
  for ( int i = 0; i <= len1; i++ )
    d[i][0] = i;

  // init col 1
  for ( int j = 1; j <= len2; j++ )
    d[0][j] = j;

  // compute array getValues
  for ( int i = 1; i <= len1; i++ )
  {
    for ( int j = 1; j <= len2; j++ )
    {
      if ( str1[i - 1] == str2[j - 1] )
        dist = 0;
      else
        dist = 1;

      // d(i,j) = min{ d( i-1 , j   ) + 1,
      //         d( i   , j-1 ) + 1,
      //         d( i-1 , j-1 ) + dist }
      d[i][j] = min( d[i - 1][j] + 1,
                min(( d[i][j - 1] ) + 1,
                 d[i - 1][j - 1] + dist ) );
    }
  }
  result = ( double ) d[len1][len2];
}

/*
Method ~HistogramMetric~ :

*/
void MetricRegistry::HistogramMetric(
    const void* data1, const void* data2, double& result )
{
  // TODO compute result value
}

/*
Method ~PictureMetric~ :

*/
void MetricRegistry::PictureMetric(
    const void* data1, const void* data2, double& result )
{
  const double* values1 = static_cast<const double*>
      ( static_cast<const DistData*>( data1 )->value() );

  const double* values2 = static_cast<const double*>
      ( static_cast<const DistData*>( data2 )->value() );

  result = 0;

  for (int i=0; i<64; i++)
    result += pow(( values1[i] - values2[i] ), 2);

  result = sqrt(result);
//   cout << result << endl;
}

/*
Method ~Initialize~ :

Insert a call of "registerMetric"[4] in the following method for every metric,
that should be avaliable for the using algebras. The "registerMetric"[4]
parameter have the folowing meanings:

  1 Name of the metric (must be unique for every type constructor)

  2 "MF[_]Data"[4] object, which contains the neccesary data for the metric

The MF[_]Data constructor takes the following parameter:

  1 Name of the associated type constructor.

  2 Reference to the method, which implements the metric.

  3 Parameter type ("DF[_]DATA"[4] or "DF[_]REFERENCE"[4])

  4 Description of the metric

*/
void
MetricRegistry::initialize()
{
  //int type constructor
  registerMetric( MF_DEFAULT,
      MetricData( "int", &EuclideanInt, &GetDataInt,
      "Euclidean distance metric" ));

  // real type constructor
  registerMetric( MF_DEFAULT,
      MetricData( "real", &EuclideanReal, &GetDataReal,
      "Euclidean distance metric" ));

  // string type constructor
  registerMetric( MF_DEFAULT,
      MetricData( "string", &EditDistance, GetDataString,
      "Edit distance metric" ));

  // histogram type constructor
  registerMetric( MF_DEFAULT,
      MetricData( "histogram", &HistogramMetric, &GetDataHistogram,
      "Not yet implemented" ));

  // picture type constructor
  registerMetric( MF_DEFAULT,
      MetricData( "picture", &PictureMetric, &GetDataPicture,
      "Not yet implemented" ));
}
