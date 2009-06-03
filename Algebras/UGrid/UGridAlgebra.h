
/*
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
*/

/*
paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
paragraph [10] Footnote: [{\footnote{] [}}]

March 2009 Brigitte Metzker
          
*/

/*
1.    Declarations Necessary for Algebra UGrid 

*/
#ifndef __UGRID_ALGEBRA_H__
#define __UGRID_ALGEBRA_H__

#include "stdarg.h"

#ifdef SECONDO_WIN32
#define Rectangle SecondoRectangle
#endif

#include <iostream>
#include <stack>
#include <limits>
#include <string.h>

//Includes required by extensions for the NearestNeighborAlgebra:
#include <vector>
#include <queue>

using namespace std;

#include "SpatialAlgebra.h"
#include "RelationAlgebra.h"
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"
#include "StandardTypes.h"
#include "DBArray.h"

#include "UGridUtilities.h"


extern NestedList* nl;
extern QueryProcessor* qp;

namespace ugridAlgebra 
{
// possible dimensions of an ugrid
enum dimsize {dim1 = 64, dim2 = 32, dim3 = 16, dim4 = 8, dim5 = 4, dim6 = 2, 
              dim7 = 1, dim8 = 0};
// continue flag used by splitting historyunit 
enum  husplit {none, colsp, colsm, linesp, linesm}; 
// namesuffix of cellcontainer 
enum  contsuffix {Q1=1, Q2=2, Q3=3, Q4=4, Q5=5, Q6=6, Q7=7, Q8=8, 
                  Q9=9, Q10=10, Q11=11, Q12=12, Q13=13, 
				  Q14=14, Q15=15, Q16=16};

const int sizeHistoryArray = 70;  // max num of history units in ugridcell     
const int sizeCellArray = 60;     // max num of history units in ugridcell      
const int maxCont = 16;           // max num of container with cellrecordIds
const int maxSlave = 6;           // max num of childs of slave ugrid
const int maxCellRec = 256;       // max num cellRecords in cellcontainer

/*
           Data Structure - Class   MobPos  

*/
class MobPos  
{
 public: 

/*
Constructors and destructor:

*/
  MobPos( double x=0, double y=0 );
  MobPos(const MobPos& Pos);
  ~MobPos();
  
  double  GetX() const;
  double  GetY() const;
  void SetX( double x );
  void SetY( double y );

/*
  Assignment operator

*/
MobPos& operator= (const MobPos& mobpos)
  {
    x = mobpos.x;
	y = mobpos.y;
	return *this;
  }

/*
  Comparison operator

*/
  bool operator== (const MobPos& pos)
  {
    bool ret = (x == pos.x   &&
	            y == pos.y);
	return ret;
  }


/*
  Methode zum Berechnen der ersten Next-Position
  Aufruf beim ersten Update 
  zu diesem Zeitpunkt noch keine CurrentUnit zu dieser ID 

*/
  MobPos FirstPredictPosition (double x, double y);

  MobPos* Clone();

  static Word     InMobPos( const ListExpr typeInfo, const ListExpr instance,
                        const int errorPos, ListExpr& errorInfo, 
                        bool& correct );

  static ListExpr OutMobPos( ListExpr typeInfo, Word value );

  static Word     CreateMobPos( const ListExpr typeInfo );

  static void     DeleteMobPos( const ListExpr typeInfo, Word& w );

  static void     CloseMobPos( const ListExpr typeInfo, Word& w );

  static Word     CloneMobPos( const ListExpr typeInfo, const Word& w );

  static bool     KindCheckMobPos( ListExpr type, ListExpr& errorInfo );

  static int      SizeOfObjMobPos();  
  
  static ListExpr PropertyMobPos();

public:
  double x;
  double y;
};

ostream& operator << (ostream &cout, const MobPos &pos);

/*
            Data Structure - Class ~UGridArea~  

*/
class UGridArea
{
 public:
  UGridArea( double X1, double Y1, double X2, double Y2 );
  UGridArea( const UGridArea& area );
  ~UGridArea();
  
