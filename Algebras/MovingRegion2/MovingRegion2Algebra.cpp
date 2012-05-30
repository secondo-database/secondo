/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

\setcounter{tocdepth}{4}

[1] The ~MovingRegion2Algebra~

May 2012, initial version created by Stefanie Renner for master
thesis with Prof. Dr. G[ue]ting, Fachbereich Informatik,
Fernuniversit[ae]t Hagen.

[TOC]

1 Introduction

The ~MovingRegion2Algebra~ provides data types and operators relating to
moving regions. It is the second attempt to implement an algebra for moving regions
and focuses especially on numerical robustness. Therefor all coordinates are
split into an integer part and the non-integer rest. Calculations are first done on
the integer grid using the integer coordinates only. If no decision can be made
that way, the precise coordinates are taken into account.

Due to the deadline of the master's thesis and the
complexity of the ~MovingRegion2Algebra~, the current version should be
considered as prototype or proof of concept only, and needs to be finalized
before production usage.

The class definitions of ~MSegmentData2~, PreciseMSegmentData, PreciseInterval,
~URegionEmb2~, ~URegion2~ and ~MRegion2~, which
are implemented in ~MovingRegion2Algebra.cpp~, have been moved to
the header file ~MovingRegion2Algebra.h~ to facilitate development work on
top of the ~MovingRegion2Algebra~ without modifying the ~MovingRegion2Algebra~
itself. This file contains detailed descriptions for the usage of the methods
of these classes too.

1 Defines and includes
\label{defines}

*/


#include <queue>
#include <stdexcept>

//includes for the Gnu Multiple Precision (GMP) library
#include <gmp.h>
#include <gmpxx.h>

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include "MovingRegion2Algebra.h"
#include "DateTime.h"
#include "Symbols.h"


extern NestedList* nl;
extern QueryProcessor* qp;

/*
Set ~MR2\_DEBUG~ to ~true~ for debug output. Please note that debug output is
very verbose and has significant negative input on the algebra's performance.
Only enable debug output if you know what you are doing!

*/
const bool MR2_DEBUG = true;

/*
~eps2~ is used for comparison of two double values.

*/
double eps2 = 0.00001;

/*
~gmpCharNum~ specifies the number of elements reserved for each coordinate  in the
DbArray$<$char$>$ for the preciseCoordinates and preciseInstants. If a given coordinate
value needs more elements, another ~gmpCharNum~ elements will be used, and so on.
the last element of each group of ~gmpCharNum~ elements is always the index of the
next of these groups representing the same coordinate.

*/
unsigned int gmpCharNum = 20;

/*
1.1 Function ~simpleSelect()~

Simple selection function for non-overloaded operators.

*/
static int simpleSelect(ListExpr args) {
    return 0;
}

/*
1.1 Comparison functions for ~double~

To avoid rounding issues, comparisons for equality are done using constant
~eps2~.

1.1.1 Function ~nearlyEqual()~

Returns ~true~ if $-eps2 \le a-b \le eps2$.

*/
static bool nearlyEqual(double a, double b) {
    return abs(a-b) <= eps2;
}

/*
1 Helper functions

1.1 General helper functions

1.1.1 Numerical comparison functions

1.1.1.1 Function ~minmax8()~

Returns the minimum and maximum value of $a$, $b$, $c$, $d$, $e$, $f$, $g$
and $h$ in $min$ and $max$.

*/
static void minmax8(
		double a,
		double b,
		double c,
		double d,
		double e,
		double f,
		double g,
		double h,
		double& min,
		double& max) {

    min = a;
    max = a;

    if (b < min) min = b;
    if (b > max) max = b;
    if (c < min) min = c;
    if (c > max) max = c;
    if (d < min) min = d;
    if (d > max) max = d;
    if (e < min) min = e;
	if (e > max) max = e;
	if (f < min) min = f;
	if (f > max) max = f;
	if (g < min) min = g;
	if (g > max) max = g;
	if (h < min) min = h;
	if (h > max) max = h;
}

/*
1.1.1.1 Function ~minmax4()~

Returns the minimum and maximum value of $a$, $b$, $c$ and $d$ in $min$
and $max$.

*/
static void minmax4(
		int a,
		int b,
		int c,
		int d,
		int& min,
		int& max) {
	min = a;
	max = a;

	if (b < min) min = b;
	if (b > max) max = b;
	if (c < min) min = c;
	if (c > max) max = c;
	if (d < min) min = d;
	if (d > max) max = d;
}

static void minmax4(
		mpq_class a,
		mpq_class b,
		mpq_class c,
		mpq_class d,
		mpq_class& min,
		mpq_class& max) {

	min = a;
	max = a;

	if (cmp(b, min) < 0) min = b;
	if (cmp(b, max) > 0) max = b;
	if (cmp(c, min) < 0) min = c;
	if (cmp(c, max) > 0) max = c;
	if (cmp(d, min) < 0) min = d;
	if (cmp(d, max) > 0) max = d;

}

/*
1.1.1.1 Function ~maybeEqual()~

Checks if two values may be equal, having in mind there respective maximal errors.
Function ~maybeEqual()~ is provided for data type double and for the different
geometric classes, like GridPoint, GridPointSegment, ProvisionalPoint, and so on.
These functions will help to detect such situations where a recalculation with
precise coordinates is needed.

*/
static bool maybeEqual(double a, double b, double aErr, double bErr) {
	if (a + aErr + bErr < b || b + aErr + bErr < a)
	{
		return false;
	}
	else
	{
		return true;
	}
}

static bool maybeEqual(ProvisionalPoint point1,
		ProvisionalPoint point2) {
    return (maybeEqual(point1.x, point2.x, point1.xErr, point2.xErr) &&
		maybeEqual(point1.y, point2.y, point1.yErr, point2.yErr));
}

static bool maybeEqual(GridPoint point1, GridPoint point2) {
	return (maybeEqual(point1.x, point2.x, 1.0, 1.0) &&
		maybeEqual(point1.y, point2.y, 1.0, 1.0));
}

static bool maybeEqual(ProvisionalSegment segment1,
		ProvisionalSegment segment2) {
	return ((maybeEqual(segment1.point1, segment2.point1) &&
		maybeEqual(segment1.point2, segment2.point2)) ||
		(maybeEqual(segment1.point2, segment2.point1) &&
		maybeEqual(segment1.point1, segment2.point2)));
}

static bool maybeEqual(GridPointSegment segment1,
		GridPointSegment segment2) {
	return ((maybeEqual(segment1.point1, segment2.point1) &&
		maybeEqual(segment1.point2, segment2.point2)) ||
		(maybeEqual(segment1.point2, segment2.point1) &&
		maybeEqual(segment1.point1, segment2.point2)));
}

static bool maybeEqual(ProvisionalTrapezium trapezium1,
		ProvisionalTrapezium trapezium2) {
	return ((maybeEqual(trapezium1.segment1, trapezium2.segment1)
		&& maybeEqual(trapezium1.segment2, trapezium2.segment2)) ||
		(maybeEqual(trapezium1.segment2, trapezium2.segment1)
		&& maybeEqual(trapezium1.segment1, trapezium2.segment2)));
}

static bool maybeEqual(GridPointTrapezium trapezium1,
		GridPointTrapezium trapezium2) {
	return ((maybeEqual(trapezium1.segment1, trapezium2.segment1)
		&& maybeEqual(trapezium1.segment2, trapezium2.segment2)) ||
		(maybeEqual(trapezium1.segment2, trapezium2.segment1)
		&& maybeEqual(trapezium1.segment1, trapezium2.segment2)));
}

/*
1.1.1.1 Function ~lowerOrMaybeEqual()~

Checks if two values are closer to each other than the critical radius,
or if the first value is below the second.
Needed to detect such situations where a recalculation with precise coordinates
is needed.

*/
static bool lowerOrMaybeEqual
	(double a, double b, double aErr, double bErr) {

	if (a < b || maybeEqual(a, b, aErr, bErr))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
1.1.1.1 Function ~isLower()~

Checks if a is lower than b, keeping in mind the respective errors, so that no
precise recalculation is needed.

*/
static bool isLower
	(double a, double b, double aErr, double bErr) {

	if (a + aErr < b - bErr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
1.1.1.1 Function ~isGreater()~

Checks if a is greater than b, keeping in mind the respective errors, so that no
precise recalculation is needed.

*/
static bool isGreater
	(double a, double b, double aErr, double bErr) {

	if (a - aErr > b + bErr)
	{
		return true;
	}
	return false;
}

/*
1.1.1.1 Function ~greaterOrMaybeEqual()~

Checks if value a is greater or maybe equal with value b, keeping in mind
their respective errors.
Needed to detect such situations where a recalculation with precise coordinates
is needed.

*/
static bool greaterOrMaybeEqual
	(double a, double b, double aErr, double bErr) {

	if (a > b || maybeEqual(a, b, aErr, bErr))
	{
		return true;
	}
	return false;
}

/*
1.1.1.1 Function ~isBetween()~

Returns true if b is between a and c for sure, keeping in mind the
respective errors in the case of double parameters, so that no precise
recalculation is needed.

*/
static bool isBetween(double a, double b, double c, double aErr,
		double bErr, double cErr) {
	if ((a + aErr < b - bErr && b + bErr < c - cErr) ||
			(c + cErr < b - bErr && b + bErr < a - aErr))
	{
		return true;
	}
	return false;
}

static bool preciseBetween
	(mpq_class a, mpq_class b, mpq_class c) {

	if ((cmp(a,b) < 0 && cmp(b,c) < 0 )
		|| (cmp(c,b) < 0 && cmp(b,a) < 0 ))
	{
		return true;
	}
	return false;
}

static bool preciseBetweenOrEqual
	(mpq_class a, mpq_class b, mpq_class c) {

	if ((cmp(a,b) <= 0 && cmp(b,c) <= 0 )
		|| (cmp(c,b) <= 0 && cmp(b,a) <= 0 ))
	{
		return true;
	}
	return false;
}

static bool isBetween
	(mpq_class a, double b, mpq_class c) {

	if ((cmp(a,b) < 0 && cmp(b,c) < 0 )
		|| (cmp(c,b) < 0 && cmp(b,a) < 0 ))
	{
		return true;
	}
	return false;
}

static bool isBetween
	(double a, mpq_class b, double c) {

	if ((cmp(a,b) < 0 && cmp(b,c) < 0 )
		|| (cmp(c,b) < 0 && cmp(b,a) < 0 ))
	{
		return true;
	}
	return false;
}

/*
1.1.1.1 Function ~betweenOrMaybeEqual()~

Returns true if b is between a and c or maybe of equal value with one of the two
or with both, so that it might be between the two values.

*/
static bool betweenOrMaybeEqual(double a, double b, double c,
		double aErr, double bErr, double cErr) {
	if (isBetween(a, b, c, aErr, bErr, cErr)
			|| maybeEqual(a, b, aErr, bErr) ||
			maybeEqual(b, c, bErr, cErr))
	{
		return true;
	}
	return false;
}

/*
1.1.1 Other mathematical helper functions

1.1.1.1 Function ~roundedUp()~

Rounds a double up to the next higher integer.

*/
static int roundedUp (double doubleValue)
{
	return ((int)doubleValue ==
		doubleValue ? (int)doubleValue :
			(int)doubleValue + 1);
}

/*
1.1.1.1 Function ~broughtDown()~

Brings a double down to the next lower integer (same as cast, just for readability).

*/
static int broughtDown (double doubleValue)
{
	return (int)doubleValue;
}

/*
1.1.1.1 Function ~GaussTransform()~

Apply the Gauss transformation to system of equations. The left hand sides
of the equations are in matrix $a$ with $n$ rows and $m$ columns and the
right hand sides are in vector $b$ with $n$ rows.
The maximal errors of the left hand side components are in matrix $aMaxErr$,
the maximal errors of the right hand side components are in vector $bMaxErr$.
The maximal errors can be used to detect cases where a precise recalculation
will be necessary at some point.
In order to keep the maximal errors small, the equations will be sorted during
the transformation, so that factors by which an equation needs to be
multiplied remain as small as possible.

The transformed matrix and vector are returned in $a$ and $b$ if successfull.

*/
static void GaussTransform(const unsigned int n,
		   const unsigned int m,
		   double** a,
		   double* b,
		   double** aMaxErr,
		   double* bMaxErr) {
	if (MR2_DEBUG)
	{
		cerr << "GaussTransform() called, n="
			 << n << " m=" << m << endl;

		for (unsigned int j = 0; j < n; j++)
		{
			for (unsigned int k = 0; k < m; k++)
			{
				fprintf(stderr, "%7.3f ", a[j][k]);
			}
			fprintf(stderr, "| %7.3f\n", b[j]);
		}
		cerr << endl;
		cerr << "Maximal errors will be transformed as well."
			<< endl;
		for (unsigned int j = 0; j < n; j++)
		{
			for (unsigned int k = 0; k < m; k++)
			{
				fprintf(stderr, "%7.3f ", aMaxErr[j][k]);
			}
			fprintf(stderr, "| %7.3f\n", bMaxErr[j]);
		}
		cerr << "============================="
			 << "============================="
			 << endl;
	}

	double currentPivotValue;
	unsigned int currentPivotLine;

/*
For each row...

*/
    for (unsigned int i = 0; i < n-1; i++)
    {
/*
Check for non-zero element in column $i$ below and including row $i$, which
can be used as pivot. Search for the greatest possible pivot in these rows.

*/
        if (MR2_DEBUG)
        {
        	cerr << "i=" << i << endl;
        }

        currentPivotValue = 0.0;
        currentPivotLine = i;

        unsigned int j;
        for (j = i; j < n; j++)
        {
        	if (a[j][i] == 0)
        	{

        		if (MR2_DEBUG)
				{
					cerr << " failed as pivot: a["
						 << j
						 << "]["
						 << i
						 << "]="
						 << a[j][i]
						 << endl;
				}
        	}
        	else
        	{
        		if (MR2_DEBUG)
				{
					cerr << " possible pivot: a["
						 << j
						 << "]["
						 << i
						 << "]="
						 << a[j][i]
						 << endl;
				}
        		if (currentPivotValue < a[j][i])
        		{
        			currentPivotValue = a[j][i];
        			currentPivotLine = j;
        		}
        	}
        }

/*
If we found a pivot, apply it to the rows below row $i$. If we did not find
a pivot, column $i$ is already zero below and including row $i$ and we do
not have to do anything.

*/
        if (currentPivotValue != 0.0)
        {
            if (MR2_DEBUG)
            {
                cerr << " taking as pivot: a["
                     << currentPivotLine
                     << "]["
                     << i
                     << "]="
                     << currentPivotValue
                     << endl;
            }

/*
The pivot is in row $currentPivotLine$. If $currentPivotLine\ne i$, the pivot is in another row than row $i$.
Swap rows $i$ and $currentPivotLine$ in this case.
Apply the same operation on the maximal errors of the respective components.

*/
            if (currentPivotLine != i)
            {
                double dummy = b[i];
                double dummyErr = bMaxErr[i];
                b[i] = b[currentPivotLine];
                bMaxErr[i] = bMaxErr[currentPivotLine];
                b[currentPivotLine] = dummy;
                bMaxErr[currentPivotLine] = dummyErr;
                for (unsigned int k = 0; k < m; k++)
                {
                    dummy = a[i][k];
                    dummyErr = aMaxErr[i][k];
                    a[i][k] = a[currentPivotLine][k];
                    aMaxErr[i][k] = aMaxErr[currentPivotLine][k];
                    a[currentPivotLine][k] = dummy;
                    aMaxErr[currentPivotLine][k] = dummyErr;
                }
            }

/*
Subtract row $i$ from each row below row $i$, multiplied with factor $f$,
which is calculated so that it sets the element in column $i$ of each row
to $0$ during the substraction.
The maximal error of the respective component is also multiplied with the
same factor, taking its absolute value, and during the substraction
the maximal errors of the respective components are added.

*/
            for (j = i+1; j < n; j++)
            {
			  double f = a[j][i]/a[i][i];
			  if (MR2_DEBUG && f > 1)
			  {
				cerr << "Warning: "
				<< "GaussTransform not optimal! "
				<< "Error in implementation "
					"of line sorting! "
				<< endl <<
				"Maximal errors "
				"not kept minimal! " <<
				"Factor > 1 calculated!"
				<< endl << endl;
			  }
			  if (MR2_DEBUG)
			  {
				cerr << " j=" << j << endl;
				cerr << "  f=" << f << endl;
			  }
			  a[j][i] = 0;
			  aMaxErr[j][i] += aMaxErr[i][i]*abs(f);
			  for (unsigned int k = i+1; k < m; k++)
		 	  {
				a[j][k] -= a[i][k]*f;
				aMaxErr[j][k] += aMaxErr[i][k]*abs(f);
			  }
			  b[j] -= b[i]*f;
			  bMaxErr[j] += bMaxErr[i]*abs(f);
			  }
		}

		if (MR2_DEBUG)
		{
			for (j = 0; j < n; j++)
			{
			  for (unsigned int k = 0; k < m; k++)
			  {
				fprintf(stderr, "%7.3f ", a[j][k]);
			  }
			  fprintf(stderr, "| %7.3f\n", b[j]);
		    }
		    cerr << endl;
		    cerr << "Maximal errors:" << endl;
		    for (j = 0; j < n; j++)
		    {
			  for (unsigned int k = 0; k < m; k++)
			  {
			    fprintf(stderr, "%7.3f ", aMaxErr[j][k]);
			  }
			  fprintf(stderr, "| %7.3f\n", bMaxErr[j]);
		    }
		    cerr
			<< "-----------------------------"
			<< "-----------------------------"
			<< endl;
		}
	}
}

static void GaussTransform(const unsigned int n,
                           const unsigned int m,
                           mpq_class** a,
                           mpq_class* b) {
    if (MR2_DEBUG) {
        cerr << "GaussTransform() called, n="
             << n << " m=" << m << endl;

        cerr << "============================="
             << endl;
    }


/*
For each row...

*/
    for (unsigned int i = 0; i < n-1; i++) {
/*
Check for non-zero element in column $i$ below and including row $i$, which
can be used as pivot.

*/
        if (MR2_DEBUG) cerr << "i=" << i << endl;
        unsigned int j;
        for (j = i; j < n && cmp(a[j][i], 0) == 0; j++)
            if (MR2_DEBUG)
                cerr << " failed as pivot: a["
                     << j
                     << "]["
                     << i
                     << "]"
                     << endl;

/*
If we found a pivot, apply it to the rows below row $i$. If we did not find
a pivot, column $i$ is already zero below and including row $i$ and we do
not have to do anything.

*/
        if (j != n) {
            if (MR2_DEBUG)
                cerr << " pivot: a["
                     << j
                     << "]["
                     << i
                     << "]"
                     << endl;

/*
The pivot is in row $j$. If $j\ne i$, the pivot is in another row than row $i$.
Swap rows $i$ and $j$ in this case.

*/
            if (j != i) {
                mpq_class dummy = b[i];
                b[i] = b[j];
                b[j] = dummy;
                for (unsigned int k = 0; k < m; k++) {
                    dummy = a[i][k];
                    a[i][k] = a[j][k];
                    a[j][k] = dummy;
                }
            }

/*
Subtract row $i$ from each row below row $i$, multiplied with factor $f$,
which is calculated so that it sets the element in column $i$ of each row
to $0$ during the substraction.

*/
            for (j = i+1; j < n; j++) {
                mpq_class f = a[j][i]/a[i][i];

                a[j][i] = 0;
                for (unsigned int k = i+1; k < m; k++)
                    a[j][k] -= a[i][k]*f;
                b[j] -= b[i]*f;
            }
        }

    }
}



/*
1.1.1 Type conversions for the external format of GMP types

1.1.1.1 Function ~gmpTypeToTextType()~

Reads from inValue and stores its representation as TextType in resultList.

*/
static void gmpTypeToTextType
	(const mpq_class& inValue, ListExpr& resultList) {

	if (MR2_DEBUG)
	{
		cerr << "gmpTypeToTextType called with gmp input "
				<< inValue << "." << endl;
	}

	int n = 50;
	stringstream theStream;
	theStream << inValue;
	resultList = nl->TextAtom();

	while (theStream.good())
	{
		int i = 0;
		char* theArray = new char[n];
		while (theStream.good() && i < n-1)
		{
			char c;
			theStream.get(c);
			if (theStream.good())
			{
				theArray[i] = c;
				i++;
			}
		}
		for (int j = i; j < n; j++)
		{
			theArray[j] = 0;
		}
		if (MR2_DEBUG)
		{
			cerr << "Content of array is: (";
			for (int j = 0; j < n-1; j++)
			{
				cerr << theArray[j] << ",";
			}
			cerr << theArray[n-1] << ")." << endl;
		}

		string str = string(theArray);
		if (MR2_DEBUG)
			cerr << "gmpTypeToTextType(), next string is "
			<< str << endl;
		nl->AppendText(resultList, str);
	}

	if (MR2_DEBUG)
		cerr << "The resulting TextAtom is "
		<< nl->ToString(resultList) << endl;
}

/*
1.1.1.1 Function ~textTypeToGmpType()~

Reads from inList and stores its representation as mpq\_class in outValue.

*/
static void textTypeToGmpType
	(const ListExpr& inList, mpq_class& outValue) {

	if (MR2_DEBUG)
	{

		cerr << "textTypeToGmpType() called with text input "
				<< nl->ToString(inList) << endl;
	}

	TextScan theScan = nl->CreateTextScan(inList);
	stringstream theStream;
	if (MR2_DEBUG)
		cerr << "Reading character for character from TextAtom: ";

	char lastChar = '*'; //just a random initialization...

	for (unsigned int i = 0; i < nl->TextLength(inList); i++)
	{
	  string str = "";
	  nl->GetText(theScan, 1, str);
	  if (MR2_DEBUG)
		cerr << str[0] << " ";

	  //Checking for valid character
	  if ((int)str[0] < 47 || (int)str[0] > 57
		|| (i == 0 && (int)str[0] == 48
				&& nl->TextLength(inList) > 1)
		|| (lastChar == '/' && (int)str[0] == 48))
	  {
		stringstream message;
		message << "Precise coordinate not valid: "
		  << nl->ToString(inList)
		  << endl << "Only characters 1, 2, 3, 4, 5, "
			"6, 7, 8, 9, 0 and / allowed." << endl
		  << "0 mustn't be leading "
			"character when "
			"more than one character "
			"in total are given"
		  << endl << ", and 0 is "
			"not allowed directly after /";
		throw invalid_argument(message.str());

	  }

	  theStream.put(str[0]);
	  lastChar = str[0];
	}
	if (MR2_DEBUG)
		cerr << endl;

	theStream >> outValue;

	outValue.canonicalize();

	if (MR2_DEBUG)
		cerr << "The resulting gmp value is "
		<< outValue << " after canonicalization."
		<< endl;

	//Checking the value - must be between 0 and 1
	if (outValue >= 1 || outValue < 0)
	{
	  stringstream message;
	  message << "Precise coordinate not valid: "
		<< nl->ToString(inList)
		<< endl
		<< "Resulting value is not between 0 and 1, "
		"where 0 is allowed, but 1 is not.";
	  throw invalid_argument(message.str());
	}

}



/*
1.1 Geometrical helper types

1.1.1 Class ~GridPoint~

1.1.1.1 Constructors

*/
GridPoint::GridPoint(int x, int y) :
		x (x),
		y (y),
		isBasic (false),
		xErr (1),
		yErr (1) {}

GridPoint::GridPoint(int x, int y, bool isBasic) :
		x (x),
		y (y),
		isBasic (isBasic),
		xErr (isBasic ? 0 : 1),
		yErr (isBasic ? 0 : 1) {}

/*
1.1.1.1 Transformation functions

*/
ProvisionalPoint GridPoint::transformToProvisional() {
	ProvisionalPoint result((double)x,
			(double)y, (isBasic? 0.0 : 1.0),
			(isBasic? 0.0 : 1.0));
	return result;
}

/*
1.1.1.1 Operator ==

*/
bool GridPoint::operator==(const GridPoint& other) const {
	return (this->x == other.x && this->y ==
			other.y && this->isBasic == other.isBasic);
}

/*
1.1.1 Class ~ProvisionalPoint~

1.1.1.1 Constructor

*/
ProvisionalPoint::ProvisionalPoint(double x,
		double y, double xErr, double yErr) :
		x (x),
		y (y),
		xErr (xErr),
		yErr (yErr) {}

/*
1.1.1.1 Operator ==

*/
bool ProvisionalPoint::operator==
		(const ProvisionalPoint& other) const {
	return (this->x == other.x && this->y == other.y);
}

/*
1.1.1 Class ~PrecisePoint~

1.1.1.1 Constructors

*/
PrecisePoint::PrecisePoint(mpq_class x, mpq_class y) :
	x (x),
	y (y) {
}

PrecisePoint::PrecisePoint(mpq_class x1, int x2,
		mpq_class y1, int y2) :
	x (x1 + x2),
	y (y1 + y2) {
}

/*
1.1.1.1 Operator ==

*/
bool PrecisePoint::operator==(const PrecisePoint& other) const {
	return (cmp(this->x, other.x) == 0
			&& cmp(this->y, other.y) == 0);
}


/*
1.1.1 Class ~GridPointSegment~

1.1.1.1 Constructors

*/
GridPointSegment::GridPointSegment(GridPoint point1,
		GridPoint point2) :
		point1 (point1),
		point2 (point2),
		isBasic (point1.isBasic && point2.isBasic) {}

GridPointSegment::GridPointSegment(GridPoint point1,
		GridPoint point2, bool isBasic) :
		point1 (point1),
		point2 (point2),
		isBasic (isBasic) {}

/*
1.1.1.1 Access methods

*/
BasicBBox2D GridPointSegment::getBasicBbox2D() {
	int minX;
	int maxX;
	int minY;
	int maxY;
	minX = point1.x < point2.x ? point1.x : point2.x;
	minY = point1.y < point2.y ? point1.y : point2.y;

	if (isBasic)
	{
		maxX = point1.x < point2.x ? point2.x : point1.x;
		maxY = point1.y < point2.y ? point2.y : point1.y;
	}
	else
	{
		maxX = point1.x < point2.x ?
				point2.x + 1 : point1.x + 1;
		maxY = point1.y < point2.y ?
				point2.y + 1 : point1.y + 1;
	}

	BasicBBox2D result(minX, maxX+1, minY, maxY+1);
	return result;
}

/*
1.1.1.1 Operator ==

*/
bool GridPointSegment::operator==
		(const GridPointSegment& other) const {
	return ((this->point1 == other.point1
			&& this->point2 == other.point2) ||
			(this->point2 == other.point1
					&& this->point1 == other.point2));
}


/*
1.1.1.1 Transformation functions

*/
ProvisionalSegment GridPointSegment::transformToProvisional() {
	ProvisionalSegment result(point1.transformToProvisional(),
			point2.transformToProvisional());
	return result;
}

/*
1.1.1 Class ~ProvisionalSegment~

1.1.1.1 Constructor

*/
ProvisionalSegment::ProvisionalSegment(ProvisionalPoint point1,
		ProvisionalPoint point2) :
		point1 (point1),
		point2 (point2) {}

/*
1.1.1.1 Operator ==

*/
bool ProvisionalSegment::operator==
		(const ProvisionalSegment& other) const {
	return ((this->point1 == other.point1
			&& this->point2 == other.point2) ||
			(this->point2 == other.point1
					&& this->point1 == other.point2));
}



/*
1.1.1 Class ~PreciseSegment~

1.1.1.1 Constructor

*/
PreciseSegment::PreciseSegment(PrecisePoint point1,
		PrecisePoint point2) :
		point1 (point1),
		point2 (point2) {}

/*
1.1.1.1 Operator ==

*/
bool PreciseSegment::operator==(const PreciseSegment& other) const {
	return ((this->point1 == other.point1
			&& this->point2 == other.point2) ||
			(this->point2 == other.point1
					&& this->point1 == other.point2));
}



/*
1.1.1 Class ~GridPointTrapezium~

1.1.1.1 Constructors

*/
GridPointTrapezium::GridPointTrapezium
(GridPointSegment segment1, GridPointSegment segment2) :
		segment1 (segment1),
		segment2 (segment2),
		isBasic (segment1.isBasic
				&& segment2.isBasic) {}

GridPointTrapezium::GridPointTrapezium
(GridPointSegment segment1, GridPointSegment segment2, bool isBasic) :
		segment1 (segment1),
		segment2 (segment2),
		isBasic (isBasic) {}

/*
1.1.1.1 Access methods

*/
BasicBBox2D GridPointTrapezium::getBasicBbox2D() {
	int minX;
	int maxX;
	int minY;
	int maxY;
	minmax4(segment1.point1.x, segment1.point2.x,
			segment2.point1.x, segment2.point2.x, minX, maxX);
	minmax4(segment1.point1.y, segment1.point2.y,
			segment2.point1.y, segment2.point2.y, minY, maxY);
	if (isBasic)
	{
		return BasicBBox2D(minX, maxX, minY, maxY);
	}
	else
	{
		return BasicBBox2D(minX, maxX+1, minY, maxY+1);
	}
}

GridPointSegment GridPointTrapezium::getConnectingSegment1() {
	GridPointSegment result(segment1.point1, segment2.point1,
			(segment1.isBasic && segment2.isBasic));
	return result;
}

GridPointSegment GridPointTrapezium::getConnectingSegment2() {
	GridPointSegment result(segment1.point2, segment2.point2,
			(segment1.isBasic && segment2.isBasic));
	return result;
}

/*
1.1.1.1 Operator ==

*/
bool GridPointTrapezium::operator==(const GridPointTrapezium& other) const {
	return ((this->segment1 == other.segment1
			&& this->segment2 == other.segment2) ||
			(this->segment2 == other.segment1
					&& this->segment1 == other.segment2));
}



/*
1.1.1 Class ~ProvisionalTrapezium~

1.1.1.1 Constructor

*/
ProvisionalTrapezium::ProvisionalTrapezium(ProvisionalSegment segment1,
		ProvisionalSegment segment2) :
		segment1 (segment1),
		segment2 (segment2) {}

/*
1.1.1.1 Access methods

*/
ProvisionalBBox2D ProvisionalTrapezium::getProvisionalBbox2D() {
	double minX;
	double maxX;
	double minY;
	double maxY;
	minmax8(segment1.point1.x - segment1.point1.xErr,
			segment1.point1.x + segment1.point1.xErr,
			segment1.point2.x - segment1.point2.xErr,
			segment1.point2.x + segment1.point2.xErr,
			segment2.point1.x - segment2.point1.xErr,
			segment2.point1.x + segment2.point1.xErr,
			segment2.point2.x - segment2.point2.xErr,
			segment2.point2.x + segment2.point2.xErr,
			minX, maxX);
	minmax8(segment1.point1.y - segment1.point1.yErr,
			segment1.point1.y + segment1.point1.yErr,
			segment1.point2.y - segment1.point2.yErr,
			segment1.point2.y + segment1.point2.yErr,
			segment2.point1.y - segment2.point1.yErr,
			segment2.point1.y + segment2.point1.yErr,
			segment2.point2.y - segment2.point2.yErr,
			segment2.point2.y + segment2.point2.yErr,
			minY, maxY);
	ProvisionalBBox2D result(minX, maxX, minY, maxY);
	return result;
}

ProvisionalSegment ProvisionalTrapezium::getConnectingSegment1() {
	ProvisionalSegment result(segment1.point1, segment2.point1);
	return result;
}

ProvisionalSegment ProvisionalTrapezium::getConnectingSegment2() {
	ProvisionalSegment result(segment2.point1, segment2.point2);
	return result;
}

/*
1.1.1.1 Operator ==

*/
bool ProvisionalTrapezium::operator==
		(const ProvisionalTrapezium& other) const {
	return ((this->segment1 == other.segment1
		&& this->segment2 == other.segment2) ||
		(this->segment2 == other.segment1
			&& this->segment1 == other.segment2));
}



/*
1.1.1 Class ~PreciseTrapezium~

1.1.1.1 Constructor

*/
PreciseTrapezium::PreciseTrapezium(PreciseSegment segment1,
		PreciseSegment segment2) :
		segment1 (segment1),
		segment2 (segment2) {}

/*
1.1.1.1 Access methods

*/
PreciseBBox2D PreciseTrapezium::getPreciseBbox2D() {
	mpq_class minX;
	mpq_class maxX;
	mpq_class minY;
	mpq_class maxY;
	minmax4(segment1.point1.x, segment1.point2.x,
			segment2.point1.x, segment2.point2.x,
			minX, maxX);
	minmax4(segment1.point1.y, segment1.point2.y,
			segment2.point1.y, segment2.point2.y,
			minY, maxY);
	PreciseBBox2D result(minX, maxX, minY, maxY);
	return result;
}

PreciseSegment PreciseTrapezium::getConnectingSegment1() {
	PreciseSegment result(segment1.point1, segment2.point1);
	return result;
}

PreciseSegment PreciseTrapezium::getConnectingSegment2() {
	PreciseSegment result(segment2.point1, segment2.point2);
	return result;
}

/*
1.1.1.1 Operator ==

*/
bool PreciseTrapezium::operator==
		(const PreciseTrapezium& other) const {
	return ((this->segment1 == other.segment1
		&& this->segment2 == other.segment2) ||
		(this->segment2 == other.segment1
			&& this->segment1 == other.segment2));
}


/*
1.1.1 Class ~BasicBBox2D~

1.1.1.1 Constructor

*/
BasicBBox2D::BasicBBox2D(int minX, int minY,
		int maxX, int maxY) :
		minX (minX),
		maxX (maxX),
		minY (minY),
		maxY (maxY) {}

/*
1.1.1.1 Function ~overlapsWith()~

Checks if the bounding boxes of two geometric elements overlap.
The bboxes take already care of the possible error of the elements, so that
if true is returned, the bboxes of the precise representation may overlap.
This function is provided for basic and provisional bboxes and one for a
combination of both types.

*/
bool BasicBBox2D::overlapsWith (BasicBBox2D& other) {
	if (minX > other.maxX ||
			other.minX > maxX ||
			minY > other.maxY ||
			other.minY > maxY)
	{
		return false;
	}
	else
	{
		return true;
	}
}

/*
1.1.1 Class ~ProvisionalBBox2D~

1.1.1.1 Constructor

*/
ProvisionalBBox2D::ProvisionalBBox2D(double minX, double minY,
		double maxX, double maxY) :
		minX (minX),
		maxX (maxX),
		minY (minY),
		maxY (maxY) {}

/*
1.1.1.1 Function ~overlapsWith()~

Checks if the bounding boxes of two geometric elements overlap.
The bboxes take already care of the possible error of the elements, so that
if true is returned, the bboxes of the precise representation may overlap.
This function is provided for basic and provisional bboxes and one for a
combination of both types.

*/
bool ProvisionalBBox2D::overlapsWith (ProvisionalBBox2D& other) {
	if (minX > other.maxX ||
			other.minX > maxX ||
			minY > other.maxY ||
			other.minY > maxY)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool ProvisionalBBox2D::overlapsWith (BasicBBox2D& other) {
	if (minX > other.maxX ||
			other.minX > maxX ||
			minY > other.maxY ||
			other.minY > maxY)
	{
		return false;
	}
	else
	{
		return true;
	}
}

/*
1.1.1 Class ~PreciseBBox2D~

1.1.1.1 Constructor

*/
PreciseBBox2D::PreciseBBox2D(mpq_class minX, mpq_class minY,
		mpq_class maxX, mpq_class maxY) :
		minX (minX),
		maxX (maxX),
		minY (minY),
		maxY (maxY) {}

/*
1.1.1.1 Function ~overlapsWith()~

*/
bool PreciseBBox2D::overlapsWith (PreciseBBox2D& other) {
	if (cmp(minX, other.maxX) > 0 ||
			cmp(other.minX, maxX) > 0 ||
			cmp(minY, other.maxY) > 0 ||
			cmp(other.minY, maxY) > 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}



/*
1.1 Geometrical helper functions

1.1.1 Basic geometrical functions

1.1.1.1 Function ~touching()~

This function is provided for the different geometric helper types like
GridPointSegment, PreciseSegment, PreciseTrapezium, and so on. It returns true
if the two parameters touch (in the case of precise parameters) or may touch
(in the case of integer grid parameters).

*/
static bool touching (GridPointSegment segment1,
		GridPointSegment segment2) {
	if (segment1.point1 == segment2.point1
			|| segment1.point1 == segment2.point2 ||
			segment1.point2 == segment2.point1
			|| segment1.point2 == segment2.point2)
	{
		return true;
	}
	else
	{
		return false;
	}
}

static bool touching (PreciseSegment segment1,
		PreciseSegment segment2) {
	if (segment1.point1 == segment2.point1
			|| segment1.point1 == segment2.point2 ||
			segment1.point2 == segment2.point1
			|| segment1.point2 == segment2.point2)
	{
		return true;
	}
	else
	{
		return false;
	}
}

static bool touching (GridPointTrapezium trapezium1,
		GridPointTrapezium trapezium2) {
	if (trapezium1.segment1 == trapezium2.segment1
			|| trapezium1.segment2 == trapezium2.segment2
			|| trapezium1.segment1 == trapezium2.segment2
			|| trapezium1.segment2 == trapezium2.segment1
			|| trapezium1.getConnectingSegment1() ==
					trapezium2.getConnectingSegment1()
			|| trapezium1.getConnectingSegment2() ==
					trapezium2.getConnectingSegment2()
			|| trapezium1.getConnectingSegment1() ==
					trapezium2.getConnectingSegment2()
			|| trapezium1.getConnectingSegment2() ==
					trapezium2.getConnectingSegment1())
	{
		return true;
	}

	return false;
}

static bool touching (PreciseTrapezium trapezium1,
		PreciseTrapezium trapezium2) {
	if (trapezium1.segment1 == trapezium2.segment1
		|| trapezium1.segment2 == trapezium2.segment2
		|| trapezium1.segment1 == trapezium2.segment2
		|| trapezium1.segment2 == trapezium2.segment1
		|| trapezium1.getConnectingSegment1() ==
			trapezium2.getConnectingSegment1()
		|| trapezium1.getConnectingSegment2() ==
			trapezium2.getConnectingSegment2()
		|| trapezium1.getConnectingSegment1() ==
			trapezium2.getConnectingSegment2()
		|| trapezium1.getConnectingSegment2() ==
			trapezium2.getConnectingSegment1())
	{
		return true;
	}

	return false;
}




/*
1.1.1 Intersections between segments or between a segment and a line

1.1.1.1 Function ~segmentsMayIntersect()~ for integer
\label{smi}

Returns ~true~ if the two specified segments may intersect on integer grid
in three-dimensional space $(x, y, t)$.
It is not considered as intersection if they touch in their end points.
If the integer coordinates of the segments end points are the exact coordinates,
this will be used to come to an early conclusion.
The segments both have start point at time $t=0$ and end point at time $t=dt$.

*/
static bool segmentsMayIntersect(int dt,
			 GridPointSegment& segment1,
			 GridPointSegment& segment2,
			 double& zMin, double& zMax) {
    if (MR2_DEBUG)
    {
        cerr << "segmentsMayIntersect() for integer called"
             << endl;
        cerr << "segmentsMayIntersect() segment 1: ("
             << segment1.point1.x
             << ", "
             << segment1.point1.y
             << ", 0.0)-("
             << segment1.point2.x
             << ", "
             << segment1.point2.y
             << ", "
             << dt
             << "), isBasic = "
             << segment1.isBasic
             << endl;
        cerr << "segmentsMayIntersect() segment 2: ("
             << segment2.point1.x
             << ", "
             << segment2.point1.y
             << ", 0.0)-("
             << segment2.point2.x
             << ", "
             << segment2.point2.y
             << ", "
             << dt
             << "), isBasic = "
             << segment2.isBasic
             << endl;
    }

/*
Check if both segments are identical or touch in their endpoints on integer grid.

*/
    if (segment1 == segment2)
    {
    	if (MR2_DEBUG)
    	{
			cerr << "segmentsMayIntersect1(): "
			"Segments identical on integer grid!"
			<< endl;
    	}
    	if (segment1.isBasic && segment2.isBasic)
		{
			return false;
		}
    	zMin = -1;
    	zMax = dt + 1;
    	return true;
    }
    else if (segment1.point1 == segment2.point1)
    {
    	if (MR2_DEBUG)
    	{
			cerr << "segmentsMayIntersect(): "
			 << "segments touch in ("
			 << segment1.point1.x << ", "
			 << segment1.point1.y << ") "
			 << "on integer grid!"
			 << endl;
    	}
    	if (segment1.isBasic && segment2.isBasic)
    	{
    		return false;
    	}
    	zMin = -1;
    	zMax = 1;
    	return true;

    }
    else if (segment1.point2 == segment2.point2)
    {
    	if (MR2_DEBUG)
    	{
			cerr << "specialSegmentIntersects(): "
			 << "segments touch in ("
			 << segment1.point2.x << ", "
			 << segment2.point2.y << ") "
			 << "on integer grid!"
			 << endl;
    	}
    	if (segment1.isBasic && segment2.isBasic)
		{
			return false;
		}
    	zMin = dt -1;
    	zMax = dt + 1;
    	return true;
    }


/*
The line through segment (seg) 1 is
$(seg1.p1.x, seg1.p1.y, 0) + (seg1.p2.x-seg1.p1.x,
seg1.p2.y-seg1.p1.y, dt)\cdot s$.\\
The line through segment 2 is
$(seg2.p1.x, seg2.p1.y, 0) + (seg2.p2.x-seg2.p1.x,
seg2.p2.y-seg2.p1.y, dt)\cdot s$.

The intersection of the two lines is the solution of the following
linear system of equations:
\begin{eqnarray}
\begin{array}{ccc}
\begin{array}{cc}
seg1.p2.x-seg1.p1.x & seg2.p1.x-seg2.p2.x \\
seg1.p2.y-seg1.p1.y & seg2.p1.y-seg2.p2.y \\
dt          & -dt
\end{array} & \left| \begin{array}{c}
seg2.p1.x-seg1.p1.x \\
seg2.p1.y-seg1.p1.y \\
0
\end{array} \right.
\end{array}
\nonumber\end{eqnarray}

We put the left handed sides of the equations in matrix $a$ and the right
handed sides into array $b$ and are applying the Gaussian elimination to these.
The maximal errors of each component are also passed in $aMaxErr$ and $bMaxErr$,
respectively:

*/
    double A[3][2] =
        {{ segment1.point2.x-segment1.point1.x,
        		segment2.point1.x-segment2.point2.x },
         { segment1.point2.y-segment1.point1.y,
        		 segment2.point1.y-segment2.point2.y },
         { dt, -dt}};
    double B[3] =
        { segment2.point1.x-segment1.point1.x,
        		segment2.point1.y-segment1.point1.y, 0 };

    double* Ap[] = { A[0], A[1], A[2] };

    double AMaxErr[3][2] = {{ 2, 2 }, { 2, 2 }, { 1, 1 }};
    double BMaxErr[3] = { 2, 2, 1 };

    double* ApMaxErr[] = { AMaxErr[0], AMaxErr[1], AMaxErr[2] };

    GaussTransform(3, 2, Ap, B, ApMaxErr, BMaxErr);

/*
Now, we will determine the solution $t$ from the transformed system.
We examine each row from the bottom to the top:

*/
    for (int i = 2; i >= 0; i--)
    {
        if (!(Ap[i][0] == 0.0))
		{
//We reached the first line, nothing more to do here
        	break;
		}

/*
Row is in format $0 c \left| b \right.$.

*/
        if (Ap[i][1] == 0.0) {
/*
Row is in format $0 0 \left| b \right.$. Check if there is a contradiction
indicating that there are no solutions.
Keep in mind the errors of each component!

*/
            if (abs(B[i]) <= BMaxErr[i])
            {
                //We cannot be sure that B is really also zero,
            	//but we cannot be sure it is a contradiction either!
            	continue;
            }
            else
            {
              if (MR2_DEBUG)
               cerr << "segmentsMayIntersect1() "
               << "do not intersect #1"
               << " - found contradiction "
            	"in system of equations!"
               << endl;

              return false;
            }
        } else
        {
		  //Calculate the z coordinate of the intersection point
		  //and check if it is in the interval [0, dt]
		  //Calculate also maximal error for z
		  double z = dt*B[i]/Ap[i][1];
		  double zMaxError = 2*BMaxErr[i]*ApMaxErr[i][1];
		  zMin = z - zMaxError;
		  zMax = z + zMaxError;

/*
Row is in format $0 c \left| b \right.$ with $c\ne 0$. All rows below are
entirely 0.
Inserting $t=b/c$ into the equation of line 2 yields the value $z=dt\cdot b/c$
for the third component. If $0\le z \le dt$, the two segments intersect. If
$z=0$ or $z=dt$, they touch at one of their end points, which is not considered
as intersection.
Again we need to keep in mind the maximal errors! We only return false if we
are sure that there is no intersection!

*/
            if (MR2_DEBUG)
             cerr << "segmentsMayIntersect() z="
             << z << "maxError="
             << zMaxError
             << endl;

            if (lowerOrMaybeEqual(0, z, 1, zMaxError)
            		&& lowerOrMaybeEqual(z, dt, zMaxError, 1))
            {
                if (MR2_DEBUG)
                 cerr << "segmentsMayIntersect() "
                 << "intersect"
                 << endl;

                if (maybeEqual(z, 0, zMaxError, 1)
                		|| maybeEqual(z, dt, zMaxError, 1))
                {
                  if (MR2_DEBUG)
                  cerr << "segmentsMayIntersect() "
                  << "may only touch, not intersect. "
                  << "Recheck necessary!"
                  << endl;
                }

                return true;
            } else {
               if (MR2_DEBUG)
                cerr << "segmentsMayIntersect() "
                << "do not intersect #2"
                << endl;

               return false;
            }
        }
    }

/*
Both segments are located on the same line. We have to check whether they
are overlapping. As they are on the same line, it is sufficient to check if
one of the coordinates is overlapping...

*/

    if (segment1.isBasic && segment2.isBasic
    		&& touching(segment1, segment2))
	{
		//Touching is not intersecting!
		return false;
	}
    else if (betweenOrMaybeEqual(segment1.point1.x,
    		segment2.point1.x, segment1.point2.x, 1, 1, 1)
            || betweenOrMaybeEqual(segment1.point1.x,
            		segment2.point2.x, segment1.point2.x, 1, 1, 1)
            || betweenOrMaybeEqual(segment2.point1.x,
            		segment1.point1.x, segment2.point2.x, 1, 1, 1)
            || betweenOrMaybeEqual(segment2.point1.x,
            		segment1.point2.x, segment2.point2.x, 1, 1, 1))
    {
    	//Segments may intersect,
    	//a precise recheck is necessary
    	return true;
    }
    else
    {
    	//Segments are quite far from each other,
    	//surely no intersection
    	return false;
    }
}



/*
1.1.1.1 Function ~segmentsMayIntersect()~ for double
\label{smi2}

Returns ~true~ if the two specified segments may intersect
in three-dimensional space $(x, y, t)$.
It is not considered as intersection if they touch in their end points.
The segments both have start point at time $t=0$ and end point at time $t=dt$.

The values zMin and zMax return the minimum and maximum value for the
z coordinate of the intersection point in case of an intersection. The value
of zMin and zMax is always a relative value, since no real instants are passed to
that function but just the time difference dt.

*/
static bool segmentsMayIntersect(int dt,
		 ProvisionalSegment& segment1,
		 ProvisionalSegment& segment2,
		 double& zMin, double& zMax) {
    if (MR2_DEBUG)
    {
        cerr << "segmentsMayIntersect1() for double "
        	 << "(with zMin and zMax) called"
             << endl;
        cerr << "segmentsMayIntersect1() segment 1: ("
             << segment1.point1.x
             << ", "
             << segment1.point1.y
             << ", 0.0)-("
             << segment1.point2.x
             << ", "
             << segment1.point2.y
             << ", "
             << dt
             << endl;
        cerr << "segmentsMayIntersect1() segment 2: ("
             << segment2.point1.x
             << ", "
             << segment2.point1.y
             << ", 0.0)-("
             << segment2.point2.x
             << ", "
             << segment2.point2.y
             << ", "
             << dt
             << endl;
        cerr << "maximum errors for segment 1: ("
             << segment1.point1.xErr
             << ", "
             << segment1.point1.yErr
             << ", "
             << segment1.point2.xErr
             << ", "
             << segment1.point2.yErr
             << endl;
	   cerr << "maximum errors for segment 2: ("
             << segment2.point1.xErr
             << ", "
             << segment2.point1.yErr
             << ", "
             << segment2.point2.xErr
             << ", "
             << segment2.point2.yErr
             << endl;
    }

    //Initialize zMin and zMax with random values
    zMin = -1000;
    zMax = 1000;

/*
Check if both segments are identical or touch in their endpoints -
then we need a precise recalculation for sure!
Take also care about the maximum errors here!

*/
    if (segment1 == segment2)
    {
    	if (MR2_DEBUG)
    	 cerr << "segmentsMayIntersect(): Segments identical "
         "on double precision, they may intersect in reality!"
		 << endl;
    	zMin = 0 - 1;
    	zMax = dt + 1;
    	return true;
    }
    else if (segment1.point1 == segment2.point1)
    {
    	if (MR2_DEBUG)
    	 cerr << "segmentsMayIntersect(): "
		 << "segments touch in ("
		 << segment1.point1.x << ", "
		 << segment1.point1.y << ") "
		 << "on double precision, they may intersect in reality!"
		 << endl;
    	zMin = 0 - 1;
    	zMax = 0 + 1;
    	return true;
    }
    else if (segment1.point2 == segment2.point2)
    {
    	if (MR2_DEBUG)
		  cerr << "segmentsMayIntersect(): "
		  << "segments touch in ("
		  << segment1.point2.x << ", " << segment2.point2.y << ") "
		  << "on double precision, they may intersect in reality!"
		  << endl;
    	zMin = dt - 1;
    	zMax = dt + 1;
    	return true;
    }


/*
The line through segment 1 is
$(segment1.point1.x, segment1.point1.y, 0)+(segment1.point2.x-segment1.point1.x,
segment1.point2.y-segment1.point1.y, dt)\cdot s$.\\
The line through segment 2 is
$(segment2.point1.x, segment2.point1.y, 0)+(segment2.point2.x-segment2.point1.x,
segment2.point2.y-segment2.point1.y, dt)\cdot s$.

The intersection of the two lines is the solution of the following
linear system of equations:
\begin{eqnarray}
\begin{array}{ccc}
\begin{array}{cc}
seg1.p2.x-seg1.p1.x & seg2.p1.x-seg2.p2.x \\
seg1.p2.y-seg1.p1.y & seg2.p1.y-seg2.p2.y \\
dt          & -dt
\end{array} & \left| \begin{array}{c}
seg2.p1.x-seg1.p1.x \\
seg2.p1.y-seg1.p1.y \\
0
\end{array} \right.
\end{array}
\nonumber\end{eqnarray}

We put the left handed sides of the equations in matrix $a$ and the right
handed sides into array $b$ and are applying the Gaussian elimination to these.
The maximal errors of each component are also passed in $aMaxErr$ and $bMaxErr$,
respectively:

*/
    double A[3][2] =
        {{ segment1.point2.x-segment1.point1.x,
        		segment2.point1.x-segment2.point2.x },
         { segment1.point2.y-segment1.point1.y,
        		 segment2.point1.y-segment2.point2.y },
         { dt, -dt}};
    double B[3] =
        { segment2.point1.x-segment1.point1.x,
        		segment2.point1.y-segment1.point1.y, 0 };

    double* Ap[] = { A[0], A[1], A[2] };

/*
We also need to pass the maximum errors for each component: If values are subtracted,
their respective errors must be added.

*/

    double AMaxErr[3][2] =
    	{{ segment1.point2.xErr + segment1.point1.xErr,
    		segment2.point1.xErr + segment2.point2.xErr },
    		{ segment1.point2.yErr + segment1.point1.yErr,
    			segment2.point1.yErr + segment2.point2.yErr },
    		{ 1, 1 }};
    double BMaxErr[3] = { segment2.point1.xErr + segment1.point1.xErr,
    		segment2.point1.yErr + segment1.point1.yErr, 1 };

    double* ApMaxErr[] = { AMaxErr[0], AMaxErr[1], AMaxErr[2] };

    GaussTransform(3, 2, Ap, B, ApMaxErr, BMaxErr);

/*
Now, we will determine the solution $t$ from the transformed system.
We examine each row from the bottom to the top:

*/
    for (int i = 2; i >= 0; i--)
    {
        if (!(Ap[i][0] == 0.0))
		{
//We reached the first line, nothing more to do here
        	break;
		}

/*
Row is in format $0 c \left| b \right.$.

*/
        if (Ap[i][1] == 0.0) {
/*
Row is in format $0 0 \left| b \right.$. Check if there is a contradiction
indicating that there are no solutions.
Keep in mind the errors of each component!

*/
            if (abs(B[i]) <= BMaxErr[i])
            {
                //We cannot be sure that it is a contradiction!
            	//B[i] is too close to zero!
            	continue;
            }
            else
            {
                if (MR2_DEBUG)
                cerr << "segmentsMayIntersect1() "
                << "do not intersect #1"
                << " - found contradiction in system of equations!"
                << endl;

                return false;
            }
        } else
        {
            //Calculate the z coordinate of the intersection point
        	//and check if it is in the interval [0, dt]
        	//Calculate also maximal error for z
        	double z = dt*B[i]/Ap[i][1];
        	double zMaxError = 2*BMaxErr[i]*ApMaxErr[i][1];
        	zMin = z - zMaxError;
        	zMax = z + zMaxError;

/*
Row is in format $0 c \left| b \right.$ with $c\ne 0$. All rows below are
entirely 0.
Inserting $t=b/c$ into the equation of line 2 yields the value $z=dt\cdot b/c$
for the third component. If $0\le z \le dt$, the two segments intersect. If
$z=0$ or $z=dt$, they touch at one of their end points, which is not considered
as intersection.
Again we need to keep in mind the maximal errors! We only return false if we
are sure that there is no intersection!

*/
            if (MR2_DEBUG)
                cerr << "segmentsMayIntersect() z="
                     << z << "maxError="
                     << zMaxError << endl;

            if (lowerOrMaybeEqual(0, z, 1, zMaxError)
            	&& lowerOrMaybeEqual(z, dt, zMaxError, 1))
            {
                if (MR2_DEBUG)
                cerr << "segmentsMayIntersect() "
                 << "may intersect"
                 << endl;

                if (maybeEqual(z, 0, zMaxError, 1)
                	|| maybeEqual(z, dt, zMaxError, 1))
                {
                	if (MR2_DEBUG)
                    cerr << "segmentsMayIntersect() "
                    << "may only touch, not intersect. "
                    << "Recheck necessary!"
                    << endl;
                }

                return true;
            } else {
                if (MR2_DEBUG)
                 cerr << "segmentsMayIntersect() "
                 << "do not intersect"
                 << endl;

                return false;
            }
        }
    }

/*
Both segments are located on the same line. We have to check whether they
are overlapping. As they are on the same line, it is sufficient to check if
one of the coordinates is overlapping...

*/

    if (betweenOrMaybeEqual
    	(segment1.point1.x, segment2.point1.x,
    	 segment1.point2.x, segment1.point1.xErr,
    	 segment2.point1.xErr, segment1.point2.xErr)
        || betweenOrMaybeEqual(segment1.point1.x,
        	segment2.point2.x, segment1.point2.x,
            segment1.point1.xErr, segment2.point2.xErr,
            segment1.point2.xErr)
        || betweenOrMaybeEqual(segment2.point1.x,
        	segment1.point1.x, segment2.point2.x,
            segment2.point1.xErr, segment1.point1.xErr,
            segment2.point2.xErr)
        || betweenOrMaybeEqual(segment2.point1.x,
        	segment1.point2.x, segment2.point2.x,
            segment2.point1.xErr, segment1.point2.xErr,
            segment2.point2.xErr))
    {
    	//Segments may intersect, a precise recheck is necessary
    	return true;
    }
    else
    {
    	//Segments are quite far from each other, surely no intersection
    	return false;
    }
}

static bool segmentsMayIntersect(int dt,
		GridPointSegment& segment1,
		GridPointSegment& segment2) {
	if (MR2_DEBUG)
		cerr << "segemntsMayIntersect "
			"without z for integer called" << endl;

	double dummy1;
	double dummy2;

	return segmentsMayIntersect
		(dt, segment1, segment2, dummy1, dummy2);
}

/*
1.1.1.1 Function ~preciseSegmentsIntersect()~ for precise coordinates
\label{psi}

Returns ~true~ if the two specified segments intersect
in three-dimensional space $(x, y, t)$.
It is not considered as intersection if they touch in their end points.
The segments both have start point at time $t=0$ and end point at time $t=dt$.

*/
static bool preciseSegmentsIntersect(mpq_class dt,
			 PreciseSegment& segment1,
			 PreciseSegment& segment2,
			 mpq_class& z) {

	if (MR2_DEBUG)
	cerr << "preciseSegmentsIntersect() called"
	<< endl;

/*
Check if both segments are identical or touch in their endpoints -
then we need a precise recalculation for sure!
Take also care about the maximum errors here!

*/
	if (segment1 == segment2)
	{
		if (MR2_DEBUG)
         cerr << "preciseSegmentsIntersect(): Segments identical!"
           << endl;
		return false;
	}
	else if (segment1.point1 == segment2.point1)
	{
		if (MR2_DEBUG)
           cerr << "preciseSegmentsIntersect(): "
        		<< "segments touch at z=0."
                << endl;

		return false;
	}
	else if (segment1.point2 == segment2.point2)
	{
		if (MR2_DEBUG)
           cerr << "preciseSegmentsIntersect(): "
        		<< "segments touch at z=dt."
                << endl;

		return false;
	}


/*
The line through segment 1 is
$(segment1.point1.x, segment1.point1.y, 0)+(segment1.point2.x-segment1.point1.x,
segment1.point2.y-segment1.point1.y, dt)\cdot s$.\\
The line through segment 2 is
$(segment2.point1.x, segment2.point1.y, 0)+(segment2.point2.x-segment2.point1.x,
segment2.point2.y-segment2.point1.y, dt)\cdot s$.

The intersection of the two lines is the solution of the following
linear system of equations:
\begin{eqnarray}
\begin{array}{ccc}
\begin{array}{cc}
seg1.p2.x-seg1.p1.x & seg2.p1.x-seg2.p2.x \\
seg1.p2.y-seg1.p1.y & seg2.p1.y-seg2.p2.y \\
dt          & -dt
\end{array} & \left| \begin{array}{c}
seg2.p1.x-seg1.p1.x \\
seg2.p1.y-seg1.p1.y \\
0
\end{array} \right.
\end{array}
\nonumber\end{eqnarray}

We put the left handed sides of the equations in matrix $a$ and the right
handed sides into array $b$ and are applying the Gaussian elimination to these.
The maximal errors of each component are also passed in $aMaxErr$ and $bMaxErr$,
respectively:

*/
	mpq_class A[3][2] =
		{{ segment1.point2.x-segment1.point1.x,
			segment2.point1.x-segment2.point2.x },
		 { segment1.point2.y-segment1.point1.y,
				segment2.point1.y-segment2.point2.y },
		 { dt, -dt}};
	mpq_class B[3] =
		{ segment2.point1.x-segment1.point1.x,
			segment2.point1.y-segment1.point1.y, 0 };

	mpq_class* Ap[] = { A[0], A[1], A[2] };

	GaussTransform(3, 2, Ap, B);

/*
Now, we will determine the solution $t$ from the transformed system.
We examine each row from the bottom to the top:

*/
	for (int i = 2; i >= 0; i--)
	{
		if (cmp(Ap[i][0], 0) != 0)
		{
			break;
		}

/*
Row is in format $0 c \left| b \right.$.

*/
		if (cmp(Ap[i][1], 0) == 0) {
/*
Row is in format $0 0 \left| b \right.$. Check if there is a contradiction
indicating that there are no solutions.
Keep in mind the errors of each component!

*/
			if (cmp(B[i], 0) == 0)
			{

				continue;
			}
			else
			{
				if (MR2_DEBUG)
                cerr << "preciseSegmentsIntersect() "
                << "do not intersect"
                << " - found contradiction in system of equations!"
                << endl;
				return false;
			}
		} else
		{
			//Calculate the z coordinate of
			//the intersection point and check
			//if it is in the interval [0, dt]
			z = dt*B[i]/Ap[i][1];


/*
Row is in format $0 c \left| b \right.$ with $c\ne 0$. All rows below are
entirely 0.
Inserting $t=b/c$ into the equation of line 2 yields the value $z=dt\cdot b/c$
for the third component. If $0\le z \le dt$, the two segments intersect. If
$z=0$ or $z=dt$, they touch at one of their end points, which is not considered
as intersection.
Again we need to keep in mind the maximal errors! We only return false if we
are sure that there is no intersection!

*/

			if (cmp(0, z) < 0 && cmp(z, dt) < 0)
			{
				if (MR2_DEBUG)
                cerr << "preciseSegmentsIntersect() "
                << "intersect"
                << endl;

				return true;

			} else {
				if (MR2_DEBUG)
                cerr << "preciseSegmentsIntersect() "
                << "do not intersect (they may touch, but not intersect)"
                << endl;

				return false;
			}
		}
	}

/*
Both segments are located on the same line. We have to check whether they
are overlapping. As they are on the same line, it is sufficient to check if
one of the coordinates is overlapping...

*/

	if ((segment1.point1.x < segment2.point1.x
			&& segment2.point1.x < segment1.point2.x)
			|| (segment1.point1.x < segment2.point2.x
				&& segment2.point2.x < segment1.point2.x)
			|| (segment2.point1.x < segment1.point1.x
				&& segment1.point1.x < segment2.point2.x)
			|| (segment2.point1.x < segment1.point2.x
				&& segment1.point2.x < segment2.point2.x))
	{

		return true;
	}
	else
	{

		return false;
	}
}

static bool preciseSegmentsIntersect(mpq_class dt,
		PreciseSegment& segment1,
		PreciseSegment& segment2) {
	if (MR2_DEBUG)
		cerr << "preciseSegmentsIntersect without z called"
		<< endl;

	mpq_class dummy;

	return preciseSegmentsIntersect(dt, segment1, segment2, dummy);
}



/*
1.1.1.1 Function ~segmentAndLineMayIntersect()~

Returns ~true~ if the specified segment and line may intersect
in three-dimensional space $(x, y, t)$.

The segment is parallel to the $(x, y)$-plane.

The line is $P+Q\cdot t$. The line must not be parallel to the $(x, y)-plane$.

*/
static bool segmentAndLineMayIntersect(int z,
                                       GridPointSegment& segment,
                                       double* P,
                                       double* Q,
                                       double* PErr,
                                       double* QErr) {
    if (MR2_DEBUG) {
        cerr << "segmentAndLineMayIntersect() called"
             << endl;
        cerr << "segmentAndLineMayIntersect() segment: ("
             << segment.point1.x
             << ", "
             << segment.point1.y
             << ", "
             << z
             << ")-("
             << segment.point2.x
             << ", "
             << segment.point2.y
             << ", "
             << z
             << ")"
             << endl;
        cerr << "segmentAndLineMayIntersect() line: ("
             << P[0]
             << ", "
             << P[1]
             << ", "
             << P[2]
             << ")-("
             << Q[0]
             << ", "
             << Q[1]
             << ", "
             << Q[2]
             << ")*t"
             << endl;
        cerr << "segmentAndLineMayIntersect() maximum errors of segment: "
             << "1 in each component, as this segment is on integer grid."
             << endl;
        cerr << "segmentAndLineMayIntersect() maximum errors "
	     << "of line: P errors: ("
             << PErr[0]
             << ", "
             << PErr[1]
             << ", "
             << PErr[2]
             << "), Q errors: ("
             << QErr[0]
             << ", "
             << QErr[1]
             << ", "
             << QErr[2]
             << "), where line = P - Q*t"
             << endl;
    }

    if (segment.point1 == segment.point2 && segment.isBasic) {
/*
The segment is actually a point.

*/
    if (MR2_DEBUG)
	cerr
	<< "segmentAndLineMayIntersect() "
	<< "no intersection #1 - segment is just a point!"
	<< endl;

	return false;
    }

/*
Calculate point $(x, y)$ where the line intersects the plane parallel to the
$(x, y)$-plane with distance z from the $(x, y)$-plane.

*/

    double t2 = (z-P[2])/Q[2];
    double t2Err = (1 + PErr[2]) * QErr[2];

    if (MR2_DEBUG)
        cerr << "segmentAndLineMayIntersect(), "
	<< "line intersects with plane parallel to (x,y) "
	<< "at distance z at "
        << endl << "t= " << t2 << ", tErr = " << t2Err
        << endl;

    double x = P[0]+Q[0]*t2;
    double y = P[1]+Q[1]*t2;
    double xErr = PErr[0] + QErr[0]*t2Err;
    double yErr = PErr[1] + QErr[1]*t2Err;

    if (MR2_DEBUG)
        cerr << "segmentAndLineMayIntersect() x="
             << x
             << " y="
             << y
             << " xErr=" << xErr << ", yErr=" << yErr
             << endl;

/*
Next we check if the bounding box of the segment overlaps with the bounding box
built of the points coordinates and their respective maximum errors.

*/

    BasicBBox2D segmentBbox = segment.getBasicBbox2D();

    if (MR2_DEBUG)
        cerr << "segmentAndLineMayIntersect() segments "
	<< "bounding box is: " << endl
        << "minX="
        << segmentBbox.minX
        << ", maxX="
        << segmentBbox.maxX
        << ", minY="
        << segmentBbox.minY
        << ", maxY="
        << segmentBbox.maxY
        << endl;

    ProvisionalBBox2D intersectionPointBbox
	(x - xErr, y - yErr, x + xErr, y + yErr);

    if (!intersectionPointBbox.overlapsWith(segmentBbox))
    {
    	if (MR2_DEBUG)
	    cerr << "segmentAndLineMayIntersect() "
	    << "do not intersect - bboxes do not overlap "
	    << "on plane parallel to (x,y) at distance z!" 
	    << endl;

    	return false;
    }

/*
Next we check for special cases. Is the segment vertical or horizontal in $(x, y)$?

*/

	if (segment.point1.x == segment.point2.x)
	{
/*
The segment is vertical in the $(x, y)$-plane.
We already know that the bounding boxes overlap.
We cannot make a more precise check because of the insecurity of the
precise point coordinates. Thus, line and segment may overlap.

*/
	    if (MR2_DEBUG)
		cerr << "segmentAndLineMayIntersect() "
		<< "may intersect - segment is vertical "
		<< "in (x,y) and bbox overlaps"
		<< " with the one of the intersection point!" 
		<< endl;

	    return true;
	}
	else if (segment.point1.y == segment.point2.y)
	{
/*
The segment is horizontal in the $(x, y)$-plane.
We already know that the bounding boxes overlap.
We cannot make a more precise check because of the insecurity of the
precise point coordinates. Thus, line and segment may overlap.

*/
	    if (MR2_DEBUG)
		cerr << "segmentAndLineMayIntersect() "
		<< "may intersect - segment is horizontal "
		<< "in (x,y) and bbox overlaps"
		<< " with the one of the intersection point!" 
		<< endl;

	    return true;
	}


/*
If the segment is neither vertical nor horizontal in $(x, y)$, we can make a finer
decision than just that based on the bounding box (the bbox is very imprecise here).

We now calculate the line built by the segment. We then calculate
the distance of the intersection point $(x, y)$ from the line both by its x- and
y-Komponent. If and only if these distances are smaller than the sum of the
respective errors, line and segment may intersect.

*/

	//Calculate the y-component of the point on the line at x
	double lineY = (x - segment.point1.x) / 
		(segment.point2.x - segment.point1.x) 
		* (segment.point2.y - segment.point1.y) + 
		segment.point1.y;

	if (MR2_DEBUG)
	cerr << "segmentAndLineMayIntersect() "
	<< "y on segment where x is same as line intersection point: "
	<< lineY << endl
	<< "Will compare with intersection point y: " << y
	<< " and its error: " << yErr << endl;

	//Calculate the x-component of the point on the line at y
	double lineX = (y - segment.point1.y) / 
		(segment.point2.y - segment.point1.y) 
		* (segment.point2.x - segment.point1.x) + 
		segment.point1.x;

	if (MR2_DEBUG)
        cerr << "segmentAndLineMayIntersect() "
	<< "x on segment where y is same as line intersection point: "
	<< lineX << endl
	<< "Will compare with intersection point x: " << x
	<< " and its error: " << xErr << endl;

	if (lineX + 1 < x - xErr || lineX - 1 > x + xErr )
	{
	    if (MR2_DEBUG)
	    cerr << "segmentAndLineMayIntersect() cannot intersect, "
	     << "intersection point "
	     << "too far from line!" 
	     << endl;

	    return false;
	}
	else
	{
	    if (MR2_DEBUG)
	    cerr << "segmentAndLineMayIntersect() may intersect. " 
		<< endl;

	    return true;
	}
}

static bool segmentsMayIntersect(int dt,
		ProvisionalSegment& segment1,
		ProvisionalSegment& segment2) {

    if (MR2_DEBUG)
	cerr << "segmentsMayIntersect without z "
	<< "for double called" 
	<< endl;

    double dummy1;
    double dummy2;

    return segmentsMayIntersect
	(dt, segment1, segment2, dummy1, dummy2);
}

/*
1.1.1.1 Function ~preciseSegmentAndLineIntersect()~

Returns ~true~ if the specified segment and line intersect
in three-dimensional space $(x, y, t)$.

The segment is parallel to the $(x, y)$-plane.

The line is $P+Q\cdot t$. The line must not be parallel to the $(x, y)$-plane.

*/
static bool preciseSegmentAndLineIntersect(mpq_class z,
                                       PreciseSegment& segment,
                                       mpq_class* P,
                                       mpq_class* Q) {
    if (MR2_DEBUG) {
        cerr << "preciseSegmentAndLineIntersect() called"
             << endl;

    }

    if (segment.point1 == segment.point2) {
/*
The segment is actually a point.

*/
    if (MR2_DEBUG)
        cerr << "preciseSegmentAndLineIntersect() "
	     << "no intersection - segment is just a point!" 
	     << endl;

	     return false;
    }

/*
Calculate point $(x, y)$ where the line intersects the plane parallel to the
$(x, y)$-plane with distance z from the $(x, y)$-plane.

*/

    mpq_class t2 = (z-P[2])/Q[2];

    if (MR2_DEBUG)
        cerr << "preciseSegmentAndLineIntersect(), "
	     << "line intersects with plane parallel to (x,y) "
	     << "at distance z"
             << endl;

    mpq_class x = P[0]+Q[0]*t2;
    mpq_class y = P[1]+Q[1]*t2;

    mpq_class l1MinX = 
	cmp(segment.point1.x, segment.point2.x) < 0 ? 
	segment.point1.x : segment.point2.x;
    mpq_class l1MaxX = 
	cmp(segment.point1.x, segment.point2.x) > 0 ? 
	segment.point1.x : segment.point2.x;
    mpq_class l1MinY = 
	cmp(segment.point1.y, segment.point2.y) < 0 ? 
	segment.point1.y : segment.point2.y;
    mpq_class l1MaxY = 
	cmp(segment.point1.y, segment.point2.y) > 0 ? 
	segment.point1.y : segment.point2.y;

/*
Next we check for special cases. Is the segment vertical or horizontal in $(x, y)$?

*/

    if (cmp(segment.point1.x , segment.point2.x) == 0)
    {
/*
The segment is vertical in the $(x, y)$-plane.

*/
	if (cmp(x, segment.point1.x) == 0 
		&& cmp(y, l1MaxY) < 0 && cmp(l1MinY, y) < 0)
	{
	    if (MR2_DEBUG)
		cerr << "preciseSegmentAndLineIntersect() "
		     << "intersect - segment is vertical "
		     << "in (x,y)!" << endl;

	    return true;
	}
	else
	{
	    if (MR2_DEBUG)
		cerr << "preciseSegmentAndLineIntersect() "
		<< "no intersection." 
		<< endl;

	    return false;
	}
    }
    else if (cmp(segment.point1.y, segment.point2.y) == 0)
    {
/*
The segment is horizontal in the $(x, y)$-plane.

*/
	if (cmp(y, segment.point1.y) == 0 
		&& cmp(x, l1MaxX) < 0 && cmp(l1MinX, x) < 0)
	{
	    if (MR2_DEBUG)
		cerr << "preciseSegmentAndLineIntersect() "
		<< "intersect - segment is horizontal "
		<< "in (x,y)!" << endl;

	    return true;
	}
	else
	{
	    if (MR2_DEBUG)
		cerr << "preciseSegmentAndLineIntersect() "
		<< "no intersection." << endl;

	    return false;
	}
    }


/*
If the segment is neither vertical nor horizontal in $(x, y)$, we can use the equations below
without running into problems dividing through zero.

We now calculate the line built by the segment.
First, check whether $(x, y)$ is on the line through the segment.

*/

    mpq_class t1 = 
	(x-segment.point1.x)/(segment.point2.x-segment.point1.x);

    if (cmp(t1, (y - segment.point1.y)/
	(segment.point2.y - segment.point1.y)) != 0) 
    {
	if (MR2_DEBUG)
	    cerr << "preciseSegmentAndLineIntersect() "
		 << "no intersection" << endl;

	return false;
    }

/*
Verify that $(x, y)$ is actually on the segment: As we already know that
it is on the line through the segment, we just have to check its bounding
box parallel to the $(x, y)$-plane.

*/
	if (cmp(x, l1MinX) < 0
		|| cmp(l1MaxX, x) < 0
		|| cmp(y, l1MinY) < 0
		|| cmp(l1MaxY, y) < 0) {
		if (MR2_DEBUG)
		    cerr << "preciseSegmentAndLineIntersect() "
			 << "no intersection" << endl;

		return false;
	}

	if (MR2_DEBUG)
		cerr << "preciseSegmentAndLineIntersect() "
			 << "intersects" << endl;

	return true;
}







/*
1.1.1 Intersections between two trapeziums

1.1.1.1 Function ~trapeziumsMayIntersect()~
\label{stinu}

Returns ~true~ if the two specified trapeziums may intersect in three-dimensional
space $(x, y, t)$.

The two trapeziums must meet the following conditions. These conditions are
not checked in the function. If the conditions are not met, the function will
deliver incorrect results.

Both trapeziums are spanned by two segments which are parallel to the plane (x,y).
Of each trapezium one segment, called segment1, one is in distance z=0 while the
other segment, called segment2, is in distance z=dt.

For both trapeziums, it is allowed that either the segment between the two
points at instant 0 or the segment between the two points at instant dt reduces
to a single point, but never both.

The segment between the two points at instant 0 and the segment between the two
points at instant dt must be collinear, and this must be true for both trapeziums.

$dt$ must be greater than $0$.

The coordinates of all points spanning the trapeziums are given on the integer grid.
If the methods returns true, the calculation needs to be repeated with precise
coordinates to ensure that the trapeziums really intersect with each other.

*/
static bool trapeziumsMayIntersect(
	int dt,
	GridPointTrapezium trapezium1,
	GridPointTrapezium trapezium2) {

    if (MR2_DEBUG) {
	cerr << "trapeziumsMayIntersect() called" 
	<< endl;
	cerr << "Trapezium 1: ("
	<< trapezium1.segment1.point1.x << ", " 
	<< trapezium1.segment1.point1.y
	<< "), (" 
	<< trapezium1.segment1.point2.x << ", " 
	<< trapezium1.segment1.point2.y
	<< "), (" 
	<< trapezium1.segment2.point1.x << ", " 
	<< trapezium1.segment2.point1.y
	<< "), (" 
	<< trapezium1.segment2.point2.x << ", " 
	<< trapezium1.segment2.point2.y
	<< ")"
	<< endl;
	cerr << "Trapezium2: ("
	<< trapezium2.segment1.point1.x << ", " 
	<< trapezium2.segment1.point1.y
	<< "), (" 
	<< trapezium2.segment1.point2.x << ", " 
	<< trapezium2.segment1.point2.y
	<< "), (" 
	<< trapezium2.segment2.point1.x << ", " 
	<< trapezium2.segment2.point1.y
	<< "), (" 
	<< trapezium2.segment2.point2.x << ", " 
	<< trapezium2.segment2.point2.y
	<< ")"
	<< endl;
    }

/*
First, lets check the bounding boxes in the $(x, y)$-plane of the two
trapeziums. If they don't overlap, we are finished.

*/

	BasicBBox2D bboxT1 = trapezium1.getBasicBbox2D();
	BasicBBox2D bboxT2 = trapezium2.getBasicBbox2D();

	bool doOverlap = bboxT1.overlapsWith(bboxT2);

	if (!doOverlap)
	{
	    if (MR2_DEBUG) {
		cerr << "trapeziumsMayIntersect() "
		<< "bboxes do not overlap" 
		<< endl;
	    }

	    return false;
	}

	if (MR2_DEBUG) {
	    cerr << "trapeziumsMayIntersect() "
	    << "bboxes overlap" << endl;
	}

/*
Now, lets see if the trapeziums are equal or touch in one segment.
For basic trapeziums, that means that they do not intersect, but for
non-basic trapeziums, that means that they are sufficiently close to
each other, so that we need a precise recalculation!

*/

	if (trapezium1 == trapezium2 || 
		touching(trapezium1, trapezium2))
	{
		if (trapezium1.isBasic && trapezium2.isBasic)
		{
		    if (MR2_DEBUG)
			cerr << "trapeziumsMayIntersect(): "
			<< "Trapeziums are basic and equal "
			<< "or only touching!"
			<< endl;

		    return false;
		}
		else
		{
		    if (MR2_DEBUG)
			cerr << "trapeziumsMayIntersect() equal "
			<< "or touching on integer grid. "
			<< "Recheck with precise coordinates!"
			<< endl;

		    return true;
		}
	}

/*
The bounding boxes of the trapeziums overlap but they do not touch in one
segment and are not equal.
To determine if they really intersect, we first calculate the
intersection of the two planes spanned by the trapezium.

Create equations for the two planes spanned by the trapeziums,
considering that one edge of
each trapezium may be a single point: Plane 1 is
$T1B+T1A\cdot (s, t)$ and Plane 2 is $T2B+T1A\cdot (s', t')$.

*/

	double T1A[3][2]; //left side of equations for trapezium1
	double T1B[3]; //right side of equations for trapezium1
	double T2A[3][2]; //left side of equations for trapezium2
	double T2B[3]; //right side of equations for trapezium2
	double T1aErr[3][2]; //maximal errors, left side
	double T2aErr[3][2]; //maximal errors, left side
	double T1bErr[3]; //maximal errors, right side
	double T2bErr[3]; //maximal errors, right side

	if (trapezium1.segment1.point1 == 
		trapezium1.segment1.point2) {

		//Trapezium1 is a triangle, the first segment 
		//is just a point - we use the second segment instead
		T1A[0][0] = trapezium1.segment2.point2.x 
			-trapezium1.segment2.point1.x;
		T1aErr[0][0] = trapezium1.segment2.point2.xErr 
			+ trapezium1.segment2.point1.xErr;
		T1A[1][0] = trapezium1.segment2.point2.y
			- trapezium1.segment2.point1.y;
		T1aErr[1][0] = trapezium1.segment2.point2.yErr 
			+ trapezium1.segment2.point1.yErr;
		T1A[2][0] = 0;
		T1aErr[2][0] = 1;

		T1A[0][1] = trapezium1.segment1.point1.x
			-trapezium1.segment2.point1.x;
		T1aErr[0][1] = trapezium1.segment1.point1.xErr 
			+ trapezium1.segment2.point1.xErr;
		T1A[1][1] = trapezium1.segment1.point1.y
			-trapezium1.segment2.point1.y;
		T1aErr[1][1] = trapezium1.segment1.point1.yErr 
			+ trapezium1.segment2.point1.yErr;
		T1A[2][1] = -dt;
		T1aErr[2][1] = 1;

		T1B[0] = trapezium1.segment2.point1.x;
		T1bErr[0] = trapezium1.segment2.point1.xErr;
		T1B[1] = trapezium1.segment2.point1.y;
		T1bErr[1] = trapezium1.segment2.point1.yErr;
		T1B[2] = dt;
		T1bErr[2] = 1;

	} else {

		//The first segment is no point, we can use it
		T1A[0][0] = trapezium1.segment1.point2.x
			-trapezium1.segment1.point1.x;
		T1A[1][0] = trapezium1.segment1.point2.y
			-trapezium1.segment1.point1.y;
		T1aErr[0][0] = trapezium1.segment1.point2.xErr 
			+ trapezium1.segment1.point1.xErr;
		T1aErr[1][0] = trapezium1.segment1.point2.yErr 
			+ trapezium1.segment1.point1.yErr;
		T1A[2][0] = 0;
		T1aErr[2][0] = 1;

		T1A[0][1] = trapezium1.segment2.point1.x
			-trapezium1.segment1.point1.x;
		T1A[1][1] = trapezium1.segment2.point1.y
			-trapezium1.segment1.point1.y;
		T1aErr[0][0] = trapezium1.segment2.point1.xErr 
			+ trapezium1.segment1.point1.xErr;
		T1aErr[1][1] = trapezium1.segment2.point1.yErr 
			+ trapezium1.segment1.point1.yErr;
		T1A[2][1] = dt;
		T1aErr[2][1] = 1;

		T1B[0] = trapezium1.segment1.point1.x;
		T1B[1] = trapezium1.segment1.point1.y;
		T1bErr[0] = trapezium1.segment1.point1.xErr;
		T1bErr[1] = trapezium1.segment1.point1.yErr;
		T1B[2] = 0;
		T1bErr[2] = 1;
	}

	if (trapezium2.segment1.point1 == 
		trapezium2.segment1.point2) {

		//Trapezium2 is a triangle, the first segment 
		//is just a point - we use segment 2 instead
		T2A[0][0] = trapezium2.segment2.point2.x
			-trapezium2.segment2.point1.x;
		T2A[1][0] = trapezium2.segment2.point2.y
			-trapezium2.segment2.point1.y;
		T2aErr[0][0] = trapezium2.segment2.point2.xErr 
			+ trapezium2.segment2.point1.xErr;
		T2aErr[1][0] = trapezium2.segment2.point2.yErr 
			+ trapezium2.segment2.point1.yErr;
		T2A[2][0] = 0;
		T2aErr[2][0] = 1;

		T2A[0][1] = trapezium2.segment1.point1.x
			-trapezium2.segment2.point1.x;
		T2A[1][1] = trapezium2.segment1.point1.y
			-trapezium2.segment2.point1.y;
		T2aErr[0][1] = trapezium2.segment1.point1.xErr 
			+ trapezium2.segment2.point1.xErr;
		T2aErr[1][1] = trapezium2.segment1.point1.yErr 
			+ trapezium2.segment2.point1.yErr;
		T2A[2][1] = -dt;
		T2aErr[2][1] = 1;

		T2B[0] = trapezium2.segment2.point1.x;
		T2B[1] = trapezium2.segment2.point1.y;
		T2bErr[0] = trapezium2.segment2.point1.xErr;
		T2bErr[1] = trapezium2.segment2.point1.yErr;
		T2B[2] = dt;
		T2bErr[2] = 1;

	} else {

		//The first segment is no point, so we can use it
		T2A[0][0] = trapezium2.segment1.point2.x
			-trapezium2.segment1.point1.x;
		T2A[1][0] = trapezium2.segment1.point2.y
			-trapezium2.segment1.point1.y;
		T2aErr[0][0] = trapezium2.segment1.point2.xErr 
			+ trapezium2.segment1.point1.xErr;
		T2aErr[1][0] = trapezium2.segment1.point2.yErr 
			+ trapezium2.segment1.point1.yErr;
		T2A[2][0] = 0;
		T2aErr[2][0] = 1;

		T2A[0][1] = trapezium2.segment2.point1.x
			-trapezium2.segment1.point1.x;
		T2A[1][1] = trapezium2.segment2.point1.y
			-trapezium2.segment1.point1.y;
		T2aErr[0][1] = trapezium2.segment2.point1.xErr 
			+ trapezium2.segment1.point1.xErr;
		T2aErr[1][1] = trapezium2.segment2.point1.yErr 
			+ trapezium2.segment1.point1.yErr;
		T2A[2][1] = dt;
		T2aErr[2][1] = 1;

		T2B[0] = trapezium2.segment1.point1.x;
		T2B[1] = trapezium2.segment1.point1.y;
		T2bErr[0] = trapezium2.segment1.point1.xErr;
		T2bErr[1] = trapezium2.segment1.point1.yErr;
		T2B[2] = 0;
		T2bErr[2] = 1;
	}


	if (MR2_DEBUG) {
		cerr << "T1B      T1A                "
			 << "T2B      T2A"
			 << endl;
		cerr << "-------  ---------------    "
			 << "-------  ---------------"
			 << endl;
		for (unsigned int i = 0; i < 3; i++)
			fprintf(stderr,
					"%7.3f  %7.3f %7.3f"    
					"%7.3f  %7.3f %7.3f\n",
					T1B[i],
					T1A[i][0],
					T1A[i][1],
					T2B[i],
					T2A[i][0],
					T2A[i][1]);
		cerr << "T1bErr      T1aErr                "
			 << "T2bErr      T2aErr"
			 << endl;
		cerr << "-------  ---------------    "
			 << "-------  ---------------"
			 << endl;
		for (unsigned int i = 0; i < 3; i++)
			fprintf(stderr,
					"%7.3f  %7.3f %7.3f"    
					"%7.3f  %7.3f %7.3f\n",
					T1bErr[i],
					T1aErr[i][0],
					T1aErr[i][1],
					T2bErr[i],
					T2aErr[i][0],
					T2aErr[i][1]);
	}

/*
Create a linear system of equations for the intersection of the two calculated
planes. The system of equations is created in $A$ and $B$, which represents
$T1B+T1A\cdot (s, t)=T2B+T2A\cdot (s', t')$. Apply Gaussian elimination to
$A$ and $B$.

*/
	double A[3][4]; //left side of equations
	double B[3]; //right side of equations
	double* Ap[3];
	double Aerr[3][4];
	double Berr[3];
	double* ApErr[3];

	for (unsigned int i = 0; i < 3; i++) {

		A[i][0] = T1A[i][0];
		Aerr[i][0] = T1aErr[i][0];

		A[i][1] = T1A[i][1];
		Aerr[i][1] = T1aErr[i][1];

		A[i][2] = -T2A[i][0];
		Aerr[i][2] = T2aErr[i][0];

		A[i][3] = -T2A[i][1];
		Aerr[i][3] = T2aErr[i][1];

		B[i] = T2B[i]-T1B[i];
		Berr[i] = T2bErr[i] + T1bErr[i];

		Ap[i] = A[i];
		ApErr[i] = Aerr[i];

	}

	GaussTransform(3, 4, Ap, B, ApErr, Berr);

/*
Now, the linear system of equation has the following format
\begin{eqnarray}
\begin{array}{ccc}
\begin{array}{cccc}
\ast & \ast & \ast & \ast \\
0    & \ast & \ast & \ast \\
0    & 0    & c1   & c2
\end{array} & \left| \begin{array}{c}
\ast \\
\ast \\
b
\end{array} \right.
\end{array}
\nonumber\end{eqnarray}
The asterisk denotes arbitrary values.

If $c1=0$, $c2=0$ and $b\ne0$, the two planes are parallel and not identical:
They do not intersect. If $c1=0$, $c2=0$ and $b=0$, the two planes are
identical and we have to check if the two trapeziums overlap.

*/

	if (Ap[2][2] == 0 && Ap[2][3] == 0)
	{
		//The two planes are parallel. 
		//Check if it is the same plane:
		if (B[2] == 0)
		{
		    if (MR2_DEBUG)
			cerr << "trapeziumsMayIntersect() "
			<< "identical plane"
			<< endl;

/*
The two planes are identical.
The trapeziums overlap if one of the segments of each trapezium, which is not
parallel to the $(x, y)$-plane, intersects with another of such segments
of the other trapezium.

If - because of the imprecision on the integer grid - the two planes are not
really identical but just almost parallel, the function will return true,
because segmentsMayIntersect will return true for at least one of the below
calls. Therefor we can ignore this kind of insecurity.

*/

		    GridPointSegment trapez1connecting1 = 
			trapezium1.getConnectingSegment1();
		    GridPointSegment trapez1connecting2 = 
			trapezium1.getConnectingSegment2();
		    GridPointSegment trapez2connecting1 = 
			trapezium2.getConnectingSegment1();
		    GridPointSegment trapez2connecting2 = 
			trapezium2.getConnectingSegment2();

		    if (segmentsMayIntersect
			(dt, trapez1connecting1, trapez2connecting1)
			|| segmentsMayIntersect
			(dt, trapez1connecting1, trapez2connecting2)
			|| segmentsMayIntersect
			(dt, trapez1connecting2, trapez2connecting1)
			|| segmentsMayIntersect
			(dt, trapez1connecting2, trapez2connecting2))
		    {
			if (MR2_DEBUG) {
			    cerr << "trapeziumsMayIntersect() "
			 	 << "intersects"
				 << endl;
			    cerr << ::std::fixed
				 << ::std::setprecision(6);
			    for (unsigned int i = 0; i < 3; i++)
				cerr <<Ap[i][0]<<" "
				<<Ap[i][1]<<" "
				<<Ap[i][2]<<" "
				<<Ap[i][3]
				<<"|"
				<<B[i]
				<<endl;
			}

			return true;
		    } else {
			if (MR2_DEBUG)
			cerr << "trapeziumsMayIntersect() "
			<< "do not intersect"
			<< endl;

			return false;
		    }
		} else {
			//parallel, but not identical plane: 
			//No intersection possible if the
			//distance between the two parallel planes 
			//is larger than the insecurity
			//about the fact if they are really 
			//parallel or just almost...

			if (ApErr[2][2] + ApErr[2][3] < 
				B[2] - Berr[2])
			{
			    //There is a contradiction 
			    //in the system of equations, 
			    //and it is greater than 
			    //the error about it!
			    if (MR2_DEBUG)
				cerr << "trapeziumsMayIntersect()"
				<< " parallel" << endl;

			    return false;
			}
		}
	}


/*
Otherwise, the two planes intersect in a line, which we need to
calculate now. We have to consider the following cases: For $c1=0$, it
follows that $t'=b/c2$ with $s'$ arbitrary; for $c2=0$, it follows that
$s'=b/c1$ with $t'$ arbitrary; otherwise, it follows that
$s'=(b-c2\cdot t')/c1$ with $t'$ arbitrary.

Inserting $s'$ and $t'$ into the equation of the plane spanned by
trapezium 2 provides the equation of the intersection line, which we
will use in notation $P+Q\cdot u$.

*/

	double P[3];
	double Q[3];
	double Perr[3];
	double Qerr[3];

	if (maybeEqual(Ap[2][2], 0, ApErr[2][2], 0))
	{
/*
Case 1: $c1=0$.

*/

		double f = B[2]/Ap[2][3]; // = b/c2
		double fErr = Berr[2] * ApErr[2][3];

		if (MR2_DEBUG)
			cerr << "trapeziumsMayIntersect() c1=0 f="
				 << f << "fErr = " << fErr
				 << endl;

		for (unsigned int i = 0; i < 3; i++)
		{
			P[i] = T2B[i]+T2A[i][1]*f;
			Perr[i] = T2bErr[i] + T2aErr[i][i]*fErr;
			Q[i] = T2A[i][0];
			Qerr[i] = T2aErr[i][0];
		}
	} else if (maybeEqual(Ap[2][3], 0.0, ApErr[2][3], 0))
	{
/*
Case 2: $c2=0$.

*/

		double f = B[2]/Ap[2][2]; // = b/c1
		double fErr = Berr[2] * ApErr[2][2];

		if (MR2_DEBUG)
			cerr << "trapeziumsMayIntersect() c2=0 f="
				 << f << "fErr = " << fErr
				 << endl;

		for (unsigned int i = 0; i < 3; i++) {
			P[i] = T2B[i]+T2A[i][0]*f;
			Perr[i] = T2bErr[i] + T2aErr[i][0] * fErr;
			Q[i] = T2A[i][1];
			Qerr[i] = T2aErr[i][1];
		}
	} else
	{
/*
Case 3: $c1\ne 0$ and $c2\ne 0$.

Inserting $s'=(b-c2\cdot t')/c1$ into $T2B+T2A\cdot (s', t')$ yields
\begin{eqnarray}
\left(
\begin{array}{c}
T2B[0] \\
T2B[1] \\
T2B[2]
\end{array}
\right)+\left(
\begin{array}{c}
T2A[0][0] \\
T2A[1][0] \\
T2A[2][0]
\end{array}
\right)\cdot (b-c2\cdot t')/c1+\left(
\begin{array}{c}
T2A[0][1] \\
T2A[1][1] \\
T2A[2][1]
\end{array}
\right)\cdot t'
\nonumber\end{eqnarray}
and
\begin{eqnarray}
\left(
\begin{array}{c}
T2B[0]+T2A[0][0]\cdot b/c1 \\
T2B[1]+T2A[1][0]\cdot b/c1 \\
T2B[2]+T2A[2][0]\cdot b/c1
\end{array}
\right)+
\left(
\begin{array}{c}
 T2A[0][1]-T2A[0][0]\cdot c2/c1 \\
 T2A[1][1]-T2A[1][0]\cdot c2/c1 \\
 T2A[2][1]-T2A[2][0]\cdot c2/c1
\end{array}
\right)\cdot t'
\nonumber\end{eqnarray}

*/

		double f1 = B[2]/Ap[2][2];    // = b/c1
		double f2 = A[2][3]/Ap[2][2]; // = c2/c1
		double f1Err = Berr[2] * ApErr[2][2];
		double f2Err = Aerr[2][3] * ApErr[2][2];

		if (MR2_DEBUG)
			cerr << "trapeziumsMayIntersect() f1="
				 << f1 << "f1Err = " << f1Err
				 << " f2="
				 << f2 << "f2Err = " << f2Err
				 << endl;

		for (unsigned int i = 0; i < 3; i++) {
			P[i] = T2B[i]+T2A[i][0]*f1;
			Q[i] = T2A[i][1]-T2A[i][0]*f2;
			Perr[i] = T2bErr[i] + T2aErr[i][0] * f1Err;
			Qerr[i] = T2aErr[i][1] + T2aErr[i][0] * f2Err;
		}

		if (MR2_DEBUG)
			cerr << "trapeziumsMayIntersect(): " 
			     << "Intersection line is ("
				 << P[0]
				 << ", "
				 << P[1]
				 << ", "
				 << P[2]
				 << ")+("
				 << Q[0]
				 << ", "
				 << Q[1]
				 << ", "
				 << Q[2]
				 << ")*u"
				 << "Errors: ("
				 << Perr[0]
				 << ", "
				 << Perr[1]
				 << ", "
				 << Perr[2]
				 << "), ("
				 << Qerr[0]
				 << ", "
				 << Qerr[1]
				 << ", "
				 << Qerr[2]
				 << ")."
				 << endl;
	}

/*
Now, we have to check whether the intersection line may intersect
at least with one of the segments of each trapezium.

If $Q[2]=0$, the intersection line is parallel to the $(x, y)$-plane.

*/

	if (maybeEqual(Q[2], 0.0, Qerr[2], 0))
	{
	    if (MR2_DEBUG)
		cerr << "trapeziumsMayIntersect() intersection line "
		<< "parallel to (x, y)-plane"
		<< endl;

/*
Check whether the intersection line is within the z-Range covered by the two
trapeziums.
Here we need to be extremely careful, as the information that the line is
parallel to (x,y) in somewhat insecure due to the error in $Q[2]$. This
insecurity adds to the one of $P[2]$.

We take the length of the longer segment of the trapezium, and calculate how
much the z coordinate of the line can change on that length because of the maximal
possible error in $Q[2]$, and then we add this error to the one of $P[2]$.

*/
	    double segmentLength1 =
		sqrt(trapezium1.segment1.point1.x * 
		trapezium1.segment1.point1.x +
		trapezium1.segment1.point2.x * 
		trapezium1.segment1.point2.x);
	    double segmentLength2 =
		sqrt(trapezium1.segment2.point1.x * 
		trapezium1.segment2.point1.x +
		trapezium1.segment2.point2.x * 
		trapezium1.segment2.point2.x);
	    double segmentLength3 =
		sqrt(trapezium2.segment1.point1.x * 
		trapezium2.segment1.point1.x +
		trapezium2.segment1.point2.x * 
		trapezium2.segment1.point2.x);
	    double segmentLength4 =
		sqrt(trapezium2.segment2.point1.x * 
		trapezium2.segment2.point1.x +
		trapezium2.segment2.point2.x * 
		trapezium2.segment2.point2.x);
	    double maxLength = segmentLength1;
	    maxLength = segmentLength2 > maxLength ? 
		segmentLength2 : maxLength;
	    maxLength = segmentLength3 > maxLength ? 
		segmentLength3 : maxLength;
	    maxLength = segmentLength4 > maxLength ? 
		segmentLength4 : maxLength;

	    double maxError = Perr[2] + Qerr[2] * 
		maxLength / sqrt(Q[0]*Q[0] + Q[1]*Q[1] + 
		Q[2]*Q[2]);

		if (isLower(P[2], 0, maxError, 1)
			|| isLower(dt, P[2], 1, maxError))
		{

			if (MR2_DEBUG)
				cerr << "trapeziumsMayIntersect() "
					 << "no intersection"
					 << endl;

			return false;
		}

/*
Calculate the points where the trapeziums' segments intersect with the
intersection line.

*/

		double ip1x = trapezium1.segment1.point1.x + 
			(trapezium1.segment2.point1.x - 
			trapezium1.segment1.point1.x) * P[2]/dt;
		double ip1y = trapezium1.segment1.point1.y + 
			(trapezium1.segment2.point1.y - 
			trapezium1.segment1.point1.y) * P[2]/dt;

		double ip2x = trapezium1.segment1.point2.x + 
			(trapezium1.segment2.point2.x - 
			trapezium1.segment1.point2.x) * P[2]/dt;
		double ip2y = trapezium1.segment1.point2.y + 
			(trapezium1.segment2.point2.y - 
			trapezium1.segment1.point2.y) * P[2]/dt;

		double ip3x = trapezium2.segment1.point1.x + 
			(trapezium2.segment2.point1.x - 
			trapezium1.segment1.point1.x) * P[2]/dt;
		double ip3y = trapezium2.segment1.point1.y + 
			(trapezium2.segment2.point1.y - 
			trapezium1.segment1.point1.y) * P[2]/dt;

		double ip4x = trapezium2.segment1.point2.x + 
			(trapezium2.segment2.point2.x - 
			trapezium2.segment1.point2.x) * P[2]/dt;
		double ip4y = trapezium2.segment1.point2.y + 
			(trapezium2.segment2.point2.y - 
			trapezium2.segment1.point2.y) * P[2]/dt;

		double intersectionPointError = 1 + (2*Perr[2]); 
		//same for x and y and for all points

/*
Check for overlaps of the bounding boxes of the two intersection segments.
If they overlap, the sections may intersect.

*/

		double ip1ip2MinX = ip1x < ip2x ? ip1x : ip2x;
		double ip1ip2MaxX = ip1x > ip2x ? ip1x : ip2x;

		double ip1ip2MinY = ip1y < ip2y ? ip1y : ip2y;
		double ip1ip2MaxY = ip1y > ip2y ? ip1y : ip2y;

		double ip3ip4MinX = ip3x < ip4x ? ip3x : ip4x;
		double ip3ip4MaxX = ip3x > ip4x ? ip3x : ip4x;

		double ip3ip4MinY = ip3y < ip4y ? ip3y : ip4y;
		double ip3ip4MaxY = ip3y > ip4y ? ip3y : ip4y;

		if (isLower(ip1ip2MaxX, ip3ip4MinX, 
			intersectionPointError, intersectionPointError)
			|| isLower(ip3ip4MaxX, ip1ip2MinX, 
			intersectionPointError, intersectionPointError)
			|| isLower(ip1ip2MaxY, ip3ip4MinY, 
			intersectionPointError, intersectionPointError)
			|| isLower(ip3ip4MaxY, ip1ip2MinY, 
			intersectionPointError,intersectionPointError))
		{
		    if (MR2_DEBUG)
			cerr << "trapeziumsMayIntersect() "
			<< "no intersection"
			<< endl;

		    return false;
		} else
		{
		    if (MR2_DEBUG)
			cerr << "trapeziumsMayIntersect() "
			<< "trapeziums intersect"
			<< endl;

		    return true;
		}
    } else
    {
/*
Intersection line hits $(x, y)$-plane in $(P[0], P[1], 0)$ and the
plane parallel to $(x, y)$-plane in distance $dt$ in
$(P[0]+Q[0], P[1]+Q[1], dt)$ as we have handled the case $Q[2]=0$
separately.

*/

		double zMin;
		double zMax;

		double t1zMin = dt;
		double t1zMax = 0;
		bool t1Intersects = false;

		if (segmentAndLineMayIntersect
			(0, trapezium1.segment1, P, Q, Perr, Qerr)) {

			t1zMin = t1zMin > 0 ? 0 : t1zMin;
			t1zMax = t1zMax < 0 ? 0 : t1zMax;
			t1Intersects = true;
		}
		if (segmentAndLineMayIntersect
			(dt, trapezium1.segment2, P, Q, Perr, Qerr)) {

			t1zMin = t1zMin > dt ? dt : t1zMin;
			t1zMax = t1zMax < dt ? dt : t1zMax;
			t1Intersects = true;
		}

		ProvisionalSegment connectingSegment1 = 
			trapezium1.getConnectingSegment1().
				transformToProvisional();
		ProvisionalSegment connectingSegment2 = 
			trapezium1.getConnectingSegment2().
				transformToProvisional();
		ProvisionalPoint point1(P[0], P[1], Perr[0], Perr[1]);
		ProvisionalPoint point2(P[0]+Q[0], P[1]+Q[1], Perr[0]
			+Qerr[0], Perr[1]+Qerr[1]);
		ProvisionalSegment lineSegment(point1, point2);

		if (segmentsMayIntersect
			(dt, connectingSegment1, lineSegment, 
			zMin, zMax)) {

			t1zMin = t1zMin > zMin ? zMin : t1zMin;
			t1zMax = t1zMax < zMax ? zMax : t1zMax;
			t1Intersects = true;
		}
		if (segmentsMayIntersect
			(dt, connectingSegment2, lineSegment, 
			zMin, zMax)) {

			t1zMin = t1zMin > zMin ? zMin : t1zMin;
			t1zMax = t1zMax < zMax ? zMax : t1zMax;
			t1Intersects = true;
		}

		double t2zMin = dt;
		double t2zMax = 0;

		bool t2Intersects = false;

		if (segmentAndLineMayIntersect
			(0, trapezium2.segment1, P, Q, Perr, Qerr)) {

			t2zMin = t2zMin > 0 ? 0 : t2zMin;
			t2zMax = t2zMax < 0 ? 0 : t2zMax;
			t2Intersects = true;
		}
		if (segmentAndLineMayIntersect
			(dt, trapezium2.segment2, P, Q, Perr, Qerr)) {

			t2zMin = t2zMin > dt ? dt : t2zMin;
			t2zMax = t2zMax < dt ? dt : t2zMax;
			t2Intersects = true;
		}

		ProvisionalSegment connectingSegment3 = 
			trapezium2.getConnectingSegment1().
			transformToProvisional();
		ProvisionalSegment connectingSegment4 = 
			trapezium2.getConnectingSegment2().
			transformToProvisional();

		if (segmentsMayIntersect
			(dt, connectingSegment3, lineSegment, zMin, 
			zMax)) {

			t2zMin = t2zMin > zMin ? zMin : t2zMin;
			t2zMax = t2zMax < zMax ? zMax : t2zMax;
			t2Intersects = true;
		}
		if (segmentsMayIntersect
			(dt, connectingSegment4, lineSegment, zMin, 
			zMax)) {

			t2zMin = t2zMin > zMin ? zMin : t2zMin;
			t2zMax = t2zMax < zMax ? zMax : t2zMax;
			t2Intersects = true;
		}

		if (MR2_DEBUG) {
		    cerr << "trapeziumsMayIntersect() dt="
			<< dt
			<< endl;
		    cerr << "trapeziumsMayIntersect() t1Intersects="
			<< t1Intersects
			<< endl;
		    if (t1Intersects)
			cerr << "trapeziumsMayIntersect() t1zMin="
			<< t1zMin
			<< " t1zMax="
			<< t1zMax
			<< endl;
		    cerr << "trapeziumsMayIntersect() t2Intersects="
			<< t2Intersects
			<< endl;
		    if (t2Intersects)
			cerr << "trapeziumsMayIntersect() t2zMin="
			<< t2zMin
			<< " t2zMax="
			<< t2zMax
			<< endl;
		}

		if (t1Intersects
			&& t2Intersects
			&& !(t1zMax < t2zMin || t2zMax < t1zMin)
			&& !(t1zMax == 0.0 && t1zMin == 0.0 
			&& t2zMax == 0.0 && t2zMin == 0.0)
			&& !(t1zMax == dt && t1zMin == dt 
			&& t2zMax == dt && t2zMin == dt))
		{
		    if (MR2_DEBUG)
			cerr << "trapeziumsMayIntersect() intersect"
			<< endl;

		    return true;
		} else
		{
		    if (MR2_DEBUG)
			cerr << "trapeziumsMayIntersect() "
			     << "no intersection"
			     << endl;

		    return false;
		}
	}
}

/*
1.1.1.1 Function ~preciseTrapeziumsIntersect()~

Returns ~true~ if the specified trapeziums intersect
in three-dimensional space $(x, y, t)$.

*/
static bool preciseTrapeziumsIntersect(
		mpq_class dt,
		PreciseTrapezium trapezium1,
		PreciseTrapezium trapezium2) {

/*
	First, lets see if the trapeziums are equal or touch in one segment.

*/

	if (trapezium1 == trapezium2 || touching(trapezium1, trapezium2))
	{
		return false;
	}

/*
The segments do not touch in one segment and are not equal.
To determine if they really intersect, we first calculate the
intersection of the two planes spanned by the trapezium.

Create equations for the two planes spanned by the trapeziums,
considering that one edge of
each trapezium may be a single point: Plane 1 is
$T1B+T1A\cdot (s, t)$ and Plane 2 is $T2B+T1A\cdot (s', t')$.

*/

	mpq_class T1A[3][2]; //left side of equations for trapezium1
	mpq_class T1B[3]; //right side of equations for trapezium1

	if (trapezium1.segment1.point1 == trapezium1.segment1.point2) 
	{
		//Trapezium1 is a triangle, the first segment 
		//is just a point - we use the second segment instead
		T1A[0][0] = trapezium1.segment2.point2.x 
			- trapezium1.segment2.point1.x;
		T1A[1][0] = trapezium1.segment2.point2.y 
			- trapezium1.segment2.point1.y;
		T1A[2][0] = 0;

		T1A[0][1] = trapezium1.segment1.point1.x 
			- trapezium1.segment2.point1.x;
		T1A[1][1] = trapezium1.segment1.point1.y 
			- trapezium1.segment2.point1.y;
		T1A[2][1] = -dt;

		T1B[0] = trapezium1.segment2.point1.x;
		T1B[1] = trapezium1.segment2.point1.y;
		T1B[2] = dt;

	} else {

		//The first segment is no point, we can use it
		T1A[0][0] = trapezium1.segment1.point2.x 
			- trapezium1.segment1.point1.x;
		T1A[1][0] = trapezium1.segment1.point2.y 
			- trapezium1.segment1.point1.y;
		T1A[2][0] = 0;

		T1A[0][1] = trapezium1.segment2.point1.x 
			- trapezium1.segment1.point1.x;
		T1A[1][1] = trapezium1.segment2.point1.y 
			- trapezium1.segment1.point1.y;
		T1A[2][1] = dt;

		T1B[0] = trapezium1.segment1.point1.x;
		T1B[1] = trapezium1.segment1.point1.y;
		T1B[2] = 0;
	}

	mpq_class T2A[3][2]; //left side of equations for trapezium2
	mpq_class T2B[3]; //right side of equations for trapezium2

	if (trapezium2.segment1.point1 == trapezium2.segment1.point2) 
	{
		//Trapezium2 is a triangle, the first segment 
		//is just a point - we use segment 2 instead
		T2A[0][0] = trapezium2.segment2.point2.x 
			- trapezium2.segment2.point1.x;
		T2A[1][0] = trapezium2.segment2.point2.y 
			- trapezium2.segment2.point1.y;
		T2A[2][0] = 0;

		T2A[0][1] = trapezium2.segment1.point1.x 
			- trapezium2.segment2.point1.x;
		T2A[1][1] = trapezium2.segment1.point1.y 
			- trapezium2.segment2.point1.y;
		T2A[2][1] = -dt;

		T2B[0] = trapezium2.segment2.point1.x;
		T2B[1] = trapezium2.segment2.point1.y;
		T2B[2] = dt;
	} else {

		//The first segment is no point, so we can use it
		T2A[0][0] = trapezium2.segment1.point2.x 
			- trapezium2.segment1.point1.x;
		T2A[1][0] = trapezium2.segment1.point2.y 
			- trapezium2.segment1.point1.y;
		T2A[2][0] = 0;

		T2A[0][1] = trapezium2.segment2.point1.x 
			- trapezium2.segment1.point1.x;
		T2A[1][1] = trapezium2.segment2.point1.y 
			- trapezium2.segment1.point1.y;
		T2A[2][1] = dt;

		T2B[0] = trapezium2.segment1.point1.x;
		T2B[1] = trapezium2.segment1.point1.y;
		T2B[2] = 0;
	}

	if (MR2_DEBUG) {
	    cerr << "Creating system of equations."
		<< endl;
	}

/*
Create a linear system of equations for the intersection of the two calculated
planes. The system of equations is created in $A$ and $B$, which represents
$T1B+T1A\cdot (s, t)=T2B+T2A\cdot (s', t')$. Apply Gaussian elimination to
$A$ and $B$.

*/
	mpq_class A[3][4]; //left side of equations
	mpq_class B[3]; //right side of equations
	mpq_class* Ap[3];

	for (unsigned int i = 0; i < 3; i++) {

		A[i][0] = T1A[i][0];
		A[i][1] = T1A[i][1];
		A[i][2] = -T2A[i][0];
		A[i][3] = -T2A[i][1];

		B[i] = T2B[i]-T1B[i];

		Ap[i] = A[i];
	}

	GaussTransform(3, 4, Ap, B);

/*
Now, the linear system of equation has the following format
\begin{eqnarray}
\begin{array}{ccc}
\begin{array}{cccc}
\ast & \ast & \ast & \ast \\
0    & \ast & \ast & \ast \\
0    & 0    & c1   & c2
\end{array} & \left| \begin{array}{c}
\ast \\
\ast \\
b
\end{array} \right.
\end{array}
\nonumber\end{eqnarray}
The asterisk denotes arbitrary values.

If $c1=0$, $c2=0$ and $b\ne0$, the two planes are parallel and not identical:
They do not intersect. If $c1=0$, $c2=0$ and $b=0$, the two planes are
identical and we have to check if the two trapeziums overlap.

*/

	if (cmp(Ap[2][2], 0) == 0 && cmp(Ap[2][3], 0) == 0)
	{
		//The two planes are parallel. 
		//Check if it is the same plane:
		if (cmp(B[2], 0) == 0)
		{
		    if (MR2_DEBUG)
			cerr << "preciseTrapeziumsIntersect() "
			<< "identical plane"
			<< endl;

/*
The two planes are identical.
The trapeziums overlap if one of the segments of each trapezium, which is not
parallel to the $(x, y)$-plane, intersects with another of such segments
of the other trapezium.

If - because of the imprecision on the integer grid - the two planes are not
really identical but just almost parallel, the function will return true,
because segmentsMayIntersect will return true for at least one of the below
calls. Therefor we can ignore this kind of insecurity.

*/

		    PreciseSegment trapez1connecting1 = 
			trapezium1.getConnectingSegment1();
		    PreciseSegment trapez1connecting2 = 
			trapezium1.getConnectingSegment2();
		    PreciseSegment trapez2connecting1 = 
			trapezium2.getConnectingSegment1();
		    PreciseSegment trapez2connecting2 = 
			trapezium2.getConnectingSegment2();

		    if (preciseSegmentsIntersect(dt, 
			trapez1connecting1, trapez2connecting1)
				|| preciseSegmentsIntersect(dt, 
			trapez1connecting1, trapez2connecting2)
				|| preciseSegmentsIntersect(dt, 
			trapez1connecting2, trapez2connecting1)
				|| preciseSegmentsIntersect(dt, 
			trapez1connecting2, trapez2connecting2))
		    {
			if (MR2_DEBUG) {
				cerr << "preciseTrapeziumsIntersect()" 
				<< " intersect"
				<< endl;
			}

			return true;
		    } else {
			if (MR2_DEBUG)
				cerr << "preciseTrapeziumsIntersect() "
				<< "do not intersect"
				<< endl;

			return false;
		    }
		} else {
		    //parallel, but not identical plane: 
		    //No intersection possible

			if (MR2_DEBUG)
			    cerr << "preciseTrapeziumsIntersect()" 
				 << " parallel"
				 << endl;

			return false;
		}
	}


/*
Otherwise, the two planes intersect in a line, which we need to
calculate now. We have to consider the following cases: For $c1=0$, it
follows that $t'=b/c2$ with $s'$ arbitrary; for $c2=0$, it follows that
$s'=b/c1$ with $t'$ arbitrary; otherwise, it follows that
$s'=(b-c2\cdot t')/c1$ with $t'$ arbitrary.

Inserting $s'$ and $t'$ into the equation of the plane spanned by
trapezium 2 provides the equation of the intersection line, which we
will use in notation $P+Q\cdot u$.

*/

	mpq_class P[3];
	mpq_class Q[3];

	if (cmp(Ap[2][2], 0) == 0)
	{
/*
Case 1: $c1=0$.

*/

		mpq_class f = B[2]/Ap[2][3]; // = b/c2

		if (MR2_DEBUG)
		    cerr << "preciseTrapeziumsIntersect() c1=0 f="
			<< f
			<< endl;

		for (unsigned int i = 0; i < 3; i++)
		{
			P[i] = T2B[i]+T2A[i][1]*f;
			Q[i] = T2A[i][0];
		}
	} else if (cmp(Ap[2][3], 0) ==0)
	{
/*
Case 2: $c2=0$.

*/

		mpq_class f = B[2]/Ap[2][2]; // = b/c1

		if (MR2_DEBUG)
		    cerr << "preciseTrapeziumsIntersect() c2=0 f="
			<< f
			<< endl;

		for (unsigned int i = 0; i < 3; i++) {
			P[i] = T2B[i]+T2A[i][0]*f;
			Q[i] = T2A[i][1];
		}
	} else
	{
/*
Case 3: $c1\ne 0$ and $c2\ne 0$.

Inserting $s'=(b-c2\cdot t')/c1$ into $T2B+T2A\cdot (s', t')$ yields
\begin{eqnarray}
\left(
\begin{array}{c}
T2B[0] \\
T2B[1] \\
T2B[2]
\end{array}
\right)+\left(
\begin{array}{c}
T2A[0][0] \\
T2A[1][0] \\
T2A[2][0]
\end{array}
\right)\cdot (b-c2\cdot t')/c1+\left(
\begin{array}{c}
T2A[0][1] \\
T2A[1][1] \\
T2A[2][1]
\end{array}
\right)\cdot t'
\nonumber\end{eqnarray}
and
\begin{eqnarray}
\left(
\begin{array}{c}
T2B[0]+T2A[0][0]\cdot b/c1 \\
T2B[1]+T2A[1][0]\cdot b/c1 \\
T2B[2]+T2A[2][0]\cdot b/c1
\end{array}
\right)+
\left(
\begin{array}{c}
 T2A[0][1]-T2A[0][0]\cdot c2/c1 \\
 T2A[1][1]-T2A[1][0]\cdot c2/c1 \\
 T2A[2][1]-T2A[2][0]\cdot c2/c1
\end{array}
\right)\cdot t'
\nonumber\end{eqnarray}

*/

		mpq_class f1 = B[2]/Ap[2][2];    // = b/c1
		mpq_class f2 = A[2][3]/Ap[2][2]; // = c2/c1

		if (MR2_DEBUG)
			cerr << "preciseTrapeziumsIntersect() f1="
				 << f1
				 << " f2="
				 << f2
				 << endl;

		for (unsigned int i = 0; i < 3; i++) {
			P[i] = T2B[i]+T2A[i][0]*f1;
			Q[i] = T2A[i][1]-T2A[i][0]*f2;
		}
	}

/*
Now, we have to check whether the intersection line may intersect
at least with one of the segments of each trapezium.

If $Q[2]=0$, the intersection line is parallel to the $(x, y)$-plane.

*/

	if (cmp(Q[2], 0) == 0)
	{
		if (MR2_DEBUG)
		    cerr << "preciseTrapeziumsIntersect()" 
			<< " intersection line "
			 << "parallel to (x, y)-plane"
			 << endl;

/*
Check whether the intersection line is within the z-Range covered by the two
trapeziums.

*/

		if (cmp(P[2], 0) <= 0 || cmp(dt, P[2]) <= 0) {
		    if (MR2_DEBUG)
			cerr << "preciseTrapeziumsIntersect() "
			 << "no intersection"
			 << endl;

		    return false;
		}


/*
Calculate the points where the trapeziums' segments intersect with the
intersection line.

*/

		mpq_class ip1x = trapezium1.segment1.point1.x 
			+ (trapezium1.segment2.point1.x 
			- trapezium1.segment1.point1.x) * P[2]/dt;
		mpq_class ip1y = trapezium1.segment1.point1.y 
			+ (trapezium1.segment2.point1.y 
			- trapezium1.segment1.point1.y) * P[2]/dt;

		mpq_class ip2x = trapezium1.segment1.point2.x 
			+ (trapezium1.segment2.point2.x 
			- trapezium1.segment1.point2.x) * P[2]/dt;
		mpq_class ip2y = trapezium1.segment1.point2.y 
			+ (trapezium1.segment2.point2.y 
			- trapezium1.segment1.point2.y) * P[2]/dt;

		mpq_class ip3x = trapezium2.segment1.point1.x 
			+ (trapezium2.segment2.point1.x 
			- trapezium1.segment1.point1.x) * P[2]/dt;
		mpq_class ip3y = trapezium2.segment1.point1.y 
			+ (trapezium2.segment2.point1.y 
			- trapezium1.segment1.point1.y) * P[2]/dt;

		mpq_class ip4x = trapezium2.segment1.point2.x 
			+ (trapezium2.segment2.point2.x 
			- trapezium2.segment1.point2.x) * P[2]/dt;
		mpq_class ip4y = trapezium2.segment1.point2.y 
			+ (trapezium2.segment2.point2.y 
			- trapezium2.segment1.point2.y) * P[2]/dt;


/*
Check for overlaps of the bounding boxes of the two intersection segments.
If they overlap, the sections may intersect.

*/

		mpq_class ip1ip2MinX = cmp(ip1x, ip2x) < 0 ? 
			ip1x : ip2x;
		mpq_class ip1ip2MaxX = cmp(ip1x, ip2x) > 0 ? 
			ip1x : ip2x;

		mpq_class ip1ip2MinY = cmp(ip1y, ip2y) < 0 ? 
			ip1y : ip2y;
		mpq_class ip1ip2MaxY = cmp(ip1y, ip2y) > 0 ? 
			ip1y : ip2y;

		mpq_class ip3ip4MinX = cmp(ip3x, ip4x) < 0 ? 
			ip3x : ip4x;
		mpq_class ip3ip4MaxX = cmp(ip3x, ip4x) > 0 ? 
			ip3x : ip4x;

		mpq_class ip3ip4MinY = cmp(ip3y, ip4y) < 0 ? 
			ip3y : ip4y;
		mpq_class ip3ip4MaxY = cmp(ip3y, ip4y) > 0 ? 
			ip3y : ip4y;

		if (cmp(ip1ip2MaxX, ip3ip4MinX) < 0
			|| cmp(ip3ip4MaxX, ip1ip2MinX) < 0
			|| cmp(ip1ip2MaxY, ip3ip4MinY) < 0
			|| cmp(ip3ip4MaxY, ip1ip2MinY) < 0)
		{
		    if (MR2_DEBUG)
			cerr << "preciseTrapeziumsIntersect() "
			 << "no intersection"
			 << endl;

		    return false;
		} else
		{
		    if (MR2_DEBUG)
			cerr << "preciseTrapeziumsIntersect() "
			 << "trapeziums intersect"
			 << endl;

		    return true;
		}
	} else
	{
/*
Intersection line hits $(x, y)$-plane in $(P[0], P[1], 0)$ and the
plane parallel to $(x, y)$-plane in distance $dt$ in
$(P[0]+Q[0], P[1]+Q[1], dt)$ as we have handled the case $Q[2]=0$
separately.

*/

		mpq_class z;

		mpq_class t1zMin = dt;
		mpq_class t1zMax = 0;
		bool t1Intersects = false;

		if (preciseSegmentAndLineIntersect
			(0, trapezium1.segment1, P, Q)) {

			t1zMin = cmp(t1zMin, 0) > 0 ? 0 : t1zMin;
			t1zMax = cmp(t1zMax, 0) < 0 ? 0 : t1zMax;
			t1Intersects = true;
		}
		if (preciseSegmentAndLineIntersect
			(dt, trapezium1.segment2, P, Q)) {

			t1zMin = cmp(t1zMin, dt) > 0 ? dt : t1zMin;
			t1zMax = cmp(t1zMax, dt) < 0 ? dt : t1zMax;
			t1Intersects = true;
		}

		PreciseSegment connectingSegment1 = 
			trapezium1.getConnectingSegment1();
		PreciseSegment connectingSegment2 = 
			trapezium1.getConnectingSegment2();
		PrecisePoint point1(P[0], P[1]);
		PrecisePoint point2(P[0]+Q[0], P[1]+Q[1]);
		PreciseSegment lineSegment(point1, point2);

		if (preciseSegmentsIntersect
			(dt, connectingSegment1, lineSegment, z)) {

			t1zMin = cmp(t1zMin, z) > 0 ? z : t1zMin;
			t1zMax = cmp(t1zMax, z) < 0 ? z : t1zMax;
			t1Intersects = true;
		}
		if (preciseSegmentsIntersect
			(dt, connectingSegment2, lineSegment, z)) {

			t1zMin = cmp(t1zMin, z) > 0 ? z : t1zMin;
			t1zMax = cmp(t1zMax, z) < 0 ? z : t1zMax;
			t1Intersects = true;
		}

		mpq_class t2zMin = dt;
		mpq_class t2zMax = 0;

		bool t2Intersects = false;

		if (preciseSegmentAndLineIntersect(0, 
			trapezium2.segment1, P, Q)) {

			t2zMin = cmp(t2zMin, 0) > 0 ? 0 : t2zMin;
			t2zMax = cmp(t2zMax, 0) < 0 ? 0 : t2zMax;
			t2Intersects = true;
		}
		if (preciseSegmentAndLineIntersect(dt, 
			trapezium2.segment2, P, Q)) {

			t2zMin = cmp(t2zMin, dt) > 0 ? dt : t2zMin;
			t2zMax = cmp(t2zMax, dt) < 0 ? dt : t2zMax;
			t2Intersects = true;
		}

		PreciseSegment connectingSegment3 = 
			trapezium2.getConnectingSegment1();
		PreciseSegment connectingSegment4 = 
			trapezium2.getConnectingSegment2();

		if (preciseSegmentsIntersect
			(dt, connectingSegment3, lineSegment, z)) {

			t2zMin = cmp(t2zMin, z) > 0 ? z : t2zMin;
			t2zMax = cmp(t2zMax, z) < 0 ? z : t2zMax;
			t2Intersects = true;
		}
		if (preciseSegmentsIntersect
			(dt, connectingSegment4, lineSegment, z)) {

			t2zMin = cmp(t2zMin, z) > 0 ? z : t2zMin;
			t2zMax = cmp(t2zMax, z) < 0 ? z : t2zMax;
			t2Intersects = true;
		}

		if (MR2_DEBUG) {
		    cerr << "preciseTrapeziumsIntersect() "
		     << "t1Intersects="
			 << t1Intersects
			 << endl;
		    if (t1Intersects)
		    cerr << "preciseTrapeziumsIntersect() " 
			 << "t2Intersects="
			 << t2Intersects
			 << endl;
		}

		if (t1Intersects
			&& t2Intersects
			&& !(cmp(t1zMax, t2zMin) < 0 
				|| cmp(t2zMax, t1zMin) < 0)
			&& !(cmp(t1zMax, 0) == 0 
				&& cmp(t1zMin, 0) == 0 
				&& cmp(t2zMax, 0) == 0 
				&& cmp(t2zMin, 0) == 0)
			&& !(cmp(t1zMax, dt) == 0 
				&& cmp(t1zMin, dt) == 0 
				&& cmp(t2zMax, dt) == 0 
				&& cmp(t2zMin, dt) == 0))
		{
		    if (MR2_DEBUG)
			cerr << "preciseTrapeziumsIntersect() " 
			     << "intersect"
			     << endl;

		    return true;
		} else
		{
		    if (MR2_DEBUG)
			cerr << "preciseTrapeziumsIntersect() " 
			     << "no intersection"
			     << endl;

		    return false;
		}
	}

}


/*
1.1.1 Intersections between segments and trapeziums

1.1.1.1 Function ~segmentAndTrapeziumMayIntersect()~

1.1.1.1 Function ~preciseSegmentAndTrapeziumIntersect()~

1.1.1 Relative positions of points and segments

1.1.1.1 Function ~maybeInside()~

*/
bool maybeInside(int tPoint,
	 GridPoint point,
	 int t1Trapezium,
	 int t2Trapezium,
	 GridPointTrapezium trapezium) {
    if (MR2_DEBUG) cerr << "maybeInside() called" << endl;

/*
Check if $t1Trapezium <= tPoint <= t2Trapezium$ or $t2Trapezium <= tPoint < t1Trapezium$ holds.
If not, no intersection can occur.

*/
    if (!(lowerOrMaybeEqual(t1Trapezium, tPoint, 1, 1)
          && lowerOrMaybeEqual(tPoint, t2Trapezium, 1, 1))
        && !(lowerOrMaybeEqual(t2Trapezium, tPoint, 1, 1)
             && lowerOrMaybeEqual(tPoint, t1Trapezium, 1, 1))) {
        if (MR2_DEBUG) cerr << "sIPT() t check failed" << endl;

        return false;
    }

    if (MR2_DEBUG) cerr << "maybeInside() t check succeeded" << endl;

/*
Calculate the segment parallel to segment1 and segment2 of the trapezium and
at instant tPoint, which will be between the two of the trapeziums segments.

*/
    ProvisionalPoint point1(trapezium.segment1.point1.x, 
	trapezium.segment1.point1.y, 1, 1);
    ProvisionalPoint point2(trapezium.segment1.point2.x, 
	trapezium.segment1.point2.y, 1, 1);
    ProvisionalSegment segment(point1, point2);

    if (maybeEqual(t1Trapezium, t2Trapezium, 1, 1)) {
        segment.point1.x = trapezium.segment1.point1.x;
        segment.point1.y = trapezium.segment1.point1.y;
        segment.point2.x = trapezium.segment1.point2.x;
        segment.point2.y = trapezium.segment1.point2.y;
    } else {
        double f = (tPoint-t1Trapezium)/(t2Trapezium-t1Trapezium);
        double fErr = 4;

        if (MR2_DEBUG) cerr << "maybeInside() f=" << f << endl;

        segment.point1.x = trapezium.segment1.point1.x 
		+ (trapezium.segment2.point1.x 
		- trapezium.segment1.point1.x) * f;
        segment.point1.y = trapezium.segment1.point1.y 
		+ (trapezium.segment2.point1.y 
		- trapezium.segment1.point1.y) * f;
        segment.point2.x = trapezium.segment1.point2.x 
		+ (trapezium.segment2.point2.x 
		- trapezium.segment1.point2.x) * f;
        segment.point2.y = trapezium.segment1.point2.y 
		+ (trapezium.segment2.point2.y 
		+ trapezium.segment1.point2.y) * f;
        segment.point1.xErr = 1+2*fErr;
        segment.point2.xErr = 1+2*fErr;
        segment.point1.yErr = 1+2*fErr;
        segment.point2.yErr = 1+2*fErr;
    }

    if (MR2_DEBUG)
        cerr << "maybeEqual() x1=" << segment.point1.x
             << " y1=" << segment.point1.y
             << " x2=" << segment.point2.x
             << " y2=" << segment.point2.y
             << endl;

/*
If the point maybe located on this resulting segment, the point may also be
inside the trapezium, and it is for sure outside otherwise.

*/
    if (((lowerOrMaybeEqual(segment.point1.x, point.x, 9, 1)
          && lowerOrMaybeEqual(point.x, segment.point2.x, 1, 9))
         || (lowerOrMaybeEqual(segment.point2.x, point.x, 9, 1)
             && lowerOrMaybeEqual(point.x, segment.point1.x, 1, 9)))
        && ((lowerOrMaybeEqual(segment.point1.y, point.y, 9, 1)
             && lowerOrMaybeEqual(point.y, segment.point2.y, 1, 9))
            || (lowerOrMaybeEqual(segment.point2.y, point.y, 9, 1)
                && lowerOrMaybeEqual(point.y, segment.point1.y, 
			1, 9)))) {
        if (MR2_DEBUG) cerr << "maybeInside() inside" << endl;

        return true;
    } else {
        if (MR2_DEBUG) cerr << "maybeInside() not inside" << endl;

        return false;
    }
}

/*
1.1.1.1 Function ~preciseInside()~

*/
bool preciseInside(mpq_class tPoint,
	PrecisePoint point,
	mpq_class t1Trapezium,
	mpq_class t2Trapezium,
	PreciseTrapezium trapezium) {
    if (MR2_DEBUG) cerr << "preciseInside() called" << endl;

/*
Check if $t1Trapezium <= tPoint <= t2Trapezium$ or $t2Trapezium <= tPoint < t1Trapezium$ holds.
If not, no intersection can occur.

*/
    if (!(cmp(t1Trapezium, tPoint) <= 0 
	&& cmp(tPoint, t2Trapezium) <= 0 ) 
	&& !(cmp(t2Trapezium, tPoint) <= 0 
	&& cmp(tPoint, t1Trapezium) <= 0)) 
    {
        if (MR2_DEBUG) 
		cerr << "preciseInside() t check failed" << endl;

        return false;
    }

    if (MR2_DEBUG) 
	cerr << "preciseInside() t check succeeded" << endl;

/*
Calculate the segment parallel to segment1 and segment2 of the trapezium and
at instant tPoint, which will be between the two of the trapeziums segments.

*/
	PrecisePoint point1(trapezium.segment1.point1.x, 
		trapezium.segment1.point1.y);
	PrecisePoint point2(trapezium.segment1.point2.x, 
		trapezium.segment1.point2.y);
    PreciseSegment segment(point1, point2);

	if (cmp(t1Trapezium, t2Trapezium) != 0) {
	    mpq_class f = 
		(tPoint-t1Trapezium)/(t2Trapezium-t1Trapezium);

	    if (MR2_DEBUG) cerr << "preciseInside() f=" << f << endl;

	    segment.point1.x = trapezium.segment1.point1.x 
		+ (trapezium.segment2.point1.x 
		- trapezium.segment1.point1.x) * f;
	    segment.point1.y = trapezium.segment1.point1.y 
		+ (trapezium.segment2.point1.y 
		- trapezium.segment1.point1.y) * f;
	    segment.point2.x = trapezium.segment1.point2.x 
		+ (trapezium.segment2.point2.x 
		- trapezium.segment1.point2.x) * f;
	    segment.point2.y = trapezium.segment1.point2.y 
		+ (trapezium.segment2.point2.y 
		- trapezium.segment1.point2.y) * f;
	}

	if (MR2_DEBUG)
		cerr << "preciseInside() x1=" << segment.point1.x
		 << " y1=" << segment.point1.y
		 << " x2=" << segment.point2.x
		 << " y2=" << segment.point2.y
		 << endl;

/*
If the point is located on this resulting segment, it is also
inside the trapezium, and outside otherwise.

*/
	if (((cmp(segment.point1.x, point.x) <= 0
		  && cmp(point.x, segment.point2.x) <= 0)
		 || (cmp(segment.point2.x, point.x) <= 0
			 && cmp(point.x, segment.point1.x) <= 0))
		&& ((cmp(segment.point1.y, point.y) <= 0
			 && cmp(point.y, segment.point2.y) <= 0)
			|| (cmp(segment.point2.y, point.y) <= 0
			&& cmp(point.y, segment.point1.y) <= 0))) {
	    if (MR2_DEBUG) cerr << "preciseInside() inside" << endl;

	    return true;
	} else {
	    if (MR2_DEBUG) 
		cerr << "preciseInside() not inside" 
		<< endl;

	    return false;
	}
}

/*
1.1.1 Relative positions of points and trapeziums

1.1.1.1 Function ~maybeLeftOrAbove()~

*/
static bool maybeLeftOrAbove(GridPoint point,
             GridPointSegment segment) {
    if (MR2_DEBUG)
        cerr << "maybeLeftOrAbove() called p=(" 
	     << point.x << " " << point.y
             << ") p1=(" << segment.point1.x 
	     << " " << segment.point1.y
             << ") p2=(" << segment.point2.x << " " 
	     << segment.point2.y << ")"
             << endl;

    if (maybeEqual(segment.point1.x, segment.point2.x, 1, 1)) {
        if (MR2_DEBUG)
            cerr << "maybeLeftOrAbove() segment is horizontal" 
	         << endl;

        return lowerOrMaybeEqual(point.x, segment.point1.x, 1, 1);
    } else if (maybeEqual(segment.point1.y, segment.point2.y, 1, 1)) {
        if (MR2_DEBUG)
            cerr << "maybeLeftOrAbove() segment is vertical" 
	         << endl;

        return greaterOrMaybeEqual(point.y, segment.point1.y, 1, 1);
    } else {
        if (MR2_DEBUG)
            cerr << "maybeLeftOrAbove() "
		 << "segment is not horizontal nor vertical" 
		 << endl;

        double t = (point.x- segment.point1.x)/(segment.point2.x 
		- segment.point1.x); //maxError 4
        double py = segment.point1.y + (segment.point2.y 
		- segment.point1.y) * t; //maxError 9

        if (MR2_DEBUG)
            cerr << "maybeLeftOrAbove() py=" << py << endl;

        return greaterOrMaybeEqual(point.y, py, 1, 9);
    }
}

/*
1.1.1.1 Function ~preciseLeftOrAbove()~

*/
static bool preciseLeftOrAbove(PrecisePoint point,
                PreciseSegment segment) {
    if (MR2_DEBUG)
        cerr << "preciseLeftOrAbove() called p=(" 
	     << point.x << " " << point.y
             << ") p1=(" << segment.point1.x << " " 
	     << segment.point1.y
             << ") p2=(" << segment.point2.x << " " 
	     << segment.point2.y << ")"
             << endl;

    if (cmp(segment.point1.x, segment.point2.x) == 0) {
        if (MR2_DEBUG)
            cerr << "preciseLeftOrAbove() segment is horizontal" 
	         << endl;

        return cmp(point.x, segment.point1.x) <= 0;
    } else if (cmp(segment.point1.y, segment.point2.y) == 0) {
        if (MR2_DEBUG)
            cerr << "preciseLeftOrAbove() segment is vertical" 
		 << endl;

        return cmp(point.y, segment.point1.y) >= 0;
    } else {
        if (MR2_DEBUG)
            cerr << "preciseLeftOrAbove() segment is not "
		 << "horizontal nor vertical" << endl;

        mpq_class t = (point.x- segment.point1.x)/
		(segment.point2.x - segment.point1.x);
        mpq_class py = segment.point1.y + 
		(segment.point2.y - segment.point1.y) * t;

        if (MR2_DEBUG)
            cerr << "preciseLeftOrAbove() py=" << py << endl;

        return cmp(point.y, py) >= 0;
    }
}

/*
1.1 Other helper functions

1.1.1 Function ~restrictUPointToInterval()~

*/
static void restrictUPointToInterval(const UPoint& up,
                     const Interval<Instant> iv,
                     UPoint& rUp) {
    if (MR2_DEBUG)
        cerr << "restrictUPointToInterval() called" << endl;

    if (nearlyEqual(iv.start.ToDouble(), iv.end.ToDouble()))
        rUp = up;
    else {
        double ti =
            (iv.start.ToDouble()
             -up.timeInterval.start.ToDouble())
            /(up.timeInterval.end.ToDouble()
              -up.timeInterval.start.ToDouble());
        double tf =
            (iv.end.ToDouble()
             -up.timeInterval.start.ToDouble())
            /(up.timeInterval.end.ToDouble()
              -up.timeInterval.start.ToDouble());

        UPoint dummy(
            iv,
            up.p0.GetX()+(up.p1.GetX()-up.p0.GetX())*ti,
            up.p0.GetY()+(up.p1.GetY()-up.p0.GetY())*ti,
            up.p0.GetX()+(up.p1.GetX()-up.p0.GetX())*tf,
            up.p0.GetY()+(up.p1.GetY()-up.p0.GetY())*tf);

        rUp = dummy;
    }
}


/*
1 Supporting classes

Supporting classes are those, which are not registered as SECONDO datatypes
but are used to implement the SECONDO datatypes ~intimeregion2~, ~uregion2~ and
~movingregion2~.

1.1 Class ~MSegmentData2~

This class is used to represent the segments, which are used to represent
region units in section \ref{uregion2}.

1.1.1 Class definition

The class definition has been moved to ~MovingRegion2Algebra.h~.

1.1.1 Constructor for a basic segment
\label{collinear}

*/
MSegmentData2::MSegmentData2(
    unsigned int fno,
    unsigned int cno,
    unsigned int sno,
    bool ia,
    int isx,
    int isy,
    int iex,
    int iey,
    int fsx,
    int fsy,
    int fex,
    int fey) :
    faceno(fno),
    cycleno(cno),
    segmentno(sno),
    insideAbove(ia),
    isBasicSegment(true),
    degeneratedInitialNext(-1),
    degeneratedFinalNext(-1),
    degeneratedInitial(DGM_UNKNOWN),
    degeneratedFinal(DGM_UNKNOWN),
    initialStartX(isx),
    initialStartY(isy),
    initialEndX(iex),
    initialEndY(iey),
    finalStartX(fsx),
    finalStartY(fsy),
    finalEndX(fex),
    finalEndY(fey)  {

	if (MR2_DEBUG)
	        cerr << "MSegmentData2::MSegmentData2() #2 "
	             << "called counter=["
	             << faceno << " " << cycleno << " " << segmentno
	             << "] flags=["
	             << insideAbove
	             << " " << degeneratedInitialNext
	             << " " << degeneratedFinalNext
	             << "] initial=["
	             << initialStartX << " " << initialStartY
	             << " "
	             << initialEndX << " " << initialEndY
	             << "] final=["
	             << finalStartX << " " << finalStartY
	             << " "
	             << finalEndX << " " << finalEndY
	             << "]"
	             << endl;

/*
Calculate whether segment is point in initial or final instant.

*/

    	pointInitial = (isx == iex && isy == iey);
    	pointFinal = (fsx == fex && fsy == fey);

/*
Check whether initial and final segment are colinear,

*/
    	bool collinear;

    	if (pointInitial && pointFinal) {
/*
Error: A segment may not be reduced to a point both in initial and final
instant.

*/

    		if (MR2_DEBUG)
    		    cerr << "MSegmentData2::MSegmentData2() "
			 << "both reduced"
    		         << endl;

		throw invalid_argument(
			"both initial and final segment "
			"reduced to point, which is not "
			"allowed");

    	} else if (pointInitial) {
/*
Only initial segment reduced to point. Initial and final segment are trivially
colinear.

*/

    		if (MR2_DEBUG)
	            cerr << "MSegmentData2::MSegmentData2() "
	                 << "initial reduced"
	                 << endl;

    		collinear = true;
    	} else if (pointFinal) {
/*
Only final segment reduced to point. Initial and final segment are trivially
colinear.

*/

    		if (MR2_DEBUG)
	            cerr << "MSegmentData2::MSegmentData2() "
	                 << "final reduced"
	                 << endl;

    		collinear = true;
    	} else if (isx == iex && fsx == fex) {
/*
Both segments are vertical. Check if both segments have the same
orientation.

*/

    		if (MR2_DEBUG)
	            cerr << "MSegmentData2::MSegmentData2() "
	                 << "both vertical"
	                 << endl;


    		collinear = ((isy <= iey && fsy <= fey)
    		            || (isy >= iey && fsy >= fey));
    	} else if (isx == iex || fsx == fex) {
/*
Only initial or final segment is vertical but not both.

*/

    		if (MR2_DEBUG)
	            cerr << "MSegmentData2::MSegmentData2() "
	                 << "one vertical" << endl;

    		collinear = false;
    	} else {
/*
Both segments are not vertical.

*/

    		if (MR2_DEBUG)
	            cerr << "MSegmentData2::MSegmentData2() "
	                 << "none vertical"
	                 << endl;

    		double deltaIY = iey - isy;
    		double deltaIX = iex - isx;
    		double deltaFY = fey - fsy;
    		double deltaFX = fex - fsx;

    		collinear = 
			deltaIY / deltaIX == deltaFY / deltaFX
    			|| deltaIY * deltaFX == deltaFY * deltaIX;

    		if (!collinear) {
    		    cerr << setprecision(10)
    		         << "parameters for "
			 << "segment orientation comparison:"
	                 << endl
	                 << "  1. (iey-isy)/(iex-isx) = "
	                 << deltaIY / deltaIX
	                 << endl
	                 << "  2. (fey-fsy)/(fex-fsx) = "
	                 << deltaFY / deltaFX
	                 << endl
	                 << "  3. (iey-isy)*(fex-fsx) = "
	                 << deltaIY * deltaFX
	                 << endl
	                 << "  4. (fey-fsy)*(iex-isx) = "
	                 << deltaFY * deltaIX
	                 << endl
	                 << "1. and 2. or 3. and 4. should be equal."
	                 <<"("<<isx<<";"<<isy<<")"
	                 <<"("<<iex<<";"<<iey<<")"
	                 <<"("<<fsx<<";"<<fsy<<")"
	                 <<"("<<fex<<";"<<fey<<")"
	                 <<fno<<" "
	                 <<cno<<" "
	                 <<sno
	                 << endl;
    		}

		if (MR2_DEBUG) {
			cerr << ::std::fixed << ::std::setprecision(6);
			cerr << "MSegmentData2::MSegmentData2() isx=" 
			     << isx
			     << " isy=" << isy
			     << " iex=" << iex
			     << " iey=" << iey
			     << " fsx=" << fsx
			     << " fsy=" << fsy
			     << " fex=" << fex
			     << " fey=" << fey
			     << endl;
			cerr << "MSegmentData2::MSegmentData2() " 
			     << "(iey-isy)/(iex-isx)="
			     << deltaIY / deltaIX << endl;
			cerr << "MSegmentData2::MSegmentData2() "
			     << "(fey-fsy)/(fex-fsx)="
			     << deltaFY / deltaFX << endl;
			cerr << "MSegmentData2::MSegmentData2() "
			     << "(iey-isy)*(fex-fsx)="
			     << deltaIY * deltaFX << endl;
			cerr << "MSegmentData2::MSegmentData2() "
			     << "(fey-fsy)*(iex-isx)="
			     << deltaFY * deltaIX << endl;
			cerr << "MSegmentData2::MSegmentData2() "
			     << "collinear="
			     << collinear << endl;
		}

    	} //Check if collinear

    	if (!collinear) {
    		throw invalid_argument(
		"initial and final segment not collinear");
    	}


} //End of constructor for a basic segment


/*
1.1.1 Constructor for a non-basic segment
\label{collinear2}

*/
MSegmentData2::MSegmentData2(
    unsigned int fno,
    unsigned int cno,
    unsigned int sno,
    bool ia,
    int isx,
    int isy,
    int iex,
    int iey,
    int fsx,
    int fsy,
    int fex,
    int fey,
    PreciseMSegmentData& preciseSegment,
    DbArray<int>* preciseCoordinates) :
    faceno(fno),
    cycleno(cno),
    segmentno(sno),
    insideAbove(ia),
    isBasicSegment(false),
    degeneratedInitialNext(-1),
    degeneratedFinalNext(-1),
    degeneratedInitial(DGM_UNKNOWN),
    degeneratedFinal(DGM_UNKNOWN),
    initialStartX(isx),
    initialStartY(isy),
    initialEndX(iex),
    initialEndY(iey),
    finalStartX(fsx),
    finalStartY(fsy),
    finalEndX(fex),
    finalEndY(fey) {

	if (MR2_DEBUG)
	        cerr << "MSegmentData2::MSegmentData2() #2 "
	             << "called for non-basic segment. counter=["
	             << faceno << " " << cycleno << " " << segmentno
	             << "] flags=["
	             << insideAbove
	             << " " << degeneratedInitialNext
	             << " " << degeneratedFinalNext
	             << "]" << endl
	             << "initial=["
	             << initialStartX << " " << initialStartY
	             << " "
	             << initialEndX << " " << initialEndY
	             << "] final=["
	             << finalStartX << " " << finalStartY
	             << " "
	             << finalEndX << " " << finalEndY
	             << "]"
	             << endl;
/*
Calculate whether segment is point in initial or final instant.

*/
    	pointInitial = (maybeEqual(isx, iex, 1, 1) 
	    && maybeEqual(isy, iey, 1, 1)
    	    && (preciseSegment.GetInitialStartY
		(preciseCoordinates) + isx ==
    		preciseSegment.GetInitialEndX
			(preciseCoordinates) + iex)
    	    && (preciseSegment.GetInitialStartY
			(preciseCoordinates) + isy ==
    		preciseSegment.GetInitialEndY
			(preciseCoordinates) + iey));

    	pointFinal = (maybeEqual(fsx, fex, 1, 1) 
	    && maybeEqual(fsy, fey, 1, 1)
    	    && (preciseSegment.GetFinalStartX
		(preciseCoordinates) + fsx ==
    		preciseSegment.GetFinalEndX
			(preciseCoordinates) + fex)
    	    && (preciseSegment.GetFinalStartY
			(preciseCoordinates) + fsy ==
    		preciseSegment.GetFinalEndY
			(preciseCoordinates) + fey));

/*
Check whether initial and final segment are collinear,

*/


    	bool collinear;

    	if (pointInitial && pointFinal) {
/*
Error: A segment may not be reduced to a point both in initial and final
instant.

*/
    		if (MR2_DEBUG)
			cerr << "MSegmentData2::MSegmentData2() "
			     << "segments both reduced to point!"
			     << endl;

		throw invalid_argument
			("both initial and final segment "
			 "reduced to point, which is not "
			"allowed");
    	} else if (pointInitial) {
/*
Only initial segment reduced to point. Initial and final segment are trivially
collinear.

*/

    		collinear = true;
    	} else if (pointFinal) {
/*
Only final segment reduced to point. Initial and final segment are trivially
collinear.

*/

    		collinear = true;
    	} else if (maybeEqual(isx, iex, 1, 1) 
		&& maybeEqual(fsx, fex, 1, 1)
    		&& (preciseSegment.GetInitialStartX
			(preciseCoordinates) + isx ==
    		    preciseSegment.GetInitialEndX
			(preciseCoordinates) + iex)
    		&& (preciseSegment.GetFinalStartX
			(preciseCoordinates) + fsx ==
    		    preciseSegment.GetFinalEndX
			(preciseCoordinates) + fex)) {
/*
Both segments are vertical. Check if both segments have the same
orientation.

*/

		collinear = ((isLower(isy, iey, 1, 1) 
			&& isLower(fsy, fey, 1, 1))	
			|| (isGreater(isy, iey, 1, 1) 
			&& isGreater(fsy, fey, 1, 1)));
    		if (!collinear)
    		{
    			if ((maybeEqual(isy, iey, 1, 1) 
				&& maybeEqual(fsy, fey, 1, 1))
				|| (maybeEqual(isy, iey, 1, 1) 
				&& maybeEqual(fsy, fey, 1, 1)))
    			{

    			    collinear = (cmp(preciseSegment.GetInitialStartY
					(preciseCoordinates) + isy,
				    preciseSegment.GetInitialEndY
					(preciseCoordinates) + iey) < 0
    				&& cmp(preciseSegment.GetFinalStartY
					(preciseCoordinates) + fsy,
    		  		    preciseSegment.GetFinalEndY
					(preciseCoordinates) + fey) < 0)
    				|| (cmp(preciseSegment.GetInitialStartY
					(preciseCoordinates) + isy,
    				    preciseSegment.GetInitialEndY
					(preciseCoordinates) + iey) > 0
    				&& cmp(preciseSegment.GetFinalStartY
					(preciseCoordinates) + fsy,
    				    preciseSegment.GetFinalEndY
					(preciseCoordinates) + fey) > 0);

    			}

    		}

    	} else if ((maybeEqual(isx, iex, 1, 1) 
		|| maybeEqual(fsx, fex, 1, 1))
    		&& ((preciseSegment.GetInitialStartX
			(preciseCoordinates) + isx ==
    				preciseSegment.GetInitialEndX
					(preciseCoordinates) + iex)
    		|| (preciseSegment.GetFinalStartX(preciseCoordinates) 
			+ fsx == preciseSegment.GetFinalEndX
				(preciseCoordinates) + fex))) {
/*
Only initial or final segment is vertical but not both.

*/



    		collinear = false;
    	} else {
/*
Both segments are not vertical. We first try to find out with grid coordinates if the segments are collinear or not. If they seem to be collinear, we make a precise recheck.
Here we must be extremely careful not to divide by zero! If the denominator is zero, we cannot make the check on integer grid and have to do a precise check at once!

*/

		bool denumZero = (iex == isx || fex == fsx );
		if (!denumZero)
		{
			collinear = maybeEqual((iey-isy)/(iex-isx),
				(fey-fsy)/(fex-fsx), 4, 4)
				|| maybeEqual((iey-isy)*(fex-fsx), 
				(fey-fsy)*(iex-isx), 4, 4);
		}

    		if (denumZero || collinear)
    		{
    		    collinear = ((preciseSegment.GetInitialEndY
			(preciseCoordinates) + iey 
			- (preciseSegment.GetInitialStartY
			(preciseCoordinates) + isy)) /
    			(preciseSegment.GetInitialEndX
			(preciseCoordinates) + iex 
			- (preciseSegment.GetInitialStartX
			(preciseCoordinates) + isx)) == 
			(preciseSegment.GetFinalEndY(preciseCoordinates) 
			+ fey - (preciseSegment.GetFinalStartY
			(preciseCoordinates) + fsy)) /
    			(preciseSegment.GetFinalEndX(preciseCoordinates) 
			+ fex -
    			(preciseSegment.GetFinalStartX
			(preciseCoordinates) + fsx)));

    			if (!collinear)
    			{
			    //Check if second part of the OR is true:
			    collinear = ((preciseSegment.GetInitialEndY
				(preciseCoordinates) + iey -
				(preciseSegment.GetInitialStartY
				(preciseCoordinates) + isy)) *
				(preciseSegment.GetFinalEndX
				(preciseCoordinates) + fex -
				(preciseSegment.GetFinalStartX
				(preciseCoordinates) + fsx)) ==
    				(preciseSegment.GetFinalEndY
				(preciseCoordinates) + fey -	
				(preciseSegment.GetFinalStartY
				(preciseCoordinates) + fsy)) *
				(preciseSegment.GetInitialEndX
				(preciseCoordinates) + iex -
				(preciseSegment.GetInitialStartX
				(preciseCoordinates) + isx)));
    			}
    		}
    	} //Check if collinear

    	if (!collinear) {
    		throw invalid_argument(
			"initial and final segment not collinear");
    	}


} //End of constructor for a non-basic segment

/*
1.1.1 Constructor for a segment from a segment pointer.

*/
MSegmentData2::MSegmentData2(MSegmentData2* segmentPointer) :
    faceno(segmentPointer->faceno),
    cycleno(segmentPointer->cycleno),
    segmentno(segmentPointer->segmentno),
    insideAbove(segmentPointer->insideAbove),
    isBasicSegment(segmentPointer->isBasicSegment),
    degeneratedInitialNext(segmentPointer->degeneratedInitialNext),
    degeneratedFinalNext(segmentPointer->degeneratedFinalNext),
    degeneratedInitial(segmentPointer->degeneratedInitial),
    degeneratedFinal(segmentPointer->degeneratedFinal),
    initialStartX(segmentPointer->initialStartX),
    initialStartY(segmentPointer->initialStartY),
    initialEndX(segmentPointer->initialEndX),
    initialEndY(segmentPointer->initialEndY),
    finalStartX(segmentPointer->finalStartX),
    finalStartY(segmentPointer->finalStartY),
    finalEndX(segmentPointer->finalEndX),
    finalEndY(segmentPointer->finalEndY)  {

	if (MR2_DEBUG)
	        cerr << "MSegmentData2::MSegmentData2() #3 "
	             << "called for segment from segment pointer. counter=["
	             << faceno << " " << cycleno << " " << segmentno
	             << "] flags=["
	             << insideAbove
	             << " " << degeneratedInitialNext
	             << " " << degeneratedFinalNext
	             << "]" << endl
	             << "isBasicSegment is " << isBasicSegment << endl
	             << "initial=["
	             << initialStartX << " " << initialStartY
	             << " "
	             << initialEndX << " " << initialEndY
	             << "] final=["
	             << finalStartX << " " << finalStartY
	             << " "
	             << finalEndX << " " << finalEndY
	             << "]"
	             << endl;

}

/*
1.1.1 Method ~ToString()~

*/
string MSegmentData2::ToString(void) const {
    ostringstream tmp;

    tmp << "f/c/s=" << faceno
        << "/" << cycleno
        << "/" << segmentno
        << " initial: p/d/dn=" << pointInitial
        << "/" << degeneratedInitial
        << "/" << degeneratedInitialNext
        << " (" << initialStartX
        << ", " << initialStartY
        << ")-(" << initialEndX
        << ", " << initialEndY
        << ") final: p/d/dn=" << pointFinal
        << "/" << degeneratedFinal
        << "/" << degeneratedFinalNext
        << " (" << finalStartX
        << ", " << finalStartY
        << ")-(" << finalEndX
        << ", " << finalEndY
        << ")";

    return tmp.str();
}



/*
1.1 Class ~PreciseMSegmentData~

This class is used to represent the precise segments

1.1.1 Class definition

The class definition has been moved to ~MovingRegion2Algebra.h~.

1.1.1 Constructors

The integer arguments serve to initialize the array indices.
An index of -1 means that elements will be attached at the end
of the array, and indices will be set accordingly.

*/
	PreciseMSegmentData::PreciseMSegmentData(int isxPos, int isyPos, 
			int iexPos, int ieyPos,
			int fsxPos, int fsyPos, int fexPos, int feyPos,
			int isxNum, int isyNum, int iexNum, int ieyNum,
			int fsxNum, int fsyNum, int fexNum, int feyNum) :
			isxStartPos(isxPos),
			isyStartPos(isyPos),
			iexStartPos(iexPos),
			ieyStartPos(ieyPos),
			fsxStartPos(fsxPos),
			fsyStartPos(fsyPos),
			fexStartPos(fexPos),
			feyStartPos(feyPos),
			isxNumOfChars(isxNum),
			isyNumOfChars(isyNum),
			iexNumOfChars(iexNum),
			ieyNumOfChars(ieyNum),
			fsxNumOfChars(fsxNum),
			fsyNumOfChars(fsyNum),
			fexNumOfChars(fexNum),
			feyNumOfChars(feyNum) {}

	PreciseMSegmentData::PreciseMSegmentData(int startPos) :
			isxStartPos(startPos),
			isyStartPos(startPos),
			iexStartPos(startPos),
			ieyStartPos(startPos),
			fsxStartPos(startPos),
			fsyStartPos(startPos),
			fexStartPos(startPos),
			feyStartPos(startPos),
			isxNumOfChars(0),
			isyNumOfChars(0),
			iexNumOfChars(0),
			ieyNumOfChars(0),
			fsxNumOfChars(0),
			fsyNumOfChars(0),
			fexNumOfChars(0),
			feyNumOfChars(0) {}

/*
1.1.1 Write access methods.

All these methods take the argument of type mpq-class, convert it to an
arbitrary number of chars and stores these chars in the given DbArray. The
private attributes representing the array indices to restore the coordinates
are of course also updated.

*/
void PreciseMSegmentData::SetInitialStartX (mpq_class x, 
		DbArray<int>* preciseCoordinates) {

	if (MR2_DEBUG)
	    cerr << "PreciseMSegmentData::SetInitialStartX() "
		 << "called with isx = " << x
		 << ", starting at array position " 
		 << isxStartPos << ", array address is " 
		 << preciseCoordinates << endl;

	stringstream theStream;
	theStream << x;
	if (isxStartPos == -1) isxStartPos = preciseCoordinates->Size();
	int index = isxStartPos;
	if (index >= preciseCoordinates->Size())
	{
		preciseCoordinates->resize(preciseCoordinates->Size() 
			+ gmpCharNum);
	}
	for (int i = 0; theStream.good(); i++)
	{
		char nextChar;
		theStream.get(nextChar);
		if (i > 0 && i % gmpCharNum == (gmpCharNum - 1))
		{
			if (MR2_DEBUG)
				cerr << "resizing array..." << endl;

/*
A new group of int values is needed since the one(s) used so far is / are not enough to represent this mpq-class value.

*/
			int nextGroupsIndex = preciseCoordinates->Size();
			if (nextGroupsIndex == index)
				nextGroupsIndex++;
			preciseCoordinates->Put(index, nextGroupsIndex);
			if (MR2_DEBUG)
				cerr << "Index of next group is " 
				<< nextGroupsIndex << endl;
			index = nextGroupsIndex;
			preciseCoordinates->resize
				(preciseCoordinates->Size() + gmpCharNum);
		}

		if (theStream.good())
		{
		    if (MR2_DEBUG)
			cerr << "Putting isx in the array in loop : " 
			    << i << ", value is " << nextChar << endl;
		    preciseCoordinates->Put(index, (int)nextChar);
		    index++;
		}
		else
		{
		    if (MR2_DEBUG)
			cerr << "Setting isxNumOfChars to " 
				<< i << endl;

		    isxNumOfChars = i;
		    for (int j = index; j % gmpCharNum > 0; j++)
		    {
			preciseCoordinates->Put(j, -1);
		    }
		}
	}

}

void PreciseMSegmentData::SetInitialStartY (mpq_class x, 
		DbArray<int>* preciseCoordinates) {
	if (MR2_DEBUG)
		cerr << "PreciseMSegmentData::SetInitialStartY() called" 
		<< endl;
	stringstream theStream;
	theStream << x;
	if (isyStartPos == -1) isyStartPos = preciseCoordinates->Size();
	int index = isyStartPos;
	if (index >= preciseCoordinates->Size())
	{
		preciseCoordinates->resize(preciseCoordinates->Size() 
			+ gmpCharNum);
	}
	for (int i = 0; theStream.good(); i++)
	{
		char nextChar;
		theStream.get(nextChar);
		if (i > 0 && i % gmpCharNum == (gmpCharNum - 1))
		{
			if (MR2_DEBUG)
				cerr << "resizing array..." << endl;

/*
A new group of int values is needed since the one(s) used so far is / are not enough to represent this mpq-class value.

*/
			int nextGroupsIndex = preciseCoordinates->Size();
			if (nextGroupsIndex == index)
				nextGroupsIndex++;
			preciseCoordinates->Put(index, nextGroupsIndex);
			if (MR2_DEBUG)
				cerr << "Index of next group is " 
				<< nextGroupsIndex << endl;
			index = nextGroupsIndex;
			preciseCoordinates->resize
				(preciseCoordinates->Size() + gmpCharNum);
		}

		if (theStream.good())
		{
		    if (MR2_DEBUG)
			cerr << "Putting isy in the array in loop : " 
			<< i << ", value is " << nextChar << endl;
		    preciseCoordinates->Put(index, (int)nextChar);
		    index++;
		}
		else
		{
			isyNumOfChars = i;
			for (int j = index; j % gmpCharNum > 0; j++)
			{
				preciseCoordinates->Put(j, -1);
			}
		}
	}
}

void PreciseMSegmentData::SetInitialEndX (mpq_class x, 
		DbArray<int>* preciseCoordinates) {
	stringstream theStream;
	theStream << x;
	if (iexStartPos == -1) iexStartPos = preciseCoordinates->Size();
	int index = iexStartPos;
	if (index >= preciseCoordinates->Size())
	{
		preciseCoordinates->resize
			(preciseCoordinates->Size() + gmpCharNum);
	}
	for (int i = 0; theStream.good(); i++)
	{
		char nextChar;
		theStream.get(nextChar);
		if (i > 0 && i % gmpCharNum == (gmpCharNum - 1))
		{
			if (MR2_DEBUG)
				cerr << "resizing array..." << endl;

/*
A new group of int values is needed since the one(s) used so far is / are not enough to represent this mpq-class value.

*/
			int nextGroupsIndex = preciseCoordinates->Size();
			if (nextGroupsIndex == index)
				nextGroupsIndex++;
			preciseCoordinates->Put(index, nextGroupsIndex);
			if (MR2_DEBUG)
				cerr << "Index of next group is " 
				<< nextGroupsIndex << endl;
			index = nextGroupsIndex;
			preciseCoordinates->resize
				(preciseCoordinates->Size() + gmpCharNum);
		}

		if (theStream.good())
		{
		    if (MR2_DEBUG)
			cerr << "Putting iex in the array in loop : " 
			<< i << ", value is " << nextChar << endl;
		    preciseCoordinates->Put(index, (int)nextChar);
		    index++;
		}
		else
		{
			iexNumOfChars = i;
			for (int j = index; j % gmpCharNum > 0; j++)
			{
				preciseCoordinates->Put(j, -1);
			}
		}
	}
}

void PreciseMSegmentData::SetInitialEndY (mpq_class x, 
		DbArray<int>* preciseCoordinates) {
	stringstream theStream;
	theStream << x;
	if (ieyStartPos == -1) ieyStartPos = preciseCoordinates->Size();
	int index = ieyStartPos;
	if (index >= preciseCoordinates->Size())
	{
		preciseCoordinates->resize(preciseCoordinates->Size() 
			+ gmpCharNum);
	}
	for (int i = 0; theStream.good(); i++)
	{
		char nextChar;
		theStream.get(nextChar);
		if (i > 0 && i % gmpCharNum == (gmpCharNum - 1))
		{
			if (MR2_DEBUG)
				cerr << "resizing array..." << endl;

/*
A new group of int values is needed since the one(s) used so far is / are not enough to represent this mpq-class value.

*/
			int nextGroupsIndex = preciseCoordinates->Size();
			if (nextGroupsIndex == index)
				nextGroupsIndex++;
			preciseCoordinates->Put(index, nextGroupsIndex);
			if (MR2_DEBUG)
				cerr << "Index of next group is " 
				<< nextGroupsIndex << endl;
			index = nextGroupsIndex;
			preciseCoordinates->resize
				(preciseCoordinates->Size() + gmpCharNum);
		}

		if (theStream.good())
		{
		    if (MR2_DEBUG)
			cerr << "Putting iey in the array in loop : " 
			<< i << ", value is " << nextChar << endl;
		    preciseCoordinates->Put(index, (int)nextChar);
		    index++;
		}
		else
		{
			ieyNumOfChars = i;
			for (int j = index; j % gmpCharNum > 0; j++)
			{
				preciseCoordinates->Put(j, -1);
			}
		}
	}
}


void PreciseMSegmentData::SetFinalStartX (mpq_class x, 
		DbArray<int>* preciseCoordinates) {
	stringstream theStream;
	theStream << x;
	if (fsxStartPos == -1) fsxStartPos = preciseCoordinates->Size();
	int index = fsxStartPos;
	if (index >= preciseCoordinates->Size())
	{
		preciseCoordinates->resize
			(preciseCoordinates->Size() + gmpCharNum);
	}
	for (int i = 0; theStream.good(); i++)
	{
		char nextChar;
		theStream.get(nextChar);
		if (i > 0 && i % gmpCharNum == (gmpCharNum - 1))
		{
			if (MR2_DEBUG)
				cerr << "resizing array..." << endl;

/*
A new group of int values is needed since the one(s) used so far is / are not enough to represent this mpq-class value.

*/
			int nextGroupsIndex = preciseCoordinates->Size();
			if (nextGroupsIndex == index)
					nextGroupsIndex++;
			preciseCoordinates->Put(index, nextGroupsIndex);
			if (MR2_DEBUG)
				cerr << "Index of next group is " 
				<< nextGroupsIndex << endl;
			index = nextGroupsIndex;
			preciseCoordinates->resize
				(preciseCoordinates->Size() + gmpCharNum);
		}

		if (theStream.good())
		{
		    if (MR2_DEBUG)
			cerr << "Putting fsx in the array in loop :"
			<< i << ", value is " << nextChar << endl;
		    preciseCoordinates->Put(index, (int)nextChar);
		    index++;
		}
		else
		{
			fsxNumOfChars = i;
			for (int j = index; j % gmpCharNum > 0; j++)
			{
				preciseCoordinates->Put(j, -1);
			}
		}
	}
}


void PreciseMSegmentData::SetFinalStartY (mpq_class x, 
		DbArray<int>* preciseCoordinates) {
	stringstream theStream;
	theStream << x;
	if (fsyStartPos == -1) fsyStartPos = preciseCoordinates->Size();
	int index = fsyStartPos;
	if (index >= preciseCoordinates->Size())
	{
		preciseCoordinates->resize(preciseCoordinates->Size() 
			+ gmpCharNum);
	}
	for (int i = 0; theStream.good(); i++)
	{
		char nextChar;
		theStream.get(nextChar);
		if (i > 0 && i % gmpCharNum == (gmpCharNum - 1))
		{
			if (MR2_DEBUG)
				cerr << "resizing array..." << endl;

/*
A new group of int values is needed since the one(s) used so far is / are not enough to represent this mpq-class value.

*/
			int nextGroupsIndex = preciseCoordinates->Size();
			if (nextGroupsIndex == index)
					nextGroupsIndex++;
			preciseCoordinates->Put(index, nextGroupsIndex);
			if (MR2_DEBUG)
				cerr << "Index of next group is " 
				<< nextGroupsIndex << endl;
			index = nextGroupsIndex;
			preciseCoordinates->resize
				(preciseCoordinates->Size() 
				+ gmpCharNum);
		}

		if (theStream.good())
		{
		    if (MR2_DEBUG)
			cerr << "Putting fsy in the array in loop : " 
			<< i << ", value is " << nextChar << endl;
		    preciseCoordinates->Put(index, (int)nextChar);
		    index++;
		}
		else
		{
			fsyNumOfChars = i;
			for (int j = index; j % gmpCharNum > 0; j++)
			{
				preciseCoordinates->Put(j, -1);
			}
		}
	}
}

void PreciseMSegmentData::SetFinalEndX (mpq_class x, 
		DbArray<int>* preciseCoordinates) {
	stringstream theStream;
	theStream << x;
	if (fexStartPos == -1) fexStartPos = preciseCoordinates->Size();
	int index = fexStartPos;
	if (index >= preciseCoordinates->Size())
	{
		preciseCoordinates->resize(preciseCoordinates->Size() 
			+ gmpCharNum);
	}
	for (int i = 0; theStream.good(); i++)
	{
		char nextChar;
		theStream.get(nextChar);
		if (i > 0 && i % gmpCharNum == (gmpCharNum - 1))
		{
			if (MR2_DEBUG)
				cerr << "resizing array..." << endl;

/*
A new group of int values is needed since the one(s) used so far is / are not enough to represent this mpq-class value.

*/
			int nextGroupsIndex = preciseCoordinates->Size();
			if (nextGroupsIndex == index)
					nextGroupsIndex++;
			preciseCoordinates->Put(index, nextGroupsIndex);
			if (MR2_DEBUG)
				cerr << "Index of next group is " 
				<< nextGroupsIndex << endl;
			index = nextGroupsIndex;
			preciseCoordinates->resize(
				preciseCoordinates->Size() + gmpCharNum);
		}

		if (theStream.good())
		{
		    if (MR2_DEBUG)
			cerr << "Putting fex in the array in loop : " 
			<< i << ", value is " << nextChar << endl;
		    preciseCoordinates->Put(index, (int)nextChar);
		    index++;
		}
		else
		{
			fexNumOfChars = i;
			for (int j = index; j % gmpCharNum > 0; j++)
			{
				preciseCoordinates->Put(j, -1);
			}
		}
	}
}

void PreciseMSegmentData::SetFinalEndY (mpq_class x, 
		DbArray<int>* preciseCoordinates) {
	stringstream theStream;
	theStream << x;
	if (feyStartPos == -1) feyStartPos = preciseCoordinates->Size();
	int index = feyStartPos;
	if (index >= preciseCoordinates->Size())
	{
		preciseCoordinates->resize(preciseCoordinates->Size() 
			+ gmpCharNum);
	}
	for (int i = 0; theStream.good(); i++)
	{
		char nextChar;
		theStream.get(nextChar);
		if (i > 0 && i % gmpCharNum == (gmpCharNum - 1))
		{
			if (MR2_DEBUG)
				cerr << "resizing array..." << endl;

/*
A new group of int values is needed since the one(s) used so far is / are not enough to represent this mpq-class value.

*/
			int nextGroupsIndex = preciseCoordinates->Size();
			if (nextGroupsIndex == index)
					nextGroupsIndex++;
			preciseCoordinates->Put(index, nextGroupsIndex);
			if (MR2_DEBUG)
				cerr << "Index of next group is " 
				<< nextGroupsIndex << endl;
			index = nextGroupsIndex;
			preciseCoordinates->resize(
				preciseCoordinates->Size() + gmpCharNum);
		}

		if (theStream.good())
		{
		    if (MR2_DEBUG)
			cerr << "Putting fey in the array in loop : " 
				<< i << ", value is " << nextChar << endl;
		    preciseCoordinates->Put(index, (int)nextChar);
		    index++;
		}
		else
		{
			feyNumOfChars = i;
			for (int j = index; j % gmpCharNum > 0; j++)
			{
				preciseCoordinates->Put(j, -1);
			}
		}
	}
}

/*
1.1.1 Read access methods

All these methods fetch the chars representing the given coordinate from the
DbArray using the indices given in this instance's private attributes, and
convert them to the correct instance of type mpq-class, representing the value
of the given coordinate.

*/

mpq_class PreciseMSegmentData::GetInitialStartX(const 
		DbArray<int>* preciseCoordinates) const {

	if (MR2_DEBUG)
		cerr << "PreciseMSegmentData::GetInitialStartX(),"
			<< " isxNumOfChars: " << isxNumOfChars
			<< ", isxStartPos: " << isxStartPos 
			<< ", array size: " 
			<< preciseCoordinates->Size() << endl;

	stringstream theStream;
	mpq_class theValue(0);

	int index = isxStartPos;
	if (index == -1) return theValue;

	if (MR2_DEBUG)
		cerr << "Reading int values (chars) from DbArray: ";

	for (int i = 0; i < isxNumOfChars; i++)
	{
		if (i > 0 && i % gmpCharNum == gmpCharNum-1)
		{
			int nextIndex;
			preciseCoordinates->Get(index, nextIndex);
			index = nextIndex;
		}
		char nextChar;
		int nextInt;
		preciseCoordinates->Get(index, nextInt);
		nextChar = static_cast<char>(nextInt);

		if (MR2_DEBUG)
			cerr << nextInt << "(" << nextChar << "), ";

		theStream.put(nextChar);
		index++;
	}

	if (MR2_DEBUG)
		cerr << "putting now in gmp return value." << endl;

	theStream >> theValue;

	if (MR2_DEBUG)
		cerr << "Return value is " << theValue << endl;

	return theValue;
}

mpq_class PreciseMSegmentData::GetInitialStartY(const 
		DbArray<int>* preciseCoordinates) const {
	stringstream theStream;
	mpq_class theValue(0);

	int index = isyStartPos;
	if (index == -1) return theValue;

	for (int i = 0; i < isyNumOfChars; i++)
	{
		if (i > 0 && i % gmpCharNum == gmpCharNum-1)
		{
			int nextIndex;
			preciseCoordinates->Get(index, nextIndex);
			index = nextIndex;
		}
		char nextChar;
		int nextInt;
		preciseCoordinates->Get(index, nextInt);
		nextChar = static_cast<char>(nextInt);
		theStream.put(nextChar);
		index++;
	}

	theStream >> theValue;
	return theValue;
}

mpq_class PreciseMSegmentData::GetInitialEndX(const 
		DbArray<int>* preciseCoordinates) const {
	stringstream theStream;
	mpq_class theValue(0);

	int index = iexStartPos;
	if (index == -1) return theValue;

	for (int i = 0; i < iexNumOfChars; i++)
	{
		if (i > 0 && i % gmpCharNum == gmpCharNum-1)
		{
			int nextIndex;
			preciseCoordinates->Get(index, nextIndex);
			index = nextIndex;
		}
		char nextChar;
		int nextInt;
		preciseCoordinates->Get(index, nextInt);
		nextChar = static_cast<char>(nextInt);
		theStream.put(nextChar);
		index++;
	}

	theStream >> theValue;
	return theValue;
}

mpq_class PreciseMSegmentData::GetInitialEndY(const 
		DbArray<int>* preciseCoordinates) const {
	stringstream theStream;
	mpq_class theValue(0);

	int index = ieyStartPos;
	if (index == -1) return theValue;

	for (int i = 0; i < ieyNumOfChars; i++)
	{
		if (i > 0 && i % gmpCharNum == gmpCharNum-1)
		{
			int nextIndex;
			preciseCoordinates->Get(index, nextIndex);
			index = nextIndex;
		}
		char nextChar;
		int nextInt;
		preciseCoordinates->Get(index, nextInt);
		nextChar = static_cast<char>(nextInt);
		theStream.put(nextChar);
		index++;
	}

	theStream >> theValue;
	return theValue;
}

mpq_class PreciseMSegmentData::GetFinalStartX(const 
		DbArray<int>* preciseCoordinates) const {
	stringstream theStream;
	mpq_class theValue(0);

	int index = fsxStartPos;
	if (index == -1) return theValue;

	for (int i = 0; i < fsxNumOfChars; i++)
	{
		if (i > 0 && i % gmpCharNum == gmpCharNum-1)
		{
			int nextIndex;
			preciseCoordinates->Get(index, nextIndex);
			index = nextIndex;
		}
		char nextChar;
		int nextInt;
		preciseCoordinates->Get(index, nextInt);
		nextChar = static_cast<char>(nextInt);
		theStream.put(nextChar);
		index++;
	}

	theStream >> theValue;
	return theValue;
}

mpq_class PreciseMSegmentData::GetFinalStartY(const 
		DbArray<int>* preciseCoordinates) const {
	stringstream theStream;
	mpq_class theValue(0);

	int index = fsyStartPos;
	if (index == -1) return theValue;

	for (int i = 0; i < fsyNumOfChars; i++)
	{
		if (i > 0 && i % gmpCharNum == gmpCharNum-1)
		{
			int nextIndex;
			preciseCoordinates->Get(index, nextIndex);
			index = nextIndex;
		}
		char nextChar;
		int nextInt;
		preciseCoordinates->Get(index, nextInt);
		nextChar = static_cast<char>(nextInt);
		theStream.put(nextChar);
		index++;
	}

	theStream >> theValue;
	return theValue;
}

mpq_class PreciseMSegmentData::GetFinalEndX(const 
		DbArray<int>* preciseCoordinates) const {
	stringstream theStream;
	mpq_class theValue(0);

	int index = fexStartPos;
	if (index == -1) return theValue;

	for (int i = 0; i < fexNumOfChars; i++)
	{
		if (i > 0 && i % gmpCharNum == gmpCharNum-1)
		{
			int nextIndex;
			preciseCoordinates->Get(index, nextIndex);
			index = nextIndex;
		}
		char nextChar;
		int nextInt;
		preciseCoordinates->Get(index, nextInt);
		nextChar = static_cast<char>(nextInt);
		theStream.put(nextChar);
		index++;
	}

	theStream >> theValue;
	return theValue;
}

mpq_class PreciseMSegmentData::GetFinalEndY(const 
		DbArray<int>* preciseCoordinates) const {
	stringstream theStream;
	mpq_class theValue(0);

	int index = feyStartPos;
	if (index == -1) return theValue;

	for (int i = 0; i < feyNumOfChars; i++)
	{
		if (i > 0 && i % gmpCharNum == gmpCharNum-1)
		{
			int nextIndex;
			preciseCoordinates->Get(index, nextIndex);
			index = nextIndex;
		}
		char nextChar;
		int nextInt;
		preciseCoordinates->Get(index, nextInt);
		nextChar = static_cast<char>(nextInt);
		theStream.put(nextChar);
		index++;
	}

	theStream >> theValue;
	return theValue;
}

/*
1.1 Class ~PreciseInterval~

This class is used to represent the precise time intervals for region units.

1.1.1 Class definition

The class definition has been moved to ~MovingRegion2Algebra.h~.

1.1.1 Constructors

The integer parameters are taken to initialize the array indices. Values
-1 mean the indices will be set to the arrays size, the new element will
be attached at the end of the array.

*/

PreciseInterval::PreciseInterval(int startPos, int endPos, 
	int startNum, int endNum) :
		startStartPos(startPos),
		endStartPos(endPos),
		startNumOfChars(startNum),
		endNumOfChars(endNum) {}

PreciseInterval::PreciseInterval(int pos) :
		startStartPos(pos),
		endStartPos(pos),
		startNumOfChars(0),
		endNumOfChars(0) {}

/*
1.1.1 Attribute read and write access methods

The internal functionality of these access methods is just like of those
from class PreciseMSegmentData. Private attributes are used as indices to the
given char arrays to retrieve the objects, and when object values are updated,
the indices are updated as well.

*/

mpq_class PreciseInterval::GetPreciseInitialInstant (
	const DbArray<int>* preciseInstants)
{
    if (MR2_DEBUG)
    cerr << "PreciseInterval::GetPreciseInitialInstant() called"
	 << endl
	 << "StartPos is " << startStartPos
	 << endl
	 << "Size of Array is " << preciseInstants->Size()
	 << endl;

	 stringstream theStream;
	 mpq_class theValue(0);

	 int index = startStartPos;

	 if (index == -1) return theValue;
	 for (int i = 0; i < startNumOfChars; i++)
	 {
		if (i > 0 && i % gmpCharNum == gmpCharNum-1)
		{
			int nextIndex;
			preciseInstants->Get(index, nextIndex);
			index = nextIndex;
		}
		char nextChar;
		int nextInt;
		preciseInstants->Get(index, nextInt);
		nextChar = static_cast<char>(nextInt);
		theStream.put(nextChar);
		index++;
	}

	theStream >> theValue;
	return theValue;
}

mpq_class PreciseInterval::GetPreciseFinalInstant 
	(const DbArray<int>* preciseInstants)
{
	stringstream theStream;
	mpq_class theValue(0);

	int index = endStartPos;
	if (index == -1) return theValue;

	for (int i = 0; i < endNumOfChars; i++)
	{
		if (i > 0 && i % gmpCharNum == gmpCharNum-1)
		{
			int nextIndex;
			preciseInstants->Get(index, nextIndex);
			index = nextIndex;
		}
		char nextChar;
		int nextInt;
		preciseInstants->Get(index, nextInt);
		nextChar = static_cast<char>(nextInt);
		theStream.put(nextChar);
		index++;
	}

	theStream >> theValue;
	return theValue;
}

void PreciseInterval::SetPreciseInitialInstant (
	mpq_class initial, DbArray<int>* preciseInstants)
{
    if (MR2_DEBUG)
	cerr << "PreciseInterval::SetPreciseInitialInstant()"
	     << " with start value: " << initial << endl;

	stringstream theStream;
	theStream << initial;
	if (startStartPos == -1) startStartPos = 
		preciseInstants->Size();
	int index = startStartPos;
	if (index >= preciseInstants->Size())
	{
		preciseInstants->resize
			(preciseInstants->Size() + gmpCharNum);
	}
	for (int i = 0; theStream.good(); i++)
	{
		char nextChar;
		theStream.get(nextChar);
		if (i > 0 && i % gmpCharNum == (gmpCharNum - 1))
		{
			if (MR2_DEBUG)
			    cerr << "resizing array..." << endl;

/*
A new group of int values is needed since the one(s) used so far is / are not enough to represent this mpq\_class value.

*/
			int nextGroupsIndex = 
				preciseInstants->Size();
			if (nextGroupsIndex == index)
				nextGroupsIndex++;
			preciseInstants->Put(
				index, nextGroupsIndex);
			if (MR2_DEBUG)
			cerr << "Index of next group is " 
			<< nextGroupsIndex << endl;

			index = nextGroupsIndex;
			preciseInstants->resize(
			    preciseInstants->Size() + gmpCharNum);
		}

		if (theStream.good())
		{
		    if (MR2_DEBUG)
			cerr << "Putting start in the array "
			<< "in loop : " << i 
			<< ", value is " << nextChar << endl;
		    preciseInstants->Put(index, (int)nextChar);
		    index++;
		}
		else
		{
			startNumOfChars = i;
			for (int j = index; j % gmpCharNum > 0; j++)
			{
				preciseInstants->Put(j, -1);
			}
		}
	}
	if (MR2_DEBUG)
		cerr << "Start position for this value is " 
		<< startStartPos
		<< ", array size is " << preciseInstants->Size() 
		<< endl;
}

void PreciseInterval::SetPreciseFinalInstant (mpq_class final, 
		DbArray<int>* preciseInstants)
{
	if (MR2_DEBUG)
	cerr << "PreciseInterval::SetPreciseFinalInstant() "
	<< "with end value " << final << endl;

	stringstream theStream;
	theStream << final;
	if (endStartPos == -1) endStartPos = 
		preciseInstants->Size();
	int index = endStartPos;
	if (index >= preciseInstants->Size())
	{
		preciseInstants->resize(
			preciseInstants->Size() + gmpCharNum);
	}
	for (int i = 0; theStream.good(); i++)
	{
		char nextChar;
		theStream.get(nextChar);
		if (i > 0 && i % gmpCharNum == (gmpCharNum - 1))
		{
			if (MR2_DEBUG)
				cerr << "resizing array..." << endl;
/*
A new group of chars is needed since the one(s) used so far is / are not enough to represent this mpq\_class value.

*/
			int nextGroupsIndex = preciseInstants->Size();
			if (nextGroupsIndex == index)
					nextGroupsIndex++;
			preciseInstants->Put(index, nextGroupsIndex);
			if (MR2_DEBUG)
				cerr << "Index of next group is " 
				<< nextGroupsIndex << endl;
			index = nextGroupsIndex;
			preciseInstants->resize(preciseInstants->Size() 
				+ gmpCharNum);
		}

		if (theStream.good())
		{
		    if (MR2_DEBUG)
			cerr << "Putting end in the array in loop : " 
				<< i << ", value is " << nextChar << endl;
		    preciseInstants->Put(index, (int)nextChar);
		    index++;
		}
		else
		{
			endNumOfChars = i;
			for (int j = index; j % gmpCharNum > 0; j++)
			{
				preciseInstants->Put(j, -1);
			}
		}
	}
	if (MR2_DEBUG)
		cerr << "Start position for this value is " 
			<< endStartPos
			<< ", array size is " << preciseInstants->Size() 
			<< endl;
}

/*
1 Data type ~iregion2~

1.1 Class ~IRegion2~

1.1.1 Class definition

The class definition has been moved to ~MovingRegion2Algebra.h~.

1.1.1 Constructors

*/
IRegion2::IRegion2(bool dummy): Intime<Region>(0) {
    if (MR2_DEBUG) cerr << "IRegion2::IRegion2() #2 called" << endl;

/*
This is quite ugly and may not work with other compilers than gcc.
Since the ~Intime<Alpha>()~ constructors do not properly initialise their
~value~ attribute (which if of type ~Region~ in this case), there is
no better solution right now to assure that ~value~ has a valid DBArray.

*/
    /*Region* tmp = new Region(0);
    memcpy(&value, tmp, sizeof(*tmp));
    delete(tmp);*/
    value.SetEmpty();
    value.SetDefined(true);
    SetDefined(true);
}

IRegion2::IRegion2(const IRegion2& ir) : Intime<Region>(0) {
    if (MR2_DEBUG) cerr << "IRegion2::IRegion2() #2 called" << endl;

    instant = ir.instant;
    del.isDefined = ir.del.isDefined;

/*
This is quite ugly and may not work with other compilers than gcc.
Since the ~Intime<Alpha>()~ constructors do not properly initialise their
~value~ attribute (which if of type ~Region~ in this case), there is
no better solution right now to assure that ~value~ has a valid DBArray.

*/
    /*Region* tmp = new Region(0);
    memcpy(&value, tmp, sizeof(*tmp));
    delete(tmp);*/
    value.SetEmpty();
    if (ir.del.isDefined) value.CopyFrom(&ir.value);
}

/*
1.1.1 Methods for algebra integration

1.1.1.1 Method ~Clone()~

*/
IRegion2* IRegion2::Clone(void) const {
    if (MR2_DEBUG) cerr << "IRegion2::Clone() called" << endl;

    return new IRegion2(*this);
}

/*
1.1.1.1 ~DBArray~ access

*/
int IRegion2::NumOfFLOBs(void) const {
    if (MR2_DEBUG) cerr << "IRegion2::NumOfFLOBs() called" << endl;

    return 1;
}

Flob* IRegion2::GetFLOB(const int i) {
    if (MR2_DEBUG) cerr << "IRegion2::GetFLOB() called" << endl;

    assert(i == 0);
    return value.GetFLOB(0);
}

/*
1.1 Algebra integration

1.1.1 Function ~IRegion2Property()~

*/
static ListExpr IRegion2Property() {
    if (MR2_DEBUG) cerr << "IRegion2Property() called" << endl;

    ListExpr example = nl->TextAtom();
    nl->AppendText(example,
                   "(\"2003-01-10\" ((((1.0 3.5)(2.0 5.5)(3.0 6.5)(4.0 6.5)"
                   "(4.0 5.5)(5.0 4.5)(5.0 2.5)(4.0 1.5)(3.0 1.5))"
                   "((2.0 3.0)(2.0 4.0)(3.0 4.0)(3.0 3.0)))))");
    return
        nl->TwoElemList(
            nl->FourElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List")),
            nl->FourElemList(
                nl->StringAtom("-> UNIT"),
                nl->StringAtom("(iregion2)"),
                nl->StringAtom("(<instant> <region>)"),
                example));
}

/*
1.1.1 Function ~CheckIRegion2()~

*/
static bool CheckIRegion2(ListExpr type, ListExpr& errorInfo) {
    if (MR2_DEBUG) cerr << "CheckIRegion2() called" << endl;

  return nl->IsEqual(type, IRegion2::BasicType())
      || nl->IsEqual(type, "intimeregion2"); // backward-compatibility!
}

/*
1.1.1 Function ~CreateIRegion2()~

*/
static Word CreateIRegion2(const ListExpr typeInfo) {
    if (MR2_DEBUG) cerr << "CreateIRegion2() called" << endl;

    return SetWord(new IRegion2(false));
}

/*
1.1.1 Type constructor ~intimeregion2~

*/
static TypeConstructor intimeregion2(
    IRegion2::BasicType(),
    IRegion2Property,
    OutIntime<Region, OutRegion>,
    InIntime<Region, InRegion>,
    0, 0, // SaveToList, RestoreFromList
    CreateIRegion2,
    DeleteIntime<Region>,
    OpenAttribute<Intime<Region> >,
    SaveAttribute<Intime<Region> >,  // object open and save
    CloseIntime<Region>,
    CloneIntime<Region>,
    CastIntime<Region>,
    SizeOfIntime<Region>,
    CheckIRegion2 );




/*
1 Class ~URegionEmb2~

1.1 Class definition

The class definition has been moved to ~MovingRegion2Algebra.h~.

1.1 Constructors

*/

URegionEmb2::URegionEmb2( const bool Defined ) :
    segmentsStartPos(0),
    segmentsNum(0),
    bbox(false)
    {
      if (MR2_DEBUG)
        cerr << "URegionEmb2::URegionEmb2(bool) called"
            << endl;
    }

URegionEmb2::URegionEmb2(const Interval<Instant>& tiv,
			const PreciseInterval& piv,
                       unsigned int pos) :
    segmentsStartPos(pos),
    segmentsNum(0),
    bbox(false),
    timeInterval(tiv),
    pInterval(piv){
    if (MR2_DEBUG)
        cerr << "URegionEmb2::URegionEmb2(tiv, piv, pos) called"
             << endl;
}

URegionEmb2::URegionEmb2(DbArray<MSegmentData2>* segments,
		DbArray<PreciseMSegmentData>* preciseSegments,
		DbArray<int>* preciseCoordinates,
		DbArray<int>* preciseInstants,
		const Interval<Instant>& tiv,
		PreciseInterval& piv,
		const URegionEmb& origUremb,
		const DbArray<MSegmentData>* origSegments,
		unsigned int pos,
		unsigned int scaleFactor) :
		segmentsStartPos(pos),
		segmentsNum(0),
		timeInterval(tiv),
		pInterval(piv){

	if (MR2_DEBUG) 
		cerr << "URegionEmb2::URegionEmb2 called "
			<< "with coarse uremb input"
			<< endl
			<< "Number of segments in orig uremb instance: "
			<< origUremb.GetSegmentsNum() << endl
			<< "Number of elements in the orig DbArray: "
			<< origSegments->Size() << endl
			<< "Start position is "
			<< origUremb.GetStartPos()
			<< endl;

	//Store precise part of time interval
	double rest_start =
			origUremb.timeInterval.start.ToDouble() -
			tiv.start.ToDouble();
	double rest_end =
			origUremb.timeInterval.end.ToDouble() -
			tiv.end.ToDouble();

	if (rest_start > 0)
	{
		int num = 1;
		int denom = 1;

		while (!nearlyEqual(((rest_start*denom) - num), 0)
				&& denom < 1000000000)
		{
			denom *= 10;
			num = broughtDown(rest_start * denom);
		}

		mpq_class prestS(num, denom);
		mpq_class testrest(rest_start);

		prestS.canonicalize();

		if (denom == 1000000000)
			pInterval.SetPreciseInitialInstant(
					testrest, preciseInstants);
		else
			pInterval.SetPreciseInitialInstant(
					prestS, preciseInstants);
	}

	if (rest_end > 0)
	{
		int num = 1;
		int denom = 1;

		while (!nearlyEqual(((rest_end*denom) - num), 0)
				&& denom < 1000000000)
		{
			denom *= 10;
			num = broughtDown(rest_end * denom);
		}

		mpq_class prestE(num, denom);
		mpq_class testrest(rest_end);

		prestE.canonicalize();

		if (denom == 1000000000)
			pInterval.SetPreciseFinalInstant(
					testrest, preciseInstants);
		else
			pInterval.SetPreciseFinalInstant(
					prestE, preciseInstants);
	}


	//Store all the segments
	for (int i = 0; i < origUremb.GetSegmentsNum(); i++)
	{
		if (MR2_DEBUG)
		cerr << "URegionEmb2::URegionEmb2() processing segment " 
			<< i
			<< ", which is segment number " 
			<< origUremb.GetStartPos() + i
			<< " in the DbArray from " 
			<< origSegments->Size() 
			<< " elements in total..." << endl;

		//Get the original MSegmentData and split coordinates
		MSegmentData seg;
		origUremb.GetSegment(origSegments, i, seg);
		MSegmentData segment(seg);

		if (MR2_DEBUG) cerr 
			<< "Successfully retrieved coarse segment number " 
			<< i << endl;

		PreciseMSegmentData preciseSegment(-1);
		MSegmentData2* auxDms;

		int isx = broughtDown(segment.GetInitialStartX() 
			* scaleFactor);
		double isxRest = segment.GetInitialStartX() 
			* scaleFactor - isx;
		int isy = broughtDown(segment.GetInitialStartY() 
			* scaleFactor);
		double isyRest = segment.GetInitialStartY() 
			* scaleFactor - isy;
		int iex = broughtDown(segment.GetInitialEndX() 
			* scaleFactor);
		double iexRest = segment.GetInitialEndX() 
			* scaleFactor - iex;
		int iey = broughtDown(segment.GetInitialEndY() 
			* scaleFactor);
		double ieyRest = segment.GetInitialEndY() 
			* scaleFactor - iey;
		int fsx = broughtDown(segment.GetFinalStartX() 
			* scaleFactor);
		double fsxRest = segment.GetFinalStartX() 
			* scaleFactor - fsx;
		int fsy = broughtDown(segment.GetFinalStartY() 
			* scaleFactor);
		double fsyRest = segment.GetFinalStartY() 
			* scaleFactor - fsy;
		int fex = broughtDown(segment.GetFinalEndX() 
			* scaleFactor);
		double fexRest = segment.GetFinalEndX() 
			* scaleFactor - fex;
		int fey = broughtDown(segment.GetFinalEndY() 
			* scaleFactor);
		double feyRest = segment.GetFinalEndY() 
			* scaleFactor - fey;

		bool isBasic = true;
		if (!nearlyEqual(isxRest, 0) 
			|| !nearlyEqual(isyRest, 0) 
			|| !nearlyEqual(iexRest, 0) 
			|| !nearlyEqual(ieyRest, 0)
			|| !nearlyEqual(fsxRest, 0) 
			|| !nearlyEqual(fsyRest, 0) 
			|| !nearlyEqual(fexRest, 0) 
			|| !nearlyEqual(feyRest, 0))
		{
			isBasic = false;
		}

		if (MR2_DEBUG)
		{
		cerr << "Calculating new segment with isBasic = " 
			<< isBasic
			<< endl
			<< "orig isx: " << segment.GetInitialStartX() 
			<< ", to integer grid: " << isx
			<< ", rest for precise representation: " 
			<< isxRest << ", scaleFactor was: " 
			<< scaleFactor
			<< endl
			<< "orig isy: " << segment.GetInitialStartY() 
			<< ", to integer grid: " << isy
			<< ", rest for precise representation: " 
			<< isyRest << ", scaleFactor was: " << scaleFactor
			<< endl
			<< "orig iex: " << segment.GetInitialEndX() 
			<< ", to integer grid: " << iex
			<< ", rest for precise representation: " 
			<< iexRest << ", scaleFactor was: " << scaleFactor
			<< endl
			<< "orig iey: " << segment.GetInitialEndY() 
			<< ", to integer grid: " << iey
			<< ", rest for precise representation: " 
			<< ieyRest << ", scaleFactor was: " << scaleFactor
			<< endl
			<< "orig fsx: " << segment.GetFinalStartX() 
			<< ", to integer grid: " << fsx
			<< ", rest for precise representation: " 
			<< fsxRest << ", scaleFactor was: " << scaleFactor
			<< endl
			<< "orig fsy: " << segment.GetFinalStartY() 
			<< ", to integer grid: " << fsy
			<< ", rest for precise representation: " 
			<< fsyRest << ", scaleFactor was: " << scaleFactor
			<< endl
			<< "orig fex: " << segment.GetFinalEndX() 
			<< ", to integer grid: " << fex
			<< ", rest for precise representation: " 
			<< fexRest << ", scaleFactor was: " << scaleFactor
			<< endl
			<< "orig fey: " << segment.GetFinalEndY() 
			<< ", to integer grid: " << fey
			<< ", rest for precise representation: " 
			<< feyRest << ", scaleFactor was: " << scaleFactor
			<< endl;
		}

/*
In the precise segment, only the difference between the absolute precise value and the grid cell edge is stored

*/
		mpq_class pisx(isxRest);
		mpq_class pisy(isyRest);
		mpq_class piex(iexRest);
		mpq_class piey(ieyRest);
		mpq_class pfsx(fsxRest);
		mpq_class pfsy(fsyRest);
		mpq_class pfex(fexRest);
		mpq_class pfey(feyRest);

		preciseSegment.SetInitialStartX(pisx, preciseCoordinates);
		preciseSegment.SetInitialStartY(pisy, preciseCoordinates);
		preciseSegment.SetInitialEndX(piex, preciseCoordinates);
		preciseSegment.SetInitialEndY(piey, preciseCoordinates);
		preciseSegment.SetFinalStartX(pfsx, preciseCoordinates);
		preciseSegment.SetFinalStartY(pfsy, preciseCoordinates);
		preciseSegment.SetFinalEndX(pfex, preciseCoordinates);
		preciseSegment.SetFinalEndY(pfey, preciseCoordinates);

		if (isBasic)
		{
			auxDms = new MSegmentData2(
					segment.GetFaceNo(),
					segment.GetCycleNo(),
					segment.GetSegmentNo(),
					segment.GetInsideAbove(),
					isx,
					isy,
					iex,
					iey,
					fsx,
					fsy,
					fex,
					fey);
		}
		else
		{
			auxDms = new MSegmentData2(
					segment.GetFaceNo(),
					segment.GetCycleNo(),
					segment.GetSegmentNo(),
					segment.GetInsideAbove(),
					isx,
					isy,
					iex,
					iey,
					fsx,
					fsy,
					fex,
					fey,
					preciseSegment,
					preciseCoordinates);
		}

		MSegmentData2 dms(auxDms);
		delete auxDms;

		dms.SetDegeneratedInitial(DGM_NONE);
		dms.SetDegeneratedFinal(DGM_NONE);

		//Store the segments in the DbArrays
		segments->Put(segmentsStartPos+segmentsNum, dms);
		preciseSegments->Put(
			segmentsStartPos+segmentsNum, preciseSegment);

		segmentsNum++;
	}
/*
The bbox is calculated on the integer grid, using the down left corner
of the respective cell for the minimum values and the upper right corner
for the maximum values.
We take the bbox values from the coarse region and round the values
accordingly.

*/

	const Rectangle<3> rbb = origUremb.BoundingBox();
	double min[3] = { broughtDown(rbb.MinD(0)),
					  broughtDown(rbb.MinD(1)),
					  timeInterval.start.ToDouble() };
	double max[3] = { roundedUp(rbb.MaxD(0)),
					  roundedUp(rbb.MaxD(1)),
					  timeInterval.end.ToDouble() };
	bbox.Set(true, min, max);


}


/*
1.1 Methods for database operators

1.1.1 Method ~Transform()~

*/
void URegionEmb2::Transform(double deltaX, double deltaY, 
		DbArray<MSegmentData2>* segments,
		DbArray<PreciseMSegmentData>* preciseSegments, 
		DbArray<int>* preciseCoordinates)
{
	if (MR2_DEBUG)
		cerr << "URegionEmb2::Transform() called" << endl;

/*
First transform double paramters into fractions in order to avoid
rounding errors.

*/
	int num = 1;
	int denom = 1;
	while (!nearlyEqual(((deltaX*denom) - num), 0) && denom < 1000000000)
	{
		denom *= 10;
		num = broughtDown(deltaX * denom);
	}
	mpq_class pDeltaX(num, denom);
	pDeltaX.canonicalize();

	if (MR2_DEBUG) cerr << "new deltaX: " << pDeltaX << endl;

	num = 1;
	denom = 1;
	while (!nearlyEqual(((deltaY*denom) - num), 0) && denom < 1000000000)
	{
		denom *= 10;
		num = broughtDown(deltaY * denom);
	}
	mpq_class pDeltaY(num, denom);
	pDeltaY.canonicalize();
	if (MR2_DEBUG) cerr << "new deltaY: " << pDeltaY << endl;

	for (int j = 0; j < this->GetSegmentsNum(); j++)
	{
		if (MR2_DEBUG)
			cerr << "Transforming segment number " << j 
			<< " by deltaX = " << pDeltaX <<", deltaY = " 
			<< pDeltaY << " " << endl;

		MSegmentData2 segment;
		this->GetSegment(segments, j, segment);

		PreciseMSegmentData psegment;
		this->GetPreciseSegment(preciseSegments, j, psegment);

		/*
		Calculate the new coordinates by adding the deltas.

		*/
		mpq_class newInitialStartX = 
			psegment.GetInitialStartX(preciseCoordinates) 
			+ segment.GetInitialStartX() + pDeltaX;
		mpq_class newInitialStartY = 
			psegment.GetInitialStartY(preciseCoordinates) 
			+ segment.GetInitialStartY() + pDeltaY;
		mpq_class newInitialEndX = 
			psegment.GetInitialEndX(preciseCoordinates) 
			+ segment.GetInitialEndX() + pDeltaX;
		mpq_class newInitialEndY = 
			psegment.GetInitialEndY(preciseCoordinates) 
			+ segment.GetInitialEndY() + pDeltaY;
		mpq_class newFinalStartX = 
			psegment.GetFinalStartX(preciseCoordinates) + 
			segment.GetFinalStartX() + pDeltaX;
		mpq_class newFinalStartY = 
			psegment.GetFinalStartY(preciseCoordinates) + 
			segment.GetFinalStartY() + pDeltaY;
		mpq_class newFinalEndX = 
			psegment.GetFinalEndX(preciseCoordinates) + 
			segment.GetFinalEndX() + pDeltaX;
		mpq_class newFinalEndY = 
			psegment.GetFinalEndY(preciseCoordinates) + 
			segment.GetFinalEndY() + pDeltaY;

		if (MR2_DEBUG)
			cerr << "New coordinates as double values: " 
			<< endl
			<< "(" << newInitialStartX.get_d() << ", " 
			<< newInitialStartY.get_d() << ", " 
			<< newInitialEndX.get_d()
			<< ", " << newInitialEndY.get_d() << ", " 
			<< newFinalStartX.get_d() << ", " 
			<< newFinalStartY.get_d() << ", "
			<< newFinalEndX.get_d() << ", " 
			<< newFinalEndY.get_d() << "): " 
			<< "Splitting now..." << endl
			<< "Integer grid (precise rest of) coordinates: (";

/*
Split the absolute precise coordinates in integer part and precise rest and
writes the resulting coordinates back to the db arrays.
Also reset the isBasicSegment flag and sets it accoring to new values.

*/
		segment.SetIsBasicSegment(true);
		int temp = broughtDown(newInitialStartX.get_d());
		segment.SetInitialStartX(temp);
		psegment.SetInitialStartX(
			newInitialStartX - temp, preciseCoordinates);
		if (newInitialStartX - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
			cerr << temp << "(" 
			<< newInitialStartX-temp << "), ";

		temp = broughtDown(newInitialStartY.get_d());
		segment.SetInitialStartY(temp);
		psegment.SetInitialStartY(
			newInitialStartY - temp, preciseCoordinates);
		if (newInitialStartY - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
			cerr << temp << "(" 
			<< newInitialStartY-temp << "), ";

		temp = broughtDown(newInitialEndX.get_d());
		segment.SetInitialEndX(temp);
		psegment.SetInitialEndX(
			newInitialEndX - temp, preciseCoordinates);
		if (newInitialEndX - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
			cerr << temp << "(" 
			<< newInitialEndX-temp << "), ";

		temp = broughtDown(newInitialEndY.get_d());
		segment.SetInitialEndY(temp);
		psegment.SetInitialEndY(
			newInitialEndY - temp, preciseCoordinates);
		if (newInitialEndY - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
			cerr << temp << "(" 
			<< newInitialEndY-temp << "), ";

		temp = broughtDown(newFinalStartX.get_d());
		segment.SetFinalStartX(temp);
		psegment.SetFinalStartX(
			newFinalStartX - temp, preciseCoordinates);
		if (newFinalStartX - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
			cerr << temp << "(" 
			<< newFinalStartX-temp << "), ";

		temp = broughtDown(newFinalStartY.get_d());
		segment.SetFinalStartY(temp);
		psegment.SetFinalStartY(
			newFinalStartY - temp, preciseCoordinates);
		if (newFinalStartY - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
			cerr << temp << "(" 
			<< newFinalStartY-temp << "), ";

		temp = broughtDown(newFinalEndX.get_d());
		segment.SetFinalEndX(temp);
		psegment.SetFinalEndX(
			newFinalEndX - temp, preciseCoordinates);
		if (newFinalEndX - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
			cerr << temp << "(" 
			<< newFinalEndX-temp << "), ";

		temp = broughtDown(newFinalEndY.get_d());
		segment.SetFinalEndY(temp);
		psegment.SetFinalEndY(
			newFinalEndY - temp, preciseCoordinates);
		if (newFinalEndY - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
			cerr << temp << "(" 
			<< newFinalEndY-temp << ")" << endl
			<< "New precise coordinates:" 
			<< endl << "PreciseInitialStartX: "
			<< psegment.GetInitialStartX(preciseCoordinates) 
			<< endl
			<< "PreciseInitialStartY: " 
			<< psegment.GetInitialStartY(preciseCoordinates) 
			<< "usw." << endl;

		//Write the segments back
		this->PutSegment(segments, j, segment, false);
		this->PutPreciseSegment(preciseSegments, j, psegment);
	}
}

/*
1.1.1 Method ~Scale()~

*/
void URegionEmb2::Scale(double deltaX, double deltaY, 
		DbArray<MSegmentData2>* segments,
		DbArray<PreciseMSegmentData>* preciseSegments, 
		DbArray<int>* preciseCoordinates)
{
	if (MR2_DEBUG)
		cerr << "URegionEmb2::Scale() called" << endl;

/*
First transform the double paramters into fractions in order to avoid
rounding errors.

*/
	int num = 1;
	int denom = 1;
	while (!nearlyEqual(((deltaX*denom) - num), 0))
	{
		denom *= 10;
		num = broughtDown(deltaX * denom);
	}
	mpq_class pDeltaX(num, denom);
	pDeltaX.canonicalize();
	if (MR2_DEBUG) cerr << "new deltaX: " << pDeltaX << endl;

	num = 1;
	denom = 1;
	while (!nearlyEqual(((deltaY*denom) - num), 0))
	{
		denom *= 10;
		num = broughtDown(deltaY * denom);
	}
	mpq_class pDeltaY(num, denom);
	pDeltaY.canonicalize();
	if (MR2_DEBUG) cerr << "new deltaY: " << pDeltaY << endl;

	for (int j = 0; j < this->GetSegmentsNum(); j++)
	{
		if (MR2_DEBUG)
			cerr << "Scaling segment " << j 
			<< ", factors are: " << pDeltaX << ", " 
			<< pDeltaY << " " << endl;

		MSegmentData2 segment;
		this->GetSegment(segments, j, segment);

		PreciseMSegmentData psegment;
		this->GetPreciseSegment(preciseSegments, j, psegment);

		/*
		Calculate the new coordinates (precise calculation)

		*/
		mpq_class newInitialStartX = 
			(psegment.GetInitialStartX(preciseCoordinates) + 
			segment.GetInitialStartX()) * pDeltaX;
		mpq_class newInitialStartY = 
			(psegment.GetInitialStartY(preciseCoordinates) + 
			segment.GetInitialStartY()) * pDeltaY;
		mpq_class newInitialEndX = 
			(psegment.GetInitialEndX(preciseCoordinates) + 
			segment.GetInitialEndX()) * pDeltaX;
		mpq_class newInitialEndY = 
			(psegment.GetInitialEndY(preciseCoordinates) + 
			segment.GetInitialEndY()) * pDeltaY;
		mpq_class newFinalStartX = 
			(psegment.GetFinalStartX(preciseCoordinates) + 
			segment.GetFinalStartX()) * pDeltaX;
		mpq_class newFinalStartY = 
			(psegment.GetFinalStartY(preciseCoordinates) + 
			segment.GetFinalStartY()) * pDeltaY;
		mpq_class newFinalEndX = 
			(psegment.GetFinalEndX(preciseCoordinates) + 
			segment.GetFinalEndX()) * pDeltaX;
		mpq_class newFinalEndY = 
			(psegment.GetFinalEndY(preciseCoordinates) + 
			segment.GetFinalEndY()) * pDeltaY;

		if (MR2_DEBUG)
			cerr << "New coordinates as double values: " 
			<< endl
			<< "(" << newInitialStartX.get_d() << ", " 
			<< newInitialStartY.get_d() << ", " 
			<< newInitialEndX.get_d()
			<< ", " << newInitialEndY.get_d() << ", " 
			<< newFinalStartX.get_d() << ", " 
			<< newFinalStartY.get_d() << ", "
			<< newFinalEndX.get_d() << ", " 
			<< newFinalEndY.get_d() << "): " 
			<< "Splitting now..." << endl
			<< "Integer grid (precise rest of) coordinates: (";

/*
Split results in integer part and precise rest.
Also reset flag isBasicSegment.

*/
		segment.SetIsBasicSegment(true);

		int temp = broughtDown(newInitialStartX.get_d());
		segment.SetInitialStartX(temp);
		psegment.SetInitialStartX(
			newInitialStartX - temp, preciseCoordinates);
		if (newInitialStartX - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
		cerr << temp << "(" << newInitialStartX-temp << "), ";

		temp = broughtDown(newInitialStartY.get_d());
		segment.SetInitialStartY(temp);
		psegment.SetInitialStartY(
			newInitialStartY - temp, preciseCoordinates);
		if (newInitialStartY - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
		cerr << temp << "(" << newInitialStartY-temp << "), ";

		temp = broughtDown(newInitialEndX.get_d());
		segment.SetInitialEndX(temp);
		psegment.SetInitialEndX(
			newInitialEndX - temp, preciseCoordinates);
		if (newInitialEndX - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
		cerr << temp << "(" << newInitialEndX-temp << "), ";

		temp = broughtDown(newInitialEndY.get_d());
		segment.SetInitialEndY(temp);
		psegment.SetInitialEndY(
			newInitialEndY - temp, preciseCoordinates);
		if (newInitialEndY - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
		cerr << temp << "(" << newInitialEndY-temp << "), ";

		temp = broughtDown(newFinalStartX.get_d());
		segment.SetFinalStartX(temp);
		psegment.SetFinalStartX(
			newFinalStartX - temp, preciseCoordinates);
		if (newFinalStartX - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
		cerr << temp << "(" << newFinalStartX-temp << "), ";

		temp = broughtDown(newFinalStartY.get_d());
		segment.SetFinalStartY(temp);
		psegment.SetFinalStartY(
			newFinalStartY - temp, preciseCoordinates);
		if (newFinalStartY - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
		cerr << temp << "(" << newFinalStartY-temp << "), ";

		temp = broughtDown(newFinalEndX.get_d());
		segment.SetFinalEndX(temp);
		psegment.SetFinalEndX(
			newFinalEndX - temp, preciseCoordinates);
		if (newFinalEndX - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
		cerr << temp << "(" << newFinalEndX-temp << "), ";

		temp = broughtDown(newFinalEndY.get_d());
		segment.SetFinalEndY(temp);
		psegment.SetFinalEndY(
			newFinalEndY - temp, preciseCoordinates);
		if (newFinalEndY - temp != 0) 
			segment.SetIsBasicSegment(false);

		if (MR2_DEBUG)
		cerr << temp << "(" << newFinalEndY-temp << ")" 
		<< endl
		<< "New precise coordinates:" << endl 
		<< "PreciseInitialStartX: "
		<< psegment.GetInitialStartX(preciseCoordinates) 
		<< endl
		<< "PreciseInitialStartY: " 
		<< psegment.GetInitialStartY(preciseCoordinates) 
		<< "usw." << endl;



		//Write the segments back
		this->PutSegment(segments, j, segment, false);
		this->PutPreciseSegment(preciseSegments, j, psegment);

		if (MR2_DEBUG)
		{
		    PreciseMSegmentData testsegment;
		    this->GetPreciseSegment(
		    preciseSegments, j, testsegment);
		    cerr << "Precise coordinates restored from DbArray: "
		    << testsegment.GetInitialStartX(preciseCoordinates) 
		    << " as example, isx" << endl;
		}


	}
}



/*
1.1.1 Method ~TemporalFunction()~

This method calculates the resulting Region instance by evaluating the
URegionEmb2 instance at one single instant t.
The coordinates are first calculated at full precision. As region uses
double coordinates, the result is then transformed to double by
ignoring the precise part of the coordinates and multiplying the integer
part with scaleFactor.

Note that because of ignoring the precise parts, the result might not be
a valid region instance in all cases. Therefor ~res~ might be not defined
at the end in spite of valid parameters.

*/
void URegionEmb2::TemporalFunction(
    const DbArray<MSegmentData2>* segments,
    const DbArray<PreciseMSegmentData>* preciseSegments,
    const DbArray<int>* preciseCoordinates,
    const DbArray<int>* preciseInstants,
    const Instant& t,
    const mpq_class& preciseInstant,
    const int scaleFactor,
    Region& res,
    bool ignoreLimits ) const {

    if (MR2_DEBUG){
        cerr << "URegionEmb2::TemporalFunction() called" << endl;
    }
/*
Calculate segments at specified instant using full precision in all
three dimensions.
Coarse the resulting segments by simply ignoring the precise part of
each coordinate, after multiplication with scaleFactor.
Remove degenerated segments of initial or final instant, when they
are not border of any region, and try to create region. If the result
is no valid region instance, ~res~ is set to not defined.

*/


    PreciseInterval pInt = pInterval;
    mpq_class pt0 = pInt.GetPreciseInitialInstant(preciseInstants) + 
		timeInterval.start.ToDouble();
    mpq_class pt1 = pInt.GetPreciseFinalInstant(preciseInstants) + 
		timeInterval.end.ToDouble();

    mpq_class pt;
    bool instantContained = false;

    assert(t.IsDefined());

/*
We now check if the given instant is contained in the time interval.
If this is not the case the result value is set to not defined.

*/
    if (t.IsDefined())
    {
    	pt = t.ToDouble() + preciseInstant;

    	if (MR2_DEBUG)
    	    cerr << "Werte für check von instantContained: "
    	    << endl
    	    << "pt0, pt, pt1: "
    	    << pt0 << ", " << pt << ", " << pt1
    	    << endl
    	    << "Damit ergibt preciseBetween(pt0, pt, pt1) "
    	    << preciseBetween(pt0, pt, pt1)
    	    << endl
    	    << "und cmp(pt0, pt) " << cmp(pt0, pt)
    	    << endl
    	    << "und cmp(pt1, pt) " << cmp(pt1, pt)
    	    << endl;

    	instantContained = (preciseBetween(pt0, pt, pt1)
    		|| (((cmp(pt0, pt) == 0 && timeInterval.lc) 
		|| (cmp(pt1, pt) == 0 && timeInterval.rc))));

    	if (!instantContained && !ignoreLimits)
    	{
    		if (MR2_DEBUG)
    		    cerr << "instant not contained in interval" << endl;
    		res.SetDefined(false);
    		return;
    	}
    }

    assert(ignoreLimits || instantContained);

    res.Clear();
    if(!t.IsDefined() || !(ignoreLimits || instantContained) ) {
      res.SetDefined( false );
      return;
    }
    res.SetDefined( true );

    bool initialInstant = cmp(pt0, pt) == 0;
    bool finalInstant = cmp(pt1, pt) == 0;

    mpq_class f;

    if (cmp(pt0, pt1) == 0)
    {
    	f = 0;
    }
    else
    {
    	f = (pt - pt0)/(pt1 - pt0);
    }

    int partnerno = 0;

    res.StartBulkLoad();

    for (unsigned int i = 0; i < segmentsNum; i++) {
            if (MR2_DEBUG){
                cerr << "URegionEmb2::TemporalFunction() segment #"
                     << i
                     << "/"
                     << segmentsNum
                     << " ("
                     << segmentsStartPos
                     << ")"
                     << endl;
            }
            MSegmentData2 dms;
            segments->Get(segmentsStartPos+i, &dms);

            PreciseMSegmentData pdms;
            preciseSegments->Get(segmentsStartPos+i, &pdms);

            mpq_class pxs =
                dms.GetInitialStartX() + 
			pdms.GetInitialStartX(preciseCoordinates)
                + (dms.GetFinalStartX() + 
			pdms.GetFinalStartX(preciseCoordinates) -
                	(dms.GetInitialStartX() + 
			pdms.GetInitialStartX(preciseCoordinates)))*f;
            mpq_class pys =
                dms.GetInitialStartY() + 
			pdms.GetInitialStartY(preciseCoordinates)
                + (dms.GetFinalStartY() + 
			pdms.GetFinalStartY(preciseCoordinates) -
                	(dms.GetInitialStartY() + 
			pdms.GetInitialStartY(preciseCoordinates)))*f;
            mpq_class pxe =
                dms.GetInitialEndX() + 
			pdms.GetInitialEndX(preciseCoordinates)
                + (dms.GetFinalEndX() + 
			pdms.GetFinalEndX(preciseCoordinates) -
                	(dms.GetInitialEndX() + 
			pdms.GetInitialEndX(preciseCoordinates)))*f;
            mpq_class pye =
                dms.GetInitialEndY() + 
			pdms.GetInitialEndY(preciseCoordinates)
                + (dms.GetFinalEndY() + 
			pdms.GetFinalEndY(preciseCoordinates) -
                	(dms.GetInitialEndY() + 
			pdms.GetInitialEndY(preciseCoordinates)))*f;

            if (MR2_DEBUG){
            	cerr << "URegionEmb2::TemporalFunction() "
		<< "next precise segment is ("
            	<< pxs << ", " << pys << ", " << pxe << ", " 
		<< pye << ")" << endl;
            	cerr << "Scaling now with scale factor " << scaleFactor 
		<< " and converting to double values." << endl;
            }

            pxs = pxs / scaleFactor;
            pys = pys / scaleFactor;
            pxe = pxe / scaleFactor;
            pye = pye / scaleFactor;

            double xs = pxs.get_d();
            double ys = pys.get_d();
            double xe = pxe.get_d();
            double ye = pye.get_d();

            if (MR2_DEBUG){
                cerr << "URegionEmb2::TemporalFunction() "
		     << "next value is ("
                     << xs << ", " << ys << ", " << xe << ", " 
		     << ye << ")"
                     << endl;
            }

            if (nearlyEqual(xs, xe) && nearlyEqual(ys, ye)) {
                if (MR2_DEBUG){
                    cerr << "URegionEmb2::TemporalFunction() "
                         << "reduced to point"
                         << endl;
                }
                continue;
            }

            assert(dms.GetDegeneratedInitial() != DGM_UNKNOWN);
            assert(dms.GetDegeneratedFinal() != DGM_UNKNOWN);

            if ((initialInstant
                 && dms.GetDegeneratedInitial() == DGM_IGNORE)
                || (finalInstant
                    && dms.GetDegeneratedFinal() == DGM_IGNORE)) {
                if (MR2_DEBUG){
                    cerr << "URegionEmb2::TemporalFunction() "
                         << "ignored degenerated"
                         << endl;
                }
                continue;
            }

            Point s(true, xs, ys);
            Point e(true, xe, ye);

            if (AlmostEqual(s, e))
            {
            	cerr << endl
		<< "--------------------------------------------------"
		<< endl
		<< "URegionEmb2::TemporalFunction() " 			
		<< "halfsegment creation failure, both "
		<< "points almost equal."
		<< endl
		<< "No valid region instance can be created."
		<< endl
		<< "--------------------------------------------------"
		<< endl;
            	res.SetDefined(false);
            	return;
            }
            HalfSegment hs(true, s, e);

            hs.attr.faceno = 0;
            hs.attr.cycleno = 0;
            hs.attr.edgeno = partnerno;
            hs.attr.partnerno = partnerno++;

            if (initialInstant
                 && dms.GetDegeneratedInitial() == DGM_INSIDEABOVE){
              hs.attr.insideAbove = true;
            }else if (initialInstant
                 && dms.GetDegeneratedInitial() == DGM_NOTINSIDEABOVE){
              hs.attr.insideAbove = false;
            }else if (finalInstant
                 && dms.GetDegeneratedFinal() == DGM_INSIDEABOVE){
              hs.attr.insideAbove = true;
            }else if (finalInstant
                 && dms.GetDegeneratedFinal() == DGM_NOTINSIDEABOVE){
              hs.attr.insideAbove = false;
            }else{
              hs.attr.insideAbove = dms.GetInsideAbove();
            }

            if (MR2_DEBUG){
              cerr << "URegionEmb2::TemporalFunction()   "
                  << "Adding hs="
                  << hs
                  << endl;
            }
            if (!res.InsertOk(hs))
            {
            	if (MR2_DEBUG){
            	cerr << "URegionEmb2::TemporalFunction() "
		<< "adding hs failed! "
		<< "Ignoring hs="
		<< hs
		<< endl;
            	}

            }
            else {
		res += hs;
		hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
		if (MR2_DEBUG){
		  cerr << "URegionEmb2::TemporalFunction()   "
			  << "Adding hs="
			  << hs
			  << endl;
		}
		res += hs;
            }
    }

    //Try region calculation
    res.EndBulkLoad();

    if (!res.IsDefined())
    {
    	if (MR2_DEBUG)
    	{
    		cerr << "URegionEmb2::TemporalFunction() failed. "
    		<< "EndBulkLoad method did not return a valid region unit."
		<< endl;
    	}
    	res.SetDefined(false);
    	return;
    }

    if (MR2_DEBUG){
        for (int i = 0; i < res.Size(); i++) {
            HalfSegment hs;
            res.Get(i, hs);

            cerr << "URegionEmb2::TemporalFunction() segment #"
                 << i
                 << " lp=("
                 << hs.GetLeftPoint().GetX()
                 << ", "
                 << hs.GetLeftPoint().GetY()
                 << ") rp=("
                 << hs.GetRightPoint().GetX()
                 << ", "
                 << hs.GetRightPoint().GetY()
                 << ") ldp="
                 << hs.IsLeftDomPoint()
                 << " attr="
                 << hs.attr.faceno
                 << " "
                 << hs.attr.cycleno
                 << " "
                 << hs.attr.edgeno
                 << " "
                 << hs.attr.partnerno
                 << " "
                 << hs.attr.insideAbove
                 << endl;
        }
    }
}






/*
1.1 Methods required to act as unit in ~Mapping~ template

1.1.1 Method ~IsValid()~

*/
bool URegionEmb2::IsValid(void) const {
    return true;
}

/*
1.1.1 Operator ~==~

*/
bool URegionEmb2::operator==(const URegionEmb2& ur) const {
    return (timeInterval == ur.timeInterval);

}

/*
1.1.1 Method ~Before()~

*/
bool URegionEmb2::Before(const URegionEmb2& ur) const {
    return timeInterval.Before(ur.timeInterval);
}

/*
1.1.1 Method ~Compare()~

*/
bool URegionEmb2::Compare(const URegionEmb2* ur) const {
    assert(false);
    return false;
}

/*
1.1 Access methods

1.1.1 Method ~SetSegmentsNum()~

*/
void URegionEmb2::SetSegmentsNum(int i){
  segmentsNum=i;
}

/*
1.1.1 Method ~SetStartPos()~

*/
void URegionEmb2::SetStartPos(int i){
  segmentsStartPos=i;
}


/*
1.1.1 Method ~GetSegmentsNum()~

*/
int URegionEmb2::GetSegmentsNum(void) const {
    if (MR2_DEBUG)
        cerr << "URegionEmb2::GetSegmentsNum() called, num="
             << segmentsNum
             << endl;

    return segmentsNum;
}


/*
1.1.1 Method ~GetStartPos()~

*/
const int URegionEmb2::GetStartPos() const{
    if (MR2_DEBUG)
        cerr << "URegionEmb2::GetStartPos() called, sp="
             << segmentsStartPos
             << endl;

    return segmentsStartPos;
}



/*
1.1.1 Method ~BoundingBox()~

*/
const Rectangle<3> URegionEmb2::BoundingBox(const Geoid* geoid /*=0*/) const {
    if (MR2_DEBUG) cerr << "URegionEmb2::BoundingBox() called" << endl;
    if(geoid){
      cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
      << endl;
      assert( !geoid ); // TODO: implement spherical geometry case
    }
    return bbox;
}
/*
1.1.1 Assignment Operator

*/
URegionEmb2& URegionEmb2::operator=(const URegionEmb2& U) {
  segmentsStartPos = U.segmentsStartPos;
  segmentsNum = U.segmentsNum;
  bbox = U.bbox;
  timeInterval = U.timeInterval;
  return *this;
}

/*
1.1.1 Method ~GetSegment()~

*/
void URegionEmb2::GetSegment(
    const DbArray<MSegmentData2>* segments,
    int pos,
    MSegmentData2& dms) const {

	if (MR2_DEBUG)
	        cerr << "URegionEmb2::GetSegment() called, pos=" << pos
	             << ", segments="
	             << segments
	             << endl;

    segments->Get(segmentsStartPos+pos, dms);
}

/*
1.1.1 Method ~GetPreciseSegment()~

*/
void URegionEmb2::GetPreciseSegment(
		const DbArray<PreciseMSegmentData>* preciseSegments,
		int pos,
		PreciseMSegmentData& pdms) const {

	preciseSegments->Get(segmentsStartPos+pos, pdms);
}




/*
1.1.1 Method ~PutSegment()~

*/
void URegionEmb2::PutSegment(
    DbArray<MSegmentData2>* segments,
    int pos,
    const MSegmentData2& dms,
    const bool isNew) {

    if (MR2_DEBUG)
        cerr << "URegionEmb2::PutSegment() called, pos=" << pos <<
        " " << segmentsStartPos << dms.ToString() << endl;
    segments->Put(segmentsStartPos+pos, dms);
    if(isNew){
        segmentsNum++;
    }
}

/*
1.1.1 Method ~PutPreciseSegment()~

*/
void URegionEmb2::PutPreciseSegment(
	DbArray<PreciseMSegmentData>* segments,
	int pos,
	const PreciseMSegmentData& dms) {

	if (MR2_DEBUG)
		cerr << "UregionEmb2::PutPreciseSegment() called, pos= " 
			<< pos << endl;
	segments->Put(segmentsStartPos+pos, dms);
}

/*
1.1.1 Method ~PutCoordinate()~

*/
void URegionEmb2::PutCoordinate(
		DbArray<int>* coords,
		int pos,
		const int& c) {

	if (MR2_DEBUG)
		cerr << "URegionEmb2::PutCoordinate() called, pos= " 
			<< pos << endl;
	coords->Put(pos, c);
}


/*
1.1.1 Method ~SetSegmentInsideAbove()~

*/
void URegionEmb2::SetSegmentInsideAbove(
    DbArray<MSegmentData2>* segments,
    int pos,
    bool insideAbove) {

    if (MR2_DEBUG)
        cerr << "URegionEmb2::SetSegmentInsideAbove() called, segments="
             << segments
             << endl;

    MSegmentData2 auxDms;
    segments->Get(segmentsStartPos+pos, &auxDms);
    MSegmentData2 dms( auxDms );
    dms.SetInsideAbove(insideAbove);
    segments->Put(segmentsStartPos+pos, dms);
}




/*
1.1 In and out functions

1.1.1 Function ~InURegionEmbedded2()~

*/
static URegionEmb2* InURegionEmbedded2(
    const ListExpr instance,
    const int errorPos,
    ListExpr& errorInfo,
    DbArray<MSegmentData2>* segments,
    DbArray<PreciseMSegmentData>* preciseSegments,
    DbArray<int>* preciseCoordinates,
    DbArray<int>* preciseInstants,
    unsigned int segmentsStartPos) {

	if (MR2_DEBUG)
	        cerr << "InURegionEmbedded2() called, segmentsStartPos = "
	             << segmentsStartPos << endl;

/*
Please that ~Region~ creation is done as shown in ~SpatialAlgebra~.
See there for more details.

*/

		if (nl->ListLength(instance) <= 1) {
			cerr << "uregion not in format "
			<< "(<interval> <faces>)" 
			<< endl;
	        return 0;
	    }

		if (MR2_DEBUG)
		cerr << "InURegionEmbedded() (<interval> <faces>) found" 
			<< endl;

	    //Get Inner List representing the time interval
	    ListExpr interval = nl->First(instance);

/*
First we check the syntactical correctness of the list expression representing the time interval.
No syntax check will be done for the fifth part, which is the representation of
the precise time interval.
The syntax check of this part will be done later, and - if it fails - the part will be ignored
and a basic interval will be created instead.

*/

	    if (nl->ListLength(interval) != 5
	            || !nl->IsAtom(nl->First(interval))
	            || (nl->AtomType(nl->First(interval)) != StringType
	                && nl->AtomType(nl->First(interval)) != RealType
	                && nl->AtomType(nl->First(interval)) != IntType)
	            || !nl->IsAtom(nl->Second(interval))
	            || (nl->AtomType(nl->Second(interval)) != StringType
	                && nl->AtomType(nl->Second(interval)) != RealType
	                && nl->AtomType(nl->Second(interval)) != IntType)
	            || !nl->IsAtom(nl->Third(interval))
	            || nl->AtomType(nl->Third(interval)) != BoolType
	            || !nl->IsAtom(nl->Fourth(interval))
	            || nl->AtomType(nl->Fourth(interval)) != BoolType) {

	    		cerr << "uregion interval not in correct format "
	    	             << endl;
	            return 0;
        }


/*
Create first a basic interval (one in the integer grid). The part of the list
expression describing the time information may already be given as integer values,
but we cannot be sure about that. Therefor the components for start and end time
need to be split into an integer component and maybe some fraction that will later
be added to the precise time interval given in nl->fifth(interval).

*/
	    	    int int_start = 0;
	    	    int int_end = 0;
	    	    double rest_start = 0.0;
	    	    double rest_end = 0.0;
	    	    bool correct;
	    	    Instant *start;
	    	    Instant *end;

	    	    if (nl->AtomType(nl->First(interval)) == RealType)
	    	    {
	    	    	int_start = (int)nl->RealValue(
	    	    		nl->First(interval));
	    	    	rest_start = nl->RealValue(nl->First(interval)) 
				- int_start;
	    	    	ListExpr newStart =
	    	    		nl->RealAtom((double)int_start);
	    	    	start = (Instant *) InInstant(
	    	    		nl->TheEmptyList(),
	    	   	     newStart,
	    	    	     errorPos,
	    	    	     errorInfo,
	    	    	     correct).addr;
	    	    	if (!correct) {
	    	    	    if (!correct) {
	    	    		cerr << "uregion interval invalid start"
					<< " time" << endl;
	    	    		        return 0;
	    	    	    }
	    	    	    return 0;
	    	    	}
	    	    }
	    	    else if (nl->AtomType(nl->First(interval)) == IntType){
	    	    	int_start = nl->IntValue(nl->First(interval));
	    	    	ListExpr newStart =
	    	    		nl->RealAtom((double)int_start);
	    	    	start = (Instant *) InInstant(
	    	    		nl->TheEmptyList(),
	    	    		newStart,
	    	    		errorPos,
	    	    	        errorInfo,
	    	    	        correct).addr;
	    	    	if (!correct) {
	    	    	    if (!correct) {
	    	    		cerr << "uregion interval invalid start"
					<<" time" << endl;
	    	    		return 0;
	    	    	    }
	    	    		return 0;
	    	    	}
	    	    }
	    	    else {
	    	    	Instant *help = 
				(Instant *) InInstant(nl->TheEmptyList(),
	    	    			nl->First(interval),
	    	    			errorPos,
	    	    			errorInfo,
	    	    			correct).addr;
	    	    	int_start = broughtDown(help->ToDouble());
	    	    	rest_start = help->ToDouble() - int_start;
	    	    	ListExpr newStart =
	    	    		nl->RealAtom((double)int_start);
	    	    	start = (Instant *) InInstant(
	    	    		nl->TheEmptyList(),
	    	    			newStart,
	    	    			errorPos,
	    	    			errorInfo,
	    	    			correct).addr;
	    	    	delete help;
	    	    }

	    	    if (nl->AtomType(nl->Second(interval)) == RealType)
	    	    {
	    	    	int_end = (int)nl->RealValue(
	    	    		nl->Second(interval));
	    	    	rest_end = nl->RealValue(nl->Second(interval)) 
				- int_end;
	    	    	ListExpr newEnd = nl->IntAtom((double)int_end);
	    	    	end = (Instant *) InInstant(nl->TheEmptyList(),
	    	    		newEnd,
	    	    		errorPos,
	    	    		errorInfo,
	    	    	        correct ).addr;
	    	    	if (!correct) {
	    	    	    correct = false;
	    	    	    delete start;
	    	    	    cerr << "uregion interval invalid end time" 
				<< endl;
	    	    	    return 0;
	    	    	}
	    	    }
	    	    else if (nl->AtomType(nl->Second(interval)) == IntType){
	    	    	int_end = nl->IntValue(nl->Second(interval));
	    	    	ListExpr newEnd = nl->IntAtom((double)int_end);
	    	    	end = (Instant *) InInstant(nl->TheEmptyList(),
	    	    		newEnd,
	    	    		errorPos,
	    	    		errorInfo,
	    	    		correct ).addr;
	    	    	if (!correct) {
	    	    	    correct = false;
	    	    	    delete start;
	    	    	    cerr << "uregion interval invalid end time" 
				<< endl;
	    	    	    return 0;
	    	    	}
	    	    }
	    	    else {
	    	    	Instant *help = 
				(Instant *) InInstant(nl->TheEmptyList(),
	    	    		    	nl->Second(interval),
	    	    		    	errorPos,
	    	    		    	errorInfo,
	    	    		    	correct).addr;
	    	    	int_end = broughtDown(help->ToDouble());
	    	    	rest_end = help->ToDouble() - int_end;
	    	    	ListExpr new_end =
	    	    		nl->RealAtom((double)int_end);
	    	    	end = (Instant *) InInstant(
	    	    		nl->TheEmptyList(),
    	    			new_end,
    	    			errorPos,
    	    			errorInfo,
	    	    		correct).addr;
	    	    	delete help;
	    	    }


	            bool lc = nl->BoolValue(nl->Third(interval));
	            bool rc = nl->BoolValue(nl->Fourth(interval));

/*
Now create the precise interval. Don't forget to add precise information
obtained by splitting the first two components of the interval list.
After creation of the precise interval, the syntax of the complete time
interval can be checked.

*/

	            ListExpr preciseInterval = nl->Fifth(interval);
	            PreciseInterval preciseTimeInterval(-1);
	            preciseTimeInterval.SetPreciseInitialInstant(
			mpq_class(0), preciseInstants);
	            preciseTimeInterval.SetPreciseFinalInstant(
			mpq_class(0), preciseInstants);
	            bool hasPreciseRepresentation = true;
/*
Check if we have precise time interval information from the splitting before.

*/

	            if (rest_start > 0)
			{
				int num = 1;
				int denom = 1;

				while (!nearlyEqual(((rest_start*denom) -
					num), 0) && denom < 1000000000)
				{
				  denom *= 10;
				  num = broughtDown(rest_start * denom);
				}

				mpq_class prestS(num, denom);
				mpq_class testrest(rest_start);

				prestS.canonicalize();

				if (denom == 1000000000)
				  preciseTimeInterval.
					SetPreciseInitialInstant(
					testrest, preciseInstants);
				else
				  preciseTimeInterval.
					SetPreciseInitialInstant(
					prestS, preciseInstants);
			}

	            if (rest_end > 0)
	            {
	            	int num = 1;
			int denom = 1;
			while (!nearlyEqual(((rest_end*denom) - num), 0) 
				&& denom < 1000000000)
			{
				denom *= 10;
				num = broughtDown(rest_end * denom);
			}
			mpq_class prestE(num, denom);
			mpq_class testrest(rest_end);

			prestE.canonicalize();

			if (denom == 1000000000)
				preciseTimeInterval.SetPreciseFinalInstant
					(testrest, preciseInstants);
			else
    	    	preciseTimeInterval.
    	    	SetPreciseFinalInstant
				(prestE, preciseInstants);
	            }

/*
Check the syntax for the precise representation of the time interval,
and simply ignore it if it is not okay.

*/
	    		if (nl->ListLength(preciseInterval) != 2
	    			|| !nl->IsAtom(nl->First(preciseInterval))
	    			|| nl->AtomType(nl->First
					(preciseInterval)) != TextType
	    			|| !nl->IsAtom(nl->Second(preciseInterval))
	    			|| nl->AtomType(nl->Second
					(preciseInterval)) != TextType){
	    				
	    			hasPreciseRepresentation = false;
	    		}

	    		if (hasPreciseRepresentation){
    			  mpq_class pstart2;
    			  mpq_class pend2;
    			  try
    			  {
    				textTypeToGmpType(nl->First
					(preciseInterval), pstart2);
    				textTypeToGmpType(nl->Second
					(preciseInterval), pend2);
    			  }
    			  catch (invalid_argument& e)
    			  {
    				cerr << endl
    				<< "--------------------------------"
				<< "-------------------------------" << endl
    				<< "Checking time interval failed." << endl
    				<< e.what() << endl
    				<< "----------------------------------"
				<< "-----------------------------" << endl;
    				return 0;
    			  }

    			  preciseTimeInterval.SetPreciseInitialInstant(
    				preciseTimeInterval.GetPreciseInitialInstant
				(preciseInstants) +
    				pstart2, preciseInstants);
    			  preciseTimeInterval.SetPreciseFinalInstant(
    				preciseTimeInterval.GetPreciseFinalInstant
				(preciseInstants) +
    				pend2, preciseInstants);
	    		}

/*
Now we check the syntax of the time interval. If we don't have a correct interval
on the integer grid, we check if it is correct considering the precise part of it.

*/
	            if (end->ToDouble() <= start->ToDouble()) {

	            	if (MR2_DEBUG)
	            	    cerr << "Time interval on integer grid invalid"
	            	    << " or start and end equal"
	            	    << endl
	            	    << "Check with precise values..."
	            	    << endl
	            	    << "start to double is " << start->ToDouble()
	            	    << endl
	            	    << "end to double is " << end->ToDouble()
	            	    << endl;

	            	int compare_value =
	            	    cmp(preciseTimeInterval.
	            		GetPreciseFinalInstant(preciseInstants),
	            		preciseTimeInterval.
	            		GetPreciseInitialInstant(preciseInstants));

	            	if (MR2_DEBUG)
	            	    cerr << "Precise start is "
	            	    << preciseTimeInterval.
	            	    GetPreciseFinalInstant(preciseInstants)
	            	    << endl
	            	    << "Precise end is "
	            	    << preciseTimeInterval.
	            	    GetPreciseInitialInstant(preciseInstants)
	            	    << "cmp(preciseEnd, preciseStart) is "
	            	    << cmp(preciseTimeInterval.
	            	    GetPreciseFinalInstant(preciseInstants),
	            	    preciseTimeInterval.
	            	    GetPreciseInitialInstant(preciseInstants))
	            	    << endl;

	            	if (end->ToDouble() < start->ToDouble()
	            		|| (end->ToDouble() == start->ToDouble()
	            		&& compare_value < 0)
	            		|| (compare_value == 0
	            		&& end->ToDouble() == start->ToDouble()
	            		&& !(lc && rc)))
					{
			cerr << "uregion invalid interval"
						<< endl;
						delete start;
						delete end;
						return 0;
					}
	            }

	            Interval<Instant> tinterval(*start, *end, lc, rc);

	            DateTime  intervalLen = *end-*start;

	            delete start;
	            delete end;




/*
Create ~URegionEmb2~ instance and pass storage of segments, if we received
any.
A detailed syntax check is needed in order to be sure that a correct unit
will result. For that purpose, all the segments are checked and compared to each other:
So far this is done by inserting the segments into a region and relying on the
algorithms there for syntax check.
Note that this way region units that are correct just because of their precise
coordinate information will be deleted because of this syntax check fails!
This is only a temporarely solution!

*/

        URegionEmb2* uregion = new URegionEmb2(
		tinterval, preciseTimeInterval, segmentsStartPos);

        unsigned int faceno = 0;
        unsigned int partnerno = 0;
        ListExpr faces = nl->Second(instance);

        Region cr(0);
        cr.StartBulkLoad();

        if (nl->ListLength(faces) == 0) {
        	cerr << "uregion should contain at least one face" 
			<< endl;
            delete uregion;
            return 0;
        }

/*
... all this is similar to ~Region~ creation in ~SpatialAlgebra~...

*/
        while (!nl->IsEmpty(faces)) {
        	if (MR2_DEBUG)
        	            cerr << "InURegionEmbedded2() face #" 
				<< faceno << endl;
            ListExpr cycles = nl->First(faces); //first face

            if (nl->ListLength(cycles) == 0) {
            	cerr << "uregion face should contain at least one cycle"
            	                 << endl;
                delete uregion;
                return 0;
            }

            unsigned int cycleno = 0;
            unsigned int pointno = 0;

            while (!nl->IsEmpty(cycles)) {
            	if (MR2_DEBUG)
            	    cerr << "InURegionEmbedded2()   cycle #" 
			<< cycleno << endl;

/*
For each cycle a region is created and the segments are inserted to it
in order to get the direction information.

*/

            	Region rDir(0);
            	rDir.StartBulkLoad();

            	ListExpr cyclepoints = nl->First(cycles); //first cycle

            	if (nl->ListLength(cyclepoints) < 3) {
            	   cerr << "uregion cycle should contain at "
            	                        << "least three points"
            	                        << endl;
                   delete uregion;
                   return 0;
            	}

            	ListExpr firstPoint = nl->First(cyclepoints); 
			//the first point of this cycle
            	ListExpr prevPoint = 0;

            	unsigned int initialSegmentsNum =
                               uregion->GetSegmentsNum();

            	while (!nl->IsEmpty(cyclepoints)) {
            	   if (MR2_DEBUG)
                       cerr << "InURegionEmbedded2() point #"
                            << pointno
                            << endl;

                   ListExpr point = nl->First(cyclepoints);

/*
In the following, the segments will be added to the region unit.
In method ~AddSegment()~ some syntax checks will also be done. So far
this syntax check relies on the one of datatype Region, that means that
double values will be used for the check.
This can only be a temporarely solution because the syntax check might fail
in cases where the syntax is actually correct because of the precise
coordinate information.

*/

                   if (prevPoint != 0
                       	&& !uregion->AddSegment(segments,
						preciseSegments,
						preciseCoordinates,
						preciseInstants,
				               	cr,
				               	rDir,
				               	faceno,
				               	cycleno,
				               	pointno,
				               	partnerno,
				               	intervalLen,
				               	prevPoint,
				               	point)) {
                	   cerr << "uregion's segment checks failed"
                	        << endl;
                       delete uregion;
                       return 0;
                   }

                   prevPoint = point;
                   cyclepoints = nl->Rest(cyclepoints);
                   pointno++;
                   partnerno++;
               }

               if (!uregion->AddSegment(segments,
            		                    preciseSegments,
            		                    preciseCoordinates,
            		                    preciseInstants,
                                        cr,
                                        rDir,
                                        faceno,
                                        cycleno,
                                        pointno,
                                        partnerno,
                                        intervalLen,
                                        prevPoint,
                                        firstPoint)) {
            	   cerr << "uregion's segment checks failed "
			<< "with closing segment"
            	        << endl;
                   delete uregion;
                   return 0;
               }

               partnerno++;

               rDir.EndBulkLoad(true, true, false, false);
               bool direction = rDir.GetCycleDirection();

               int h = cr.Size()-(rDir.Size()*2);
               int i = initialSegmentsNum;

               while (h < cr.Size()) {
                   HalfSegment hsInsideAbove;
                   bool insideAbove;

                   cr.Get(h, hsInsideAbove);

                   if (MR2_DEBUG)
			   cerr << "InURegionEmbedded2() i="
				<< i
				<< " insideAbove="
				<< hsInsideAbove.attr.insideAbove
				<< endl;

                   if (direction == hsInsideAbove.attr.insideAbove)
                       insideAbove = false;
                   else
                       insideAbove = true;
                   if (cycleno > 0) insideAbove = !insideAbove;

                   uregion->SetSegmentInsideAbove(segments, i, insideAbove);

                   h += 2;
                   i++;
               }

               cycles = nl->Rest(cycles);
               cycleno++;
           }

           faces = nl->Rest(faces);
           faceno++;
       }

       cr.EndBulkLoad();

       if (MR2_DEBUG)
               for (int i = 0; i < uregion->GetSegmentsNum(); i++) {
                   MSegmentData2 dms;
                   uregion->GetSegment(segments, i, dms);

                   cerr << "InURegionEmbedded2() segment #"
                        << i
                        << ": "
                        << dms.ToString()
                        << endl;
               }

/*
This is different from ~Region~ handling in ~SpatialAlgebra~. We have
to go through the lists of degenerated segments and count how often
a region is inside above in each list. If there are more inside above
segments than others, we need one inside above segment for the
~TemporalFunction()~ and set all others to ignore. Vice versa if there
are more inside below segments. If there is the same number of inside above
and inside below segments, we can ignore the entire list.

*/

       for (int i = 0; i < uregion->GetSegmentsNum(); i++) {

           MSegmentData2 auxDms;
           uregion->GetSegment(segments, i, auxDms);
           MSegmentData2 dms( auxDms );

           if (dms.GetDegeneratedInitial() == DGM_UNKNOWN) {
        	   
               if (dms.GetDegeneratedInitialNext() >= 0) {

                   MSegmentData2 auxDegenDms;
                   MSegmentData2 degenDms;
                   unsigned int numInsideAbove = 0;
                   unsigned int numNotInsideAbove = 0;
                   for (int j = i+1;
                        j != 0;
                        j = degenDms.GetDegeneratedInitialNext()) {
                       uregion->GetSegment(segments, j-1, auxDegenDms);
                       degenDms = auxDegenDms;

                       if (MR2_DEBUG)
			   cerr << "InURegionEmbedded2() degen-magic-i "
				<< i
				<< " "
				<< j-1
				<< endl;

                       if (degenDms.GetInsideAbove())
                           numInsideAbove++;
                       else
                           numNotInsideAbove++;
                       if (j != i+1) {
                           degenDms.SetDegeneratedInitial(DGM_IGNORE);
                           uregion->PutSegment(segments, j-1, degenDms);
                       }
                   }

                   if (MR2_DEBUG)
			cerr << "InURegionEmbedded2() degen-magic-i result "
			<< numInsideAbove
			<< " "
			<< numNotInsideAbove
			<< endl;

                   if (numInsideAbove == numNotInsideAbove) {
                       dms.SetDegeneratedInitial(DGM_IGNORE);
                   } else if (numInsideAbove == numNotInsideAbove+1) {
                       dms.SetDegeneratedInitial(DGM_INSIDEABOVE);
                   } else if (numInsideAbove+1 == numNotInsideAbove) {
                       dms.SetDegeneratedInitial(DGM_NOTINSIDEABOVE);
                   } else {
                	   cerr << "segment ("
				<< dms.GetInitialStartX()
				<< ", " << dms.GetInitialStartY()
				<< ")-(" << dms.GetInitialEndX()
				<< ", " << dms.GetInitialEndY()
				<< ") / (" << dms.GetFinalStartX()
				<< ", " << dms.GetFinalStartY()
				<< ")-(" << dms.GetFinalEndX()
				<< ", " << dms.GetFinalEndY()
				<< ") incorrectly degenerated "
				<< "in initial instant"
				<< endl;
                       delete uregion;
                       return 0;
                   }
               } else
                   dms.SetDegeneratedInitial(DGM_NONE);
           }


           if (dms.GetDegeneratedFinal() == DGM_UNKNOWN) {
               if (dms.GetDegeneratedFinalNext() >= 0) {

                   MSegmentData2 auxDegenDms;
                   MSegmentData2 degenDms;
                   unsigned int numInsideAbove = 0;
                   unsigned int numNotInsideAbove = 0;
                   for (int j = i+1;
                        j != 0;
                        j = degenDms.GetDegeneratedFinalNext()) {
                       uregion->GetSegment(segments, j-1, auxDegenDms);
                       degenDms = auxDegenDms;
                       if (MR2_DEBUG)
			   cerr << "InURegionEmbedded2() degen-magic-f "
				<< i
				<< " "
				<< j-1
				<< endl;

                       if (degenDms.GetInsideAbove())
                           numInsideAbove++;
                       else
                           numNotInsideAbove++;
                       if (j != i+1) {
                           degenDms.SetDegeneratedFinal(DGM_IGNORE);
                           uregion->PutSegment(segments, j-1, degenDms);
                       }
                   }

                   if (MR2_DEBUG)
		   cerr << "InURegionEmbedded2() degen-magic-f result "
			<< numInsideAbove
			<< " "
			<< numNotInsideAbove
			<< endl;

                   if (numInsideAbove == numNotInsideAbove) {
                       dms.SetDegeneratedFinal(DGM_IGNORE);
                   } else if (numInsideAbove == numNotInsideAbove+1) {
                       dms.SetDegeneratedFinal(DGM_INSIDEABOVE);
                   } else if (numInsideAbove+1 == numNotInsideAbove) {
                       dms.SetDegeneratedFinal(DGM_NOTINSIDEABOVE);
                   } else {
                	   cerr << "segment ("
				<< dms.GetInitialStartX()
				<< ", " << dms.GetInitialStartY()
				<< ")-(" << dms.GetInitialEndX()
				<< ", " << dms.GetInitialEndY()
				<< ") / (" << dms.GetFinalStartX()
				<< ", " << dms.GetFinalStartY()
				<< ")-(" << dms.GetFinalEndX()
				<< ", " << dms.GetFinalEndY()
				<< ") incorrectly degenerated "
				<< "in final instant"
				<< endl;
                       delete uregion;
                       return 0;
                   }
               } else
                   dms.SetDegeneratedFinal(DGM_NONE);
           }


           uregion->PutSegment(segments, i, dms);
       }
       if (MR2_DEBUG)
               for (int i = 0; i < uregion->GetSegmentsNum(); i++) {
                   MSegmentData2 dms;

                   uregion->GetSegment(segments, i, dms);

                   cerr << "InURegionEmbedded2() resulting segment #"
                        << i
                        << ": "
                        << dms.ToString()
                        << endl;
               }

       return uregion;
} //end of function InURegionEmbedded2







/*
1.1.1 Function ~OutURegionEmbedded()~

*/
static ListExpr OutURegionEmbedded2(
    const URegionEmb2* ur,
    DbArray<MSegmentData2>* segments,
    DbArray<PreciseMSegmentData>* preciseSegments,
    DbArray<int>* preciseCoordinates,
    DbArray<int>* preciseInstants) {

	if (MR2_DEBUG)
	         cerr << "OutURegionEmbedded2() called" << endl;

/*
Conversion to list representation is straightforward. Just loop through
faces, cylces and segments and make sure that it is realised when one of
these changes.

*/

    int num = ur->GetSegmentsNum();

    ListExpr faces = nl->TheEmptyList();
    ListExpr facesLastElem = faces;
    ListExpr face = nl->TheEmptyList();
    ListExpr faceLastElem = face;
    ListExpr cycle = nl->TheEmptyList();
    ListExpr cycleLastElem = cycle;

    //cycle over all the segments the unit contains of...
    for (int i = 0; i < num; i++) {

    	if (MR2_DEBUG)
    	            cerr << "OutURegionEmbedded2() segment #" << i << endl;

    	//Get segment in integer-grid
        MSegmentData2 dms;
        ur->GetSegment(segments, i, dms);

        if (MR2_DEBUG)
		cerr << "OutURegionEmbedded2() returned, dms=" 
			<< &dms << endl;

		if (MR2_DEBUG)
			cerr << "OutURegionEmbedded2() point is "
				 << dms.GetFaceNo()
				 << " "
				 << dms.GetCycleNo()
				 << " ("
				 << dms.GetInitialStartX()
				 << ", "
				 << dms.GetInitialStartY()
				 << ", "
				 << dms.GetFinalStartX()
				 << ", "
				 << dms.GetFinalStartY()
				 << ")"
				 << endl;

/*
We check if the segment has precise coordinate information, and - if so -
we get this information and also construct a list representation for it.

*/
		ListExpr preciseList;

		if (!dms.GetIsBasicSegment())
		{
			PreciseMSegmentData pdms;
			ur->GetPreciseSegment(preciseSegments, i, pdms);


			const mpq_class preciseInitialStartX = 
				pdms.GetInitialStartX(preciseCoordinates);
			const mpq_class preciseInitialStartY = 
				pdms.GetInitialStartY(preciseCoordinates);
			const mpq_class preciseFinalStartX = 
				pdms.GetFinalStartX(preciseCoordinates);
			const mpq_class preciseFinalStartY = 
				pdms.GetFinalStartY(preciseCoordinates);

			ListExpr l1;
			ListExpr l2;
			ListExpr l3;
			ListExpr l4;
			gmpTypeToTextType(preciseInitialStartX, l1);
			gmpTypeToTextType(preciseInitialStartY, l2);
			gmpTypeToTextType(preciseFinalStartX, l3);
			gmpTypeToTextType(preciseFinalStartY, l4);


        	preciseList = nl->FourElemList(l1, l2, l3, l4);
        }
        else {
        	preciseList = nl->TheEmptyList();
        }

        ListExpr p = nl->FiveElemList(
			nl->IntAtom(dms.GetInitialStartX()),
			nl->IntAtom(dms.GetInitialStartY()),
			nl->IntAtom(dms.GetFinalStartX()),
			nl->IntAtom(dms.GetFinalStartY()),
			preciseList);

        if (cycle == nl->TheEmptyList()) {
        	if (MR2_DEBUG) 
			cerr << "OutURegionEmbedded2() new cycle" 
				<< endl;
            cycle = nl->OneElemList(p);
            cycleLastElem = cycle;
        } else {
        	if (MR2_DEBUG) 
			cerr << "OutURegionEmbedded2() existing cycle" 
				<< endl;
            cycleLastElem = nl->Append(cycleLastElem, p);
        }

        MSegmentData2 nextDms;
        if (i < num-1) {
        	ur->GetSegment(segments, i+1, nextDms);
        }


        if (i == num-1
            || dms.GetCycleNo() != nextDms.GetCycleNo()
            || dms.GetFaceNo() != nextDms.GetFaceNo()) {

        	if (MR2_DEBUG)
        	     cerr << "OutURegionEmbedded2() end of cycle" << endl;

            if (face == nl->TheEmptyList()) {
            	if (MR2_DEBUG)
			cerr << "OutURegionEmbedded2() new face" << endl;
                face = nl->OneElemList(cycle);
                faceLastElem = face;
            } else {
            	if (MR2_DEBUG)
			cerr << "OutURegionEmbedded2() existing face" 
				<< endl;
                faceLastElem = nl->Append(faceLastElem, cycle);
            }

            if (i == num-1
                || dms.GetFaceNo() != nextDms.GetFaceNo()) {
            	if (MR2_DEBUG)
			cerr << "OutURegionEmbedded2() end of face" 
				<< endl;

                if (faces == nl->TheEmptyList()) {
                    faces = nl->OneElemList(face);
                    facesLastElem = faces;
                } else
                    facesLastElem = nl->Append(facesLastElem, face);

                face = nl->TheEmptyList();
                faceLastElem = face;
            }

            cycle = nl->TheEmptyList();
            cycleLastElem = cycle;
        }
    }

    PreciseInterval pInt = ur->pInterval;
    ListExpr preciseTimeInterval;

    if (MR2_DEBUG) cerr << "Processing precise time interval..." << endl;

    if (pInt.GetPreciseInitialInstant(preciseInstants) != 0 ||
    		pInt.GetPreciseFinalInstant(preciseInstants) != 0)
    {
		ListExpr pStart;
		ListExpr pEnd;
		gmpTypeToTextType(
			pInt.GetPreciseInitialInstant(preciseInstants), 
				pStart);
		gmpTypeToTextType(
			pInt.GetPreciseFinalInstant(preciseInstants), 
				pEnd);

		preciseTimeInterval = nl->TwoElemList(
			pStart,
			pEnd);
    }
    else
    	preciseTimeInterval = nl->TheEmptyList();



    ListExpr res =
        nl->TwoElemList(
            nl->FiveElemList(
                OutDateTime(nl->TheEmptyList(),
                      SetWord((void*) &ur->timeInterval.start)),
                OutDateTime(nl->TheEmptyList(),
                      SetWord((void*) &ur->timeInterval.end)),
                nl->BoolAtom(ur->timeInterval.lc),
                nl->BoolAtom(ur->timeInterval.rc),
                preciseTimeInterval),
            faces);

    return res;
}


/*
1.1.1 Method ~AddSegment()~

This method is only called from InURegionEmbedded2. It returns ~true~
if the new segment can be added successfully to the region unit.
Therefor currently this method relies on the syntax check made by the
method EndBulkLoad from data type Region from SpatialAlgebra. this is
a workaround until MRegion2 and URegion2 have its own method
implementation for EndBulkLoad, or until a new data type for Region
with precise coordinates is supported.

*/
bool URegionEmb2::AddSegment(
    DbArray<MSegmentData2>* segments,
    DbArray<PreciseMSegmentData>* preciseSegments,
    DbArray<int>* preciseCoordinates,
    DbArray<int>* preciseInstants,
    Region& cr,
    Region& rDir,
    unsigned int faceno,
    unsigned int cycleno,
    unsigned int segmentno,
    unsigned int partnerno,
    DateTime& intervalLen,
    ListExpr start,
    ListExpr end) {

	if (MR2_DEBUG)
	        cerr << "URegionEmb2::AddSegment() called "
	             << faceno
	             << " "
	             << cycleno
	             << " "
	             << segmentno
	             << endl;

/*
To avoid awkward return value handling, we throw an exception if we find
a mistake. This is caught and converted into a proper error message and
return value.

*/

	try {
/*
Check list representation.

*/

	    if (nl->ListLength(start) != 5
    		|| !nl->IsAtom(nl->First(start))
    		|| !nl->IsAtom(nl->Second(start))
    		|| !nl->IsAtom(nl->Third(start))
    		|| !nl->IsAtom(nl->Fourth(start))
    		|| nl->AtomType(nl->First(start)) != IntType
    		|| nl->AtomType(nl->Second(start)) != IntType
    		|| nl->AtomType(nl->Third(start)) != IntType
    		|| nl->AtomType(nl->Fourth(start)) != IntType)
	    {
	    	throw invalid_argument(
		" Start point "
		+ nl->ToString(start)
		+ " not in format "
		+ "(<number> <number> <number> <number> <ListExpr>)");
	    }

	    if (nl->ListLength(start) != 5
		|| !nl->IsAtom(nl->First(end))
		|| !nl->IsAtom(nl->Second(end))
		|| !nl->IsAtom(nl->Third(end))
		|| !nl->IsAtom(nl->Fourth(end))
		|| nl->AtomType(nl->First(end)) != IntType
		|| nl->AtomType(nl->Second(end)) != IntType
		|| nl->AtomType(nl->Third(end)) != IntType
		|| nl->AtomType(nl->Fourth(end)) != IntType)
	   {
		throw invalid_argument(
		" End point "
		+nl->ToString(end)
		+" not in format "
		+ "(<number> <number> <number> <number> <ListExpr>)");
	   }
/*
Create segment from list representation.
If we have precise information for this segment, basic and precise
segments must be created, otherwise only basic segment.

*/

        MSegmentData2* pointDms;
        PreciseMSegmentData pdms(-1);

	if (nl->ListLength(nl->Fifth(start)) >= 1
			|| nl->ListLength(nl->Fifth(end)) >= 1)
	{
		if (MR2_DEBUG)
		cerr << "URegionEmb2::AddSegment(): "
		<< "There is precise coordinate information!" 
		<< endl;

		mpq_class piStartX(0);
		mpq_class piStartY(0);
		mpq_class piEndX(0);
		mpq_class piEndY(0);
		mpq_class pfStartX(0);
		mpq_class pfStartY(0);
		mpq_class pfEndX(0);
		mpq_class pfEndY(0);

		if (nl->ListLength(nl->Fifth(start)) >= 1)
		{
			//Check syntax
			if (nl->ListLength(nl->Fifth(start)) == 4 
				&& nl->IsAtom(
				nl->First(nl->Fifth(start))) 
				&& nl->AtomType(nl->First(
				nl->Fifth(start))) == TextType 
				&& nl->IsAtom(nl->Second(
				nl->Fifth(start))) 
				&& nl->AtomType(nl->Second(
				nl->Fifth(start))) == TextType 
				&& nl->IsAtom(nl->Third(
				nl->Fifth(start))) 
				&& nl->AtomType(nl->Third(
				nl->Fifth(start))) == TextType 
				&& nl->IsAtom(nl->Fourth(
				nl->Fifth(start))) 
				&& nl->AtomType(nl->Fourth(
				nl->Fifth(start))) == TextType)
			{
			    textTypeToGmpType(nl->First(
				nl->Fifth(start)), piStartX);
			    textTypeToGmpType(nl->Second(
				nl->Fifth(start)), piStartY);
			    textTypeToGmpType(nl->Third(
				nl->Fifth(start)), pfStartX);
			    textTypeToGmpType(nl->Fourth(
				nl->Fifth(start)), pfStartY);
			}
		}
		if (nl->ListLength(nl->Fifth(end)) >= 1)
		{
			//Check syntax
			if (nl->ListLength(nl->Fifth(start)) == 4 
				&& nl->IsAtom(nl->First(
				nl->Fifth(end))) 
				&& nl->AtomType(nl->First(
				nl->Fifth(end))) == TextType 
				&& nl->IsAtom(nl->Second(
				nl->Fifth(end))) 
				&& nl->AtomType(nl->Second(
				nl->Fifth(end))) == TextType 
				&& nl->IsAtom(nl->Third(
				nl->Fifth(end))) 
				&& nl->AtomType(nl->Third(
				nl->Fifth(end))) == TextType 
				&& nl->IsAtom(nl->Fourth(
				nl->Fifth(end))) 
				&& nl->AtomType(nl->Fourth(
				nl->Fifth(end))) == TextType)
			{
			    textTypeToGmpType(nl->First(
				nl->Fifth(end)), piEndX);
			    textTypeToGmpType(nl->Second(
				nl->Fifth(end)), piEndY);
			    textTypeToGmpType(nl->Third(
				nl->Fifth(end)), pfEndX);
			    textTypeToGmpType(nl->Fourth(
				nl->Fifth(end)), pfEndY);
			}
		}

		pdms.SetInitialStartX(piStartX, preciseCoordinates);
		pdms.SetInitialStartY(piStartY, preciseCoordinates);
		pdms.SetInitialEndX(piEndX, preciseCoordinates);
		pdms.SetInitialEndY(piEndY, preciseCoordinates);
		pdms.SetFinalStartX(pfStartX, preciseCoordinates);
		pdms.SetFinalStartY(pfStartY, preciseCoordinates);
		pdms.SetFinalEndX(pfEndX, preciseCoordinates);
		pdms.SetFinalEndY(pfEndY, preciseCoordinates);

		pointDms = new MSegmentData2(faceno,
			 cycleno,
			 segmentno,
			 false,
			 nl->IntValue(nl->First(start)),
			 nl->IntValue(nl->Second(start)),
			 nl->IntValue(nl->First(end)),
			 nl->IntValue(nl->Second(end)),
			 nl->IntValue(nl->Third(start)),
			 nl->IntValue(nl->Fourth(start)),
			 nl->IntValue(nl->Third(end)),
			 nl->IntValue(nl->Fourth(end)),
			 pdms,
			 preciseCoordinates);
		}
		else
		{
			if (MR2_DEBUG)
				cerr << "URegionEmb2::AddSegment(): "
				<< "No precise coordinate information, "
				<< "basic segment." << endl;


			mpq_class dummyValue(0);
			pdms.SetAll(dummyValue, preciseCoordinates);

			pointDms = new MSegmentData2(faceno,
				 cycleno,
				 segmentno,
				 false,
				 nl->IntValue(nl->First(start)),
				 nl->IntValue(nl->Second(start)),
				 nl->IntValue(nl->First(end)),
				 nl->IntValue(nl->Second(end)),
				 nl->IntValue(nl->Third(start)),
				 nl->IntValue(nl->Fourth(start)),
				 nl->IntValue(nl->Third(end)),
				 nl->IntValue(nl->Fourth(end)));
		} //if-else-check for precise-information...

		if (MR2_DEBUG)
			cerr << "URegionEmb2::AddSegment() segmentsNum="
				 << segmentsNum
				 << endl;

		MSegmentData2 dms(pointDms);
		delete pointDms;

/*
For each of the already existing segments:

*/

		for (int i = segmentsNum-1; i >= 0; i--) {

			if (MR2_DEBUG)
				cerr << "URegionEmb2::AddSegment() i=" 
				<< i << endl;

			MSegmentData2 auxExistingDms;
			PreciseMSegmentData auxExistingPdms;

			segments->Get(segmentsStartPos+i, auxExistingDms);
			MSegmentData2 existingDms( auxExistingDms );
			preciseSegments->Get(
				segmentsStartPos+i, auxExistingPdms);
			PreciseMSegmentData existingPdms( auxExistingPdms );

/*
Check whether the current segment degenerates with this segment in the
initial instant. Note that segments reduced to points are excluded from
this.

All segments, which degenerate into each other, are collected in a list
using the ~degeneratedInitialNext~ attribute.

*/
			bool degenerationFound = false;

			if (dms.GetDegeneratedInitialNext() < 0
				&& !dms.GetPointInitial()
				&& !existingDms.GetPointInitial())
			{
			    if (dms.GetIsBasicSegment()
				&& existingDms.GetIsBasicSegment())
			    {
			      if (((dms.GetInitialStartX() == 
					existingDms.GetInitialStartX()
					&& dms.GetInitialStartY() == 
					existingDms.GetInitialStartY()
					&& dms.GetInitialEndX() == 
					existingDms.GetInitialEndX()
					&& dms.GetInitialEndY() == 
					existingDms.GetInitialEndY())
					|| (dms.GetInitialStartX() == 
					existingDms.GetInitialEndX()
					&& dms.GetInitialStartY() == 
					existingDms.GetInitialEndY()
					&& dms.GetInitialEndX() == 
					existingDms.GetInitialStartX()
					&& dms.GetInitialEndY() == 
					existingDms.GetInitialStartY())))
				    {
					if (MR2_DEBUG)
					cerr << "Both segments are basic, "
					<< "and they degenerate in "
					<< "initial instant!" 
					<< endl;
					degenerationFound = true;
				    }
			      }
			      else if ((maybeEqual(
				    dms.GetInitialStartX(),
				    existingDms.GetInitialStartX(), 1, 1)
				    && maybeEqual(dms.GetInitialStartY(),
			            existingDms.GetInitialStartY(), 1, 1)
				    && maybeEqual(dms.GetInitialEndX(),	
				    existingDms.GetInitialEndX(), 1, 1)
				    && maybeEqual(dms.GetInitialEndY(),
				    existingDms.GetInitialEndY(), 1, 1))
				    || (maybeEqual(dms.GetInitialStartX(),
				    existingDms.GetInitialEndX(), 1, 1)
				    && maybeEqual(dms.GetInitialStartY(),
				    existingDms.GetInitialEndY(), 1, 1)
				    && maybeEqual(
				    dms.GetInitialEndX(),
				    existingDms.GetInitialStartX(), 1, 1)
				    && maybeEqual(
				    dms.GetInitialEndY(),
				    existingDms.GetInitialStartY(), 1, 1)))
			    {
				if (MR2_DEBUG)
				    cerr << "At least one segment not"
					<< " basic, precise check for"
					<< " degeneration..." << endl;

//Check if degenerated with precise coordinate information...
				if ((cmp(pdms.GetInitialStartX
				    (preciseCoordinates) + 
				    dms.GetInitialStartX(),
			            existingPdms.GetInitialStartX
				    (preciseCoordinates) + 
				    existingDms.GetInitialStartX()) == 0
				    && cmp(pdms.GetInitialStartY
				    (preciseCoordinates) + 
				    dms.GetInitialStartY(),
				    existingPdms.GetInitialStartY
				    (preciseCoordinates) + 
				    existingDms.GetInitialStartY()) == 0
				    && cmp(pdms.GetInitialEndX
				    (preciseCoordinates) + 
				    dms.GetInitialEndX(),
				    existingPdms.GetInitialEndX
				    (preciseCoordinates) + 
				    existingDms.GetInitialEndX()) == 0
				    && cmp(pdms.GetInitialEndY
				    (preciseCoordinates) + 
				    dms.GetInitialEndY(),
				    existingPdms.GetInitialEndY
				    (preciseCoordinates) + 
				    existingDms.GetInitialEndY()) == 0)
				    || (cmp(pdms.GetInitialStartX	
				    (preciseCoordinates) + 	
				    dms.GetInitialStartX(),	
				    existingPdms.GetInitialEndX
				    (preciseCoordinates) + 
				    existingDms.GetInitialEndX()) == 0
				    && cmp(pdms.GetInitialStartY
				    (preciseCoordinates) + 
				    dms.GetInitialStartY(),		
				    existingPdms.GetInitialEndY
				    (preciseCoordinates) + 
				    existingDms.GetInitialEndY()) == 0
				    && cmp(pdms.GetInitialEndX
				    (preciseCoordinates) + 
				    dms.GetInitialEndX(),	
				    existingPdms.GetInitialStartX
				    (preciseCoordinates) + 
				    existingDms.GetInitialStartX()) == 0
				    && cmp(pdms.GetInitialEndY
				    (preciseCoordinates) + 
				    dms.GetInitialEndY(),
				    existingPdms.GetInitialStartY
				    (preciseCoordinates) + 
				    existingDms.GetInitialStartY()) == 0))
				{
				    if (MR2_DEBUG)
					cerr << "URegionEmb2::Addsegment():"
					<< " DegenerationFound in"
					<< " preciseCheck!" << endl;

				    degenerationFound = true;
				}
			    }
			}

			if (degenerationFound)
			{
			    if (MR2_DEBUG)
				cerr << "URegionEmb2::AddSegment() "
				 << "degenerated initial in " << i
				 << endl;

			    dms.SetDegeneratedInitialNext(0);
			    existingDms.SetDegeneratedInitialNext
				(segmentsNum+1);

			    segments->Put(segmentsStartPos+i, existingDms);
			}

/*
Same for the final instant.

*/
			degenerationFound = false;

			if (dms.GetDegeneratedFinalNext() < 0
				&& !dms.GetPointFinal()
				&& !existingDms.GetPointFinal())
			{
			    if (dms.GetIsBasicSegment() && 
				existingDms.GetIsBasicSegment())
			    {
				if (((dms.GetFinalStartX() == 
				    existingDms.GetFinalStartX()
				    && dms.GetFinalStartY() == 
					existingDms.GetFinalStartY()
				    && dms.GetFinalEndX() == 
					existingDms.GetFinalEndX()
				    && dms.GetFinalEndY() == 
					existingDms.GetFinalEndY())
				    || (dms.GetFinalStartX() == 
					existingDms.GetFinalEndX()
				    && dms.GetFinalStartY() == 
					existingDms.GetFinalEndY()
				    && dms.GetFinalEndX() == 
					existingDms.GetFinalStartX()
				    && dms.GetFinalEndY() == 
					existingDms.GetFinalStartY())))
				{
				    if (MR2_DEBUG)
					cerr << "Both segments are basic"
					<< " and degenerate in final"
					<< " instant" << endl;

				    degenerationFound = true;
				}
			    }
			    else if ((maybeEqual(
				    dms.GetFinalStartX(),
				    existingDms.GetFinalStartX(), 1, 1)
			 	    && maybeEqual(
					dms.GetFinalStartY(),
					existingDms.GetFinalStartY(), 1, 1)
			 	    && maybeEqual(
					dms.GetFinalEndX(),
					existingDms.GetFinalEndX(), 1, 1)
			 	    && maybeEqual(
					dms.GetFinalEndY(),
					existingDms.GetFinalEndY(), 1, 1))
				    || (maybeEqual(
					dms.GetFinalStartX(),
					existingDms.GetFinalEndX(), 1, 1)
				    && maybeEqual(
					dms.GetFinalStartY(),
					existingDms.GetFinalEndY(), 1, 1)
				    && maybeEqual(
					dms.GetFinalEndX(),
					existingDms.GetFinalStartX(), 1, 1)
				    && maybeEqual(
					dms.GetFinalEndY(),
					existingDms.GetFinalStartY(), 
					1, 1)))
			    {
				    if (MR2_DEBUG)
				    cerr << "At least one segment is not"
					 << " basic, precise check for "
					 << "degeneration..." << endl;

//Check if degenerated with precise coordinate information...
				    if ((cmp(pdms.GetFinalStartX
					(preciseCoordinates) + 
					dms.GetFinalStartX(),
					existingPdms.GetFinalStartX
					(preciseCoordinates) + 
					existingDms.GetFinalStartX()) == 0
					&& cmp(pdms.GetFinalStartY
					(preciseCoordinates) + 
					dms.GetFinalStartY(),
					existingPdms.GetFinalStartY
					(preciseCoordinates) + 
					existingDms.GetFinalStartY()) == 0
					&& cmp(pdms.GetFinalEndX	
					(preciseCoordinates) + 
					dms.GetFinalEndX(),
					existingPdms.GetFinalEndX
					(preciseCoordinates) + 
					existingDms.GetFinalEndX()) == 0
					&& cmp(pdms.GetFinalEndY
					(preciseCoordinates) + 
					dms.GetFinalEndY(),
					existingPdms.GetFinalEndY
					(preciseCoordinates) + 
					existingDms.GetFinalEndY()) == 0)
					|| (cmp(pdms.GetFinalStartX
					(preciseCoordinates) + 
					dms.GetFinalStartX(),
					existingPdms.GetFinalEndX
					(preciseCoordinates) + 
					existingDms.GetFinalEndX()) == 0
					&& cmp(pdms.GetFinalStartY
					(preciseCoordinates) + 
					dms.GetFinalStartY(),
					existingPdms.GetFinalEndY
					(preciseCoordinates) + 
					existingDms.GetFinalEndY()) == 0
					&& cmp(pdms.GetFinalEndX
					(preciseCoordinates) + 
					dms.GetFinalEndX(),
					existingPdms.GetFinalStartX
					(preciseCoordinates) + 
					existingDms.GetFinalStartX()) == 0
					&& cmp(pdms.GetFinalEndY
					(preciseCoordinates) + 
					dms.GetFinalEndY(),
					existingPdms.GetFinalStartY
					(preciseCoordinates) + 
					existingDms.GetFinalStartY()) == 0))
				    {
					if (MR2_DEBUG)
					cerr << "URegionEmb2::AddSegment():"
					<< " Degeneration found in "
					<< "precise check!" << endl;

					degenerationFound = true;
				    }
			    }
			}

			if (degenerationFound)
			{
				if (MR2_DEBUG)
				    cerr << "URegionEmb2::AddSegment() "
					 << "degenerated final in " << i
					 << endl;

				dms.SetDegeneratedFinalNext(0);
				existingDms.SetDegeneratedFinalNext
					(segmentsNum+1);

				segments->Put(segmentsStartPos+i, 
					existingDms);
			}

/*
If we have a point time interval, degeneration is not allowed.

*/


			if (intervalLen.IsZero() && 
				cmp(pInterval.GetPreciseInitialInstant
				(preciseInstants),
				pInterval.GetPreciseFinalInstant
				(preciseInstants)) == 0
				&& (dms.GetDegeneratedInitialNext() >= 0
				|| dms.GetDegeneratedFinalNext() >= 0)) {
				stringstream msg;
				msg << " Units with zero length time"
					<< " interval must not"
					<< " be degenerated." << endl
					<< "    New segment:" << endl
					<< "      Initial: ("
					<< dms.GetInitialStartX()
					<< " "
					<< dms.GetInitialStartY()
					<< ") - ("
					<< dms.GetInitialEndX()
					<< " "
					<< dms.GetInitialEndY()
					<< ")" << endl
					<< "      Final: ("
					<< dms.GetFinalStartX()
					<< " "
					<< dms.GetFinalStartY()
					<< ") - ("
					<< dms.GetFinalEndX()
					<< " "
					<< dms.GetFinalEndY();
				throw invalid_argument(msg.str());
			} else if ((!intervalLen.IsZero() 
				|| cmp(pInterval.GetPreciseInitialInstant
					(preciseInstants),
					pInterval.GetPreciseFinalInstant
					(preciseInstants)) != 0)
					&& dms.GetDegeneratedInitialNext() 
						>= 0
					&& dms.GetDegeneratedFinalNext() 
						>= 0) {
			    stringstream msg;
			    msg << "URegionEmb2::AddSegment() "
				<< "degeneration check failed!"
				<< " Units must not degenerate both "
				<< "in initial and"
				<< " final instant." << endl
				<< "    New segment:" << endl
				<< "      Initial: ("
				<< dms.GetInitialStartX()
				<< " "
				<< dms.GetInitialStartY()
				<< ") - ("
				<< dms.GetInitialEndX()
				<< " "
				<< dms.GetInitialEndY()
				<< ")" << endl
				<< "      Final: ("
				<< dms.GetFinalStartX()
				<< " "
				<< dms.GetFinalStartY()
				<< ") - ("
				<< dms.GetFinalEndX()
				<< " "
				<< dms.GetFinalEndY()
				<< ")" << endl;
			    throw invalid_argument(msg.str());
			}

/*
Check if the current segment intersects with the existing segment.
Since both are moving segments and are spanning a trapezium in 3d space
$(x, y, t)$, this is equivalent to the intersection of two trapeziums.

*/

			GridPoint point1(dms.GetInitialStartX(), 
				dms.GetInitialStartY(), 
				dms.GetIsBasicSegment());
			GridPoint point2(dms.GetInitialEndX(), 
				dms.GetInitialEndY(), 
				dms.GetIsBasicSegment());
			GridPoint point3(dms.GetFinalStartX(), 
				dms.GetFinalStartY(), 
				dms.GetIsBasicSegment());
			GridPoint point4(dms.GetFinalEndX(), 
				dms.GetFinalEndY(), 
				dms.GetIsBasicSegment());
			GridPointSegment segment1(point1, point2);
			GridPointSegment segment2(point3, point4);
			GridPoint point5(existingDms.GetInitialStartX(), 
				existingDms.GetInitialStartY(), 
				existingDms.GetIsBasicSegment());
			GridPoint point6(existingDms.GetInitialEndX(), 
				existingDms.GetInitialEndY(), 
				existingDms.GetIsBasicSegment());
			GridPoint point7(existingDms.GetFinalStartX(), 
				existingDms.GetFinalStartY(), 
				existingDms.GetIsBasicSegment());
			GridPoint point8(existingDms.GetFinalEndX(), 
				existingDms.GetFinalEndY(), 
				existingDms.GetIsBasicSegment());
			GridPointSegment segment3(point5, point6);
			GridPointSegment segment4(point7, point8);
			GridPointTrapezium trapezium1(segment1, segment2);
			GridPointTrapezium trapezium2(segment3, segment4);

			if (trapeziumsMayIntersect(broughtDown
				(intervalLen.ToDouble()), trapezium1, 
				trapezium2))
			{
				bool trapeziumsIntersect;
				if (existingDms.GetIsBasicSegment() 
					&& dms.GetIsBasicSegment())
				{
//precise coordinates are all zero, trivially true
				    trapeziumsIntersect = true; 
				}
				else {
/*
We create the precise trapeziums and recheck if they really intersect. Therefor we need to add coordinates of precise and integer representation to get the absolute values.

*/
				    PrecisePoint ppoint1
					(pdms.GetInitialStartX
					(preciseCoordinates)+
					dms.GetInitialStartX(),
					pdms.GetInitialStartY
					(preciseCoordinates)+
					dms.GetInitialStartY());
				    PrecisePoint ppoint2(pdms.GetInitialEndX
					(preciseCoordinates)
					+dms.GetInitialEndX(),
					pdms.GetInitialEndY
					(preciseCoordinates)
					+dms.GetInitialEndY());
				    PrecisePoint ppoint3(pdms.GetFinalStartX
					(preciseCoordinates)
					+dms.GetFinalStartX(),
					pdms.GetFinalStartY
					(preciseCoordinates)
					+dms.GetFinalStartY());
				    PrecisePoint ppoint4(pdms.GetFinalEndX
					(preciseCoordinates)
					+dms.GetFinalEndX(),
					pdms.GetFinalEndY
					(preciseCoordinates)
					+dms.GetFinalEndY());
				    PreciseSegment psegment1(ppoint1, 
					ppoint2);
				    PreciseSegment psegment2(ppoint3, 
					ppoint4);
				    PreciseTrapezium ptrapezium1(psegment1, 
					psegment2);
				    PrecisePoint ppoint5
					(existingPdms.GetInitialStartX
					(preciseCoordinates)+
					existingDms.GetInitialStartX(),
					existingPdms.GetInitialStartY
					(preciseCoordinates)+
					existingDms.GetInitialStartY());
				    PrecisePoint ppoint6
					(existingPdms.GetInitialEndX
					(preciseCoordinates)+
					existingDms.GetInitialEndX(),
					existingPdms.GetInitialEndY
					(preciseCoordinates)+
					existingDms.GetInitialEndY());
				    PrecisePoint ppoint7
					(existingPdms.GetFinalStartX
					(preciseCoordinates)+
					existingDms.GetFinalStartX(),
					existingPdms.GetFinalStartY
					(preciseCoordinates)+
					existingDms.GetFinalStartY());
				    PrecisePoint ppoint8(existingPdms.
					GetFinalEndX(preciseCoordinates)+
					existingDms.GetFinalEndX(),
					existingPdms.GetFinalEndY
					(preciseCoordinates)+
					existingDms.GetFinalEndY());
				    PreciseSegment psegment3(ppoint5, 
					ppoint6);
				    PreciseSegment psegment4(ppoint7, 
					ppoint8);
				    PreciseTrapezium ptrapezium2(
					psegment3, psegment4);

				    mpq_class pdt(pInterval.
					GetPreciseFinalInstant
					(preciseInstants) -
					pInterval.GetPreciseInitialInstant
					(preciseInstants) +
					intervalLen.ToDouble());

				    trapeziumsIntersect = 
					preciseTrapeziumsIntersect(pdt, 
					ptrapezium1, ptrapezium2);
				}

				if (trapeziumsIntersect)
				{
				    stringstream msg;
				    msg << " Moving segments intersect "
					<< endl
					<< "    Existing segment:" << endl
					<< "      Initial: ("
					<< existingDms.GetInitialStartX()
					<< " "
					<< existingDms.GetInitialStartY()
					<< ") - ("
					<< existingDms.GetInitialEndX()
					<< " "
					<< existingDms.GetInitialEndY()
					<< ")" << endl
					<< "      Final:  ("
					<< existingDms.GetFinalStartX()
					<< " "
					<< existingDms.GetFinalStartY()
					<< ") - ("
					<< existingDms.GetFinalEndX()
					<< " "
					<< existingDms.GetFinalEndY()
					<< ")" << endl
					<< "    New segment:" << endl
					<< "      Initial: ("
					<< dms.GetInitialStartX()
					<< " "
					<< dms.GetInitialStartY()
					<< ") - ("
					<< dms.GetInitialEndX()
					<< " "
					<< dms.GetInitialEndY()
					<< ")" << endl
					<< "      Final:  ("
					<< dms.GetFinalStartX()
					<< " "
					<< dms.GetFinalStartY()
					<< ") - ("
					<< dms.GetFinalEndX()
					<< " "
					<< dms.GetFinalEndY();

				    throw invalid_argument(msg.str());
				} //if precise trapeziums intersect
			} //if trapeziums may intersect
		} //for: for each of the already existing segments...

/*
Add half segments to $cr$ and $rDir$ for region check and direction
computation.
Note that the syntax check done in Region may fail although we have a
syntactically correct moving region value, because syntactical correctness
may depend on precise coordinates which are not passed to region!
This can only be a provisional solution, but not the final one!

*/

		double t = intervalLen.IsZero() ? 0 : 0.5;
		double xs =
			dms.GetInitialStartX()
			+(dms.GetFinalStartX()-dms.GetInitialStartX())*t;
		double ys =
			dms.GetInitialStartY()
			+(dms.GetFinalStartY()-dms.GetInitialStartY())*t;
		double xe =
			dms.GetInitialEndX()
			+(dms.GetFinalEndX()-dms.GetInitialEndX())*t;
		double ye =
			dms.GetInitialEndY()
			+(dms.GetFinalEndY()-dms.GetInitialEndY())*t;

		Point s(true, xs, ys);
		Point e(true, xe, ye);

/*
Here we need an extra check, because HalfSegment creation will lead to a failed
assertion if start and end point are equal or almost equal.
This might happen on the integer grid, although the precise coordinates of both
points are different!

*/
		if (AlmostEqual(s, e))
		{
		    cerr << endl
		    << "--------------------------------------------------"
		    << endl
		    << "URegionEmb2::AddSegment() returns false, "
		    << "failure reason is:" 
		    << endl
		    << "Halfsegment creation not possible with start "
		    << "and end point almost equal." 
		    << endl
		    << "Start point: (" << xs << ", " << ys << ")"
		    << endl
		    << "End point: ("  << xe << ", " << ye << ")"
		    << endl 
		    << "Try input with larger coordinates "
		    << "and scale afterwards with scale operator!"
		    << endl
		    << "--------------------------------------------------"
		    << endl;
		return false;
		}
		HalfSegment hs(true, s, e);

		hs.attr.faceno = faceno;
		hs.attr.cycleno = cycleno;
		hs.attr.partnerno = partnerno;
		hs.attr.edgeno = segmentno;

		hs.attr.insideAbove = hs.GetLeftPoint() == s;

		if (MR2_DEBUG)
			cerr << "URegionEmb2::AddSegment() "
				 << dms.ToString()
				 << endl;

		if (MR2_DEBUG)
			cerr << "URegionEmb2::AddSegment() "
				 << faceno
				 << " "
				 << cycleno
				 << " "
				 << partnerno
				 << " ("
				 << xs
				 << ", "
				 << ys
				 << ")-("
				 << xe
				 << ", "
				 << ye
				 << ") ia="
				 << hs.attr.insideAbove
				 << endl;

		if (!cr.InsertOk(hs)) {
		    stringstream msg;
		    msg << " Region checks for segment failed." << endl
			<< "    New segment:" << endl
			<< "      Initial: ("
			<< dms.GetInitialStartX()
			<< " "
			<< dms.GetInitialStartY()
			<< ") - ("
			<< dms.GetInitialEndX()
			<< " "
			<< dms.GetInitialEndY()
			<< ")" << endl
			<< "      Final:  ("
			<< dms.GetFinalStartX()
			<< " "
			<< dms.GetFinalStartY()
			<< ") - ("
			<< dms.GetFinalEndX()
			<< " "
			<< dms.GetFinalEndY();
		    throw invalid_argument(msg.str());
		}

		cr += hs;
		if( hs.IsLeftDomPoint() )
		{
		  rDir += hs;
		  hs.SetLeftDomPoint( false );
		}
		else
		{
		  hs.SetLeftDomPoint( true );
		  rDir += hs;
		}
		cr += hs;

		segments->resize(segmentsStartPos+segmentsNum+1);
		segments->Put(segmentsStartPos+segmentsNum, dms);

		//The same for the precise segment!
		preciseSegments->resize(segmentsStartPos+segmentsNum+1);
		preciseSegments->Put(segmentsStartPos+segmentsNum, pdms);

		segmentsNum++;

/*
The bbox is calculated on the integer grid. We use the down left corner
of the respective grid cell for the minimum values and the upper right
corner of the grid cell for the max values.
As the coordinate's representation on the grid already use the down left
corners, just for the maximum values we need to adapt them...

*/
		if (bbox.IsDefined()) {
			double min[3] = { bbox.MinD(0), bbox.MinD(1), 
				bbox.MinD(2) };
			double max[3] = { bbox.MaxD(0), bbox.MaxD(1), 
				bbox.MaxD(2) };
			if (dms.GetInitialStartX() < min[0])
				min[0] = dms.GetInitialStartX();
			if (dms.GetFinalStartX() < min[0])
				min[0] = dms.GetFinalStartX();
			if (dms.GetInitialStartY() < min[1])
				min[1] = dms.GetInitialStartY();
			if (dms.GetFinalStartY() < min[1])
				min[1] = dms.GetFinalStartY();
			if ((dms.GetInitialStartX()+1) > max[0])
				max[0] = dms.GetInitialStartX()+1;
			if ((dms.GetFinalStartX()+1) > max[0])
				max[0] = dms.GetFinalStartX()+1;
			if ((dms.GetInitialStartY()+1) > max[1])
				max[1] = dms.GetInitialStartY()+1;
			if ((dms.GetFinalStartY()+1) > max[1])
				max[1] = dms.GetFinalStartY()+1;
			bbox.Set(true, min, max);
		} else {
		  double min[3] =
	    	    { dms.GetInitialStartX() < dms.GetFinalStartX() ?
		        dms.GetInitialStartX() : dms.GetFinalStartX(),
		    dms.GetInitialStartY() < dms.GetFinalStartY() ? 
			dms.GetInitialStartY() : dms.GetFinalStartY(),
		    timeInterval.start.ToDouble() };
	  	  double max[3] =
		    { dms.GetInitialStartX() > dms.GetFinalStartX() ? 
			dms.GetInitialStartX()+1 : dms.GetFinalStartX()+1,
		      dms.GetInitialStartY() > dms.GetFinalStartY() ? 
			dms.GetInitialStartY()+1 : dms.GetFinalStartY()+1,
		      timeInterval.end.ToDouble() };
		  bbox.Set(true, min, max);
		} //if-else...
	} //try
	catch (invalid_argument& e) {
	cerr << endl 
	  << "-----------------------------------------------------------"
	  << endl
	  << "Checking segment failed."
	  << endl
	  << e.what()
	  << endl
	  << "-----------------------------------------------------------"
	  << endl;
	return false;
    }
    return true;
}





/*
1 Data type ~uregion2~

1.1 Class ~URegion2~

1.1.1 Class definition

The class definition has been moved to ~MovingRegion2Algebra.h~.

1.1.1 Constructors

*/


URegion2::URegion2(unsigned int n) :
    SpatialTemporalUnit<Region, 3>(true),
    segments(n),
    preciseSegments(n),
    preciseCoordinates(0),
    preciseInstants(0) {
    SetDefined(true);
    if (MR2_DEBUG)
		cerr << "URegion2::URegion2() #1 called"
			 << endl;
}



	
/*
This constructor creates a URegion2 object from an existing URegion object, by splitting the double
coordinates into an integer part and the remaining rest, which will be stored as mpq\_class values in
PreciseMSegmentData instances...
In a first step, the double coordinates are multiplied with scaleFactor to get the integer grid values.

*/
URegion2::URegion2(URegion& coarseRegion, const int scaleFactor) :
	segments(coarseRegion.GetMSegmentData()->Size()),
	preciseSegments(coarseRegion.GetMSegmentData()->Size()),
	preciseCoordinates(0),
	preciseInstants(0) {

	if (MR2_DEBUG)
	cerr << "URegion2::URegion2(coarseRegion, scaleFactor) called" 
		<< endl
		<< "ScaleFactor: " << scaleFactor << " " << endl
		<< "Number of segments: "
 		<< coarseRegion.GetMSegmentData()->Size() << " " << endl;

	this->scaleFactor = scaleFactor;

//Transform the time interval to integer grid coordinates - 
//preciseInterval is calculated in URegionEmb2-Constructor
	double initial = (double)broughtDown
		(coarseRegion.timeInterval.start.ToDouble());
	double final = (double)broughtDown
		(coarseRegion.timeInterval.end.ToDouble());
	Instant initInst(initial);
	Instant finInst(final);
	Interval<Instant> interval(initInst, finInst, 
		coarseRegion.timeInterval.lc, coarseRegion.timeInterval.rc);

	//initialize precise time interval
	PreciseInterval pInt(-1);

	URegionEmb origUremb = coarseRegion.GetEmbedded();
	origUremb.SetSegmentsNum(coarseRegion.GetMSegmentData()->Size());

	URegionEmb2 uremb(&segments, &preciseSegments, &preciseCoordinates, 
			&preciseInstants,
			coarseRegion.timeInterval, pInt, origUremb, 
			coarseRegion.GetMSegmentData(), 0, scaleFactor);

	SetEmbedded(&uremb);
	timeInterval = uremb.timeInterval;

}












/*
1.1.1 Function ~AddURegion2()~

*/
void URegion2::AddURegion2(URegion2 *newRegion)
{
  if(timeInterval != newRegion->timeInterval)
  {
    throw invalid_argument("Intervals are not equal");
  }

  const DbArray<int> *insts = newRegion->GetPreciseInstants();
  if (uremb.pInterval.GetPreciseInitialInstant(&preciseInstants) != 
		newRegion->uremb.pInterval.GetPreciseInitialInstant(insts)
		|| uremb.pInterval.GetPreciseFinalInstant
			(&preciseInstants) != 
		newRegion->uremb.pInterval.GetPreciseFinalInstant(insts))
  {
	  throw invalid_argument("Precise Intervals are not equal");
  }

  const DbArray<MSegmentData2> *lines = newRegion->GetMSegmentData2();
  const DbArray<PreciseMSegmentData> *pLines = 
		newRegion->GetPreciseMSegmentData();
  const DbArray<int> *coords = newRegion->GetPreciseCoordinates();

  for(int i = 0; i < lines->Size(); i++)
  {
    MSegmentData2 line;
    lines->Get(i, line);
    AddSegment(line);

    PreciseMSegmentData pLine;
    pLines->Get(i, pLine);
    AddPreciseSegment(pLine);
  }

  for (int i = 0; i < coords->Size(); i++)
  {
	  int coord;
	  coords->Get(i, coord);
	  AddCoordinate(coord);
  }

}




/*
1.1.1 Function ~AddSegment()~

The bounding boxes are calculated on the integer grid, using the
down left corner of the respective cell for the minimum values and
the upper right corner for the maximum values.

*/
void URegion2::AddSegment(MSegmentData2 newSeg)
{
  uremb.PutSegment(&segments, segments.Size(), newSeg, true);
  if (uremb.BoundingBox().IsDefined())
  {
        double min[3] =
           { uremb.BoundingBox().MinD(0),
              uremb.BoundingBox().MinD(1),
              uremb.BoundingBox().MinD(2) };
        double max[3] =
           { uremb.BoundingBox().MaxD(0),
              uremb.BoundingBox().MaxD(1),
              uremb.BoundingBox().MaxD(2) };
        if (newSeg.GetInitialStartX() < min[0])
            min[0] = newSeg.GetInitialStartX();
        if (newSeg.GetFinalStartX() < min[0])
            min[0] = newSeg.GetFinalStartX();
        if (newSeg.GetInitialStartY() < min[1])
            min[1] = newSeg.GetInitialStartY();
        if (newSeg.GetFinalStartY() < min[1])
            min[1] = newSeg.GetFinalStartY();
        if (newSeg.GetInitialStartX()+1 > max[0])
            max[0] = newSeg.GetInitialStartX()+1;
        if (newSeg.GetFinalStartX()+1 > max[0])
            max[0] = newSeg.GetFinalStartX()+1;
        if (newSeg.GetInitialStartY()+1 > max[1])
            max[1] = newSeg.GetInitialStartY()+1;
        if (newSeg.GetFinalStartY()+1 > max[1])
            max[1] = newSeg.GetFinalStartY()+1;
        uremb.SetBBox(Rectangle<3>(true, min, max));

    } else {
        double min[3] =
            { newSeg.GetInitialStartX() < newSeg.GetFinalStartX()
              ? newSeg.GetInitialStartX() : newSeg.GetFinalStartX(),
              newSeg.GetInitialStartY() < newSeg.GetFinalStartY()
              ? newSeg.GetInitialStartY() : newSeg.GetFinalStartY(),
              uremb.timeInterval.start.ToDouble() };
        double max[3] =
            { newSeg.GetInitialStartX() > newSeg.GetFinalStartX()
              ? newSeg.GetInitialStartX()+1 : newSeg.GetFinalStartX()+1,
              newSeg.GetInitialStartY() > newSeg.GetFinalStartY()
              ? newSeg.GetInitialStartY()+1 : newSeg.GetFinalStartY()+1,
              uremb.timeInterval.end.ToDouble() };
        uremb.SetBBox(Rectangle<3>(true, min, max));
  }
}

/*
1.1.1 Function ~AddPreciseSegment()~

*/
void URegion2::AddPreciseSegment(PreciseMSegmentData newSeg)
{
  uremb.PutPreciseSegment(&preciseSegments, preciseSegments.Size(), newSeg);
}

/*
1.1.1 Function ~AddCoordinate()~

*/
void URegion2::AddCoordinate(int newCoord)
{
  uremb.PutCoordinate(
		&preciseCoordinates, preciseCoordinates.Size(), newCoord);
}





size_t URegionEmb2::HashValue() const{
  return ( timeInterval.start.HashValue() ^ timeInterval.end.HashValue() );
}






/*
1.1.1 Methods for database operators

1.1.1.1 Method ~BoundingBox()~

*/
const Rectangle<3> URegion2::BoundingBox(const Geoid* geoid /*=0*/) const {
	if (MR2_DEBUG) cerr << "URegion2::BoundingBox() called" << endl;
    if(geoid){
      cerr << __PRETTY_FUNCTION__ << ": Spherical geometry not implemented."
      << endl;
      assert( !geoid ); // TODO: implement spherical geometry case
    }
    return uremb.BoundingBox();
}

/*
1.1.1.1 Method ~TemporalFunction()~

*/
void URegion2::TemporalFunction(const Instant& t,
                               Region& res,
                               bool ignoreLimits ) const {
    if (MR2_DEBUG)
        cerr << "URegion2::TemporalFunction() called" << endl;
    res.Clear();
    if(     !IsDefined() || !t.IsDefined()) {
      res.SetDefined( false );
    } else {
      res.SetDefined( true );
      const mpq_class pt(0);
      uremb.TemporalFunction(&segments, &preciseSegments, 
		  &preciseCoordinates, &preciseInstants,
    		  t, pt, scaleFactor, res, ignoreLimits);
    }
}


/*
1.1.1.1 Method ~At()~

This method is not part of the master thesis but a stub is required to make
~URegion2~ non-abstract. To assure that accidential calls of this method are
recognized, its body contains a failed assertion.

*/
bool URegion2::At(const Region& val, TemporalUnit<Region>& result) const {
    if (MR2_DEBUG) cerr << "URegion2::At() called" << endl;
    assert( IsDefined() );
    assert( val.IsDefined() );

    cerr << __PRETTY_FUNCTION__ << " not implemented!" << endl;
    assert(false);

    URegion2* res = static_cast<URegion2*>(&result);
    res->SetDefined( false );

    return false;
}

/*
1.1.1.1 Method ~Passes()~

This method is not part of the master thesis but a stub is required to make
~URegion2~ non-abstract. To assure that accidential calls of this method are
recognized, its body contains a failed assertion.

*/
bool URegion2::Passes(const Region& val) const {
    if (MR2_DEBUG) cerr << "URegion2::At() called" << endl;
    assert( IsDefined() );
    assert( val.IsDefined() );

    cerr << __PRETTY_FUNCTION__ << " not implemented!" << endl;
    assert(false);

    return false;
}


/*
1.1.1.1 Method ~Clone()~

*/
URegion2* URegion2::Clone(void) const {
    if (MR2_DEBUG) cerr << "URegion2::Clone() called" << endl;

    URegion2* res = new URegion2( (unsigned int)0 );
    res->CopyFrom(this);
    res->SetDefined( this->del.isDefined );
    return res;
}

/*
1.1.1.1 Method ~CopyFrom()~

*/
void URegion2::CopyFrom(const Attribute* right) {
    if (MR2_DEBUG) cerr << "URegion2::CopyFrom() called" << endl;

    const URegion2* ur = (const URegion2*) right;

    *this = *ur;
    this->SetDefined( ur->IsDefined() );
}

/*
1.1.1.1 Assignment operator

*/
URegion2& URegion2::operator= ( const URegion2& U) {

   if(!U.IsDefined())
     {
       uremb = U.uremb;
       segments = U.segments;
       preciseSegments = U.preciseSegments;
       preciseCoordinates = U.preciseCoordinates;
       preciseInstants = U.preciseInstants;
       SetDefined(false);
       return *this;
     }
   timeInterval = U.timeInterval; // copy units copy of deftime!
   // copy bbox, timeInterval, segmentsNum and pInterval
   uremb = U.uremb;      
   uremb.SetStartPos(0); // set uremb.segmentsStartPos = 0
   scaleFactor = U.scaleFactor;
   URegionEmb2 Uuremb = U.uremb;
   uremb.pInterval = Uuremb.pInterval;
   del.isDefined = U.del.isDefined;

   int start = U.uremb.GetStartPos(); //this is always zero!
   int numsegs = U.uremb.GetSegmentsNum();
   segments.clean();
   segments.resize( U.uremb.GetSegmentsNum() );
   preciseSegments.resize( U.uremb.GetSegmentsNum() );
   preciseCoordinates.resize( U.uremb.GetSegmentsNum()*gmpCharNum*8);

   for( int i=0 ; i<numsegs ; i++ )
     {// copy single movingsegment
       MSegmentData2 seg;
       U.segments.Get(i+start, &seg);
       segments.Put(i, seg);
       PreciseMSegmentData pSeg;
       U.preciseSegments.Get(i+start, &pSeg);
       preciseSegments.Put(i, pSeg);
     }

   int numCoords = U.preciseCoordinates.Size();
   for (int i = 0; i < numCoords; i++)
   {
	   // copy single int
	   int coord;
	   U.preciseCoordinates.Get(i, &coord);
	   preciseCoordinates.Put(i, coord);
   }

   int numInsts = U.preciseInstants.Size();
   for (int i = 0; i < numInsts; i++)
   {
	   // copy single int
	   int inst;
	   U.preciseInstants.Get(i, &inst);
	   preciseInstants.Put(i, inst);
   }

   return *this;
}


/*
1.1.1 Functions for DbArray-Access

*/
int URegion2::NumOfFLOBs() const {

    return 4;
}

Flob* URegion2::GetFLOB(const int i) {

    assert(i >= 0 && i <= 3);

    if (i==0) {
    	return &segments;
    }
    else if (i == 1) {
    	return &preciseSegments;
    }
    else if (i == 2) {
    	return &preciseCoordinates;
    }
    else {
    	return &preciseInstants;
    }


}

/*
1.1.1 Access methods

*/
const int URegion2::GetScaleFactor() {
	return scaleFactor;
}

void URegion2::SetScaleFactor(int factor) {
	scaleFactor = factor;
}




/*
1.1.1 In- and Out-Functions

1.1.1.1 Function ~OutURegion2()~

*/
static ListExpr OutURegion2(ListExpr typeInfo, Word value) {

	if (MR2_DEBUG)
	         cerr << "OutURegion2() called" << endl;

	URegion2* ur = (URegion2*) value.addr;

    // check for undefined value
    if( !ur->IsDefined() )
      return (nl->SymbolAtom("undef"));

    ListExpr l =
        OutURegionEmbedded2(
            ur->GetEmbedded(),
            (DbArray<MSegmentData2>*) ur->GetFLOB(0),
            (DbArray<PreciseMSegmentData>*) ur->GetFLOB(1),
            (DbArray<int>*) ur->GetFLOB(2),
            (DbArray<int>*) ur->GetFLOB(3));

    return nl->TwoElemList(nl->IntAtom(ur->GetScaleFactor()), l);
}

/*
1.1.1.1 Function ~InURegion2()~

*/
static Word InURegion2(const ListExpr typeInfo,
                      const ListExpr instance,
                      const int errorPos,
                      ListExpr& errorInfo,
                      bool& correct) {

	if (MR2_DEBUG) cerr << "InURegion2() called" << endl;

    URegion2* ur = new URegion2((unsigned int)0);

    // Check for undefined value
	if ( listutils::isSymbolUndefined( instance ) )
	  {
		ur->SetDefined(false);
		correct = true;
		return SetWord ( ur );
	  }

	if (nl->ListLength(instance) != 2 
		|| !nl->IsAtom(nl->First(instance)) 
		|| nl->AtomType(nl->First(instance)) != IntType)
	{
		cerr << endl
		<< "--------------------------------------------------"
		<< endl
		<< "ListExpr not in format (<int> <unit-list>)"
		<< endl
		<< "--------------------------------------------------"
		<< endl;
		return SetWord(Address(0));
	}

	if (nl->IntValue(nl->First(instance)) < 1)
	{
	    cerr << endl
	    << "--------------------------------------------------"
	    << endl
	    << "First element scale factor must be greater than zero!"
	    << endl
	    << "--------------------------------------------------"
	    << endl;
	    return SetWord(Address(0));
	}
	ur->SetScaleFactor(nl->IntValue(nl->First(instance)));

    URegionEmb2* uremb =
        InURegionEmbedded2(
            nl->Second(instance),
            errorPos,
            errorInfo,
            (DbArray<MSegmentData2>*) ur->GetFLOB(0),
            (DbArray<PreciseMSegmentData>*) ur->GetFLOB(1),
            (DbArray<int>*) ur->GetFLOB(2),
            (DbArray<int>*) ur->GetFLOB(3),
            0);


    if (!uremb) {
    	cerr << "Could not grok list representation of region unit."
    	             << endl;
        delete ur;
        correct = false;
        return SetWord(Address(0));
    }

    ur->timeInterval = uremb->timeInterval;
    ur->SetEmbedded(uremb);
    ur->SetDefined(true);

    delete uremb;

    correct = true;
    return SetWord(Address(ur));
}




/*
1.1 Algebra Integration

1.1.1 Function ~URegionProperty2()~

*/
static ListExpr URegionProperty2() {

    ListExpr listrep = nl->TextAtom();
    nl->AppendText(listrep,
    		   "(<int> (<interval> <face>*)), where <int> "
                   "represents the scaleFactor, <interval> is "
                   "(<int> <int> <bool> <bool> <preciseInterval>) "
                   "and where <preciseInterval> is (<text> <text>) "
                   "or an empty list, "
                   "representing the initial and final instant. "
                   "<face> is (<outercycle> <holecycle>*), "
                   "where <outercycle> and <holecycle> are "
                   "(<int> <int> <int> <int> <precisePoint>), "
                   "and where <precisePoint> is "
                   "(<text> <text> <text> <text>) "
                   "or an empty list, representing "
                   "start X, start Y, end X and end Y values.");
    ListExpr example = nl->TextAtom();
    nl->AppendText(example,
		   "(10 ((0 10 TRUE TRUE ())"
		   "((((10 35 15 15 ())"
		   "(20 55 30 45 ())"
		   "(30 65 35 50 ())"
		   "(40 65 55 50 ())"
		   "(40 55 55 45 ())"
		   "(50 45 75 25 ())"
		   "(50 25 75 10 ())"
		   "(40 15 70 5 ())"
	    	   "(30 15 25 5 ()))"
	    	   "((20 30 30 20 ())"
		   "(20 40 30 30 ())"
		   "(30 40 40 30 ())"
		   "(30 30 40 20 ()))))))");
    ListExpr remarks = nl->TextAtom();
    nl->AppendText(remarks,
                   "All <holecycle> must be completely within "
                   "corresponding <outercylce>.");

    return
        nl->TwoElemList(
            nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")),
            nl->FiveElemList(
                nl->StringAtom("-> UNIT"),
                nl->StringAtom("(uregion2)"),
                listrep,
                example,
                remarks));
}

/*
1.1.1 Function ~CheckURegion2()~

*/
static bool CheckURegion2(ListExpr type, ListExpr& errorInfo) {
    if (MR2_DEBUG) cerr << "CheckURegion2() called" << endl;

    return nl->IsEqual(type, URegion2::BasicType());
}

/*
1.1.1 Function ~CreateURegion2()~

*/

static Word CreateURegion2(const ListExpr typeInfo) {
    if (MR2_DEBUG) cerr << "CreateURegion2() called" << endl;

    return (SetWord(new URegion2((unsigned int)0)));
}

/*
1.1.1 Function ~DeleteURegion2()~

*/
static void DeleteURegion2(const ListExpr typeInfo, Word& w) {
    if (MR2_DEBUG) cerr << "DeleteURegion2() called" << endl;

    URegion2 *ur = (URegion2*)w.addr;
    delete ur;
    w.addr = 0;
}

/*
1.1.1 Function ~CloseURegion2()~

*/
static void CloseURegion2(const ListExpr typeInfo, Word& w) {
    if (MR2_DEBUG) cerr << "CloseURegion2() called" << endl;

    delete (URegion2*) w.addr;
    w.addr = 0;
}

/*
1.1.1 Function ~CloneURegion2()~

*/
static Word CloneURegion2(const ListExpr typeInfo, const Word& w) {
    if (MR2_DEBUG) cerr << "CloneURegion2() called" << endl;

    return SetWord(((URegion2*)w.addr)->Clone());
}

/*
1.1.1 Function ~CastURegion2()~

*/
static void* CastURegion2(void* addr) {
    if (MR2_DEBUG) cerr << "CastURegion2() called" << endl;

    return new (addr) URegion2;
}

/*
1.1.1 Method ~Sizeof()~

*/
size_t URegion2::Sizeof() const {
    if (MR2_DEBUG) cerr << "URegion2::Sizeof() called" << endl;
    return sizeof(*this);
}

/*
1.1.1 Function ~SizeOfURegion2()~

*/
static int SizeOfURegion2() {
    if (MR2_DEBUG) cerr << "SizeOfURegion2() called" << endl;

    return sizeof(URegion2);
}

/*
1.1.1 Function ~OpenURegion2()~

*/
bool OpenURegion2(SmiRecord& rec,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& w) {
    if (MR2_DEBUG) cerr << "OpenURegion2() called" << endl;

    w = SetWord(Attribute::Open(rec, offset, typeInfo));

    return true;
}

/*
1.1.1 Function ~SaveURegion2()~

Makes sense on ~URegion2~ instances with own segment storage only and will
run into failed assertion for other instances.

*/
static bool SaveURegion2(SmiRecord& rec,
                        size_t& offset,
                        const ListExpr typeInfo,
                        Word& w) {
    if (MR2_DEBUG) cerr << "SaveURegion2() called" << endl;

    URegion2* ur = static_cast<URegion2*> (w.addr);
    Attribute::Save(rec, offset, typeInfo, ur);

    return true;
}

/*
1.1.1 Type constructor ~uregion2~

*/
static TypeConstructor uregion2(
    "uregion2",
    URegionProperty2,
    OutURegion2,
    InURegion2,
    0, 0, // SaveToList, RestoreFromList
    CreateURegion2,
    DeleteURegion2,
    OpenURegion2,
    SaveURegion2,
    CloseURegion2,
    CloneURegion2,
    CastURegion2,
    SizeOfURegion2,
    CheckURegion2);



/*
1 Data type ~mregion2~

1.1 Class ~MRegion2~

1.1.1 Class definition

The class definition has been moved to ~MovingRegion2Algebra.h~.

1.1.1 Constructors

*/

MRegion2::MRegion2(const int n) :
    Mapping<URegionEmb2, Region>(n),
    msegmentdata(n),
    preciseSegmentData(n),
    preciseCoordinates(0),
    preciseInstants(0){
	if (MR2_DEBUG)
	        cerr << "MRegion2::MRegion2(int) called" << endl;
}



/*
This constructor creates a MRegion2 object from an existing MRegion object, by splitting the double
coordinates into an integer part and the remaining rest, which will be stored as mpq\_class values in
PreciseMSegmentData instances...
In a first step, the double coordinates are multiplied with scaleFactor to get the integer grid values.

*/
MRegion2::MRegion2(MRegion& coarseRegion, const int scaleFactor) :
	Mapping<URegionEmb2, Region>(coarseRegion.GetNoComponents()),
	msegmentdata(coarseRegion.GetMSegmentData()->Size()),
	preciseSegmentData(coarseRegion.GetMSegmentData()->Size()),
	preciseCoordinates(0),
	preciseInstants(0){

	if (MR2_DEBUG)
		cerr << "MRegion2::MRegion2"
		<< "(coarseRegion, scaleFactor) called" << endl
		<< "ScaleFactor: " << scaleFactor << " " << endl
		<< "Number of Units: " << coarseRegion.GetNoComponents() 
		<< " " << endl
		<< "Number of segments: " 
		<< coarseRegion.GetMSegmentData()->Size() << " " << endl;

	if (scaleFactor < 1)
	{
		cerr << endl
		<< "--------------------------------------------------"
		<< endl
		<< "Scale factor must be greater than zero!"
		<< endl
		<< "--------------------------------------------------"
		<< endl;
		SetDefined(false);
		return;
	}

	this->scaleFactor = scaleFactor;

	//For each of the units of coarseRegion
	for (int i = 0; i < coarseRegion.GetNoComponents(); i++)
	{
		URegionEmb origUremb;
		coarseRegion.Get(i, origUremb);

		if (MR2_DEBUG)
			cerr << "MRegion2::MRegion2, handling unit " 
				<< i << " of coarseRegion" << endl;

//Transform the time interval to integer grid coordinates - 
//preciseInterval is calculated in URegionEmb2-Constructor
		double initial = (double)broughtDown
			(origUremb.timeInterval.start.ToDouble());
		double final = (double)broughtDown
			(origUremb.timeInterval.end.ToDouble());
		Instant initInst(initial);
		Instant finInst(final);
		Interval<Instant> interval(initInst, finInst, 
			origUremb.timeInterval.lc, 
			origUremb.timeInterval.rc);

		//initialize precise time interval
		PreciseInterval pInt(-1);

		URegionEmb2 uremb(&msegmentdata, &preciseSegmentData, 
				&preciseCoordinates, &preciseInstants,
				origUremb.timeInterval, pInt, origUremb, 
				coarseRegion.GetMSegmentData(), 
				origUremb.GetStartPos(), scaleFactor);

		this->Put(i, uremb);
	}

}



/*
1.1.1 Attribute access methods

Get ~URegionEmb2~ unit ~i~ from this ~MRegion2~ instance and return it in ~ur~.

*/

void MRegion2::Get(const int i, URegionEmb2& ur) const {
    Mapping<URegionEmb2, Region>::Get(i, ur);
}

const DbArray<MSegmentData2>* MRegion2::GetMSegmentData2(void) {
    return &msegmentdata;
}

const DbArray<PreciseMSegmentData>* MRegion2::GetPreciseMSegmentData(void) {
	return &preciseSegmentData;
}

const DbArray<int>* MRegion2::GetPreciseCoordinates(void) {
	return &preciseCoordinates;
}

const DbArray<int>* MRegion2::GetPreciseInstants(void) {
	return &preciseInstants;
}

const int MRegion2::GetScaleFactor(void) {
	return scaleFactor;
}

void MRegion2::SetScaleFactor(int factor) {
	scaleFactor = factor;
}

/*
1.1.1 Methods for database operators

1.1.1.1 Method ~Transform()~

*/
void MRegion2::Transform(double deltaX, double deltaY) {

	if (MR2_DEBUG)
		cerr << "MRegion2::Transform() called" << endl;

	//for every unit
	for (int i = 0; i < this->GetNoComponents(); i++)
	{
		URegionEmb2 uremb;
		this->Get(i, uremb);

		uremb.Transform(deltaX, deltaY, &msegmentdata, 
			&preciseSegmentData, &preciseCoordinates);
	}

}

/*
1.1.1.1 Method ~Scale()~

*/
void MRegion2::Scale(double deltaX, double deltaY) {

	if (MR2_DEBUG)
		cerr << "MRegion2::Scale() called" << endl;

	if (deltaX <= 0 || deltaY <= 0)
	{
		cerr << endl
		<< "--------------------------------------------------"
		<< endl
		<< "Scale factors must be greater than zero!"
		<< endl
		<< "--------------------------------------------------"
		<< endl;
		SetDefined(false);
		return;
	}

	//for every unit
	for (int i = 0; i < this->GetNoComponents(); i++)
	{
		URegionEmb2 uremb;
		this->Get(i, uremb);

		uremb.Scale(deltaX, deltaY, &msegmentdata, 
			&preciseSegmentData, &preciseCoordinates);
	}

}
/*
1.1.1.1 Method ~AtInstant()~

*/
void MRegion2::AtInstant(const Instant& t, Intime<Region>& result) {
    if (MR2_DEBUG) cerr << "MRegion2::AtInstant() called" << endl;

    assert(IsOrdered() && t.IsDefined());

    int pos = Position(t );

    if( pos == -1 )
    {
    	//try with precise coordinates...
    	Instant help((double)broughtDown(t.ToDouble()));
    	pos = Position(help );
    	if (pos == -1)
    	    result.SetDefined(false);
    }
    if (pos != -1)
    {
        URegionEmb2 posUnit;
        Get(pos, posUnit);
        result.SetDefined(true);
        const mpq_class pt(0);
        posUnit.TemporalFunction(&msegmentdata, &preciseSegmentData, 
			&preciseCoordinates, &preciseInstants,
        		t, pt, scaleFactor, result.value);
        result.instant.CopyFrom(&t);

        if (!result.value.IsDefined())
        	result.SetDefined(false);
  }
}

/*
1.1.1.1 Method ~Initial()~

*/

void MRegion2::Initial(Intime<Region>& result) {
    if (MR2_DEBUG) cerr << "MRegion2::Initial() called" << endl;

    if ( !IsDefined() || IsEmpty() ) {
      result.SetDefined(false);
      return;
    }
    result.SetDefined(true);

    assert(IsOrdered());
    URegionEmb2 unit;
    Get(0, unit);

    if (!unit.timeInterval.lc) {
        result.SetDefined(false);
        return;
    }

    result.value.Clear();
    result.value.SetDefined(true);
    result.SetDefined(true);
    unit.TemporalFunction(
        &msegmentdata,
        &preciseSegmentData,
        &preciseCoordinates,
        &preciseInstants,
        unit.timeInterval.start,
        unit.pInterval.GetPreciseInitialInstant(&preciseInstants),
        scaleFactor,
        result.value);

    Instant inst(unit.timeInterval.start.ToDouble()+
        unit.pInterval.GetPreciseInitialInstant(
        &preciseInstants).get_d());
    result.instant.CopyFrom(&inst);
}

/*
1.1.1.1 Method ~Final()~

*/

void MRegion2::Final(Intime<Region>& result) {
    if (MR2_DEBUG) cerr << "MRegion2::Final() called" << endl;

    if ( !IsDefined() || IsEmpty() ) {
        result.SetDefined(false);
        return;
    }
    result.SetDefined(true);
    assert(IsOrdered());

    URegionEmb2 unit;
    Get(GetNoComponents()-1, unit);

    if (!unit.timeInterval.rc) {
        result.SetDefined(false);
        return;
    }

    result.value.Clear();
    result.value.SetDefined(true);
    result.SetDefined(true);
    unit.TemporalFunction(
        &msegmentdata,
        &preciseSegmentData,
        &preciseCoordinates,
        &preciseInstants,
        unit.timeInterval.end,
        unit.pInterval.GetPreciseFinalInstant(&preciseInstants),
        scaleFactor,
        result.value);

    Instant inst(unit.timeInterval.end.ToDouble()+
		unit.pInterval.GetPreciseFinalInstant(
		&preciseInstants).get_d());
	result.instant.CopyFrom(&inst);
}







/*
1.1.1 Methods for algebra integration

1.1.1.1 Method ~Clone()~

*/

MRegion2* MRegion2::Clone(void) const {
    if (MR2_DEBUG) cerr << "MRegion2::Clone() called" << endl;

    MRegion2* res = new MRegion2(0);
    res->CopyFrom(this);
    return res;
}

/*
1.1.1.1 Method ~CopyFrom()~

*/
void MRegion2::CopyFrom(const Attribute* right) {
    if (MR2_DEBUG)
        cerr << "MRegion2::CopyFrom() called, this="
             << this
             << ", &msegmentdata="
             << &msegmentdata
             << endl;

    MRegion2* mr = (MRegion2*) right;

    Clear();
    if( !mr->IsDefined() ){
      SetDefined( false );
      return;
    }
    SetDefined( true );
    if (MR2_DEBUG)
        cerr << "MRegion2::CopyFrom() called, &mr->msegmentdata="
             << &mr->msegmentdata
             << endl;

/*
~InMRegion2()~ sorts the region units, therefore the following assumption
should hold.

*/
    assert(mr->IsOrdered());

/*
Copy the units and assure that their pointer to the ~DBAarry~ containing
their moving segments points to this instance's ~DBArray~.

*/
    StartBulkLoad();
    for(int i = 0; i < mr->GetNoComponents(); i++) {
      URegionEmb2 ur;
      mr->Get(i, ur);
      Add(ur);
    }
    EndBulkLoad(false);

/*
Copy the moving segments.

*/
    msegmentdata.clean();
    if (mr->msegmentdata.Size() > 0)
      msegmentdata.resize(mr->msegmentdata.Size());
    for (int i = 0; i < mr->msegmentdata.Size(); i++) {
        MSegmentData2 dms;
        mr->msegmentdata.Get(i, dms);

        if (MR2_DEBUG)
            cerr << "MRegion2::CopyFrom() segment "
                 << i
                 << ": initial=["
                 << dms.GetInitialStartX()
                 << " " << dms.GetInitialStartY()
                 << " " << dms.GetInitialEndX()
                 << " " << dms.GetInitialEndY()
                 << "] final=["
                 << dms.GetFinalStartX()
                 << " " << dms.GetFinalStartY()
                 << " " << dms.GetFinalEndX()
                 << " " << dms.GetFinalEndY()
                 << "]"
                 << endl;

        msegmentdata.Put(i, dms);

    }
/*
Also copy the precise moving segments and the precise intervals.

*/
     preciseSegmentData.clean();
     if (mr->preciseSegmentData.Size() > 0)
     {
    	 preciseSegmentData.resize(mr->preciseSegmentData.Size());
     }
     for (int i = 0; i < mr->preciseSegmentData.Size(); i++) {
    	 PreciseMSegmentData pdms;
    	 mr->preciseSegmentData.Get(i, pdms);

    	 if (MR2_DEBUG)
    		 cerr << "MRegion2::CopyFrom(), copy precise segment " 
			<< i << endl;

    	 preciseSegmentData.Put(i, pdms);
     }

     preciseCoordinates.clean();
     if (mr->preciseCoordinates.Size() > 0)
     {
    	 preciseCoordinates.resize(mr->preciseCoordinates.Size());
     }
     for (int i = 0; i < mr->preciseCoordinates.Size(); i++) {
    	 int coord;
    	 mr->preciseCoordinates.Get(i, coord);
    	 preciseCoordinates.Put(i, coord);
     }

     preciseInstants.clean();
     if (mr->preciseInstants.Size() > 0)
     {
    	 preciseInstants.resize(mr->preciseInstants.Size());
     }
     for (int i = 0; i < mr->preciseInstants.Size(); i++)
     {
    	 int inst;
    	 mr->preciseInstants.Get(i, inst);
    	 preciseInstants.Put(i, inst);
     }

     if (MR2_DEBUG)
    	 cerr << "copied " << preciseCoordinates.Size() 
		<< " coordinate's chars and "
    	 	<< preciseInstants.Size() << " instant's chars" << endl;

/*
And finally don't forget the scale factor!

*/
     scaleFactor = mr->GetScaleFactor();
}

/*
1.1.1.1 ~DBArray~ access

*/
int MRegion2::NumOfFLOBs() const {

    return 5;
}

Flob* MRegion2::GetFLOB(const int i) {

    assert(i >= 0 && i <= 4);

    if (i == 0)
    {
    	return Mapping<URegionEmb2, Region>::GetFLOB(0); //Units-FLOB
    }
    else if (i == 1)
    {
    	return &msegmentdata; //segments in the integer-grid
    }
    else if (i == 2)
    {
    	return &preciseSegmentData; //precise segments
    }
    else if (i == 3)
    {
    	return &preciseCoordinates; //coordinates of precise segments
    }
    else
    {
    	return &preciseInstants; //coordinates of precise interval
    }

}



/*
1.1 In- and Out-Functions

1.1.1 Function ~OutMRegion2()~

*/
static ListExpr OutMRegion2(ListExpr typeInfo, Word value) {

	if (MR2_DEBUG)
		cerr << "OutMRegion2() called" << endl;

    MRegion2* mr = (MRegion2*) value.addr;

    if (mr->IsEmpty()) return (nl->TheEmptyList());

    assert(mr->IsOrdered());

    ListExpr l = nl->TheEmptyList();
    ListExpr lastElem;

    //Do this for every unit of the Moving Region:
    for (int i = 0; i < mr->GetNoComponents(); i++) {

        URegionEmb2 ur;
        mr->Get(i, ur);

        ListExpr unitList =
            OutURegionEmbedded2(
                &ur,
                (DbArray<MSegmentData2>*) mr->GetFLOB(1),
                (DbArray<PreciseMSegmentData>*) mr->GetFLOB(2),
                (DbArray<int>*) mr->GetFLOB(3),
                (DbArray<int>*) mr->GetFLOB(4));

        if (l == nl->TheEmptyList()) {
            l = nl->Cons(unitList, nl->TheEmptyList());
            lastElem = l;
        } else
            lastElem = nl->Append(lastElem, unitList);
    }
    return nl->TwoElemList(nl->IntAtom(mr->GetScaleFactor()), l);
}



/*
1.1.1 Function ~InMRegion2()~

*/
Word InMRegion2(const ListExpr typeInfo,
                      const ListExpr instance,
                      const int errorPos,
                      ListExpr& errorInfo,
                      bool& correct) {

	if (MR2_DEBUG) cerr << "InMRegion2() called" << endl;
    MRegion2* mr = new MRegion2(0);

    mr->StartBulkLoad();

/*
First get the scaleFactor from the list.

*/
	if (!nl->IsEmpty(instance))
	{
		if (nl->ListLength(instance) != 2 
		|| !nl->IsAtom(nl->First(instance)) 
		|| nl->AtomType(nl->First(instance)) != IntType)
		{
		    cerr << endl 
		    << "--------------------------------------------------" 
		    << endl
		    << "ListExpr is not in correct format "
		    << "(<int> (u1 u2 ... un))." 
		    << endl
		    << "First element must be atom of IntType and "
		    << "number of elements must be 2." 
		    << endl
		    << "--------------------------------------------------" 
		    << endl;
		    return SetWord(Address(0));
		}
		if (nl->IntValue(nl->First(instance)) < 1)
		{
		    cerr << endl 
		    << "--------------------------------------------------" 
		    << endl
		    << "First element scale factor must be greater "
		    << "than zero!" 
		    << endl
		    << "--------------------------------------------------"
		    << endl;
		    return SetWord(Address(0));
		}
		mr->SetScaleFactor(nl->IntValue(nl->First(instance)));
	}
	else {
	    cerr << endl 
	    << "--------------------------------------------------" 
	    << endl
	    << "ListExpr is not in correct format (<int> (u1 u2 ... un))"
	    << " but is empty!" 
	    << endl
	    << "--------------------------------------------------" << endl;
	    return SetWord(Address(0));
	}


    ListExpr rest = nl->Second(instance);
    //Get the units one by one from the ListExpr
    while(!nl->IsEmpty(rest)) {
        ListExpr first = nl->First(rest);
        rest = nl->Rest(rest);

        URegionEmb2* ur =
            (URegionEmb2*) InURegionEmbedded2(
                first,
                errorPos,
                errorInfo,
                &mr->msegmentdata,
                &mr->preciseSegmentData,
                &mr->preciseCoordinates,
                &mr->preciseInstants,
                mr->msegmentdata.Size());
        if (!ur) {
            correct = false;
            mr->Destroy();
            delete mr;
            return SetWord(Address(0));
        }
        mr->Add(*ur);
        delete ur;
    }

    mr->EndBulkLoad(true);

    if (mr->IsValid()) {
        correct = true;
        return SetWord(mr);
    } else {
        correct = false;
        mr->Destroy();
        delete mr;
        return SetWord(Address(0));
    }
}





/*
1.1 Algebra integration

1.1.1 Function ~MRegion2Property()~

*/
static ListExpr MRegion2Property() {

    ListExpr listrep = nl->TextAtom();
    nl->AppendText(listrep,
           "(<int> (u1 ... un)) with ui uregion2 list representations, "
           "and n >= 1. The <int> value is the scaleFactor for "
	   "transformation "
	   "of coordinates to double. Each ui is of format "
           "(<interval> <faces>), where <interval> is "
           "(<int> <int> <bool> <bool> <preciseInterval>) and where "
	   "<preciseInterval> is (<text> <text>) or an empty list, "
	   "representing precise initial and final instant. "
           "Each <face> in <faces> is (<outercycle> <holecycle>*), "
           "where <outercycle> and <holecycle> are "
           "(<int> <int> <int> <int> <precisePoint>), and where "
           "<precisePoint> is (<text> <text> <text> <text>) or "
           "an empty list, and the <int> and <text> values representing "
           "initial start X, initial start Y, final start X "
	   "and final start Y values.");
    ListExpr example = nl->TextAtom();

    nl->AppendText(example,
    	   "(1000 (((0 10 TRUE TRUE ())"
		   "((((10 35 15 15 ())"
		   "(20 55 30 45 ())"
		   "(30 65 35 50 ())"
		   "(40 65 55 50 ())"
		   "(40 55 55 45 ())"
		   "(50 45 75 25 ())"
		   "(50 25 75 10 ())"
		   "(40 15 70 5 ())"
    	   "(30 15 25 5 ()))"
		   "((20 30 30 20 ())"
		   "(20 40 30 30 ())"
		   "(30 40 40 30 ())"
		   "(30 30 40 20 ())))))))");

    return
        nl->TwoElemList(
            nl->FourElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List")),
            nl->FourElemList(
                nl->StringAtom("-> MAPPING"),
                nl->StringAtom("(mregion2)"),
                listrep,
                example));
}


/*
1.1.1 Function ~CheckMRegion2()~

*/
static bool CheckMRegion2(ListExpr type, ListExpr& errorInfo) {
	if (MR2_DEBUG) cerr << "CheckMRegion2() called" << endl;
	return nl->IsEqual(type, MRegion2::BasicType())
	        || nl->IsEqual(type, "movingregion2");
}







/*
1.1.1 Function ~OpenMRegion2()~

*/
bool OpenMRegion2(SmiRecord& rec,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& w) {
	if (MR2_DEBUG) cerr << "OpenMRegion2() called" << endl;

    w = SetWord(Attribute::Open(rec, offset, typeInfo));

    return true;
}

/*
1.1.1 Function ~SaveMRegion2()~

*/
static bool SaveMRegion2(SmiRecord& rec,
                        size_t& offset,
                        const ListExpr typeInfo,
                        Word& w) {

	if (MR2_DEBUG) cerr << "SaveMRegion2() called" << endl;

    MRegion2* mr = static_cast<MRegion2*> (w.addr);
    Attribute::Save(rec, offset, typeInfo, mr);

    return true;
}

/*
1.1.1 Function ~CreateMregion2~

*/
static Word CreateMRegion2(const ListExpr typeInfo) {
	if (MR2_DEBUG) cerr << "CreateMRegion2() called" << endl;

	return SetWord(new MRegion2(0));
}

/*
1.1.1 Function ~DeleteMregion2~

*/
static void DeleteMRegion2(const ListExpr typeInfo, Word& w) {
	if (MR2_DEBUG) cerr << "DeleteMRegion2() called" << endl;

  ((MRegion2*) w.addr)->Destroy();
  delete (MRegion2*) w.addr;
  w.addr = 0;
}

/*
1.1.1 Function ~CloseMregion2~

*/
static void CloseMRegion2(const ListExpr typeInfo, Word& w) {
	if (MR2_DEBUG) cerr << "CloseMRegion2() called" << endl;

  delete (MRegion2*) w.addr;
  w.addr = 0;
}

/*
1.1.1 Function ~CloneMregion2~

*/
static Word CloneMRegion2(const ListExpr typeInfo, const Word& w) {
	if (MR2_DEBUG) cerr << "CloneMRegion2() called" << endl;

  return SetWord(((MRegion2*) w.addr)->Clone());
}

/*
1.1.1 Function ~CastMregion2~

*/
static void* CastMRegion2(void* addr) {
	if (MR2_DEBUG) cerr << "CastMRegion2() called" << endl;

  return new (addr) MRegion2;
}

/*
1.1.1 Function ~SizeOfMregion2~

*/
static int SizeOfMRegion2() {
	if (MR2_DEBUG) cerr << "SizeOfMRegion2() called" << endl;

  return sizeof(MRegion2);
}

/*
1.1.1  Type constructor ~mregion2~

*/
static TypeConstructor movingregion2(
	MRegion2::BasicType(),
    MRegion2Property,
    OutMRegion2,
    InMRegion2,
    0, 0, // SaveToList, RestoreFromList
    CreateMRegion2, DeleteMRegion2,
    OpenMRegion2, SaveMRegion2,
    CloseMRegion2, CloneMRegion2,
    CastMRegion2,
    SizeOfMRegion2,
    CheckMRegion2);






/*
1.1 Helper function(s)

Create ~MPoint~ instance from intervals in units in ~MRegion2~ instance
~mr~ and the constant point ~p~.

*/

static MPoint CreateMPointFromPoint(MRegion2* mr, Point* p) {
    if (MR2_DEBUG) cerr << "CreateMPointFromPoint() called" << endl;

    MPoint mp(0);
    if( !mr->IsDefined() || !p->IsDefined() ){
      mp.SetDefined( false );
    } else {
      mp.SetDefined( true );
      for (int i = 0; i < mr->GetNoComponents(); i++) {
        URegionEmb2 ur;

        mr->Get(i, ur);

        UPoint up(ur.timeInterval,
                  p->GetX(), p->GetY(),
                  p->GetX(), p->GetY());

        mp.Add(up);
      }
    }
    return mp;
}





/*
1 Operator definition

1.1 Type mapping functions

1.1.1 Generic

Used by ~intersection~:

*/

static ListExpr MPointMRegion2ToMPointTypeMap(ListExpr args)
{
    if (MR2_DEBUG)
    {
        cerr << "MPointMRegion2ToMPointTypeMap() called" << endl;
        cerr << nl->SymbolValue(nl->First(args)) << endl;
        cerr << nl->SymbolValue(nl->Second(args)) << endl;
    }

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), MPoint::BasicType())
        && nl->IsEqual(nl->Second(args), MRegion2::BasicType()))
        return nl->SymbolAtom(MPoint::BasicType());
    else
        return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
Used by ~inside~:

*/

static ListExpr MPointMRegion2ToMBoolTypeMap(ListExpr args) {
    if (MR2_DEBUG)
        cerr << "MPointMRegion2ToMBoolTypeMap() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), MPoint::BasicType())
        && nl->IsEqual(nl->Second(args), MRegion2::BasicType()))
        return nl->SymbolAtom(MBool::BasicType());
    else
        return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
Used by ~initial~ and ~final~:

*/
static ListExpr MRegion2ToIRegion2TypeMap(ListExpr args) {
    if (MR2_DEBUG)
        cerr << "MRegion2ToIRegion2TypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), MRegion2::BasicType()))
        return nl->SymbolAtom(IRegion2::BasicType());
    else
        return nl->SymbolAtom(Symbol::TYPEERROR());
}



/*
Used by ~atinstant~:

*/
static ListExpr AtInstantTypeMap(ListExpr args) {
    if (MR2_DEBUG)
        cerr << "AtInstantTypeMap() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), MRegion2::BasicType())
        && nl->IsEqual(nl->Second(args), Instant::BasicType()))
        return nl->SymbolAtom(IRegion2::BasicType());
    else if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), URegion2::BasicType())
        && nl->IsEqual(nl->Second(args), Instant::BasicType()))
        return nl->SymbolAtom(IRegion2::BasicType());
    else
        return nl->SymbolAtom(Symbol::TYPEERROR());
}



/*
Used by ~mregiontomregion2~:

*/
static ListExpr MRegionToMRegion2TypeMap(ListExpr args){
	if (MR2_DEBUG) cout << "MRegionToMRegion2TypeMap called " << endl;
	if (nl->ListLength(args)!=2){
		ErrorReporter::ReportError("invalid number of arguments");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if(!nl->IsEqual(nl->First(args),MRegion::BasicType())){
		ErrorReporter::ReportError(
			"MRegion as first argument required");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if(!nl->IsEqual(nl->Second(args),CcInt::BasicType())){
		ErrorReporter::ReportError(
			"int as second argument required");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if (MR2_DEBUG) cout << "Typemap returns mregion2" << endl;
	return nl->SymbolAtom(MRegion2::BasicType());
}

/*
Used by ~uregiontouregion2~:

*/
static ListExpr URegionToURegion2TypeMap(ListExpr args) {
	if (MR2_DEBUG) cout << "URegionToURegion2TypeMap called " << endl;
	if (nl->ListLength(args) != 2) {
		ErrorReporter::ReportError("invalid number of arguments");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if (!nl->IsEqual(nl->First(args), URegion::BasicType())) {
		ErrorReporter::ReportError(
			"URegion as first argument required");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if (!nl->IsEqual(nl->Second(args), CcInt::BasicType())) {
		ErrorReporter::ReportError(
			"int as second argument required");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if (MR2_DEBUG) cout << "Typemap returns uregion2" << endl;
	return nl->SymbolAtom(URegion2::BasicType());
}

/*
Used by ~scale~:

*/
static ListExpr ScaleTypeMap(ListExpr args){
	if (MR2_DEBUG) cout << "ScaleTypeMap called " << endl;
	if (nl->ListLength(args) != 3){
		ErrorReporter::ReportError("invalid number of arguments");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if (!nl->IsEqual(nl->First(args),MRegion2::BasicType())){
		ErrorReporter::ReportError(
			"MRegion2 as first argument required");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if (!nl->IsEqual(nl->Second(args),CcReal::BasicType())){
		ErrorReporter::ReportError(
			"double as second argument required");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if (!nl->IsEqual(nl->Third(args),CcReal::BasicType())){
		ErrorReporter::ReportError(
			"double as third argument required");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if(MR2_DEBUG) cout << "Typemap returns mregion2" << endl;
	return nl->SymbolAtom(MRegion2::BasicType());
}

/*
Used by ~transform~:

*/
static ListExpr TransformTypeMap(ListExpr args){
	if (MR2_DEBUG) cout << "TransformTypeMap called " << endl;
	if (nl->ListLength(args) != 3){
		ErrorReporter::ReportError("invalid number of arguments");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if (!nl->IsEqual(nl->First(args),MRegion2::BasicType())){
		ErrorReporter::ReportError(
			"MRegion2 as first argument required");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if (!nl->IsEqual(nl->Second(args),CcReal::BasicType())){
		ErrorReporter::ReportError(
			"double as second argument required");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if (!nl->IsEqual(nl->Third(args),CcReal::BasicType())){
		ErrorReporter::ReportError(
			"double as third argument required");
		return nl->SymbolAtom(Symbol::TYPEERROR());
	}
	if(MR2_DEBUG) cout << "Typemap returns mregion2" << endl;
	return nl->SymbolAtom(MRegion2::BasicType());
}




/*
1.1 Selection functions

1.1.1 Generic

Used by ~intersection~ and ~inside~:

*/

static int MPointMRegion2Select(ListExpr args) {
    if (MR2_DEBUG)
        cerr << "MPointMRegion2Select() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->SymbolValue(nl->First(args)) == MPoint::BasicType()
        && nl->SymbolValue(nl->Second(args)) == MRegion2::BasicType())
        return 0;
    else
        return -1;
}

/*
Used by ~initial~, ~final~, ~deftime~ and ~traversed~:

*/

static int MRegion2Select(ListExpr args) {
    if (MR2_DEBUG)
        cerr << "MRegion2Select() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->SymbolValue(nl->First(args)) == MRegion2::BasicType())
        return 0;
    else
        return -1;
}




/*
Used by ~atinstant~:

*/

static int AtInstantSelect(ListExpr args) {
    if (MR2_DEBUG)
        cerr << "AtInstantSelect() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->SymbolValue(nl->First(args)) == MRegion2::BasicType()
        && nl->SymbolValue(nl->Second(args)) == Instant::BasicType())
        return 0;
    else if (nl->ListLength(args) == 2
        && nl->SymbolValue(nl->First(args)) == URegion2::BasicType()
        && nl->SymbolValue(nl->Second(args)) == Instant::BasicType())
        return 1;
    else
        return -1;
}



/*
1.1 Value mapping functions

1.1.1 Normal value mapping functions

*/


static int AtInstantValueMap_URegion2(Word* args,
                                     Word& result,
                                     int message,
                                     Word& local,
                                     Supplier s) {
    if (MR2_DEBUG) cerr << "AtInstantValueMap_URegion2() called" << endl;

    result = qp->ResultStorage(s);
    Intime<Region>* res = (Intime<Region>*) result.addr;
    URegion2* ur = (URegion2*) args[0].addr;
    Instant* inst = (Instant*) args[1].addr;

    if (ur->IsDefined()) {
        ur->TemporalFunction(*inst, res->value);
        if ((res->value).IsDefined())
        {
          if (MR2_DEBUG) cerr << "success!" << endl;
        	res->instant.CopyFrom(inst);
            res->SetDefined(true);
        }
        else
        {
          if (MR2_DEBUG) cerr << "Not defined..." << endl;
          res->SetDefined(false);
        }
    } else {
        res->SetDefined(false);
    }
    return 0;
}




static int MRegionToMRegion2ValueMap(Word* args,
				 Word& result,
				 int message,
				 Word& local,
				 Supplier s) {
	result = qp->ResultStorage(s);
	MRegion* mr = (MRegion*) args[0].addr;
	int factor = StdTypes::GetInt(args[1]);

	MRegion2 res(*mr, factor);

	((MRegion2*)result.addr)->CopyFrom(&res);
	res.Destroy();
	return (0);
}

static int URegionToURegion2ValueMap(Word* args,
				Word& result,
				int message,
				Word& local,
				Supplier s) {
	result = qp->ResultStorage(s);
	URegion* ur = (URegion*) args[0].addr;
	int factor = StdTypes::GetInt(args[1]);

	if (factor < 1)
	{
		cerr << endl
		<< "--------------------------------------------------"
		<< endl
		<< "Scale factor must be greater than zero!"
		<< endl
		<< "--------------------------------------------------"
		<< endl;
		((URegion2*)result.addr)->SetDefined(false);
		return 0;
	}

	URegion2 res(*ur, factor);

	((URegion2*)result.addr)->CopyFrom(&res);
	return(0);
}

static int ScaleValueMap(Word* args,
			 Word& result,
			 int message,
			 Word& local,
			 Supplier s) {
	result = qp->ResultStorage(s);
	MRegion2* mr = (MRegion2*) args[0].addr;
	CcReal* deltaX = static_cast<CcReal*>(args[1].addr);
	CcReal* deltaY = static_cast<CcReal*>(args[2].addr);

	MRegion2 res(true);
	res.CopyFrom(*&mr);

	res.Scale(deltaX->GetRealval(), deltaY->GetRealval());
	((MRegion2*)result.addr)->CopyFrom(&res);
	return (0);
}

static int TransformValueMap(Word* args,
			 Word& result,
			 int message,
			 Word& local,
			 Supplier s) {
	result = qp->ResultStorage(s);
	MRegion2* mr = (MRegion2*) args[0].addr;

	CcReal* deltaX = static_cast<CcReal*>(args[1].addr);
	CcReal* deltaY = static_cast<CcReal*>(args[2].addr);
	MRegion2 res(true);
	res.CopyFrom(*&mr);

	res.Transform(deltaX->GetRealval(), deltaY->GetRealval());
	((MRegion2*)result.addr)->CopyFrom(&res);
	return (0);
}


/*
1.1 Value mapping arrays

*/

static ValueMapping atinstantvaluemap[] =
    { MappingAtInstant<MRegion2, Region>,
      AtInstantValueMap_URegion2 };

static ValueMapping initialvaluemap[] =
    { MappingInitial<MRegion2, URegionEmb2, Region> };

static ValueMapping finalvaluemap[] =
    { MappingFinal<MRegion2, URegionEmb2, Region> };


/*
1.1 Operator specifications

*/
static const string atinstantspec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(mregion2 instant) -> iregion2, "
    "    (uregion2 instant) -> iregion2,</text--->"
    "    <text>_ atinstant _ </text--->"
    "    <text>Get the iregion2 value corresponding "
    "to the instant.</text--->"
    "    <text>mregion2_1 atinstant instant1</text---> ) )";

static const string initialspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mregion2 -> iregion2</text--->"
    "    <text>initial( _ )</text--->"
    "    <text>Get the iregion2 value corresponding to "
    "the initial instant."
    "    </text--->"
    "    <text>initial( mregion2_1 )</text---> ) )";

static const string finalspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mregion2 -> iregion2</text--->"
    "    <text>final( _ )</text--->"
    "    <text>Get the iregion2 value corresponding to "
    "the final instant."
    "    </text--->"
    "    <text>final( mregion2_1 )</text---> ) )";

static const string mregiontomregion2spec =
	"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
	" ( <text>mregion x int -> mregion2</text--->"
	"   <text>mregiontomregion2( _, _)</text--->"
	"   <text>Creates a moving region2 from the given moving region"
	" using the int value as scale factor </text--->"
	"   <text>query mregiontomregion2(mr, sfac)</text--->) )";

static const string uregiontouregion2spec =
	"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
	" ( <text>uregion x int -> uregion2</text--->"
	"   <text>uregiontouregion2(_, _)</text--->"
	"   <text>Creates a moving region2 unit"
	" from the given moving region unit"
	" using the int value as scale factor </text--->"
	"   <text>query uregiontouregion2(ur, sfac)</text--->) )";

static const string scalespec =
	"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
	" ( <text>mregion2 x double x double -> mregion2</text--->"
	"   <text>scale( _, _, _)</text--->"
	"   <text>Changes a given moving region2 through scaling,"
	" using the double values as scale factors for x and y </text--->"
	"   <text>query scale(mr2, deltax, deltay)</text--->) )";

static const string transformspec =
	"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
	" ( <text>mregion2 x double x double -> mregion2</text--->"
	"   <text>transform( _, _, _)</text--->"
	"   <text>Changes a given moving region2 through transforming"
	" using the double values to transform each point"
	" in x and y direction </text--->"
	"   <text>query transform(mr2, deltax, deltay)</text--->) )";





/*
1.1 Operator creation

*/

static Operator atinstant("atinstant",
                          atinstantspec,
                          2,
                          atinstantvaluemap,
                          AtInstantSelect,
                          AtInstantTypeMap);

static Operator initial("initial",
                        initialspec,
                        1,
                        initialvaluemap,
                        MRegion2Select,
                        MRegion2ToIRegion2TypeMap);

static Operator final("final",
                      finalspec,
                      1,
                      finalvaluemap,
                      MRegion2Select,
                      MRegion2ToIRegion2TypeMap);

static Operator mregiontomregion2("mregiontomregion2",
			mregiontomregion2spec,
			MRegionToMRegion2ValueMap,
			simpleSelect,
			MRegionToMRegion2TypeMap);

static Operator uregiontouregion2("uregiontouregion2",
		uregiontouregion2spec,
		URegionToURegion2ValueMap,
		simpleSelect,
		URegionToURegion2TypeMap);

static Operator scale("scale",
		scalespec,
		ScaleValueMap,
		simpleSelect,
		ScaleTypeMap);

static Operator mra2transform("mra2transform",
		transformspec,
		TransformValueMap,
		simpleSelect,
		TransformTypeMap);


/*
1 Algebra creation

*/

class MovingRegion2Algebra : public Algebra {
public:
  MovingRegion2Algebra() : Algebra() {
    AddTypeConstructor(&intimeregion2);
    AddTypeConstructor(&uregion2);
    AddTypeConstructor(&movingregion2);

    intimeregion2.AssociateKind(Kind::TEMPORAL());
    intimeregion2.AssociateKind(Kind::DATA());

    uregion2.AssociateKind(Kind::TEMPORAL());
    uregion2.AssociateKind(Kind::DATA());

    movingregion2.AssociateKind(Kind::TEMPORAL());
    movingregion2.AssociateKind(Kind::DATA());


    AddOperator(&atinstant);
    AddOperator(&initial);
    AddOperator(&final);
    AddOperator(&mregiontomregion2);
    AddOperator(&uregiontouregion2);
    AddOperator(&scale);
    AddOperator(&mra2transform);
    //AddOperator(&addURegion2);

  }
  ~MovingRegion2Algebra() {}
};


extern "C"
Algebra* InitializeMovingRegion2Algebra(NestedList* nlRef,
	 QueryProcessor *qpRef) {
  nl = nlRef;
  qp = qpRef;
  return new MovingRegion2Algebra();
}

