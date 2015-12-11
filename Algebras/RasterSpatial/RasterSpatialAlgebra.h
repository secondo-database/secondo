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

Declarations for the RasterSpatialAlgebra


May, 2007 Leonardo Azevedo, Rafael Brand

*/

#ifndef __RASTERSPATIAL_ALGEBRA_H__
#define __RASTERSPATIAL_ALGEBRA_H__

/*
1 Preliminaries

1.1 Includes and global declarations

*/

#include "../Raster/RasterAlgebra.h"

#include "SpatialAlgebra.h"
#include <fstream>

#include "../../Tools/Flob/DbArray.h"
#include "./../Raster/Signature/GenerateRaster.h"

class Raster4CRS;
extern long compareSignatures4CRS( Signature4CRS *assinat4crs1,
                            Signature4CRS *assinat4crs2, MBR &mbrIntersection);

/*

2 Data structures

2.1 RasterRegion

*/

class CRasterRegion: public Region
{
  private:

  public:
    static const std::string BasicType() { return "rasterRegion"; }

    Raster4CRS *rasterSignature;

   //potency dx dy signature...
    DbArray<unsigned long> rasterFLOB;

    bool rasterDefined;

/*
Do not use this constructor.

*/
    CRasterRegion(){};

    CRasterRegion(const Region& rr);
    CRasterRegion(const CRasterRegion& rr);
    CRasterRegion(int i);
    Raster4CRS *getRaster() ;
    Raster4CRS *readRaster() const;
    void setRaster(Raster4CRS *raster);
    Raster4CRS *calculateRaster(int signatureType) const;
    void Raster4CRSToFLOB();
    void FLOBToRaster4CRS();

    bool Intersects(CRasterRegion &rr2);
    int preIntersects( const Region& r) const;
    bool MBRIntersects( const Region& r) const;
    bool ExactIntersects( const Region& r) const;

    CRasterRegion* Clone() const
    {
      return new CRasterRegion(*(const CRasterRegion *)this);
    }

    int NumOfFLOBs(void) const;
    Flob *GetFLOB(const int i);

  CRasterRegion& operator=( CRasterRegion& r );
};

CRasterRegion& CRasterRegion::operator=( CRasterRegion& r )
{
  Region::operator=((Region) r);

  rasterFLOB.clean();
  if( r.rasterFLOB.Size() > 0 )
  {
    rasterFLOB.resize( r.rasterFLOB.Size() );
    for( int i = 0; i < r.rasterFLOB.Size(); i++ )
    {
      unsigned long hs;
      r.rasterFLOB.Get( i, hs );
      rasterFLOB.Put( i, hs );
    }
  }

  setRaster(r.getRaster()->Clone());

  return *this;
}

/*
2.1.1 Constructors.


*/


CRasterRegion::CRasterRegion(const Region& rr) : Region(rr), rasterFLOB(0){
  rasterDefined = false;
  rasterFLOB.clean();
  //rasterSignature = calculateRaster();
  rasterFLOB.clean();
  rasterSignature = NULL;
};

CRasterRegion::CRasterRegion(const CRasterRegion& rr) :
           Region(rr), rasterFLOB(0){
  rasterDefined = false;
  rasterFLOB.clean();
  if( rr.rasterFLOB.Size() > 0 )
  {
    rasterFLOB.resize( rr.rasterFLOB.Size() );
    for( int i = 0; i < rr.rasterFLOB.Size(); i++ )
    {
      unsigned long hs;
      rr.rasterFLOB.Get( i, hs );
      rasterFLOB.Put( i, hs );
    }

    FLOBToRaster4CRS();
  } else {

    setRaster(rr.readRaster()->Clone());
  }
};


CRasterRegion::CRasterRegion(int i) : Region(i), rasterFLOB(0){
  //rasterFLOB.Clear();
  rasterDefined = false;
  rasterSignature = NULL;
};

/*
2.1.2 getRaster.

*/

Raster4CRS *CRasterRegion::getRaster() {

  if (IsEmpty())
      return NULL;

  if (rasterDefined)
    return rasterSignature;
  else {
    rasterDefined = true;
    rasterSignature = calculateRaster(3);
    return rasterSignature;
  }
};

Raster4CRS *CRasterRegion::readRaster() const{

  if (!IsEmpty() && rasterDefined) {
    return rasterSignature;
  } else {
    return NULL;
  }
};

void CRasterRegion::setRaster(Raster4CRS *raster){
  rasterDefined = true;
  rasterSignature = raster;
  Raster4CRSToFLOB();
};

Raster4CRS *CRasterRegion::calculateRaster(int signatureType) const{
  Raster4CRS* raster = NULL;
  Signature4CRS* signature = NULL;

  const Region *r1 = this;

  int potency =0;
  do
  {
    SignatureType type;
  if (signatureType == 3)
    type = SIGNAT_3CRS;
  else if (signatureType == 4)
    type = SIGNAT_4CRS;
  else
    cout << "Invalid signatureType" << std::endl;

  signature = GeraRasterSecondo::generateRaster( 1, r1, NULL, NULL,
                 potency, type);


    potency++;
  } while (signature==NULL);
  raster = new Raster4CRS(signature->fullMap(), signatureType);
  raster->signatureType = signatureType;

  return raster;
};

