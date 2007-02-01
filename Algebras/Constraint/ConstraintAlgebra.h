/*
----
This file is part of SECONDO.

Copyright (C) 2006, University in Hagen, Department of Computer Science,
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

[1] Header File of the Constraint Algebra

January, 2007 Simon Muerner

[TOC]

1 Overview

This header file essentially contains the definition of the class Constraint.
This class corresponds to the memory representation of the type constructor
~constraint~ which represents a 2-dimensional (potentially infinite) point set.


2 Defines and includes

*/
#ifndef __CONSTRAINT_ALGEBRA_H__
#define __CONSTRAINT_ALGEBRA_H__

using namespace std;

#include <cmath>
#include <string>
#include <vector>
#include "SpatialAlgebra.h"
#include "./triangulation/interface.h"

/*
The header of the triangulation library contains two important things:

  * Declaration of the function ~triangulate\_polygon~

  * Definition of the max number of segments of the input (SEGSIZE\_TRIANGULATION)


*/


namespace Constraint {

// constants:
const string OP_EQ = "eq";
const string OP_LEQ = "leq";
const Rectangle<2> WORLD(true, -10000.0, 10000.0, -10000.0, 10000.0);
// Word-Coordinates (arbitionary big)
const int X = 0;
const int Y = 1;


/*
3 Auxiliary functions and structures

*/

// foreward declaration:
class LinearConstraint;

/*
3.1 Function ~AlmostEqual~

Returns ~true~ if ~d1~ and ~d2~ are nearly equal ( $\epsilon$-test with  $\epsilon$=FACTOR from the SpatialAlgebra).
This functions should be used for the comparison of two doubles.

*/
inline bool AlmostEqual( const double d1, const double d2 )
{
  return fabs(d1 - d2) < FACTOR;
}

/*
3.2 Structure ~Point2D~

This structure is used for all the computational geometry algorithms in the constraint algebra module and represents a simple point $P=(x,y)$.
The associated functions are self-explaining.

*/
struct Point2D
{
  Point2D( )
  {
    this->x = 0.0;
    this->y = 0.0;
  }
  Point2D( double xCord, double yCord )
  {
    this->x = xCord;
    this->y = yCord;
  }
  Point2D( const Point2D& otherPoint )
  {
    this->x = otherPoint.x;
    this->y = otherPoint.y;
  }

  Point2D& operator=( const Point2D& otherPoint )
  {
    this->x = otherPoint.x;
    this->y = otherPoint.y;
    return *this;
  }

  bool operator==( const Point2D& otherPoint ) const
  {
    return ((AlmostEqual(this->x, otherPoint.x)) &&
            (AlmostEqual(this->y, otherPoint.y)));
  }

