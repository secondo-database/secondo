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
#include "RasterSpatialAlgebra.h"
#include "Symbols.h"

extern ListExpr OutRaster4CRS( ListExpr typeInfo, Word value );
extern Word InRaster4CRS( const ListExpr typeInfo, const ListExpr instance,
         const int errorPos, ListExpr& errorInfo, bool& correct );
extern ListExpr OutPoints( ListExpr typeInfo, Word value );
extern Word InPoints( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct );


using namespace std;

//2. ~In~ and ~Out~ Functions



//2.2 RasterRegion

namespace rasterspatial{

void
DisplayRaster4CRSTmp( ListExpr value)
{
  unsigned long dx; //, dy;
  string output = "";
  string outputLine;

  if ( nl->ListLength( value ) == 10 )
  {
    ListExpr idAtom = nl->First(value);
    ListExpr restOfList = nl->Rest(value);
    ListExpr typeAtom = nl->First(restOfList);
    restOfList = nl->Rest(restOfList);
    ListExpr minXAtom = nl->First(restOfList);
    restOfList = nl->Rest(restOfList);
    ListExpr minYAtom = nl->First(restOfList);
    restOfList = nl->Rest(restOfList);
    ListExpr maxXAtom = nl->First(restOfList);
    restOfList = nl->Rest(restOfList);
    ListExpr maxYAtom = nl->First(restOfList);
    restOfList = nl->Rest(restOfList);
    ListExpr tamanhoAtom = nl->First(restOfList);
    restOfList = nl->Rest(restOfList);
    ListExpr dxAtom = nl->First(restOfList);
    restOfList = nl->Rest(restOfList);
    ListExpr dyAtom = nl->First(restOfList);
    restOfList = nl->Rest(restOfList);

    if ( nl->IsAtom(idAtom) && nl->AtomType(idAtom) == IntType
        && nl->IsAtom(typeAtom) && nl->AtomType(typeAtom) == IntType
        && nl->IsAtom(minXAtom) && nl->AtomType(minXAtom) == IntType
        && nl->IsAtom(minYAtom) && nl->AtomType(minYAtom) == IntType
        && nl->IsAtom(maxXAtom) && nl->AtomType(maxXAtom) == IntType
        && nl->IsAtom(maxYAtom) && nl->AtomType(maxYAtom) == IntType
        && nl->IsAtom(tamanhoAtom) && nl->AtomType(tamanhoAtom) == IntType
        && nl->IsAtom(dxAtom) && nl->AtomType(dxAtom) == IntType
        && nl->IsAtom(dyAtom) && nl->AtomType(dyAtom) == IntType )
    {
      dx = nl->IntValue( dxAtom );
      //dy = nl->IntValue( dyAtom );
      restOfList = nl->First( restOfList );
      output = "";
      outputLine = "";
      for (unsigned long int i = 0; !nl->IsEmpty( restOfList ); i++ )
      {
        ListExpr first = nl->First( restOfList );
        restOfList = nl->Rest( restOfList );

        if( nl->IsAtom( first ) && nl->AtomType( first ) == IntType )
        {
          if ( nl->IntValue( first ) == 0x0 )
            outputLine += ". ";
          else if ( nl->IntValue( first ) == 0x1 )
            outputLine += "o ";
          else if ( nl->IntValue( first ) == 0x2 )
            outputLine += "X ";
          else if ( nl->IntValue( first ) == 0x3 )
            outputLine += "# ";
        }
        else
        {
          cout << "Incorrect Data Format 1" << endl;
          return;
        }
        if ( i % dx == dx -1)
        {
          output = outputLine + "\n" + output;
          outputLine = "";
        }
      }
    }
    else
    {
      cout << "Incorrect Data Format 2" << endl;
      return;
    }
  }
  else
  {
    cout << "Incorrect Data Format 3 - " << nl->ListLength( value ) << endl;
    return;
  }
  cout << output << endl;
}

ListExpr
OutRasterRegion( ListExpr typeInfo, Word value )
{
  ListExpr rasterNL;
  ListExpr regionNL;
  ListExpr resultNL;

  CRasterRegion* crr = (CRasterRegion*)(value.addr);

  regionNL = OutRegion(typeInfo, value);

  if (crr->IsEmpty())
    rasterNL = nl->Empty();
  else
  {
    //when log is off, the following line prevents error when quit secondo
    //then return to it
    crr->FLOBToRaster4CRS();
    rasterNL = OutRaster4CRS(typeInfo, SetWord( (void *)(crr->getRaster()) ) );
  }


  resultNL = nl->TwoElemList(rasterNL, regionNL);

  return resultNL;

}


Word
InRasterRegion( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{

  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr rasterNL = nl->First(instance);
    ListExpr restOfList = nl->Rest(instance);
    ListExpr regionNL = nl->First(restOfList);

    bool regionCorrect = false;
    bool rasterCorrect = false;

    Region* r = (Region*)InRegion(typeInfo, regionNL, errorPos, errorInfo,
         regionCorrect).addr;
    CRasterRegion* rr = new CRasterRegion(*r);
    Raster4CRS* raster = (Raster4CRS*)InRaster4CRS(typeInfo, rasterNL,
         errorPos, errorInfo, rasterCorrect).addr;
    rr->setRaster(raster);

    if (regionCorrect && rasterCorrect)
    {
      correct = true;
      return SetWord(rr);
    }
  }

  cout << "error in InRasterRegion" << endl;
  correct = false;
  return SetWord(Address(0));

}


//2.3 RasterLine


ListExpr
OutRasterLine( ListExpr typeInfo, Word value )
{

  ListExpr rasterNL;
  ListExpr lineNL;
  ListExpr resultNL;

  CRasterLine* crl = (CRasterLine*)(value.addr);

  lineNL = OutLine(typeInfo, value);

  if (crl->IsEmpty())
    rasterNL = nl->Empty();
  else
  {
    //when log is off, the following line prevents error when quit secondo
    //then return to it
    crl->FLOBToRaster4CRS();
    rasterNL = OutRaster4CRS(typeInfo, SetWord( (void *)(crl->getRaster()) ));
  }

  resultNL = nl->TwoElemList(rasterNL, lineNL);

  return resultNL;

}


Word
InRasterLine( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{

  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr rasterNL = nl->First(instance);
    ListExpr restOfList = nl->Rest(instance);
    ListExpr lineNL = nl->First(restOfList);

    bool lineCorrect = false;
    bool rasterCorrect = false;

    Line* l = (Line*)InLine(typeInfo, lineNL, errorPos, errorInfo,
        lineCorrect).addr;
    CRasterLine* rl = new CRasterLine(*l);
    Raster4CRS* raster = (Raster4CRS*)InRaster4CRS(typeInfo, rasterNL,
        errorPos, errorInfo, rasterCorrect).addr;
    rl->setRaster(raster);

    if (lineCorrect && rasterCorrect)
    {
      correct = true;
      return SetWord(rl);
    }
  }

  correct = false;
  return SetWord(Address(0));

}


//2.4 RasterLine


ListExpr
OutRasterPoints( ListExpr typeInfo, Word value )
{

  ListExpr rasterNL;
  ListExpr pointsNL;
  ListExpr resultNL;

  CRasterPoints* crp = (CRasterPoints*)(value.addr);

  pointsNL = OutPoints(typeInfo, value);

  if (crp->IsEmpty())
    rasterNL = nl->Empty();
  else
  {
    //when log is off, the following line prevents error when quit secondo
    //then return to it
    crp->FLOBToRaster4CRS();
    rasterNL = OutRaster4CRS(typeInfo, SetWord( (void *)(crp->getRaster()) ));
  }

  resultNL = nl->TwoElemList(rasterNL, pointsNL);

  return resultNL;

}


Word
InRasterPoints( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{

  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr rasterNL = nl->First(instance);
    ListExpr restOfList = nl->Rest(instance);
    ListExpr pointsNL = nl->First(restOfList);

    bool pointsCorrect = false;
    bool rasterCorrect = false;

    Points* l = (Points*)InPoints(typeInfo, pointsNL, errorPos, errorInfo,
         pointsCorrect).addr;
    CRasterPoints* rp = new CRasterPoints(*l);
    Raster4CRS* raster = (Raster4CRS*)InRaster4CRS(typeInfo, rasterNL,
         errorPos, errorInfo, rasterCorrect).addr;
    rp->setRaster(raster);

    if (pointsCorrect && rasterCorrect)
    {
      correct = true;
      return SetWord(rp);
    }
  }

  correct = false;
  return SetWord(Address(0));

}


//3. Persistent Storage and Related Generic Functions



//3.2 RasterRegion


Word
CreateRasterRegion( const ListExpr typeInfo )
{
  return (SetWord( new CRasterRegion(0) ));
}

void
DeleteRasterRegion( const ListExpr typeInfo, Word& w )
{
  //CRasterRegion *cr = (CRasterRegion *)w.addr;
  //cr->Destroy();
  //delete cr;
  //w.addr = 0;
}

void
CloseRasterRegion( const ListExpr typeInfo, Word& w )
{
  //delete (CRasterRegion *)w.addr;
  //w.addr = 0;
}

Word
CloneRasterRegion( const ListExpr typeInfo, const Word& w )
{
  CRasterRegion *cr = new CRasterRegion( *((CRasterRegion *)w.addr) );
  return SetWord( cr );
}

int SizeOfRasterRegion()
{
  return sizeof(CRasterRegion);
}

bool
OpenRasterRegion( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  CRasterRegion *rr = (CRasterRegion*)Attribute::Open( valueRecord,
     offset, typeInfo );

  rr->FLOBToRaster4CRS();
  value = SetWord( rr );
  return true;
}

bool
SaveRasterRegion( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{

  CRasterRegion *rr = (CRasterRegion *)value.addr;
  rr->Raster4CRSToFLOB();

  Attribute::Save( valueRecord, offset, typeInfo, rr );

  return true;
}

void* CastRasterRegion( void* addr ) {
  return (new (addr) CRasterRegion);
}


//3.3 RasterLine


Word
CreateRasterLine( const ListExpr typeInfo )
{
  return (SetWord( new CRasterLine(0) ));
}

void
DeleteRasterLine( const ListExpr typeInfo, Word& w )
{
  //CRasterRegion *cl = (CRasterLine *)w.addr;
  //cl->Destroy();
  //delete cl;
  //w.addr = 0;
}

void
CloseRasterLine( const ListExpr typeInfo, Word& w )
{
  //delete (CRasterLine *)w.addr;
  //w.addr = 0;
}

Word
CloneRasterLine( const ListExpr typeInfo, const Word& w )
{
  CRasterLine *cl = new CRasterLine( *((CRasterLine *)w.addr) );
  return SetWord( cl );
}

int SizeOfRasterLine()
{
  return sizeof(CRasterLine);
}

void* CastRasterLine( void* addr ) {
  return (new (addr) CRasterLine);
}

bool
OpenRasterLine( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  CRasterLine *rl = (CRasterLine*)Attribute::Open( valueRecord,
         offset, typeInfo );

  rl->FLOBToRaster4CRS();
  value = SetWord( rl );

  return true;
}

bool
SaveRasterLine( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  CRasterLine *rl = (CRasterLine *)value.addr;
  rl->Raster4CRSToFLOB();

  Attribute::Save( valueRecord, offset, typeInfo, rl );

  return true;
}


//3.4 RasterPoints


Word
CreateRasterPoints( const ListExpr typeInfo )
{
  return (SetWord( new CRasterPoints(0) ));
}

void
DeleteRasterPoints( const ListExpr typeInfo, Word& w )
{
  //CRasterRegion *cp = (CRasterPoints *)w.addr;
  //cp->Destroy();
  //delete cp;
  //w.addr = 0;
}

void
CloseRasterPoints( const ListExpr typeInfo, Word& w )
{
  //delete (CRasterPoints *)w.addr;
  //w.addr = 0;
}

bool
OpenRasterPoints( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  CRasterPoints *rp = (CRasterPoints*)Attribute::Open( valueRecord,
         offset, typeInfo );

  rp->FLOBToRaster4CRS();
  //rr->setRaster(rr->calculateRaster());
  value = SetWord( rp );
  return true;
}

bool
SaveRasterPoints( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  CRasterPoints *rp = (CRasterPoints *)value.addr;
  rp->Raster4CRSToFLOB();

  Attribute::Save( valueRecord, offset, typeInfo, rp );

  return true;
}

Word
CloneRasterPoints( const ListExpr typeInfo, const Word& w )
{
  CRasterPoints *cp = new CRasterPoints( *((CRasterPoints *)w.addr) );
  return SetWord( cp );
}

int SizeOfRasterPoints()
{
  return sizeof(CRasterPoints);
}

void* CastRasterPoints( void* addr ) {
  return (new (addr) CRasterPoints);
}


//4 Properties


ListExpr
RasterRegionProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Remarks")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom(CRasterRegion::BasicType()),
           nl->StringAtom("( <raster4CRS> <region> "),
           nl->StringAtom(""))));
}