  double GetX1() const;
  double GetY1() const;
  double GetX2() const;
  double GetY2() const;  
  
  UGridArea GetArea();
  bool  intersects( const UGridArea& r ) const;
  
  void SetX1(double a1);
  void SetY1(double b1);
  void SetX2(double a2);
  void SetY2(double b2);

/*
   Assignment operator

*/
  UGridArea& operator= (const UGridArea& area)
  {
    x1 = area.x1;
	y1 = area.y1;
	x2 = area.x2;
	y2 = area.y2;
	return *this;
  }
/*
   Comparison operator

*/
  bool operator== (const UGridArea& area)
  {
    bool ret = (x1 == area.x1    &&
	            y1 == area.y1    &&
	            x2 == area.x2    &&
	            y2 == area.y2);
	return ret;
  }

  static Word    InUGridArea( const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct );

  static ListExpr OutUGridArea( ListExpr typeInfo, Word value );

  static Word     CreateUGridArea( const ListExpr typeInfo );

  static bool     OpenUGridArea( SmiRecord& valueRecord, 
                        size_t& offset, const ListExpr typeInfo, 
                        Word& value );

  static bool     SaveUGridArea( SmiRecord& valueRecord, size_t& offset, 
                        const ListExpr typeInfo, Word& w );
  
  static void     DeleteUGridArea( const ListExpr typeInfo, Word& w );

  static void     CloseUGridArea( const ListExpr typeInfo, Word& w );

  static Word     CloneUGridArea( const ListExpr typeInfo, const Word& w );

  static int      SizeOfObjUGridArea();  

  static bool     KindCheckUGridArea( ListExpr type, ListExpr& errorInfo );

  static ListExpr PropertyUGridArea();
  public: 
  UGridArea() {}

  friend class ConstructorFunctions<UGridArea>; 

  double x1;
  double y1;
  double x2;
  double y2;

};

/*
            Data Structure - Class ~UpdateUnit~             
            Data Structure - Class UpdateUnit and            
                  the mandatory set of algebra   

*/
class UpdateUnit//: public StandardAttribute  
{
 public:
  UpdateUnit( const int& Id, const MobPos& pos);
  UpdateUnit( const int& Id, const Instant& Time, const MobPos& Pos );
  UpdateUnit( const UpdateUnit& uu );
  ~UpdateUnit() {}
  
  int GetUId()   const;
  MobPos GetPos() const;
  double GetXPos() const;
  double GetYPos() const;
  Instant GetTime() const;

/*
   Assignment operator

*/
  UpdateUnit& operator= (const UpdateUnit& upu)
  {
    id = upu.id;
	time = upu.time;
	pos.x = upu.pos.x;
	pos.y = upu.pos.y;
	return *this;
  }

/*
   Comparison operator

*/
  bool operator== (const UpdateUnit& upu)
  {
    bool ret = (id == upu.id         &&
	            time == upu.time     &&
	            pos.x == upu.pos.x   &&
	            pos.y == upu.pos.y);
	return ret;
  }
  static Word   InUpdateUnit( const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct );

  static ListExpr OutUpdateUnit( ListExpr typeInfo, Word value );

  static Word     CreateUpdateUnit( const ListExpr typeInfo );

  static bool     OpenUpdateUnit( SmiRecord& valueRecord, 
                        size_t& offset, const ListExpr typeInfo, 
                        Word& value );

  static bool     SaveUpdateUnit( SmiRecord& valueRecord, size_t& offset, 
                        const ListExpr typeInfo, Word& w );

  static void     DeleteUpdateUnit( const ListExpr typeInfo, Word& w );

  static void     CloseUpdateUnit( const ListExpr typeInfo, Word& w );

  static Word     CloneUpdateUnit( const ListExpr typeInfo, const Word& w );

  static bool     KindCheckUpdateUnit( ListExpr type, ListExpr& errorInfo );