  bool operator!=( const Point2D& otherPoint ) const
  {
    return ((!AlmostEqual(this->x, otherPoint.x)) ||
            (!AlmostEqual(this->y, otherPoint.y)));
  }

/*
  The following function (operator) is important for the generic sort-function from the C++ STL:

*/
  bool operator<( const Point2D& otherPoint ) const
  {
    if(AlmostEqual(this->x, otherPoint.x))
    {
      if(AlmostEqual(this->y, otherPoint.y))
      {
        return false;
      }
      else
      {
        return (this->y < otherPoint.y);
      }
    }
    else
    {
      return (this->x < otherPoint.x);
    }
  }

/*
  The following function returns ~true~ if the currenct point is on the same line
  as the to points ~p1~ and ~p2~:

*/
  bool OnSameLineAs( const Point2D& p1, const Point2D& p2 ) const
  {
    // Prerequisite: at minimum two of the points in {*this, p1, p2} are
    // not equal
    // Special case (vertical line)
    if(AlmostEqual(p1.x, p2.x) && AlmostEqual(p1.x, (*this).x))
    {
      return true;
    }
    else // if line, not a vertical line, {p1,p2,p3} doesn't have
         // all the same x-coordinates
    {
      // in order to calculate a line, we have to have two different points
      // pFirst, pSecond (different x-coordinates!)
      Point2D pFirst, pSecond, pThird;
      pFirst = p1;
      if(p1!=p2)
      {
        pSecond = p2;
        pThird = (*this);
      }
      else // p1!=p3
      {
        pSecond = (*this);
        pThird = p2;
      }

	  // if third Point is equal to first or second => true!
	  if(pThird==pFirst || pThird==pSecond)
      {
        return true;
      }
      else // there are 3 different points!
      {
	    if(AlmostEqual(pThird.x,pFirst.x))
		{
		  // because pFirst and pSecond are not on vertical line:
		  return false;
		}
		else
		{
		  double m1 = (pSecond.y-pFirst.y)/(pSecond.x-pFirst.x);
		  double m2 = (pThird.y-pFirst.y)/(pThird.x-pFirst.x);
		  if(AlmostEqual(m1,m2))
		  {
			 return true;
		  }
		  else
		  {
			 return false;
		  }
		}
	  }
    }
  }

/*
  The following function returns ~true~ if the currenct point is on the line segment
  with the end points ~p1~ and ~p2~:

*/
  bool OnSegment( const Point2D& p1, const Point2D& p2 ) const
  {
    // Prerequisite: the points in {p1, p2} are not equal
   if(!this->OnSameLineAs(p1, p2))
   {
     return false;
   }
   else
   {
   	 // {p1, p2, *this} are on same line
	 if(AlmostEqual(p1.x,p2.x))
	 {
	   // VERTICAL CASE:
       Point2D pSegmentUpp = p1;
       Point2D pSegmentLow = p2;
       if(p1.y < p2.y)
       {
         pSegmentUpp = p2;
         pSegmentLow = p1;
       }
       if((pSegmentLow.y < (*this).y ||
           AlmostEqual(pSegmentLow.y, (*this).y)) &&
           ((*this).y < pSegmentUpp.y ||
           AlmostEqual((*this).y, pSegmentUpp.y)))
       {
         return true;
       }
       else
       {
         return false;
       }
	 }
	 else
	 {
	   // NON-VERTICAL CASE:
       Point2D pSegmentLeft, pSegmentRight;
       if(p1.x < p2.x)
       {
         pSegmentLeft = p1;
         pSegmentRight = p2;

       }
       else
       {
         pSegmentLeft = p2;
         pSegmentRight = p1;
       }

       // => (pSegmentLeft.x!=pSegmentRight.x)

       if((pSegmentLeft.x < (*this).x ||
           AlmostEqual(pSegmentLeft.x, (*this).x)) &&
           ((*this).x < pSegmentRight.x ||
           AlmostEqual((*this).x, pSegmentRight.x)))
       {
         return true;
       }
       else
       {
         return false;
       }
	 }
   }
  }

  void PrintOut() const
  {
     cout << " (x,y) == (" << (*this).x << ", " << (*this).y <<
       ") " <<endl;
  }

  double x;
  double y;
};

/*
3.3 Structure ~VerticalTrapez~

This structure is used for the CPI-algorithm (convex polyon intersection)
and does define a vertical trapez with four points.

*/
struct VerticalTrapez
{
  VerticalTrapez& operator=( const VerticalTrapez& otherVerticalTrapez )
  {
    this->pUppLeft = otherVerticalTrapez.pUppLeft;
    this->pUppRight = otherVerticalTrapez.pUppRight;
    this->pLowLeft = otherVerticalTrapez.pLowLeft;
    this->pLowRight = otherVerticalTrapez.pLowRight;
    this->IsEmpty = otherVerticalTrapez.IsEmpty;
    return *this;
  }
  Point2D pUppLeft;
  Point2D pUppRight;
  Point2D pLowLeft;
  Point2D pLowRight;
  bool IsEmpty;
};

/*
3.4 Structure ~SymbolicTuple~

This structure is used for the internal (class) representation of a symbolic relation.
See also class ~SymbolicRelation~.

*/
struct SymbolicTuple
{
  inline SymbolicTuple& operator=( const SymbolicTuple& symTup )
  {
    this->startIndex = symTup.startIndex;
    this->endIndex = symTup.endIndex;
    this->mbbox = symTup.mbbox;
    this->isNormal = symTup.isNormal;
    return *this;
  }

