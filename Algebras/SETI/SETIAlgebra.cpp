/******************************************************************************
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
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

May 2010, Daniel Brockmann

1 Overview

SETI-Algebra implements a SETI index structure. The implementation makes
use of the existing SECONDO component RTree-Algebra. The memory management
is based on SmiUpdateFile. In addition the UploadUnit is an object type of
this Algebra.

SETI-Algebra offers the following methods:

- setiCreate            -> Creates a new SETI object.
- setiInsertUnit        -> Inserts a single UploadUnit into SETI.
- setiInsertStream      -> Inserts a stream of UploadUnits into SETI.
- setiIntersectsWindow  -> Returns all moving objects (Id) which trajectory
                           intersects the search window.
- setiInsideWindow      -> Returns all moving objects (Id) which trajectory
                           is inside the search window.
- setiCurrentUpload     -> Returns the current UploadUnit.
 

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


using namespace std;
using namespace symbols;

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

namespace SETIAlgebra {

#include "SETIAlgebra.h"

/******************************************************************************

3. Auxiliary functions

3.1 CompareFloats

Returns 'true' if the two floating-point numbers are almost equal.

******************************************************************************/

bool CompareFloats(double f1 ,double f2)
{
  if ( f1 <= (f2 + 0.001) &&
       f1 >= (f2 - 0.001) )
  {
    return true;
  }
  else return false;
}

/******************************************************************************

4.2 ModifyArea

Modifies the given area. x1/x2 and y1/y2 will be swapped if necessary.

******************************************************************************/

SETIArea ModifyArea(SETIArea area)
{
  double tempX = 0;
  double tempY = 0;
  
  if (area.x1 > area.x2)
  {
    tempX   = area.x2;
    area.x2 = area.x1;
    area.x1 = tempX;
  }
  if (area.y1 > area.y2)
  {
    tempY   = area.y2;
    area.y2 = area.y1;
    area.y1 = tempY;
  }
  area.x1 = area.x1 - tol;
  area.y1 = area.y1 - tol;
  area.x2 = area.x2 + tol;
  area.y2 = area.y2 + tol;
  return area;
}

/******************************************************************************

3.3 ComputeLine

Identifies a column or a row in a grid in dependence of a given position.

******************************************************************************/

int ComputeLine(double border1 ,double border2, double pos)
{
  double len = abs(border1-border2) / splits;
  int i = 0;

  while ((border1 + (i*len)) <= pos) i++;
  
  return i-1;
}

/******************************************************************************

4.4 ComputeIntersection

Computes the intersection point of two lines.

******************************************************************************/

bool ComputeIntersection( double Ax, double Ay,
                          double Bx, double By,
                          double Cx, double Cy,
                          double Dx, double Dy,
                          UnitPos* is )
{

  double  distAB, theCos, theSin, newX, ABpos ;

  if (Ax==Bx && Ay==By || Cx==Dx && Cy==Dy) return false;

  if (Ax==Cx && Ay==Cy || Bx==Cx && By==Cy
  ||  Ax==Dx && Ay==Dy || Bx==Dx && By==Dy) return false;

  Bx-=Ax; By-=Ay;
  Cx-=Ax; Cy-=Ay;
  Dx-=Ax; Dy-=Ay;

  distAB=sqrt(Bx*Bx+By*By);

  theCos=Bx/distAB;
  theSin=By/distAB;
  newX=Cx*theCos+Cy*theSin;
  Cy  =Cy*theCos-Cx*theSin; Cx=newX;
  newX=Dx*theCos+Dy*theSin;
  Dy  =Dy*theCos-Dx*theSin; Dx=newX;

  if (Cy<0. && Dy<0. || Cy>=0. && Dy>=0.) return false;

  ABpos=Dx+(Cx-Dx)*Dy/(Dy-Cy);

  if (ABpos<0. || ABpos>distAB) return false;

  is->x = Ax+ABpos*theCos;
  is->y = Ay+ABpos*theSin;
  
  return true;
}

/******************************************************************************

4 Implementation of class SETI

4.1 Basic constructor

******************************************************************************/

