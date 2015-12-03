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

[1] Implementation of SETIAlgebra

September 2010, Daniel Brockmann

1 Overview

SETI-Algebra implements a SETI index structure. The implementation makes
use of the existing SECONDO component RTree-Algebra. The memory management
is based on SmiUpdateFile. In addition the UploadUnit is an object type of
this Algebra.

SETI-Algebra offers the following methods:

- createSETI        -> Creates a new SETI object.
- insertUpload      -> Inserts a single upload into SETI.
- insertStream      -> Inserts a stream of uploads into SETI.
- intersectsWindow  -> Returns all trajectory segments which
                       intersect the search window.
- insideWindow      -> Returns all trajectory segments
                       inside the search window.
- getTrajectory     -> Returns all trajectory segments wich belongs
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
#include "RTreeAlgebra.h"
#include "RectangleAlgebra.h"
#include "TemporalAlgebra.h"
#include "UploadUnit.h"
#include "Symbols.h"


using namespace std;
extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;


using namespace temporalalgebra;

namespace SETIAlgebra {

#include "SETIAlgebra.h"

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

SETIArea ModifyArea(SETIArea AREA)
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

Computes the intersection point of two lines (A,B) and (C,D).

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

4 Implementation of class SETI

4.1 Basic constructor

******************************************************************************/

SETI::SETI(SETIArea AREA, int SPLITS) : suf(0), rtreeFile(0)
{
  // Create SETI Header
  header = new SETIHeader(ModifyArea(AREA), SPLITS);

  // Create SmiUpdateFile
  suf = new SmiUpdateFile(pageSize);
  suf->Create();
  header->fileID = suf->GetFileId();

  // Create RTree file
  rtreeFile = new SmiRecordFile(true, rtreePageSize);
  rtreeFile->Create();
  header->rtreeFileID = rtreeFile->GetFileId();

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

  // Create and initialize first cell page
  SmiUpdatePage* cellPage;
  AppendedPage = suf->AppendNewPage(cellPage);
  assert( AppendedPage );
  header->cellPageNo = cellPage->GetPageNo();
  PageSelected = suf->GetPage(header->cellPageNo, cellPage);
  assert( PageSelected );
  cellPage->Write(&nextPageNo, sizeof(db_pgno_t), 0);
  cellPage->Write(&numEntries, sizeof(int), sizeof(db_pgno_t));

  // Calculate x/y cell length
  double areaLenX = abs(AREA.x2 - AREA.x1);
  double areaLenY = abs(AREA.y2 - AREA.y1);
  double cellLenX = areaLenX / header->splits;
  double cellLenY = areaLenY / header->splits;

  // Create an area partition
  SETIArea partition(0,0,0,0);
  for (int i = 0; i < header->splits; i++)
  {
    for (int j = 0; j < header->splits; j++)
    {
      partition.x1 = AREA.x1 + (cellLenX * j);
      partition.x2 = partition.x1 + cellLenX;
      partition.y1 = AREA.y1 + (cellLenY * i);
      partition.y2 = partition.y1 + cellLenY;

      // Create and initialize a new cell object
      cells[j][i] = new SETICell();
      cells[j][i]->numEntries = 0;
      cells[j][i]->area = partition;
      cells[j][i]->currentPage = (db_pgno_t)0;
      cells[j][i]->tiv.start = DateTime(0,0,instanttype);
      cells[j][i]->tiv.end = DateTime(0,0,instanttype);
      cells[j][i]->tiv.lc = false;
      cells[j][i]->tiv.rc = false;

      // Create RTree for cell
      cells[j][i]->rtreePtr = new R_Tree<2,TupleId>(rtreeFile);
      cells[j][i]->rtreeRecID = cells[j][i]->rtreePtr->HeaderRecordId();
    }
  }

  // Write header and cell page into SmiUpdateFile
  UpdateHeader();
  UpdateCells();

  // Initialize semaphore
  SetSemaphore(false);
}

/******************************************************************************

4.2 Query constructor

******************************************************************************/

SETI::SETI(SmiFileId FILEID) : suf(0)
{
  // Open existing file
  suf = new SmiUpdateFile(pageSize);
  suf->Open(FILEID, pageSize);

  // Create header object
  header = new SETIHeader();
}

/******************************************************************************

4.3 Destructor

******************************************************************************/

SETI::~SETI()
{
  // Delete cells
  for (int i = 0; i < header->splits; i++)
  {
    for (int j = 0; j < header->splits; j++)
    {
      delete cells[j][i]->rtreePtr;
      delete cells[j][i];
    }
  }

  // Delete header
  delete header;

  // Clear frontline
  for (int i = 0; i < flBuckets; i++)
  {
    frontline[i].clear();
  }

  // Delete file objects
  if(suf->IsOpen()) suf->Close();
  delete suf;
  if(rtreeFile->IsOpen()) rtreeFile->Close();
  delete rtreeFile;
}

/******************************************************************************

4.4 The mandatory set of algebra support functions

4.4.1 In-method

******************************************************************************/

Word SETI::In( ListExpr typeInfo, ListExpr instance, int errorPos,
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
  SETIArea area(x1,y1,x2,y2);

  // Check number of partitions type
  if ( !(nl->IsAtom( nl->Second( instance ) ) &&
         nl->AtomType( nl->Second( instance ) ) == IntType ))
  {
    cmsg.inFunError("An integer value is expected for second argument!");
    return result;
  }

   // Check number of partitions
  int numPartitions = nl->IntValue( nl->Second(instance) );
  if (!(CompareFloats(fmod(sqrt(numPartitions),1),0) &&
        numPartitions >= 1 &&
        numPartitions <= 4096))
  {
    // Wrong number of partitions -> create default SETI
    result.addr = new SETI(ModifyArea(area), 1);
  }
  else
  {
    result.addr = new SETI(ModifyArea(area), (int)sqrt(numPartitions));
  }
  correct = true;
  return result;
}

/******************************************************************************

4.4.2 Out-method

******************************************************************************/

ListExpr SETI::Out(ListExpr typeInfo, Word value )
{
  SETI* setiPtr = static_cast<SETI*>(value.addr);

  // Create area list
  ListExpr area = nl->FourElemList(
  nl->RealAtom(setiPtr->header->area.x1),
  nl->RealAtom(setiPtr->header->area.x2),
  nl->RealAtom(setiPtr->header->area.y1),
  nl->RealAtom(setiPtr->header->area.y2));

  // Create time interval list
  ListExpr  tiv;
  if (setiPtr->header->numEntries == 0)
  {
    // Time interval is undefined if no entry exist
    tiv = nl->TwoElemList( nl->StringAtom("undefined"),
                           nl->StringAtom("undefined"));
  }
  else if (setiPtr->header->numEntries == 1)
  {
    // End time is undefined if only one entry exist
    tiv = nl->TwoElemList(
              OutDateTime(nl->TheEmptyList(),
                          SetWord(&setiPtr->header->tiv.start)),
                          nl->StringAtom("undefined"));
  }
  else
  {
    // Time interval is defined if SETI has two entries or more
    tiv = nl->TwoElemList(
               OutDateTime(nl->TheEmptyList(),
                          SetWord(&setiPtr->header->tiv.start)),
              OutDateTime(nl->TheEmptyList(),
                          SetWord(&setiPtr->header->tiv.end)));
  }
  // Return output list
  return nl->TwoElemList(area, tiv);
}

/******************************************************************************

4.4.3 Create-method

******************************************************************************/

Word SETI::Create( const ListExpr typeInfo )
{
  return SetWord(Address(0));
}

/******************************************************************************

4.4.4 Delete-method

******************************************************************************/

void SETI::Delete( const ListExpr typeInfo, Word& w )
{
  SETI* setiPtr = static_cast<SETI*>(w.addr);
  setiPtr->rtreeFile->Close();
  if(setiPtr->rtreeFile->IsOpen()) setiPtr->rtreeFile->Drop();
  delete setiPtr;
  w.addr = 0;
}

/******************************************************************************

4.4.5 Open-method

******************************************************************************/

bool SETI::Open( SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value )
{
  SmiFileId fileID;
  db_pgno_t headerPageNo;

  bool ok = true;
  ok = ok && valueRecord.Read( &fileID, sizeof(SmiFileId), offset );
  offset += sizeof(SmiFileId);
  ok = ok && valueRecord.Read( &headerPageNo, sizeof(db_pgno_t), offset );
  offset += sizeof(db_pgno_t);
  // Create new SETI object with existing file
  SETI* setiPtr = new SETI(fileID);
  setiPtr->header->fileID = fileID;
  setiPtr->header->headerPageNo = headerPageNo;
  // Reader header, frontline and cells information from file
  setiPtr->ReadSETI();

  value.addr = setiPtr;
  return ok;
}

/******************************************************************************

4.4.6 Save-method

******************************************************************************/

bool SETI::Save( SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value )
{
  SETI* setiPtr = static_cast<SETI*>(value.addr);
  bool ok = true;
  SmiFileId fileID = setiPtr->header->fileID;
  db_pgno_t headerPageNo = setiPtr->header->headerPageNo;
  ok = ok && valueRecord.Write(&fileID, sizeof(SmiFileId), offset );
  offset += sizeof(SmiFileId);
  ok = ok && valueRecord.Write(&headerPageNo, sizeof(db_pgno_t), offset );
  offset += sizeof(db_pgno_t);
  return ok;
}

/******************************************************************************

4.4.7 Close-method

******************************************************************************/

void SETI::Close( const ListExpr typeInfo, Word& w )
{
  SETI* setiPtr = static_cast<SETI*>(w.addr);
  delete setiPtr;
  w.addr = 0;
}

/******************************************************************************

4.4.8 Clone-method

******************************************************************************/

Word SETI::Clone( const ListExpr typeInfo, const Word& w )
{
  return SetWord(Address(0));
}

/******************************************************************************

4.4.9 Cast-method

******************************************************************************/

void* SETI::Cast( void* addr)
{
  return (0);
}

/******************************************************************************

4.4.10 SizeOfObj-method

******************************************************************************/

int SETI::SizeOfObj()
{
  return sizeof(SETI);
}

/******************************************************************************

4.4.11 KindCheck-method

******************************************************************************/

bool SETI::KindCheck(ListExpr type, ListExpr& errorInfo)
{
  return (nl->IsEqual(type, SETI::BasicType()));
}

/******************************************************************************

4.4.12 Property-method

******************************************************************************/

ListExpr SETI::Property()
{
  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom(SETI::BasicType()),
                             nl->StringAtom("((<x1> <x2> <y1> <y2>) p)"),
                             nl->StringAtom("((8.2 1.6 9.7 4,6) 4096)"),
                             nl->StringAtom("x/y must be of type double, "
                                            "p of type int."))));
}

