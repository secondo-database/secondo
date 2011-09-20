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

This file was originally written by Christopher John Kline, under the copy
right statement below. Mahomud Sakr, September 2011, has made the necessary
changes to make it available as a SECONDO operator.

  Copyright (C) 1996, Christopher John Kline
  Electronic mail: ckline@acm.org

  This software may be freely copied, modified, and redistributed
  for academic purposes and by not-for-profit organizations, provided that
  this copyright notice is preserved on all copies, and that the source
  code is included or notice is given informing the end-user that the source
  code is publicly available under the terms described here.

  Persons or organizations wishing to use this code or any modified version
  of this code in a commercial and/or for-profit manner must contact the
  author via electronic mail (preferred) or other method to arrange the terms
  of usage. These terms may be as simple as giving the author visible credit
  in the final product.

  There is no warranty or other guarantee of fitness for this software,
  it is provided solely "as is". Bug reports or fixes may be sent
  to the author, who may or may not act on them as he desires.

  If you use this software the author politely requests that you inform him
  via electronic mail.


September 2011 Mahmoud Sakr: The Boids simulator/data-generator is now
available as a SECONDO operator.

*/

#ifndef _OBSTACLE_H
#define _OBSTACLE_H

#include <iostream>
#include "Vector.h"
using namespace std;

/*
information about a possible intersection of a ray with a polygonal obstacle

*/

struct ISectData {
  int intersectionflag;      // 1 = intersection did occur, 0 = didn't
  MathVector normal;         // normal at intersection
  MathVector point;          // point of intersection
};

class Obstacle;
class Polygon;
class Box;
class Sphere;
class ObstacleList;

//---------------- CLASS OBSTACLE ------------------------------

// A generic object class 

class Obstacle {

public:

  Obstacle(void) { _id = 0; }
  
  virtual ~Obstacle(void) { }

  virtual Obstacle *Clone(void) const = 0;
  // Interface to copy constructors for all derived classes. Returns a deep
  // copy of this obstacle.
  
  ISectData DoesRayIntersect(MathVector raydirection, MathVector rayorigin);
  // Does a ray intersect this obstacle if it projects from rayorigin in the
  // direction specified by raydirection? raydirection MUST NOT be (0,0,0) !
  
  virtual int getId(void) const;
  
  virtual void setId(int id);
  
  friend std::ostream &operator<<(std::ostream &strm, const Obstacle &o);
  
protected:

  virtual ISectData IntersectionWithRay(const MathVector & raydirection,
					const MathVector &rayorigin) const = 0;

  virtual std::ostream &Disp(std::ostream &strm) const = 0;

  int _id; // do anything you want with this (perhaps have "types" of objects)
};

//---------------- CLASS POLYGON ------------------------------

// A convex polygon object

class Polygon : public Obstacle {

public:
  
  Polygon(int numverts,
	  const MathVector v0, const MathVector v1, const MathVector v2, ...);
  // Constructor. Vertices must be specified in clockwise or
  // counter-clockwise order.
  
  Polygon(const Polygon &p); 
  // Copy constructor.
    
  ~Polygon(void);
  
  virtual Obstacle *Clone(void) const;  

  friend int operator==(const Polygon &a, const Polygon &b);  
  
protected:
  virtual ISectData IntersectionWithRay(const MathVector &raydirection,
					const MathVector &rayorigin) const;

  virtual std::ostream &Disp(std::ostream &strm) const;

private:
  Polygon(void) { };
  // Default constructor is private to forced users to specify vertices when
  // constructing.

  float d;         // plane of this polygon is: (normal dot v[0]) + d = 0 
  short i1, i2;    // indices of axis of closest major plane to polygon 
  MathVector normal;   // normal to this polygon
  int numvertices; // size of array of vertices 
  MathVector *vertex;  // array of vertices

};

//---------------- CLASS BOX ------------------------------

// A box-shaped object

class Box : public Obstacle {

public: 
  Box(const MathVector &topLeftBackCorner,
      const MathVector &bottomRightFrontCorner);
/*
Constructor.
When constructing the box, imagine that it is originally positioned
with the bottomRightFrontCorner at (0, 0, 0) and the topLeftBackCorner
at (width, height, length). The top and bottom sides are parallel to
the x-z plane; the left and right are parallel to the y-z plane, and
the back and front are parallel to the x-y plane.
Then it is translated by the amount (x, y, z) where x, y, and z are the
respective components of the bottomRightFrontCorner.

*/
  ~Box(void);

  virtual Obstacle *Clone(void) const;  

  Box(const Box &b);
  // copy constructor
  
  friend int operator==(const Box &a, const Box &b);  