  static int      SizeOfObjUpdateUnit();  
  
  static ListExpr PropertyUpdateUnit();

  public: 
  UpdateUnit() {}

  friend class ConstructorFunctions<UpdateUnit>; 

  int id;
  Instant time;
  MobPos pos;
};

/*
         Data Structure - Class CurrentUnit and              
               the mandatory set of algebra                 
                                                                                  
           Data Structure - Class ~CurrentUnit~    

*/
class CurrentUnit
{
 public:
  CurrentUnit( int CId, Interval<Instant> CTimeInterval, 
	           MobPos Cpos, MobPos Npos);  
  CurrentUnit( const CurrentUnit& cu );
  ~CurrentUnit() {}
  
  int GetCId()   const;
  MobPos GetCPos() const;
  MobPos GetNPos() const;
  double GetCXPos() const;
  double GetCYPos() const;
  double GetNXPos() const;
  double GetNYPos() const;

/*
   Assignment operator

*/
  CurrentUnit& operator= (const CurrentUnit& cuu)
  {
    cid = cuu.cid;
	timestamp = cuu.timestamp;
	cpos.x = cuu.cpos.x;
	cpos.y = cuu.cpos.y;
	npos.x = cuu.npos.x;
	npos.y = cuu.npos.y;
	return *this;
  }

/*
   Comparison operator

*/
  bool operator== (const CurrentUnit& cuu)
  {
    bool ret = (cid == cuu.cid               &&
	            timestamp == cuu.timestamp   &&
	            cpos.x == cuu.cpos.x         &&
	            cpos.y == cuu.cpos.y         &&
	            npos.x == cuu.npos.x         &&
	            npos.y == cuu.npos.y);
	return ret;
  }

  static Word  InCurrentUnit( const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct );

  static ListExpr OutCurrentUnit( ListExpr typeInfo, Word value );

  static Word     CreateCurrentUnit( const ListExpr typeInfo );

  static void     DeleteCurrentUnit( const ListExpr typeInfo, Word& w );

  static void     CloseCurrentUnit( const ListExpr typeInfo, Word& w );

  static Word     CloneCurrentUnit( const ListExpr typeInfo, const Word& w );

  static bool     KindCheckCurrentUnit( ListExpr type, ListExpr& errorInfo );

  static int      SizeOfObjCurrentUnit();  
  
  static ListExpr PropertyCurrentUnit();

  static bool     OpenCurrentUnit( SmiRecord& valueRecord, 
                        size_t& offset, const ListExpr typeInfo, 
                        Word& value );

  static bool     SaveCurrentUnit( SmiRecord& valueRecord, size_t& offset, 
                        const ListExpr typeInfo, Word& w );
  public: 
  CurrentUnit() {}

  friend class ConstructorFunctions<CurrentUnit>;  
  //currentmap theCurrentUnit;
  int    cid;
  long   timestamp;
  Interval<Instant> cTimeInterval;
  MobPos cpos;
  MobPos npos;
 
};

/*
         Data Structure - Class HistoryUnit and            
               the mandatory set of algebra                 

          Data Structure - Class ~HistoryUnit~   

*/
class HistoryUnit
{
 public:
  HistoryUnit( int HId, Interval<Instant> HTimeInterval, 
	           MobPos Spos, MobPos Epos);  
  HistoryUnit( int HId, long HuTimestart, long HuTimeend, 
	           MobPos Spos, MobPos Epos);
  HistoryUnit( const HistoryUnit& hu );
  ~HistoryUnit() {}
  
