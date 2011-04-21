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

Class BoundingBox 2D: the MBR.

Original: Prof. Claudio Esperanca.
Adapted by Geraldo Zimbrao in 01/10/1997.
May, 2007 Leonardo Azevedo, Rafael Brand

*/


#ifndef BBOX_H
#define BBOX_H

//#include "gerpag.h"
#include "coord.h"

struct BBox2D : public rRectangle
{
  public:
    //--- Constructors ------------------------------------------------
    BBox2D() : rRectangle( Coordinate( 0, 0 ), Coordinate( -1, -1 ) )
      {}

    BBox2D( Coordinate min, Coordinate max ) : rRectangle( min, max )
      {}

    BBox2D( long p1[ 2 ], long p2[ 2 ] ) :
     rRectangle( Coordinate( MIN( p1[ 0 ], p2[ 0 ] ), MIN( p1[ 1 ], p2[ 1 ] ) ),
                Coordinate( MAX( p1[ 0 ], p2[ 0 ] ), MAX( p1[ 1 ], p2[ 1 ] ) ) )
      {}

    BBox2D( long p1x, long p1y, long p2x, long p2y ) :
      rRectangle( Coordinate( MIN( p1x, p2x ), MIN( p1y, p2y ) ),
                 Coordinate( MAX( p1x, p2x ), MAX( p1y, p2y ) ) )
      {}

    //--- Selectors ---------------------------------------------------
    inline int overlaps( const BBox2D& b ) const;
      // Returns true if this bounding box overlaps b.

    double area () const
      // Returns area of this bounding box.
      { return rectangleArea( min, max ); }

    double perimeter () const
      // Returns perimeter of this bounding box.
      { return 2 * ((max.x - min.x + 1) + (max.y - min.y + 1)); }

    static int dimension()
      { return 2; }
      // The dimension of this class

    long minD( int dim ) const
      // Returns the min coord value for the given dimension.
      { return ((long *) &min)[ dim ]; }

    long maxD( int dim ) const
      // Returns the max coord value for the given dimension.
      { return ((long *) &max)[ dim ]; }

    static inline BBox2D alignedMBR( BBox2D mbr, long celSize );
    // Returns the bounding box aligned in the grid

    static inline BBox2D aligned2MBR( BBox2D mbr, long power, long celSize,
                                     long dx, long dy );
    // Returns the bounding box aligned in the grid

    inline BBox2D unionWith( const BBox2D& b ) const;
      // Returns the bounding box that contains both this and b.

    inline BBox2D intersection( const BBox2D& b ) const;
      // bounding box of the intersection between this and b.

  private:
    //--- Seletores ---------------------------------------------------
    int proper() const
      // Returns True if this is a "proper" bounding box, i.e. it does not
      // represent an empty set.
      { return (min.x <= max.x) && (min.y <= max.y); }
};

typedef BBox2D Box;
typedef BBox2D MBR;

inline int operator == ( BBox2D a, BBox2D b );

//--- INLINES ---------------------------------------------------------

inline int BBox2D::overlaps( const BBox2D& b ) const
{
  //ASSERT_CONDICAO( proper() && b.proper(), ErroNaoInicializado );
  assert(proper() && b.proper());

  return existsIntersection( *this, b );
}

inline BBox2D BBox2D::unionWith( const BBox2D& b ) const
{
  if( !proper() ) return b;
  if( !b.proper() ) return *this;

  return BBox2D( Coordinate( MIN( min.x, b.min.x ), MIN( min.y, b.min.y ) ),
                 Coordinate( MAX( max.x, b.max.x ), MAX( max.y, b.max.y ) ) );
}

inline BBox2D BBox2D::intersection( const BBox2D& b ) const
{
  if( !proper() ) return *this;
  if( !b.proper() ) return b;

  return BBox2D( Coordinate( MAX( min.x, b.min.x ), MAX( min.y, b.min.y ) ),
                 Coordinate( MIN( max.x, b.max.x ), MIN( max.y, b.max.y ) ) );
}

inline BBox2D BBox2D::alignedMBR( BBox2D mbr, long celSize )
{
  Coordinate maxAligned( mbr.max.x < 0 ? mbr.max.x - celSize : mbr.max.x,
                          mbr.max.y < 0 ? mbr.max.y - celSize : mbr.max.y ),
             minAligned( mbr.min.x < 0 ? mbr.min.x - celSize : mbr.min.x,
                          mbr.min.y < 0 ? mbr.min.y - celSize : mbr.min.y );

  double dx = (maxAligned.x+1) / celSize - minAligned.x / celSize + 1,
         dy = (maxAligned.y+1) / celSize - minAligned.y / celSize + 1;

  long ldx = (long) dx, ldy = (long) dy;
  return BBox2D( Coordinate( (minAligned.x / celSize) * celSize,
                             (minAligned.y / celSize) * celSize ),
                 Coordinate( ((minAligned.x / celSize) + ldx) * celSize,
                             ((minAligned.y / celSize) + ldy) * celSize ) );
}

inline BBox2D BBox2D::aligned2MBR( BBox2D mbr, long power, long celSize,
                                 long dx, long dy)
{
   Coordinate min ( mbr.min.x & (0xFFFFFFFFlu << power),
                    mbr.min.y & (0xFFFFFFFFlu << power) );
  return BBox2D( min, Coordinate ( min.x + dx * celSize, min.y + dy * celSize));

}

//--------------------######

inline int operator == ( BBox2D a, BBox2D b )
{
  return (a.min == b.min) && (a.max == b.max);
}

struct BoxReal
{
  RealCoordinate min, max;

  BoxReal() : min(), max() {}
  BoxReal( RealCoordinate min, RealCoordinate max ) : min( min ), max( max ) {}
};

#endif // BBOX_H
