/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] 

April 2008, initial version created by M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

*/


#ifndef NUMERICUTIL_H_
#define NUMERICUTIL_H_

using namespace std;

namespace mregionops {

class NumericUtil {
	
public:
	
	//static const double eps = 0.0001;
	//static const double eps = 0.00000001;
	static const double eps = 0.00001;
	static const double epsRelaxFactor = 10;
	//static const double MAXDOUBLE = numeric_limits<double>::max();
	//static const double MINDOUBLE = numeric_limits<double>::min();
	
	/*
	1.1.1 Function ~nearlyEqual()~

	Returns ~true~ if $-eps \le a-b \le eps$.

	*/
	inline static bool NearlyEqual(double a, double b) {
		
	    return fabs(a - b) <= eps;
	}
	
	/*
	1.1.1 Function ~lowerOrNearlyEqual()~

	Returns ~true~ if $a \le b+eps$.

	*/
	inline static bool LowerOrNearlyEqual(double a, double b) {
		
	    return a < b || NearlyEqual(a, b);
	}

	/*
	1.1.1 Function ~lower()~

	Returns ~true~ if $a < b-eps$.

	*/
	inline static bool Lower(double a, double b) {
		
	    return a < b - eps;
	}

	/*
	1.1.1 Function ~greaterOrNearlyEqual()~

	Returns ~true~ if $a \ge b-eps$.

	*/
	inline static bool GreaterOrNearlyEqual(double a, double b) {
		
	    return a > b || NearlyEqual(a, b);
	}
	
	/*
		1.1.1 Function ~greater()~

		Returns ~true~ if $a > b+eps$.

		*/
		inline static bool Greater(double a, double b) {
			
		    return a > b + eps;
		}

	/*
	1.1.1 Function ~between()~

	Returns ~true~ if $a-eps \le x \le b+eps$ or $b-eps \le x \le a+eps$.

	*/
	inline static bool Between(double a, double x, double b) {
		
	    return (LowerOrNearlyEqual(a, x) && LowerOrNearlyEqual(x, b)) || 
	           (LowerOrNearlyEqual(b, x) && LowerOrNearlyEqual(x, a));
	}
	
	/*
	1.1 Function ~minmax4()~

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

} // end of namespace mregionops

#endif /*NUMERICUTIL_H_*/

