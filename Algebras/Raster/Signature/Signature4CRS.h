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

May, 2007 Leonardo Azevedo, Rafael Brand

*/
//---------------------------------------------------------------------------
#ifndef Assinat4CRSH
#define Assinat4CRSH

#include "../RTree/BBox.h"
#include "../RTree/Plane.h"
//#include "../RTree/coord.h"
//#include "Estruturas.h"
  // Full, Strong, Weak, Empty
//#define TRES_BITS_A
  // Full, Strong3, Strong2, Strong1, Weak3, Weak2, Weak1, Empty
//#define TRES_BITS_B
  // Full, Strong, WeakNO, WeakNE, WeakSE, WeakSO, Weak, Empty
//#define TRES_BITS_C
  // Full, Strong, WeakN, WeakS, WeakE, WeakO, Weak, Empty

typedef enum { SIGNAT_5CDRS, SIGNAT_4CRS , SIGNAT_5C_POLILINHA,
                SIGNAT_4CDRS, SIGNAT_4CDRS_DE_4CRS,
                SIGNAT_3CRS } SignatureType ;
#define FACTOR_AREA_STRONG  0.75   //  0.7734  0.7828
#define FACTOR_AREA_WEAK  0.25  //   0.2235  0.2261
#define MAX_DIMENSION_GRID                 255
#define FALSE_HIT          0
#define HIT                1
#define INCONCLUSIVE       2
typedef unsigned char uchar;

#define APPROXIMATE_AREA

#define N_BITS_COLORS 2

#define N_CORES      (1 << N_BITS_COLORS)

#define MAX_BITS_NUMERATOR ((PRODUCT + sizeof( GroupOfBits ) * 8 - 1)
#define MAX_BITS_DENOMINATOR (sizeof( GroupOfBits ) * 8))
#define MAX_BITS MAX_BITS_NUMERATOR / MAX_BITS_DENOMINATOR

typedef unsigned long GroupOfBits;

extern long sameScales, differentScales;

class Signature4CRS
{
  public:
    //--- Types ---------------------------------------------------------

    enum Weight { Empty = 0x0, Weak = 0x1,
                    Strong = 0x2, Full = 0x3 };

    struct RasterMap4CRS
    {
      struct Header {
                          unsigned long id;
                          MBR mbr;
                          unsigned dx : 12;
                          unsigned dy : 12;
                          unsigned potency : 8;

                          Header()
                          {}

                          Header( unsigned long id,
                                     MBR mbr,
                                     unsigned dx,
                                     unsigned dy,
                                     unsigned potency ) :
                          id( id ),
                          mbr( mbr ),
                          dx( dx ),
                          dy( dy ),
                          potency( potency )
                          {}

                        } header;
      GroupOfBits bits0[ MAX_BITS ];
      GroupOfBits bits1[ MAX_BITS ];

      RasterMap4CRS()
      {}

      RasterMap4CRS( unsigned long id,
                      MBR mbr,
                      unsigned dx,
                      unsigned dy,
                      unsigned potency ) :
      header( id,mbr,dx,dy,potency )
      {}

      void block( unsigned x, unsigned y, Weight weight );
      Weight block( int x, int y ) const;
      
      void setGroupOfBits(Weight);

    };

    //--- Atributos -----------------------------------------------------
    //const RasterMap4CRS::Header& map;
    RasterMap4CRS::Header* map;
    long sizeOfBlock;
    // the size of the block.

    //--- Constructors and Destructors --------------------------------------
    Signature4CRS( ){};
    Signature4CRS( RasterMap4CRS& source );
    Signature4CRS( unsigned long id,  Coordinate min, Coordinate max,
                unsigned long sizeOfBlock, unsigned dx, unsigned dy,
                const Weight weight[] );
    Signature4CRS( unsigned long id, MBR mbr, unsigned dx,
                    unsigned dy, unsigned potency, void* buffer1,
                    long bufSize );

    //--- Selectors -----------------------------------------------------
    const RasterMap4CRS& fullMap() const;
    Weight block( int x, int y ) const;
      // Returns the weight of the block ([0..dx-1]x[0..dy-1]).
    Weight block( Coordinate min, unsigned long sizeOfBlock ) const;
    Weight block( long minX, long minY, unsigned long sizeOfBlock ) const;
      // Returns the weight given the Coordinate and the size of the block.
    void mapBlock( unsigned x, unsigned y, Weight weight );

    //--- Operators ----------------------------------------------------
    int operator == ( const Signature4CRS& ) const;
    int operator != ( const Signature4CRS& ) const;
    Signature4CRS& operator=(const Signature4CRS&);

    Signature4CRS::Weight resolutionChange(int x, int y,
                        unsigned long sizeOfBlock) const;

    void setRaster(RasterMap4CRS map) {
      attr_map = map;
      //this->map = &(map.header);
      this->map = new RasterMap4CRS::Header( map.header.id, map.header.mbr,
                       map.header.dx, map.header.dy, map.header.potency ) ;
    }
  private:
    //--- Attributes -----------------------------------------------------
    RasterMap4CRS attr_map;
};

inline const Signature4CRS::RasterMap4CRS& Signature4CRS::fullMap() const
{
  return attr_map;
}

inline void Signature4CRS::mapBlock( unsigned x, unsigned y, Weight weight )
{
  attr_map.block(x, y, weight);
}

inline Signature4CRS::Weight Signature4CRS::block( int x, int y ) const
{
  return attr_map.block( x, y );
}

inline Signature4CRS::Signature4CRS( RasterMap4CRS& source ) :
//map( &(attr_map.header) ),
sizeOfBlock( 0x1lu << source.header.potency  ),
attr_map( source )
{
  map = new RasterMap4CRS::Header( source.header.id, source.header.mbr, 
                  source.header.dx, source.header.dy, source.header.potency );
}

//---------------------------------------------------------------------------

#endif
 