SETI::SETI(SETIArea AREA) : suf(0)
{   
  // Create SETI Header
  header = new SETIHeader(ModifyArea(AREA));

  // Create SmiUpdateFile
  suf = new SmiUpdateFile(pageSize);
  suf->Create();
  header->fileID = suf->GetFileId();
  
  // Create header page
  SmiUpdatePage* headerPage;
  int AppendedPage = suf->AppendNewPage(headerPage);
  assert( AppendedPage );
  header->headerPageNo = headerPage->GetPageNo();
  
   // Create frontline pages
  SmiUpdatePage* flPage;
  db_pgno_t nextPageNo    = 0;
  int       numEntries  = 0;
  for(int i = 0; i < flBuckets; i++)
  {
    AppendedPage = suf->AppendNewPage(flPage);
    assert( AppendedPage );
    header->flPageNo[i] = flPage->GetPageNo();
    int PageSelected = suf->GetPage(header->flPageNo[i], flPage);
    assert( PageSelected );
    flPage->Write(&nextPageNo, sizeof(db_pgno_t), 0);
    flPage->Write(&numEntries, sizeof(int), sizeof(db_pgno_t));
  }

  // Create cells page
  SmiUpdatePage* cellsPage;
  AppendedPage = suf->AppendNewPage(cellsPage);
  assert( AppendedPage );
  header->cellsPageNo = cellsPage->GetPageNo();
  
  // Calculate x/y cell length
  double areaLenX = abs(AREA.x2 - AREA.x1); 
  double areaLenY = abs(AREA.y2 - AREA.y1);
  double cellLenX = areaLenX / splits; 
  double cellLenY = areaLenY / splits;
  
  for (int i = 0; i < splits; i++)       
  {
    for (int j = 0; j < splits; j++)    
    {
      // Create an area partition
      SETIArea partition(0,0,0,0);
      partition.x1 = AREA.x1 + (cellLenX * j);
      partition.x2 = partition.x1 + cellLenX;
      partition.y1 = AREA.y1 + (cellLenY * i); 
      partition.y2 = partition.y1 + cellLenY;

      // Create and initialize a new cell object
      cells[j][i] = new SETICell();
      cells[j][i]->numSegments = 0;
      cells[j][i]->area = partition;
      cells[j][i]->rtreePtr = new R_Tree<2,TupleId>(4000);
      cells[j][i]->rtreeFileID = cells[j][i]->rtreePtr->FileId();
      cells[j][i]->currentPage = (db_pgno_t)0;
      cells[j][i]->tiv.start = DateTime(0,0,instanttype);
      cells[j][i]->tiv.end = DateTime(0,0,instanttype);
      cells[j][i]->tiv.lc = false;
      cells[j][i]->tiv.rc = false;
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
  // Open extisting file
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
  for (int i = 0; i < splits; i++)
  {
    for (int j = 0; j < splits; j++)
    {
      delete cells[j][i]->rtreePtr;
      delete cells[j][i];
      cells[j][i] = 0;
    }
  }
  // Delete header
  delete header;
  header = 0;
  // Delete file object
  delete suf;
  suf = 0;
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
  if ( nl->ListLength(instance) != 4 )
  {
    cmsg.inFunError("A list of four real type values expected!");
    return result;
  }
  // Check values
  if ( !(nl->IsAtom( nl->First( instance ) ) &&
         nl->AtomType( nl->First( instance) ) == RealType &&
         nl->IsAtom( nl->Second( instance ) ) &&
         nl->AtomType( nl->Second( instance ) ) == RealType &&
         nl->IsAtom( nl->Third( instance ) ) &&
         nl->AtomType( nl->Third( instance) ) == RealType &&
         nl->IsAtom( nl->Fourth( instance ) ) &&
         nl->AtomType( nl->Fourth( instance ) ) == RealType ))
  {
    cmsg.inFunError("A list of four real type values expected!");
    return result;
  }
    
  double x1 = nl->RealValue( nl->First(instance) );
  double x2 = nl->RealValue( nl->Second(instance) );
  double y1 = nl->RealValue( nl->Third(instance) );
  double y2 = nl->RealValue( nl->Fourth(instance) );
  
  if (( x2 == x1 ) || (y1 == y2 ))
  {
    cmsg.inFunError("x1/x2 and y1/y2 must be different!");
    return result;
  }
  // Create new SETI
  SETIArea area(x1,y1,x2,y2);
  result.addr = new SETI(ModifyArea(area));
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
  for (int i = 0; i < (splits); i++)
  {
    for (int j = 0; j < (splits); j++)
    {
       // Delete RTree file
       setiPtr->cells[j][i]->rtreePtr->DeleteFile();
    }
  }
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
  return (nl->IsEqual(type, "seti"));
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
                             nl->StringAtom("seti"),
                             nl->StringAtom("(<x1> <x2> <y1> <y2>)"),
                             nl->StringAtom("(8.2 1.6 9.7 4,6)"),
                             nl->StringAtom("x/y must be of type double."))));
}

/******************************************************************************

4.5 ReadSETI-method

Reads header, frontline and cells information from file.

******************************************************************************/

