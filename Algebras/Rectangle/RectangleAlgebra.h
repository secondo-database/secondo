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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]

[1] Header File of the Rectangle Algebra

October, 2003 Victor Teixeira de Almeida

1 Overview

This header file essentially contains the definition of the struct ~Rectangle~.
This class correspond to the memory representation for the type constructor
~rect~, which is a 2-dimensional rectangle alligned with the axes x and y. A
rectangle in such a way can be represented by four numbers, the upper and lower
values for the two dimensions.

2 Defines and includes

*/
#ifndef __RECTANGLE_ALGEBRA_H__
#define __RECTANGLE_ALGEBRA_H__

#ifdef SECONDO_WIN32
#define Rectangle SecondoRectangle
#endif

#define MAX( a, b ) ((a) > (b) ? (a) : (b))
#define MIN( a, b ) ((a) < (b) ? (a) : (b))

/*
3 Struct ~Rectangle~

*/

class Point;

class Rectangle: public StandardAttribute
{
  public:

    Rectangle() {}
/*
Do not use this constructor.

*/

    Rectangle( const bool defined,
               const double bottom = 0,
               const double top = 0,
               const double left = 0,
               const double right = 0 );
/*
The constructor. First one can set if the rectangle is defined, and if it is,
the four coordinates can be set.

*/

    Rectangle( const Rectangle& r );
/*
The copy constructor.

*/

    bool IsDefined() const;
/*
Checks if the rectangle is defined.

*/
    Rectangle& operator = ( const Rectangle& r );
/*
Redefinition of operator ~=~.

*/

    bool Contains( const Point& p ) const;
/*
Checks if the rectangle contains the point ~p~.

*/

    bool Contains( const Rectangle& r ) const;
/*
Checks if the rectangle contains the rectangle ~r~.

*/

    bool Intersects( const Rectangle& r ) const;
/*
Checks if the rectangle intersects with rectangle ~r~.

*/

    bool operator == ( const Rectangle& r ) const;
/*
Redefinition of operator ~==~.

*/

    bool operator != ( const Rectangle& r ) const;
/*
Redefinition of operator ~!=~.

*/

    double Area() const;
/*
Returns the area of a rectangle.

*/

    double Perimeter() const;
/*
Returns the perimeter of a rectangle.

*/

    const double MinD( int dim ) const;
/*
Returns the min coord value for the given dimension ~dim~.

*/

    const double MaxD( int dim ) const;
/*
Returns the max coord value for the given dimension ~dim~.

*/

    double Left() const
      { return left; }
    double Right() const
      { return right; }
    double Bottom() const
      { return bottom; }
    double Top() const
      { return top; }
/*
Returns the four coordinates of the rectangle.

*/
    Rectangle& Translate( const double x, const double y );
/*
Translates the rectangle given ~x~ and ~y~ which can be negative values.

*/

    Rectangle Union( const Rectangle& b ) const;
/*
Returns the bounding box that contains both this and the rectangle ~r~.

*/

    Rectangle Intersection( const Rectangle& b ) const;
/*
Returns the intersection between this and the rectangle ~r~.

This next funnctions are necessary for a ~rectangle~ be
an attribute in the Relational Algebra.

*/
    void SetDefined( bool Defined )
      {}

    size_t HashValue()
      { return 0; }

    void CopyFrom( StandardAttribute* right )
      { *this = *((Rectangle*)right); }

    int Compare( Attribute *arg )
      { return 0; }

    bool Adjacent( Attribute *arg )
      { return false; }

    int Sizeof() const
      { return sizeof( Rectangle ); }

    Rectangle* Clone()
      { return new Rectangle( *this ); }

    ostream& Print( ostream &os )
      { if( IsDefined() )
          return os << "( " << left << ", " << right << ", " << bottom << ", " << top << " )";
        else
          return os << "undef";
      }

  private:

    bool Proper() const;
/*
Returns ~true~ if this is a "proper" rectangle.

*/
    bool defined;

    double bottom;
    double top;
    double left;
    double right;

//    Point min;
/*
The bottom left point of the rectangle.

*/
//    Point max;
/*
The top right point of the rectangle.

*/
};

#endif