/*
2.1.3 Intersect functions.

*/

bool CRasterRegion::Intersects(CRasterRegion &rr2)
{
  int result;
  result = preIntersects((Region)rr2);
  if(result == 0 || result == 1)
    return result == 1;

  //MBR test
  if(!MBRIntersects((Region)rr2))
    return false;

  //Raster signature test
  MBR mbrIntersection;
  int rasterIntersects = compareSignatures4CRS( this->getRaster(),
         rr2.getRaster(), mbrIntersection);

  if (rasterIntersects == 0 || rasterIntersects == 1)
    return rasterIntersects;

  //exact test
  return ExactIntersects((Region) rr2);
}


int CRasterRegion::preIntersects( const Region& r) const
{
  assert( IsOrdered() && r.IsOrdered() );

  if( IsEmpty() && r.IsEmpty() )
    return 1;

  if( IsEmpty() || r.IsEmpty() )
    return 0;

  return 2;
}

bool CRasterRegion::MBRIntersects( const Region& r) const
{
  if( !BoundingBox().Intersects( r.BoundingBox() ) )
    return false;
  return true;
}

// Function RegionIntersects used for distinguish the first and second steps
//of the Intersects funcion
bool CRasterRegion::ExactIntersects(const Region &r) const
{

  if( Inside( r ) || r.Inside( *this ) )
    return true;

  HalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() )
    {
      for( int j = 0; j < r.Size(); j++ )
      {
        r.Get( j, hs2 );
        if( hs2.IsLeftDomPoint() &&
            hs1.Intersects( hs2 ) )
          return true;
      }
    }
  }

  return false;
}

/*
2.1.4. Function that converts a Raster4CRS to a FLOB

*/

void CRasterRegion::Raster4CRSToFLOB(){
  if (rasterDefined) {
    Signature4CRS::Weight filling;

    rasterFLOB.clean();

    rasterFLOB.Append(rasterSignature->signatureType);
    rasterFLOB.Append(rasterSignature->map->potency);
    rasterFLOB.Append(rasterSignature->map->dx);
    rasterFLOB.Append(rasterSignature->map->dy);

    long cellSize = 1l << rasterSignature->map->potency;
    long computedCells = 0;
    unsigned long FLOBelement = 0;

    long minXcell = rasterSignature->map->mbr.min.x
          - (rasterSignature->map->mbr.min.x % cellSize)
          - (cellSize * (rasterSignature->map->mbr.min.x < 0
             && (rasterSignature->map->mbr.min.x % cellSize != 0) ? 1 : 0));
    long minYcell = rasterSignature->map->mbr.min.y
          - (rasterSignature->map->mbr.min.y % cellSize)
          - (cellSize * (rasterSignature->map->mbr.min.y < 0
             && (rasterSignature->map->mbr.min.y % cellSize != 0) ? 1 : 0));
    for( long i=minXcell; i <= rasterSignature->map->mbr.max.x; i+=cellSize) {
      for(long j=minYcell; j <= rasterSignature->map->mbr.max.y; j+=cellSize)
      {
        filling = rasterSignature->block(i,j,cellSize);
        FLOBelement = (FLOBelement << 2) | filling;
        computedCells++;
        if (computedCells == (sizeof (unsigned long) * 4) ) {
          rasterFLOB.Append(FLOBelement);
          FLOBelement = 0;
          computedCells = 0;
        }
      }
    }

    if (computedCells > 0) {
      rasterFLOB.Append(FLOBelement);
    }


  } else {
    if (rasterFLOB.Size() > 0) {
      rasterFLOB.clean();
    }
  }
}

/*
2.1.5. Function that converts a FLOB to Raster4CRS

*/

