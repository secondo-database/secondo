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


#include "RasterAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"  //needed because we return a CcBool in an op.
#include <string>
#include "Attribute.h"
#include "Symbols.h"

#include "Algebras/Spatial/SpatialAlgebra.h"
#include <fstream>

#include "./Signature/Signature4CRS.h"
#include "./Signature/GenerateRaster.cpp"
#include "./Signature/CompareRaster.cpp"
#include "Tools/Flob/DbArray.h"


//#define DEBUGMESSAGES
using namespace std;

typedef unsigned char RasterBit;

extern NestedList* nl;
extern QueryProcessor *qp;

extern ListExpr OutPoints( ListExpr typeInfo, Word value );
extern Word InPoints( const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct );

extern long compareSignatures4CRS( Signature4CRS *assinat4crs1,
                 Signature4CRS *assinat4crs2, MBR &mbrIntersecao);


//void printSignature(const Signature4CRS *raster4CRS);

/*
1. Classes

*/

Raster4CRS::~Raster4CRS() {  }

Raster4CRS* Raster4CRS::Clone() const{
  //cout<<"Raster4CRS::Clone"<<endl;
  //printSignature(this);
  return new Raster4CRS( *this );
}

int Raster4CRS::NumOfFLOBs(void) const {
  return 1;
}

Flob *Raster4CRS::GetFLOB(const int i){
    assert(i == 0);

    return &rasterFLOB;
}

Raster4CRS& Raster4CRS::operator=( Raster4CRS& r )
{
  signatureType = r.signatureType;

  rasterFLOB.clean();
  if( r.rasterFLOB.Size() > 0 )
  {
    rasterFLOB.resize( r.rasterFLOB.Size() );
    for( int i = 0; i < r.rasterFLOB.Size(); i++ )
    {
      unsigned long l;
      r.rasterFLOB.Get( i, l );
      rasterFLOB.Put( i, l );
    }

    FLOBToRaster4CRS();
  } else {
    RasterMap4CRS *rasterMap = new RasterMap4CRS(1, r.map->mbr, r.map->dx,
                    r.map->dy, r.map->potency);
    Signature4CRS::Weight weight;
    long cellSize = 1l << r.map->potency;
    int x = 0;
    int y = 0;
    long minXcell = r.map->mbr.min.x - (r.map->mbr.min.x % cellSize)
                    - (cellSize * (r.map->mbr.min.x < 0
                    && (r.map->mbr.min.x % cellSize != 0) ? 1 : 0));
    long minYcell = r.map->mbr.min.y - (r.map->mbr.min.y % cellSize)
                    - (cellSize * (r.map->mbr.min.y < 0
                    && (r.map->mbr.min.y % cellSize != 0) ? 1 : 0));

    for( long i=minXcell; i <= r.map->mbr.max.x; i+=cellSize, x++) {
      y = 0;
      for(long j=minYcell; j <= r.map->mbr.max.y; j+=cellSize, y++) {
        weight = r.block(i,j,cellSize);
        rasterMap->block(x, y, weight);
      }
    }
    setRaster(*rasterMap);
  }

  return *this;
}

void Raster4CRS::Raster4CRSToFLOB(){
  Signature4CRS::Weight weight;

  rasterFLOB.clean();

  rasterFLOB.Append(signatureType);
  rasterFLOB.Append(map->potency);
  rasterFLOB.Append(map->dx);
  rasterFLOB.Append(map->dy);
  rasterFLOB.Append((unsigned long)map->mbr.min.x);
  rasterFLOB.Append((unsigned long)map->mbr.min.y);
  rasterFLOB.Append((unsigned long)map->mbr.max.x);
  rasterFLOB.Append((unsigned long)map->mbr.max.y);

  long cellSize = 1l << map->potency;
  long computedCells = 0;
  unsigned long FLOBelement = 0;

  int x = 0;
  int y = 0;
  long minXcell = map->mbr.min.x - (map->mbr.min.x % cellSize)
      - (cellSize * (map->mbr.min.x < 0
      && (map->mbr.min.x % cellSize != 0)? 1 : 0));
  long minYcell = map->mbr.min.y - (map->mbr.min.y % cellSize)
      - (cellSize * (map->mbr.min.y < 0
      && (map->mbr.min.y % cellSize != 0) ? 1 : 0));

  for( long i=minXcell; i <= map->mbr.max.x; i+=cellSize, x++) {
    y = 0;
    for(long j=minYcell; j <= map->mbr.max.y; j+=cellSize, y++)
    {
      weight = block(x,y);
      FLOBelement = (FLOBelement << 2) | weight;
      computedCells++;
      if (computedCells == (sizeof (unsigned long) * 4) ) {
        rasterFLOB.Append(FLOBelement);
        FLOBelement = 0;
        computedCells = 0;
      }
    }
  }

  if (computedCells > 0)
    rasterFLOB.Append(FLOBelement);

}