  int GetHId()   const;
  MobPos GetSPos() const;
  MobPos GetEPos() const;
  double GetSXPos() const;
  double GetSYPos() const;
  double GetEXPos() const;
  double GetEYPos() const;
  long GetHuTimeStart() const;
  long GetHuTimeEnd() const;
 
/*
   Assignment operator

*/
  HistoryUnit& operator= (const HistoryUnit& huu)
  {
    hid = huu.hid;
	htimestart = huu.htimestart;
	htimeend = huu.htimeend;
	spos.x = huu.spos.x;
	spos.y = huu.spos.y;
	epos.x = huu.epos.x;
	epos.y = huu.epos.y;
	return *this;
  }

/*
   Comparison operator

*/
  bool operator== (const HistoryUnit& huu)
  {
    bool ret = (hid == huu.hid               &&
	            htimestart == huu.htimestart &&
				htimeend == huu.htimeend     &&
	            spos.x == huu.spos.x         &&
	            spos.y == huu.spos.y         &&
	            epos.x == huu.epos.x         &&
	            epos.y == huu.epos.y);
	return ret;
  }
  
  static int SizeOfHistoryUnit();
  void StartBulkLoad();
  void EndBulkLoad();

  static Word  InHistoryUnit( const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct );

  static ListExpr OutHistoryUnit( ListExpr typeInfo, Word value );

  static Word     CreateHistoryUnit( const ListExpr typeInfo );

  static void     DeleteHistoryUnit( const ListExpr typeInfo, Word& w );

  static void     CloseHistoryUnit( const ListExpr typeInfo, Word& w );

  static Word     CloneHistoryUnit( const ListExpr typeInfo, const Word& w );

  static bool     KindCheckHistoryUnit( ListExpr type, ListExpr& errorInfo );

  static int      SizeOfObjHistoryUnit();  
  
  static ListExpr PropertyHistoryUnit();

  static bool     OpenHistoryUnit( SmiRecord& valueRecord, 
                        size_t& offset, const ListExpr typeInfo, 
                        Word& value );

  static bool     SaveHistoryUnit( SmiRecord& valueRecord, size_t& offset, 
                        const ListExpr typeInfo, Word& w );
  public: 
  HistoryUnit() {}

  friend class ConstructorFunctions<HistoryUnit>;  
  //historymap theHistoryUnit;
  int    hid;
  Interval<Instant> htimeInterval;
  long   htimestart;
  long   htimeend;
  MobPos spos;
  MobPos epos;

  // Reads a historyunit from the buffer. Offset is increased.
  void Read( char *buffer, int& offset )
  {
    memcpy( &hid, buffer+offset, sizeof(int) );
    offset += sizeof(int);
    memcpy( &htimeInterval, buffer+offset, sizeof( Interval<Instant>) );
    offset += sizeof( Interval<Instant>);
	memcpy( &spos, buffer+offset, sizeof(MobPos) );
    offset += sizeof(MobPos);
    memcpy( &epos, buffer+offset, sizeof( MobPos ));
    offset += sizeof( MobPos);
  }

  // Writes a historyunit to the buffer. Offset is increased.
  void Write( char *buffer, int& offset )
  {
    memcpy( buffer+offset, &hid, sizeof(int) );
    offset += sizeof(int);
	memcpy(  buffer+offset, &htimeInterval, sizeof( Interval<Instant>) );
    offset += sizeof( Interval<Instant>);
	memcpy( buffer+offset, &spos, sizeof(MobPos) );
    offset += sizeof(MobPos);
    memcpy(  buffer+offset, &epos, sizeof( MobPos) );
    offset += sizeof( MobPos);
  }
};

/*
         Data Structure - Class UGridCell and               
               the mandatory set of algebra                  

           Data Structure - Class ~UGridCell~   

*/
struct CellUnit
{
   SmiRecordId    cellRecId;
   int            cellabel;
   long           ugctimestart;
   long           ugctimeend;
   MobPos         ugcspos;
   MobPos         ugcepos;
};
class UGridCell
{
  public: 

  /*
     Constructors and destructor:

  */
  UGridCell( int labelindex, dimsize dim, UGridArea ucarea); 
  UGridCell(int intLabel, UGridArea area, int nofEntries, long t1, long t2,
	        bool modified, HistoryUnit historyArray[sizeHistoryArray]);
  UGridCell(int intLabel, UGridArea area, int nofEntries, Instant int1,
	        Instant int2,
	        bool modified, HistoryUnit historyArray[sizeHistoryArray]);
  UGridCell(const UGridCell& ugridcell);
  UGridCell(int ind, SmiRecordId cellRecId, const UGridCell& ugridcell);