void CRasterRegion::FLOBToRaster4CRS(){
  unsigned long potency, dx, dy, signatureType;

  unsigned long l;


  rasterFLOB.Get(0, l);
  signatureType = l;
  rasterFLOB.Get(1, l);
  potency = l;
  rasterFLOB.Get(2, l);
  dx = l;
  rasterFLOB.Get(3, l);
  dy = l;

  long cellSize = 1l << potency;

  long numElements = dx * dy;
  Signature4CRS::Weight *filling = new Signature4CRS::Weight[numElements];
for (int i = 0; i < numElements; i++)
 filling[i] = Signature4CRS::Empty;

  unsigned long pFLOBelement;
  unsigned long FLOBelement;
  int positionInElement = -1;
  unsigned int currentCell = 0;
  Signature4CRS::Weight ocup;

  int positionInFLOB = 0;
  long int minXcell = (long int)(this->BoundingBox().MinD(0)
       - ((long int)this->BoundingBox().MinD(0) % cellSize)
       - (cellSize * (this->BoundingBox().MinD(0) < 0
          && ((long int)this->BoundingBox().MinD(0) % cellSize != 0) ? 1 : 0)));
  long int minYcell = (long int)(this->BoundingBox().MinD(1)
       - ((long int)this->BoundingBox().MinD(1) % cellSize)
       - (cellSize * (this->BoundingBox().MinD(1) < 0
          && ((long int)this->BoundingBox().MinD(1) % cellSize != 0) ? 1 : 0)));

  Coordinate cmin ((long int)this->BoundingBox().MinD(0),
           (long int)this->BoundingBox().MinD(1));
  Coordinate cmax ((long int)this->BoundingBox().MaxD(0),
           (long int)this->BoundingBox().MaxD(1));
  rasterSignature = new Raster4CRS(1, cmin, cmax, cellSize, dx, dy, filling,
           signatureType);

  long totalCells = (long)((this->BoundingBox().MaxD(0) - minXcell + 1)
           * (this->BoundingBox().MaxD(1) - minYcell + 1));

  for( long i=minXcell; i <= this->BoundingBox().MaxD(0); i+=cellSize)
    for(long j=minYcell; j <= this->BoundingBox().MaxD(1); j+=cellSize)
    {
      if (positionInElement < 0){
        // the 4 itens at the beggining are signatureType, potency, dx and dy
        rasterFLOB.Get(positionInFLOB + 4, pFLOBelement);
        FLOBelement = pFLOBelement;
        positionInElement = 0;
        //if (dx * dy - currentCell < sizeof(unsigned long) * 4)
        if (totalCells - currentCell < sizeof(unsigned long) * 4)
          //positionInElement = (dx * dy - currentCell) - 1;
          positionInElement = (totalCells - currentCell) - 1;
        else
          positionInElement = sizeof(unsigned long) * 4 - 1;
        positionInFLOB++;
      }
      switch ( (FLOBelement >> (positionInElement * 2)) & 3 ) {
        case 0:
          ocup = Signature4CRS::Empty;
          break;
        case 1:
          ocup = Signature4CRS::Weak;
          break;
        case 2:
          ocup = Signature4CRS::Strong;
          break;
        default:
          ocup = Signature4CRS::Full;
      }
      rasterSignature->mapBlock((unsigned)((i - minXcell) / cellSize),
                          (unsigned)((j - minYcell) / cellSize), ocup);

      positionInElement--;
      currentCell++;
    }

  rasterDefined = true;
}

int CRasterRegion::NumOfFLOBs(void) const {
  return 2;
}

Flob *CRasterRegion::GetFLOB(const int i){
    assert(i == 0 || i == 1);

    return
        i == 0
        ? Region::GetFLOB(0)
        : &rasterFLOB;
}

/*
2.2. RasterLine

*/

class CRasterLine: public Line
{
  private:
    //Raster4CRS *rasterSignature;

   //potency dx dy signature...
    DbArray<unsigned long> rasterFLOB;

  public:
    static const std::string BasicType() { return "rasterLine"; }

    Raster4CRS *rasterSignature;

    bool rasterDefined;
    CRasterLine() {};
    CRasterLine(Line l);
    CRasterLine(const CRasterLine& rl);
    CRasterLine(int i);
    Raster4CRS *getRaster();
    Raster4CRS *readRaster() const;
    void setRaster(Raster4CRS *raster);
    Raster4CRS *calculateRaster(int signatureType) const;
    bool Intersects(CRasterLine &rl2);
    int preIntersects( const Line& l) const;
    bool MBRIntersects( const Line& l) const;
    bool ExactIntersects( const Line& l) const;

    bool Intersects(CRasterRegion &rr2);
    int preIntersects( const Region& r) const;
    bool MBRIntersects( const Region& r) const;
    bool ExactIntersects( const Region& r) const;

    void Raster4CRSToFLOB();
    void FLOBToRaster4CRS();

    virtual CRasterLine* Clone() const
    {
      return new CRasterLine(*(const CRasterLine *)this);
    }

    int NumOfFLOBs(void) const;
    Flob *GetFLOB(const int i);

  CRasterLine& operator=( CRasterLine& rl );
};

CRasterLine& CRasterLine::operator=( CRasterLine& rl )
{
  Line::operator=((Line) rl);

  rasterFLOB.clean();
  if( rl.rasterFLOB.Size() > 0 )
  {
    rasterFLOB.resize( rl.rasterFLOB.Size() );
    for( int i = 0; i < rl.rasterFLOB.Size(); i++ )
    {
      unsigned long hs;
      rl.rasterFLOB.Get( i, hs );
      rasterFLOB.Put( i, hs );
    }
  }

  setRaster(rl.getRaster()->Clone());

  return *this;
}

CRasterLine::CRasterLine(Line l) : Line(l), rasterFLOB(0){
  //rasterSignature = calculateRaster();
  rasterDefined = false;
  rasterSignature = NULL;
};

CRasterLine::CRasterLine(const CRasterLine& rl) : Line(rl), rasterFLOB(0){
  rasterDefined = false;
  rasterFLOB.clean();
  rasterFLOB.clean();
  if( rl.rasterFLOB.Size() > 0 )
  {
    rasterFLOB.resize( rl.rasterFLOB.Size() );
    for( int i = 0; i < rl.rasterFLOB.Size(); i++ )
    {
      unsigned long hs;
      rl.rasterFLOB.Get( i, hs );
      rasterFLOB.Put( i, hs );
    }
  }

  setRaster(rl.readRaster()->Clone());
};

