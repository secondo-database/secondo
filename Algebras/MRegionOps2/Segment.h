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

[1] Headerfile of the Segment classes

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype, 

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/


#ifndef SEGMENT_H_
#define SEGMENT_H_

#include "PointVector.h"
//#include "Point2D.h"
//#include "Point3D.h"
//#include "Vector2D.h"
//#include "Vector3D.h"



namespace mregionops2 {

/*
3 Class Segment3D

This class provides an oriented segment in the euclidian space.
It's start- and endpoint is represented by a Point3D each.

*/
class Segment3D {

public:
    
/*
3.1 Constructors

*/   

    inline Segment3D() {
        
    }

    inline Segment3D(const Point3D& _start, const Point3D& _end) :
        start(_start), end(_end) {

    }
    
/*
3.2 Getter methods.

*/

    inline const Point3D& GetStart() const {

        return start;
    }
    inline const Point3D& GetEnd() const {

        return end;
    }
    
/*
3.3 Operators and Predicates
        
3.3.1 IsOrthogonalToTAxis

Returns ~true~, if this is parallel to the xy-plane.

*/   

    inline const bool IsOrthogonalToTAxis() const {
          return (start.GetT() == end.GetT());
    }
    

private:

    Point3D start, end;
};

/*

4 Class Segment2D

This class provides an oriented segment in the euclidian plane.
It's start- and endpoint is represented by a Point2D each.

*/

class Segment2D {

public:
    
/*

4.1 Constructors

*/   
    
    inline Segment2D() {
        
    }

    inline Segment2D(const Point2D& p1, 
                     const Point2D& p2,
                     const bool sort = false) {

        if (sort && p2 < p1) {
            
            start = p2;
            end = p1;
            
        } else { // !sort || p1 <= p2
            
            start = p1;
            end = p2;
        }
    }
    
    inline Segment2D(const Segment3D& s) {
        
        start = Point2D(s.GetStart());
        end = Point2D(s.GetEnd());
    }
    
/*

4.2 Getter methods.

*/    
                         
    inline const Point2D& GetStart() const {

        return start;
    }

    inline const Point2D& GetEnd() const {

        return end;
    }
    
/*
4.3 Operators and Predicates
            
4.3.1 IsParallel

Returns ~true~, if this is parallel to s.

*/       

    inline bool IsParallel(const Segment2D& s) const {

        Vector2D v1 = end - start;
        Vector2D v2 = s.end - s.start;

    return ((v1.GetX() * v2.GetY()) - (v1.GetY() * v2.GetX())) == 0;
    }
    
/*

4.3.2 IsColinear

Returns ~true~, if this is colinear to s.

*/  
    
    inline bool IsColinear(const Segment2D& s) const {
        
        return start.IsColinear(s) && end.IsColinear(s);
    }

/*

4.3.3 IsVertical

Returns ~true~, if start.x equals end.x

*/  
    
   inline bool IsVertical() const {
    return (start.GetX() == end.GetX());
    }

/*

4.3.4 IsHorizontal

Returns ~true~, if start.y equals end.y

*/ 
    
    inline bool IsHorizontal() const {

     return (start.GetY() == end.GetY());

    }

/*

3.3.6 Length2
        
Returns the quadratic length of this segment.

*/   

    inline mpq_class Length2() const {

        return (end - start).Length2();
    }

/*

3.3.7 Flip
        
Swaps the start- and endpoint of this segment.

*/   
    
    inline void Flip() {
        
        Point2D temp = start;
        
        start = end;
        end = temp;
    }

private:

    Point2D start, end;
};

/*

4 Overloaded output operators
    
*/ 

ostream& operator <<(ostream& o, const Segment2D& s);
ostream& operator <<(ostream& o, const Segment3D& s);

} // end of namespace mregionops2

#endif /*SEGMENT_H_*/
