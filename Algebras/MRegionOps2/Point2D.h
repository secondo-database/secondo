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
//[pow] [\verb+^+]

[1] Headerfile 

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype

Oktober 2014 - Maerz 2015, S. Schroer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#ifndef POINT2D_H_
#define POINT2D_H_

#include <gmp.h>
#include <gmpxx.h>
#include <iostream>


using namespace std;

namespace mregionops2 {

/*

2.2 Forward declarations

*/

class Point3D;

/*

5 Class Point2D

*/

class Point2D {

public:
    
/*

5.1 Constructors

*/   

    inline Point2D() :
        x(0.0), y(0.0) {
    }

    inline Point2D(mpq_class _x, mpq_class _y) :
        x(_x), y(_y) {
    }

    Point2D(const Point3D& p);

    inline Point2D(Point2D p1, Point2D p2, mpq_class ratio)
    {
      x = p1.x + (p2.x - p1.x) * ratio;
      y = p1.y + (p2.y - p1.y) * ratio;
    }

/*

5.3 Operators and Predicates
    
5.3.1 Operators for comparison.

*/     
    
    inline bool operator ==(const Point2D& p) const {
       return (x == p.x) && (y == p.y);
    }   
    inline bool operator !=(const Point2D& p) const {
           return !(*this == p);
    }

/*

5.2 Getter and setter methods.

*/      

        inline mpq_class GetX() const {
               return x;
        }

         inline mpq_class GetW() const {
               return x;
        }

         inline mpq_class GetY() const {
               return y;
        }
         inline mpq_class GetT() const {
               return y;
        }
        
        inline bool operator <(const Point2D& p) const {
        if (x < p.x)
            return true;

        if (x > p.x)
            return false;
            
            return (y < p.y);
        }

        inline mpq_class WhichSide(const Point2D& start, 
                                   const Point2D& end) const {
          return (start.x - x) * (end.y - y) - 
          (end.x - x) * (start.y - y);
        }
    
        inline bool IsLeft(const Point2D& start, 
                           const Point2D& end) const {
          return ( WhichSide(start, end) > 0);
        }
    
        bool LiesBetween(Point2D p1, Point2D p2);

private:

    mpq_class x;
    mpq_class y;
};

ostream& operator <<(ostream& o, Point2D& p);


} 
#endif /* POINT2D_H_*/