CRasterLine::CRasterLine(int i) : Line(i), rasterFLOB(0){
  //if(i > 0)
    //rasterSignature = calculateRaster();
  rasterDefined = false;
  rasterSignature = NULL;
};

Raster4CRS *CRasterLine::getRaster(){
    if (IsEmpty())
    return NULL;
  if (rasterDefined)
    return rasterSignature;
  else {
    rasterDefined = true;
    rasterSignature = calculateRaster(3);
    return rasterSignature;
  }
};

Raster4CRS *CRasterLine::readRaster() const{

  if (!IsEmpty() && rasterDefined)
    return rasterSignature;
  else
    return NULL;
};

void CRasterLine::setRaster(Raster4CRS *raster){
  rasterDefined = true;
  rasterSignature = raster;
  Raster4CRSToFLOB();
};

Raster4CRS *CRasterLine::calculateRaster(int signatureType) const{
  Raster4CRS* raster;
  Signature4CRS* signature = NULL;

  const Line *l1 = this;
  int potency =0;
  do
  {
    SignatureType type;
  if (signatureType == 3)
    type = SIGNAT_3CRS;
  else if (signatureType == 4)
    type = SIGNAT_4CRS;
  else
    cout << "Invalid signatureType" << std::endl;

  signature = GeraRasterSecondo::generateRaster( 1, NULL, l1, NULL, potency,
              type);
    potency++;
  } while (signature==NULL);
  raster = new Raster4CRS(signature->fullMap(), signatureType);
  raster->signatureType = signatureType;

  return raster;
};

bool CRasterLine::Intersects(CRasterLine &rl2)
{
  int result;
  result = preIntersects((Line)rl2);
  if(result == 0 || result == 1)
    return result == 1;

  if(!MBRIntersects((Line)rl2)) {
    return false;
  }

  MBR mbrIntersection;
  int rasterIntersects = compareSignatures4CRS( this->getRaster(),
          rl2.getRaster(), mbrIntersection);

  //FALSE_HIT          0
  //HIT                1
  //INCONCLUSIVE       2

  if (rasterIntersects == 0 || rasterIntersects == 1)
    return rasterIntersects == 1;

  //exact test
  return ExactIntersects((Line)rl2);
}

int CRasterLine::preIntersects( const Line& l) const
{
  assert( IsOrdered() && l.IsOrdered() );

  if( IsEmpty() && l.IsEmpty() )
    return 1;

  if( IsEmpty() || l.IsEmpty() )
    return 0;

  return 2;
}

bool CRasterLine::MBRIntersects( const Line& l) const
{
  if( !BoundingBox().Intersects( l.BoundingBox() ) )
    return false;
  return true;
}

bool CRasterLine::ExactIntersects( const Line& l) const
{
  HalfSegment hs1, hs2;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, hs1 );
    if( hs1.IsLeftDomPoint() )
    {
      for( int j = 0; j < l.Size(); j++ )
      {
        l.Get( j, hs2 );
        if (hs2.IsLeftDomPoint())
        {
          if( hs1.Intersects( hs2 ) )
            return true;
        }
      }
    }
  }
  return false;
}

bool CRasterLine::Intersects(CRasterRegion &rr2)
{
  int result;
  result = preIntersects((Region)rr2);
  if(result == 0 || result == 1)
    return result == 1;

  if(!MBRIntersects((Region)rr2))
    return false;

  MBR mbrIntersection;
  int rasterIntersects = compareSignatures4CRS( this->getRaster(),
                             rr2.getRaster(), mbrIntersection);

  if (rasterIntersects == 0 || rasterIntersects == 1)
    return rasterIntersects == 1;

  //exact test
  return ExactIntersects((Region)rr2);
}

int CRasterLine::preIntersects( const Region& r) const
{
  assert( IsOrdered() && r.IsOrdered() );

  if( IsEmpty() && r.IsEmpty() )
    return 1;

  if( IsEmpty() || r.IsEmpty() )
    return 0;

  return 2;
}

bool CRasterLine::MBRIntersects( const Region& r) const
{
  if( !BoundingBox().Intersects( r.BoundingBox() ) )
    return false;
  return true;
}

bool CRasterLine::ExactIntersects( const Region& r) const
{
  HalfSegment hsl, hsr;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, hsl );
    if( hsl.IsLeftDomPoint() )
    {
      for( int j = 0; j < r.Size(); j++ )
      {
        r.Get( j, hsr );
        if( hsr.IsLeftDomPoint() )
        {
          if( hsl.Intersects( hsr ) )
            return true;
        }
      }

      if( r.Contains( hsl.GetLeftPoint() ) ||
          r.Contains( hsl.GetRightPoint() ) )
        return true;
    }
  }
  return false;
}