  const Rectangle<2> BoundingBox() const
  {
    if(!this->mbbox.IsDefined())
    {
  	  return this->mbbox;
    }
    else
    {
      double minx = mbbox.MinD(0);
	  double maxx = mbbox.MaxD(0);
	  double miny = mbbox.MinD(1);
	  double maxy = mbbox.MaxD(1);
      return Rectangle<2>( true,
                       minx - FACTOR,
                       maxx + FACTOR,
                       miny - FACTOR,
                       maxy + FACTOR );
    }
  }

  void PrintOut() const
  {
     cout << " (startIndex, endIndex, isNormal) == ("
          << (*this).startIndex << ", "
          << (*this).endIndex  << ", "
          << (*this).isNormal << ") " <<  endl;
  }

  // datastructure:

  int startIndex;
  int endIndex;
/*
The two indexes refer to certain elements of the DBArray with the linear constraints.

*/
  Rectangle<2> mbbox;
/*
The minimum bounding box is used for fast computations.

*/
  bool isNormal;
};

/*
3.5 Function ~ToConstraint~ for a rectangle.

This is just a wrapper-function that calls the real ToConstraint-function
with the polygon that describes the rectangle ~mbbox~.

*/
void ToConstraint(const Rectangle<2>& mbbox,
                  vector<LinearConstraint>& vLinConstraints);

/*
3.5 Function ~ToConstraint~ for a poylgon.

Transforms a convex polygon in boundary-representation (list of vertices in clockwise-order)
to a list of linear constraints which describes the convex polygon in the same order.
Degenerated cases are possible: segment, point; not possible is the empty set as input.

*/
void ToConstraint(const vector<Point2D>& vConvexPolygon,
        vector<LinearConstraint>& vLinConstraints);

/*
3.6 Function ~ComputeXOrderedSequenceOfSlabs~

Compute an $x$-ordered sequence of slabs for the vertices of ~vP~'s upper and lower boundary.
This function is used for the CPI-algorithm (convex polygon intersection).

*/
void ComputeXOrderedSequenceOfSlabs(const vector<Point2D>& vP,
            vector<Point2D>& vUpperBoundary,
            vector<Point2D>& vLowerBoundary);

/*
3.7 Function ~MinimumBoundingBox~

Computes and returns the minimum bounding box of the polygon ~vConvexPolygon~.

*/
Rectangle<2> MinimumBoundingBox(const vector<Point2D>& vConvexPolygon);

/*
3.8 Function ~ConvexPolygonIntersection~

Computes the intersection of two polygons. ~vP~ and ~vQ~ are polygons,
each given as a sequence of vertices in clockwise order.
~vConvexPolygon~ is again in clockwise order.

*/
void ConvexPolygonIntersection(const vector<Point2D>& vP,
        const vector<Point2D>& vQ,
        vector<Point2D>& vPQIntersection);

/*
3.9 Function ~HalfPlaneIntersection~

Computes the half plane intersection of a set of linear constraints
(degenerated cases possible: unlimited objects, segment, point; not possible: empty set).
Returns a (possibly degenared) polygon.

*/
void HalfPlaneIntersection(
        const vector<LinearConstraint>& vLinConstraints,
        vector<Point2D>& vConvexPolygon);

/*
3.10 Function ~TriangulateRegion~

Computes a triangulation of the region (set of faces which are polygons).
Returns a (not ordered) set of triangles indexing to a set of vertices.

*/
void TriangulateRegion(const Region* reg,
  vector<vector<double> >& vVertices,
  vector<vector<int> >& vTriangles);

/*
3.11 Function ~GetTheta~

Computes a pseudo-angle $\theta$ between an anker point ~p1~ and another point ~p2~.
Returns a value between 0 and 360 which is not the real angle but has the same
order-relation.

This function is used for a fast and stable computations in order to sort
a set of objects by the pseudo-angle.

*/
double GetTheta(Point2D& p1, Point2D& p2);

/*
3.12 Function ~GetOrientedArea~

Computes the oriented area of three points.

*/
double GetOrientedArea(double px, double py,
             double vx, double vy,
             double qx, double qy);

/*
3.13 Function ~MergeTriangles2ConvexPolygons~

Given a triangulation of a polygon by its set of triangles, this function
tries to delete diagonals (merge step) if the merged components are still convex.
It's an implementation of the well-known Hertel-Melhorn algorithm.

*/
void MergeTriangles2ConvexPolygons(
  const vector<vector<double> >& vVertices,
  const vector<vector<int> >& vTriangles,
  vector<vector<Point2D> >& vCWPoints);

/*
3.14 Function ~GetIntersectionPoint~

Returns the intersection point of two linear constraints interpreted as
EQ-Constraints (using = as comparator).

Prerequisite: The constraints are not parallel or equal (otherwise they woudn't
intersection in one point).

*/
Point2D GetIntersectionPoint(const LinearConstraint& linConFirst,
  const LinearConstraint& linConSecond);

/*
3.15 Function ~GetPointFromSegment~

Returns the point $P_{cut} = (x_{cut}, y_{cut})$
with $x_{cut} = $ ~xCut~ and $P_{cut}$ is inside the segment defined with
the two end points ~pLeft~ and ~pRight~.

Prerequisite: ~pLeft.x~ $<$ ~xCut~ $<$ ~pRight.x~

*/
Point2D GetPointFromSegment(const double xCut,
      const Point2D& pLeft, const Point2D& pRight);

/*
3.16 Function ~PrintSlab~

Prints (standard-out) out a slab (just for debug-test).

*/
void PrintSlab(VerticalTrapez currentSlab);

/*
4 Class ~LinearConstraint~

A linear constraint (in 2D) is a predicate of the form: $a_{1}x_{1} + a_2x_{2} + b = 0$ or of the form
$a_{1}x_{1} + a_2x_{2} + b \leq 0$. Instances of this class are used by the class ~SymbolicRelation~.

*/
class LinearConstraint
{  public:
    LinearConstraint();
/*
The empty constructor.

*/
    LinearConstraint(
      double a1,
      double a2,
      double b,
      string strOp);
/*
This constructor can be used for the definition of a linear constraint by its coefficients and the comparison operator ('eq' for $=$ and 'leq' for $\leq$).

*/
    ~LinearConstraint();

