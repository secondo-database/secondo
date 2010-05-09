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

[1] Declaration of SETIAlgebra

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

******************************************************************************/

#ifndef __SETI_ALGEBRA_H__
#define __SETI_ALGEBRA_H__

/******************************************************************************

2 Globals constants and variables

******************************************************************************/

const int    maxTrjSeg = 70;      // Max number of trajectory segments in page
const int    splits    = 2;       // Number of SETI grid partitions for one dim
const int    pageSize  = 4096;    // Alternatively: WinUnix::getPageSize()
const int    flBuckets = 50;      // Number of hash buckets in frontline
const double tol       = 0.00001; // Tolerance for floating points
bool         intersects[4];       // Indicates an intersection of cell border
                                  // (left,right,bottom,top) and trajectory seg


/******************************************************************************

3.1 Definition of SETIArea

The SETIArea structure defines the boundary of a SETI grid or a search window.

******************************************************************************/

struct SETIArea
{
  SETIArea(){}
  SETIArea( double X1, double Y1, double X2, double Y2 ):
            x1( X1 ), y1( Y1 ), x2( X2 ), y2( Y2 ){}
  double x1;  // x1 coordinate
  double y1;  // y1 coordinate
  double x2;  // x2 coordinate
  double y2;  // y2 coordinate
};

/******************************************************************************

3.2 Definition of TrjSeg

TrjSeg holds the information of a trajectory segment.

******************************************************************************/

struct TrjSeg
{
  TrjSeg( int MOID, int SEGID, double TIVSTART, double TIVEND,
          UnitPos POS1,  UnitPos POS2 )
  {
    moID      = MOID;
    segID     = SEGID;
    tivStart  = TIVSTART;
    tivEnd    = TIVEND;
    pos1      = POS1;
    pos2      = POS2;
  }
  int moID;         // Moving object id
  int segID;        // Segment id
  double tivStart;  // Start time of segment
  double tivEnd;    // End time of segment
  UnitPos pos1;     // Start position of segment
  UnitPos pos2;     // End position of segment
};

/******************************************************************************

3.3 Definition of SETICell

The SETICell structure contains all information  of a cell in a SETI grid.

******************************************************************************/

struct SETICell
{
  int         numSegments;      // Number of segments in cell
  SETIArea    area;             // Cell area (partition)
  SmiFileId   rtreeFileID;      // RTree file id
  db_pgno_t   currentPage;      // Number of current cell page
  R_Tree<2,TupleId>* rtreePtr;  // RTree pointer
  Interval<Instant> tiv;        // Cell time interval
};

/******************************************************************************

3.4 Definition of SETIHeader

The SETIHeader is used to store the most important SETI data.

******************************************************************************/

struct SETIHeader
{
  SETIHeader()
  { 
    fileID       = (SmiFileId)0;
    headerPageNo = (db_pgno_t)0;
    for(int i = 0; i < flBuckets; i++) { flPageNo[i] = (db_pgno_t)0; }
    cellsPageNo  = (db_pgno_t)0;
    area         = SETIArea(0,0,0,0);
    numCells     = splits*splits;
    numEntries   = 0;
    numFlEntries = 0;
    tiv.start    = DateTime(0,0,instanttype);
    tiv.end      = DateTime(0,0,instanttype);
    tiv.lc       = false;
    tiv.rc       = false;    
  }
  
  SETIHeader(SETIArea AREA)
  { 
    fileID       = (SmiFileId)0;
    headerPageNo = (db_pgno_t)0;
    for(int i = 0; i < flBuckets; i++) { flPageNo[i] = (db_pgno_t)0; }
    cellsPageNo  = (db_pgno_t)0;
    area         = AREA;
    numCells     = splits*splits;
    numEntries   = 0;
    numFlEntries = 0;
    tiv.start    = DateTime(0,0,instanttype);
    tiv.end      = DateTime(0,0,instanttype);
    tiv.lc       = false;
    tiv.rc       = false;
  }
  
  SmiFileId         fileID;              // SETI file id
  db_pgno_t         headerPageNo;        // Header page number
  db_pgno_t         flPageNo[flBuckets]; // Front-line page number
  db_pgno_t         cellsPageNo;         // Cells page number
  SETIArea          area;                // SETI area
  int               numCells;            // Number of cells
  int               numEntries;          // Number of TrjSegments/UploadUnits
  int               numFlEntries;        // Number of front-line entries
  Interval<Instant> tiv;                 // SETI time interval
};

/******************************************************************************

4 Declaration of class SETI

This class defines a SETI index which consists of a
- header
- frontline
- grid apportioned by cells (including RTrees)
- SmiUpdateFile to store all information in a shared memory

******************************************************************************/

class SETI
{
  public:
    // Basic constructor
    SETI(SETIArea AREA);
    // Query constructor
    SETI(SmiFileId FILEID);
    // Destructor
    ~SETI();
    
    // The mandatory set of algebra support functions
    static Word    In( const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct );
    static ListExpr Out( ListExpr typeInfo, Word value );
    static Word     Create( const ListExpr typeInfo );
    static void     Delete( const ListExpr typeInfo, Word& w );
    static bool     Open( SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value );
    static bool     Save( SmiRecord& valueRecord, size_t& offset, 
                          const ListExpr typeInfo, Word& w );
    static void     Close( const ListExpr typeInfo, Word& w );
    static Word     Clone( const ListExpr typeInfo, const Word& w );
    static bool     KindCheck( ListExpr type, ListExpr& errorInfo );
    static void*    Cast(void* addr);
    static int      SizeOfObj();  
    static ListExpr Property();
    
    // Writes header information into file
    void UpdateHeader();
    // Writes front-line information into file
    void UpdateFLine(int BUCKET);
    // Writes cell information into file
    void UpdateCells();
    // Returns the SETI file id
    SmiUpdateFile* GetUpdateFile();
    // Returns the pointer to the SETI header
    SETIHeader* GetHeader();
    // Returns the pointer to the stated cell
    SETICell* GetCell(int COL, int ROW);
    // Returns the state of the semaphore
    bool GetSemaphore();
    // Sets the state of the semaphore
    void SetSemaphore(bool VALUE);
    
    // SETI frontline hash
    map<int,UploadUnit> frontline[flBuckets];
    
  private:
    // Reads header, frontline and cells information
    void ReadSETI();
    
    SmiUpdateFile*  suf;                   // SmiUpdateFile
    SETIHeader*     header;                // SETI header
    SETICell*       cells[splits][splits]; // SETI cells
};

#endif