void CRasterLine::Raster4CRSToFLOB(){
  if (rasterDefined) {
    Signature4CRS::Weight filling;

    rasterFLOB.clean();

    rasterFLOB.Append(rasterSignature->signatureType);
    rasterFLOB.Append(rasterSignature->map->potency);
    rasterFLOB.Append(rasterSignature->map->dx);
    rasterFLOB.Append(rasterSignature->map->dy);

    long cellSize = 1l << rasterSignature->map->potency;
    long computedCells = 0;
    unsigned long FLOBelement = 0;

    int numCelulas = 0;
    long minXcell = rasterSignature->map->mbr.min.x
          - (rasterSignature->map->mbr.min.x % cellSize)
          - (cellSize * (rasterSignature->map->mbr.min.x < 0
               && (rasterSignature->map->mbr.min.x % cellSize != 0) ? 1 : 0));
    long minYcell = rasterSignature->map->mbr.min.y
          - (rasterSignature->map->mbr.min.y % cellSize)
          - (cellSize * (rasterSignature->map->mbr.min.y < 0
               && (rasterSignature->map->mbr.min.y % cellSize != 0) ? 1 : 0));
    for( long i=minXcell; i <= rasterSignature->map->mbr.max.x; i+=cellSize)
      for(long j=minYcell; j <= rasterSignature->map->mbr.max.y; j+=cellSize)
      {
        filling = rasterSignature->block(i,j,cellSize);
        FLOBelement = (FLOBelement << 2) | filling;
        computedCells++;
        if (computedCells == (sizeof (unsigned long) * 4) ) {
          rasterFLOB.Append(FLOBelement);
          FLOBelement = 0;
          computedCells = 0;
        }
        numCelulas++;
      }

    if (computedCells > 0) {
      rasterFLOB.Append(FLOBelement);
    }

  } else {
    if (rasterFLOB.Size() > 0) {
      rasterFLOB.clean();
    }
  }
}

void CRasterLine::FLOBToRaster4CRS(){
  unsigned long potency, dx, dy, signatureType;
  //const Signature4CRS::Weight *filling;
  unsigned long l;

  rasterFLOB.Get(0, l);
  signatureType = l;
  rasterFLOB.Get(1, l);
  potency = l;
  rasterFLOB.Get(2, l);
  dx = l;
  rasterFLOB.Get(3, l);
  dy = l;

  long cellSize = 1l << potency;

  long numElements = dx * dy;
  Signature4CRS::Weight *filling = new Signature4CRS::Weight[numElements];

  unsigned long pFLOBelement;
  unsigned long FLOBelement;
  int positionInElement = -1;
  unsigned int currentCell = 0;
  Signature4CRS::Weight ocup;

  int positionInFLOB = 0;
  long minXcell = (long)((this->BoundingBox().MinD(0)
     - ((long int)this->BoundingBox().MinD(0) % cellSize))
     - (cellSize * (this->BoundingBox().MinD(0) < 0
         && ((long int)this->BoundingBox().MinD(0) % cellSize != 0) ? 1 : 0)));
  long minYcell = (long)((this->BoundingBox().MinD(1)
     - ((long int)this->BoundingBox().MinD(1) % cellSize))
     - (cellSize * (this->BoundingBox().MinD(1) < 0
         && ((long int)this->BoundingBox().MinD(1) % cellSize != 0) ? 1 : 0)));
  for( long i= minXcell; i <= this->BoundingBox().MaxD(0); i+=cellSize)
    for(long j= minYcell; j <= this->BoundingBox().MaxD(1); j+=cellSize)
    {
      if (positionInElement < 0){
        // +4 to consider potency, dx e dy
        rasterFLOB.Get(positionInFLOB + 4, pFLOBelement);
        FLOBelement = pFLOBelement;
        positionInElement = 0;
        if (dx * dy - currentCell < sizeof(unsigned long) * 4)
          positionInElement = (dx * dy - currentCell) - 1;
        else
          positionInElement = sizeof(unsigned long) * 4 - 1;
        positionInFLOB++;
      }
      switch ( (FLOBelement >> (positionInElement * 2)) & 3 ) {
        case 0:
          ocup = Signature4CRS::Empty;
          break;
        case 1:
          ocup = Signature4CRS::Weak;
          break;
        case 2:
          ocup = Signature4CRS::Strong;
          break;
        default:
          ocup = Signature4CRS::Full;
      }
      filling[currentCell] = ocup;

      positionInElement--;
      currentCell++;
    }

  Coordinate x ((long int)this->BoundingBox().MinD(0),
       (long int)this->BoundingBox().MinD(1));
  Coordinate y ((long int)this->BoundingBox().MaxD(0),
       (long int)this->BoundingBox().MaxD(1));
  rasterSignature = new Raster4CRS(1,  x,   y, cellSize, dx, dy,
       filling, signatureType);
  rasterDefined = true;

}

int CRasterLine::NumOfFLOBs(void) const {
  return 3;
}

Flob *CRasterLine::GetFLOB(const int i){

    assert(i == 0 || i == 1 || i == 2);

    return
        i < 2
        ? Line::GetFLOB(i)
        : &rasterFLOB;
}

/*
2.3. RasterPoints

*/

enum object {none, first, second, both};
enum status {endnone, endfirst, endsecond, endboth};

class CRasterPoints: public Points
{
  private:
    Raster4CRS *rasterSignature;

   //potency dx dy signature...
    DbArray<unsigned long> rasterFLOB;

    void SelectFirst_pp( const Points& P1, const Points& P2,
            object& obj, status& stat );
    void SelectNext_pp( const Points& P1, const Points& P2,
            object& obj, status& stat );

  public:
    static const std::string BasicType() { return "rasterPoints"; }