ListExpr
RasterLineProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Remarks")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom(CRasterLine::BasicType()),
           nl->StringAtom("( <raster4CRS> <line> "),
           nl->StringAtom(""))));
}

ListExpr
RasterPointsProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom("Remarks")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom(CRasterPoints::BasicType()),
           nl->StringAtom("( <raster4CRS> <points> "),
           nl->StringAtom(""))));
}


//5 Kind Checking


bool
CheckRasterRegion( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, CRasterRegion::BasicType() ));
}

bool
CheckRasterLine( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, CRasterLine::BasicType() ));
}

bool
CheckRasterPoints( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, CRasterPoints::BasicType() ));
}


//6 Type Constructors


TypeConstructor RasterRegion(
  CRasterRegion::BasicType(), RasterRegionProperty,
                             OutRasterRegion, InRasterRegion, 0, 0,
  CreateRasterRegion, DeleteRasterRegion,
  OpenRasterRegion, SaveRasterRegion,
     //0, 0,
  CloseRasterRegion, CloneRasterRegion, CastRasterRegion,
  SizeOfRasterRegion, CheckRasterRegion
);

TypeConstructor RasterLine(
  CRasterLine::BasicType(), RasterLineProperty,
                           OutRasterLine, InRasterLine, 0, 0,
  CreateRasterLine, DeleteRasterLine,
  OpenRasterLine, SaveRasterLine,
        //0, 0,
  CloseRasterLine, CloneRasterLine, CastRasterLine,
  SizeOfRasterLine, CheckRasterLine
);

