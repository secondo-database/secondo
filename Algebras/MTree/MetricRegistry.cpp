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

November/December 2007, Mirko Dibbert

//[_] [\_]

5.8 Implementation of class "MetricRegistry"[4] (file: MetricRegistry.cpp)

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
#include <fstream>

extern SecondoInterface* si;

/*
Initialize static members :

*/
bool MetricRegistry::initialized = false;
map< string, MetricRegistry::MetricData > MetricRegistry::metric_map;
map< string, string > MetricRegistry::defaults;

double* MetricRegistry::pictureSimMatrix1 = 0;
double* MetricRegistry::pictureSimMatrix2 = 0;
double* MetricRegistry::pictureSimMatrix3 = 0;
unsigned* MetricRegistry::pictureLabOffsetTable = 0;

/*
Method ~registerMetric~ :

The default metric will be stored without a name to ensure that this metric is
allways the first one for each type constructor, which "listMetrics"[4] put into
the result list. The algebra- and type-id are only used to order the output of
"listMetrics"[4] by these id's.

*/
void
MetricRegistry::registerMetric( const string& metricName,
                const MetricData& data, bool isDefault )
{
  int algebraId, typeId;
  si->GetTypeId( data.tcName, algebraId, typeId );

  ostringstream osId;
  osId << algebraId << "#" << typeId << "." << metricName;
  metric_map[osId.str()] = data;

  if ( isDefault )
    defaults[ data.tcName ] = metricName;
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

  if ( metricName == "default" )
  { // search default metric for type constructor 'tcName'
    map< string, string >::iterator pos = defaults.find( tcName );
    if ( pos != defaults.end() )
    {
      osId << pos->second;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    osId << metricName;
  }

  map< string, MetricData >::iterator pos =
      metric_map.find( osId.str() );

  if ( pos != metric_map.end() )
  {
    return pos->second.metric;
  }
  else
  {
    return 0;
  }
}

/*
Method ~getDefaultName~

*/
string
MetricRegistry::getDefaultName( const string& tcName )
{
  if (!initialized)
    initialize();

  int algebraId, typeId;
  si->GetTypeId( tcName, algebraId, typeId );
  ostringstream osId;
  osId << algebraId << "#" << typeId << ".";

  // search default metric for type constructor 'tcName'
  map< string, string >::iterator pos = defaults.find( tcName );
  if ( pos != defaults.end() )
  {
    return pos->second;
  }
  else
  {
    return "unknown";
  }
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

  if ( metricName == "default" )
  { // search default metric for type constructor 'tcName'
    map< string, string >::iterator pos = defaults.find( tcName );
    if ( pos != defaults.end() )
    {
      osId << pos->second;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    osId << metricName;
  }

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
      metricName = "default";

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
DistData* MetricRegistry::getDataInt( void* attr )
{
  int value = static_cast<CcInt*>(attr)->GetValue();
  char buffer[sizeof(int)];
  memcpy( buffer, &value, sizeof(int) );
  return new DistData( sizeof(int), buffer );
}

DistData* MetricRegistry::getDataReal( void* attr )
{
  SEC_STD_REAL value =
      static_cast<CcReal*>(attr)-> GetValue();
  char buffer[sizeof(SEC_STD_REAL)];
  memcpy( buffer, &value, sizeof(SEC_STD_REAL) );
  return new DistData( sizeof(SEC_STD_REAL), buffer );
}

DistData* MetricRegistry::getDataString( void* attr )
{
  string value = static_cast<CcString*>( attr )-> GetValue();
  return new DistData( value );
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

void rgb2hsv( unsigned char r, unsigned char g, unsigned char b,
              int &h, int &s, int &v )
{
    unsigned char rgbMin = min( min( r, g ), b );
    unsigned char rgbMax = max( max( r, g ), b );
    unsigned char delta = rgbMax - rgbMin;

    // compute h
    if ( delta == 0 )
    {
      h = 0;
    }
    else
    {
      if ( rgbMax == r )
      {
        h = 60 * (g - b) / delta;
      }
      else if ( rgbMax == g )
      {
        h = 120 * (g - b) / delta;
      }
      else // rgbMax == b
      {
        h = 240 * (g - b) / delta;
      }
    }

    if ( h < 0 )
      h += 360;

    // compute s
    if ( rgbMax == 0 )
      s = 0;
    else
      s = 255 * delta / rgbMax;

    // compute v
    v = rgbMax;
}

  void rgb2lab( unsigned char r_, unsigned char g_, unsigned char b_,
           signed char L, signed char a, signed char b )
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

/*
Method ~getDataPicture~

*/
DistData* MetricRegistry::getDataPicture( void* attr )
{
  unsigned long size;
  const char* imgdata = static_cast<Picture*>( attr )->
      GetJPEGData(size);

  JPEGPicture rgb( (unsigned char *) imgdata, size );

  unsigned long int rgbSize;
  unsigned char* rgbData = rgb.GetImageData( rgbSize );

  const unsigned int numOfPixels = rgbSize/3;

  unsigned long colorhist_abs[128];
  double colorhist[128];

  for (int i=0; i<128; i++)
    colorhist_abs[i] = 0;

  for (unsigned long pos=0; pos<(numOfPixels); pos++)
  {
    int h,s,v;
    unsigned char r = rgbData[(3*pos)];
    unsigned char g = rgbData[(3*pos)+1];
    unsigned char b = rgbData[(3*pos)+2];

    rgb2hsv( r, g, b, h, s, v );

    int h_offset = h/45;  // 8 parts
    int s_offset = s/64;  // 4 parts
    int v_offset = v/128; // 4 parts
    colorhist_abs[16*h_offset + 4*s_offset + v_offset]++;
  }

  for (int i=0; i<128; i++)
    colorhist[i] = (double)colorhist_abs[i] / numOfPixels;

  return new DistData(128*sizeof(double), &colorhist);
}

/*
Method ~getDataPicture2~

*/
DistData* MetricRegistry::getDataPicture2( void* attr )
{
  unsigned long size;
  const char* imgdata = static_cast<Picture*>( attr )->
      GetJPEGData(size);

  JPEGPicture rgb( (unsigned char *) imgdata, size );

  unsigned long int rgbSize;
  unsigned char* rgbData = rgb.GetImageData( rgbSize );

  const unsigned int numOfPixels = rgbSize/3;

  unsigned long colorhist_abs[256];
  double colorhist[256];

  for (int i=0; i<256; i++)
    colorhist_abs[i] = 0;

  for (unsigned long pos=0; pos<(numOfPixels); pos++)
  {
    int h,s,v;
    unsigned char r = rgbData[(3*pos)];
    unsigned char g = rgbData[(3*pos)+1];
    unsigned char b = rgbData[(3*pos)+2];

    rgb2hsv( r, g, b, h, s, v );

    int h_offset = (int)(h/22.5); // 16 parts
    int s_offset = s/64;          // 4 parts
    int v_offset = v/128;         // 4 parts
    colorhist_abs[16*h_offset + 4*s_offset + v_offset]++;
  }

  for (int i=0; i<256; i++)
    colorhist[i] = (double)colorhist_abs[i] / numOfPixels;

  return new DistData(256*sizeof(double), &colorhist);
}

/*
Method ~getDataPicture3~

*/
DistData* MetricRegistry::getDataPicture3( void* attr )
{
  if (!pictureLabOffsetTable)
  {
    pictureLabOffsetTable = new unsigned[64*64*64];
    for (signed char r=0; r<64; r++)
      for (signed char g=0; g<64; g++)
        for (signed char b=0; b<64; b++)
        {
          Lab lab(2+(r*4), 2+(g*4), 2+(b*4));

          // map values [0, 99] x [-86, 98] x [-107,94] to
          // [0, 3] x [0, 7] x [0, 7] (6 x 8 x 8 = 384 bins)
          int L_offset = (int)(lab.L / 25);
          int a_offset = (int)((lab.a+86) / 23.125);
          int b_offset = (int)((lab.b+107) / 25.1);
          pictureLabOffsetTable[r*4096 + g*64 + b] =
              64*L_offset + 8*a_offset + b_offset;
        }
  }

  unsigned long size;
  const char* imgdata = static_cast<Picture*>( attr )->
      GetJPEGData(size);

  JPEGPicture rgb( (unsigned char *) imgdata, size );

  unsigned long int rgbSize;
  unsigned char* rgbData = rgb.GetImageData( rgbSize );

  const unsigned int numOfPixels = rgbSize/3;

  unsigned long colorhist_abs[256];
  double colorhist[256];

  for (int i=0; i<256; i++)
    colorhist_abs[i] = 0;

  for (unsigned long pos=0; pos < numOfPixels; pos++)
  {
    unsigned char r = static_cast<unsigned char>
        ( rgbData[(3*pos)] / 4 );

    unsigned char g = static_cast<unsigned char>
        ( rgbData[(3*pos)+1] / 4 );

    unsigned char b = static_cast<unsigned char>
        ( rgbData[(3*pos)+2] / 4 );

    colorhist_abs[pictureLabOffsetTable[(r*4096) + (g*64) + b] ]++;
  }

  for (int i=0; i<256; i++)
    colorhist[i] = (double)colorhist_abs[i] / numOfPixels;

  return new DistData(256*sizeof(double), &colorhist);
}

/*
Method PictureMetric1 :

*/
void MetricRegistry::PictureMetric(
    const void* data1, const void* data2, double& result )
{
  if (!pictureSimMatrix1)
  {
    pictureSimMatrix1 = new double[128*128];
    for (int h1=0; h1<8; h1++)
      for (int s1=0; s1<4; s1++)
        for (int v1=0; v1<4; v1++)
          for (int h2=0; h2<8; h2++)
            for (int s2=0; s2<4; s2++)
              for (int v2=0; v2<4; v2++)
                {
                  double d1 = pow(0.25 * (v1-v2), 2);
                  double d2 = pow((0.125+(s1*0.25)) * cos((h1*45.0))-
                              (0.125+(s2*0.25)) * cos((h2*45.0)), 2);
                  double d3 = pow((0.125+(s1*0.25)) * sin((h1*45.0))-
                              (0.125+(s2*0.25)) * sin((h2*45.0)), 2);

                  int pos1 = (16*h1)+(4*s1)+v1;
                  int pos2 = (16*h2)+(4*s2)+v2;
                  pictureSimMatrix1[pos1*128 + pos2]
                      = exp((-2)*sqrt(d1+d2+d3));
                }
  }

  const double* values1 = static_cast<const double*>
      ( static_cast<const DistData*>( data1 )->value() );

  const double* values2 = static_cast<const double*>
      ( static_cast<const DistData*>( data2 )->value() );

  long double res = 0;

  for (int pos1 = 0; pos1<128; pos1++)
    for (int pos2 = 0; pos2<128; pos2++)
    {
      res += ( (values1[pos1] - values2[pos1])  *
              ( values1[pos2] - values2[pos2]) *
                pictureSimMatrix1[pos1*128 + pos2] );
    }

   result = sqrt(res);
}

/*
Method PictureMetric2 :

*/
void MetricRegistry::PictureMetric2(
    const void* data1, const void* data2, double& result )
{
  if (!pictureSimMatrix2)
  {
    pictureSimMatrix2 = new double[256*256];
    for (int h1=0; h1<16; h1++)
      for (int s1=0; s1<4; s1++)
        for (int v1=0; v1<4; v1++)
          for (int h2=0; h2<16; h2++)
            for (int s2=0; s2<4; s2++)
              for (int v2=0; v2<4; v2++)
                {
                  double d1 = pow(0.25 * (v1-v2), 2);
                  double d2 = pow((0.125+(s1*0.25)) * cos((h1*22.5))-
                              (0.125+(s2*0.25)) * cos((h2*22.5)), 2);
                  double d3 = pow((0.125+(s1*0.25)) * sin((h1*22.5))-
                              (0.125+(s2*0.25)) * sin((h2*22.5)), 2);

                  int pos1 = (16*h1)+(4*s1)+v1;
                  int pos2 = (16*h2)+(4*s2)+v2;
                  pictureSimMatrix2[pos1*128 + pos2]
                      = exp((-2)*sqrt(d1+d2+d3));
                }
  }

  const double* values1 = static_cast<const double*>
      ( static_cast<const DistData*>( data1 )->value() );

  const double* values2 = static_cast<const double*>
      ( static_cast<const DistData*>( data2 )->value() );

  long double res = 0;

  for (int pos1 = 0; pos1<256; pos1++)
    for (int pos2 = 0; pos2<256; pos2++)
    {
      res += ( (values1[pos1] - values2[pos1])  *
              ( values1[pos2] - values2[pos2]) *
                pictureSimMatrix2[pos1*256 + pos2] );
    }

   result = sqrt(res);
}

/*
Method PictureMetric3 :

*/
void MetricRegistry::PictureMetric3(
    const void* data1, const void* data2, double& result )
{
  // generate similarity matrix for this metric, if it doesen't exist
  if (!pictureSimMatrix3)
  {
    pictureSimMatrix3 = new double[256*256];
    for (int L1=0; L1<4; L1++)
      for (int a1=0; a1<8; a1++)
        for (int b1=0; b1<8; b1++)
          for (int L2=0; L2<4; L2++)
            for (int a2=0; a2<8; a2++)
              for (int b2=0; b2<8; b2++)
              {
                double d = pow(25.0 * (L1-L2), 2) +
                           pow(23.125 * (a1-a2), 2) +
                           pow(25.250 * (b1-b2), 2);

                int pos1 = (64*L1)+(8*a1)+b1;
                int pos2 = (64*L2)+(8*a2)+b2;
                pictureSimMatrix3[(pos1*256) + pos2] =
                    exp((-2)*sqrt(d));
              }
  }

  // get histogram1 from data1
  const double* hist1 = static_cast<const double*> (
      static_cast<const DistData*>( data1 )->value() );

  // get histogram2 from data1
  const double* hist2 = static_cast<const double*> (
      static_cast<const DistData*>( data2 )->value() );

  // compute result
  long double res = 0;

  double histDiff[256];
  for (int pos = 0; pos<256; pos++)
    histDiff[pos] = hist1[pos] - hist2[pos];

  for (int pos1 = 0; pos1<256; pos1++)
    for (int pos2 = 0; pos2<256; pos2++)
      res += histDiff[pos1] * histDiff[pos2] *
             pictureSimMatrix3[(pos1*256) + pos2];

  result = sqrt(res);
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
  if ( initialized )
    return;

  const bool isDefault = true;

  //int type-constructor
  registerMetric( "Euclid", MetricData(
      "int", &EuclideanInt, &getDataInt,
      "Euclidean distance metric"
      ), isDefault );

  // real type-constructor
  registerMetric( "Euclid", MetricData(
      "real", &EuclideanReal, &getDataReal,
      "Euclidean distance metric"
      ), isDefault );

  // string type-constructor
  registerMetric( "EditDist", MetricData(
      "string", &EditDistance, &getDataString,
      "Edit distance metric"
      ), isDefault );

  // picture type-constructor (hsv128)
  registerMetric( "hsv128", MetricData(
      "picture", &PictureMetric, &getDataPicture,
      "Quadratic distance metric (HSV hisogram with 128 bins)"
      ));

  // picture type-constructor (hsv256)
  registerMetric( "hsv256", MetricData(
      "picture", &PictureMetric2, &getDataPicture2,
      "Quadratic distance metric (HSV hisogram with 256 bins)"
      ));

  // picture type-constructor (lab)
  registerMetric( "lab", MetricData(
      "picture", &PictureMetric3, &getDataPicture3,
      "Quadratic distance metric (LAB histogram with 256 bins)"
      ), isDefault );

  initialized = true;
}