    bool rasterDefined;
    CRasterPoints(){};
    CRasterPoints(Points p);
    CRasterPoints(const CRasterPoints& rp);
    CRasterPoints(const int initsize);
    Raster4CRS *getRaster();
    Raster4CRS *readRaster() const;
    void setRaster(Raster4CRS *raster);
    Raster4CRS *calculateRaster(int signatureType) const;

    bool Intersects(CRasterRegion &rr2);
    int preIntersects( const Region& r) const;
    bool MBRIntersects( const Region& r) const;
    bool ExactIntersects( const Region& r) const;

    bool Intersects(CRasterLine &rl2);
    int preIntersects( const Line& l) const;
    bool MBRIntersects( const Line& l) const;
    bool ExactIntersects( const Line& l) const;

    bool Intersects(CRasterPoints &rp2);
    int preIntersects( const Points& ps) const;
    bool MBRIntersects( const Points& ps) const;
    bool ExactIntersects( const Points& ps) ;

    void Raster4CRSToFLOB();
    void FLOBToRaster4CRS();

    virtual CRasterPoints* Clone() const
    {
      return new CRasterPoints(*(const CRasterPoints *)this);
    }

    int NumOfFLOBs(void) const;
    Flob *GetFLOB(const int i);

  CRasterPoints& operator=( CRasterPoints& rp );
};

CRasterPoints& CRasterPoints::operator=( CRasterPoints& rp )
{
  Points::operator=((Points) rp);

  rasterFLOB.clean();
  if( rp.rasterFLOB.Size() > 0 )
  {
    rasterFLOB.resize( rp.rasterFLOB.Size() );
    for( int i = 0; i < rp.rasterFLOB.Size(); i++ )
    {
      unsigned long hs;
      rp.rasterFLOB.Get( i, hs );
      rasterFLOB.Put( i, hs );
    }
  }

  setRaster(rp.getRaster()->Clone());

  return *this;
}

CRasterPoints::CRasterPoints(Points p) : Points(p), rasterFLOB(0){
  rasterDefined = false;
  rasterFLOB.clean();
  //rasterSignature = calculateRaster();
  rasterSignature = NULL;
};

CRasterPoints::CRasterPoints(const CRasterPoints& rp) :
       Points(rp), rasterFLOB(0){
  rasterDefined = false;
  rasterFLOB.clean();
  rasterFLOB.clean();
  if( rp.rasterFLOB.Size() > 0 )
  {
    rasterFLOB.resize( rp.rasterFLOB.Size() );
    for( int i = 0; i < rp.rasterFLOB.Size(); i++ )
    {
      unsigned long hs;
      rp.rasterFLOB.Get( i, hs );
      rasterFLOB.Put( i, hs );
    }
  }

  setRaster(rp.readRaster()->Clone());
};

CRasterPoints::CRasterPoints(const int initsize) :
      Points(initsize), rasterFLOB(0){
  rasterDefined = false;
  rasterSignature = NULL;
};

Raster4CRS *CRasterPoints::getRaster(){
    if (IsEmpty())
      return NULL;
  if (rasterDefined)
    return rasterSignature;
  else {
    rasterDefined = true;
    rasterSignature = calculateRaster(3);
    return rasterSignature;
  }
};

Raster4CRS *CRasterPoints::readRaster() const{

  if (!IsEmpty() && rasterDefined)
    return rasterSignature;
  else
    return NULL;
};

void CRasterPoints::setRaster(Raster4CRS *raster){
  rasterDefined = true;
  rasterSignature = raster;
  Raster4CRSToFLOB();
};

Raster4CRS *CRasterPoints::calculateRaster(int signatureType) const{
  Raster4CRS* raster;
  Signature4CRS* signature;

  const CRasterPoints *rp = this;
  int potency =0;
  do
  {
    SignatureType type;
  if (signatureType == 3)
    type = SIGNAT_3CRS;
  else if (signatureType == 4)
    type = SIGNAT_4CRS;
  else
    cout << "Invalid signatureType" << std::endl;

  signature = GeraRasterSecondo::generateRaster( 1,
                  NULL, NULL, rp, potency, type);
    potency++;
  } while (signature==NULL);
  raster = new Raster4CRS(signature->fullMap(), signatureType);
  raster->signatureType = signatureType;

  return raster;
};

bool CRasterPoints::Intersects(CRasterRegion &rr2)
{
  int result;
  result = preIntersects(rr2);
  if(result == 0 || result == 1)
    return result == 1;

  //MBR test
  if(!MBRIntersects(rr2))
    return false;

  //Raster Signature test
  MBR mbrIntersection;
  int rasterIntersects = compareSignatures4CRS( this->getRaster(),
       rr2.getRaster(), mbrIntersection);

  if (rasterIntersects == 0 || rasterIntersects == 1)
    return rasterIntersects == 1;

  //exact test
  return ExactIntersects(rr2);
}

int CRasterPoints::preIntersects( const Region& r) const
{
  assert( IsOrdered() && r.IsOrdered() );

  if( IsEmpty() && r.IsEmpty() )
    return 1;

  if( IsEmpty() || r.IsEmpty() )
    return 0;

  return 2;
}

