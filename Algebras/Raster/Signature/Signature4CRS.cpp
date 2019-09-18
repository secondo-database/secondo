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

#include "Signature4CRS.h"

long sameScales = 0, differentScales = 0;

#define larger(a,b) ((a>b) ? a : b)
#define smaller(a,b) ((a<b) ? a : b)

Signature4CRS::Signature4CRS( unsigned long id, MBR mbr, unsigned dx,
                                unsigned dy, unsigned potency, void* buffer,
                                long bufSize ) :
//map( &(attr_map.header) ),
sizeOfBlock( 0x1lu << potency  ),
attr_map( id,mbr,dx,dy,potency )
{
  memcpy( attr_map.bits0, buffer, bufSize );
  map = new RasterMap4CRS::Header( id, mbr, dx, dy, potency ) ;
}

Signature4CRS::Signature4CRS( unsigned long id,  Coordinate min, Coordinate max,
                        unsigned long sizeOfBlock, unsigned dx,
                        unsigned dy, const Weight weight[] ) :
//map( &(attr_map.header) ),
sizeOfBlock( sizeOfBlock )
{
  //cout << "Signature4CRS::Signature4CRS 2" << endl;
  MBR mbr;
  mbr.min = min;
  mbr.max = max;
  unsigned potency = 0x1lu >> sizeOfBlock;
  map = new RasterMap4CRS::Header( id, mbr, dx, dy, potency ) ;
  register unsigned i, j, step;
  Weight current;
  GroupOfBits bits0 = 0, bits1 = 0;

  attr_map.header.id = id;
  attr_map.header.mbr.min = min;
  attr_map.header.mbr.max = max;
  attr_map.header.dx = dx;
  attr_map.header.dy = dy;

  for( i = 0; (sizeOfBlock >> i) != 0; i++ ){
    attr_map.header.potency = i - 1;
  }
  
  map->potency = attr_map.header.potency;
  
  for( i = 0; i < MAX_BITS; i++ )
  {
    attr_map.bits0[ i ] = attr_map.bits1[ i ] = 0;
  }

  for( step = 8 * sizeof( unsigned long ), i = j = 0; i < dx * dy; i++ )
  {
    step--;

    current = weight[ i ];
    bits0 |= ((current & 0x1lu) >> 0) << step;
    bits1 |= ((current & 0x2lu) >> 1) << step;

    if( step == 0 )
    {
      attr_map.bits0[ j ] = bits0;
      attr_map.bits1[ j ] = bits1;
      j++;
      step = 8 * sizeof( GroupOfBits );

      bits0 = bits1 = 0;
    }
  }

  if( step != 0 && j < MAX_BITS )
  {
    attr_map.bits0[ j ] = bits0;
    attr_map.bits1[ j ] = bits1;
  }
}

void Signature4CRS::RasterMap4CRS::setGroupOfBits(Weight o)
{
  memset(bits0,o,MAX_BITS*sizeof(GroupOfBits));
  memset(bits1,o,MAX_BITS*sizeof(GroupOfBits));
}

Signature4CRS::Weight Signature4CRS::RasterMap4CRS::block( int x, int y ) const
{
  if((x > -1 && y > -1) && ((unsigned)x < header.dx && (unsigned)y < header.dy))
  {
    unsigned index = x*header.dy + y,
             position = index / (8 * sizeof( GroupOfBits )),
             displacement = 8 * sizeof( GroupOfBits ) - 1 
             - (index % (8 * sizeof( GroupOfBits )));

    return (Weight) ((bits0[ position ] & (0x1lu << displacement) ? 0x1 : 0x0) |
                     (bits1[ position ] & (0x1lu << displacement) ? 0x2 : 0x0));

  }
  else {
    return Empty;
  }
}

Signature4CRS::Weight Signature4CRS::block( Coordinate min, 
           unsigned long sizeOfBlock ) const
{
  return block( min.x, min.y, sizeOfBlock );
}

Signature4CRS::Weight Signature4CRS::resolutionChange(int x, int y,
                        unsigned long sizeOfBlock) const
{
  double totalWeight = 0;
  int factor = (unsigned) (sizeOfBlock/this->sizeOfBlock),
          startX = x < 0 ? 0 : x,
          startY = y < 0 ? 0 : y,
          endX = x + factor > (int) map->dx ? map->dx : x + factor,
          endY = y + factor > (int) map->dy ? map->dy : y + factor,
          i, j;

  for( i = startX; i < endX; i++ )
    for( j = startY; j < endY; j++ )
    {
      Weight current = block( i, j );

      switch( current )
      {
        case Full : totalWeight  += 1; break;
        case Strong : totalWeight  += FACTOR_AREA_STRONG; break;
        case Weak : totalWeight  += FACTOR_AREA_WEAK; break;
        case Empty : break;
      }
    }
  unsigned long factor2 = (factor * (long) factor);
    // factor2: number of cells of the smaller size that fits
    // in a cell of the larger size

  double weight = double(totalWeight / factor2 * 100 );
  // Sets the weight of the group of cells according to the individual weight
  // of each cell (rounding the value)

  if ( weight == 0.0  ) return Signature4CRS::Empty;  // weight = 0
  if ( weight == 100.0  ) return Signature4CRS::Full;  // weight = 100
  if ( weight >  50.0 ) return Signature4CRS::Strong;  // weight (50,100)
  // if ( weight >  0  ) 
  return Signature4CRS::Weak;  // weight (0, 50)

}