    LinearConstraint* Clone();
    LinearConstraint& operator=(const LinearConstraint& linC);

    bool IsParallel(const LinearConstraint& linC);
/*
Returns ~true~ if the current linear constraint interpreted as
EQ-Constraints (using = as Comparator) is parallel to the linear constraint ~linC~.

*/
    bool IsEqual(const LinearConstraint& linC);
/*
Returns ~true~ if the current linear constraint does define the same pointset
(half-plane or line) as the linear constraint ~linC~.

*/

/*
Getter and Setter:

*/
    double Get_a1() const;
    double Get_a2() const;
    double Get_b() const;
    string Get_Op() const;
    void Set_a1(double a1);
    void Set_a2(double a2);
    void Set_b(double b);
    void Set_Op(string strOp);
    void PrintOut() const;
/*
Debug and Test-function

*/
  private:
/*
Attributes of a linear constraints:

*/
    bool defined;
    unsigned int dim;
    double a1;
    double a2;
    double b;
    char strOp[4];
};


/*
5 Class ~SymbolicRelation~

This class is used for the internal (class) representation of a ~constraint~ value.

A ~constraint~ value is a disjunction of a set of symbolic tuples.
A tuple is a conjunction of a set of linear constraints.
See class ~LinearConstraint~ for a definition of a linear constraint.

*/
class SymbolicRelation : public StandardSpatialAttribute<2>
{
  public:
    inline SymbolicRelation() {}
/*
The empty constructor should not be used.

*/
    SymbolicRelation(const int nConstraints, const int nTuple);
/*
This constructor constructs empty sets of tuples and linear constraints but
opens space for ~nConstraints~ linear Constraints and ~nTuple~ symbolic tuples.

*/
    SymbolicRelation(const SymbolicRelation& symRel);
/*
The copy constructor.

*/
	void Destroy();
/*
This function should be called before the destructor if one wants to destroy the
persistent array of tupels and the persistent array of linear constraints.
It marks the persistent array for destroying. The
destructor will perform the real destroying.

*/

