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


#include "../Raster/RasterAlgebra.h"

#include "SpatialAlgebra.h"
#include <fstream>

#include "DBArray.h"
#include "./../Raster/Signature/GenerateRaster.h"

class Raster4CRS;
extern long compareSignatures4CRS( Signature4CRS *assinat4crs1, 
                            Signature4CRS *assinat4crs2, MBR &mbrIntersection);

//1.2 RasterRegion

class CRasterRegion: public Region
{
  private:
 
  public:
    Raster4CRS *rasterSignature;
    
   //potency dx dy signature...   
    DBArray<unsigned long> rasterFLOB;
    
    bool rasterDefined;
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
    FLOB *GetFLOB(const int i);

  CRasterRegion& operator=( CRasterRegion& r );
};

CRasterRegion& CRasterRegion::operator=( CRasterRegion& r )
{
  Region::operator=((Region) r);

  rasterFLOB.Clear();
  if( r.rasterFLOB.Size() > 0 )
  {
    rasterFLOB.Resize( r.rasterFLOB.Size() );
    for( int i = 0; i < r.rasterFLOB.Size(); i++ )
    {
      const unsigned long *hs;
      r.rasterFLOB.Get( i, hs );
      rasterFLOB.Put( i, *hs );
    }
  }

  setRaster(r.getRaster()->Clone());

  return *this;
}

CRasterRegion::CRasterRegion(const Region& rr) : Region(rr), rasterFLOB(0){
  rasterDefined = false;
  rasterFLOB.Clear();
  //rasterSignature = calculateRaster();
  rasterFLOB.Clear();
  rasterSignature = NULL;
};

CRasterRegion::CRasterRegion(const CRasterRegion& rr) : 
           Region(rr), rasterFLOB(0){
  rasterDefined = false;
  rasterFLOB.Clear();
  if( rr.rasterFLOB.Size() > 0 )
  {
    rasterFLOB.Resize( rr.rasterFLOB.Size() );
    for( int i = 0; i < rr.rasterFLOB.Size(); i++ )
    {
      const unsigned long *hs;
      rr.rasterFLOB.Get( i, hs );
      rasterFLOB.Put( i, *hs );
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
  
  const HalfSegment *hs1, *hs2;
  for( int i = 0; i < Size(); i++ )
  {
    Get( i, hs1 );
    if( hs1->IsLeftDomPoint() )
    {
      for( int j = 0; j < r.Size(); j++ )
      {
        r.Get( j, hs2 );
        if( hs2->IsLeftDomPoint() && 
            hs1->Intersects( *hs2 ) )
          return true;
      }
    }
  }

  return false;
}

void CRasterRegion::Raster4CRSToFLOB(){
  if (rasterDefined) {
    Signature4CRS::Weight filling;
    
    rasterFLOB.Clear();
    
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
      rasterFLOB.Clear();
    }
  }
}

void CRasterRegion::FLOBToRaster4CRS(){
  unsigned long potency, dx, dy, signatureType;
  
  const unsigned long *l;


  rasterFLOB.Get(0, l);
  signatureType = *l;
  rasterFLOB.Get(1, l);
  potency = *l;
  rasterFLOB.Get(2, l);
  dx = *l;
  rasterFLOB.Get(3, l);
  dy = *l;
  
  long cellSize = 1l << potency;
  
  long numElements = dx * dy;
  Signature4CRS::Weight *filling = new Signature4CRS::Weight[numElements];
for (int i = 0; i < numElements; i++)
 filling[i] = Signature4CRS::Empty;

  const unsigned long *pFLOBelement;
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
  
  long totalCells = (this->BoundingBox().MaxD(0) - minXcell + 1) 
           * (this->BoundingBox().MaxD(1) - minYcell + 1);

  for( long i=minXcell; i <= this->BoundingBox().MaxD(0); i+=cellSize)
    for(long j=minYcell; j <= this->BoundingBox().MaxD(1); j+=cellSize)
    {
      if (positionInElement < 0){
        // the 4 itens at the beggining are signatureType, potency, dx and dy
        rasterFLOB.Get(positionInFLOB + 4, pFLOBelement); 
        FLOBelement = *pFLOBelement;
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

FLOB *CRasterRegion::GetFLOB(const int i){
    assert(i == 0 || i == 1);

    return
        i == 0
        ? Region::GetFLOB(0)
        : &rasterFLOB;
}