bool CRasterPoints::MBRIntersects( const Region& r) const
{
  if( !BoundingBox().Intersects( r.BoundingBox() ) )
    return false;
  return true;
}

bool CRasterPoints::ExactIntersects( const Region& r) const
{
  Point p;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    if( r.Contains( p ) )
      return true;
  }
  return false;
}

bool CRasterPoints::Intersects(CRasterLine &rl2)
{
  int result;
  result = preIntersects(rl2);
  if(result == 0 || result == 1)
    return result == 1;

  //MBR test
  if(!MBRIntersects(rl2))
    return false;

  //Raster Signature test
  MBR mbrIntersection;
  int rasterIntersects = compareSignatures4CRS( this->getRaster(),
         rl2.getRaster(), mbrIntersection);

  if (rasterIntersects == 0 || rasterIntersects == 1)
    return rasterIntersects == 1;

  //exact test
  return ExactIntersects(rl2);
}

int CRasterPoints::preIntersects( const Line& l) const
{
  assert( IsOrdered() && l.IsOrdered() );

  if( IsEmpty() && l.IsEmpty() )
    return 1;

  if( IsEmpty() || l.IsEmpty() )
    return 0;

  return 2;
}

bool CRasterPoints::MBRIntersects( const Line& l) const
{
  if( !BoundingBox().Intersects( l.BoundingBox() ) )
    return false;
  return true;
}

bool CRasterPoints::ExactIntersects( const Line& l) const
{
  Point p;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, p );
    if( l.Contains( p ) )
      return true;
  }

  return false;
}

bool CRasterPoints::Intersects(CRasterPoints &rp2)
{
  int result;
  result = preIntersects(rp2);
  if(result == 0 || result == 1)
    return result == 1;

  //MBR test
  if(!MBRIntersects(rp2))
    return false;

  //Raster Signature test
  MBR mbrIntersection;
  int rasterIntersects = compareSignatures4CRS( this->getRaster(),
           rp2.getRaster(), mbrIntersection);

  if (rasterIntersects == 0 || rasterIntersects == 1)
    return rasterIntersects == 1;

  //exact test
  return ExactIntersects(rp2);
}


int CRasterPoints::preIntersects( const Points& ps) const
{
  assert( IsOrdered() && ps.IsOrdered() );

  if( IsEmpty() && ps.IsEmpty() )
    return 1;

  if( IsEmpty() || ps.IsEmpty() )
    return 0;

  return 2;
}

bool CRasterPoints::MBRIntersects( const Points& ps) const
{
  if( !BoundingBox().Intersects( ps.BoundingBox() ) )
    return false;
  return true;
}

bool CRasterPoints::ExactIntersects( const Points& ps)
{
  object obj;
  status stat;
  SelectFirst_pp( *this, ps, obj, stat );

  while( stat != endboth )
  {
    if( obj == both )
      return true;
    SelectNext_pp( *this, ps, obj, stat );
  }
  return false;
}


void CRasterPoints::Raster4CRSToFLOB(){
  if (rasterDefined) {
    Signature4CRS::Weight filling;

    rasterFLOB.clean();

    rasterFLOB.Append(rasterSignature->signatureType);
    rasterFLOB.Append(rasterSignature->map->potency);
    rasterFLOB.Append(rasterSignature->map->dx);
    rasterFLOB.Append(rasterSignature->map->dy);

    long cellSize = 1l << rasterSignature->map->potency;
    long computedCells = 0;
    unsigned long FLOBelement = 0;

    long minXcell = rasterSignature->map->mbr.min.x
           - (rasterSignature->map->mbr.min.x % cellSize)
           - (cellSize * (rasterSignature->map->mbr.min.x < 0
              && (rasterSignature->map->mbr.min.x % cellSize != 0) ? 1 : 0));
    long minYcell = rasterSignature->map->mbr.min.y
           - (rasterSignature->map->mbr.min.y % cellSize)
           - (cellSize * (rasterSignature->map->mbr.min.y < 0
              && (rasterSignature->map->mbr.min.y % cellSize != 0) ? 1 : 0));
    for( long i=minXcell; i <= rasterSignature->map->mbr.max.x; i+=cellSize)
      for(long j=minYcell; j <= rasterSignature->map->mbr.max.y; j+=cellSize)
      {
        filling = rasterSignature->block(i,j,cellSize);
        FLOBelement = (FLOBelement << 2) | filling;
        computedCells++;
        if (computedCells == (sizeof (unsigned long) * 4) ) {
          rasterFLOB.Append(FLOBelement);
          FLOBelement = 0;
          computedCells = 0;
        }
      }

    if (computedCells > 0)
      rasterFLOB.Append(FLOBelement);


  } else {
    if (rasterFLOB.Size() > 0) {
      rasterFLOB.clean();
    }
  }
}

