/*
//paragraph	[10]	title:		[{\Large \bf ] [}]
//paragraph	[11]	title:		[{\large \bf ] [}]
//paragraph	[12]	title:		[{\normalsize \bf ] [}]
//paragraph	[21]	table1column:	[\begin{quote}\begin{tabular}{l}]	[\end{tabular}\end{quote}]
//paragraph	[22]	table2columns:	[\begin{quote}\begin{tabular}{ll}]	[\end{tabular}\end{quote}]
//paragraph	[23]	table3columns:	[\begin{quote}\begin{tabular}{lll}]	[\end{tabular}\end{quote}]
//paragraph	[24]	table4columns:	[\begin{quote}\begin{tabular}{llll}]	[\end{tabular}\end{quote}]
//[--------]	[\hline]
//characters	[1]	verbatim:	[$]	[$]
//characters	[2]	formula:	[$]	[$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [$\leq$]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: PagedArray


August 2003 M. Spiekermann 

1.1 Overview

This module offers a generic persistent array implemented as a template class on top of the
SecondoSMI interface. Many slots of the array are stored inside a fixed sized Berkeley-DB
record which is a multiple of the operating systems pagesize. All records IDs which contain
array slots are hold in a vector in main memory. The currently used record is bufferd in a memory
array reducing SMI calls.

Currently this tool can not be used for saving and reconstructing large arrays, but it is
useful to use it temporary instead of main memory.  Therefore some code snipets are
commented out and hav to be revised for usage as persistent array. 

Since the record-size is an attribute of the record-file, this size is defined 
at construction time of the file and therefore a parameter for the constructor of this class.


1.2 Interface methods

This module offers the following methods:

[23]	Creation/Removal 	& Access   	& Inquiries	\\ 	
	[--------]
	PagedArray        		& Get 		& Size		\\  	
	[tilde]PagedArray		& Put		& Id		\\
	MarkDelete		&		& 		\\

Operations have to follow the protocol shown below:

		Figure 1: Protocol [Protocol.eps]

1.3 Class ~PagedArray~

An instance of the class is a handle to a persistent array of fixed size with
elements of type ~T~.

*/

#ifndef PAGED_ARRAY_H
#define PAGED_ARRAY_H

#include <iostream> 
#include <sstream> 
#include <fstream> 
#include <cassert>
#include <vector>
#include <map>

#include "SecondoSMI.h"

typedef unsigned long Cardinal;

extern unsigned int FileCtr;  // quick and dirty!, sorry

template<class T>
class PagedArray
{
 public:

  PagedArray( SmiRecordFile *parrays, bool logon=false );

/*
Creates creates a new ~SmiRecord~ on the ~SmiRecordFile~ for this
persistent array. One can define an initial size of the persistent
array with the argument ~initsize~. 

*/
  
  PagedArray( SmiRecordFile *parrays, const SmiRecordId id, const bool update = true );

/*
Opens the ~SmiRecordFile~ and the ~SmiRecord~ for the persistent array. The boolean 
value ~update~ indicates if open mode: ~true~ for update and ~false~ for read-only.

*/

  ~PagedArray();

/*
Destroys the handle. If the array is marked for deletion, then it also destroys the
persistent array.

*/

  void MarkDelete();

/*
Marks the persistent array for deletion. It will be permanently deleted on the 
destruction of the object.

*Precondition:* The array must be opened in update mode.

*/

  void Put(Cardinal const index, T& elem);

/*
Copies element ~elem~ into the persistent array at index ~index~.

*Precondition:* 0 [<=] ~index~ [<=] ~size~ - 1. The array must be opened in update mode.

*/

  void Get(Cardinal const index, T& elem);

/*
Returns the element ~index~ of the array.

*Precondition:* 0 [<=] ~index~ [<=] ~size~ - 1. 

*/

  Cardinal Size();

/*
Returns the size of this array.

*/

  const SmiRecordId Id() const;

/*
Returns the identifier of this array.

*/

 unsigned long PageChanges() {  
     unsigned long swap = log.pageChangeCounter; 
     log.pageChangeCounter=0;    
     return swap; 
 }

  unsigned long SlotAccess() {  
     unsigned long swap = log.slotAccessCounter; 
     log.slotAccessCounter=0;    
     return swap; 
 }



 private:

  
  static const int MAX_PAGE_FRAMES = 4;

  // the next method loads another record if neccessary and calculates a slot number inside a record
  inline void GetSlot( Cardinal const index, int &slot );

  bool writeable;
  bool canDelete;
 
  SmiRecord record;
  SmiRecordId recid;
 
  SmiRecordFile *parrays;
  vector<SmiRecordId> recidVec;

  Cardinal size;
  Cardinal maxPageNo;
  
  typedef map<int, T*> PageMapType;
  PageMapType pageTable;
  typedef typename PageMapType::iterator PageMapIter;
  PageMapIter it, frameChangeIter; 

  Cardinal frameTable[MAX_PAGE_FRAMES];

  typedef struct {
    
    int size;
    int slotSize;
    int slots;

  } PageRecordInfo;

  typedef struct {

    int no;
    int nextFrame; 
    T *bufPtr; 
  
  } PageInfo;
 
  PageInfo currentPage;
  PageRecordInfo pageRecord;
  

  typedef struct {

    bool switchedOn;
    ofstream *filePtr;
    stringstream fileName;
    unsigned long pageChangeCounter; 
    unsigned long slotAccessCounter;

  }  LogInfo;

  LogInfo log;

};


/*
2 Implementation of PagedArray

Version: 0.7

August 2003 M. Spiekermann 

2.1 Overview

This module offers a generic persistent array implemented on top of the
SecondoSMI interface.

*/


