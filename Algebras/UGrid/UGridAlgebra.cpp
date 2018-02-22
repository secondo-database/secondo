/******************************************************************************
----
This file is part of SECONDO.

Copyright (C) 2004-2010, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

//paragraph [1] Title: [{\Large \bf] [}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Declaration of UGridAlgebra

September 2010, Daniel Brockmann

1 Overview

UGrid-Algebra implements a UGrid index structure. The memory management is
based on SmiUpdateFile.

UGrid-Algebra offers the following methods:

- createUGrid       -> Creates a new UGrid object.
- insertUpload      -> Inserts a single upload into UGrid.
- insertStream      -> Inserts a stream of uploads into UGrid.
- intersectsWindow  -> Returns all history units which
                       intersect the search window.
- insideWindow      -> Returns all history units
                       inside the search window.
- getTrajectory     -> Returns all history units wich belongs
                       to the stated moving object.
- currentUpload     -> Returns the current upload.

2 Includes and globals

******************************************************************************/

#include <iostream>
#include <fstream>
#include <map>
#include "WinUnix.h"
#include "SecondoSystem.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "Attribute.h"
#include "DateTime.h"
#include "Algebras/RTree/RTreeAlgebra.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/SETI/UploadUnit.h"
#include "Symbols.h"

using namespace std;
using namespace temporalalgebra;
using namespace datetime;

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

namespace UGridAlgebra {

#include "UGridAlgebra.h"

/******************************************************************************

3. Auxiliary functions

3.1 CompareFloats

Returns 'true' if the two floating-point numbers are almost equal.

******************************************************************************/

bool CompareFloats(double F1 ,double F2)
{
  if ( F1 <= (F2 + 0.001) &&
       F1 >= (F2 - 0.001) )
  {
    return true;
  }
  else return false;
}

/******************************************************************************

3.2 ModifyArea

Modifies the given area. x1/x2 and y1/y2 will be swapped if necessary.

******************************************************************************/

UGridArea ModifyArea(UGridArea AREA)
{
  double tempX = 0;
  double tempY = 0;

  if (AREA.x1 > AREA.x2)
  {
    tempX   = AREA.x2;
    AREA.x2 = AREA.x1;
    AREA.x1 = tempX;
  }
  if (AREA.y1 > AREA.y2)
  {
    tempY   = AREA.y2;
    AREA.y2 = AREA.y1;
    AREA.y1 = tempY;
  }
  AREA.x1 = AREA.x1 - tol;
  AREA.y1 = AREA.y1 - tol;
  AREA.x2 = AREA.x2 + tol;
  AREA.y2 = AREA.y2 + tol;
  return AREA;
}

/******************************************************************************

3.3 ComputeLine

Identifies a column or a row in a grid in dependence of a given position.

******************************************************************************/

int ComputeLine(double BORDER1 ,double BORDER2, int SPLITS, double POS)
{
  double len = abs(BORDER1-BORDER2) / SPLITS;
  int i = 0;

  while ((BORDER1 + (i*len)) <= POS) i++;

  return i-1;
}

/******************************************************************************

3.4 ComputeIntersection

Computes the intersection point of two lines (A,B), (C,D).

******************************************************************************/

bool ComputeIntersection( double Ax, double Ay,
                          double Bx, double By,
                          double Cx, double Cy,
                          double Dx, double Dy,
                          UnitPos* is )
{

  double  distAB, cos, sin, newX, ABpos ;

  if ( ((Ax==Bx) && (Ay==By)) || ((Cx==Dx) && (Cy==Dy)) ) return false;

  if ( ((Ax==Cx) && (Ay==Cy)) || ((Bx==Cx) && (By==Cy))
        ||  ((Ax==Dx) && (Ay==Dy)) || ((Bx==Dx) && (By==Dy)) ) return false;

  Bx-=Ax; By-=Ay;
  Cx-=Ax; Cy-=Ay;
  Dx-=Ax; Dy-=Ay;

  distAB=sqrt(Bx*Bx+By*By);

  cos=Bx/distAB;
  sin=By/distAB;
  newX=Cx*cos+Cy*sin;
  Cy  =Cy*cos-Cx*sin; Cx=newX;
  newX=Dx*cos+Dy*sin;
  Dy  =Dy*cos-Dx*sin; Dx=newX;

  if ( ((Cy<0) && (Dy<0)) || ((Cy>=0) && (Dy>=0)) ) return false;

  ABpos=Dx+(Cx-Dx)*Dy/(Dy-Cy);

  if ( (ABpos<0) || (ABpos>distAB) ) return false;

  is->x = Ax+ABpos*cos;
  is->y = Ay+ABpos*sin;

  return true;
}

/******************************************************************************

3.5 CreateUGridBox

Creates an UGridBox which constists of a page number and a 3D-rectangle.

******************************************************************************/

UGridBox* CreateUGridBox( UGridCell* CELLPTR, UGridNode* NODEPTR )
{
  UGridBox* ugridBox = (UGridBox*)0;

  if ( CELLPTR != 0 )
  {
    Rectangle<3>* box = (Rectangle<3>*)0;
                  box = new Rectangle<3>(true,
                                    CELLPTR->pos1.x-tol, CELLPTR->pos2.x+tol,
                                    CELLPTR->pos1.y-tol, CELLPTR->pos2.y+tol,
                                    CELLPTR->tiv.start.ToDouble(),
                                    CELLPTR->tiv.end.ToDouble() );
    ugridBox = new UGridBox(CELLPTR->currentPage, true, *box);

    // Reset cell
    CELLPTR->pos1.x = 0;
    CELLPTR->pos1.y = 0;
    CELLPTR->pos2.x = 0;
    CELLPTR->pos2.y = 0;
    CELLPTR->tiv.lc = false;
    CELLPTR->tiv.rc = false;
    CELLPTR->numEntries = 0;

  }
  else if ( NODEPTR != 0 )
  {
    Rectangle<3>* box = (Rectangle<3>*)0;
                  box = new Rectangle<3>(true,
                                    NODEPTR->pos1.x-tol, NODEPTR->pos2.x+tol,
                                    NODEPTR->pos1.y-tol, NODEPTR->pos2.y+tol,
                                    NODEPTR->tiv.start.ToDouble(),
                                    NODEPTR->tiv.end.ToDouble());

    ugridBox = new UGridBox(NODEPTR->currentPage, false, *box);

    // Reset node
    NODEPTR->pos1.x = 0;
    NODEPTR->pos1.y = 0;
    NODEPTR->pos2.x = 0;
    NODEPTR->pos2.y = 0;
    NODEPTR->tiv.lc = false;
    NODEPTR->tiv.rc = false;
  }
  return ugridBox;
}

/******************************************************************************

3.6 ModifyUGridNode

Modifies the values of a UGridNode in dependence of the UgridBox.

******************************************************************************/

void ModifyUGridNode( UGridNode* NODEPTR, UGridBox* BOX )
{
  double x1(BOX->box.MinD(0));
  double x2(BOX->box.MaxD(0));
  double y1(BOX->box.MinD(1));
  double y2(BOX->box.MaxD(1));
  double t1(BOX->box.MinD(2));
  double t2(BOX->box.MaxD(2));

  Instant T1 = DateTime(0,0,instanttype);
  Instant T2 = DateTime(0,0,instanttype);
  T1.ReadFrom(t1);
  T2.ReadFrom(t2);

  if (x1 < NODEPTR->pos1.x) NODEPTR->pos1.x = x1;
  if (y1 < NODEPTR->pos1.y) NODEPTR->pos1.y = y1;
  if (x2 > NODEPTR->pos2.x) NODEPTR->pos2.x = x2;
  if (y2 > NODEPTR->pos2.y) NODEPTR->pos2.y = y2;

  if ((NODEPTR->tiv.lc == false) || (T1 < NODEPTR->tiv.start))
  {
     NODEPTR->tiv.start = T1;
     NODEPTR->tiv.lc = true;
  }
  if ((NODEPTR->tiv.rc == false) || (T2 > NODEPTR->tiv.end))
  {
     NODEPTR->tiv.end = T2;
     NODEPTR->tiv.rc = true;
  }

  NODEPTR->numEntries++;
}

/******************************************************************************

3.7 CheckUGridNode

Returns true if the UGridNode intersects the search window.

******************************************************************************/

bool CheckUGridNode( UGridArea AREA, Instant TIME1, Instant TIME2,
                      UGridNode* NODEPTR )
{
  if ( AREA.x1 <= NODEPTR->pos2.x && AREA.y1 <= NODEPTR->pos2.y &&
       AREA.x2 >= NODEPTR->pos1.x && AREA.y2 >= NODEPTR->pos1.y &&
       TIME1 <= NODEPTR->tiv.end  && TIME2 >= NODEPTR->tiv.start )
  {
    return true;
  }
  else
  {
    return false;
  }
}

/******************************************************************************

3.8 CheckUGridBox

Returns true if the UGridBox intersects the search window.

******************************************************************************/

bool CheckUGridBox(UGridArea AREA, Instant TIME1, Instant TIME2, UGridBox BOX)
{
  double boxPos1x(BOX.box.MinD(0));
  double boxPos2x(BOX.box.MaxD(0));
  double boxPos1y(BOX.box.MinD(1));
  double boxPos2y(BOX.box.MaxD(1));
  double boxT1(BOX.box.MinD(2));
  double boxT2(BOX.box.MaxD(2));

  Instant* boxTivStart = new DateTime(0,0,instanttype);
  Instant* boxTivEnd   = new DateTime(0,0,instanttype);
  boxTivStart->ReadFrom(boxT1);
  boxTivEnd->ReadFrom(boxT2);

  if ( AREA.x1 <= boxPos2x && AREA.y1 <= boxPos2y &&
       AREA.x2 >= boxPos1x && AREA.y2 >= boxPos1y &&
       TIME1 <= *boxTivEnd && TIME2 >= *boxTivStart )
  {
    delete boxTivStart;
    delete boxTivEnd;
    return true;
  }
  else
  {
    delete boxTivStart;
    delete boxTivEnd;
    return false;
  }
}

/******************************************************************************

4 Implementation of class UGrid

4.1 Basic constructor

******************************************************************************/

UGrid::UGrid(UGridArea AREA, int SPLITS) : suf(0)
{
  // Create UGrid Header
  header = new UGridHeader(ModifyArea(AREA), SPLITS);

  // Create SmiUpdateFile
  suf = new SmiUpdateFile(pageSize);
  suf->Create();
  header->fileID = suf->GetFileId();

  // Create header page
  SmiUpdatePage* headerPage;
  int AppendedPage = suf->AppendNewPage(headerPage);
  assert( AppendedPage );
  header->headerPageNo = headerPage->GetPageNo();

  // Create and initialize fontline pages
  SmiUpdatePage* flPage;
  db_pgno_t nextPageNo  = 0;
  int       numEntries  = 0;
  AppendedPage = suf->AppendNewPage(flPage);
  assert( AppendedPage );
  header->flPageNo = flPage->GetPageNo();
  int PageSelected = suf->GetPage(header->flPageNo, flPage);
  assert( PageSelected );
  size_t offset = 0;
  for(int i = 0; i < flBuckets; i++)
  {
    SmiUpdatePage* bucketPage;
    db_pgno_t bucketPageNo;

    // Create new bucket page
    AppendedPage = suf->AppendNewPage(bucketPage);
    assert( AppendedPage );
    bucketPageNo = bucketPage->GetPageNo();

    // Write bucket page number into flPage
    flPage->Write(&bucketPageNo, sizeof(db_pgno_t), offset);
    offset += sizeof(db_pgno_t);

    // Initialize bucket page
    PageSelected = suf->GetPage(bucketPageNo, bucketPage);
    assert( PageSelected );
    bucketPage->Write(&nextPageNo, sizeof(db_pgno_t), 0);
    bucketPage->Write(&numEntries, sizeof(int), sizeof(db_pgno_t));
    suf->PutPage(bucketPageNo, true);
  }

  // Create cell pages
  SmiUpdatePage* cellPage;
  numEntries  = 0;
  for(int i = 0; i < 8; i++)
  {
    for(int j = 0; j < 8; j++)
    {
      AppendedPage = suf->AppendNewPage(cellPage);
      assert( AppendedPage );
      header->cellPageNo[j][i] = cellPage->GetPageNo();
      // Initialize cell page
      int PageSelected = suf->GetPage(header->cellPageNo[j][i], cellPage);
      assert( PageSelected );
      cellPage->Write(&numEntries, sizeof(db_pgno_t), 0);
    }
  }

  // Create and initialize first node page
  SmiUpdatePage* nodePage;
  AppendedPage = suf->AppendNewPage(nodePage);
  assert( AppendedPage );
  header->nodePageNo = nodePage->GetPageNo();
  PageSelected = suf->GetPage(header->nodePageNo, nodePage);
  assert( PageSelected );
  nodePage->Write(&nextPageNo, sizeof(db_pgno_t), 0);

  // Create a quad-tree like structure
  header->rootNode = CreateUGridTree((UGridNode*)0, header->numCells,
                                      header->splits, header->splits);

  // Write header and tree into SmiUpdateFile
  UpdateHeader();
  UpdateUGridTree(header->rootNode, header->numCells,
                  header->splits, header->splits);

  // Initialize semaphore
  SetSemaphore(false);
}

/******************************************************************************

4.2 Query constructor

******************************************************************************/

UGrid::UGrid(SmiFileId FILEID) : suf(0)
{
  // Open extisting file
  suf = new SmiUpdateFile(pageSize);
  suf->Open(FILEID, pageSize);

  // Create header object
  header = new UGridHeader();
}

/******************************************************************************

4.3 Destructor

******************************************************************************/

UGrid::~UGrid()
{
  // Destruct UGrid-tree
  DestructUGridTree( header->rootNode, header->numCells );

  // Delete header
  delete header;

  // Clear frontline
  for (int i = 0; i < flBuckets; i++)
  {
    frontline[i].clear();
  }

  // Delete file object
  if(suf->IsOpen()) suf->Close();
  delete suf;
}

/******************************************************************************

4.4 The mandatory set of algebra support functions

4.4.1 In-method

******************************************************************************/

Word UGrid::In( ListExpr typeInfo, ListExpr instance, int errorPos,
                ListExpr& errorInfo, bool& correct )
{
  correct = false;
  Word result = SetWord(Address(0));

  // Check list length
  if ( nl->ListLength(instance) != 2 )
  {
    cmsg.inFunError("A list of length two expected!");
    return result;
  }

  // Check area types
  ListExpr areaList = nl->First(instance);
  if ( !(nl->ListLength(areaList) == 4 &&
         nl->IsAtom( nl->First(areaList) ) &&
         nl->AtomType( nl->First(areaList) ) == RealType &&
         nl->IsAtom( nl->Second(areaList) ) &&
         nl->AtomType( nl->Second(areaList ) ) == RealType &&
         nl->IsAtom( nl->Third(areaList) ) &&
         nl->AtomType( nl->Third(areaList) ) == RealType &&
         nl->IsAtom( nl->Fourth(areaList) ) &&
         nl->AtomType( nl->Fourth(areaList) ) == RealType ))
  {
    cmsg.inFunError("A list of four real type values is "
                    "expected for the first argument!");
    return result;
  }

  // Area values
  double x1 = nl->RealValue( nl->First(areaList) );
  double x2 = nl->RealValue( nl->Second(areaList) );
  double y1 = nl->RealValue( nl->Third(areaList) );
  double y2 = nl->RealValue( nl->Fourth(areaList) );

  if (( x2 == x1 ) || (y1 == y2 ))
  {
    cmsg.inFunError("x1/x2 and y1/y2 must be different!");
    return result;
  }
  UGridArea area(x1,y1,x2,y2);

  // Check number of partitions type
  if ( !(nl->IsAtom( nl->Second( instance ) ) &&
         nl->AtomType( nl->Second( instance ) ) == IntType ))
  {
    cmsg.inFunError("An integer value is expected for second argument!");
    return result;
  }

   // Check number of partitions
  int numPartitions = nl->IntValue( nl->Second(instance) );
  if (!(numPartitions == 4096 ||
        numPartitions ==  256 ||
        numPartitions ==   16 ||
        numPartitions ==    4))
  {
    // Wrong number of partitions -> create default UGrid
    result.addr = new UGrid(ModifyArea(area), 4);
  }
  else
  {
    result.addr = new UGrid(ModifyArea(area), (int)sqrt(numPartitions));
  }
  correct = true;
  return result;
}

/******************************************************************************

4.4.2 Out-method

******************************************************************************/

ListExpr UGrid::Out(ListExpr typeInfo, Word value )
{
  UGrid* ugridPtr = static_cast<UGrid*>(value.addr);

  // Create area list
  ListExpr area = nl->FourElemList(
  nl->RealAtom(ugridPtr->header->area.x1),
  nl->RealAtom(ugridPtr->header->area.x2),
  nl->RealAtom(ugridPtr->header->area.y1),
  nl->RealAtom(ugridPtr->header->area.y2));

  // Create time interval list
  ListExpr  tiv;
  if (ugridPtr->header->numEntries == 0)
  {
    // Time interval is undefined if no entry exist
    tiv = nl->TwoElemList( nl->SymbolAtom(Symbol::UNDEFINED()),
                           nl->SymbolAtom(Symbol::UNDEFINED()));
  }
  else if (ugridPtr->header->numEntries == 1)
  {
    // End time is undefined if only one entry exist
    tiv = nl->TwoElemList(
              OutDateTime(nl->TheEmptyList(),
                          SetWord(&ugridPtr->header->tiv.start)),
                          nl->SymbolAtom(Symbol::UNDEFINED()));
  }
  else
  {
    // Time interval is defined if UGrid has two entries or more
    tiv = nl->TwoElemList( OutDateTime(nl->TheEmptyList(),
                                       SetWord(&ugridPtr->header->tiv.start)),
                           OutDateTime(nl->TheEmptyList(),
                                       SetWord(&ugridPtr->header->tiv.end)));
  }
  // Return output list
  return nl->TwoElemList(area, tiv);
}

/******************************************************************************

4.4.3 Create-method

******************************************************************************/

Word UGrid::Create( const ListExpr typeInfo )
{
  return SetWord(Address(0));
}

/******************************************************************************

4.4.4 Delete-method

******************************************************************************/

void UGrid::Delete( const ListExpr typeInfo, Word& w )
{
  UGrid* ugridPtr = static_cast<UGrid*>(w.addr);
  delete ugridPtr;
  w.addr = 0;
}

/******************************************************************************

4.4.5 Open-method

******************************************************************************/

bool UGrid::Open( SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value )
{
  SmiFileId fileID;
  db_pgno_t headerPageNo;

  bool ok = true;
  ok = ok && valueRecord.Read( &fileID, sizeof(SmiFileId), offset );
  offset += sizeof(SmiFileId);
  ok = ok && valueRecord.Read( &headerPageNo, sizeof(db_pgno_t), offset );
  offset += sizeof(db_pgno_t);
  if(!ok){
    return false;
  }

  // Create new UGrid object with existing file
  UGrid* ugridPtr = new UGrid(fileID);
  ugridPtr->header->fileID = fileID;
  ugridPtr->header->headerPageNo = headerPageNo;

  // Reader header, frontline and UGridTree information from file
  ugridPtr->ReadHeader();
  ugridPtr->ReadFLine();
  UGridHeader* hPtr = ugridPtr->header;
  hPtr->rootNode = ugridPtr->ReadUGridTree( (UGridNode*)0,
                   hPtr->area, hPtr->numCells, hPtr->splits, hPtr->splits );

  value.addr = ugridPtr;
  return ok;
}

/******************************************************************************

4.4.6 Save-method

******************************************************************************/

bool UGrid::Save( SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value )
{
  UGrid* ugridPtr = static_cast<UGrid*>(value.addr);
  bool ok = true;
  SmiFileId fileID = ugridPtr->header->fileID;
  db_pgno_t headerPageNo = ugridPtr->header->headerPageNo;
  ok = ok && valueRecord.Write(&fileID, sizeof(SmiFileId), offset );
  offset += sizeof(SmiFileId);
  ok = ok && valueRecord.Write(&headerPageNo, sizeof(db_pgno_t), offset );
  offset += sizeof(db_pgno_t);
  return ok;
}

/******************************************************************************

4.4.7 Close-method

******************************************************************************/

void UGrid::Close( const ListExpr typeInfo, Word& w )
{
  UGrid* ugridPtr = static_cast<UGrid*>(w.addr);
  delete ugridPtr;
  w.addr = 0;
}

/******************************************************************************

4.4.8 Clone-method

******************************************************************************/

Word UGrid::Clone( const ListExpr typeInfo, const Word& w )
{
  return SetWord(Address(0));
}

/******************************************************************************

4.4.9 Cast-method

******************************************************************************/

void* UGrid::Cast( void* addr)
{
  return (0);
}

/******************************************************************************

4.4.10 SizeOfObj-method

******************************************************************************/

int UGrid::SizeOfObj()
{
  return sizeof(UGrid);
}

/******************************************************************************

4.4.11 KindCheck-method

******************************************************************************/

bool UGrid::KindCheck(ListExpr type, ListExpr& errorInfo)
{
  return (nl->IsEqual(type, UGrid::BasicType()));
}

/******************************************************************************

4.4.12 Property-method

******************************************************************************/

ListExpr UGrid::Property()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom(UGrid::BasicType()),
                             nl->StringAtom("((<x1> <x2> <y1> <y2>) p)"),
                             nl->StringAtom("((8.2 1.6 9.7 4,6) 4096)"),
                             nl->StringAtom("x/y must be of type real, "
                                            "p of type int."))));
}