  ~UGridCell();
  
  int GetIntLabel() const; 
  UGridArea GetArea() const;
  int GetNoOfEntries() const;
  long GetFirstEntry() const;
  long GetLastEntry() const;
  bool IsModified() const;

/*
   Assignment operator

*/
  UGridCell& operator= (const UGridCell& ugc)
  {
    intlabel = ugc.intlabel;
	ucarea.x1 = ugc.ucarea.x1;
	ucarea.y1 = ugc.ucarea.y1;
	ucarea.x2 = ugc.ucarea.x2;
	ucarea.y2 = ugc.ucarea.y2;
	noOfEntries = ugc.noOfEntries;
	firstTime = ugc.firstTime;
	lastTime = ugc.lastTime;
	modified = ugc.modified;
	for (int i = 0; i < noOfEntries; i++)
	{
      historyArray[i].hid = ugc.historyArray[i].hid;
	  historyArray[i].htimestart = ugc.historyArray[i].htimestart;
	  historyArray[i].htimeend = ugc.historyArray[i].htimeend;
	  historyArray[i].spos.x = ugc.historyArray[i].spos.x;
	  historyArray[i].spos.y = ugc.historyArray[i].spos.y;
	  historyArray[i].epos.x = ugc.historyArray[i].epos.x;
	  historyArray[i].epos.y = ugc.historyArray[i].epos.y;
	}
	return *this;
  }

/*
 Copy operator

*/
  UGridCell& CellCopy (const UGridCell& ugc)
  {
    intlabel = ugc.intlabel;
	ucarea.x1 = ugc.ucarea.x1;
	ucarea.y1 = ugc.ucarea.y1;
	ucarea.x2 = ugc.ucarea.x2;
	ucarea.y2 = ugc.ucarea.y2;
	noOfEntries = ugc.noOfEntries;
	firstTime = ugc.firstTime;
	lastTime = ugc.lastTime;
	modified = ugc.modified;
	for (int i = 0; i < noOfEntries; i++)
	{
      cellArray[i].cellRecId = ugc.cellArray[i].cellRecId;
	  cellArray[i].cellabel = ugc.cellArray[i].cellabel;
	  cellArray[i].ugctimestart = ugc.cellArray[i].ugctimestart;
	  cellArray[i].ugctimeend = ugc.cellArray[i].ugctimeend;
	  cellArray[i].ugcspos.x = ugc.cellArray[i].ugcspos.x;
	  cellArray[i].ugcspos.y = ugc.cellArray[i].ugcspos.y;
	  cellArray[i].ugcepos.x = ugc.cellArray[i].ugcepos.x;
	  cellArray[i].ugcepos.y = ugc.cellArray[i].ugcepos.y;
	}
	return *this;
  }

/*
   Comparison operator

*/
  bool operator== (const UGridCell& ugc)
  {
    bool ret = (intlabel == ugc.intlabel     &&
	            ucarea.x1 == ugc.ucarea.x1   &&
	            ucarea.y1 == ugc.ucarea.y1   &&
	            ucarea.x2 == ugc.ucarea.x2   &&
	            ucarea.y2 == ugc.ucarea.y2   &&
	            firstTime == ugc.firstTime   &&
	            lastTime == ugc.lastTime     &&
	            modified == ugc.modified);
	return ret;
  }

  void SetLabel(int label)
  {
    this->intlabel = label;
  }
  void SetArea(UGridArea carea)
  {
    this->ucarea.x1 = carea.x1;
	this->ucarea.y1 = carea.y1;
	this->ucarea.x2 = carea.x2;
	this->ucarea.y2 = carea.y2;
  }
  void SetNoOfEntries(int noOfEntries)
  {
    this->noOfEntries = noOfEntries;
  }
  void SetFirstEntry(Instant firstEntry)
  {
    this->ugcInterval.start = firstEntry;
  }  
  void SetLastEntry(Instant lastEntry)
  {
    this->ugcInterval.end = lastEntry;
  }	  
  void SetModified(bool mod)
  {
    this->modified = mod;
  }