void Raster4CRS::FLOBToRaster4CRS(){
  unsigned long potency, dx, dy;
  long mbrMinX, mbrMinY, mbrMaxX, mbrMaxY;
  unsigned long l;

  rasterFLOB.Get(0, l);
  signatureType = l;
  rasterFLOB.Get(1, l);
  potency = l;
  rasterFLOB.Get(2, l);
  dx = l;
  rasterFLOB.Get(3, l);
  dy = l;
  rasterFLOB.Get(4, l);
  mbrMinX = (long)l;
  rasterFLOB.Get(5, l);
  mbrMinY = (long)l;
  rasterFLOB.Get(6, l);
  mbrMaxX = (long)l;
  rasterFLOB.Get(7, l);
  mbrMaxY = (long)l;

  long cellSize = 1l << potency;

  MBR mbr;
  mbr.min.x = mbrMinX;
  mbr.min.y = mbrMinY;
  mbr.max.x = mbrMaxX;
  mbr.max.y = mbrMaxY;

  RasterMap4CRS *rasterMap = new RasterMap4CRS(1, mbr, dx, dy, potency);

  unsigned long pFLOBelement;
  unsigned long FLOBelement;
  int position = -1;
  unsigned int currentCell = 0;
  Signature4CRS::Weight ocup;

  int positionInFLOB = 0;
  long int minXcell = (long int)(mbrMinX - ((long int)mbrMinX % cellSize)
            - (cellSize * ((mbrMinX < 0)
            && ((long int)mbrMinX % cellSize != 0) ? 1 : 0)));
  long int minYcell = (long int)(mbrMinY - ((long int)mbrMinY % cellSize)
            - (cellSize * ((mbrMinY < 0)
            && ((long int)mbrMinY % cellSize != 0)? 1 : 0)));
  for( long i=minXcell; i <= mbrMaxX; i+=cellSize)
    for(long j=minYcell; j <= mbrMaxY; j+=cellSize)
    {
      if (position < 0){
        // the 8 itens at the beggining are
        //signatureType, potency, dx, dy and mbr properties
        rasterFLOB.Get(positionInFLOB + 8, pFLOBelement);
        FLOBelement = pFLOBelement;
        //cout << "FLOBelement = " << FLOBelement << std::endl;
        position = 0;
        if (dx * dy - currentCell < sizeof(unsigned long) * 4)
          position = (dx * dy - currentCell) - 1;
        else
          position = sizeof(unsigned long) * 4 - 1;
        positionInFLOB++;
      }
      //each position has 2 bits. position * 2 makes me come to the position
      //of the current bit.
      //&3 makes me reak only the two last bits (3 -> 00...000011)
      switch ( (FLOBelement >> (position * 2)) & 3 ) {
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
      rasterMap->block(i, j, ocup);

      position--;
      currentCell++;
    }

  setRaster(*rasterMap);

}


//2. ~In~ and ~Out~ Functions



//2.1 Raster4CRS

//( id type min.x min.y max.x max.y potency
//  dx dy (weight[0] weight[1] weight[2] ...) )
ListExpr
OutRaster4CRS( ListExpr typeInfo, Word value )
{
  #ifdef DEBUGMESSAGES
  cout << "OutRaster4CRS" << std::endl;
  #endif
  Raster4CRS* raster = (Raster4CRS*)(value.addr);
  ListExpr ocup = nl->OneElemList( nl->IntAtom(raster->block( 0, 0 )) );
  ListExpr last = ocup;
  for( unsigned i = 1; i < raster->map->dx * raster->map->dy; i++ )
  {
    last = nl->Append( last, nl->IntAtom(raster->block( i % raster->map->dx,
                   (unsigned long int)(i / raster->map->dx) )) );
  }

  ListExpr cabec = nl->OneElemList(nl->IntAtom(raster->map->id));
  last = cabec;
  last = nl->Append(cabec, nl->IntAtom(raster->signatureType));
  last = nl->Append(last, nl->IntAtom(raster->map->mbr.min.x));
  last = nl->Append(last, nl->IntAtom(raster->map->mbr.min.y));
  last = nl->Append(last, nl->IntAtom(raster->map->mbr.max.x));
  last = nl->Append(last, nl->IntAtom(raster->map->mbr.max.y));
  last = nl->Append(last, nl->IntAtom(raster->map->potency));
  last = nl->Append(last, nl->IntAtom(raster->map->dx));
  last = nl->Append(last, nl->IntAtom(raster->map->dy));
  last = nl->Append(last, ocup);

  #ifdef DEBUGMESSAGES
  string listRaster;
  nl->WriteToString (listRaster, cabec);
  cout << "Result of outRaster4CRS:" << listRaster << endl;
  #endif

  return cabec;
}

Word
InRaster4CRS( const ListExpr typeInfo, const ListExpr instance,
          const int errorPos, ListExpr& errorInfo, bool& correct )
{
  #ifdef DEBUGMESSAGES
  cout << "InRaster4CRS" << std::endl;

  string listaRaster;
  nl->WriteToString (listaRaster, instance);
  cout << "Input of InRaster4CRS:" << listaRaster << endl;
  #endif
  unsigned long dx, dy;
  if ( nl->ListLength( instance ) == 10 )
  {
    ListExpr idAtom = nl->First(instance);
    ListExpr Rest = nl->Rest(instance);
    ListExpr typeAtom = nl->First(Rest);
    Rest = nl->Rest(Rest);
    ListExpr minXAtom = nl->First(Rest);
    Rest = nl->Rest(Rest);
    ListExpr minYAtom = nl->First(Rest);
    Rest = nl->Rest(Rest);
    ListExpr maxXAtom = nl->First(Rest);
    Rest = nl->Rest(Rest);
    ListExpr maxYAtom = nl->First(Rest);
    Rest = nl->Rest(Rest);
    ListExpr sizeAtom = nl->First(Rest);
    Rest = nl->Rest(Rest);
    ListExpr dxAtom = nl->First(Rest);
    Rest = nl->Rest(Rest);
    ListExpr dyAtom = nl->First(Rest);
    Rest = nl->Rest(Rest);

    if ( nl->IsAtom(idAtom) && nl->AtomType(idAtom) == IntType
        && nl->IsAtom(typeAtom) && nl->AtomType(typeAtom) == IntType
        && nl->IsAtom(minXAtom) && nl->AtomType(minXAtom) == IntType
        && nl->IsAtom(minYAtom) && nl->AtomType(minYAtom) == IntType
        && nl->IsAtom(maxXAtom) && nl->AtomType(maxXAtom) == IntType
        && nl->IsAtom(maxYAtom) && nl->AtomType(maxYAtom) == IntType
        && nl->IsAtom(sizeAtom) && nl->AtomType(sizeAtom) == IntType
        && nl->IsAtom(dxAtom) && nl->AtomType(dxAtom) == IntType
        && nl->IsAtom(dyAtom) && nl->AtomType(dyAtom) == IntType )
    {
      MBR mbr( Coordinate( nl->IntValue( minXAtom ), nl->IntValue( minYAtom ) ),
              Coordinate( nl->IntValue( maxXAtom ), nl->IntValue( maxYAtom ) ));
      dx = nl->IntValue( dxAtom );
      dy = nl->IntValue( dyAtom );
      Signature4CRS::RasterMap4CRS mapaRaster4CRS;
      mapaRaster4CRS = Signature4CRS::RasterMap4CRS(nl->IntValue( idAtom ), mbr,
                                         dx, dy, nl->IntValue( sizeAtom ));
      mapaRaster4CRS.setGroupOfBits(Signature4CRS::Empty);
      Rest = nl->First( Rest );
      for (unsigned long int i = 0; !nl->IsEmpty( Rest ); i++ )
      {
        ListExpr first = nl->First( Rest );
        Rest = nl->Rest( Rest );

        if( nl->IsAtom( first ) && nl->AtomType( first ) == IntType )
        {
          if ( nl->IntValue( first ) == 0x0 )
            mapaRaster4CRS.block(i % dx, (unsigned long int)(i / dx),
                Signature4CRS::Empty);
          else if ( nl->IntValue( first ) == 0x1 )
            mapaRaster4CRS.block(i % dx, (unsigned long int)(i / dx),
                Signature4CRS::Weak);
          else if ( nl->IntValue( first ) == 0x2 )
            mapaRaster4CRS.block(i % dx, (unsigned long int)(i / dx),
                Signature4CRS::Strong);
          else if ( nl->IntValue( first ) == 0x3 )
            mapaRaster4CRS.block(i % dx, (unsigned long int)(i / dx),
                Signature4CRS::Full);
          else
          {
            correct = false;
            return SetWord( Address(0) );
          }
        }
        else
        {
          correct = false;
          return SetWord( Address(0) );
        }
      }

    Raster4CRS *raster = new Raster4CRS(mapaRaster4CRS, nl->IntValue(typeAtom));
      correct = true;
      //return SetWord((Signature4CRS *) new Signature4CRS(mapaRaster4CRS));
      return SetWord(raster);
    }
  }
  correct = false;
  return SetWord(Address(0));

}



//3. Persistent Storage and Related Generic Functions



//3.1 Raster4CRS


Word
CreateRaster4CRS( const ListExpr typeInfo )
{
  //cout << "Entrou no CreateRaster4CRS" << endl;
  return (SetWord( new Raster4CRS( 1, Coordinate (0,0),
           Coordinate (0,0), 0, 0, 0, NULL, 3 )));
  //cout << "Saiu do CreateRaster4CRS" << endl;
}

void
DeleteRaster4CRS( const ListExpr typeInfo, Word& w )
{
  //cout << "Entrou no DeleteRaster4CRS" << endl;
  delete (Raster4CRS *)w.addr;
  w.addr = 0;
}

void
CloseRaster4CRS( const ListExpr typeInfo, Word& w )
{
  //cout << "Entrou no CloseRaster4CRS" << endl;
  delete (Raster4CRS *)w.addr;
  w.addr = 0;
}

Word
CloneRaster4CRS( const ListExpr typeInfo, const Word& w )
{
  //cout << "Entrou no CloneRaster4CRS" << endl;
  //return SetWord( ((Raster4CRS *)w.addr)->Clone() );

  Raster4CRS *cr = new Raster4CRS( *((Raster4CRS *)w.addr) );
  return SetWord( cr );
}

int
SizeOfRaster4CRS()
{
  return sizeof(Raster4CRS);
}

bool
OpenRaster4CRS( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  #ifdef DEBUGMESSAGES
  cout << "entrou no openRaster4CRS" << std::endl;
  #endif
  Raster4CRS *r = (Raster4CRS*)Attribute::Open( valueRecord, offset, typeInfo );

  //r->FLOBToRaster4CRS();
  value = SetWord( r );
  return true;
}

bool
SaveRaster4CRS( SmiRecord& valueRecord,
             size_t& offset,
             const ListExpr typeInfo,
             Word& value )
{
  #ifdef DEBUGMESSAGES
  cout << "entrou no saveRaster4CRS" << std::endl;
  #endif

  Raster4CRS *r = (Raster4CRS *)value.addr;
  //cout << "dx no no saveRaster4CRS: " << r->map->dx << endl;
  r->Raster4CRSToFLOB();

  Attribute::Save( valueRecord, offset, typeInfo, r );

  return true;
}

void* CastRaster4CRS( void* addr ) {
  //cout << "Entrou no CastRaster4CRS" << endl;
  return (new (addr) Raster4CRS);
}


//4 Properties


ListExpr
Raster4CRSProperty()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
           nl->StringAtom("List Rep"),
           nl->StringAtom(""),
           nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                       nl->StringAtom(Raster4CRS::BasicType()),
           nl->StringAtom("( <id> <type> <min.x> <min.y> <max.x> <max.y> "),
           nl->StringAtom("<sizeOfBlock> <dx> <dy> (<weight[i]>*))"),
           nl->StringAtom("all values must be of type int."))));
}