/******************************************************************************

4.5 CreateUGridTree

Creates an UGridTree (quad-tree like structure) and initializes all UGridCells
(leaf nodes).

******************************************************************************/

UGridNode* UGrid::CreateUGridTree( UGridNode* FATHERNODE, int SQUARES,
                                   int RIGHTCOL, int TOPROW )
{
  // Create and initialize UGrideNode
  UGridNode* node   = new UGridNode();
  node->fatherNode  = FATHERNODE;
  node->numEntries  = 0;
  node->currentPage = (db_pgno_t)0;
  node->pos1        = UnitPos(0,0);
  node->pos2        = UnitPos(0,0);
  node->tiv.start   = DateTime(0,0,instanttype);
  node->tiv.end     = DateTime(0,0,instanttype);
  node->tiv.lc      = false;
  node->tiv.rc      = false;

  if (SQUARES > 4)
  {
    // Create left-bottom son
    node->leftBottomSon = CreateUGridTree( node, (SQUARES/4),
                                             (RIGHTCOL-((int)sqrt(SQUARES)/2)),
                                              (TOPROW-((int)sqrt(SQUARES)/2)));
    // Create right-bottom son
    node->rightBottomSon = CreateUGridTree( node, (SQUARES/4), RIGHTCOL,
                                             (TOPROW-((int)sqrt(SQUARES)/2)));
    // Create left-top son
    node->leftTopSon = CreateUGridTree( node, (SQUARES/4),
                                   (RIGHTCOL-((int)sqrt(SQUARES)/2)), TOPROW );
    // Create right-top son
    node->rightTopSon = CreateUGridTree( node, (SQUARES/4), RIGHTCOL, TOPROW );

    node->leftBottomCell  = (UGridCell*)0;
    node->rightBottomCell = (UGridCell*)0;
    node->leftTopCell     = (UGridCell*)0;
    node->rightTopCell    = (UGridCell*)0;
  }
  else
  {
    node->leftBottomSon  = (UGridNode*)0;
    node->rightBottomSon = (UGridNode*)0;
    node->leftTopSon     = (UGridNode*)0;
    node->leftBottomSon  = (UGridNode*)0;

    for (int i = 0; i < 4; i++)
    {
      int col = RIGHTCOL-1;
      int row = TOPROW-1;

      if ( i == 0 ) { col = col-1; row = row-1; } // left-bottom cell
      if ( i == 1 ) {              row = row-1; } // right-bottom cell
      if ( i == 2 ) { col = col-1;              } // left-top cell

      // Create and initialize UGridCell
      cells[col][row] = new UGridCell();
      UGridCell* cellPtr = cells[col][row];
      cellPtr->fatherNode = node;
      cellPtr->numEntries = 0;
      cellPtr->currentPage = (db_pgno_t)0;
      cellPtr->pos1 = UnitPos(0,0);
      cellPtr->pos2 = UnitPos(0,0);
      cellPtr->tiv.start = DateTime(0,0,instanttype);
      cellPtr->tiv.end = DateTime(0,0,instanttype);
      cellPtr->tiv.lc = false;
      cellPtr->tiv.rc = false;

      if ( i == 0 ) node->leftBottomCell  = cellPtr;
      if ( i == 1 ) node->rightBottomCell = cellPtr;
      if ( i == 2 ) node->leftTopCell     = cellPtr;
      if ( i == 3 ) node->rightTopCell    = cellPtr;
    }
  }
  return node;
}

/******************************************************************************

4.6 InsertUGridBox

Inserts a UGridBox into the UGridTree.

******************************************************************************/

