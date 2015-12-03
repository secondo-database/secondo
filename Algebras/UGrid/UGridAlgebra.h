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

July 2010, Daniel Brockmann

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

******************************************************************************/

#ifndef __UGrid_ALGEBRA_H__
#define __UGrid_ALGEBRA_H__

/******************************************************************************

2 Globals constants and variables

******************************************************************************/

const int pageSize        = 4096;    // Alternatively: WinUnix::getPageSize()
const int maxSplits       = 64;      // Number of max splits for one dim
const int flBuckets       = 1000;    // Number of hash buckets in front-line
const int updateCycle     = 100000;  // Update cycle for SmiUpdateFile
const int maxHistoryUnits = 70;      // Max number of history units in page
const double tol          = 0.00001; // Tolerance for floating points
bool intersects[4];                  // Indicates an intersection of cell ...
                                     // ... border and history unit

/******************************************************************************

3.2 Definition of UGridArea

The UGridArea structure defines the boundary of a UGrid grid or a search window.

******************************************************************************/

struct UGridArea
{
  UGridArea(){}
  UGridArea( double X1, double Y1, double X2, double Y2 ):
            x1( X1 ), y1( Y1 ), x2( X2 ), y2( Y2 ){}
  double x1;  // x1 coordinate
  double y1;  // y1 coordinate
  double x2;  // x2 coordinate
  double y2;  // y2 coordinate
};

/******************************************************************************

3.3 Definition of HistoryUnit

HistoryUnit holds the information of a trajectory segment.

******************************************************************************/

struct HistoryUnit
{
  HistoryUnit( int MOID, int HUID, double TIVSTART, double TIVEND,
               UnitPos POS1,  UnitPos POS2 )
  {
    moID      = MOID;
    huID      = HUID;
    tivStart  = TIVSTART;
    tivEnd    = TIVEND;
    pos1      = POS1;
    pos2      = POS2;
  }
  int moID;         // Moving object id
  int huID;         // History unit id
  double tivStart;  // Start time of history unit
  double tivEnd;    // End time of history unit
  UnitPos pos1;     // Start position of history unit
  UnitPos pos2;     // End position of history unit
};

/******************************************************************************

3.4 Definition of UGridBox

The UGridBox consists of a 3D-bounding box and a page number.

******************************************************************************/

struct UGridBox
{
  UGridBox (db_pgno_t PAGEID, bool HUPAGEID, Rectangle<3> BOX)
  {
    pageID   = PAGEID;
    huPageID = HUPAGEID;
    box      = BOX;
  }

  db_pgno_t    pageID;   // HistoryUnit or node page, the box refers
  bool         huPageID; // History page id indicator
  Rectangle<3> box;      // 3D bounding box
};

// Record size of UGridBox
const size_t boxSize = sizeof(db_pgno_t) + sizeof(bool) + (6*sizeof(double));

/******************************************************************************

3.5 Definition of UGridNode

The UGridNode stores a number of UGridBoxes.

******************************************************************************/

struct UGridCell;
struct UGridNode
{
  UGridNode* fatherNode;      // Pointer to father node
  UGridNode* leftBottomSon;   // Pointer to left-bottom son
  UGridNode* rightBottomSon;  // Pointer to right-bottom son
  UGridNode* leftTopSon;      // Pointer to left-top son
  UGridNode* rightTopSon;     // Pointer to right-top son
  UGridCell* leftBottomCell;  // Pointer to left-bottom cell
  UGridCell* rightBottomCell; // Pointer to right-bottom cell
  UGridCell* leftTopCell;     // Pointer to left-top cell
  UGridCell* rightTopCell;    // Pointer to right-top cell
  int        numEntries;      // Number of UGridBoxes in node*
  db_pgno_t  currentPage;     // Current page for UGridBoxes*
  UnitPos    pos1;            // Start position of UGridNode*
  UnitPos    pos2;            // End position of UGridNode*
  temporalalgebra::Interval<Instant> tiv;      // UGridNode time interval*
};

// Record size of UGridNode -> Only variables with (*) will be stored in page.
const size_t nodeSize = sizeof(int) + sizeof(db_pgno_t) + (6*sizeof(double))
                        + (2*sizeof(bool));

/******************************************************************************

3.6 Definition of UGridCell

The UGridCell structure contains all information  of a cell in a UGrid grid.

******************************************************************************/

struct UGridCell
{
  UGridNode*  fatherNode;  // Pointer to father node
  UGridArea   area;        // Cell area (partition)
  int         numEntries;  // Number of history units in cell page*
  db_pgno_t   currentPage; // current page for history units*
  UnitPos     pos1;        // Position 1 of cell*
  UnitPos     pos2;        // Position 2 of cell*
  temporalalgebra::Interval<Instant> tiv;   // Cell time interval*
};

// Record size of UGridCell -> Only variables with (*) will be stored in page.
const size_t cellSize = sizeof(int) + sizeof(db_pgno_t) + (6*sizeof(double))
                        + (2*sizeof(bool));

/******************************************************************************

3.7 Definition of UGridHeader

The UGridHeader is used to store the most important UGrid data.

******************************************************************************/