Signature4CRS::Weight Signature4CRS::block( long minX, long minY, 
         unsigned long sizeOfBlock ) const
{

  Coordinate min=map->mbr.min;

  min.x = min.x & (0xFFFFFFFFlu << map->potency);
  min.y = min.y & (0xFFFFFFFFlu << map->potency);
  minX  = minX  & (0xFFFFFFFFlu << map->potency);
  minY  = minY  & (0xFFFFFFFFlu << map->potency);

  int x = (int) ((minX - min.x) / this->sizeOfBlock),
      y = (int) ((minY - min.y) / this->sizeOfBlock);

  if( (unsigned long)this->sizeOfBlock == sizeOfBlock )
            return block( x, y );
  else
    if( (unsigned long)this->sizeOfBlock < sizeOfBlock )
    {
      unsigned temWeak = 0;
      unsigned long fullCells = 0;

#ifdef APPROXIMATE_AREA
      return resolutionChange( x,  y, sizeOfBlock);

#endif

      int factor = (unsigned) (sizeOfBlock/this->sizeOfBlock),
          startX = x < 0 ? 0 : x,
          startY = y < 0 ? 0 : y,
          endX = x + factor > (int) map->dx ? map->dx : x + factor,
          endY = y + factor > (int) map->dy ? map->dy : y + factor,
          i, j;

      for( i = startX; i < endX; i++ )
        for( j = startY; j < endY; j++ )
        {
          Weight current = block( i, j );
          //cout << "weight " << i << ", " << j << " = " << current << endl;

          switch( current )
          {
             case Full : fullCells  += 2; break;
             case Strong : fullCells  += 1; break;
             case Weak : temWeak = 1; break;
             case Empty : break;

          }
        }

      unsigned long factor2 = (factor * (long) factor);
      // factor2: number of cells of the smaller size that fit in a cell 
      // with larger size

      if( fullCells == 2 * factor2 )
        return Full;
        //if the "weight" of full cells equals the number of total cells
        //than returns full (100% of weight)
      else
        if( fullCells > factor2 )
          return Strong;
          //if the "weight" of full cells is larger than half of the total 
          // number of cells then returns Strong ]50%,100%[ of weight
        else
          if( fullCells < factor2 )
            return (temWeak || fullCells > 0 ) ? Weak : Empty;
            //if the "weight" of full cells is smaller than half of the
            // total number of cells and exists weak or strong or full
            // than returns wead ]0%,50%] of weight else, returns empty
          else
            return temWeak ? Strong : Weak;
            //if the "weight" of full cells equals half the number
            //of total cells and there is no weak cell, then returns strong
            //else returns weak

    }
    else
    {
      //cout << "Critical error in " __FILE__ << ", line " <<  __LINE__ << endl;
      abort();

      return Empty;
    }
}

int existsIntersection( const Signature4CRS* a, const Signature4CRS* b )
{
  unsigned long sizeOfBlock = MAX( a->sizeOfBlock, b->sizeOfBlock );
  unsigned potency = MAX( a->map->potency, b->map->potency );
  Coordinate min( MAX( a->map->mbr.min.x, b->map->mbr.min.x ),
                  MAX( a->map->mbr.min.y, b->map->mbr.min.y ) ),
             max( MIN( a->map->mbr.max.x, b->map->mbr.max.x ),
                  MIN( a->map->mbr.max.y, b->map->mbr.max.y ) );
  long i, j, maybe = 0;

  (a->sizeOfBlock == b->sizeOfBlock ?
   sameScales :
   differentScales)++;

  min.x = min.x & (0xFFFFFFFFlu << potency);
  min.y = min.y & (0xFFFFFFFFlu << potency);
  max.x = max.x & (0xFFFFFFFFlu << potency);
  max.y = max.y & (0xFFFFFFFFlu << potency);

  for( i = min.x; i <= max.x; i += sizeOfBlock )
    for( j = min.y; j <= max.y; j += sizeOfBlock )
    {
      Signature4CRS::Weight blockA = a->block( i, j, sizeOfBlock ),
                           blockB = b->block( i, j, sizeOfBlock );

      if( blockA == Signature4CRS::Empty || blockB == Signature4CRS::Empty )
        continue; // Does not have intersection
      else if( (blockA == Signature4CRS::Full 
                         || blockB == Signature4CRS::Full) ||
               (blockA == Signature4CRS::Strong 
                         && blockB == Signature4CRS::Strong) )
        return 2;
      else
        maybe = 1;
    }

  return maybe;
}

void Signature4CRS::RasterMap4CRS::block(unsigned x, unsigned y, Weight weight)
{
  if( x < header.dx && y < header.dy && weight != Empty )
  {
    unsigned index = x*header.dy + y,
             position = index / (8 * sizeof( GroupOfBits )),
             displacement = 8 * sizeof( GroupOfBits ) - 1 
             - (index % (8 * sizeof( GroupOfBits )));

    bits0[ position ] |= ((weight & 0x1lu) >> 0) << displacement;
    bits1[ position ] |= ((weight & 0x2lu) >> 1) << displacement;
  }
}

Signature4CRS& Signature4CRS::operator=(const Signature4CRS& a)
{
  this->attr_map = a.fullMap();
  return *this;
}

int Signature4CRS::operator == ( const Signature4CRS& a ) const
{
  return !memcmp( &attr_map, &a.attr_map, sizeof attr_map );
}


//---------------------------------------------------------------------------