TypeConstructor RasterPoints(
  CRasterPoints::BasicType(), RasterPointsProperty,
                             OutRasterPoints, InRasterPoints, 0, 0,
  CreateRasterPoints, DeleteRasterPoints,
  OpenRasterPoints, SaveRasterPoints,
        //0, 0,
  CloseRasterPoints, CloneRasterPoints, CastRasterPoints,
  SizeOfRasterPoints, CheckRasterPoints
);


//7 Validation functions


ListExpr
RegionBool( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, Region::BasicType()) )
      return nl->SymbolAtom(CcBool::BasicType());
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr
PointsLineRegionCRasterRegion( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, Line::BasicType()) )
      return nl->SymbolAtom(CRasterLine::BasicType());
    else if ( nl->IsEqual(arg1, Region::BasicType()) )
      return nl->SymbolAtom(CRasterRegion::BasicType());
    else if ( nl->IsEqual(arg1, Points::BasicType()) )
      return nl->SymbolAtom(CRasterPoints::BasicType());
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr
RegionCRasterRegion( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, Region::BasicType()) )
      return nl->SymbolAtom(CRasterRegion::BasicType());
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

//kind checking for
//'(rasterRegion || rasterLine || rasterPoints)
// rIntersects
// (rasterRegion || rasterLine || rasterPoints)'
ListExpr
CRasterRegionCRasterLineCRasterPointsBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength(args) == 2 )
  {
    arg1 = nl->First(args);
    arg2 = nl->Second(args);
    if ( (nl->IsEqual(arg1, CRasterRegion::BasicType())
             || nl->IsEqual(arg1, CRasterLine::BasicType())
             || nl->IsEqual(arg1, CRasterPoints::BasicType()))
        && (nl->IsEqual(arg2, CRasterRegion::BasicType())
             || nl->IsEqual(arg2, CRasterLine::BasicType())
             || nl->IsEqual(arg2, CRasterPoints::BasicType())) ) {
      return nl->SymbolAtom(CcBool::BasicType());
    }
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


//8 Functions and Auxiliary functions



//8.2 Calculate Raster


//8.3 Convert Raster 3CRS


static int
ConvertRaster3CRS_p (Word* args, Word& result, int message,
           Word& local, Supplier s){
  Points *points = ((Points*)args[0].addr);

  CRasterPoints *rasterPoints = new CRasterPoints(*points);

  rasterPoints->setRaster(rasterPoints->calculateRaster(3));

  result = qp->ResultStorage(s);

  *((CRasterPoints *)result.addr) = *rasterPoints;

  return 0;
}
static int
ConvertRaster3CRS_l (Word* args, Word& result, int message,
          Word& local, Supplier s){
  Line *line = ((Line*)args[0].addr);

  CRasterLine *rasterLine = new CRasterLine(*line);

  rasterLine->setRaster(rasterLine->calculateRaster(3));

  result = qp->ResultStorage(s);

  *((CRasterLine *)result.addr) = *rasterLine;

  return 0;
}

static int
ConvertRaster3CRS_r (Word* args, Word& result, int message,
           Word& local, Supplier s){
  Region *region = ((Region*)args[0].addr);

  CRasterRegion *rasterRegion = new CRasterRegion(*region);

  rasterRegion->setRaster(rasterRegion->calculateRaster(3));

  result = qp->ResultStorage(s);

  *((CRasterRegion *)result.addr) = *rasterRegion;

  return 0;
}


//8.4 Convert Raster 4CRS


static int
ConvertRaster4CRSFun (Word* args, Word& result, int message,
                Word& local, Supplier s){
  Region *region = ((Region*)args[0].addr);

  CRasterRegion *rasterRegion = new CRasterRegion(*region);

  rasterRegion->setRaster(rasterRegion->calculateRaster(4));

  result = qp->ResultStorage(s);

  *((CRasterRegion *)result.addr) = *rasterRegion;

  return 0;
}


//8.5 RegionToPolilyne


//creates the .dat File for visualization
static int
RegionToPolilyneFun (Word* args, Word& result, int message,
              Word& local, Supplier s)
{
  int i;
  HalfSegment chs, chs1;
  double endX1, endY1, endX2, endY2;
  long int face, cicle, cicleSize, nCicles;
  ostringstream scicle, sfile;

  face = -1;
  cicle = -1;
  cicleSize = 0;
  nCicles = 0;
//  scicle = "";
//  sfile = "";

  Region *r1 = ((Region*)args[0].addr);
  r1->LogicSort();

  for( i = 0; i < r1->Size(); i++ )
  {
    r1->Get( i, chs );
    if (chs.IsLeftDomPoint())
    {
      if (chs.GetAttr().faceno > face)
      {
        face = chs.GetAttr().faceno;
        cicle = -1;
      }
      if (chs.GetAttr().cycleno > cicle)
      {
        if (scicle.str() != "")
        {
          sfile << cicleSize << endl << scicle.str();
        }
        cicle = chs.GetAttr().cycleno;
        nCicles++;
        cicleSize = 2; //starts with 2 points in the cicle
        scicle.str(""); //cleans scicle

        // the second segment, since the i + 1 is the other
        // half-segment of the first segment
        r1->Get( i + 2, chs1 );
        if ( ( (chs.GetLeftPoint().GetX() == chs1.GetLeftPoint().GetX())
            && (chs.GetLeftPoint().GetY() == chs1.GetLeftPoint().GetY()) ) ||
             ( (chs.GetLeftPoint().GetX() == chs1.GetRightPoint().GetX())
            && (chs.GetLeftPoint().GetY() == chs1.GetRightPoint().GetY()) ) )
        {
          scicle << chs.GetRightPoint().GetX() << ","
                 << chs.GetRightPoint().GetY() << endl;
          scicle << chs.GetLeftPoint().GetX() << ","
                 << chs.GetLeftPoint().GetY() << endl;
        }
        else
        {
          scicle << chs.GetLeftPoint().GetX() << ","
                 << chs.GetLeftPoint().GetY() << endl;
          scicle << chs.GetRightPoint().GetX() << ","
                 << chs.GetRightPoint().GetY() << endl;
        }
        endX1 = chs.GetLeftPoint().GetX();
        endY1 = chs.GetLeftPoint().GetY();
        endX2 = chs.GetRightPoint().GetX();
        endY2 = chs.GetRightPoint().GetY();
      }
      else //inside the same cicle
      {
        if ( ( (chs.GetLeftPoint().GetX() == endX1)
             && (chs.GetLeftPoint().GetY() == endY1) ) ||
             ( (chs.GetLeftPoint().GetX() == endX2)
             && (chs.GetLeftPoint().GetY() == endY2) ) )
          scicle << chs.GetRightPoint().GetX() << ","
               << chs.GetRightPoint().GetY() << endl;
        else
          scicle << chs.GetLeftPoint().GetX() << ","
               << chs.GetLeftPoint().GetY() << endl;
        endX1 = chs.GetLeftPoint().GetX();
        endY1 = chs.GetLeftPoint().GetY();
        endX2 = chs.GetRightPoint().GetX();
        endY2 = chs.GetRightPoint().GetY();
        cicleSize++;
      }
    }
  }
  sfile << cicleSize << endl << scicle.str();
  ofstream arquivo("pol.dat", ios::out);
  arquivo << "Polilyne" << endl;
  arquivo << nCicles << endl;
  arquivo << sfile.str();

  result = qp->ResultStorage(s);  //query processor has provided
            //a CcBool instance to take the result
  ((CcBool*)result.addr)->Set(true, true);
            //the first argument says the boolean
            //value is defined, the second is the
            //real boolean value)
  return 0;
}


//8.6 Intersetcs Functions


static int
rIntersects_rr (Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool intersection;

  CRasterRegion *rr1 = ((CRasterRegion*)args[0].addr);
  CRasterRegion *rr2 = ((CRasterRegion*)args[1].addr);

  intersection = rr1->Intersects(*rr2);

  result = qp->ResultStorage(s);
  ((CcBool*)result.addr)->Set(true, intersection);
  #ifdef DEBUGMESSAGES
  cout << "Result of rIntersects_rr = " << intersection << endl;
  #endif
  return 0;
}

static int
rIntersects_rl (Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool intersection;

  CRasterRegion *rr1 = ((CRasterRegion*)args[0].addr);
  CRasterLine *rl2 = ((CRasterLine*)args[1].addr);

  intersection = rl2->Intersects(*rr1);

  result = qp->ResultStorage(s);
  ((CcBool*)result.addr)->Set(true, intersection);
  return 0;
}

static int
rIntersects_lr (Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool intersection;

  CRasterLine *rl1 = ((CRasterLine*)args[0].addr);
  CRasterRegion *rr2 = ((CRasterRegion*)args[1].addr);

  intersection = rl1->Intersects(*rr2);

  result = qp->ResultStorage(s);
  ((CcBool*)result.addr)->Set(true, intersection);
  return 0;
}

static int
rIntersects_ll (Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool intersection;

  CRasterLine *rl1 = ((CRasterLine*)args[0].addr);
  CRasterLine *rl2 = ((CRasterLine*)args[1].addr);

  intersection = rl1->Intersects(*rl2);

  result = qp->ResultStorage(s);
  ((CcBool*)result.addr)->Set(true, intersection);
  return 0;
}

static int
rIntersects_rp (Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool intersection;

  CRasterPoints *rp1 = ((CRasterPoints*)args[1].addr);
  CRasterRegion *rr2 = ((CRasterRegion*)args[0].addr);

  intersection = rp1->Intersects(*rr2);

  result = qp->ResultStorage(s);
  ((CcBool*)result.addr)->Set(true, intersection);
  return 0;
}

static int
rIntersects_pr (Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool intersection;

  CRasterPoints *rp1 = ((CRasterPoints*)args[0].addr);
  CRasterRegion *rr2 = ((CRasterRegion*)args[1].addr);

  intersection = rp1->Intersects(*rr2);

  result = qp->ResultStorage(s);
  ((CcBool*)result.addr)->Set(true, intersection);
  return 0;
}

static int
rIntersects_lp (Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool intersection;

  CRasterPoints *rp1 = ((CRasterPoints*)args[1].addr);
  CRasterLine *rl2 = ((CRasterLine*)args[0].addr);

  intersection = rp1->Intersects(*rl2);

  result = qp->ResultStorage(s);
  ((CcBool*)result.addr)->Set(true, intersection);
  return 0;
}

static int
rIntersects_pl (Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool intersection;

  CRasterPoints *rp1 = ((CRasterPoints*)args[0].addr);
  CRasterLine *rl2 = ((CRasterLine*)args[1].addr);

  intersection = rp1->Intersects(*rl2);

  result = qp->ResultStorage(s);
  ((CcBool*)result.addr)->Set(true, intersection);
  return 0;
}

static int
rIntersects_pp (Word* args, Word& result, int message, Word& local, Supplier s)
{
  bool intersection;

  CRasterPoints *rp1 = ((CRasterPoints*)args[0].addr);
  CRasterPoints *rp2 = ((CRasterPoints*)args[1].addr);

  intersection = rp1->Intersects(*rp2);

  result = qp->ResultStorage(s);
  ((CcBool*)result.addr)->Set(true, intersection);
  return 0;
}


//9 Value Mapping


ValueMapping convertraster3crsmap[] = {
    ConvertRaster3CRS_p,
  ConvertRaster3CRS_l,
    ConvertRaster3CRS_r
};

ValueMapping rIntersectsmap[] = {
  rIntersects_rr,
  rIntersects_rl,
  rIntersects_lr,
  rIntersects_ll,
  rIntersects_pr,
  rIntersects_rp,
  rIntersects_pl,
  rIntersects_lp,
  rIntersects_pp
};


//10 Select functions


int
ConvertRasterSelect( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, Points::BasicType()) )
      return 0;
    else if (nl->IsEqual(arg1, Line::BasicType()) )
      return 1;
    else if (nl->IsEqual(arg1, Region::BasicType()) )
      return 2;
  }

  return -1;
}

