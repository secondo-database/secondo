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

1 Declaration file for functions used by EMD and SQFD

June 2017, Michael Loris

*/


#include "../ImageSimilarity/JPEGImage.h"
#include <math.h>
#include <vector>
#include <iostream>
#include <cmath>


// round to a given precision in order to prevent C++ rounding errors
double fiveRound(double src, int precision) {
         int dst;
         double tmp;
         double result;
         tmp = src * pow(10, precision);
         if(tmp < 0) 
         {
            dst = (int)(tmp - 0.5);
         }
         else 
         {
            dst = (int)(tmp + 0.5);
         }
         result = (double)((double)dst * pow(10, -precision));
         return result;
}


// specialized euclidean distance for PCT feature signatures
// P = position, C = color, T= texture
double euclidDist(FeatureSignatureTuple ist1,
                    FeatureSignatureTuple ist2)
{
	double tmpRes = 0.0;
	
	tmpRes += pow(ist1.weight - ist2.weight, 2);
	tmpRes += pow(ist1.centroid.x - ist2.centroid.x, 2);
	tmpRes += pow(ist1.centroid.y - ist2.centroid.y, 2);
	tmpRes += pow(ist1.centroid.r - ist2.centroid.r, 2);
	tmpRes += pow(ist1.centroid.g - ist2.centroid.g, 2);
	tmpRes += pow(ist1.centroid.b - ist2.centroid.b, 2);
	tmpRes += pow(ist1.centroid.coarseness 
				- ist2.centroid.coarseness, 2);
	tmpRes += pow(ist1.centroid.contrast - ist2.centroid.contrast, 2);
	return sqrt(tmpRes);	
}

double euclidDist(FeatureSignatureTuple ist1,
                    FeatureSignatureTuple ist2)
{
	double tmpRes = 0.0;
	
	tmpRes += pow(ist1.weight - ist2.weight, 2);
	tmpRes += pow(ist1.centroid.x - ist2.centroid.x, 2);
	tmpRes += pow(ist1.centroid.y - ist2.centroid.y, 2);
	tmpRes += pow(ist1.centroid.r - ist2.centroid.r, 2);
	tmpRes += pow(ist1.centroid.g - ist2.centroid.g, 2);
	tmpRes += pow(ist1.centroid.b - ist2.centroid.b, 2);
	tmpRes += pow(ist1.centroid.coarseness 
				- ist2.centroid.coarseness, 2);
	tmpRes += pow(ist1.centroid.contrast - ist2.centroid.contrast, 2);
	return tmpRes;	
}

// similarity function used by SQFD
static double f_s(FeatureSignatureTuple fst1, 
            FeatureSignatureTuple fst2)
{	
	double res = 1.0 / (1.0 + euclidDist(fst1, fst2));	
	return res;
}


static double f_s_gauss(FeatureSignatureTuple fst1, 
            FeatureSignatureTuple fst2)
{	
    const double e = std::exp(1.0);
    const double alpha  = -0.9;
    return pow(e, alpha * euclidDistSquare(fst1, fst2));
	//double res = 1.0 / (1.0 + euclidDist(fst1, fst2));	
	//return res;
}


// structure to express flows used in EMD
struct Flow
{
  double fromWeight;
  double distance;
  double toWeight;
};
