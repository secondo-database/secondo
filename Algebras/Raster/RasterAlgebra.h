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
#ifndef __RASTER_ALGEBRA_H__
#define __RASTER_ALGEBRA_H__

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h" //needed because we return a CcBool in an op.
#include <string>
#include "Attribute.h"

#include "Algebras/Spatial/SpatialAlgebra.h"
#include <fstream>

#include "./Signature/Signature4CRS.h"

#include "Tools/Flob/DbArray.h"
#include "Tools/Flob/Flob.h"

/*
1.1 Raster4CRS

*/

enum RasterType {RT_REGION, RT_LINE, RT_POINTS};

class Raster4CRS: public Signature4CRS, public Attribute
{
 public:
  //potency dx dy mbr.min.x mbr.min.y mbr.max.x mbr.max.y signature...
  DbArray<unsigned long> rasterFLOB;

  int signatureType;

  Raster4CRS(){};
  /*
  Do not use this constructor.
  */

  Raster4CRS( unsigned long id,  Coordinate min, Coordinate max,
                unsigned long sizeOfBlock, unsigned dx, unsigned dy,
                const Weight weight[], int signature ) : Signature4CRS(id,
                min, max, sizeOfBlock, dx, dy, weight ), rasterFLOB(0)
                                              {
                                                  signatureType = signature;
                                              }

  Raster4CRS( const Raster4CRS *raster):Signature4CRS(*(Signature4CRS*)raster),
             rasterFLOB(0)
  {
    this->signatureType = raster->signatureType;
  }

  Raster4CRS( const Raster4CRS &raster) : Signature4CRS((Signature4CRS)raster),
             rasterFLOB(0)
  {
    this->signatureType = raster.signatureType;
  }

  Raster4CRS( Signature4CRS::RasterMap4CRS mapaRaster4CRS,
               int signatureType) : Signature4CRS(mapaRaster4CRS), rasterFLOB(0)
  {
    this->signatureType = signatureType;
  }


  void Raster4CRSToFLOB();
  void FLOBToRaster4CRS();

  ~Raster4CRS();
  Raster4CRS*   Clone() const;

  int NumOfFLOBs(void) const;
  Flob *GetFLOB(const int i);

  Raster4CRS& operator=( Raster4CRS& r );

  //functions from Attribute
  bool IsDefined() const {return true;}
  void SetDefined( bool Defined ){}
  size_t Sizeof() const { return sizeof( *this );}
  bool Adjacent( const Attribute* arg ) const {return false;}
  size_t HashValue() const{return (size_t)(5*map->mbr.min.x + map->mbr.max.y);}
  void CopyFrom( const Attribute* right )
          {operator=(*((Raster4CRS *)right));}
  int Compare( const Attribute *arg ) const{ return 1;} //still to do
  static const std::string BasicType() { return "raster4CRS"; }
};

void printSignature(const Signature4CRS *raster4CRS);


inline std::string unsignedLongToBinary( const unsigned long &x ) {
  unsigned long t;
  std::string ret;
  if( x > 0 ) t = x; else t = -1 * x;
  for( unsigned int i = 0; i < sizeof(unsigned long) * 8; ++i )
    if( t & ( 1 << i ) ) ret.push_back( '1' ); else ret.push_back( '0' );
  reverse( ret.begin(), ret.end() );
  if( ret.size() == 0 ) return "0";
  if( x < 0 ) return '-' + ret;
  return ret;
}

#endif
