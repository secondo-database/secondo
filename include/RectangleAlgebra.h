/*
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

    Rectangle();
/*
The simple constructor. Create two undefined points.

*/

    Rectangle( const double bottom, const double top, const double left, const double right );
/*
The second constructor. Receives four coordinates.

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

    int Adjacent( Attribute *arg )
      { return 0; }

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