/******************************************************************************

4.5 ReadSETI-method

Reads header, frontline and cell information from file.

******************************************************************************/

void SETI::ReadSETI()
{
  // Read header --------------------------------------------------------------

  SmiUpdatePage* headerPage;
  int PageSelected = suf->GetPage(header->headerPageNo, headerPage);
  assert( PageSelected );

  size_t offset = 0;
  headerPage->Read(&header->rtreeFileID, sizeof(SmiFileId), offset);
  offset += sizeof(SmiFileId);
  headerPage->Read(&header->flPageNo, sizeof(db_pgno_t), offset);
  offset += sizeof(db_pgno_t);
  headerPage->Read(&header->cellPageNo, sizeof(db_pgno_t), offset);
  offset += sizeof(db_pgno_t);
  headerPage->Read(&header->area, sizeof(SETIArea), offset);
  offset += sizeof(SETIArea);
  headerPage->Read(&header->splits, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Read(&header->numCells, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Read(&header->numEntries, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Read(&header->numFlEntries, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Read(&header->tiv, sizeof(Interval<Instant>), offset);
  offset += sizeof(Interval<Instant>);

  // Read frontline -----------------------------------------------------------

  SmiUpdatePage* flPage;
  PageSelected = suf->GetPage(header->flPageNo, flPage);
  assert( PageSelected );
  size_t flPageOffset = 0;

  for (int i = 0; i < flBuckets; i++)
  {
    SmiUpdatePage* bucketPage;
    db_pgno_t      bucketPageNo;
    db_pgno_t      nextPageNo;
    int            numEntries;

    // Select bucket page
    flPage->Read( &bucketPageNo, sizeof(db_pgno_t), flPageOffset);
    flPageOffset += sizeof(db_pgno_t);
    do
    {
      PageSelected = suf->GetPage(bucketPageNo, bucketPage);
      assert( PageSelected );
      bucketPage->Read(&nextPageNo, sizeof(db_pgno_t), 0);
      bucketPage->Read(&numEntries, sizeof(int), sizeof(db_pgno_t));
      offset = sizeof(db_pgno_t) + sizeof(int);

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

  // Read cells ---------------------------------------------------------------

  SmiUpdatePage* cellPage;
  db_pgno_t cellPageNo = header->cellPageNo;
  db_pgno_t nextPageNo;
  int j  = 1;
  int i  = 1;
  double tivStart;
  double tivEnd;
  int    numEntries;

  // Open RTree file
  rtreeFile = new SmiRecordFile(pageSize);
  rtreeFile->Open(header->rtreeFileID);

  do
  {
    PageSelected = suf->GetPage(cellPageNo, cellPage);
    assert( PageSelected );
    cellPage->Read(&nextPageNo, sizeof(db_pgno_t), 0);
    cellPage->Read(&numEntries, sizeof(int), sizeof(db_pgno_t));
    offset = sizeof(db_pgno_t) + sizeof(int);

    for (int k = 0; k < numEntries; k++)
    {
      cells[j-1][i-1] = new SETICell();
      SETICell* cellPtr = cells[j-1][i-1];
      cellPage->Read(&cellPtr->numEntries, sizeof(int), offset);
      offset += sizeof(int);
      cellPage->Read(&cellPtr->area, sizeof(SETIArea), offset);
      offset += sizeof(SETIArea);
      cellPage->Read(&cellPtr->rtreeRecID, sizeof(SmiRecordId), offset);
      offset += sizeof(SmiRecordId);
      cellPage->Read(&cellPtr->currentPage, sizeof(db_pgno_t), offset);
      offset += sizeof(db_pgno_t);
      cellPage->Read(&tivStart, sizeof(double), offset);
      offset += sizeof(double);
      cellPage->Read(&tivEnd, sizeof(double), offset);
      offset += sizeof(double);
      cellPage->Read(&cellPtr->tiv.lc, sizeof(bool), offset);
      offset += sizeof(bool);
      cellPage->Read(&cellPtr->tiv.rc, sizeof(bool), offset);
      offset += sizeof(bool);
      cellPtr->tiv.start = DateTime(0,0,instanttype);
      cellPtr->tiv.start.ReadFrom(tivStart);
      cellPtr->tiv.end = DateTime(0,0,instanttype);
      cellPtr->tiv.end.ReadFrom(tivEnd);

      // Create new RTree with existing RTree file/header
      cellPtr->rtreePtr = new R_Tree<2,TupleId>( rtreeFile,
                                                 cellPtr->rtreeRecID );

      if ((j % header->splits) == 0) { j = 0; i++; }
      j++;
    }
    if ( nextPageNo != 0 ) cellPageNo = nextPageNo;
  } while(nextPageNo != 0);
}

/******************************************************************************

4.6 UpdateHeader-method

Writes header information into file.

******************************************************************************/

void SETI::UpdateHeader()
{
  SmiUpdatePage* headerPage;
  int PageSelected = suf->GetPage(header->headerPageNo, headerPage);
  assert( PageSelected );

  size_t offset = 0;
  headerPage->Write(&header->rtreeFileID, sizeof(SmiFileId), offset);
  offset += sizeof(SmiFileId);
  headerPage->Write(&header->flPageNo, sizeof(db_pgno_t), offset);
  offset += sizeof(db_pgno_t);
  headerPage->Write(&header->cellPageNo, sizeof(db_pgno_t), offset);
  offset += sizeof(db_pgno_t);
  headerPage->Write(&header->area, sizeof(SETIArea), offset);
  offset += sizeof(SETIArea);
  headerPage->Write(&header->splits, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Write(&header->numCells, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Write(&header->numEntries, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Write(&header->numFlEntries, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Write(&header->tiv, sizeof(Interval<Instant>), offset);
  offset += sizeof(Interval<Instant>);
}

/******************************************************************************

4.7 UpdateFLine-method

Writes front-line information into file.

******************************************************************************/

void SETI::UpdateFLine()
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
    flPage->Read( &bucketPageNo, sizeof(db_pgno_t),flPageOffset);
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

4.8 UpdateCells-method

Writes cell information into file.

******************************************************************************/

void SETI::UpdateCells()
{
  SmiUpdatePage* cellPage;
  db_pgno_t nextPageNo;
  int numEntries = 0;

  int PageSelected = suf->GetPage(header->cellPageNo, cellPage);
  assert( PageSelected );

  size_t offset = sizeof(db_pgno_t) + sizeof(int);
  for (int i = 0; i < header->splits; i++)
  {
    for (int j = 0; j < header->splits; j++)
    {
      // Increase number of entries in page
      numEntries++;
      cellPage->Write( &numEntries, sizeof(int), sizeof(db_pgno_t) );

      SETICell* cellPtr = cells[j][i];
      double tivStart = cellPtr->tiv.start.ToDouble();
      double tivEnd   = cellPtr->tiv.end.ToDouble();
      cellPage->Write(&cellPtr->numEntries, sizeof(int), offset);
      offset += sizeof(int);
      cellPage->Write(&cellPtr->area, sizeof(SETIArea), offset);
      offset += sizeof(SETIArea);
      cellPage->Write(&cellPtr->rtreeRecID, sizeof(SmiRecordId), offset);
      offset += sizeof(SmiRecordId);
      cellPage->Write(&cellPtr->currentPage, sizeof(db_pgno_t), offset);
      offset += sizeof(db_pgno_t);
      cellPage->Write(&tivStart, sizeof(double), offset);
      offset += sizeof(double);
      cellPage->Write(&tivEnd, sizeof(double), offset);
      offset += sizeof(double);
      cellPage->Write(&cellPtr->tiv.lc, sizeof(bool), offset);
      offset += sizeof(bool);
      cellPage->Write(&cellPtr->tiv.rc, sizeof(bool), offset);
      offset += sizeof(bool);

      if((numEntries%64) == 0)
      {
        // If page is full up get next page
        cellPage->Read(&nextPageNo, sizeof(db_pgno_t), 0);
        if(nextPageNo == 0)
        {
          // No next page exist -> create new next page
          SmiUpdatePage* newPage;
          int AppendedPage = suf->AppendNewPage(newPage);
          assert( AppendedPage );
          nextPageNo = newPage->GetPageNo();
          cellPage->Write(&nextPageNo, sizeof(db_pgno_t), 0);
        }
        // Select and initialize next page
        PageSelected = suf->GetPage(nextPageNo, cellPage);
        assert( PageSelected );
        nextPageNo = 0;
        numEntries = 0;
        offset = sizeof(db_pgno_t) + sizeof(int);
        cellPage->Write(&nextPageNo, sizeof(db_pgno_t), 0);
        cellPage->Write(&numEntries, sizeof(int), sizeof(db_pgno_t));
      }
    }
  }
}

/******************************************************************************

4.9 UpdateSETI-method

Writes all SETI data into file.

******************************************************************************/

bool SETI::UpdateSETI()
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

    // Update cells
    UpdateCells();

    // Release SmiUpdateFile
    SetSemaphore(false);

    return true;
  }
}

/******************************************************************************

4.10 GetUpdateFile-method

Returns the SETI file id.

******************************************************************************/

SmiUpdateFile* SETI::GetUpdateFile()
{
 return suf;
}

/******************************************************************************

4.11 GetHeader-method

Returns the pointer to the SETI header.

******************************************************************************/

SETIHeader* SETI::GetHeader()
{
 return header;
}

/******************************************************************************

4.12 GetCell-method

Returns the pointer to the stated cell.

******************************************************************************/

SETICell* SETI::GetCell(int COL, int ROW)
{
 return cells[COL][ROW];
}

/******************************************************************************

4.13 GetSemaphore-method

Returns the state of the semaphore.

******************************************************************************/

bool SETI::GetSemaphore()
{
  bool value;
  SmiUpdatePage* headerPage;
  int PageSelected = suf->GetPage(header->headerPageNo, headerPage);
  assert( PageSelected );
  headerPage->Read(&value, sizeof(bool), sizeof(SETIHeader));

 // Wait until file is released or timeout is 0
 int timeout = 100000;
  while(value && (timeout >= 0))
  {
    headerPage->Read(&value, sizeof(bool), sizeof(SETIHeader));
    for(int i = 0; i < 10000; i++);
    timeout--;
  }
  return value;
}

/******************************************************************************

4.14 SetSemaphore-method

Sets the state of the semaphore.

******************************************************************************/

void SETI::SetSemaphore(bool value)
{
  SmiUpdatePage* headerPage;
  int PageSelected = suf->GetPage(header->headerPageNo, headerPage);
  assert( PageSelected );
  headerPage->Write(&value, sizeof(bool), sizeof(SETIHeader));
}

/******************************************************************************

4.15 Type constructor

******************************************************************************/

TypeConstructor SETITC(
        SETI::BasicType(),            // name
        SETI::Property,               // property function
        SETI::Out,   SETI::In,        // Out and In functions
        0, 0,                         // SaveTo and RestoreFrom functions
        SETI::Create,  SETI::Delete,  // object creation and deletion
        SETI::Open,    SETI::Save,    // object open and save
        SETI::Close,   SETI::Clone,   // object close and clone
        SETI::Cast,                   // cast function
        SETI::SizeOfObj,              // sizeof function
        SETI::KindCheck );            // kind checking function

/******************************************************************************

5 setiCreate operator

Creates a new SETI object.

5.1 createSETI - Type mapping method

******************************************************************************/

ListExpr CreateTM(ListExpr args)
{
  if( nl->ListLength( args ) == 2 &&
      nl->IsEqual(nl->First(args),Rectangle<2>::BasicType()) &&
      nl->IsEqual(nl->Second(args),CcInt::BasicType()) )
  {
    return (nl->SymbolAtom(SETI::BasicType()));
  }
  ErrorReporter::ReportError("rect x int expected!");
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

/******************************************************************************

5.2 createSETI - Value mapping method

******************************************************************************/

int CreateVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  SETI* setiPtr = static_cast<SETI*>(qp->ResultStorage(s).addr);

  Rectangle<2>* rect = static_cast<Rectangle<2>*>(args[0].addr);
  CcInt*  partitions = static_cast<CcInt*>(args[1].addr);

  // Create SETI area
  double x1(rect->MinD(0));
  double x2(rect->MaxD(0));
  double y1(rect->MinD(1));
  double y2(rect->MaxD(1));
  SETIArea area(x1,y1,x2,y2);
  area = ModifyArea(area);

  // Check number of partitions
  int numPartitions = partitions->GetValue();
   if (!(CompareFloats(fmod(sqrt(numPartitions),1),0) &&
        numPartitions >= 1 &&
        numPartitions <= 4096))
  {
   // Wrong number of partitions -> create default SETI
   setiPtr = new SETI(area,1);
  }
  else
  {
    // Create SETI with the stated number of partitions
    setiPtr = new SETI(area,(int)sqrt(numPartitions));
  }
  result.setAddr( setiPtr );
  return 0;
}

/******************************************************************************

5.3 createSETI - Specification of operator

******************************************************************************/

struct CreateInfo : OperatorInfo {
  CreateInfo()
  {
    name      = "createSETI";
    signature = "rect x int -> seti";
    syntax    = "createSETI( _, _ )";
    meaning   = "SETI construction operator. The second argument must be a "
                "sqare number, otherwise it will be set a default value.";
  }
};

/******************************************************************************

6 insertUpload operator

Inserts a single upload into SETI.

6.1 ComputeTrjSegment-method

Computes a trajectory segment and returns it to the insertHandle method.

******************************************************************************/

TrjSeg* ComputeTrjSegment( SETIArea AREA, int SPLITS,
                           UnitPos POS1, UnitPos POS2,
                           Interval<Instant> TIV )
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

  // Create new end point for segment
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

  TrjSeg* segment = new TrjSeg(0, 0, TIV.start.ToDouble(),
                               TIV.end.ToDouble(), POS1, p);
  return segment;
}

/******************************************************************************

6.2 InsertHandle-method

Inserts an upload into the SETI index structure. The insertUnit and
the insertStream operator make use of this method.

******************************************************************************/

int InsertHandle(SETI* SETIPTR, UploadUnit* UNITPTR)
{
  SmiUpdateFile* suf  = SETIPTR->GetUpdateFile();
  SETIHeader*    hPtr = SETIPTR->GetHeader();

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

  it = SETIPTR->frontline[moID%flBuckets].find(moID);
  if ( it == SETIPTR->frontline[moID%flBuckets].end() )
  {
    // No entry found -> Place UploadUnit in front line
    SETIPTR->frontline[moID%flBuckets].insert(make_pair(moID,*UNITPTR));
    hPtr->numFlEntries++;
  }
  else
  {
    // Entry found
    if(!suf->IsOpen()) suf->Open(hPtr->fileID, pageSize);
    TrjSeg* segment;
    UnitPos pos1 = it->second.GetPos();
    UnitPos pos2 = UNITPTR->GetPos();
    Interval<Instant> segTiv;
    segTiv.start = it->second.GetTime();
    segTiv.end   = UNITPTR->GetTime();
    segTiv.lc    = true;
    segTiv.rc    = true;

    // Check if the current upload is younger than the last upload
    if(segTiv.start >= segTiv.end) return 3;

    // Replace UploadUnit in frontline
    SETIPTR->frontline[moID%flBuckets][moID] = *UNITPTR;

    // Insert trajectory segment
    do
    {
      // Find cell
      int col =ComputeLine(hPtr->area.x1, hPtr->area.x2, hPtr->splits, pos1.x);
      int row =ComputeLine(hPtr->area.y1, hPtr->area.y2, hPtr->splits, pos1.y);
      SETICell* cellPtr = SETIPTR->GetCell(col, row);

      SmiUpdatePage* page;
      if ((cellPtr->numEntries % maxTrjSeg) == 0)
      {
        // Current cell page is full up or does not exist.
        if(cellPtr->currentPage != 0)
        {
          // Insert temporal entry into the RTree
          Interval<Instant> pageTiv;
          int PageSelected = suf->GetPage(cellPtr->currentPage, page);
          assert( PageSelected );
          page->Read(&pageTiv, sizeof(Interval<Instant> ), sizeof(int));

          Rectangle<2> box = Rectangle<2>(true, pageTiv.start.ToDouble()-tol,
                                                pageTiv.end.ToDouble()+tol,
                                                -1.0, 1.0);
          R_TreeLeafEntry<2,TupleId> e(box,cellPtr->currentPage);
          cellPtr->rtreePtr->Insert(e);

          // Put current page back to disk
          //suf->PutPage(cellPtr->currentPage, true);

          // Reset cell data
          cellPtr->numEntries = 0;
          cellPtr->tiv.lc = false;
          cellPtr->tiv.rc = false;
        }
        // Create new page if no page exist or current page is full up
        int AppendedPage = suf->AppendNewPage(page);
        assert( AppendedPage );
        cellPtr->currentPage = page->GetPageNo();
      }
      int PageSelected = suf->GetPage(cellPtr->currentPage, page);
      assert( PageSelected );

      // Create/split new segment
      segment = ComputeTrjSegment(hPtr->area,hPtr->splits,pos1,pos2,segTiv);
      segment->moID   = moID;
      segment->segID  = hPtr->numEntries;

      // Check/modify SETI time interval
      if ((!hPtr->tiv.lc) || (hPtr->tiv.start > segTiv.start))
      {
        hPtr->tiv.start = segTiv.start;
        hPtr->tiv.lc = true;
      }

      if ((!hPtr->tiv.rc) || (hPtr->tiv.end < segTiv.end))
      {
        hPtr->tiv.end = segTiv.end;
        hPtr->tiv.rc = true;
      }

      // Check/modify cell time interval
      if ((!cellPtr->tiv.lc) || (cellPtr->tiv.start > segTiv.start))
      {
        cellPtr->tiv.start = segTiv.start;
        cellPtr->tiv.lc = true;
      }

      if ((!cellPtr->tiv.rc) || (cellPtr->tiv.end < segTiv.end))
      {
        cellPtr->tiv.end = segTiv.end;
        cellPtr->tiv.rc = true;
      }

      // Initialize tiv and number of entries in page
      if ((cellPtr->numEntries % maxTrjSeg) == 0)
      {
        int pageEntries = 0;
        page->Write(&pageEntries, sizeof(int), 0);
        page->Write(&segTiv,sizeof(Interval<Instant>), sizeof(int));
      }

      // Check/modify page time interval
      Interval<Instant> pageTiv;
      page->Read(&pageTiv, sizeof(Interval<Instant> ), sizeof(int));
      if (segTiv.start < pageTiv.start) pageTiv.start = segTiv.start;
      if (segTiv.end > pageTiv.end) pageTiv.end = segTiv.end;

      // Read number of entries in page
      int pageEntries;
      page->Read( &pageEntries, sizeof(int), 0 );

      // Write segment in page
      size_t offset =  sizeof(int) + sizeof(Interval<Instant>) +
                      + ((pageEntries)*(sizeof(TrjSeg)));
      page->Write(&segment->moID, sizeof(int), offset );
      offset += sizeof(int);
      page->Write( &segment->segID, sizeof(int), offset );
      offset += sizeof(int);
      page->Write(&segment->tivStart, sizeof(double), offset);
      offset += sizeof(double);
      page->Write(&segment->tivEnd, sizeof(double), offset);
      offset += sizeof(double);
      page->Write( &segment->pos1, sizeof(UnitPos), offset );
      offset += sizeof(UnitPos);
      page->Write( &segment->pos2, sizeof(UnitPos), offset );
      offset += sizeof(UnitPos);

      // Increase number of entries in cell and page
      cellPtr->numEntries++;
      pageEntries++;
      page->Write(&pageEntries, sizeof(int), 0);

      // Compute offset for next cell
      if (intersects[0] == true) pos1.x = segment->pos2.x + tol;
      else if (intersects[1] == true) pos1.x = segment->pos2.x - tol;
      else pos1.x = segment->pos2.x;
      if (intersects[2] == true) pos1.y = segment->pos2.y + tol;
      else if (intersects[3] == true) pos1.y = segment->pos2.y - tol;
      else pos1.y = segment->pos2.y;
    }
    while ( !(CompareFloats(pos2.x, segment->pos2.x) &&
              CompareFloats(pos2.y, segment->pos2.y)) );
  }
  hPtr->numEntries++;


  if ((hPtr->numEntries % updateCycle) == 0)
  {
    // Update file
    if(!SETIPTR->UpdateSETI()) return 1;
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
    msg->Send(nl, msgList.listExpr());
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
       nl->IsEqual(nl->First(args), SETI::BasicType()) &&
       nl->IsEqual(nl->Second(args), UploadUnit::BasicType()))
  {
    return (nl->SymbolAtom(CcBool::BasicType()));
  }
  return listutils::typeError("Expected seti x uploadunit!");
}

/******************************************************************************

6.5 insertUpload - Value mapping method

******************************************************************************/

int InsertUploadVM(Word* args, Word& result, int message,
                  Word& local, Supplier s)
{
  SETI*       setiPtr = static_cast<SETI*>(args[0].addr);
  UploadUnit* unitPtr = static_cast<UploadUnit*>(args[1].addr);

  bool error[3];
  for (int i = 0; i < 3; i++) error[i] = false;

  // Insert UploadUnit
  int res = InsertHandle(setiPtr, unitPtr);

  // Memorize error
  if (res == 1) error[0] = true;
  if (res == 2) error[1] = true;
  if (res == 3) error[2] = true;

  // Update file
  if(!setiPtr->UpdateSETI()) error[0] = true;

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
    signature = "seti x uploadunit -> bool";
    syntax    = "insertUpload( _, _)";
    meaning   = "SETI insert upload operator.";
  }
};

/******************************************************************************

7 insertStream operator

Inserts a stream of uploads into SETI.

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

  if ( !nl->IsEqual(nl->Second(args), SETI::BasicType()) )
  {
    return NList::typeError( "SETI object for second argument expected!" );
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
    return NList( NList(Symbol::APPEND()), NList(j).enclose(),
                  resType ).listExpr();
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
  SETI*      setiPtr = static_cast<SETI*>(args[1].addr);

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
      int res = InsertHandle(setiPtr, currentAttr);

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
  if(!setiPtr->UpdateSETI()) error[0] = true;

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
                "..., an:dn]))) x seti x ai) -> bool";
    syntax    = "_ insertStream[ _, _ ]";
    meaning   = "SETI insert upload stream operator.";
  }
};


/******************************************************************************

8 intersectsWindow operator

Returns all trajectory segments which intersects the search window.

8.1 FindTrjSegments-method

Finds all trajectory segments which intersects the search window. The
intersectsWindow and the insideWindow operator make use of this method.

******************************************************************************/

set<TrjSeg*> FindTrjSegments( SETI* SETIPTR, SETIArea AREA,
                              Instant TIME1, Instant TIME2)
{
  SETIHeader* hPtr = SETIPTR->GetHeader();
  set<db_pgno_t>::iterator pIt;
  set<db_pgno_t> pages;
  map<int,TrjSeg*>::iterator sIt;
  map<int,TrjSeg*> segments;
  set<TrjSeg*> hits;

  // Check if SmiUpdateFile is released
  if(SETIPTR->GetSemaphore()) return hits;;

  // Lock SmiUpdateFile
  SETIPTR->SetSemaphore(true);

  // Check/modify search window
  if ((AREA.x2 < hPtr->area.x1) ||
      (AREA.x1 > hPtr->area.x2) ||
      (AREA.y2 < hPtr->area.y1) ||
      (AREA.y1 > hPtr->area.y2))
      return hits; // Search window does not intersect the SETI area

  if (AREA.x1<hPtr->area.x1) AREA.x1 = hPtr->area.x1+tol;
  if (AREA.x2>hPtr->area.x2) AREA.x2 = hPtr->area.x2-tol;
  if (AREA.y1<hPtr->area.y1) AREA.y1 = hPtr->area.y1+tol;
  if (AREA.y2>hPtr->area.y2) AREA.y2 = hPtr->area.y2-tol;

  // Find pages which belong to the stated cell candidates
  int left   = ComputeLine(hPtr->area.x1,hPtr->area.x2,hPtr->splits,AREA.x1);
  int right  = ComputeLine(hPtr->area.x1,hPtr->area.x2,hPtr->splits,AREA.x2);
  int bottom = ComputeLine(hPtr->area.y1,hPtr->area.y2,hPtr->splits,AREA.y1);
  int top    = ComputeLine(hPtr->area.y1,hPtr->area.y2,hPtr->splits,AREA.y2);

  Rectangle<2> searchBox = Rectangle<2>(true, TIME1.ToDouble()-0.1,
                                              TIME2.ToDouble()+0.1,
                                              -1.0, 1.0);
  for (int i = bottom; i <= top; i++)
  {
    for (int j = left; j <= right; j++)
    {
      if (SETIPTR->GetCell(j,i)->currentPage != 0)
      {
        // Insert current page number
        pages.insert(SETIPTR->GetCell(j,i)->currentPage);
      }
      R_TreeLeafEntry<2, TupleId> e;
      if ( SETIPTR->GetCell(j,i)->rtreePtr->First( searchBox, e ) )
      {
        // Insert page numbers found in RTree
        pages.insert((db_pgno_t)e.info);
        while ( SETIPTR->GetCell(j,i)->rtreePtr->Next(e) )
        {
          pages.insert((db_pgno_t)e.info);
        }
      }
    }
  }

  db_pgno_t pageID;
  SmiRecord   pageRec;
  int moID;
  int segID;
  double tivStart;
  double tivEnd;
  UnitPos pos1;
  UnitPos pos2;

  // Find  segments in pages
  for (pIt = pages.begin(); pIt != pages.end();)
  {
    pageID = *pIt;
    SmiUpdatePage* page;
    int PageSelected = SETIPTR->GetUpdateFile()->GetPage(pageID, page);
    assert( PageSelected );

    // Read number of segments in page
    int numSegments;
    page->Read( &numSegments, sizeof(int), 0 );
    size_t offset = sizeof(int) + sizeof(Interval<Instant>);

    // Read segments in page
    for (int i = 0; i < numSegments; i++)
    {
      page->Read( &moID, sizeof(int), offset );
      offset += sizeof(int);
      page->Read( &segID, sizeof(int), offset );
      offset += sizeof(int);
      page->Read( &tivStart, sizeof(double), offset );
      offset += sizeof(double);
      page->Read( &tivEnd, sizeof(double), offset );
      offset += sizeof(double);
      page->Read( &pos1, sizeof(UnitPos), offset );
      offset += sizeof(UnitPos);
      page->Read( &pos2, sizeof(UnitPos), offset );
      offset += sizeof(UnitPos);

      // Merge segments if necessary
      bool found = false;
      sIt = segments.find(segID);
      if ( sIt != segments.end() )
      {
        TrjSeg* segPtr = sIt->second;
        if ( CompareFloats(segPtr->pos2.x, pos1.x) &&
             CompareFloats(segPtr->pos2.y, pos1.y) )
        {
          segPtr->tivEnd = tivEnd;
          segPtr->pos2 = pos2;
          found = true;
        }
        if ( CompareFloats(segPtr->pos1.x, pos2.x) &&
             CompareFloats(segPtr->pos1.y, pos2.y) )
        {
          segPtr->tivStart = tivStart;
          segPtr->pos1 = pos1;
          found = true;
        }
      }
      if(!found) segments.insert(make_pair(segID,
                    new TrjSeg( moID, segID, tivStart, tivEnd, pos1, pos2 )));
    }
    // Drop page
    SETIPTR->GetUpdateFile()->PutPage(page->GetPageNo(),false);
    pages.erase(pIt++);
  }

  // Check if segment intersects window
  for (sIt = segments.begin(); sIt != segments.end();)
  {
    TrjSeg* segPtr = sIt->second;
    Instant segStart(instanttype);
    Instant segEnd(instanttype);
    segStart.ReadFrom(segPtr->tivStart);
    segEnd.ReadFrom(segPtr->tivEnd);

    if ( TIME1   <= segEnd &&
         TIME2   >= segStart &&
       ((AREA.x1 <= segPtr->pos1.x && AREA.y1 <= segPtr->pos1.y  &&
         AREA.x2 >= segPtr->pos1.x && AREA.y2 >= segPtr->pos1.y) ||
        (AREA.x1 <= segPtr->pos2.x && AREA.y1 <= segPtr->pos2.y  &&
         AREA.x2 >= segPtr->pos2.x && AREA.y2 >= segPtr->pos2.y)))
      hits.insert(segPtr);
      else delete segPtr;
      segments.erase(sIt++);
  }

  // Release SmiUpdateFile
  SETIPTR->SetSemaphore(false);

  return hits;
}

/******************************************************************************

8.2 intersectsWindow - Type mapping method

******************************************************************************/

ListExpr IntersectsWinTM(ListExpr args)
{
  // Check nested list
  if(!(nl->ListLength( args ) == 4 &&
      nl->IsEqual(nl->First(args),SETI::BasicType()) &&
      nl->IsEqual(nl->Second(args),Rectangle<2>::BasicType()) &&
      nl->IsEqual(nl->Third(args),Instant::BasicType()) &&
      nl->IsEqual(nl->Fourth(args),Instant::BasicType())))
  {
    ErrorReporter::ReportError("seti x rect x instant x instant expected!");
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }


  // Create output list
  ListExpr tupleList = nl->TwoElemList(
                       nl->TwoElemList(nl->SymbolAtom("MovObjId"),
                                       nl->SymbolAtom(CcInt::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("TrjSeg"),
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
    set<TrjSeg*> hits;
    set<TrjSeg*>::iterator it;

    Iterator(set<TrjSeg*> HITS)
    {
      hits = HITS;
      it = hits.begin();
    }
    set<TrjSeg*>::iterator GetIterator() {return it;}
  };

  Iterator* iterator = static_cast<Iterator*>(local.addr);

  switch( message )
  {
    case OPEN:
   {
      SETI* setiPtr       = static_cast<SETI*>(args[0].addr);
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

      // Create SETI area
      double x1(rect->MinD(0));
      double x2(rect->MaxD(0));
      double y1(rect->MinD(1));
      double y2(rect->MaxD(1));
      SETIArea area(x1,y1,x2,y2);
      area = ModifyArea(area);

      // Find trajectory segments which intersect the search window
      set<TrjSeg*> hits = FindTrjSegments( setiPtr, area, *time1, *time2);

      // Initialize iterator
      iterator = new Iterator(hits);
      local.addr = iterator;
      return 0;
    }
    case REQUEST:
    {
      if ( (iterator != 0) && (iterator->it != iterator->hits.end()) )
      {
        TrjSeg* segment = *iterator->GetIterator();
        CcInt* moID = new CcInt(true, segment->moID);
        Interval<Instant>* tiv = new Interval<Instant>(
                                                DateTime(0,0,instanttype),
                                                DateTime(0,0,instanttype),
                                                true, true );
        tiv->start.ReadFrom(segment->tivStart);
        tiv->end.ReadFrom(segment->tivEnd);

        UPoint* upoint = new UPoint(*tiv, segment->pos1.x, segment->pos1.y,
                                          segment->pos2.x, segment->pos2.y );
        TupleType* tupType = new TupleType(nl->Second(GetTupleResultType(s)));
        Tuple*         tup = new Tuple( tupType );
        tup->PutAttribute( 0, ( Attribute* ) moID );
        tup->PutAttribute( 1, ( Attribute* ) upoint );
        result.addr = tup;
        delete tiv;
        delete segment;
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
    signature =  "seti x rect x  instant x instant  -> "
                 "stream (tuple (MovObjId int)(TrjSeg upoint))";
    syntax    =  "intersectsWindow ( _, _, _, _ )";
    meaning   =  "Returns all trajectory segments which"
                 "intersect the search window.";
  }
};

/******************************************************************************

9 insideWindow operator

Returns all trajectory segments inside the search window.

9.1 insideWindow - Type mapping method

******************************************************************************/

ListExpr InsideWinTM(ListExpr args)
{
  // Check nested list
  if(!(nl->ListLength( args ) == 4 &&
      nl->IsEqual(nl->First(args),SETI::BasicType()) &&
      nl->IsEqual(nl->Second(args),Rectangle<2>::BasicType()) &&
      nl->IsEqual(nl->Third(args),Instant::BasicType()) &&
      nl->IsEqual(nl->Fourth(args),Instant::BasicType())))
  {
    ErrorReporter::ReportError("seti x rect x instant x instant expected!");
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
  }
  // Create output list
  ListExpr tupleList = nl->TwoElemList(
                       nl->TwoElemList(nl->SymbolAtom("MovObjId"),
                                       nl->SymbolAtom(CcInt::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("TrjSeg"),
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
    set<TrjSeg*> hits;
    set<TrjSeg*>::iterator it;

    Iterator(set<TrjSeg*> HITS)
    {
      hits = HITS;
      it = hits.begin();
    }
    set<TrjSeg*>::iterator GetIterator() {return it;}
  };

  Iterator* iterator = static_cast<Iterator*>(local.addr);

  switch( message )
  {
    case OPEN:
   {
     SETI* setiPtr       = static_cast<SETI*>(args[0].addr);
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

     // Create SETI area
     double x1(rect->MinD(0));
     double x2(rect->MaxD(0));
     double y1(rect->MinD(1));
     double y2(rect->MaxD(1));
     SETIArea area(x1,y1,x2,y2);
     area = ModifyArea(area);

     // Find trajectory segments which the intersects search window
     set<TrjSeg*> tempHits = FindTrjSegments( setiPtr, area, *time1, *time2);

     // Check if all segments are inside the window
     set<TrjSeg*>::iterator itTempHits;
     set<TrjSeg*> hits;
     for (itTempHits = tempHits.begin(); itTempHits != tempHits.end();
          itTempHits++)
     {
       TrjSeg* segPtr = *itTempHits;
       Instant segStart(instanttype);
       Instant segEnd(instanttype);
       segStart.ReadFrom(segPtr->tivStart);
       segEnd.ReadFrom(segPtr->tivEnd);

       if ( *time1  <= segStart &&
            *time2  >= segEnd &&
            area.x1 <= segPtr->pos1.x && area.x1 <= segPtr->pos2.x &&
            area.x2 >= segPtr->pos1.x && area.x2 >= segPtr->pos2.x &&
            area.y1 <= segPtr->pos1.y && area.y1 <= segPtr->pos2.y &&
            area.y2 >= segPtr->pos1.y && area.y2 >= segPtr->pos2.y )
       {
         hits.insert(segPtr);
       }
       else delete segPtr;
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
        TrjSeg* segment = *iterator->GetIterator();
        CcInt*     moID = new CcInt(true, segment->moID);
        Interval<Instant>* tiv = new Interval<Instant>(
                                                DateTime(0,0,instanttype),
                                                DateTime(0,0,instanttype),
                                                true, true );
        tiv->start.ReadFrom(segment->tivStart);
        tiv->end.ReadFrom(segment->tivEnd);

        UPoint* upoint = new UPoint(*tiv, segment->pos1.x, segment->pos1.y,
                                          segment->pos2.x, segment->pos2.y );
        TupleType* tupType = new TupleType(nl->Second(GetTupleResultType(s)));
        Tuple*         tup = new Tuple( tupType );
        tup->PutAttribute( 0, ( Attribute* ) moID );
        tup->PutAttribute( 1, ( Attribute* ) upoint );
        result.addr = tup;
        delete tiv;
        delete segment;
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
    signature =  "seti x rect x  instant x instant  -> "
                 "stream (tuple (MovObjId int)(TrjSeg upoint))";
    syntax    =  "insideWindow ( _, _, _, _ )";
    meaning   =  "Returns all trajectory segments inside the search window.";
  }
};

/******************************************************************************

10 getTrajectory operator

Returns all trajectory segments  wich belongs to the stated moving object.

10.1 getTrajectory - Type mapping method

******************************************************************************/

ListExpr GetTrajectoryTM(ListExpr args)
{
  if( nl->ListLength( args ) == 2 &&
      nl->IsEqual(nl->First(args),SETI::BasicType()) &&
      nl->IsEqual(nl->Second(args),CcInt::BasicType()))
  {
    ListExpr tupleList = nl->TwoElemList(
                         nl->TwoElemList(nl->SymbolAtom("MovObjId"),
                                         nl->SymbolAtom(CcInt::BasicType())),
                         nl->TwoElemList(nl->SymbolAtom("TrjSeg"),
                                         nl->SymbolAtom(UPoint::BasicType())));
    ListExpr streamList = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                          tupleList));
    return streamList;
  }
  ErrorReporter::ReportError("seti x int expected!");
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
    set<TrjSeg*> hits;
    set<TrjSeg*>::iterator it;

    Iterator(set<TrjSeg*> HITS)
    {
      hits = HITS;
      it = hits.begin();
    }
    set<TrjSeg*>::iterator GetIterator() {return it;}
  };

  Iterator* iterator = static_cast<Iterator*>(local.addr);

  switch( message )
  {
    case OPEN:
   {
     SETI* setiPtr   = static_cast<SETI*>(args[0].addr);
     CcInt* movObjId = static_cast<CcInt*>(args[1].addr);
     int    moID = movObjId->GetValue();

     SETIArea area (setiPtr->GetHeader()->area.x1,
                    setiPtr->GetHeader()->area.y1,
                    setiPtr->GetHeader()->area.x2,
                    setiPtr->GetHeader()->area.y2);

     // Find all trajectory segments
     set<TrjSeg*> tempHits = FindTrjSegments( setiPtr, ModifyArea(area),
                                              setiPtr->GetHeader()->tiv.start,
                                              setiPtr->GetHeader()->tiv.end );

     // Filter all segments which belongs to the stated moving object id
     set<TrjSeg*>::iterator itTempHits;
     set<TrjSeg*> hits;
     for (itTempHits = tempHits.begin(); itTempHits != tempHits.end();
          itTempHits++)
     {
       TrjSeg* segPtr = *itTempHits;

       if ( moID == segPtr->moID )
       {
         hits.insert(segPtr);
       }
       else delete segPtr;
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
        TrjSeg* segment = *iterator->GetIterator();
        CcInt* moID = new CcInt(true, segment->moID);
        Interval<Instant>* tiv = new Interval<Instant>(
                                                DateTime(0,0,instanttype),
                                                DateTime(0,0,instanttype),
                                                true, true );
        tiv->start.ReadFrom(segment->tivStart);
        tiv->end.ReadFrom(segment->tivEnd);

        UPoint* upoint = new UPoint(*tiv, segment->pos1.x, segment->pos1.y,
                                          segment->pos2.x, segment->pos2.y );
        TupleType* tupType = new TupleType(nl->Second(GetTupleResultType(s)));
        Tuple*     tup = new Tuple( tupType );
        tup->PutAttribute( 0, ( Attribute* ) moID );
        tup->PutAttribute( 1, ( Attribute* ) upoint );
        result.addr = tup;
        delete tiv;
        delete segment;
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
    signature =  "seti x int  -> "
                 "stream (tuple (MovObjId int)(TrjSeg upoint))";
    syntax    =  "getTrajectory ( _, _ )";
    meaning   =  "Returns all trajectory segments which belongs"
                 "to the stated moving object id.";
  }
};

/******************************************************************************

11 currentUpload operator

Returns the current upload.

11.1 currentUpload - Type mapping method

******************************************************************************/

ListExpr CurrentUploadTM(ListExpr args)
{
  if( nl->ListLength( args ) == 2 &&
      nl->IsEqual(nl->First(args),SETI::BasicType()) &&
      nl->IsEqual(nl->Second(args),CcInt::BasicType()) )
  {
    return nl->SymbolAtom(UploadUnit::BasicType());
  }
  ErrorReporter::ReportError("seti x int expected!");
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

/******************************************************************************

11.2 currentUpload - Value mapping method

******************************************************************************/

int CurrentUploadVM(Word* args,Word& result,int message,Word& local,Supplier s)
{
  SETI* setiPtr = static_cast<SETI*>(args[0].addr);
  CcInt* moID   = static_cast<CcInt*>(args[1].addr);

  UploadUnit* unitPtr = static_cast<UploadUnit*>(qp->ResultStorage(s).addr);

  // Find UploadUnit in frontline
  int id = moID->GetValue();
  map<int,UploadUnit>::iterator it;
  it = setiPtr->frontline[id%flBuckets].find(id);
  if ( it == setiPtr->frontline[id%flBuckets].end() )
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
    signature =  "seti x int -> uploadunit";
    syntax    =  "currentUpload ( _, _ )";
    meaning   =  "Returns the current UploadUnit.";
  }
};

/******************************************************************************

12 ConvertMP2UUTM operator

Converts a stream of mpoints/moids into a stream of uploadunits.

12.1 convertMP2UUTM - Type mapping method

******************************************************************************/

bool isMPoint = true;

ListExpr ConvertMP2UUTM(ListExpr args)
{
  NList type(args);
  if ( !type.hasLength(4) )
  {
   return listutils::typeError("Expecting four arguments.");
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

  if ( !nl->IsEqual(nl->Second(args), CcInt::BasicType()) )
  {
    return NList::typeError( "int for second argument expected!" );
  }

  NList third = type.third();
  if ( !third.isSymbol() )
  {
    return NList::typeError( "Attribute name for third argument expected!" );
  }

  NList fourth = type.fourth();
  if ( !fourth.isSymbol() )
  {
    return NList::typeError( "Attribute name for fourth argument expected!" );
  }

  string attrname1 = type.third().str();
  ListExpr attrtype1 = nl->Empty();
  int j = FindAttribute(first.second().second().listExpr(),attrname1,attrtype1);

  string attrname2 = type.fourth().str();
  ListExpr attrtype2 = nl->Empty();
  int k = FindAttribute(first.second().second().listExpr(),attrname2,attrtype2);

  if ( j != 0 )
  {
    if ( nl->SymbolValue(attrtype1) != CcInt::BasicType() )
    {
      return NList::typeError("First attribute type is not of type int.");
    }
  }
  else
  {
    return NList::typeError("Unknown attribute name '" + attrname1 + "' !");
  }

  if ( k != 0 )
  {
    if ( nl->SymbolValue(attrtype2) != "mpoint" &&
         nl->SymbolValue(attrtype2) != UPoint::BasicType())
    {
      return NList::typeError("Second attribute is not of type"
                              " mpoint or upoint.");
    }

    if ( nl->SymbolValue(attrtype2) == UPoint::BasicType()) isMPoint = false;
    else isMPoint = true;
  }
  else
  {
    return NList::typeError("Unknown attribute name '" + attrname2 + "' !");
  }

  // Create output list
  ListExpr tupleList = nl->OneElemList(
                       nl->TwoElemList(nl->SymbolAtom("Upload"),
                                      nl->SymbolAtom(UploadUnit::BasicType())));
  ListExpr outputstream = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                          tupleList));

  return NList( NList(Symbol::APPEND()),
                nl->TwoElemList(nl->IntAtom(j), nl->IntAtom(k)),
                outputstream ).listExpr();
}

/******************************************************************************

12.2 convertMP2UUVM - Value mapping method

******************************************************************************/

int ConvertMP2UUVM(Word* args, Word& result, int message,
                  Word& local, Supplier s)
{
  struct Iterator
  {
    Iterator()
    {
      it  = 0;
      cnt = 0;
    }
    int it;
    int cnt;
    Word currentTupleWord;
  };

  Iterator* iterator = static_cast<Iterator*>(local.addr);

  switch( message )
  {
    case OPEN:
    {
      iterator = new Iterator();
      local.addr = iterator;
      qp->Open(args[0].addr);
      return 0;
    }
    case REQUEST:
    {
      int numUploads = static_cast<CcInt*>( args[1].addr )->GetIntval();
      int attr1 = static_cast<CcInt*>( args[4].addr )->GetIntval() - 1;
      int attr2 = static_cast<CcInt*>( args[5].addr )->GetIntval() - 1;

      bool received = true;

      if (iterator->it == 0)
      {
         qp->Request( args[0].addr, iterator->currentTupleWord );
         received = qp->Received(args[0].addr);
      }

      if ( received && iterator->cnt < numUploads )
      {
        Tuple* currentTuple = static_cast<Tuple*>
                              ( iterator->currentTupleWord.addr );
        int moID = static_cast<CcInt*>(currentTuple->GetAttribute(attr1))
                                       ->GetIntval();
        if (isMPoint)
        {
          MPoint* mp = static_cast<MPoint*>(currentTuple->GetAttribute(attr2));
          UPoint up;
          mp->Get(iterator->it,up);
          UnitPos pos( up.p1.GetX(), up.p1.GetY() );
          UploadUnit* uu = new UploadUnit(moID, up.timeInterval.end, pos );
          TupleType* tupType =new TupleType(nl->Second(GetTupleResultType(s)));
          Tuple* tup = new Tuple( tupType );
          tup->PutAttribute( 0, ( Attribute* ) uu );
          result.addr = tup;
          iterator->it++;
          iterator->cnt++;
          if (iterator->it == mp->GetNoComponents()) iterator->it = 0;
        }
        else
        {
          UPoint* up = static_cast<UPoint*>(currentTuple->GetAttribute(attr2));
          UnitPos pos( up->p1.GetX(), up->p1.GetY() );
          UploadUnit* uu = new UploadUnit(moID, up->timeInterval.end, pos );
          TupleType* tupType =new TupleType(nl->Second(GetTupleResultType(s)));
          Tuple* tup = new Tuple( tupType );
          tup->PutAttribute( 0, ( Attribute* ) uu );
          result.addr = tup;
          iterator->cnt++;
        }
        return YIELD;
      }
      else
      {
        delete iterator;
        local.addr  = 0;
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE:
    {
      return 0;
    }
    default:
    {
      return -1;
    }
  }
}

/******************************************************************************

12.3 convertMP2UU - Specification of operator

******************************************************************************/

struct ConvertMP2UUInfo : OperatorInfo {
  ConvertMP2UUInfo()
  {
    name      = "convertMP2UU";
    signature = "((stream (tuple([a1:d1, ..., ai:int, ..., "
                "aj:mpoint|upoint, ..., an:dn]))) x int x ai x aj)"
                " -> stream (tuple (Upload uploadunit))";
    syntax    = "_ convertMP2UU [ _, _, _ ]";
    meaning   = "Converts mpoints into upload units.";
  }
};

/******************************************************************************

13 Type constructor of class UploadUnit

******************************************************************************/

TypeConstructor UploadUnitTC(
  UploadUnit::BasicType(),                // name
  UploadUnit::Property,                   // property function
  UploadUnit::Out, UploadUnit::In,        // Out and In functions
  0, 0,                                   // SaveTo and RestoreFrom functions
  UploadUnit::Create, UploadUnit::Delete, // object creation and deletion
  UploadUnit::Open,   UploadUnit::Save,   // object open and save
  UploadUnit::Close,  UploadUnit::Clone,  // object close and clone
  UploadUnit::Cast,                       // cast function
  UploadUnit::SizeOfObj,                  // sizeof function
  UploadUnit::KindCheck );                // kind checking function

/******************************************************************************

14 Declaration of SETIAlgebra

******************************************************************************/

class SETIAlgebra : public Algebra
{
  public:
    SETIAlgebra() : Algebra()
    {
      AddTypeConstructor(&UploadUnitTC);
      AddTypeConstructor(&SETITC);
      UploadUnitTC.AssociateKind( Kind::DATA() );

      AddOperator( CreateInfo(),        CreateVM,        CreateTM );
      AddOperator( InsertUploadInfo(),  InsertUploadVM,  InsertUploadTM );
      AddOperator( InsertStreamInfo(),  InsertStreamVM,  InsertStreamTM );
      AddOperator( IntersectsWinInfo(), IntersectsWinVM, IntersectsWinTM );
      AddOperator( InsideWinInfo(),     InsideWinVM,     InsideWinTM );
      AddOperator( CurrentUploadInfo(), CurrentUploadVM, CurrentUploadTM );
      AddOperator( GetTrajectoryInfo(), GetTrajectoryVM, GetTrajectoryTM );
      AddOperator( ConvertMP2UUInfo(),  ConvertMP2UUVM,  ConvertMP2UUTM );
    }
    ~SETIAlgebra() {};
};

/******************************************************************************

15 Initialization of SETIAlgebra

******************************************************************************/

extern "C"
Algebra*
InitializeSETIAlgebra( NestedList *nlRef,
                       QueryProcessor *qpRef,
                       AlgebraManager* amRef )
{
  nl = nlRef;
  qp = qpRef;
  am = amRef;
  return (new SETIAlgebra());
}
} // End of SETIAlgebra namespace