  double Width();
  double Hight();
  double Length();
  MathVector LeftTopBack();
  MathVector RightBottomFront();
  inline Sphere* BoundingSphere(){ return boundingSphere;}

protected:
  virtual ISectData IntersectionWithRay(const MathVector &raydirection,
					const MathVector &rayorigin) const;

  virtual std::ostream &Disp(std::ostream &strm) const;

private:
  Box(void) { };
  // Default constructor is private to force users to specify vertices when
  // constructing.

  MathVector tlb, brf;  // topLeftBack and bottomRightFront corners of box.

  Polygon *side[6]; // 6 polygons comprising the box's sides

  Sphere *boundingSphere; // Sphere bounding the area of this box.
};

//---------------- CLASS SPHERE ------------------------------

// A spherical object

class Sphere : public Obstacle {

public:
  Sphere(const MathVector &sphereOrigin, const double sphereRadius);

  virtual Obstacle *Clone(void) const;
  
  friend int operator==(const Sphere &a, const Sphere &b);  

  inline double Radius() {return radius;}
  inline MathVector Origin() {return origin;}
protected:
  virtual std::ostream &Disp(std::ostream &strm) const;

  virtual ISectData IntersectionWithRay(const MathVector &raydirection,
					const MathVector &rayorigin) const;
    
private:
  Sphere(void) { };
  // Default constructor is private to force users to specify radius when
  // constructing.

  double radius;  // radius of sphere

  MathVector origin;  // center of sphere

};


//---------------- CLASS OBSTACLELIST------------------------------

// An iterable list of heterogenous objects 

class ObstacleList {  

public:
  Obstacle *Add(const Obstacle &o);

  Obstacle *Delete(Obstacle *o);

  Obstacle *Iter(void);

  void ResetIter(void);
  
  ObstacleList(void);

  ~ObstacleList(void);

private:
  
  struct obnode {
    Obstacle *obj;
    obnode *next;
    
    obnode(void) { 
      obj = NULL;
      next = NULL; 
    }
    ~obnode(void) { 
      if (obj != NULL) delete obj;
    }
  };
  
  obnode *head;
  obnode *iterptr;
  
};

// ----------------------------- -------------- --------------------------
// ----------------------------- inline methods --------------------------
// ----------------------------- -------------- --------------------------

// --- inline methods for Obstacle

inline ISectData
Obstacle::DoesRayIntersect(MathVector raydirection, MathVector rayorigin)
{
  return IntersectionWithRay(raydirection, rayorigin);
}

inline int
Obstacle::getId(void) const
{
  return _id;
}
  
inline void
Obstacle::setId(int id)
{
  _id = id;
}

inline std::ostream &
operator<<(std::ostream &strm, const Obstacle &o)
{
  return o.Disp(strm);
}

// --- inline methods for Polygon

inline
Polygon::~Polygon(void)
{
  delete[] vertex;
}

inline Obstacle *
Polygon::Clone(void) const
{
  return new Polygon(*this);
}

// --- inline methods for Box

inline
Box::~Box(void)
{
  for (int i = 0; i < 6; i++)
    delete side[i];

  delete boundingSphere;
}

inline Obstacle *
Box::Clone(void) const
{
  return new Box(*this);
}

inline int 
operator==(const Box &a, const Box &b) { 
  return (a.tlb == b.tlb && a.brf == b.brf);
}

inline std::ostream &
Box::Disp(std::ostream &strm) const {

  return strm << "[Box]"
	      << " TopLeftBack: " << tlb 
	      << " BottomRightFront: " << brf 
	      << " ";
}

// --- inline methods for Sphere

inline
Sphere::Sphere(const MathVector &sphereOrigin, const double sphereRadius)
{
  radius = sphereRadius;
  origin = sphereOrigin;
}

inline int 
operator==(const Sphere &a, const Sphere &b) { 
  return (a.radius == b.radius && a.origin == b.origin);
}

inline Obstacle *
Sphere::Clone(void) const
{
  return new Sphere(*this);
}   

inline std::ostream &
Sphere::Disp(std::ostream &strm) const {

  return strm << "[Sphere]"
	      << " origin: " << origin 
	      << " radius: " << radius 
	      << " ";;
}

// --- inline methods for ObstacleList

inline
ObstacleList::ObstacleList(void) {
    head = NULL;
}

inline
ObstacleList::~ObstacleList(void) {
  while (head) {
    Delete(head->obj);
  }
}

inline Obstacle *
ObstacleList::Iter(void) {

    obnode *foo = iterptr;

    if (foo != NULL) {
	iterptr = iterptr->next;
	return(foo->obj);
    }
    else {
      return NULL;
    }
}

inline void
ObstacleList::ResetIter(void)
{
  iterptr = head;
}

#endif	// #ifndef _OBSTACLE_H