void UGrid::InsertUGridBox( UGridNode* NODE, UGridBox* BOX )
{
  if ( (NODE->numEntries%60) == 0 )
  {
    if ( NODE->currentPage == 0 )
    {
      // Current page does not exists -> create new page
      SmiUpdatePage* currentPage;
      int AppendedPage = suf->AppendNewPage(currentPage);
      assert( AppendedPage );
      NODE->currentPage = currentPage->GetPageNo();

      // Initialize page
      db_pgno_t nextRootPageNo = 0;
      int PageSelected = suf->GetPage(NODE->currentPage, currentPage);
      assert( PageSelected );
      currentPage->Write(&nextRootPageNo, sizeof(db_pgno_t), 0);
    }
    else
    {
      if ((NODE->leftBottomCell != 0) &&
          (NODE->fatherNode != 0))
      {
        SmiUpdatePage* currentPage;
        int PageSelected = suf->GetPage(NODE->currentPage, currentPage);
        assert( PageSelected );

        UGridBox* box;
        // Current page is nearly full up
        // -> write the boxes of all four son cells into page
        size_t offset = sizeof(db_pgno_t) + (boxSize * NODE->numEntries);

        // Insert boxes from all four sons(cells)
        for (int i = 0; i < 4; i++)
        {
          // Current page of cell is not empty
          if (i == 0) box =CreateUGridBox(NODE->leftBottomCell, (UGridNode*)0);
          if (i == 1) box =CreateUGridBox(NODE->rightBottomCell,(UGridNode*)0);
          if (i == 2) box =CreateUGridBox(NODE->leftTopCell,    (UGridNode*)0);
          if (i == 3) box =CreateUGridBox(NODE->rightTopCell,   (UGridNode*)0);

          if (box != 0)
          {
            ModifyUGridNode( NODE, box );

            double minD0(box->box.MinD(0)); double maxD0(box->box.MaxD(0));
            double minD1(box->box.MinD(1)); double maxD1(box->box.MaxD(1));
            double minD2(box->box.MinD(2)); double maxD2(box->box.MaxD(2));

            currentPage->Write(&box->pageID, sizeof(db_pgno_t), offset);
            offset +=  sizeof(db_pgno_t);
            currentPage->Write(&box->huPageID, sizeof(bool), offset);
            offset +=  sizeof(bool);
            currentPage->Write(&minD0, sizeof(double), offset);
            offset +=  sizeof(double);
            currentPage->Write(&maxD0, sizeof(double), offset);
            offset +=  sizeof(double);
            currentPage->Write(&minD1, sizeof(double), offset);
            offset +=  sizeof(double);
            currentPage->Write(&maxD1, sizeof(double), offset);
            offset +=  sizeof(double);
            currentPage->Write(&minD2, sizeof(double), offset);
            offset +=  sizeof(double);
            currentPage->Write(&maxD2, sizeof(double), offset);
            offset +=  sizeof(double);
          }
        }
      }

      if (NODE->fatherNode != 0)
      {
        // Create page box
        UGridBox*  box = CreateUGridBox( (UGridCell*)0, NODE );
        if (box != 0)
        {
          // Reset number of node entries
          NODE->numEntries = 0;

          // Put current node page back to file
          //suf->PutPage(NODE->currentPage, true);

          // Insert box from son node into father node page
          InsertUGridBox( NODE->fatherNode, box );

          // Create new node page
          SmiUpdatePage* currentPage;
          int AppendedPage = suf->AppendNewPage(currentPage);
          assert( AppendedPage );
          NODE->currentPage = currentPage->GetPageNo();
        }
      }
      else
      {
        // Create next page for root node
        db_pgno_t prevPageNo = NODE->currentPage;
        SmiUpdatePage* prevPage;
        db_pgno_t nextRootPageNo;
        SmiUpdatePage* nextRootPage;

        do
        {
          int PageSelected = suf->GetPage(prevPageNo, prevPage);
          assert( PageSelected );
          prevPage->Read(&nextRootPageNo, sizeof(db_pgno_t), 0);
          prevPageNo = nextRootPageNo;
        }
        while ( nextRootPageNo != 0 );

        int AppendedPage = suf->AppendNewPage(nextRootPage);
        assert( AppendedPage );
        nextRootPageNo = nextRootPage->GetPageNo();
        int PageSelected = suf->GetPage(nextRootPageNo, nextRootPage);
        assert( PageSelected );
        prevPage->Write(&nextRootPageNo, sizeof(db_pgno_t), 0);
        nextRootPageNo = 0;
        nextRootPage->Write(&nextRootPageNo, sizeof(db_pgno_t), 0);
      }
    }
  }


  SmiUpdatePage* currentPage;
  int PageSelected = suf->GetPage(NODE->currentPage, currentPage);
  assert( PageSelected );

  if (NODE->fatherNode != 0)
  {
    size_t offset = sizeof(db_pgno_t) + (boxSize * NODE->numEntries);
    ModifyUGridNode( NODE, BOX );

    // Insert new UGridBox into node page
    double minD0(BOX->box.MinD(0)); double maxD0(BOX->box.MaxD(0));
    double minD1(BOX->box.MinD(1)); double maxD1(BOX->box.MaxD(1));
    double minD2(BOX->box.MinD(2)); double maxD2(BOX->box.MaxD(2));

    currentPage->Write(&BOX->pageID, sizeof(db_pgno_t), offset);
    offset +=  sizeof(db_pgno_t);
    currentPage->Write(&BOX->huPageID, sizeof(bool), offset);
    offset +=  sizeof(bool);
    currentPage->Write(&minD0, sizeof(double), offset);
    offset +=  sizeof(double);
    currentPage->Write(&maxD0, sizeof(double), offset);
    offset +=  sizeof(double);
    currentPage->Write(&minD1, sizeof(double), offset);
    offset +=  sizeof(double);
    currentPage->Write(&maxD1, sizeof(double), offset);
    offset +=  sizeof(double);
    currentPage->Write(&minD2, sizeof(double), offset);
    offset +=  sizeof(double);
    currentPage->Write(&maxD2, sizeof(double), offset);
    offset +=  sizeof(double);
  }
  else
  {
    // Insert new UGridBox into root page
    db_pgno_t      nextRootPageNo = NODE->currentPage;
    SmiUpdatePage* nextRootPage;
    do
    {
      int PageSelected = suf->GetPage(nextRootPageNo, nextRootPage);
      assert( PageSelected );
      nextRootPage->Read(&nextRootPageNo, sizeof(db_pgno_t), 0);
    }
    while ( nextRootPageNo != 0 );

    size_t offset = sizeof(db_pgno_t) + (boxSize * (NODE->numEntries%60));
    ModifyUGridNode( NODE, BOX );

    double minD0(BOX->box.MinD(0)); double maxD0(BOX->box.MaxD(0));
    double minD1(BOX->box.MinD(1)); double maxD1(BOX->box.MaxD(1));
    double minD2(BOX->box.MinD(2)); double maxD2(BOX->box.MaxD(2));

    nextRootPage->Write(&BOX->pageID, sizeof(db_pgno_t), offset);
    offset +=  sizeof(db_pgno_t);
    nextRootPage->Write(&BOX->huPageID, sizeof(bool), offset);
    offset +=  sizeof(bool);
    nextRootPage->Write(&minD0, sizeof(double), offset);
    offset +=  sizeof(double);
    nextRootPage->Write(&maxD0, sizeof(double), offset);
    offset +=  sizeof(double);
    nextRootPage->Write(&minD1, sizeof(double), offset);
    offset +=  sizeof(double);
    nextRootPage->Write(&maxD1, sizeof(double), offset);
    offset +=  sizeof(double);
    nextRootPage->Write(&minD2, sizeof(double), offset);
    offset +=  sizeof(double);
    nextRootPage->Write(&maxD2, sizeof(double), offset);
    offset +=  sizeof(double);
  }
}

/******************************************************************************

4.7 DestructUGridTree

Destructs the UGridTree.

******************************************************************************/

void UGrid::DestructUGridTree( UGridNode* NODE, int SQUARES )
{
  if (SQUARES > 4)
  {
    DestructUGridTree( NODE->leftBottomSon, (SQUARES/4));
    DestructUGridTree( NODE->rightBottomSon, (SQUARES/4));
    DestructUGridTree( NODE->leftTopSon, (SQUARES/4));
    DestructUGridTree( NODE->rightTopSon, (SQUARES/4));
  }
  else
  {
    delete NODE->leftBottomCell;
    delete NODE->rightBottomCell;
    delete NODE->leftTopCell;
    delete NODE->rightTopCell;
  }
  delete NODE;
}

/******************************************************************************

4.8 ReadHeader-method

Reads UGrid-header information from file.

******************************************************************************/

