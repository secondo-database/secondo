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

#ifndef PARRAY_H
#define PARRAY_H

#include <iostream> 
#include <cassert>
#include "SecondoSMI.h"
#include <vector>

typedef unsigned long Cardinal;

template<class T>
class PagedArray
{
 public:

  PagedArray( SmiRecordFile *parrays );

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

 int PageChanges() {  
     int swap = pageChangeCounter; 
     pageChangeCounter=0;    
     return swap; 
 }

 private:

  void GetSlot( Cardinal const index, int &slot );
  // Loads another record if neccessary and calculates a slot number inside a record

  bool writeable;
 
  SmiRecord record;
  SmiRecordId recid;
 
  Cardinal size;
 
  int recordSize;
  int slotSize;
  int slotsPerRecord;
  int currentPage;
  int pageChangeCounter; 
 
  vector<SmiRecordId> recidVec;
  T *pageBufPtr;  

  bool canDelete;
  SmiRecordFile *parrays;

};


/*
2 Implementation of PagedArray

Version: 0.7

August 2002 RHG

2.1 Overview

This module offers a generic persistent array implemented on top of the
SecondoSMI interface.

*/

template<class T>
PagedArray<T>::PagedArray( SmiRecordFile *parrays ) :
writeable( true ),
size( 0 ),
pageChangeCounter( 0 ),
canDelete( false ),
parrays( parrays )
{
  assert( parrays->AppendRecord( recid, record ) );
  assert( recordSize = parrays->GetRecordLength() );

  slotSize = sizeof(T);
  slotsPerRecord =  recordSize/slotSize;
  //cerr << "PagedArray() slotsPerRecord: " << slotsPerRecord;
  
  pageBufPtr = new T[slotsPerRecord];
  recidVec.push_back(recid);
  //cerr << "PagedArray() recidVec: " << recidVec.size() << endl;

  currentPage = 0;
  size = slotsPerRecord;
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
  record.Write( pageBufPtr, recordSize, 0);
  record.Finish();
  delete [] pageBufPtr;
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

template< class T>
void PagedArray<T>::GetSlot(Cardinal const index, int &slot )
{
  static int pageNo = 0;
  
  pageNo=index/slotsPerRecord;
  
  if ( pageNo != currentPage) {

     record.Write( pageBufPtr, recordSize, 0);
     record.Finish();
     recid = recidVec[pageNo];
     currentPage = pageNo;
     assert( parrays->SelectRecord(recid, record, SmiFile::Update) );
     record.Read( pageBufPtr, recordSize, 0 );

     pageChangeCounter++;
  }  
  
  slot = index - (currentPage * slotsPerRecord);
  
  //cerr << "GetSlot - pageNo/slotsPerRecord - index/slot: " 
  //     << pageNo << "/" << slotsPerRecord << " - " << index << "/" << slot << endl; 
  assert ( slot >= 0 && slot < slotsPerRecord );
}



template<class T>
void PagedArray<T>::Put(Cardinal const index, T& elem)
{
  static int slot=0;

  assert ( writeable );
  assert ( index < size+slotsPerRecord );

  if (index >= size) {
    
     record.Write( pageBufPtr, recordSize, 0);
     record.Finish();
     assert( parrays->AppendRecord( recid, record ) );
     recidVec.push_back(recid);
     currentPage++;
     size = size + slotsPerRecord;
  } 

  GetSlot(index, slot);

  pageBufPtr[slot] = elem;
  //record.Write( &elem, slotSize, slot*slotSize );
}


template<class T>
void PagedArray<T>::Get(Cardinal const index, T& elem)
{
  static int slot = 0;
  assert ( 0 <= index && index < size );
  
  GetSlot(index, slot); 

  elem = pageBufPtr[slot];
  //record.Read(&elem, slotSize, slot*slotSize);
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