//5 Kind Checking

bool
CheckRaster4CRS( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, Raster4CRS::BasicType() ));
}


//6 Type Constructors

TypeConstructor TCRaster4CRS(
  Raster4CRS::BasicType(),                //name
  Raster4CRSProperty,                 //property function describing signature
        OutRaster4CRS, InRaster4CRS,            //Out and In functions
        0, 0,                        //SaveToList and RestoreFromList functions
  CreateRaster4CRS, DeleteRaster4CRS,      //object creation and deletion
  OpenRaster4CRS, SaveRaster4CRS,   //object open and save
  CloseRaster4CRS, CloneRaster4CRS, //object close, and clone
  CastRaster4CRS,              //cast function
        SizeOfRaster4CRS,             //sizeof function
  CheckRaster4CRS                        //kind checking function
);


//7 Validation functions


//validation for function: int IntersectsRaster(Raster4CRS, Raster4CRS)
ListExpr
Raster4CRSRaster4CRSInt( ListExpr args )
{
  if ( nl->ListLength(args) == 2 )
  {
    ListExpr arg1 = nl->First(args);
    ListExpr arg2 = nl->Second(args);
    if ( nl->IsEqual(arg1, Raster4CRS::BasicType()) &&
         nl->IsEqual(arg2, Raster4CRS::BasicType()) )
      return nl->SymbolAtom(CcInt::BasicType());
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

ListExpr
RegionLinePointsRaster4CRS( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);
    if ( nl->IsEqual(arg1, Region::BasicType()) ||
         nl->IsEqual(arg1, Line::BasicType())
         || nl->IsEqual(arg1, Points::BasicType()))
      return nl->SymbolAtom(Raster4CRS::BasicType());
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}


//8 Functions and Auxiliary functions



//8.1 General Functions


void printSignature(const Signature4CRS *raster4CRS)
{
    for(int y=raster4CRS->map->dy - 1;y >= 0;y--)
    {
      for(unsigned int x=0;x<raster4CRS->map->dx;x++)
      {
  Signature4CRS::Weight weight = raster4CRS->block(x,y);
        //cout<<"("<<x<<","<<y<<"): ";
        switch( weight )
  {
          case Signature4CRS::Empty : cout<<". "; break;
    case Signature4CRS::Full : cout<<"# "; break;
          case Signature4CRS::Strong : cout<<"X "; break;
          case Signature4CRS::Weak : cout<<"o "; break;
        }
      }
      cout<<endl;
    }
}


//8.2 Calculate Raster


static int
CalcRasterFun (Word* args, Word& result, int message, Word& local, Supplier s,
              SignatureType signature, RasterType rasterType)
{
  HalfSegment chs;
  Raster4CRS* raster4CRS;
  Signature4CRS* assinatura;

  Region *r1 = NULL;
  Line *l1 = NULL;
  Points *p1 = NULL;
  switch(rasterType){
      case RT_REGION:
      r1 = ((Region*)args[0].addr);
      break;
      case RT_LINE:
      l1 = ((Line*)args[0].addr);
      break;
      case RT_POINTS:
      p1 = ((Points*)args[0].addr);
  #ifdef DEBUGMESSAGES
    const Point *pt;
    p1->SelectFirst();
      p1->GetPt(pt);
      cout << "pt original = " << pt->GetX() << ", " << pt->GetY() << std::endl;
  #endif
      break;
  }
  {

    int potency =0;
    do
    {
      assinatura=GeraRasterSecondo::generateRaster( 1, r1, l1, p1,
                        potency, signature);

      potency++;
    } while (raster4CRS==NULL);
    #ifdef DEBUGMESSAGES
      cout << "Result of CalcRasterFun:" << endl;
      printSignature(assinatura);
    #endif
  }

  result = qp->ResultStorage(s);  //query processor has provided

  raster4CRS = new Raster4CRS(assinatura->fullMap(), 3);
  if (raster4CRS == NULL) {
    cout << "raster4CRS == NULL" << std::endl;
  return 1;
  }
  if (signature == SIGNAT_3CRS)
  raster4CRS->signatureType = 3;
  else if (signature == SIGNAT_4CRS)
  raster4CRS->signatureType = 4;
  else
  raster4CRS->signatureType = 0;
  *((Raster4CRS *)result.addr) = *raster4CRS;
  return 0;
}

static int
Calc4CRSFun (Word* args, Word& result, int message, Word& local, Supplier s)
{
  return CalcRasterFun(args, result, message, local, s, SIGNAT_4CRS, RT_REGION);
}

static int
Calc3CRS_p (Word* args, Word& result, int message, Word& local, Supplier s){
  return CalcRasterFun (args, result, message, local, s, SIGNAT_3CRS,RT_POINTS);
}

static int
Calc3CRS_l (Word* args, Word& result, int message, Word& local, Supplier s){
  return CalcRasterFun (args, result, message, local, s, SIGNAT_3CRS, RT_LINE);
}

static int
Calc3CRS_r (Word* args, Word& result, int message, Word& local, Supplier s){
  return CalcRasterFun (args, result, message, local, s, SIGNAT_3CRS,RT_REGION);
}





//8.6 Intersetcs Functions


int
IntersectsRasterFun (Word* args, Word& result, int message,
                 Word& local, Supplier s)
{
  Raster4CRS *r1 = ((Raster4CRS*)args[0].addr);
  Raster4CRS *r2 = ((Raster4CRS*)args[1].addr);

  int intersection = -1;

  result = qp->ResultStorage(s);  //query processor has provided
            //a CcBool instance to take the result
  MBR mbrIntersecao;
  intersection = compareSignatures4CRS( r1, r2, mbrIntersecao);

  ((CcInt*)result.addr)->Set(true, intersection);
            //the first argument says the boolean
            //value is defined, the second is the
            //real boolean value)
  return 0;
}



//9 Value Mapping


ValueMapping calc3CRSmap[] = {
  Calc3CRS_p,
  Calc3CRS_l,
    Calc3CRS_r
};


//10 Select functions


int
calc3CRSSelect( ListExpr args )
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


//11 Specification of operators

const string Calc3CRSSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
       "\"Example\" ) "
       "( <text>(region || line || points) -> Raster4CRS</text--->"
       "<text>Calc3CRS (_)</text--->"
       "<text>Calculates 3CRS.</text--->"
       "<text>query Calc3CRS (testRegion)</text--->"
       ") )";

const string Calc4CRSSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(region) -> Raster4CRS</text--->"
       "<text>Calc4CRS (_)</text--->"
       "<text>Calculates 4CRS.</text--->"
       "<text>query Calc4CRS (testRegion)</text--->"
       ") )";

const string IntersectsRasterSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" ) "
                         "( <text>(Raster4CRS, Raster4CRS) -> int</text--->"
       "<text>IntersectsRaster (_)</text--->"
"<text>Indicates whether two raster signatures intersect each other.</text--->"
"<text>query IntersectsRaster (rasterSignature1, rasterSignature2)</text--->"
       ") )";


//12 Definition of Operators


Operator Calc3CRS (  "calc3CRS",  Calc3CRSSpec, 3, calc3CRSmap,
           calc3CRSSelect, RegionLinePointsRaster4CRS);

Operator Calc4CRS (
  "calc4CRS",     //name
  Calc4CRSSpec,         //specification
  Calc4CRSFun,    //value mapping
  //Operator::DummyModel,  //dummy model mapping, defined in Algebra.h
  Operator::SimpleSelect,  //trivial selection function
  RegionLinePointsRaster4CRS  //type mapping
);

Operator IntersectsRaster (
  "intersectsRaster",     //name
  IntersectsRasterSpec,         //specification
  IntersectsRasterFun,    //value mapping
  //Operator::DummyModel,  //dummy model mapping, defined in Algebra.h
  Operator::SimpleSelect,  //trivial selection function
  Raster4CRSRaster4CRSInt    //type mapping
);


//13 Algebras


class RasterAlgebra : public Algebra
{
 public:
  RasterAlgebra() : Algebra()
  {
    AddTypeConstructor( &TCRaster4CRS );
    TCRaster4CRS.AssociateKind("SIMPLE");

    AddOperator( &Calc3CRS );
    AddOperator( &Calc4CRS );
    AddOperator( &IntersectsRaster );
  }
  ~RasterAlgebra() {};
};


//14 Initialization


extern "C"
Algebra*
InitializeRasterAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new RasterAlgebra());
}