int
rIntersectsSelect( ListExpr args )
{
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, CRasterRegion::BasicType()) )
    {
      if ( nl->IsEqual(arg2, CRasterRegion::BasicType()) )
        return 0;
      else if ( nl->IsEqual(arg2, CRasterLine::BasicType()) )
        return 1;
      else if ( nl->IsEqual(arg2, CRasterPoints::BasicType()) )
        return 5;
    }
    else if ( nl->IsEqual(arg1, CRasterLine::BasicType()) )
    {
      if ( nl->IsEqual(arg2, CRasterRegion::BasicType()) )
        return 2;
      else if ( nl->IsEqual(arg2, CRasterLine::BasicType()) )
        return 3;
      else if ( nl->IsEqual(arg2, CRasterPoints::BasicType()) )
        return 7;
    }
    else if ( nl->IsEqual(arg1, CRasterPoints::BasicType()) )
    {
      if ( nl->IsEqual(arg2, CRasterRegion::BasicType()) )
        return 4;
      else if ( nl->IsEqual(arg2, CRasterLine::BasicType()) )
        return 6;
      else if ( nl->IsEqual(arg2, CRasterPoints::BasicType()) )
        return 8;
    }
  }

  return -1;
}

//11 Specification of operators


const string RegionToPolilyneSpec = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(region) -> bool</text--->"
       "<text>RegionToPolilyne (_)</text--->"
       "<text>Generates the polilyne file for the region.</text--->"
       "<text>query RegionToPolilyne (testRegion)</text--->"
       ") )";