void CRasterPoints::FLOBToRaster4CRS(){
  unsigned long potency, dx, dy, signatureType;
  unsigned long l;

  rasterFLOB.Get(0, l);
  signatureType = l;
  rasterFLOB.Get(1, l);
  potency = l;
  rasterFLOB.Get(2, l);
  dx = l;
  rasterFLOB.Get(3, l);
  dy = l;

  long cellSize = 1l << potency;

  long numElements = dx * dy;
  Signature4CRS::Weight *filling = new Signature4CRS::Weight[numElements];

  unsigned long pFLOBelement;
  unsigned long FLOBelement;
  int positionInElement = -1;
  unsigned int currentCell = 0;
  Signature4CRS::Weight ocup;

  int positionInFLOB = 0;
  long int minXcell = (long int)((this->BoundingBox().MinD(0)
     - ((long int)this->BoundingBox().MinD(0) % cellSize))
     - (cellSize * (this->BoundingBox().MinD(0) < 0
        && ((long int)this->BoundingBox().MinD(0) % cellSize != 0) ? 1 : 0)));
  long int minYcell = (long int)((this->BoundingBox().MinD(1)
     - ((long int)this->BoundingBox().MinD(1) % cellSize))
     - (cellSize * (this->BoundingBox().MinD(1) < 0
        && ((long int)this->BoundingBox().MinD(1) % cellSize != 0) ? 1 : 0)));
  for( long i=minXcell; i <= this->BoundingBox().MaxD(0); i+=cellSize)
    for(long j=minYcell; j <= this->BoundingBox().MaxD(1); j+=cellSize)
    {
      if (positionInElement < 0){
         // add 4 to consider potency, dx e dy
        rasterFLOB.Get(positionInFLOB + 4, pFLOBelement);
        FLOBelement = pFLOBelement;
        positionInElement = 0;
        if (dx * dy - currentCell < sizeof(unsigned long) * 4)
          positionInElement = (dx * dy - currentCell) - 1;
        else
          positionInElement = sizeof(unsigned long) * 4 - 1;
        positionInFLOB++;
      }
      switch ( (FLOBelement >> (positionInElement * 2)) & 3 ) {
        case 0:
          ocup = Signature4CRS::Empty;
          break;
        case 1:
          ocup = Signature4CRS::Weak;
          break;
        case 2:
          ocup = Signature4CRS::Strong;
          break;
        default:
          ocup = Signature4CRS::Full;
      }
      filling[currentCell] = ocup;

      positionInElement--;
      currentCell++;
    }

  Coordinate x ((long int)this->BoundingBox().MinD(0),
       (long int)this->BoundingBox().MinD(1));
  Coordinate y ((long int)this->BoundingBox().MaxD(0),
       (long int)this->BoundingBox().MaxD(1));
  rasterSignature = new Raster4CRS(1,  x,   y, cellSize,
       dx, dy, filling, signatureType);
  rasterDefined = true;

}

int CRasterPoints::NumOfFLOBs(void) const {
  return 2;
}

Flob *CRasterPoints::GetFLOB(const int i){

    assert(i == 0 || i == 1);

    return
        i == 0
        ? Points::GetFLOB(0)
        : &rasterFLOB;
}

//auxiliary functions for Intersection test of two Points
void CRasterPoints::SelectFirst_pp( const Points& P1, const Points& P2,
                     object& obj, status& stat )
{
  P1.SelectFirst();
  P2.SelectFirst();

  Point p1, p2;
  bool gotP1 = P1.GetPt( p1 ),
       gotP2 = P2.GetPt( p2 );

  if( !gotP1 && !gotP2 )
  {
    obj = none;
    stat = endboth;
  }
  else if( !gotP1 )
  {
    obj = second;
    stat = endfirst;
  }
  else if( !gotP2 )
  {
    obj = first;
    stat = endsecond;
  }
  else //both defined
  {
    stat = endnone;
    if( p1 < p2 )
      obj = first;
    else if( p1 > p2 )
      obj = second;
    else
      obj = both;
  }
}

void CRasterPoints::SelectNext_pp( const Points& P1, const Points& P2,
                    object& obj, status& stat )
{
  // 1. get the current elements
  Point p1, p2;
  bool gotP1 = P1.GetPt( p1 ),
       gotP2 = P2.GetPt( p2 );

  //2. move the pointers
  if( !gotP1 && !gotP2 )
  {
    //do nothing
  }
  else if( !gotP1 )
  {
    P2.SelectNext();
    gotP2 = P2.GetPt( p2 );
  }
  else if( !gotP2 )
  {
    P1.SelectNext();
    gotP1 = P1.GetPt( p1 );
  }
  else //both currently defined
  {
    if( p1 < p2 ) //then hs1 is the last output
    {
      P1.SelectNext();
      gotP1 = P1.GetPt( p1 );
    }
    else if( p1 > p2 )
    {
      P2.SelectNext();
      gotP2 = P2.GetPt( p2 );
    }
    else
    {
      P1.SelectNext();
      gotP1 = P1.GetPt( p1 );
      P2.SelectNext();
      gotP2 = P2.GetPt( p2 );
    }
  }

  //3. generate the outputs
  if( !gotP1 && !gotP2 )
  {
    obj = none;
    stat = endboth;
  }
  else if( !gotP1 )
  {
    obj = second;
    stat = endfirst;
  }
  else if( !gotP2 )
  {
    obj = first;
    stat = endsecond;
  }
  else //both defined
  {
    stat = endnone;
    if( p1 < p2 )
      obj = first;
    else if( p1 > p2 )
      obj = second;
    else
      obj = both;
  }
}

#endif