void UGrid::ReadHeader()
{
  SmiUpdatePage* headerPage;
  int PageSelected = suf->GetPage(header->headerPageNo, headerPage);
  assert( PageSelected );

  size_t offset = 0;
  headerPage->Read(&header->flPageNo, sizeof(db_pgno_t), offset);
  offset += sizeof(db_pgno_t);
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      headerPage->Read(&header->cellPageNo[j][i], sizeof(db_pgno_t), offset);
      offset += sizeof(db_pgno_t);
    }
  }
  headerPage->Read(&header->nodePageNo, sizeof(db_pgno_t), offset);
  offset += sizeof(db_pgno_t);
  headerPage->Read(&header->rootNode, sizeof(UGridNode*), offset);
  offset += sizeof(UGridNode*);
  headerPage->Read(&header->area, sizeof(UGridArea), offset);
  offset += sizeof(UGridArea);
  headerPage->Read(&header->splits, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Read(&header->numCells, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Read(&header->numEntries, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Read(&header->numFlEntries, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Read(&header->tiv, sizeof(Interval<Instant>), offset);
}

/******************************************************************************

4.9 ReadFLine-method

Reads UGrid-front-line information from file.

******************************************************************************/

void UGrid::ReadFLine()
{
  SmiUpdatePage* flPage;
  int PageSelected = suf->GetPage(header->flPageNo, flPage);
  assert( PageSelected );
  size_t flPageOffset = 0;

  for (int i = 0; i < flBuckets; i++)
  {
    SmiUpdatePage* bucketPage;
    db_pgno_t      bucketPageNo;
    db_pgno_t      nextPageNo;
    int            numEntries;

    // Select bucket page
    flPage->Read( &bucketPageNo, sizeof(db_pgno_t),flPageOffset);
    flPageOffset += sizeof(db_pgno_t);
    do
    {
      PageSelected = suf->GetPage(bucketPageNo, bucketPage);
      assert( PageSelected );
      bucketPage->Read(&nextPageNo, sizeof(db_pgno_t), 0);
      bucketPage->Read(&numEntries, sizeof(int), sizeof(db_pgno_t));
      size_t offset = sizeof(db_pgno_t) + sizeof(int);
      for (int j = 0; j < numEntries; j++)
      {
        UploadUnit  unit;
        bucketPage->Read(&unit, sizeof(UploadUnit), offset);
        offset += sizeof(UploadUnit);
        frontline[unit.GetID()%flBuckets].insert(make_pair(unit.GetID(),unit));
      }
      if ( nextPageNo != 0 ) bucketPageNo = nextPageNo;
    }
    while(nextPageNo != 0);
  }
}

/******************************************************************************

4.10 ReadUGridTree

Reads UGridTree information from file.

******************************************************************************/

UGridNode* UGrid::ReadUGridTree( UGridNode* FATHERNODE, UGridArea AREA,
                                 int SQUARES, int RIGHTCOL, int TOPROW )
{
  if((header->currentNodeRecord%64) == 0)
  {
    if (header->selectedNodePage == (SmiUpdatePage*)0)
    {
      // Select first node page
      int PageSelected =
          suf->GetPage(header->nodePageNo,header->selectedNodePage);
      assert( PageSelected );
    }
    else
    {
      // Select next node page
      db_pgno_t nextPageNo;
      header->selectedNodePage->Read(&nextPageNo,sizeof(db_pgno_t),0);
      int PageSelected = suf->GetPage(nextPageNo,header->selectedNodePage);
      assert( PageSelected );
    }
    header->currentNodeRecord = 0;
  }

  // Read node data from page
  double tivStart, tivEnd;
  bool   tivLc, tivRc;
  size_t offset = sizeof(db_pgno_t)+(header->currentNodeRecord*nodeSize);
  header->currentNodeRecord++;

  UGridNode* node  = new UGridNode();
  node->fatherNode = FATHERNODE;

  header->selectedNodePage->Read(&node->numEntries, sizeof(int), offset);
  offset +=  sizeof(int);
  header->selectedNodePage->Read(&node->currentPage,sizeof(db_pgno_t),offset);
  offset +=  sizeof(db_pgno_t);
  header->selectedNodePage->Read(&node->pos1, sizeof(UnitPos), offset);
  offset +=  sizeof(UnitPos);
  header->selectedNodePage->Read(&node->pos2, sizeof(UnitPos), offset);
  offset +=  sizeof(UnitPos);
  header->selectedNodePage->Read(&tivStart, sizeof(double), offset);
  offset +=  sizeof(double);
  header->selectedNodePage->Read(&tivEnd, sizeof(double), offset);
  offset +=  sizeof(double);
  header->selectedNodePage->Read(&tivLc, sizeof(bool), offset);
  offset +=  sizeof(bool);
  header->selectedNodePage->Read(&tivRc, sizeof(bool), offset);
  offset +=  sizeof(bool);
  node->tiv.start  = DateTime(0,0,instanttype);
  node->tiv.end    = DateTime(0,0,instanttype);
  node->tiv.lc     = tivLc;
  node->tiv.rc     = tivRc;
  node->tiv.start.ReadFrom(tivStart);
  node->tiv.end.ReadFrom(tivEnd);

  if (SQUARES > 4)
  {
    node->leftBottomSon = ReadUGridTree(  node, AREA, (SQUARES/4),
                                             (RIGHTCOL-((int)sqrt(SQUARES)/2)),
                                             (TOPROW-((int)sqrt(SQUARES)/2)));

    node->rightBottomSon = ReadUGridTree( node, AREA, (SQUARES/4),
                                              RIGHTCOL,
                                              (TOPROW-((int)sqrt(SQUARES)/2)));

    node->leftTopSon = ReadUGridTree( node, AREA, (SQUARES/4),
                                            (RIGHTCOL-((int)sqrt(SQUARES)/2)),
                                             TOPROW );

    node->rightTopSon = ReadUGridTree(  node, AREA, (SQUARES/4),
                                        RIGHTCOL, TOPROW );

    node->leftBottomCell  = (UGridCell*)0;
    node->rightBottomCell = (UGridCell*)0;
    node->leftTopCell     = (UGridCell*)0;
    node->rightTopCell    = (UGridCell*)0;
  }
  else
  {
    node->leftBottomSon  = (UGridNode*)0;
    node->rightBottomSon = (UGridNode*)0;
    node->leftTopSon     = (UGridNode*)0;
    node->rightTopSon    = (UGridNode*)0;

    // Calculate x/y cell length
    double areaLenX = abs(AREA.x2 - AREA.x1);
    double areaLenY = abs(AREA.y2 - AREA.y1);
    double cellLenX = areaLenX / header->splits;
    double cellLenY = areaLenY / header->splits;

    for (int i = 0; i < 4; i++)
    {
      int col = RIGHTCOL-1;
      int row = TOPROW-1;

      if ( i == 0 ) { col = col-1; row = row-1; } // leftBottomCell
      if ( i == 1 ) {              row = row-1; } // rightBottomCell
      if ( i == 2 ) { col = col-1;              } // leftTopCell

      // Create new cell object
      cells[col][row] = new UGridCell();
      UGridCell* cellPtr = cells[col][row];

      // Assign node pointer
      cellPtr->fatherNode = node;
      if ( i == 0 ) node->leftBottomCell  = cells[col][row];
      if ( i == 1 ) node->rightBottomCell = cells[col][row];
      if ( i == 2 ) node->leftTopCell     = cells[col][row];
      if ( i == 3 ) node->rightTopCell    = cells[col][row];

      // Create cell area
      cellPtr->area = UGridArea(0,0,0,0);
      cellPtr->area.x1  = AREA.x1 + (cellLenX * col);
      cellPtr->area.x2  = cellPtr->area.x1 + cellLenX;
      cellPtr->area.y1  = AREA.y1 + (cellLenY * row);
      cellPtr->area.y2  = cellPtr->area.y1 + cellLenY;

      // Read cell data from cell page
      db_pgno_t cellPageNo = header->cellPageNo[(col/8)][(row/8)];
      SmiUpdatePage* cellPage;
      int PageSelected = suf->GetPage(cellPageNo, cellPage);
      assert( PageSelected );

      offset =  ((col%8)+((row%8)*8))*cellSize;
      cellPage->Read(&cellPtr->numEntries, sizeof(int), offset);
      offset +=  sizeof(int);
      cellPage->Read(&cellPtr->currentPage, sizeof(db_pgno_t), offset);
      offset +=  sizeof(db_pgno_t);
      cellPage->Read(&cellPtr->pos1, sizeof(UnitPos), offset);
      offset +=  sizeof(UnitPos);
      cellPage->Read(&cellPtr->pos2, sizeof(UnitPos), offset);
      offset +=  sizeof(UnitPos);
      cellPage->Read(&tivStart, sizeof(double), offset);
      offset +=  sizeof(double);
      cellPage->Read(&tivEnd, sizeof(double), offset);
      offset +=  sizeof(double);
      cellPage->Read(&tivLc, sizeof(bool), offset);
      offset +=  sizeof(bool);
      cellPage->Read(&tivRc, sizeof(bool), offset);

      cellPtr->tiv.start  = DateTime(0,0,instanttype);
      cellPtr->tiv.end    = DateTime(0,0,instanttype);
      cellPtr->tiv.lc     = tivLc;
      cellPtr->tiv.rc     = tivRc;
      cellPtr->tiv.start.ReadFrom(tivStart);
      cellPtr->tiv.end.ReadFrom(tivEnd);
    }
  }

  if (SQUARES == header->numCells)
  {
    header->currentNodeRecord = 0;
    header->selectedNodePage  = (SmiUpdatePage*)0;
  }
  return node;
}

/******************************************************************************

4.11 UpdateHeader-method

Writes UGrid-header information into file.

******************************************************************************/

void UGrid::UpdateHeader()
{
  SmiUpdatePage* headerPage;
  int PageSelected = suf->GetPage(header->headerPageNo, headerPage);
  assert( PageSelected );

  size_t offset = 0;
  headerPage->Write(&header->flPageNo, sizeof(db_pgno_t), offset);
  offset += sizeof(db_pgno_t);
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      headerPage->Write(&header->cellPageNo[j][i], sizeof(db_pgno_t),offset);
      offset += sizeof(db_pgno_t);
    }
  }
  headerPage->Write(&header->nodePageNo, sizeof(db_pgno_t), offset);
  offset += sizeof(db_pgno_t);
  headerPage->Write(&header->rootNode, sizeof(UGridNode*), offset);
  offset += sizeof(UGridNode*);
  headerPage->Write(&header->area, sizeof(UGridArea), offset);
  offset += sizeof(UGridArea);
  headerPage->Write(&header->splits, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Write(&header->numCells, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Write(&header->numEntries, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Write(&header->numFlEntries, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Write(&header->tiv, sizeof(Interval<Instant>), offset);
}

/******************************************************************************

4.12 UpdateFLine-method

Writes UGrid-front-line information into file.

******************************************************************************/

void UGrid::UpdateFLine()
{
  SmiUpdatePage* flPage;
  int PageSelected = suf->GetPage(header->flPageNo, flPage);
  assert( PageSelected );
  size_t flPageOffset = 0;

  for (int i = 0; i < flBuckets; i++)
  {
    SmiUpdatePage* bucketPage;
    db_pgno_t      bucketPageNo;
    db_pgno_t      nextPageNo;

    // Select bucket page
    flPage->Read( &bucketPageNo, sizeof(db_pgno_t), flPageOffset);
    flPageOffset += sizeof(db_pgno_t);
    PageSelected = suf->GetPage(bucketPageNo, bucketPage);
    assert( PageSelected );

    int numEntries = 0;
    size_t bucketPageOffset = sizeof(db_pgno_t) + sizeof(int);
    for (map<int,UploadUnit>::iterator it = frontline[i].begin();
         it != frontline[i].end(); it++)
    {
      numEntries++;
      bucketPage->Write( &numEntries, sizeof(int), sizeof(db_pgno_t) );
      bucketPage->Write( &it->second, sizeof(UploadUnit), bucketPageOffset );
      bucketPageOffset += sizeof(UploadUnit);
      if((numEntries%70) == 0)
      {
        // Page is full up
        bucketPageOffset = sizeof(db_pgno_t) + sizeof(int);
        // Get next page
        bucketPage->Read(&nextPageNo, sizeof(db_pgno_t), 0);
        if(nextPageNo == 0)
        {
          // No next page exist -> create new next page
          SmiUpdatePage* newPage;
          int AppendedPage = suf->AppendNewPage(newPage);
          assert( AppendedPage );
          nextPageNo = newPage->GetPageNo();
          bucketPage->Write(&nextPageNo, sizeof(db_pgno_t), 0);
        }
        // Select and initialize next page
        PageSelected = suf->GetPage(nextPageNo, bucketPage);
        assert( PageSelected );
        nextPageNo = 0;
        numEntries = 0;
        bucketPage->Write(&nextPageNo, sizeof(db_pgno_t), 0);
        bucketPage->Write(&numEntries, sizeof(int), sizeof(db_pgno_t));
      }
    }
  }
}

/******************************************************************************

4.13 UpdateUGridTree

Writes UGridTree information into file.

******************************************************************************/

void UGrid::UpdateUGridTree( UGridNode* NODE, int SQUARES,
                             int RIGHTCOL, int TOPROW )
{
  if((header->currentNodeRecord%64) == 0)
  {
    if (header->selectedNodePage == (SmiUpdatePage*)0)
    {
      // Select first node page
      int PageSelected = suf->GetPage(header->nodePageNo,
                                      header->selectedNodePage);
      assert( PageSelected );
    }
    else
    {
      // Select next node page
      db_pgno_t nextPageNo;
      header->selectedNodePage->Read(&nextPageNo,sizeof(db_pgno_t),0);
      if(nextPageNo == 0)
      {
        // No next page exist -> create new next page
        SmiUpdatePage* newPage;
        int AppendedPage = suf->AppendNewPage(newPage);
        assert( AppendedPage );
        nextPageNo = newPage->GetPageNo();
        // Write next page number into previous page
        header->selectedNodePage->Write(&nextPageNo, sizeof(db_pgno_t), 0);
      }
      int PageSelected = suf->GetPage(nextPageNo,header->selectedNodePage);
      assert( PageSelected );
      // Initialize next page number
      nextPageNo = 0;
      header->selectedNodePage->Write(&nextPageNo, sizeof(db_pgno_t), 0);
    }
    header->currentNodeRecord = 0;
  }

  // Write node data into page
  size_t offset = sizeof(db_pgno_t) + (header->currentNodeRecord*nodeSize);
  header->currentNodeRecord++;

  double tivStart = NODE->tiv.start.ToDouble();
  double tivEnd   = NODE->tiv.end.ToDouble();

  header->selectedNodePage->Write(&NODE->numEntries, sizeof(int), offset);
  offset +=  sizeof(int);
  header->selectedNodePage->Write(&NODE->currentPage,sizeof(db_pgno_t),offset);
  offset +=  sizeof(db_pgno_t);
  header->selectedNodePage->Write(&NODE->pos1, sizeof(UnitPos), offset);
  offset +=  sizeof(UnitPos);
  header->selectedNodePage->Write(&NODE->pos2, sizeof(UnitPos), offset);
  offset +=  sizeof(UnitPos);
  header->selectedNodePage->Write(&tivStart, sizeof(double), offset);
  offset +=  sizeof(double);
  header->selectedNodePage->Write(&tivEnd, sizeof(double), offset);
  offset +=  sizeof(double);
  header->selectedNodePage->Write(&NODE->tiv.lc, sizeof(bool), offset);
  offset +=  sizeof(bool);
  header->selectedNodePage->Write(&NODE->tiv.rc, sizeof(bool), offset);
  offset +=  sizeof(bool);

  if (SQUARES > 4)
  {
    UpdateUGridTree( NODE->leftBottomSon, (SQUARES/4),
           (RIGHTCOL-((int)sqrt(SQUARES)/2)), (TOPROW-((int)sqrt(SQUARES)/2)));

    UpdateUGridTree( NODE->rightBottomSon, (SQUARES/4),
                     RIGHTCOL, (TOPROW-((int)sqrt(SQUARES)/2)));

    UpdateUGridTree( NODE->leftTopSon, (SQUARES/4),
                     (RIGHTCOL-((int)sqrt(SQUARES)/2)), TOPROW );

    UpdateUGridTree(  NODE->rightTopSon, (SQUARES/4), RIGHTCOL, TOPROW );
  }
  else
  {
    for (int i = 0; i < 4; i++)
    {
      int col = RIGHTCOL-1;
      int row = TOPROW-1;

      if ( i == 0 ) { col = col-1; row = row-1; } // leftBottomCell
      if ( i == 1 ) {              row = row-1; } // rightBottomCell
      if ( i == 2 ) { col = col-1;              } // leftTopCell

      // Current cell pointer
      UGridCell* cellPtr = cells[col][row];

      // Select cell page
      db_pgno_t cellPageNo = header->cellPageNo[(col/8)][(row/8)];
      SmiUpdatePage* cellPage;
      int PageSelected = suf->GetPage(cellPageNo, cellPage);
      assert( PageSelected );

      tivStart = cellPtr->tiv.start.ToDouble();
      tivEnd   = cellPtr->tiv.end.ToDouble();

      // Write cell data into cell page
      offset =  ((col%8)+((row%8)*8))*cellSize;
      cellPage->Write(&cellPtr->numEntries, sizeof(int), offset);
      offset +=  sizeof(int);
      cellPage->Write(&cellPtr->currentPage, sizeof(db_pgno_t), offset);
      offset +=  sizeof(db_pgno_t);
      cellPage->Write(&cellPtr->pos1, sizeof(UnitPos), offset);
      offset +=  sizeof(UnitPos);
      cellPage->Write(&cellPtr->pos2, sizeof(UnitPos), offset);
      offset +=  sizeof(UnitPos);
      cellPage->Write(&tivStart, sizeof(double), offset);
      offset +=  sizeof(double);
      cellPage->Write(&tivEnd, sizeof(double), offset);
      offset +=  sizeof(double);
      cellPage->Write(&cellPtr->tiv.lc, sizeof(bool), offset);
      offset +=  sizeof(bool);
      cellPage->Write(&cellPtr->tiv.rc, sizeof(bool), offset);
      offset +=  sizeof(bool);
    }
  }

  if (SQUARES == header->numCells)
  {
    header->currentNodeRecord = 0;
    header->selectedNodePage  = (SmiUpdatePage*)0;
  }
}

/******************************************************************************

4.14 UpdateUGrid-method

Writes all UGrid data into file.

******************************************************************************/

bool UGrid::UpdateUGrid()
{
  // Check if SmiUpdateFile is released
  if(GetSemaphore())
  {
    return false;
  }
  else
  {
    // Lock SmiUpdateFile
    SetSemaphore(true);

    // Update header in SmiUpdateFile
    UpdateHeader();

    // Update front line in SmiUpdateFile
    UpdateFLine();

    // Update UGrid-tree
    UpdateUGridTree( header->rootNode, header->numCells,
                     header->splits, header->splits );

    // Release SmiUpdateFile
    SetSemaphore(false);

    return true;
  }
}

/******************************************************************************

4.15 insertPageID-method

Inserts HistoryUnit page id's into pages set.

******************************************************************************/

void UGrid::insertPageID(db_pgno_t NODEPAGEID, bool HUPAGEID)
{
  SmiUpdatePage* selectedNodePage;
  int PageSelected = suf->GetPage(NODEPAGEID,selectedNodePage);
  assert( PageSelected );

  db_pgno_t  pageID;
  bool       huPageID;
  size_t offset = sizeof(db_pgno_t);

  for (int i = 0; i < 60; i++)
  {
      selectedNodePage->Read(&pageID, sizeof(db_pgno_t), offset);
      offset += sizeof(db_pgno_t);
      selectedNodePage->Read(&huPageID, sizeof(bool), offset);
      offset += (sizeof(bool) + (6*sizeof(double)));

      if (huPageID && pageID != 0) pages.insert(pageID);
      if (!huPageID) insertPageID(pageID, huPageID);
  }
}

/******************************************************************************

4.16 FindHistoryPages-method

Finds HistoryUnit pages in the UGridTree.

******************************************************************************/

void UGrid::FindHistoryPages(UGridArea AREA, Instant TIME1, Instant TIME2,
                             UGridNode* NODE, int SQUARES)
{
   if ((NODE->numEntries > 0) &&
        CheckUGridNode(AREA,TIME1,TIME2,NODE))
   {
    // Find UGridBoxes in node page
    SmiUpdatePage* selectedNodePage;
    int PageSelected = suf->GetPage(NODE->currentPage,selectedNodePage);
    assert( PageSelected );

    size_t offset = sizeof(db_pgno_t);

    for (int i = 0; i < NODE->numEntries; i++)
    {
      if ((NODE->fatherNode == 0) && ((i%60) == 0) && (i > 0))
      {
        // For the root node read the following pages
        db_pgno_t nextPageNo;
        selectedNodePage->Read(&nextPageNo, sizeof(db_pgno_t), 0);
        if (nextPageNo != 0)
        {
          PageSelected = suf->GetPage(nextPageNo, selectedNodePage);
          assert( PageSelected );
          offset = sizeof(db_pgno_t);
        }
      }

      // Read UGridBox
      db_pgno_t  pageID;
      bool huPageID;
      double minD0, maxD0, minD1, maxD1, minD2, maxD2;
      selectedNodePage->Read(&pageID, sizeof(db_pgno_t), offset);
      offset +=  sizeof(db_pgno_t);
      selectedNodePage->Read(&huPageID, sizeof(bool), offset);
      offset +=  sizeof(bool);
      selectedNodePage->Read(&minD0, sizeof(double), offset);
      offset +=  sizeof(double);
      selectedNodePage->Read(&maxD0, sizeof(double), offset);
      offset +=  sizeof(double);
      selectedNodePage->Read(&minD1, sizeof(double), offset);
      offset +=  sizeof(double);
      selectedNodePage->Read(&maxD1, sizeof(double), offset);
      offset +=  sizeof(double);
      selectedNodePage->Read(&minD2, sizeof(double), offset);
      offset +=  sizeof(double);
      selectedNodePage->Read(&maxD2, sizeof(double), offset);
      offset +=  sizeof(double);

      Rectangle<3>* rect = new Rectangle<3>(true, minD0-tol, maxD0+tol,
                                                  minD1-tol, maxD1+tol,
                                                  minD2-tol, maxD2+tol);

      UGridBox* box = new UGridBox(pageID, huPageID, *rect);

      if (CheckUGridBox(AREA,TIME1,TIME2,*box))
      {
        // UGridBox is in searchWindow
        if (huPageID) pages.insert(pageID);
        else insertPageID(pageID, false);
      }
      delete box;
      delete rect;
    }
  }

  if (SQUARES > 4)
  {
    FindHistoryPages(AREA,TIME1,TIME2,NODE->leftBottomSon,(SQUARES/4));
    FindHistoryPages(AREA,TIME1,TIME2,NODE->rightBottomSon,(SQUARES/4));
    FindHistoryPages(AREA,TIME1,TIME2,NODE->leftTopSon,(SQUARES/4));
    FindHistoryPages(AREA,TIME1,TIME2,NODE->rightTopSon,(SQUARES/4));
  }
}

/******************************************************************************

4.17 GetUpdateFile-method

Returns the UGrid file id.

******************************************************************************/

SmiUpdateFile* UGrid::GetUpdateFile()
{
 return suf;
}

/******************************************************************************

4.18 GetHeader-method

Returns the pointer to the UGrid header.

******************************************************************************/

UGridHeader* UGrid::GetHeader()
{
 return header;
}

/******************************************************************************

4.19 GetCell-method

Returns the pointer to the stated cell.

******************************************************************************/

UGridCell* UGrid::GetCell(int COL, int ROW)
{
 return cells[COL][ROW];
}

/******************************************************************************

4.20 GetSemaphore-method

Returns the state of the semaphore.

******************************************************************************/

bool UGrid::GetSemaphore()
{
  bool value;
  SmiUpdatePage* headerPage;
  int PageSelected = suf->GetPage(header->headerPageNo, headerPage);
  assert( PageSelected );
  headerPage->Read(&value, sizeof(bool), sizeof(UGridHeader));

 // Wait until file is released or timeout is 0
 int timeout = 100000;
  while(value && (timeout >= 0))
  {
    headerPage->Read(&value, sizeof(bool), sizeof(UGridHeader));
    for(int i = 0; i < 10000; i++);
    timeout--;
  }
  return value;
}

/******************************************************************************

4.21 SetSemaphore-method

Sets the state of the semaphore.

******************************************************************************/

void UGrid::SetSemaphore(bool value)
{
  SmiUpdatePage* headerPage;
  int PageSelected = suf->GetPage(header->headerPageNo, headerPage);
  assert( PageSelected );
  headerPage->Write(&value, sizeof(bool), sizeof(UGridHeader));
}

/******************************************************************************

4.22 Type constructor

******************************************************************************/

TypeConstructor UGridTC(
       UGrid::BasicType(),                        // name
        UGrid::Property,               // property function
        UGrid::Out,   UGrid::In,       // Out and In functions
        0, 0,                          // SaveTo and RestoreFrom functions
        UGrid::Create,  UGrid::Delete, // object creation and deletion
        UGrid::Open,    UGrid::Save,   // object open and save
        UGrid::Close,   UGrid::Clone,  // object close and clone
        UGrid::Cast,                   // cast function
        UGrid::SizeOfObj,              // sizeof function
        UGrid::KindCheck );            // kind checking function

/******************************************************************************

5 createUGrid operator

Creates a new UGrid object.

5.1 createUGrid - Type mapping method

******************************************************************************/

ListExpr CreateTM(ListExpr args)
{
  if( nl->ListLength( args ) == 2 &&
      nl->IsEqual(nl->First(args),Rectangle<2>::BasicType()) &&
      nl->IsEqual(nl->Second(args),CcInt::BasicType()) )
  {
    return (nl->SymbolAtom(UGrid::BasicType()));
  }
  ErrorReporter::ReportError("rect x int expected!");
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

/******************************************************************************

5.2 createUGrid - Value mapping method

******************************************************************************/

int CreateVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  UGrid* ugridPtr = static_cast<UGrid*>(qp->ResultStorage(s).addr);

  Rectangle<2>* rect = static_cast<Rectangle<2>*>(args[0].addr);
  CcInt*  partitions = static_cast<CcInt*>(args[1].addr);

  // Create UGrid area
  double x1(rect->MinD(0));
  double x2(rect->MaxD(0));
  double y1(rect->MinD(1));
  double y2(rect->MaxD(1));
  UGridArea area(x1,y1,x2,y2);
  area = ModifyArea(area);

  // Check number of partitions
  int numPartitions = partitions->GetValue();
  if (!(numPartitions == 4096 ||
        numPartitions ==  256 ||
        numPartitions ==   16 ||
        numPartitions ==    4 ))
  {
   // Wrong number of partitions -> create default UGrid
   ugridPtr = new UGrid(area,4);
  }
  else
  {
    // Create UGrid with the stated number of partitions
    ugridPtr = new UGrid(area,(int)sqrt(numPartitions));
  }
  result.setAddr( ugridPtr );
  return 0;
}

/******************************************************************************

5.3 createUGrid - Specification of operator

******************************************************************************/

struct CreateInfo : OperatorInfo {
  CreateInfo()
  {
    name      = "createUGrid";
    signature = "rect x int -> ugrid";
    syntax    = "createUGrid( _, _ )";
    meaning   = "UGrid construction operator. The second argument must be 4, "
                "16, 256 or 4096, otherwise it will be set a default value.";
  }
};

/******************************************************************************

6 insertUpload operator

Inserts a single upload into UGrid.

6.1 ComputeHistoryUnit-method

Computes a history unit and returns it to the insertHandle method.

******************************************************************************/

HistoryUnit* ComputeHistoryUnit( UGridArea AREA, int SPLITS, UnitPos POS1,
                                 UnitPos POS2, Interval<Instant> TIV )
{
  int    startCol  = ComputeLine(AREA.x1, AREA.x2, SPLITS, POS1.x);
  int    startRow  = ComputeLine(AREA.y1, AREA.y2, SPLITS, POS1.y);
  double xLen      = abs(AREA.x1-AREA.x2) / SPLITS;
  double yLen      = abs(AREA.y1-AREA.y2) / SPLITS;
  double right     = (startCol+1)*xLen + AREA.x1;
  double left      = (startCol)*xLen + AREA.x1;
  double top       = (startRow+1)*yLen + AREA.y1;
  double bottom    = (startRow)*yLen + AREA.y1;

  // Compute intersection points
  UnitPos* isR = new UnitPos(0,0);
  UnitPos* isL = new UnitPos(0,0);
  UnitPos* isT = new UnitPos(0,0);
  UnitPos* isB = new UnitPos(0,0);

  bool intersectsRight = ComputeIntersection(right, bottom, right, top,
                                POS1.x, POS1.y, POS2.x, POS2.y, isR);
  bool intersectsLeft = ComputeIntersection(left, bottom, left,top,
                                POS1.x, POS1.y, POS2.x, POS2.y, isL);
  bool intersectsTop = ComputeIntersection(left, top, right, top,
                                POS1.x, POS1.y, POS2.x, POS2.y, isT);
  bool intersectsBottom = ComputeIntersection(left, bottom, right, bottom,
                                POS1.x, POS1.y, POS2.x, POS2.y, isB);

  // Create new end point for history unit
  UnitPos p(POS2.x,POS2.y);
  for (int i = 0; i < 4; i++) intersects[i] = false;
  if (intersectsRight)  {p.x = isR->x; p.y = isR->y; intersects[0] = true;}
  if (intersectsLeft)   {p.x = isL->x; p.y = isL->y; intersects[1] = true;}
  if (intersectsTop)    {p.x = isT->x; p.y = isT->y; intersects[2] = true;}
  if (intersectsBottom) {p.x = isB->x; p.y = isB->y; intersects[3] = true;}
  delete isR;
  delete isL;
  delete isT;
  delete isB;

  HistoryUnit* hu = new HistoryUnit(0, 0, TIV.start.ToDouble(),
                                    TIV.end.ToDouble(), POS1, p);
  return hu;
}

/******************************************************************************

6.2 InsertHandle-method

Inserts an upload into the UGrid index structure. The insertUpload and
the insertStream operator make use of this method.

******************************************************************************/

int InsertHandle(UGrid* UGRIDPTR, UploadUnit* UNITPTR)
{
  SmiUpdateFile* suf  = UGRIDPTR->GetUpdateFile();
  UGridHeader*    hPtr = UGRIDPTR->GetHeader();

  // Ceck UploadUnit
  if (hPtr->area.x1 < hPtr->area.x2)
  {
    if (UNITPTR->GetPos().x < hPtr->area.x1+tol ||
        UNITPTR->GetPos().x > hPtr->area.x2-tol ) return 2;
  }
  else if (UNITPTR->GetPos().x < hPtr->area.x2+tol ||
           UNITPTR->GetPos().x > hPtr->area.x1-tol ) return 2;

  if (hPtr->area.y1 < hPtr->area.y2)
  {
    if (UNITPTR->GetPos().y < hPtr->area.y1+tol ||
        UNITPTR->GetPos().y > hPtr->area.y2-tol ) return 2;
  }
  else if (UNITPTR->GetPos().y < hPtr->area.y2+tol ||
           UNITPTR->GetPos().y > hPtr->area.y1-tol ) return 2;

  // Find entry in the frontline
  int moID  = UNITPTR->GetID();
  map<int,UploadUnit>::iterator it;
  it = UGRIDPTR->frontline[moID%flBuckets].find(moID);

  if ( it == UGRIDPTR->frontline[moID%flBuckets].end() )
  {
    // No entry found -> Place UploadUnit in front line
    UGRIDPTR->frontline[moID%flBuckets].insert(make_pair(moID,*UNITPTR));
    hPtr->numFlEntries++;
  }
  else
  {
    // Entry found
    if(!suf->IsOpen()) suf->Open(hPtr->fileID, pageSize);
    HistoryUnit* hu;
    UnitPos pos1 = it->second.GetPos();
    UnitPos pos2 = UNITPTR->GetPos();
    Interval<Instant> huTiv;
    huTiv.start = it->second.GetTime();
    huTiv.end   = UNITPTR->GetTime();
    huTiv.lc    = true;
    huTiv.rc    = true;

    // Check if the current upload is younger than the last upload
    if(huTiv.start >= huTiv.end) return 3;

    // Replace UploadUnit in frontline
    UGRIDPTR->frontline[moID%flBuckets][moID] = *UNITPTR;

    do
    {
      // Find cell
      int col =ComputeLine(hPtr->area.x1, hPtr->area.x2, hPtr->splits, pos1.x);
      int row =ComputeLine(hPtr->area.y1, hPtr->area.y2, hPtr->splits, pos1.y);
      UGridCell* cellPtr = UGRIDPTR->GetCell(col,row);

      SmiUpdatePage* page;
      if ((cellPtr->numEntries % maxHistoryUnits) == 0)
      {
          if(cellPtr->currentPage != 0)
          {
            // Insert UGridBox into UGrid-tree
          UGridBox* box = CreateUGridBox( cellPtr, (UGridNode*)0 );
          if (box != 0) UGRIDPTR->InsertUGridBox( cellPtr->fatherNode, box );

          // Put current page back to disk
          //suf->PutPage(cellPtr->currentPage, true);
        }
        // Create new page if no page exist or current page is full up
        int AppendedPage = suf->AppendNewPage(page);
        assert( AppendedPage );
        cellPtr->currentPage = page->GetPageNo();
      }
      int PageSelected = suf->GetPage(cellPtr->currentPage, page);
      assert( PageSelected );

      // Create/split new history unit
      hu = ComputeHistoryUnit( hPtr->area, hPtr->splits, pos1, pos2, huTiv );
      hu->moID  = moID;
      hu->huID  = hPtr->numEntries;

      // Check/modify pos1 and pos2 of cell
      if ((!cellPtr->tiv.lc) || (cellPtr->pos1.x > hu->pos1.x))
         cellPtr->pos1.x = hu->pos1.x;
      if ((!cellPtr->tiv.lc) || (cellPtr->pos1.x > hu->pos2.x))
         cellPtr->pos1.x = hu->pos2.x;
      if ((!cellPtr->tiv.lc) || (cellPtr->pos1.y > hu->pos1.y))
         cellPtr->pos1.y = hu->pos1.y;
      if ((!cellPtr->tiv.lc) || (cellPtr->pos1.y > hu->pos2.y))
         cellPtr->pos1.y = hu->pos2.y;
      if ((!cellPtr->tiv.rc) || (cellPtr->pos2.x < hu->pos1.x))
         cellPtr->pos2.x = hu->pos1.x;
      if ((!cellPtr->tiv.rc) || (cellPtr->pos2.x < hu->pos2.x))
         cellPtr->pos2.x = hu->pos2.x;
      if ((!cellPtr->tiv.rc) || (cellPtr->pos2.y < hu->pos1.y))
         cellPtr->pos2.y = hu->pos1.y;
      if ((!cellPtr->tiv.rc) || (cellPtr->pos2.y < hu->pos2.y))
         cellPtr->pos2.y = hu->pos2.y;

      // Check/modify UGrid time interval
      if ((!hPtr->tiv.lc) || (hPtr->tiv.start > huTiv.start))
      {
        hPtr->tiv.start = huTiv.start;
        hPtr->tiv.lc = true;
      }

      if ((!hPtr->tiv.rc) || (hPtr->tiv.end < huTiv.end))
      {
        hPtr->tiv.end = huTiv.end;
        hPtr->tiv.rc = true;
      }

      // Check/modify cell time interval
      if ((!cellPtr->tiv.lc) || (cellPtr->tiv.start > huTiv.start))
      {
        cellPtr->tiv.start = huTiv.start;
        cellPtr->tiv.lc = true;
      }

      if ((!cellPtr->tiv.rc) || (cellPtr->tiv.end < huTiv.end))
      {
        cellPtr->tiv.end = huTiv.end;
        cellPtr->tiv.rc = true;
      }

      // Initialize tiv and number of entries in page
      if ((cellPtr->numEntries % maxHistoryUnits) == 0)
      {
        int pageEntries = 0;
        page->Write(&pageEntries, sizeof(int), 0);
        page->Write(&huTiv,sizeof(Interval<Instant>), sizeof(int));
      }

      // Check/modify page time interval
      Interval<Instant> pageTiv;
      page->Read(&pageTiv, sizeof(Interval<Instant> ), sizeof(int));
      if (huTiv.start < pageTiv.start) pageTiv.start = huTiv.start;
      if (huTiv.end > pageTiv.end) pageTiv.end = huTiv.end;

      // Read number of entries in page
      int pageEntries;
      page->Read( &pageEntries, sizeof(int), 0 );

      // Write HistoryUnit in page
      size_t offset =  sizeof(int) + sizeof(Interval<Instant>) +
                      + ((pageEntries)*(sizeof(HistoryUnit)));
      page->Write(&hu->moID, sizeof(int), offset );
      offset += sizeof(int);
      page->Write( &hu->huID, sizeof(int), offset );
      offset += sizeof(int);
      page->Write(&hu->tivStart, sizeof(double), offset);
      offset += sizeof(double);
      page->Write(&hu->tivEnd, sizeof(double), offset);
      offset += sizeof(double);
      page->Write( &hu->pos1, sizeof(UnitPos), offset );
      offset += sizeof(UnitPos);
      page->Write( &hu->pos2, sizeof(UnitPos), offset );
      offset += sizeof(UnitPos);

      // Increase number of entries in cell and page
      cellPtr->numEntries++;
      pageEntries++;
      page->Write(&pageEntries, sizeof(int), 0);

      // Compute offset for next cell
      if (intersects[0] == true) pos1.x = hu->pos2.x + tol;
      else if (intersects[1] == true) pos1.x = hu->pos2.x - tol;
      else pos1.x = hu->pos2.x;
      if (intersects[2] == true) pos1.y = hu->pos2.y + tol;
      else if (intersects[3] == true) pos1.y = hu->pos2.y - tol;
      else pos1.y = hu->pos2.y;
    }
    while ( !(CompareFloats(pos2.x, hu->pos2.x) &&
              CompareFloats(pos2.y, hu->pos2.y)) );
  }
  hPtr->numEntries++;

  if ((hPtr->numEntries % updateCycle) == 0)
  {
    // Update file
    if(!UGRIDPTR->UpdateUGrid()) return 1;
  }
  return 0;
}

/******************************************************************************

6.3 GenerateErrorMsg-method

Generates error messages. The insertUpload and the insertStream operator make
use of this method.

******************************************************************************/

void GenerateErrorMsg(bool error[3])
{
  static MessageCenter* msg = MessageCenter::GetInstance();

  if (error[0])
  {
    NList msgList( NList("simple"),NList("Could not access update file!") );
    msg->Send(nl,msgList.listExpr());
  }
  if (error[1])
  {
    NList msgList( NList("simple"),NList("Upload(s) is/are out of area!") );
    msg->Send(nl,msgList.listExpr());
  }
  if (error[2])
  {
    NList msgList( NList("simple"),NList("Upload(s) is/are out of date!") );
    msg->Send(nl,msgList.listExpr());
  }
}

/******************************************************************************

6.4 insertUpload - Type mapping method

******************************************************************************/


ListExpr InsertUploadTM(ListExpr args)
{
  if ( nl->ListLength( args ) == 2 &&
       nl->IsEqual(nl->First(args), UGrid::BasicType()) &&
       nl->IsEqual(nl->Second(args), UploadUnit::BasicType()))
  {
    return (nl->SymbolAtom(CcBool::BasicType()));
  }
  return listutils::typeError("Expected ugrid x uploadunit!");
}

/******************************************************************************

6.5 insertUpload - Value mapping method

******************************************************************************/

int InsertUploadVM(Word* args, Word& result, int message, Word& local,
                 Supplier s)
{
  UGrid*       ugridPtr = static_cast<UGrid*>(args[0].addr);
  UploadUnit* unitPtr = static_cast<UploadUnit*>(args[1].addr);
  bool error[3];
  for (int i = 0; i < 3; i++) error[i] = false;

  // Insert UploadUnit
  int res = InsertHandle(ugridPtr, unitPtr);

  // Memorize error
  if (res == 1) error[0] = true;
  if (res == 2) error[1] = true;
  if (res == 3) error[2] = true;

  // Update file
  if(!ugridPtr->UpdateUGrid()) error[0] = true;

  // Print error messages
  GenerateErrorMsg(error);

  // Set result
  result = qp->ResultStorage(s);
  if(error[0] || error[1] || error[2])
  {
    static_cast<CcBool*>( result.addr )->Set(true, false);
  }
  else
  {
    static_cast<CcBool*>( result.addr )->Set(true, true);
  }
  return 0;
}

/******************************************************************************

6.6 insertUpload - Specification of operator

******************************************************************************/

struct InsertUploadInfo : OperatorInfo {
  InsertUploadInfo()
  {
    name      = "insertUpload";
    signature = "ugrid x uploadunit -> bool";
    syntax    = "insertUpload( _, _)";
    meaning   = "UGrid insert upload operator.";
  }
};

/******************************************************************************

7 insertStream operator

Inserts a stream of uploads into UGrid.

7.1 insertStream - Type mapping method

******************************************************************************/

ListExpr InsertStreamTM(ListExpr args)
{
  NList type(args);
  if ( !type.hasLength(3) )
  {
   return listutils::typeError("Expecting three arguments.");
  }

  NList first = type.first();
  if ( !first.hasLength(2)  ||
       !first.first().isSymbol(Symbol::STREAM()) ||
       !first.second().hasLength(2) ||
       !first.second().first().isSymbol(Tuple::BasicType()) ||
       !IsTupleDescription( first.second().second().listExpr() ))
  {
    return listutils::typeError("Error in first argument!");
  }

  if ( !nl->IsEqual(nl->Second(args), UGrid::BasicType()) )
  {
    return NList::typeError( "UGrid object for second argument expected!" );
  }

  NList third = type.third();
  if ( !third.isSymbol() )
  {
    return NList::typeError( "Attribute name for third argument expected!" );
  }

  string attrname = type.third().str();
  ListExpr attrtype = nl->Empty();
  int j = FindAttribute(first.second().second().listExpr(),attrname,attrtype);

  if ( j != 0 )
  {
    if ( nl->SymbolValue(attrtype) != UploadUnit::BasicType() )
    {
      return NList::typeError("Attribute type is not of type uploadunit.");
    }
    NList resType = NList(CcBool::BasicType());
    return NList( NList(Symbol::APPEND()),
                  NList(j).enclose(), resType ).listExpr();
  }
  else
  {
    return NList::typeError( "Attribute name '" + attrname +"' is not known!");
  }
}

/******************************************************************************

7.2 insertStream - Value mapping method

******************************************************************************/

int InsertStreamVM(Word* args, Word& result, int message,
                  Word& local, Supplier s)
{
  Word currentTupleWord(Address(0));
  int attributeIndex = static_cast<CcInt*>( args[3].addr )->GetIntval() - 1;
  UGrid*    ugridPtr = static_cast<UGrid*>(args[1].addr);

  bool error[3];
  for (int i = 0; i < 3; i++) error[i] = false;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);

  while(qp->Received(args[0].addr))
  {
    Tuple* currentTuple = static_cast<Tuple*>( currentTupleWord.addr );
    UploadUnit* currentAttr =
    static_cast<UploadUnit*>(currentTuple->GetAttribute(attributeIndex));
    if( currentAttr->IsDefined() )
    {
      // Insert upload
      int res = InsertHandle(ugridPtr, currentAttr);

      // Memorize error
      if (res == 1) error[0] = true;
      if (res == 2) error[1] = true;
      if (res == 3) error[2] = true;
    }
    currentTuple->DeleteIfAllowed();
    qp->Request(args[0].addr, currentTupleWord);
  }
  qp->Close(args[0].addr);

  // Update file
  if(!ugridPtr->UpdateUGrid()) error[0] = true;

  // Print error messages
  GenerateErrorMsg(error);

  // Set result
  result = qp->ResultStorage(s);
  if(error[0] || error[1] || error[2])
  {
    static_cast<CcBool*>( result.addr )->Set(true, false);
  }
  else
  {
    static_cast<CcBool*>( result.addr )->Set(true, true);
  }
  return 0;
}

/******************************************************************************

7.3 insertStream - Specification of operator

******************************************************************************/

struct InsertStreamInfo : OperatorInfo {
  InsertStreamInfo()
  {
    name      = "insertStream";
    signature = "((stream (tuple([a1:d1, ..., ai:uploadunit, "
                "..., an:dn]))) x ugrid x ai) -> bool";
    syntax    = "_ insertStream[ _, _ ]";
    meaning   = "UGrid insert upload stream operator.";
  }
};

/******************************************************************************

8 IntersectsWindow operator

Returns all history units which intersects the search window.

8.1 FindHistoryUnits-method

Finds all history units which intersect the search window. The
intersectsWindow and the insideWindow operator make use of this method.

******************************************************************************/

set<HistoryUnit*> FindHistoryUnits( UGrid* UGRIDPTR, UGridArea AREA,
                                    Instant TIME1, Instant TIME2)
{
  UGridHeader* hPtr = UGRIDPTR->GetHeader();
  set<db_pgno_t>::iterator pIt;
  map<int,HistoryUnit*>::iterator huIt;
  map<int,HistoryUnit*> historyUnits;
  set<HistoryUnit*> hits;

  // Check if SmiUpdateFile is released
  if(UGRIDPTR->GetSemaphore()) return hits;;

  // Lock SmiUpdateFile
  UGRIDPTR->SetSemaphore(true);

  // Check/modify search window
  if ((AREA.x2 < hPtr->area.x1) ||
      (AREA.x1 > hPtr->area.x2) ||
      (AREA.y2 < hPtr->area.y1) ||
      (AREA.y1 > hPtr->area.y2))
      return hits; // Search window does not intersect the UGrid area

  if (AREA.x1<hPtr->area.x1) AREA.x1 = hPtr->area.x1+tol;
  if (AREA.x2>hPtr->area.x2) AREA.x2 = hPtr->area.x2-tol;
  if (AREA.y1<hPtr->area.y1) AREA.y1 = hPtr->area.y1+tol;
  if (AREA.y2>hPtr->area.y2) AREA.y2 = hPtr->area.y2-tol;

  // Find current pages of cell candidates
  int left  = ComputeLine(hPtr->area.x1, hPtr->area.x2, hPtr->splits, AREA.x1);
  int right = ComputeLine(hPtr->area.x1, hPtr->area.x2, hPtr->splits, AREA.x2);
  int bottom= ComputeLine(hPtr->area.y1, hPtr->area.y2, hPtr->splits, AREA.y1);
  int top   = ComputeLine(hPtr->area.y1, hPtr->area.y2, hPtr->splits, AREA.y2);

  // Clear page set
  UGRIDPTR->pages.clear();

  for (int i = bottom; i <= top; i++)
  {
    for (int j = left; j <= right; j++)
    {
      if (UGRIDPTR->GetCell(j,i)->currentPage != 0)
      {
        // Insert current page number
        UGRIDPTR->pages.insert(UGRIDPTR->GetCell(j,i)->currentPage);
      }
    }
  }

  // Find history pages in UGridTree
  UGRIDPTR->FindHistoryPages(AREA,TIME1,TIME2,UGRIDPTR->GetHeader()->rootNode,
                             UGRIDPTR->GetHeader()->numCells);

  db_pgno_t pageID;
  SmiRecord pageRec;
  int moID;
  int huID;
  double tivStart;
  double tivEnd;
  UnitPos pos1;
  UnitPos pos2;

  // Find  HistoryUnits in pages
  for (pIt = UGRIDPTR->pages.begin(); pIt != UGRIDPTR->pages.end();)
  {
    pageID = *pIt;
    SmiUpdatePage* page;
    int PageSelected = UGRIDPTR->GetUpdateFile()->GetPage(pageID, page);
    assert( PageSelected );

    // Read number of HistoryUnits in page
    int numHU;
    page->Read( &numHU, sizeof(int), 0 );
    size_t offset = sizeof(int) + sizeof(Interval<Instant>);

    // Read HistoryUnits
    for (int i = 0; i < numHU; i++)
    {
      page->Read( &moID, sizeof(int), offset );
      offset += sizeof(int);
      page->Read( &huID, sizeof(int), offset );
      offset += sizeof(int);
      page->Read( &tivStart, sizeof(double), offset );
      offset += sizeof(double);
      page->Read( &tivEnd, sizeof(double), offset );
      offset += sizeof(double);
      page->Read( &pos1, sizeof(UnitPos), offset );
      offset += sizeof(UnitPos);
      page->Read( &pos2, sizeof(UnitPos), offset );
      offset += sizeof(UnitPos);

      // Merge HistoryUnits if necessary
      bool found = false;
      huIt = historyUnits.find(huID);
      if ( huIt != historyUnits.end() )
      {
        HistoryUnit* huPtr = huIt->second;
        if ( CompareFloats(huPtr->pos2.x, pos1.x) &&
             CompareFloats(huPtr->pos2.y, pos1.y) )
        {
          huPtr->tivEnd = tivEnd;
          huPtr->pos2 = pos2;
          found = true;
        }
        if ( CompareFloats(huPtr->pos1.x, pos2.x) &&
            CompareFloats(huPtr->pos1.y, pos2.y) )
        {
          huPtr->tivStart = tivStart;
          huPtr->pos1 = pos1;
          found = true;
        }
      }
      if (!found) historyUnits.insert(make_pair( huID,
                  new HistoryUnit( moID, huID, tivStart, tivEnd, pos1, pos2)));
    }
    // Drop page
    UGRIDPTR->GetUpdateFile()->PutPage(page->GetPageNo(),false);
    UGRIDPTR->pages.erase(pIt++);
  }

  // Check if history unit intersects window
  for (huIt = historyUnits.begin(); huIt != historyUnits.end();)
  {
    HistoryUnit* huPtr = huIt->second;;
    Instant huStart(instanttype);
    Instant huEnd(instanttype);
    huStart.ReadFrom(huPtr->tivStart);
    huEnd.ReadFrom(huPtr->tivEnd);

    if ( TIME1   <= huEnd &&
         TIME2   >= huStart &&
       ((AREA.x1 <= huPtr->pos1.x && AREA.y1 <= huPtr->pos1.y  &&
         AREA.x2 >= huPtr->pos1.x && AREA.y2 >= huPtr->pos1.y) ||
        (AREA.x1 <= huPtr->pos2.x && AREA.y1 <= huPtr->pos2.y  &&
         AREA.x2 >= huPtr->pos2.x && AREA.y2 >= huPtr->pos2.y)))
      hits.insert(huPtr);
      else delete huPtr;
      historyUnits.erase(huIt++);
  }

  // Release SmiUpdateFile
  UGRIDPTR->SetSemaphore(false);

  return hits;
}

/******************************************************************************

8.2 intersectsWindow - Type mapping method

******************************************************************************/

ListExpr IntersectsWinTM(ListExpr args)
{
 // Check nested list
  if(!(nl->ListLength( args ) == 4 &&
      nl->IsEqual(nl->First(args),UGrid::BasicType()) &&
      nl->IsEqual(nl->Second(args),Rectangle<2>::BasicType()) &&
      nl->IsEqual(nl->Third(args),Instant::BasicType()) &&
      nl->IsEqual(nl->Fourth(args),Instant::BasicType())))
  {
    ErrorReporter::ReportError("ugrid x rect x instant x instant expected!");
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }


  // Create output list
  ListExpr tupleList = nl->TwoElemList(
                       nl->TwoElemList(nl->SymbolAtom("MovObjId"),
                                       nl->SymbolAtom(CcInt::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("HistoryUnit"),
                                       nl->SymbolAtom(UPoint::BasicType())));
  ListExpr streamList = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                        nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                                       tupleList));
  return streamList;
}

/******************************************************************************

8.3 intersectsWindow - Value mapping method

******************************************************************************/

int IntersectsWinVM(Word* args, Word& result, int message,
                       Word& local, Supplier s)
{
  struct Iterator
  {
    set<HistoryUnit*> hits;
    set<HistoryUnit*>::iterator it;

    Iterator(set<HistoryUnit*> HITS)
    {
      hits = HITS;
      it = hits.begin();
    }
    set<HistoryUnit*>::iterator GetIterator() {return it;}
  };

  Iterator* iterator = static_cast<Iterator*>(local.addr);

  switch( message )
  {
    case OPEN:
   {
      UGrid* ugridPtr     = static_cast<UGrid*>(args[0].addr);
      Rectangle<2>*  rect = static_cast<Rectangle<2>*>(args[1].addr);
      Instant* time1      = static_cast<Instant*>(args[2].addr);
      Instant* time2      = static_cast<Instant*>(args[3].addr);

      // Change instant values if necessary
      if  (*time1 > *time2 )
      {
        Instant* temp = time2;
        time2 = time1;
        time1 = temp;
      }

      // Create UGrid area
      double x1(rect->MinD(0));
      double x2(rect->MaxD(0));
      double y1(rect->MinD(1));
      double y2(rect->MaxD(1));
      UGridArea area(x1,y1,x2,y2);
      area = ModifyArea(area);

      // Find history units which intersect the search window
      set<HistoryUnit*> hits = FindHistoryUnits(ugridPtr,area,*time1,*time2);

      // Initialize iterator
      iterator = new Iterator(hits);
      local.addr = iterator;
      return 0;
    }
    case REQUEST:
    {
      if ( (iterator != 0) && (iterator->it != iterator->hits.end()) )
      {
        HistoryUnit* hu = *iterator->GetIterator();
        CcInt*      moID = new CcInt(true, hu->moID);
        Interval<Instant>* tiv = new Interval<Instant>(
                                                DateTime(0,0,instanttype),
                                                DateTime(0,0,instanttype),
                                                true, true );
        tiv->start.ReadFrom(hu->tivStart);
        tiv->end.ReadFrom(hu->tivEnd);

        UPoint* upoint = new UPoint(*tiv, hu->pos1.x, hu->pos1.y,
                                          hu->pos2.x, hu->pos2.y );
        TupleType* tupType = new TupleType(nl->Second(GetTupleResultType(s)));
        Tuple*         tup = new Tuple( tupType );
        tup->PutAttribute( 0, ( Attribute* ) moID );
        tup->PutAttribute( 1, ( Attribute* ) upoint );
        result.addr = tup;
        delete tiv;
        delete hu;
        iterator->hits.erase(iterator->it++);
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE:
    {
      if (iterator != 0)
      {
        delete iterator;
        local.addr = 0;
      }
      return 0;
    }
    default:
    {
      return -1;
    }
  }
}

/******************************************************************************

8.4 intersectsWindow - Specification of operator

******************************************************************************/

struct IntersectsWinInfo : OperatorInfo {
  IntersectsWinInfo()
  {
   name      =  "intersectsWindow";
    signature =  "ugrid x rect x  instant x instant  -> "
                 "stream (tuple (MovObjId int)(HistoryUnit upoint))";
    syntax    =  "intersectsWindow ( _, _, _, _ )";
    meaning   =  "Returns all history units which"
                 "intersect the search window.";
  }
};

/******************************************************************************

9 insideWindow operator

Returns all history units inside the search window.

9.1 insideWindow - Type mapping method

******************************************************************************/

ListExpr InsideWinTM(ListExpr args)
{
  // Check nested list
  if(!(nl->ListLength( args ) == 4 &&
      nl->IsEqual(nl->First(args),UGrid::BasicType()) &&
      nl->IsEqual(nl->Second(args),Rectangle<2>::BasicType()) &&
      nl->IsEqual(nl->Third(args),Instant::BasicType()) &&
      nl->IsEqual(nl->Fourth(args),Instant::BasicType())))
  {
    ErrorReporter::ReportError("ugrid x rect x instant x instant expected!");
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }


  // Create output list
  ListExpr tupleList = nl->TwoElemList(
                       nl->TwoElemList(nl->SymbolAtom("MovObjId"),
                                       nl->SymbolAtom(CcInt::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("HistoryUnit"),
                                       nl->SymbolAtom(UPoint::BasicType())));
  ListExpr streamList = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                        nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                                       tupleList));
  return streamList;
}

/******************************************************************************

9.2 insideWindow - Value mapping method

******************************************************************************/

int InsideWinVM(Word* args, Word& result,int message,Word& local,Supplier s)
{
  struct Iterator
  {
    set<HistoryUnit*> hits;
    set<HistoryUnit*>::iterator it;

    Iterator(set<HistoryUnit*> HITS)
    {
      hits = HITS;
      it = hits.begin();
    }
    set<HistoryUnit*>::iterator GetIterator() {return it;}
  };

  Iterator* iterator = static_cast<Iterator*>(local.addr);

  switch( message )
  {
    case OPEN:
   {
     UGrid* ugridPtr     = static_cast<UGrid*>(args[0].addr);
     Rectangle<2>*  rect = static_cast<Rectangle<2>*>(args[1].addr);
     Instant* time1      = static_cast<Instant*>(args[2].addr);
     Instant* time2      = static_cast<Instant*>(args[3].addr);

     // Change time values if necessary
     if  (*time1 > *time2 )
     {
       Instant* temp = time2;
       time2 = time1;
       time1 = temp;
     }

     // Create UGrid area
     double x1(rect->MinD(0));
     double x2(rect->MaxD(0));
     double y1(rect->MinD(1));
     double y2(rect->MaxD(1));
     UGridArea area(x1,y1,x2,y2);
     area = ModifyArea(area);

     // Find history units which intersects the search window
     set<HistoryUnit*> tempHits =FindHistoryUnits(ugridPtr,area,*time1,*time2);

     // Check if all history units are inside the window
     set<HistoryUnit*>::iterator itTempHits;
     set<HistoryUnit*> hits;
     for (itTempHits = tempHits.begin(); itTempHits != tempHits.end();
          itTempHits++)
     {
       HistoryUnit* ugridPtr = *itTempHits;
       Instant huStart(instanttype);
       Instant huEnd(instanttype);
       huStart.ReadFrom(ugridPtr->tivStart);
       huEnd.ReadFrom(ugridPtr->tivEnd);

       if ( *time1  <= huStart &&
            *time2  >= huEnd &&
            area.x1 <= ugridPtr->pos1.x && area.x1 <= ugridPtr->pos2.x &&
            area.x2 >= ugridPtr->pos1.x && area.x2 >= ugridPtr->pos2.x &&
            area.y1 <= ugridPtr->pos1.y && area.y1 <= ugridPtr->pos2.y &&
            area.y2 >= ugridPtr->pos1.y && area.y2 >= ugridPtr->pos2.y )
       {
         hits.insert(ugridPtr);
       }
       else delete ugridPtr;
     }

      // Initialize iterator
      iterator = new Iterator(hits);
      local.addr = iterator;
      return 0;
    }
    case REQUEST:
    {
      if ( (iterator != 0) && (iterator->it != iterator->hits.end()) )
      {
        HistoryUnit* hu = *iterator->GetIterator();
        CcInt*     moID = new CcInt(true, hu->moID);
        Interval<Instant>* tiv = new Interval<Instant>(
                                                DateTime(0,0,instanttype),
                                                DateTime(0,0,instanttype),
                                                true, true );
        tiv->start.ReadFrom(hu->tivStart);
        tiv->end.ReadFrom(hu->tivEnd);

        UPoint* upoint = new UPoint(*tiv, hu->pos1.x, hu->pos1.y,
                                          hu->pos2.x, hu->pos2.y );
        TupleType* tupType = new TupleType(nl->Second(GetTupleResultType(s)));
        Tuple*         tup = new Tuple( tupType );
        tup->PutAttribute( 0, ( Attribute* ) moID );
        tup->PutAttribute( 1, ( Attribute* ) upoint );
        iterator->it++;
        result.addr = tup;
        delete tiv;
        delete hu;
        iterator->hits.erase(iterator->it++);
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE:
    {
      if (iterator != 0)
      {
        delete iterator;
        local.addr = 0;
      }
      return 0;
    }
    default:
    {
      return -1;
    }
  }
}

/******************************************************************************

9.3 insideWindow - Specification of operator

******************************************************************************/

struct InsideWinInfo : OperatorInfo {
  InsideWinInfo()
  {
    name      =  "insideWindow";
    signature =  "ugrid x rect x  instant x instant -> "
                 "stream (tuple (MovObjId int)(HistoryUnit upoint))";
    syntax    =  "insideWindow ( _, _, _, _ )";
    meaning   =  "Returns all history units inside the search window.";
  }
};

/******************************************************************************

10 getTrajectory operator

Returns all history units wich belongs to the stated moving object.

10.1 getTrajectory - Type mapping method

******************************************************************************/

ListExpr GetTrajectoryTM(ListExpr args)
{
  if( nl->ListLength( args ) == 2 &&
      nl->IsEqual(nl->First(args),UGrid::BasicType()) &&
      nl->IsEqual(nl->Second(args),CcInt::BasicType()))
  {
    ListExpr tupleList = nl->TwoElemList(
                         nl->TwoElemList(nl->SymbolAtom("MovObjId"),
                                         nl->SymbolAtom(CcInt::BasicType())),
                         nl->TwoElemList(nl->SymbolAtom("HistoryUnit"),
                                         nl->SymbolAtom(UPoint::BasicType())));
    ListExpr streamList = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                                         tupleList));
    return streamList;
  }
  ErrorReporter::ReportError("ugrid x int expected!");
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

/******************************************************************************

10.2 getTrajectory - Value mapping method

******************************************************************************/

int GetTrajectoryVM( Word* args, Word& result, int message, Word& local,
                     Supplier s )
{
  struct Iterator
  {
    set<HistoryUnit*> hits;
    set<HistoryUnit*>::iterator it;

    Iterator(set<HistoryUnit*> HITS)
    {
      hits = HITS;
      it = hits.begin();
    }
    set<HistoryUnit*>::iterator GetIterator() {return it;}
  };

  Iterator* iterator = static_cast<Iterator*>(local.addr);

  switch( message )
  {
    case OPEN:
   {
     UGrid* ugritPtr  = static_cast<UGrid*>(args[0].addr);
     CcInt* movObjId = static_cast<CcInt*>(args[1].addr);
     int    moID     = movObjId->GetValue();

     UGridArea area (ugritPtr->GetHeader()->area.x1,
                     ugritPtr->GetHeader()->area.y1,
                     ugritPtr->GetHeader()->area.x2,
                     ugritPtr->GetHeader()->area.y2);

     // Find all history units
     set<HistoryUnit*> tempHits = FindHistoryUnits( ugritPtr, ModifyArea(area),
                                              ugritPtr->GetHeader()->tiv.start,
                                              ugritPtr->GetHeader()->tiv.end );

     // Filter all history units which belongs to the stated moving object id
     set<HistoryUnit*>::iterator itTempHits;
     set<HistoryUnit*> hits;
     for (itTempHits = tempHits.begin(); itTempHits != tempHits.end();
          itTempHits++)
     {
       HistoryUnit* ugritPtr = *itTempHits;

       if ( moID == ugritPtr->moID )
       {
         hits.insert(ugritPtr);
       }
       else delete ugritPtr;
     }

      // Initialize iterator
      iterator = new Iterator(hits);
      local.addr = iterator;
      return 0;
    }
    case REQUEST:
    {
      if ( iterator->it != iterator->hits.end() )
      {
        HistoryUnit* hu = *iterator->GetIterator();
        CcInt* moID = new CcInt(true, hu->moID);
        Interval<Instant>* tiv = new Interval<Instant>(
                                                DateTime(0,0,instanttype),
                                                DateTime(0,0,instanttype),
                                                true, true );
        tiv->start.ReadFrom(hu->tivStart);
        tiv->end.ReadFrom(hu->tivEnd);

        UPoint* upoint = new UPoint(*tiv, hu->pos1.x, hu->pos1.y,
                                          hu->pos2.x, hu->pos2.y );
        TupleType* tupType = new TupleType(nl->Second(GetTupleResultType(s)));
        Tuple*     tup = new Tuple( tupType );
        tup->PutAttribute( 0, ( Attribute* ) moID );
        tup->PutAttribute( 1, ( Attribute* ) upoint );
        iterator->it++;
        result.addr = tup;
        delete tiv;
        delete hu;
        iterator->hits.erase(iterator->it++);
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE:
    {
      if (iterator != 0)
      {
        delete iterator;
        local.addr = 0;
      }
      return 0;
    }
    default:
    {
      return -1;
    }
  }
}

/******************************************************************************

10.3 getTrajectory - Specification of operator

******************************************************************************/

struct GetTrajectoryInfo : OperatorInfo {
  GetTrajectoryInfo()
  {
    name      =  "getTrajectory";
    signature =  "ugrid x int  -> "
                 "stream (tuple (MovObjId int)(HistoryUnit upoint))";
    syntax    =  "getTrajectory ( _, _ )";
    meaning   =  "Returns all history units which belongs"
                 "to the stated moving object id.";
  }
};

/******************************************************************************

11 currentUpload operator

Returns the current UploadUnit.

11.1 currentUpload - Type mapping method

******************************************************************************/

ListExpr CurrentUploadTM(ListExpr args)
{
  if( nl->ListLength( args ) == 2 &&
      nl->IsEqual(nl->First(args),UGrid::BasicType()) &&
      nl->IsEqual(nl->Second(args),CcInt::BasicType()) )
  {
    return nl->SymbolAtom(UploadUnit::BasicType());
  }
  ErrorReporter::ReportError("ugrid x int expected!");
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

/******************************************************************************

11.2 currentUpload - Value mapping method

******************************************************************************/

int CurrentUploadVM(Word* args,Word& result,int message,Word& local,Supplier s)
{
  UGrid* ugridPtr = static_cast<UGrid*>(args[0].addr);
  CcInt* moID   = static_cast<CcInt*>(args[1].addr);

  UploadUnit* unitPtr = static_cast<UploadUnit*>(qp->ResultStorage(s).addr);

  // Find UploadUnit in frontline
  int id = moID->GetValue();
  map<int,UploadUnit>::iterator it;
  it = ugridPtr->frontline[id%flBuckets].find(id);
  if ( it == ugridPtr->frontline[id%flBuckets].end() )
  {
    unitPtr = new UploadUnit();
    unitPtr->SetDefined(false);
  }
  else
  {
    unitPtr = new UploadUnit(it->second);
    unitPtr->SetDefined(true);
  }
  result.setAddr( unitPtr );
  return 0;
}

/******************************************************************************

11.3 currentUpload - Specification of operator

******************************************************************************/

struct CurrentUploadInfo : OperatorInfo {
  CurrentUploadInfo()
  {
    name      =  "currentUpload";
    signature =  "ugrid x int -> uploadunit";
    syntax    =  "currentUpload ( _, _ )";
    meaning   =  "Returns the current UploadUnit.";
  }
};

/******************************************************************************

12 UGridAlgebra

******************************************************************************/

class UGridAlgebra : public Algebra
{
  public:
    UGridAlgebra() : Algebra()
    {
      AddTypeConstructor(&UGridTC);

      AddOperator( CreateInfo(),        CreateVM,        CreateTM );
      AddOperator( InsertUploadInfo(),  InsertUploadVM,  InsertUploadTM );
      AddOperator( InsertStreamInfo(),  InsertStreamVM,  InsertStreamTM );
      AddOperator( IntersectsWinInfo(), IntersectsWinVM, IntersectsWinTM );
      AddOperator( InsideWinInfo(),     InsideWinVM,     InsideWinTM );
      AddOperator( CurrentUploadInfo(), CurrentUploadVM, CurrentUploadTM );
      AddOperator( GetTrajectoryInfo(), GetTrajectoryVM, GetTrajectoryTM );
    }
    ~UGridAlgebra() {};
};

/******************************************************************************

13 Initialization

******************************************************************************/

extern "C"
Algebra*
InitializeUGridAlgebra( NestedList *nlRef,
                        QueryProcessor *qpRef,
                        AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (new UGridAlgebra());
}
} // End of UGridAlgebra namespace