const string ConvertRaster3CRSSpec ="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
       "( <text>(points||line||region) -> "
       "rasterPoints||rasterLine||rasterRegion</text--->"
       "<text>ConvertRaster3CRS (_)</text--->"
       "<text>Creates a raster object from an object, "
       "using the 3-Color Raster Signature.</text--->"
       "<text>query ConvertRaster3CRS (region)</text--->"
       ") )";

const string ConvertRaster4CRSSpec ="( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>region -> rasterRegion</text--->"
       "<text>ConvertRaster4CRS (_)</text--->"
       "<text>Creates a raster region from a region, "
       "using the 4-Color Raster Signature.</text--->"
       "<text>query ConvertRaster4CRS (region)</text--->"
       ") )";

const string rIntersectsSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
     "( <text>(rasterRegion || rasterLine || rasterPoints, "
     "rasterRegion || rasterLine || rasterPoints) -> bool</text--->"
       "<text>_ rIntersects _</text--->"
       "<text>Tests if two rasterRegion, rasterLine or RasterPoints "
       "intersects each other.</text--->"
       "<text>query rasterRegion1 rIntersects rasterRegion2</text--->"
       ") )";


//12 Definition of Operators


Operator RegionToPolilyne (
  "regionToPolilyne",     //name
  RegionToPolilyneSpec,         //specification
  RegionToPolilyneFun,    //value mapping
  //Operator::DummyModel,  //dummy model mapping, defined in Algebra.h
  Operator::SimpleSelect,  //trivial selection function
  RegionBool    //type mapping
);

