/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]

[1] Definition and Implementation of the class ~NumericUtil~

April - November 2008, M. H[oe]ger for bachelor thesis.


[TOC]

1 Introduction

This file contains the definition and implementation of the class
~NumericUtil~ which provides several static methods to compare 
floating point values.

1 Defines and Includes

*/

#ifndef NUMERICUTIL_H_
#define NUMERICUTIL_H_

#include <limits>

using namespace std;

namespace mregionops {

const double MAX_DOUBLE = numeric_limits<double>::max();
const double MIN_DOUBLE = -MAX_DOUBLE;

/*
1 Class NumericUtil

*/

class NumericUtil {
	
public:
    
    //static const double eps = 0.01;
    //static const double eps = 0.001;
	//static const double eps = 0.0001;
	//static const double eps = 0.00001;
	static const double eps = 0.000001;
	//static const double eps = 0.0000001;
    //static const double eps = 0.00000001;
	
	
/*
1.1 NearlyEqual

Returns ~true~ if $-eps \le a-b \le eps$.

*/
	inline static bool NearlyEqual(double a, double b) {
		
	    return fabs(a - b) <= eps;
	}
	
	inline static bool NearlyEqual(double a, 
	                               double b, 
	                               double _eps) {

        return fabs(a - b) < _eps;
    }
	
/*
1.1 LowerOrNearlyEqual

Returns ~true~ if $a \le b+eps$.

*/
	inline static bool LowerOrNearlyEqual(double a, double b) {
		
	    return a < b || NearlyEqual(a, b);
	}
	
	inline static bool LowerOrNearlyEqual(double a, 
	                                      double b, 
	                                      double _eps) {

        return a < b || NearlyEqual(a, b, _eps);
    }

/*
1.1 Lower

Returns ~true~ if $a < b-eps$.

*/
	inline static bool Lower(double a, double b) {
		
	    return a < b - eps;
	}
	
	inline static bool Lower(double a, double b, double _eps) {

        return a < b - _eps;
    }

/*
1.1 GreaterOrNearlyEqual

Returns ~true~ if $a \ge b-eps$.

*/
	inline static bool GreaterOrNearlyEqual(double a, double b) {
		
	    return a > b || NearlyEqual(a, b);
	}
	
	inline static bool GreaterOrNearlyEqual(double a, 
	                                        double b, 
	                                        double _eps) {

        return a > b || NearlyEqual(a, b, _eps);
    }
	
/*
1.1 Greater

Returns ~true~ if $a > b+eps$.

*/
    inline static bool Greater(double a, double b) {

        return a > b + eps;
    }

    inline static bool Greater(double a, 
                               double b, 
                               double _eps) {

        return a > b + _eps;
    }

/*
1.1 Between

Returns ~true~ if $a-eps \le x \le b+eps$ or $b-eps \le x \le a+eps$.

*/
	inline static bool Between(double a, double x, double b) {
		
	    return (LowerOrNearlyEqual(a, x) && 
	            LowerOrNearlyEqual(x, b)) || 
	           (LowerOrNearlyEqual(b, x) && 
	            LowerOrNearlyEqual(x, a));
	}
	
	inline static bool Between(double a, 
	                           double x, 
	                           double b, 
	                           double _eps) {
	        
	    return (LowerOrNearlyEqual(a, x, _eps) && 
	            LowerOrNearlyEqual(x, b, _eps)) || 
	           (LowerOrNearlyEqual(b, x, _eps) && 
	            LowerOrNearlyEqual(x, a, _eps));
	}
	
/*
1.1 MinMax4

Returns the minimum and maximum value of $a$, $b$, $c$ and $d$.

*/
	static pair<double, double> 
	MinMax4(double a, double b, double c, double d) {

		double min = a;
		double max = a;

		if (b < min)
			min = b;
		if (b > max)
			max = b;
		if (c < min)
			min = c;
		if (c > max)
			max = c;
		if (d < min)
			min = d;
		if (d > max)
			max = d;

		return pair<double, double>(min, max);
	}
	
private:
	
	NumericUtil() {
		
	}
};

/*
1 Struct DoubleCompare

*/

struct DoubleCompare {

    inline bool operator()(const double& d1, const double& d2) const {
        
        return NumericUtil::Lower(d1, d2);
    }
};

} // end of namespace mregionops

#endif /*NUMERICUTIL_H_*/