void SETI::ReadSETI()
{
  // Read header
  SmiUpdatePage* headerPage;
  int PageSelected = suf->GetPage(header->headerPageNo, headerPage);
  assert( PageSelected );
 
  size_t offset = 0;
  headerPage->Read(&header->fileID, sizeof(SmiFileId), offset);
  offset += sizeof(SmiFileId);
  headerPage->Read(&header->headerPageNo, sizeof(db_pgno_t), offset);
  offset += sizeof(db_pgno_t);
  for (int i = 0; i < flBuckets; i++)
  {
    headerPage->Read(&header->flPageNo[i], sizeof(db_pgno_t), offset);
    offset += sizeof(db_pgno_t);
  }
  headerPage->Read(&header->cellsPageNo, sizeof(db_pgno_t), offset);
  offset += sizeof(db_pgno_t);
  headerPage->Read(&header->area, sizeof(SETIArea), offset);
  offset += sizeof(SETIArea);
  headerPage->Read(&header->numCells, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Read(&header->numEntries, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Read(&header->numFlEntries, sizeof(int), offset);
  offset += sizeof(int);
  headerPage->Read(&header->tiv, sizeof(Interval<Instant>), offset);
  offset += sizeof(Interval<Instant>);
  
  // Read frontline
  SmiUpdatePage* flPage;
  db_pgno_t      nextPageNo;
  db_pgno_t      currentPageNo;
  int            numEntries;

  for (int i = 0; i < flBuckets; i++)
  {
    currentPageNo = header->flPageNo[i];
    do
    {
      PageSelected = suf->GetPage(currentPageNo, flPage);
      assert( PageSelected );
      flPage->Read(&nextPageNo, sizeof(db_pgno_t), 0);
      flPage->Read(&numEntries, sizeof(int), sizeof(db_pgno_t));
      offset = sizeof(db_pgno_t) + sizeof(int);
      for (int j = 0; j < numEntries; j++)
      { 
        UploadUnit  unit;
        flPage->Read(&unit, sizeof(UploadUnit), offset);
        offset += sizeof(UploadUnit);
        frontline[unit.GetID()%flBuckets].insert(make_pair(unit.GetID(),unit));
      }
    }
    while(nextPageNo != (db_pgno_t)0);
  }
  
  // Read cells
  SmiUpdatePage* cellsPage;
  PageSelected = suf->GetPage(header->cellsPageNo, cellsPage);
  assert( PageSelected );

  int j  = 1;
  int i  = 1;
  offset = 0;
  for (int k = 0; k < header->numCells; k++)
  {
    cells[j-1][i-1] = new SETICell();
    SETICell* cellPtr = cells[j-1][i-1];
    cellsPage->Read(&cellPtr->numSegments, sizeof(int), offset);
    offset += sizeof(int);
    cellsPage->Read(&cellPtr->area, sizeof(SETIArea), offset);
    offset += sizeof(SETIArea);
    cellsPage->Read(&cellPtr->rtreeFileID, sizeof(SmiFileId), offset);
    offset += sizeof(SmiFileId);
    cellsPage->Read(&cellPtr->currentPage, sizeof(db_pgno_t), offset);
    offset += sizeof(db_pgno_t);
    cellsPage->Read(&cellPtr->tiv, sizeof(Interval<Instant>), offset);
    offset += sizeof(Interval<Instant>);
    
    // Create new RTree with existing RTree file
    cellPtr->rtreePtr = new R_Tree<2,TupleId>(cellPtr->rtreeFileID);
    
    if ((j % splits) == 0) { j = 0; i++; }
    j++;
  }
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
  headerPage->Write(&header->fileID, sizeof(SmiFileId), offset);
  offset += sizeof(SmiFileId);
  headerPage->Write(&header->headerPageNo, sizeof(db_pgno_t), offset);
  offset += sizeof(db_pgno_t);
  for(int i = 0; i < flBuckets; i++)
  { 
    headerPage->Write(&header->flPageNo[i], sizeof(db_pgno_t), offset);
    offset += sizeof(db_pgno_t);
  }
  headerPage->Write(&header->cellsPageNo, sizeof(db_pgno_t), offset);
  offset += sizeof(db_pgno_t);
  headerPage->Write(&header->area, sizeof(SETIArea), offset);
  offset += sizeof(SETIArea);
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

void SETI::UpdateFLine(int BUCKET)
{
  SmiUpdatePage* flPage;
  db_pgno_t      nextPageNo;
  int numEntries = 0;
  size_t offset = sizeof(db_pgno_t) + sizeof(int);
  int PageSelected = suf->GetPage(header->flPageNo[BUCKET],
                                           flPage);
  assert( PageSelected );
  for (map<int,UploadUnit>::iterator it = frontline[BUCKET].begin();
       it != frontline[BUCKET].end(); it++)
  {
    numEntries++;
    flPage->Write( &numEntries, sizeof(int), sizeof(db_pgno_t) );
    flPage->Write( &it->second, sizeof(UploadUnit), offset );
    offset += sizeof(UploadUnit);
    if((numEntries%70) == 0)
    {
      // Page is full up
      numEntries = 0;
      // Get next page
      flPage->Read(&nextPageNo, sizeof(db_pgno_t), 0);
      if(nextPageNo == 0)
      {
        // No next page exist -> create new next page
        SmiUpdatePage* newPage;
        int AppendedPage = suf->AppendNewPage(newPage);
        assert( AppendedPage );
        nextPageNo = newPage->GetPageNo();
        flPage->Write(&nextPageNo, sizeof(db_pgno_t), 0);
      }
      PageSelected = suf->GetPage(nextPageNo, flPage);
      assert( PageSelected );
    }
  }
}

/******************************************************************************

4.8 UpdateCells-method

Writes cell information into file.

******************************************************************************/

void SETI::UpdateCells()
{
  SmiUpdatePage* cellsPage;
  int PageSelected = suf->GetPage(header->cellsPageNo, cellsPage);
  assert( PageSelected );
 
  size_t offset = 0;
  for (int i = 0; i < splits; i++)
  {
    for (int j = 0; j < splits; j++)
    { 
      SETICell* cellPtr = cells[j][i];
      cellsPage->Write(&cellPtr->numSegments, sizeof(int), offset);
      offset += sizeof(int);
      cellsPage->Write(&cellPtr->area, sizeof(SETIArea), offset);
      offset += sizeof(SETIArea);
      cellsPage->Write(&cellPtr->rtreeFileID, sizeof(SmiFileId), offset);
      offset += sizeof(SmiFileId);
      cellsPage->Write(&cellPtr->currentPage, sizeof(db_pgno_t), offset);
      offset += sizeof(db_pgno_t);
      cellsPage->Write(&cellPtr->tiv, sizeof(Interval<Instant>), offset);
      offset += sizeof(Interval<Instant>);
    }
  }
}

/******************************************************************************

4.9 GetUpdateFile-method

Returns the SETI file id.

******************************************************************************/

SmiUpdateFile* SETI::GetUpdateFile()
{
 return suf;
}

/******************************************************************************

4.10 GetHeader-method

Returns the pointer to the SETI header.

******************************************************************************/

SETIHeader* SETI::GetHeader()
{
 return header;
}

/******************************************************************************

4.11 GetCell-method

Returns the pointer to the stated cell.

******************************************************************************/

SETICell* SETI::GetCell(int COL, int ROW)
{
 return cells[COL][ROW];
}

/******************************************************************************

4.12 GetSemaphore-method

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

4.13 SetSemaphore-method

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

4.14 Type constructor

******************************************************************************/

TypeConstructor SETITC(
       "seti",                        //name
        SETI::Property,               //property function
        SETI::Out,   SETI::In,        //Out and In functions
        0, 0,                         //SaveTo and RestoreFrom functions
        SETI::Create,  SETI::Delete,  //object creation and deletion
        SETI::Open,    SETI::Save,    //object open and save
        SETI::Close,   SETI::Clone,   //object close and clone
        SETI::Cast,                   //cast function
        SETI::SizeOfObj,              //sizeof function
        SETI::KindCheck );            //kind checking function

/******************************************************************************

5 setiCreate operator

Creates a new SETI object.

5.1 setiCreate - Type mapping method

******************************************************************************/

ListExpr CreateTM(ListExpr args)
{  
  if( nl->ListLength( args ) == 1 &&
      nl->IsEqual(nl->First(args),"rect") )
  {
    return (nl->SymbolAtom("seti"));
  }
  ErrorReporter::ReportError("rect expected!");
  return (nl->SymbolAtom( "typeerror" ));
}

/******************************************************************************

5.2 setiCreate - Value mapping method

******************************************************************************/

int CreateVM(Word* args, Word& result, int message, Word& local, Supplier s)
{
  SETI* setiPtr = static_cast<SETI*>(qp->ResultStorage(s).addr);
  
  Rectangle<2>* rect = static_cast<Rectangle<2>*>(args[0].addr);
  
  // Create SETI area
  double x1(rect->MinD(0));
  double x2(rect->MaxD(0));
  double y1(rect->MinD(1));
  double y2(rect->MaxD(1));
  SETIArea area(x1,y1,x2,y2);
  area = ModifyArea(area);
  
  // Create SETI object
  setiPtr = new SETI(area);
  result.setAddr( setiPtr );
  return 0;
}

/******************************************************************************

5.3 setiCreate - Specification of operator

******************************************************************************/

struct CreateInfo : OperatorInfo {
  CreateInfo()
  {
    name      = "setiCreate";
    signature = "rect -> seti";
    syntax    = "setiCreate( _ )";
    meaning   = "SETI construct operator.";
    example   = "let mySeti = setiCreate( myRect )";
  }
};

/******************************************************************************

6 setiInsertUnit operator

Inserts a single UploadUnit into SETI.

6.1 ComputeTrjSegment-method

Computes a trajectory segment and returns it to the insertHandle method.

******************************************************************************/

TrjSeg* ComputeTrjSegment( SETIArea AREA,
                           UnitPos POS1, UnitPos POS2,
                           Interval<Instant> TIV )
{
  int    startCol  = ComputeLine(AREA.x1, AREA.x2, POS1.x);
  int    startRow  = ComputeLine(AREA.y1, AREA.y2, POS1.y);
  double xLen      = abs(AREA.x1-AREA.x2) / splits;
  double yLen      = abs(AREA.y1-AREA.y2) / splits;
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

Inserts an UploadUnit into the SETI index structure. The setiInsertUnit and
the setiInsertStream operator make use of this method.

******************************************************************************/

bool InsertHandle(SETI* SETIPTR, UploadUnit* UNITPTR, bool BULKLOAD)
{
  SmiUpdateFile* suf  = SETIPTR->GetUpdateFile();
  SETIHeader*    hPtr = SETIPTR->GetHeader();
  

  // Ceck UploadUnit
  if (hPtr->area.x1 < hPtr->area.x2)
  {
    if (UNITPTR->GetPos().x < hPtr->area.x1+tol ||
        UNITPTR->GetPos().x > hPtr->area.x2-tol )
       return false;
  }
  else if (UNITPTR->GetPos().x < hPtr->area.x2+tol ||
           UNITPTR->GetPos().x > hPtr->area.x1-tol )
       return false;
  
  if (hPtr->area.y1 < hPtr->area.y2)
  {
    if (UNITPTR->GetPos().y < hPtr->area.y1+tol ||
        UNITPTR->GetPos().y > hPtr->area.y2-tol ) 
    return false;
  }
  else if (UNITPTR->GetPos().y < hPtr->area.y2+tol ||
           UNITPTR->GetPos().y > hPtr->area.y1-tol )
    return false;
 
  // Check if SmiUpdateFile is released
  if(SETIPTR->GetSemaphore()) return false;
   
  // Lock SmiUpdateFile
  SETIPTR->SetSemaphore(true);

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
    
    // Replace UploadUnit in frontline
    SETIPTR->frontline[moID%flBuckets][moID] = *UNITPTR;
    
    // Insert trajectory segment
    do 
    {
      // Create/split new segment
      segment =ComputeTrjSegment(hPtr->area,pos1, pos2, segTiv);
      segment->moID   = moID;
      segment->segID  = hPtr->numEntries;
       
      // Find cell for segment
      int col =ComputeLine(hPtr->area.x1,
                           hPtr->area.x2,pos1.x);
      int row =ComputeLine(hPtr->area.y1,
                           hPtr->area.y2,pos1.y);
      SETICell* cellPtr = SETIPTR->GetCell(col, row);
      
      // Check/modify SETI time interval
      if (!hPtr->tiv.lc ||
           (hPtr->tiv.start > segTiv.start))
      {
        hPtr->tiv.start = segTiv.start;
        hPtr->tiv.lc = true;
      }
      
      if ((!hPtr->tiv.rc) ||
           (hPtr->tiv.end < segTiv.end))
      {
        hPtr->tiv.end = segTiv.end;
        hPtr->tiv.rc = true;
      }
      
      // Check/modify cell time interval
      if ((!cellPtr->tiv.lc) ||
           (cellPtr->tiv.start > segTiv.start))
      {
        cellPtr->tiv.start = segTiv.start;
        cellPtr->tiv.lc = true;
      }
      
      if ((!cellPtr->tiv.rc) ||
           (cellPtr->tiv.end < segTiv.end))
      {
        cellPtr->tiv.end = segTiv.end;
        cellPtr->tiv.rc = true;
      }
     
      SmiUpdatePage* page;
      if ((cellPtr->numSegments % maxTrjSeg) == 0)
      {
        if(cellPtr->currentPage != 0)
        {
          // Insert temporal entry into the RTree
          Interval<Instant> pageTiv;
          int PageSelected = suf->GetPage(cellPtr->currentPage, page);
          assert( PageSelected );
          page->Read(&pageTiv, sizeof(Interval<Instant> ), sizeof(int)); 
          
          Rectangle<2> box = Rectangle<2>(true, pageTiv.start.ToDouble(), 
                                          pageTiv.end.ToDouble(), -1.0, 1.0);
          R_TreeLeafEntry<2,TupleId> e(box,cellPtr->currentPage);
          if(BULKLOAD) cellPtr->rtreePtr->InsertBulkLoad(e);
          else cellPtr->rtreePtr->Insert(e);
          
          
          // Put current page back from memory to file
          suf->PutPage(cellPtr->currentPage, true);
        }
        // Create new page if no page exist or current page is full up
        int AppendedPage = suf->AppendNewPage(page);
        assert( AppendedPage );
        cellPtr->currentPage = page->GetPageNo();
      }
      int PageSelected = suf->GetPage(cellPtr->currentPage, page);
      assert( PageSelected );
      
      // Initialize tiv and number of segments in page
      if ((cellPtr->numSegments % maxTrjSeg) == 0)
      {
        int numSegments = 0;
        page->Write(&numSegments, sizeof(int), 0);
        page->Write(&segTiv,sizeof(Interval<Instant>), sizeof(int));
      }
       
      // Check/modify page time interval
      Interval<Instant> pageTiv;
      page->Read(&pageTiv, sizeof(Interval<Instant> ), sizeof(int));
      if (segTiv.start < pageTiv.start) pageTiv.start = segTiv.start;
      if (segTiv.end > pageTiv.end) pageTiv.end = segTiv.end;
      
      // Read number of segments in page
      int numSegments;
      page->Read( &numSegments, sizeof(int), 0 );
     
      // Write segment in page
      size_t offset =  sizeof(int) + sizeof(Interval<Instant>) + 
                      + ((numSegments)*(sizeof(TrjSeg)));
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
      
      // Increase number of segments in cell and page
      cellPtr->numSegments++;
      numSegments++;
      page->Write(&numSegments, sizeof(int), 0);
       
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
  
  // Update header in SmiUpdateFile
  SETIPTR->UpdateHeader();
   
  // Update front line in SmiUpdateFile
  SETIPTR->UpdateFLine(moID%flBuckets);

  // Update cells
  SETIPTR->UpdateCells();

  // Release SmiUpdateFile
  SETIPTR->SetSemaphore(false);
  
  return true;
}

/******************************************************************************

6.3 setiInsertUnit - Type mapping method

******************************************************************************/


ListExpr InsertUnitTM(ListExpr args)
{  
  if ( nl->ListLength( args ) == 2 &&
       nl->IsEqual(nl->First(args), "seti") &&
       nl->IsEqual(nl->Second(args), "uploadunit"))
  {
    return (nl->SymbolAtom("bool"));
  }
  return listutils::typeError("Expected seti x uploadunit!");
}

/******************************************************************************

6.4 setiInsertUnit - Value mapping method

******************************************************************************/

int InsertUnitVM(Word* args, Word& result, int message,
                  Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  CcBool* b = static_cast<CcBool*>(result.addr);
  
  SETI*       setiPtr = static_cast<SETI*>(args[0].addr);
  UploadUnit* unitPtr = static_cast<UploadUnit*>(args[1].addr);

  // Insert UploadUnit
  bool res = InsertHandle(setiPtr, unitPtr, false);
  b->Set(true,res);
  return 0;
}

/******************************************************************************

6.5 setiInsertUnit - Specification of operator

******************************************************************************/

struct InsertUnitInfo : OperatorInfo {
  InsertUnitInfo()
  {
    name      = "setiInsertUnit";
    signature = "seti x uploadunit -> bool";
    syntax    = "setiInsertUnit( _, _)";
    meaning   = "SETI insert UploadUnit operator.";
    example   = "query setiInsertUnit(mySeti, myUploadUnit)";
  }
};

/******************************************************************************

7 setiInsertStream operator

Inserts a stream of UploadUnits into SETI.

7.1 setiInsertStream - Type mapping method

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
       !first.first().isSymbol(STREAM) ||
       !first.second().hasLength(2) ||
       !first.second().first().isSymbol(TUPLE) ||
       !IsTupleDescription( first.second().second().listExpr() ))
  {
    return listutils::typeError("Error in first argument!");
  }

  if ( !nl->IsEqual(nl->Second(args), "seti") )
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
    if ( nl->SymbolValue(attrtype) != "uploadunit" )
    {
      return NList::typeError("Attribute type is not of type uploadunit.");
    }
    NList resType = NList(BOOL);
    return NList( NList(APPEND), NList(j).enclose(), resType ).listExpr();
  }
  else
  {
    return NList::typeError( "Attribute name '" + attrname +"' is not known!");
  }
}

/******************************************************************************

7.2 setiInsertStream - Value mapping method

******************************************************************************/

int InsertStreamVM(Word* args, Word& result, int message,
                  Word& local, Supplier s)
{  
  Word currentTupleWord(Address(0));
  int attributeIndex = static_cast<CcInt*>( args[3].addr )->GetIntval() - 1;
  SETI*      setiPtr = static_cast<SETI*>(args[1].addr);
  bool       res = true;

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, currentTupleWord);
  
  // Initialize RTrees for bulkoad
  for (int i = 0; i < splits; i++)
  {
    for (int j = 0; j < splits; j++)
    {
      bool BulkLoadInitialized = setiPtr->GetCell(j,i)->rtreePtr->
                                 InitializeBulkLoad();
      assert(BulkLoadInitialized);
    }
  }
  
  while(qp->Received(args[0].addr) && (res == true))
  {
    Tuple* currentTuple = static_cast<Tuple*>( currentTupleWord.addr );
    UploadUnit* currentAttr = 
    static_cast<UploadUnit*>(currentTuple->GetAttribute(attributeIndex));
    if( currentAttr->IsDefined() )
    {
      res = InsertHandle(setiPtr, currentAttr, true);
    }
    currentTuple->DeleteIfAllowed();
    qp->Request(args[0].addr, currentTupleWord);
  }
  
  // Finalize bulkoad for RTrees
  for (int i = 0; i < splits; i++)
  {
    for (int j = 0; j < splits; j++)
    {
      int FinalizedBulkLoad = setiPtr->GetCell(j,i)->rtreePtr->
                              FinalizeBulkLoad();
      assert( FinalizedBulkLoad );
    }
  }
  
  qp->Close(args[0].addr);
  result = qp->ResultStorage(s);
  static_cast<CcBool*>( result.addr )->Set(true, res);
  return 0;
}

/******************************************************************************

7.3 setiInsertStream - Specification of operator

******************************************************************************/

struct InsertStreamInfo : OperatorInfo {
  InsertStreamInfo()
  {
    name      = "setiInsertStream";
    signature = "((stream (tuple([a1:d1, ..., ai:int, "
                "..., an:dn]))) x seti x ai) -> bool";
    syntax    = "_ setiInsertStream[ _, _ ]";
    meaning   = "SETI insert UploadUnit stream operator.";
    example   = "query myRel feed setiInsertUnit[mySeti, upload]";
  }
};


/******************************************************************************

8 setiIntersectsWindow operator

Returns all moving objects (Id) which trajectory intersect the search window.

8.1 FindTrjSegments-method

Finds all trajectory segments which intersects the search window. The 
setiIntersectsWindow and the setiInsideWindow operator make use of this method.

******************************************************************************/

set<TrjSeg*> FindTrjSegments( SETI* SETIPTR, SETIArea AREA, 
                              Instant TIME1, Instant TIME2)
{
  SETIHeader* hPtr = SETIPTR->GetHeader();
  set<db_pgno_t> pages;
  set<db_pgno_t>::iterator pIt;
  set<TrjSeg*> segments;
  set<TrjSeg*>::iterator sIt;
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
      return hits; // Set is empty
      
  if ( AREA.x1<hPtr->area.x1) AREA.x1 =hPtr->area.x1+tol;
  if ( AREA.x2>hPtr->area.x2) AREA.x2 =hPtr->area.x2-tol;
  if ( AREA.y1<hPtr->area.y1) AREA.y1 =hPtr->area.y1+tol;
  if ( AREA.y2>hPtr->area.y2) AREA.y2 =hPtr->area.y2-tol;
  
  // Find pages
  int left   = ComputeLine(hPtr->area.x1,
               hPtr->area.x2,AREA.x1);
  int right  = ComputeLine(hPtr->area.x1,
               hPtr->area.x2,AREA.x2);
  int bottom = ComputeLine(hPtr->area.y1,
               hPtr->area.y2,AREA.y1);
  int top    = ComputeLine(hPtr->area.y1,
               hPtr->area.y2,AREA.y2);	
  	  
  Rectangle<2> searchBox = Rectangle<2>(true,TIME1.ToDouble(),TIME2.ToDouble(),
                                        -1.0, 1.0 );
  
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
  
  // Find  segments in pages
  db_pgno_t pageID;
  SmiRecord   pageRec;
  int moID;
  int segID;
  double tivStart;
  double tivEnd;
  UnitPos pos1;
  UnitPos pos2;

  for (pIt = pages.begin(); pIt != pages.end(); pIt++)
  {    
    pageID = *pIt;
    SmiUpdatePage* page;
    int PageSelected = SETIPTR->GetUpdateFile()->GetPage(pageID, page);
    assert( PageSelected );
    
    // Read number of segments in Page
    int numSegments;
    page->Read( &numSegments, sizeof(int), 0 );
    size_t offset = sizeof(int) + sizeof(Interval<Instant>);
    
    // Read segment
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
      for (sIt = segments.begin(); sIt != segments.end(); sIt++)
      {
        TrjSeg* segPtr = *sIt;
        if ( segPtr->moID == moID &&
             segPtr->segID == segID  )
        {
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
      }
      if (!found) segments.insert(new TrjSeg( moID, segID, tivStart,
                                              tivEnd, pos1, pos2 ));
    }
  }

  // Check if segment intersects window
  for (sIt = segments.begin(); sIt != segments.end(); sIt++)
  {
    TrjSeg* segPtr = *sIt;
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
  }
  pages.clear();
   
  // Release SmiUpdateFile
  SETIPTR->SetSemaphore(false);
  
  return hits;
}

/******************************************************************************

8.2 setiIntersectsWindow - Type mapping method

******************************************************************************/

ListExpr IntersectsWindowTM(ListExpr args)
{
  if( nl->ListLength( args ) == 4 &&
      nl->IsEqual(nl->First(args),"seti") &&
      nl->IsEqual(nl->Second(args),"rect") &&
      nl->IsEqual(nl->Third(args),"instant") &&
      nl->IsEqual(nl->Fourth(args),"instant"))
  {
    ListExpr tupleList = nl->OneElemList(
                         nl->TwoElemList(nl->SymbolAtom("MovObjId"),
                                         nl->SymbolAtom("int")));
    ListExpr streamList = nl->TwoElemList(nl->SymbolAtom("stream"),
                          nl->TwoElemList(nl->SymbolAtom("tuple"),tupleList));
    return streamList;
  }
  ErrorReporter::ReportError("seti x rect x instant x instant expected!");
  return (nl->SymbolAtom( "typeerror" ));
}

/******************************************************************************

8.3 setiIntersectsWindow - Value mapping method

******************************************************************************/

int IntersectsWindowVM(Word* args, Word& result, int message,
                       Word& local, Supplier s)
{


  struct Iterator 
  {
    set<int> moIdSet;
    set<int>::iterator it;

    Iterator(set<int> MOIDSET) 
    {
       moIdSet = MOIDSET;
      it = moIdSet.begin();
    }
    set<int>::iterator GetIterator() {return it;}
  };

  Iterator* iterator = static_cast<Iterator*>(local.addr);

  switch( message )
  {
    case OPEN: 
   {
      SETI* setiPtr         = static_cast<SETI*>(args[0].addr);
      Rectangle<2>* rect = static_cast<Rectangle<2>*>(args[1].addr);
      Instant* time1 = static_cast<Instant*>(args[2].addr);
      Instant* time2 = static_cast<Instant*>(args[3].addr);
      
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
      
      // Find UploadUnits in frontline which intersects search window
      set<int> moIdSet;
      map<int,UploadUnit>::iterator itFl;
      for (int i = 0; i < flBuckets; i++)
      {
        for ( itFl = setiPtr->frontline[i].begin();
                itFl !=  setiPtr->frontline[i].end(); itFl++)
        {
          UploadUnit unit = itFl->second;
          if ( *time1  <= unit.GetTime() &&
               *time2  >= unit.GetTime() &&
               area.x1 <= unit.GetPos().x && 
               area.x2 >= unit.GetPos().x &&
               area.y1 <= unit.GetPos().y && 
               area.y2 >= unit.GetPos().y )
            moIdSet.insert(unit.GetID());
        }
      }
     
      // Find trajectory segments which intersects search window
      set<TrjSeg*> hits = FindTrjSegments( setiPtr, area, *time1, *time2);

      // Put moID of all found segments into the moID-set
      set<TrjSeg*>::iterator itHits;
      for (itHits = hits.begin(); itHits != hits.end(); itHits++)
      { 
        TrjSeg* segPtr = *itHits;
        moIdSet.insert(segPtr->moID);
      }
      
      // Initialize iterator
      iterator = new Iterator(moIdSet);
      local.addr = iterator;
      return 0;
    }
    case REQUEST: 
    {
      if ( iterator->it != iterator->moIdSet.end() )
      {
      
        CcInt* elem = new CcInt(true, *iterator->GetIterator());
         TupleType* tupType = new TupleType(nl->Second(GetTupleResultType(s)));
         Tuple*     tup = new Tuple( tupType );
         tup->PutAttribute( 0, ( Attribute* ) elem );
         iterator->it++;
        result.addr = tup;

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

8.4 setiIntersectsWindow - Specification of operator

******************************************************************************/

struct IntersectsWindowInfo : OperatorInfo {
  IntersectsWindowInfo()
  {
    name      =  "setiIntersectsWindow";
    signature =  "seti x rect x  instant x instant  -> "
                 "stream (tuple (MovObjId int))";
    syntax    =  "setiIntersectsWindow ( _, _, _, _ )";
    meaning   =  "Returns all moving object IDs which"
                  "trajectory intersects the search window.";
    example   =  "query setiIntersectsWindow ( mySeti, myRect, "
                 "myDateTime1, myDateTime2 ) count";
  }
};

/******************************************************************************

9 setiInsideWindow operator

Returns all moving objects (Id) which trajectory is inside the search window.

9.1 setiInsideWindow - Type mapping method

******************************************************************************/

ListExpr InsideWindowTM(ListExpr args)
{
  if( nl->ListLength( args ) == 4 &&
      nl->IsEqual(nl->First(args),"seti") &&
      nl->IsEqual(nl->Second(args),"rect") &&
      nl->IsEqual(nl->Third(args),"instant") &&
      nl->IsEqual(nl->Fourth(args),"instant"))
  {
    //return NList(STREAM, INT).listExpr();
    ListExpr tupleList = nl->OneElemList(
                         nl->TwoElemList(nl->SymbolAtom("MovObjId"),
                                         nl->SymbolAtom("int")));
    ListExpr streamList = nl->TwoElemList(nl->SymbolAtom("stream"),
                          nl->TwoElemList(nl->SymbolAtom("tuple"),tupleList));
    return streamList;
  }
  ErrorReporter::ReportError("seti x rect x instant x instant expected!");
  return (nl->SymbolAtom( "typeerror" ));
}

/******************************************************************************

9.2 setiInsideWindow - Value mapping method

******************************************************************************/

int InsideWindowVM(Word* args, Word& result,int message,Word& local,Supplier s)
{


  struct Iterator 
  {
    set<int> moIdSet;
    set<int>::iterator it;

    Iterator(set<int> MOIDSET) 
    {
      moIdSet = MOIDSET;
      it = moIdSet.begin();
    }
    set<int>::iterator GetIterator() {return it;}
  };

  Iterator* iterator = static_cast<Iterator*>(local.addr);

  switch( message )
  {
    case OPEN: 
    {
      SETI* setiPtr         = static_cast<SETI*>(args[0].addr);
      Rectangle<2>* rect = static_cast<Rectangle<2>*>(args[1].addr);
      Instant* time1 = static_cast<Instant*>(args[2].addr);
      Instant* time2 = static_cast<Instant*>(args[3].addr);
      
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
      
      // Find UploadUnits in frontline which intersects search window
      set<int> moIdSet;
      map<int,UploadUnit>::iterator itFl;
      for (int i = 0; i < flBuckets; i++)
      {
        for ( itFl = setiPtr->frontline[i].begin();
                itFl !=  setiPtr->frontline[i].end(); itFl++)
        {
          UploadUnit unit = itFl->second;
          if ( *time1  <= unit.GetTime() &&
               *time2  >= unit.GetTime() &&
               area.x1 <= unit.GetPos().x && 
               area.x2 >= unit.GetPos().x &&
               area.y1 <= unit.GetPos().y && 
               area.y2 >= unit.GetPos().y )
            moIdSet.insert(unit.GetID());
        }
      }
     
      // Find trajectory segments which intersects search window
      set<TrjSeg*> hits = FindTrjSegments( setiPtr, area, *time1, *time2);

      // Put moID of all found segments into the moID-set
      set<TrjSeg*>::iterator itHits;
      for (itHits = hits.begin(); itHits != hits.end(); itHits++)
      { 
        TrjSeg* segPtr = *itHits;
        moIdSet.insert(segPtr->moID);
      }
    
      // Check if all segments of a moID are inside the window	  
      for (itHits = hits.begin(); itHits != hits.end(); itHits++)
      {
        // Erase moID id if segment is not inside the window
        TrjSeg* segPtr = *itHits;
        Instant segStart(instanttype);
        Instant segEnd(instanttype);
        segStart.ReadFrom(segPtr->tivStart);
        segEnd.ReadFrom(segPtr->tivEnd);
        
        if ( !(*time1  <= segStart &&
                *time2  >= segEnd &&
               area.x1 <= segPtr->pos1.x && area.x1 <= segPtr->pos2.x &&
               area.x2 >= segPtr->pos1.x && area.x2 >= segPtr->pos2.x &&
                area.y1 <= segPtr->pos1.y && area.y1 <= segPtr->pos2.y &&
               area.y2 >= segPtr->pos1.y && area.y2 >= segPtr->pos2.y ) )
              moIdSet.erase(segPtr->moID);
          delete segPtr;
       }
       
       // Initialize iterator
       iterator = new Iterator(moIdSet);
       local.addr = iterator;
       return 0;
     }
     case REQUEST: 
     {
       if ( iterator->it != iterator->moIdSet.end() )
       {
        CcInt* elem = new CcInt(true, *iterator->GetIterator());
        TupleType* tupType = new TupleType(nl->Second(GetTupleResultType(s)));
        Tuple*     tup = new Tuple( tupType );
        tup->PutAttribute( 0, ( Attribute* ) elem );
        iterator->it++;
        result.addr = tup;
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

9.2 setiInsideWindow - Specification of operator

******************************************************************************/

struct InsideWindowInfo : OperatorInfo {
  InsideWindowInfo()
  {
    name      =  "setiInsideWindow";
    signature =  "seti x rect x  instant x instant  -> "
                 "stream (tuple (MovObjId int))";
    syntax    =  "setiInsideWindow ( _, _, _, _ )";
    meaning   =  "Returns all moving object IDs which"
                 "trajectory is inside the search window.";
    example   =  "query setiInsideWindow( mySeti, myRect, "
                 "myDateTime1, myDateTime2 ) count";
  }
};

/******************************************************************************

10 setiCurrentUpload operator

Returns the current UploadUnit.

10.1 setiCurrentUpload - Type mapping method

******************************************************************************/

ListExpr CurrentUploadTM(ListExpr args)
{
  if( nl->ListLength( args ) == 2 &&
      nl->IsEqual(nl->First(args),"seti") &&
      nl->IsEqual(nl->Second(args),"int") )
  {
    return nl->SymbolAtom("uploadunit");
  }
  ErrorReporter::ReportError("seti x int expected!");
  return (nl->SymbolAtom( "typeerror" ));
}

/******************************************************************************

10.2 setiCurrentUpload - Value mapping method

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

10.3 setiCurrentUpload - Specification of operator

******************************************************************************/

struct CurrentUploadInfo : OperatorInfo {
  CurrentUploadInfo()
  {
    name      =  "setiCurrentUpload";
    signature =  "seti x int  -> int";
    syntax    =  "setiCurrentUpload ( _, _ )";
    meaning   =  "Returns the current UploadUnit.";
    example   =  "query setiCurrentUpload ( mySeti, 5 )";
  }
};

/******************************************************************************

11 Type constructor of class UploadUnit

******************************************************************************/

TypeConstructor UploadUnitTC(
  "uploadunit",                           // name
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

12 SETIAlgebra

******************************************************************************/

class SETIAlgebra : public Algebra
{
  public:
    SETIAlgebra() : Algebra()
    {
      AddTypeConstructor(&UploadUnitTC);
      AddTypeConstructor(&SETITC);
      UploadUnitTC.AssociateKind( "DATA" );
      
      AddOperator( CreateInfo(),       CreateVM,       CreateTM );
      AddOperator( InsertUnitInfo(),   InsertUnitVM,   InsertUnitTM );
      AddOperator( InsertStreamInfo(), InsertStreamVM, InsertStreamTM );
      AddOperator( IntersectsWindowInfo(), IntersectsWindowVM, 
                   IntersectsWindowTM);
      AddOperator( InsideWindowInfo(), InsideWindowVM, InsideWindowTM );
      AddOperator( CurrentUploadInfo(), CurrentUploadVM, CurrentUploadTM );
    }
    ~SETIAlgebra() {};
};

/******************************************************************************

13 Initialization of SETIAlgebra

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