struct UGridHeader
{
  UGridHeader()
  {
    fileID            = (SmiFileId)0;
    headerPageNo      = (db_pgno_t)0;
    nodePageNo        = (db_pgno_t)0;
    rootNode          = (UGridNode*)0;
    selectedNodePage  = (SmiUpdatePage*)0;
    currentNodeRecord = 0;
    area              = UGridArea(0,0,0,0);
    splits            = 0;
    numCells          = 0;
    numEntries        = 0;
    numFlEntries      = 0;
    tiv.start         = DateTime(0,0,instanttype);
    tiv.end           = DateTime(0,0,instanttype);
    tiv.lc            = false;
    tiv.rc            = false;
  }

  UGridHeader(UGridArea AREA, int SPLITS)
  {
    fileID            = (SmiFileId)0;
    headerPageNo      = (db_pgno_t)0;
    nodePageNo        = (db_pgno_t)0;
    rootNode          = (UGridNode*)0;
    selectedNodePage  = (SmiUpdatePage*)0;
    currentNodeRecord = 0;
    area              = AREA;
    splits            = SPLITS;
    numCells          = SPLITS*SPLITS;
    numEntries        = 0;
    numFlEntries      = 0;
    tiv.start         = DateTime(0,0,instanttype);
    tiv.end           = DateTime(0,0,instanttype);
    tiv.lc            = false;
    tiv.rc            = false;
  }

  SmiFileId      fileID;            // UGrid file id
  db_pgno_t      headerPageNo;      // Header page number
  db_pgno_t      flPageNo;          // Front-line page number
  db_pgno_t      cellPageNo[8][8];  // Cell storage
  db_pgno_t      nodePageNo;        // First page of node storage
  UGridNode*     rootNode;          // Root node of UGrid tree
  SmiUpdatePage* selectedNodePage;  // Selected node page
  int            currentNodeRecord; // Current node record number in page
  UGridArea      area;              // UGrid area
  int            splits;            // Number of SETI partitions for one dim
  int            numCells;          // Number of cells
  int            numEntries;        // Number of TrjSegments/UploadUnits
  int            numFlEntries;      // Number of front-line entries
  temporalalgebra::Interval<Instant> tiv;            // UGrid time interval
};

/******************************************************************************

4 Declaration of class UGrid

This class defines a UGrid which consists of a
- header
- frontline
- quad-tree like structure
- grid apportioned by cells
- SmiUpdateFile to store all information in a shared memory file

******************************************************************************/

class UGrid
{
  public:
   // Basic constructor
   UGrid(UGridArea AREA, int SPLITS);
   // Query constructor
   UGrid(SmiFileId FILEID);
   // Destructor
   ~UGrid();

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

   // type name used in Secondo
   inline static const string BasicType() { return "ugrid"; }
    static const bool checkType(const ListExpr type){
      return listutils::isSymbol(type, BasicType());
    }
   // Writes UGrid-header information into file
   void UpdateHeader();
   // Writes UGrid-front-line information into file
   void UpdateFLine();
   // Writes UGridTree information into file
   void UpdateUGridTree( UGridNode* FATHERNODE, int SQUARES,
                         int RIGHTCOL, int TOPROW );
   // Writes all UGrid data into file
   bool UpdateUGrid();
   // Inserts a UGridBox into the UGridTree.
   void InsertUGridBox( UGridNode* FATHERNODE, UGridBox* BOX );
   // Returns the UGrid file id
   SmiUpdateFile* GetUpdateFile();
   // Returns the pointer to the UGrid header
   UGridHeader* GetHeader();
   // Returns the pointer to the stated cell
   UGridCell* GetCell(int COL, int ROW);
   // Finds HistoryUnit pages in the UGridTree
   void FindHistoryPages( UGridArea AREA, Instant TIME1, Instant TIME2,
                          UGridNode* FATHERNODE, int SQUARES );
   // Returns the semaphore state
   bool GetSemaphore();
   // Sets the semaphore state
   void SetSemaphore(bool VALUE);

   // UGrid front-line hash
   map<int,UploadUnit> frontline[flBuckets];
   // HistoryUnit pages
   set<db_pgno_t> pages;

  private:
    // Reads UGrid-header information from file
    void ReadHeader();
    // Reads UGrid-front-line information from file
    void ReadFLine();
    // Reads UGridTree information from file
    UGridNode* ReadUGridTree( UGridNode* FATHERNODE, UGridArea AREA,
                              int SQUARES, int RIGHTCOL, int TOPROW );
    // Creates a new UGridTree
    UGridNode* CreateUGridTree( UGridNode* FATHERNODE, int SQUARES,
                                int RIGHTCOL, int TOPROW );
    // Destructs the UGridTree
    void DestructUGridTree( UGridNode* FATHERNODE, int SQUARES );
    // Inserts HistoryUnit page id's into pages set
    void insertPageID(db_pgno_t NODEPAGEID, bool HUPAGEID);

    SmiUpdateFile* suf;                         // SmiUpdateFile
    UGridHeader*   header;                      // UGrid header
    UGridCell*     cells[maxSplits][maxSplits]; // UGrid cells
};

#endif