  Interval<Instant> GetCellInterval () const; 
  string IntLabelToString(int label) const;
  string GetString() const ;
  void IncrNoOfEntries()
  {
    this->noOfEntries ++;
  }
  // UGridCell AddHistoryUnit (HistoryUnit hu);
  void AddHistoryUnit( HistoryUnit hu);
  void ClearHu();
  void ClearCu();
  void SetZero();
  void StartBulkLoad();
  void EndBulkLoad();

/*
 Reads this cell from an ~SmiRecordFile~ at position ~id~.

*/

 //void Read( SmiRecordFile& file, const SmiRecordId pointer );
 //void Read( SmiRecord& record );

/*
Writes this cell to an ~SmiRecordFile~ at position ~id~

*/
//void Write( SmiRecordFile& file, const SmiRecordId pointer );
//void Write( SmiRecord& record );

  UGridCell* Clone();

  static Word    InUGridCell( const ListExpr typeInfo, const ListExpr instance,
                        const int errorPos, ListExpr& errorInfo, 
                        bool& correct );

  static ListExpr OutUGridCell( ListExpr typeInfo, Word value );

  static bool     OpenUGridCell( SmiRecord& valueRecord, 
                        size_t& offset, const ListExpr typeInfo, 
                        Word& value ); 
  
  static bool     SaveUGridCell( SmiRecord& valueRecord, size_t& offset, 
                        const ListExpr typeInfo, Word& w );

  static Word     CreateUGridCell( const ListExpr typeInfo );

  static void     DeleteUGridCell( const ListExpr typeInfo, Word& w );

  static void     CloseUGridCell( const ListExpr typeInfo, Word& w );

  static Word     CloneUGridCell( const ListExpr typeInfo, const Word& w );

  static bool     KindCheckUGridCell( ListExpr type, ListExpr& errorInfo );

  static int      SizeOfObjUGridCell();  
  
  static ListExpr PropertyUGridCell();
 public:
  UGridCell() {}

  friend class ConstructorFunctions<UGridCell>; 

public:
  int intlabel; 
  UGridArea ucarea; 
  int noOfEntries; 
  long firstTime;
  long lastTime;
  bool modified;
  HistoryUnit historyArray[sizeHistoryArray];
  CellUnit cellArray[sizeCellArray];
  Instant firstEntry;
  Instant lastEntry;
  Interval<Instant> ugcInterval;
};

/*
  Data Structure - Class UGrid and                
               the mandatory set of algebra                                                                                                      

*/
struct CellRecId
{
  int          celllabel;       // intlabel of stored ugridcell
  SmiRecordId  cellRecordId;    // recordId of stored ugridcell
};
struct ContRecId
{
  contsuffix   containerId;     // namesuffix of stored cellcontainer
  SmiRecordId  contRecordId;    // recordId of stored cellcontainer
};

class UGrid
{
  public:

/*
Constructors and destructor of the class ~UGrid~   

*/
  // The simple constructor.
  UGrid();

  // The constructor that receives all information to create master UGrid.
  UGrid( UGridArea area, const int pageSize );

  // The constructor that receives the fileId to open an existing UGrid.
  UGrid( const SmiFileId fileId );

  // The copy constructor.
  UGrid(UGrid& ug);

  // The slave constructor.
  UGrid(bool slave, UGrid& mug, UGrid& ug);

  // The destructor
  ~UGrid();

/*
Methods of the class ~UGrid~    

*/  
  // Open-Method of the ugrid that opens an ugrid.
  static UGrid*  Open( SmiRecord& valueRecord, size_t& offset,
                       const ListExpr instance);