template<class T>
PagedArray<T>::PagedArray( SmiRecordFile *parrays, bool logOn /*=false*/ ) :
writeable( true ),
canDelete( false ),
parrays( parrays ),
size( 0 )
{
  log.switchedOn = logOn;

  log.pageChangeCounter = 0;
  log.slotAccessCounter = 0;

  maxPageNo = 0;

  pageRecord.size = parrays->GetRecordLength();
  pageRecord.slotSize = sizeof(T);
  pageRecord.slots =  pageRecord.size / pageRecord.slotSize;

  for (int i=0; i < MAX_PAGE_FRAMES; i++) {

    assert( parrays->AppendRecord( recid, record ) );
    record.Finish();
    pageTable[i] =  new T[ pageRecord.slots ];
    frameTable[i] = i;
    recidVec.push_back( recid );
    maxPageNo++;
  }
  size = MAX_PAGE_FRAMES * pageRecord.slots;

  currentPage.no = 0;
  currentPage.bufPtr = pageTable[0];
  currentPage.nextFrame = 0;  

  if ( log.switchedOn ) {

    FileCtr++;
    log.fileName << "PagedArray_" << FileCtr << "_" << pageRecord.slots << ".log" << ends; 
    log.filePtr = new ofstream( log.fileName.str().c_str() );  
  }

}

/*
template<class T>
PagedArray<T>::PagedArray( SmiRecordFile *parrays, const SmiRecordId id, const bool update ) :
writeable( update ),
canDelete( false ),
parrays( parrays )
{
  SmiFile::AccessType at = update ? SmiFile::Update : SmiFile::ReadOnly;
  assert( parrays->SelectRecord( id, record, at ) );
  recid = id;
  record.Read( &size, sizeof( int ) );
}
*/

template<class T>
PagedArray<T>::~PagedArray()
{
  for (it = pageTable.begin(); it != pageTable.end(); it++) {
     
     recid = recidVec[ it->first ];
     record.Finish();
     assert( parrays->SelectRecord(recid, record, SmiFile::Update) );
     record.Write( it->second, pageRecord.size, 0);
     
     delete [] it->second;
  }

  if ( log.switchedOn ) {
    delete log.filePtr;
  }
/*
  if ( canDelete ) 
  {
    parrays->DeleteRecord( recid );
  }
  else if ( writeable )
  {
    record.Write( &size, sizeof( int ) );
  } 
*/
}

/*
The function below calculates the page number which holds a specific array
index and determines if this page is still in the memory buffer or if one
of the pages in memory has to be substituted by a page on disk.
*/

template< class T>
void PagedArray<T>::GetSlot(Cardinal const index, int &slot )
{
  static int pageNo = 0;
  bool pageChange = false;
 

  /* enlarge the array if necessary */ 
  if (index >= size) {
    
     assert( parrays->AppendRecord( recid, record ) );
     recidVec.push_back(recid);
     maxPageNo++;
     size = size + pageRecord.slots;
  } 

  /* calculate page number */
  pageNo = index / pageRecord.slots;
  
  if (currentPage.no != pageNo ) {

  it = pageTable.find( pageNo );

  if ( it == pageTable.end() ) { /* substitute memory buffer */

     frameChangeIter = pageTable.find( frameTable[ currentPage.nextFrame ] );

     T* bufPtr = frameChangeIter->second;
     int removePageNo = frameChangeIter->first;
 
     recid = recidVec[ removePageNo ];
     record.Finish();
     assert( parrays->SelectRecord(recid, record, SmiFile::Update) );

     record.Write( bufPtr, pageRecord.size, 0);
     
     recid = recidVec[ pageNo ];
     record.Finish();
     assert( parrays->SelectRecord(recid, record, SmiFile::Update) );
     record.Read( bufPtr, pageRecord.size, 0 );
    
     pageTable[pageNo] = bufPtr; 
     pageTable.erase(frameChangeIter);
     frameTable[currentPage.nextFrame] = pageNo;

     currentPage.bufPtr = bufPtr;
     currentPage.no = pageNo;

     if ( log.switchedOn ) {
       *(log.filePtr) << index << " | PageChange: " << frameChangeIter->first << " -> " << pageNo; 
       pageChange = true;
       log.pageChangeCounter++;
     }
  
     currentPage.nextFrame++;
     if (  currentPage.nextFrame == MAX_PAGE_FRAMES ) {
        currentPage.nextFrame = 0;
     }
     /*  Note: A cyclic move inside the buffers cannot be implemented with an iterator for
      *  the map elements since the insert and erase method will corrupt this iterator       
      */
 
  } else { /* update page number and page frame address */

     currentPage.no = pageNo;
     currentPage.bufPtr = it->second; 
  }

  }

  slot = index - (currentPage.no * pageRecord.slots);
  assert ( slot >= 0 && slot < pageRecord.slots);
  
  if ( log.switchedOn ) {
     log.slotAccessCounter++;
     if ( pageChange ) {
        *(log.filePtr) << " | " << pageNo << "/" << slot << endl; 
     }
  }

}



template<class T>
void PagedArray<T>::Put(Cardinal const index, T& elem)
{
  static int slot = 0;

  assert ( writeable );
  assert ( index <= size );

  GetSlot(index, slot);
  currentPage.bufPtr[slot] = elem;
}


template<class T>
void PagedArray<T>::Get(Cardinal const index, T& elem)
{
  static int slot = 0;
  
  assert ( 0 <= index && index < size );
  
  GetSlot(index, slot); 
  elem = currentPage.bufPtr[slot];
}

template<class T>
void PagedArray<T>::MarkDelete() 
{
  assert( writeable );
  canDelete = true;
}


template<class T>
Cardinal PagedArray<T>::Size() 
{
  return size;
}


template<class T>
const SmiRecordId PagedArray<T>::Id() const 
{ 
  return recid;
}

#endif
