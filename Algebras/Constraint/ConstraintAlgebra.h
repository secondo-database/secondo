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

{\Large \bf Anhang B: Constraint-Template }

[1] Header File of the Constraint Algebra

July, 2006 Simon Muerner

[TOC]

1 Overview

This header file essentially contains the definition of the class Constraint.
This class corresponds to the memory representation for the type constructor
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
The header of the triangulation libary contains two important things:

  * Declaration of the function ~triangulate\_polygon~,

  * Definition of the max number of segments of the input (SEGSIZE\_TRIANGULATION).


*/


namespace Constraint {

// Konstanten und Enumerationen:
const string OP_EQ = "eq";
const string OP_LEQ = "leq";
// Word-Coordinates (arbitionary big):
const Rectangle<2> WORLD(true, -10000.0, 10000.0, -10000.0, 10000.0);
const int X = 0;
const int Y = 1;


/*
3 Auxilaries function, structures, classes

*/

// foreward declaration:
class LinearConstraint;

inline bool AlmostEqual( const double d1, const double d2 )
{
  return fabs(d1 - d2) < FACTOR;
}

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

  // wichtig fuer sort(...)-Funktion:
  bool operator<( const Point2D& otherPoint ) const
  {
    if(AlmostEqual(this->x, otherPoint.x))
    {
      // dann ist y-Koordinaten-Vergleich entschieden:
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
    { // dann ist x-Koordinante-Vergleich entscheiden:
      return (this->x < otherPoint.x);
    }
  }

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
      // in order to calculate a line, we have to have two diffrent points
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
      else // there are 3 diffrent points!
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

  // indexes to the DBArray of the linear Constraints:
  int startIndex;
  int endIndex;
  // other related data:
  Rectangle<2> mbbox;
  bool isNormal;
};

void ToConstraint(const Rectangle<2>&, vector<LinearConstraint>&);
void ToConstraint(const vector<Point2D>&, vector<LinearConstraint>&);
void ComputeXOrderedSequenceOfSlabs(const vector<Point2D>&,
            vector<Point2D>&,
            vector<Point2D>&);
Rectangle<2> MinimumBoundingBox(const vector<Point2D>&);
void ConvexPolygonIntersection(const vector<Point2D>&,
        const vector<Point2D>&,
        vector<Point2D>&);
void HalfPlaneIntersection(const vector<LinearConstraint>& ,
               vector<Point2D>&);
void TriangulateRegion(const Region*,
             vector<vector<double> >&,
             vector<vector<int> >&);
double GetTheta(Point2D&, Point2D&);
double GetOrientedArea(double, double, double, double, double, double);
void MergeTriangles2ConvexPolygons(const vector<vector<double> >&,
    const vector<vector<int> >&,
    vector<vector<Point2D> >&);
Point2D GetIntersectionPoint(const LinearConstraint&,
    const LinearConstraint&);
Point2D GetPointFromSegment(const double,
      const Point2D&, const Point2D&);
void PrintSlab(VerticalTrapez);



/*
4 Class ~LinearConstraint~ ...

*/
class LinearConstraint
{  public:
    LinearConstraint();
    LinearConstraint(double, double, double, string);
    ~LinearConstraint();
    LinearConstraint* Clone();
    LinearConstraint& operator=(const LinearConstraint&);
    bool IsParallel(const LinearConstraint&);
    bool IsEqual(const LinearConstraint&);
    double Get_a1() const;
    double Get_a2() const;
    double Get_b() const;
    string Get_Op() const;
    void Set_a1(double);
    void Set_a2(double);
    void Set_b(double);
    void Set_Op(string);
    void PrintOut() const;

  private:
    bool defined;
    unsigned int dim;
    double a1;
    double a2;
    double b;
    char strOp[4];
};


/*
5 Class ~SymbolicRelation~ ...

*/
class SymbolicRelation : public StandardSpatialAttribute<2>
{
  public:
    inline SymbolicRelation() {}
    SymbolicRelation(const int nConstraints, const int nTuple);
    SymbolicRelation(const SymbolicRelation&);
    SymbolicRelation& operator=( const SymbolicRelation&);
    void Destroy();
    void GetSymbolicTuples( const int, SymbolicTuple const*&) const;
    int LinConstraintsSize() const;
    int SymbolicTuplesSize() const;
    void GetLinConstraints( const int, LinearConstraint const*&) const;
    void AddSymbolicTuple(const vector<LinearConstraint>);
    void AppendSymbolicRelation(const SymbolicRelation&);
    void JoinSymbolicRelation(const SymbolicRelation&);
    bool OverlapsSymbolicRelation(const SymbolicRelation&) const;
    void Normalize();
    void ProjectToAxis(const bool, const bool);
    // The following functions are needed for the SymbolicRelation class
    // in order to act as an attribute of a relation:
    int NumOfFLOBs() const;
    FLOB* GetFLOB(const int);
    int Compare(const Attribute*) const;
    bool Adjacent(const Attribute*) const;
    SymbolicRelation* Clone() const;
    bool IsDefined() const;
    void SetDefined(bool);
    size_t HashValue() const;
    size_t Sizeof() const;
    void CopyFrom(const StandardAttribute*);
    const Rectangle<2> BoundingBox() const;

  private:
    DBArray<LinearConstraint> linConstraints;
    DBArray<SymbolicTuple> symbolicTuples;
    Rectangle<2> mbbox;
};


} // namespace

#endif