  // Save-Method of the ugrid that saves an ugrid.
  ListExpr Save(SmiRecord& valueRecord, size_t& offset,
	            const ListExpr instance);

  // Deletes the file of the UGrid.
  inline void DeleteFile()
  {
    file.Close();
    file.Drop();
  }

  // Returns the ~SmiFileId~ of the UGrid database file
  inline SmiFileId FileId()
  {
    return file.GetFileId();
  }

  //Returns the ~SmiRecordId~ of the UGrid - contains only header infromations
  inline SmiRecordId UGridRecordId() const
  {
    return header.ugridRecordId;
  }

  // Returns the total number of cells in this UGrid.
  inline int NoOfCells() const
  {
    return header.noOfCells;
  }

  // Returns the total number of non empty cells in this UGrid.
  inline int NoOfModCells() const
  {
    return header.noOfModCells;
  }

  // Returns the total number of entries in this UGrid.
  inline int NoOfEntries() const
  {
    return header.noOfEntries;
  }

/*
private Methods of the class ~UGrid~ 

*/
  // Write the cells of ugrids
   void WriteCells (SmiRecord &cellRecord, CellRecId cont[maxCellRec],
	                UGridCell &ugc);

  // Read the cells of slave ugrids
  void ReadSlaveCells (UGrid &sug);
   
  // Write the cells of slave ugrids
  void WriteSlaveCells (UGrid *ugPtr);

  // Reads all ugridcells from file
  void ReadCells ();

  // Given that all ugridcells are resistent stored
  void SaveCells ();
 
/*
private fields of the class ~ugrid~ 

*/
  // The record file of the ugrid.
  SmiRecordFile file;
  // The header of the ugrid which will be written (read) to (from) the file.
  struct Header
  {
    SmiRecordId   ugridRecordId;        // ugrid header recordId  
    int           noOfCells;            // number of cells in this ugrid
	int           noOfModCells;         // number of non empty cells 
    int           noOfEntries;          // number of entries in this ugrid
    UGridArea     area;                 // euklidic region in this ugrid
	long          firstTime;            // timestamp of oldest entry
	long          lastTime;             // timestamp of newest entry
    ContRecId     contArray[maxCont];   // Id and RecordId of stored container
    UGrid         *ugcPtr;              // ptr to slave ugrid
    SmiRecordId   slaveUgridId;         // header recordId of slave ugrid

    Header() :
      ugridRecordId( 0 ), 
      noOfCells( 0 ), noOfModCells(0), noOfEntries( 0 ), area(0.0,0.0,0.0,0.0), 
	  firstTime(0), lastTime(0), ugcPtr (NULL),slaveUgridId ((SmiRecordId)0)
    { 
	  long zeit;
	  zeit = time(NULL);
	  firstTime = zeit;
	  lastTime = zeit+60;
	  for (int i = 0; i < maxCont; i++)
	  {
	    contArray[i].containerId = (contsuffix)0;
	    contArray[i].contRecordId = 0;
	  }
	}
    Header(UGridArea ugarea) :
      ugridRecordId( 0 ), 
      noOfCells( 0 ), noOfModCells(0), noOfEntries( 0 ), area(ugarea), 
	  firstTime(0), lastTime(0),ugcPtr (NULL),slaveUgridId ((SmiRecordId)0)
    { 
	  long zeit;
	  zeit = time(NULL);
	  firstTime = zeit;
	  lastTime = zeit+60;
      for (int i = 0; i < maxCont; i++)
	  {
	    contArray[i].containerId = (contsuffix)0;
	    contArray[i].contRecordId = 0;
	  }
	}
    Header( SmiRecordId rootRecordId, 
            int noOfCells, int noOfModCells, int noOfEntries, UGridArea area, 
			long firstEntry, long lastEntry):
            ugridRecordId( rootRecordId ),   
            noOfCells( noOfCells ), noOfModCells( noOfModCells ),
			noOfEntries( noOfEntries ), area( area ),
			firstTime( firstEntry ), lastTime( lastEntry ),
			ugcPtr (NULL),slaveUgridId ((SmiRecordId)0)
			