Operator ConvertRaster3CRS ("convertRaster3CRS", ConvertRaster3CRSSpec, 3,
     convertraster3crsmap, ConvertRasterSelect, PointsLineRegionCRasterRegion);

Operator ConvertRaster4CRS ("convertRaster4CRS", ConvertRaster4CRSSpec,
     ConvertRaster4CRSFun, Operator::SimpleSelect, RegionCRasterRegion);

Operator rIntersects ("rIntersects", rIntersectsSpec, 9, rIntersectsmap,
     rIntersectsSelect, CRasterRegionCRasterLineCRasterPointsBool);


//13 Algebras


class RasterSpatialAlgebra : public Algebra
{
 public:
  RasterSpatialAlgebra() : Algebra()
  {

    AddTypeConstructor( &RasterRegion );
    RasterRegion.AssociateKind(Kind::SPATIAL2D());
    RasterRegion.AssociateKind(Kind::DATA());

    AddTypeConstructor( &RasterLine );
    RasterLine.AssociateKind(Kind::SPATIAL2D());
    RasterLine.AssociateKind(Kind::DATA());

    AddTypeConstructor( &RasterPoints );
    RasterPoints.AssociateKind(Kind::SPATIAL2D());
    RasterPoints.AssociateKind(Kind::DATA());

    AddOperator( &RegionToPolilyne );
    AddOperator( &ConvertRaster3CRS );
    AddOperator( &ConvertRaster4CRS );
    AddOperator( &rIntersects );
  }
  ~RasterSpatialAlgebra() {};
};


} // end of namespace rasterspatial


//14 Initialization


extern "C"
Algebra*
InitializeRasterSpatialAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new rasterspatial::RasterSpatialAlgebra());
}

