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



double euclidDistance(FeatureSignatureTuple ist1,
                    FeatureSignatureTuple ist2)
{
	double tmpRes = 0.0;
    
	tmpRes += std::pow(ist1.centroid.x - ist2.centroid.x, 2);
	tmpRes += std::pow(ist1.centroid.y - ist2.centroid.y, 2);
	tmpRes += std::pow(ist1.centroid.colorValue1 
    - ist2.centroid.colorValue1, 2);
	tmpRes += std::pow(ist1.centroid.colorValue2 
    - ist2.centroid.colorValue2, 2);
	tmpRes += std::pow(ist1.centroid.colorValue3 
    - ist2.centroid.colorValue3, 2);
	tmpRes += std::pow(ist1.centroid.coarseness 
    - ist2.centroid.coarseness, 2);
	tmpRes += std::pow(ist1.centroid.contrast 
    - ist2.centroid.contrast, 2);
	
	return std::sqrt(tmpRes);	
}



 
// Alternatively, the negative function can be used as similarity
// function 
 
//double f_negative(FeatureSignatureTuple fst1,
//                    FeatureSignatureTuple fst2)
//{
//	double dist = euclidDist(fst1, fst2);
//	return -dist;
//}
 


// heuristic similarity function
double f_s(FeatureSignatureTuple fst1,
                    FeatureSignatureTuple fst2)
{
	const double alpha = 2.0;
	double eucDist = euclidDistance(fst1, fst2);		
			
	double res = 1.0 /  (alpha + eucDist);
	return res;
}





// structure to express flows used in EMD
struct Flow
{
  double fromWeight;
  double distance;
  double toWeight;
};