    {}
  } header;
  CellRecId cellContQ1[maxCellRec];      // label/RecNo of cells index = 
                                         // 0 - 255 
  CellRecId cellContQ2[maxCellRec];      // label/RecNo of cells index = 
                                         // 256-511 
  CellRecId cellContQ3[maxCellRec];      // label/RecNo of cells index = 
                                         // 512-767 
  CellRecId cellContQ4[maxCellRec];      // label/RecNo of cells index = 
                                         // 768-1023 
  CellRecId cellContQ5[maxCellRec ];     // Addresses of cells index = 
                                         // 1024 - 1279 in the ugrid
  CellRecId cellContQ6[maxCellRec ];     // Addresses of cells index = 
                                         // 1280 - 1535 in the ugrid
  CellRecId cellContQ7[maxCellRec ];     // Addresses of cells index = 
                                         // 1536 - 1791 in the ugrid
  CellRecId cellContQ8[maxCellRec ];     // Addresses of cells index = 
                                         // 1792 - 2047 in the ugrid
  CellRecId cellContQ9[maxCellRec ];     // Addresses of cells index = 
                                         // 2048 - 2303 in the ugrid
  CellRecId cellContQ10[maxCellRec ];    // Addresses of cells index = 
                                         // 2304 - 2559 in the ugrid
  CellRecId cellContQ11[maxCellRec ];    // Addresses of cells index = 
                                         // 2560 - 2815 in the ugrid
  CellRecId cellContQ12[maxCellRec ];    // Addresses of cells index = 
                                         // 2816 - 3071 in the ugrid
  CellRecId cellContQ13[maxCellRec ];    // Addresses of cells index = 
                                         // 3072 - 3327 in the ugrid
  CellRecId cellContQ14[maxCellRec ];    // Addresses of cells index = 
                                         // 3328 - 3583 in the ugrid
  CellRecId cellContQ15[maxCellRec ];    // Addresses of cells index = 
                                         // 3584 - 3839 in the ugrid
  CellRecId cellContQ16[maxCellRec ];    // Addresses of cells index = 
                                         // 3840 - 4095 in the ugrid
  DBArray<UGridCell> cells;              // Array of ugridcells in the 
                                         // ugrid (L0) 64x64 Grid mandatory
  Interval<Instant> ugInterval;          // timeinterval from earliest 
                                         // until latest entry
public:
  void ReadSlaveHeader(SmiRecordId sUgrid, UGrid* mugPtr);  
                                         // Reads the header of a slave ugrid

  void WriteSlaveHeader(SmiRecordId sUgrid, UGrid *ugPtr); 
                                         // Writes the header of a slave ugrid

  void WriteHeader();   // Writes the header of this ugrid

  void ReadHeader();    // Reads the header of this ugrid

// Static methods of the class ~ugrid~ for the type constructor
  static Word     InUGrid( const ListExpr typeInfo, const ListExpr instance,
                           const int errorPos, ListExpr& errorInfo, 
                           bool& correct );

  static ListExpr OutUGrid( ListExpr typeInfo, Word value );

  static bool     OpenUGrid( SmiRecord& valueRecord, 
                             size_t& offset, const ListExpr typeInfo, 
                             Word& value ); 
  
  static bool     SaveUGrid( SmiRecord& valueRecord, size_t& offset, 
                             const ListExpr typeInfo, Word& w );

  static Word     CreateUGrid( const ListExpr typeInfo );

  static void     DeleteUGrid( const ListExpr typeInfo, Word& w );

  static void     CloseUGrid( const ListExpr typeInfo, Word& w );

  static Word     CloneUGrid( const ListExpr typeInfo, const Word& w );

  static bool     CheckUGrid( ListExpr type, ListExpr& errorInfo );

  static int      SizeOfObjUGrid();  
  
  static ListExpr UGridProp();
};


} // endnamespace ugridAlgebra

#endif