    SymbolicRelation& operator=( const SymbolicRelation& symRel);
/*
Assignment operator redefinition.

*/
    void GetSymbolicTuples( const int i,
      SymbolicTuple const*& symbolicTuple) const;
/*
Retrieves the symbolic tuple at a position ~i~ in the set of symbolic tuples.

*/
    int LinConstraintsSize() const;
/*
Returns the size of the set of linear constraints.

*/
    int SymbolicTuplesSize() const;
/*
Returns the size of the set of symbolic tuples.

*/
    void GetLinConstraints( const int i,
      LinearConstraint const*& linearConstraint) const;
/*
Retrieves the linear constraint at a position ~i~ in the set of linear constraints.

*/
    void AddSymbolicTuple(const vector<LinearConstraint> vLinConstraints);
/*
Adds a symbolic tupel to the symbolic relation.
This new tupel is described with the set of linear constraints in ~vLinConstraints~.

*/
    void AppendSymbolicRelation(const SymbolicRelation& otherSymRel);
/*
Adds the symbolic tuples and linear constraints from the symbolic relation ~otherSymRel~ to the current symbolic relation.

*/
    void JoinSymbolicRelation(const SymbolicRelation& otherSymRel);
/*
Computes the join-operation with another symbolic relation ~otherSymRel~.
That means if the current symbolic relation does represent the set of points $P$
and the symbolic relation ~otherSymRel~ does represent the set of points $P_{other}$
then this function computes a symbolic relation $P := P \cap P_{other}$.

*/
    bool OverlapsSymbolicRelation(const SymbolicRelation& otherSymRel) const;
/*
Returns ~true~ if the current symbolic Relation does intersect with the symbolic relation ~otherSymRel~.
It's implemented with the early-stop mechanism.

*/
    void Normalize();
/*
Transforms the current symbolic relation into the normal form:

  * No tuple has any redundant linear constraint(s).

  * The order of the linear constraints is clockwise
  (two following linear constraints interpreted as EQ-Constraints (using = as Comparator) do intersect in a vertex of the described convex polygon for each tuple).

  * Each tuple does represent a (convex) pointset $\subseteq$ WORLD.

  * No tuple does represent the empty set.

*/
    void ProjectToAxis(const bool blnXAxis, const bool blnYAxis);
/*
Compute the projection to the x-axis (blnXAxis have to be ~true~) or to the y-axis (blnYAxis have to be ~true~).

*/

/*
Functions needed to import the ~constraint~ data type to (relational) tuple

There are totally 10 functions which are defined as virtual functions. They need
to be defined here in order for the ~constraint~ data type to be used in Tuple definition
as an attribute.

*/
    int NumOfFLOBs() const;
    FLOB* GetFLOB(const int);
    int Compare(const Attribute* arg) const;
    bool Adjacent(const Attribute* arg) const;
    SymbolicRelation* Clone() const;
    bool IsDefined() const;
    void SetDefined(bool Defined);
    size_t HashValue() const;
    size_t Sizeof() const;
    void CopyFrom(const StandardAttribute* right);

    const Rectangle<2> BoundingBox() const;
/*
Returns the minimum bounding box of the point set descibed by the symbolic relation.

*/

  private:
    DBArray<LinearConstraint> linConstraints;
/*
The persistent array of linear constraints.

*/
    DBArray<SymbolicTuple> symbolicTuples;
/*
The persistent array of symbolic tuples.

*/
    Rectangle<2> mbbox;
/*
The minimum bounding box of the point set descibed by the symbolic relation.

*/
};


} // namespace

#endif
