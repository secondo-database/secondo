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

//paragraph	[10]	title:		[{\Large \bf ] [}]
//paragraph	[21]	table1column:	[\begin{quote}\begin{tabular}{l}]	[\end{tabular}\end{quote}]
//paragraph	[22]	table2columns:	[\begin{quote}\begin{tabular}{ll}]	[\end{tabular}\end{quote}]
//paragraph	[23]	table3columns:	[\begin{quote}\begin{tabular}{lll}]	[\end{tabular}\end{quote}]
//paragraph	[24]	table4columns:	[\begin{quote}\begin{tabular}{llll}]	[\end{tabular}\end{quote}]
//[--------]	[\hline]
//characters	[1]	verbatim:	[$]	[$]
//characters	[2]	formula:	[$]	[$]
//characters	[3]	capital:	[\textsc{]	[}]
//characters	[4]	teletype:	[\texttt{]	[}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Storage Management Interface\\(Berkeley DB)

January 2002 Ulrich Telle

September 2002 Ulrich Telle, introduced flag for abort transaction after deadlock

April 2003 Ulrich Telle, implemented temporary SmiFiles

1.1 Overview

The *Storage Management Interface* provides all types and classes needed
for dealing with persistent data objects in "Secondo"[3]. The interface
itself is completely independent of the implementation. To hide the
implementation from the user of the interface, the interface objects use
implementation objects for representing the implementation specific parts.

The "Berkeley DB"[3] implementation of the interface uses several concepts to
keep track of the "Secondo"[3] databases and their ~SmiFiles~.

Since each "Berkeley DB"[3] environment needs several control processes (deadlock
detection, logging, recovery) the decision was taken to use only one "Berkeley
DB"[3] environment for managing an arbitrary number of "Secondo"[3] databases.

Named ~SmiFiles~ within each "Secondo"[3] database are managed in a simple
file catalog. 

1.1.1  "Secondo"[3] databases

In the root data directory resides a "Berkeley DB"[3] file named ~databases~
which contains entries for each "Secondo"[3] database. All files of a
"Secondo"[3] database are stored in a subdirectory of the "Berkeley DB"[3]
data directory. For each "Secondo"[3] database three "Berkeley DB"[3] files
hold the information about the ~SmiFile~ objects:

  *  *sequences* -- provides unique identifiers for ~SmiFile~ objects.

  *  *filecatalog* -- keeps track of all *named* ~SmiFile~ objects.
The unique identifier is used as the primary key for the entries.

  *  *fileindex* -- represents a secondary index to the file catalog.
The name of a ~SmiFile~ object combined with the context name is used
as the primary key for the entries.

1.1.2 File catalog

Each ~SmiFile~ object - whether named or anonymous - gets a unique
identifier when created. Since in a transactional environment objects
get visible to other users only when the enclosing transaction completes
successfully, the information about named ~SmiFile~ objects is queued
for processing after completion of the transaction. Requests for
deleting ~SmiFile~ objects are queued in a similar way.

File catalog updates and file deletions take place at the time a
transaction completes. The actions taken depend on whether the transaction
is committed or aborted. In case of a commit catalog updates and file
deletions are performed as requested by the user, in case of an abort
only those files which where created during the transaction are deleted,
not catalog update is performed.

1.1.3 "Berkeley DB"[3] handles

Each ~SmiFile~ object has an associated "Berkeley DB"[3] handle. Unfortunately
the "Berkeley DB"[3] requires that handles are kept open until after the
enclosing transaction completes. Since an ~SmiFile~ object may go out of
scope before, the destructor of the object must not close the handle. To
solve the problem the storage management environment provides a container
for "Berkeley DB"[3] handles. The constructor of an ~SmiFile~ object allocates
a handle by means of the environment methods ~AllocateDbHandle~ and
~GetDbHandle~, the destructor returns the handle by means of the environment
method ~FreeDbHandle~. After completion of the enclosing transaction the
environment method ~CloseDbHandles~ closes and deallocates all "Berkeley DB"[3]
handles no longer in use.

1.3 Implementation methods

The class ~SmiEnvironment::Implementation~ provides the following methods:

[23]    Database handling & Catalog handling  & File handling     \\
        [--------]
        LookUpDatabase    & LookUpCatalog     & ConstructFileName \\
        InsertDatabase    & InsertIntoCatalog & GetFileId         \\
        DeleteDatabase    & DeleteFromCatalog & EraseFiles        \\
                          & UpdateCatalog     & AllocateDbHandle  \\
                          &                   & GetDbHandle       \\
                          &                   & FreeDbHandle      \\
                          &                   & CloseDbHandles    \\

All other implementation classes provide only data members.

1.4 Imports, Constants, Types

*/

#ifndef SMI_BDB_H
#define SMI_BDB_H

#include <errno.h>
#include <queue>
#include <map>
#include <vector>

#include <db_cxx.h>

const u_int32_t CACHE_SIZE_STD = 128;
/*
Default cache size is 128 kB

*/
const u_int32_t CACHE_SIZE_MAX = 1024*1024;
/*
Maximum cache size is   1 GB

*/
/*
These are constants which define the default and maximum cache
size for the "Berkeley DB"[3] environment.

*/

typedef size_t DbHandleIndex;
/*
Is the type definition for indices of the "Berkeley DB"[3] handle array.

*/

const DbHandleIndex DEFAULT_DBHANDLE_ALLOCATION_COUNT = 10;
/*
Space for "Berkeley DB"[3] handles is reserved in chunks of "DEFAULT\_\-DBHANDLE\_\-ALLO\-CATION\_\-COUNT"[4]
elements to avoid frequent memory reallocation.

*/

const db_recno_t SMI_SEQUENCE_FILEID = 1;
/*
Identifies the record number of the ~FileId~ sequence.

*/

struct SmiDbHandleEntry
{
  Db*           handle;
  bool          inUse;
  DbHandleIndex nextFree;
};
/*
Defines the structure of the elements of the "Berkeley DB"[3] handle array.
Handles which are not ~inUse~ anymore are closed and freed after the completion
of a transaction. The free element is put on a free list for later reuse.

*/

struct SmiCatalogEntry
{
  SmiFileId fileId;
  char      fileName[2*SMI_MAX_NAMELEN+2];
  bool      isKeyed;
  bool      isFixed;
};
/*
Defines the structure of the entries in the file catalog.
The identifier ~fileId~, the name ~fileName~ and the type is stored for each
named ~SmiFile~.

*/

struct SmiDropFilesEntry
{
  SmiFileId fileId;
  bool      dropOnCommit;   
};
/*
Defines the structure of the elements in the queue for file drop requests.
Drop requests are fulfilled on successful completion of a transaction if
the flag ~dropOnCommit~ is set or on abortion of a transaction if this
flag is *not* set. In all other cases an entry is ignored.

*/

struct SmiCatalogFilesEntry
{
  SmiCatalogEntry entry;
  bool            updateOnCommit;
};
/*
Defines the structure of the elements in the map for file catalog requests.
Catalog requests are fulfilled on successful completion of a transaction if
the flag ~updateOnCommit~ is set or on abortion of a transaction if this
flag is *not* set. In all other cases an entry is ignored.

*/

/**************************************************************************
1.3 Class "SmiEnvironment::Implementation"[1]

This class handles all implementation specific aspects of the storage environment
hiding the implementation from the user of the ~SmiEnvironment~ class.

*/

class SmiEnvironment::Implementation
{
 public:
  static DbEnv* GetTempEnvironment() { return instance.impl->tmpEnv; };
  static DbHandleIndex AllocateDbHandle();
/*
Allocates a new "Berkeley DB"[3] handle and returns the index within the handle array.

*/
  static Db*  GetDbHandle( DbHandleIndex idx );
/*
Returns the "Berkeley DB"[3] handle at the position ~idx~ of the handle array.

*/
  static void FreeDbHandle( DbHandleIndex idx );
/*
Marks the "Berkeley DB"[3] handle at position ~idx~ as *not in use*.

*/
  static void CloseDbHandles();
/*
Closes all handles in the handle array which are not in use anymore.

*/
  static SmiFileId GetFileId( const bool isTemporary = false );
/*
Returns a unique file identifier.

*/
  static bool LookUpCatalog( const string& fileName,
                             SmiCatalogEntry& entry );
/*
Looks up a file named ~fileName~ in the file catalog. If the file was found, the
function returns "true"[4] and the catalog entry ~entry~ contains information about
the file like the file identifier.

*/
  static bool LookUpCatalog( const SmiFileId fileId,
                             SmiCatalogEntry& entry );
/*
Looks up a file identified by ~fileId~ in the file catalog. If the file was found,
the function returns "true"[4] and the catalog entry ~entry~ contains information about
the file like the file name.

*/
  static bool InsertIntoCatalog( const SmiCatalogEntry& entry,
                                 DbTxn* tid );
/*
Inserts the catalog entry ~entry~ into the file catalog.

*/
  static bool DeleteFromCatalog( const string& fileName,
                                 DbTxn* tid );
/*
Deletes the catalog entry ~entry~ from the file catalog.

*/
  static bool UpdateCatalog( bool onCommit );
/*
Updates the file catalog on completion of a transaction by inserting or deleting
entries collected during the transaction. The flag ~onCommit~ tells the function
whether the transaction is committed ("true"[4]) or aborted ("false"[4]).

*/
  static bool EraseFiles( bool onCommit );
/*
Erases all files on completion of a transaction for which drop requests were
collected during the transaction. The flag ~onCommit~ tells the function
whether the transaction is committed ("true"[4]) or aborted ("false"[4]).

*/
  static string ConstructFileName( SmiFileId fileId, const bool isTemporary = false );
/*
Constructs a valid file name using the file identifier ~fileId~.

*/
  static bool LookUpDatabase( const string& dbname );
/*
Looks up the Secondo database ~dbname~ in the database catalog.
The function returns "true"[4] if a database with the given name exists.

*/
  static bool InsertDatabase( const string& dbname );
/*
Inserts the name ~dbname~ of a new "Secondo"[3] database into the database catalog.
The function returns "true"[4] if the insert was successful.

*/
  static bool DeleteDatabase( const string& dbname );
/*
Deletes the name ~dbname~ of an existing "Secondo"[3] database from the database
catalog. The function returns "true"[4] if the deletion was successful.

*/

 protected:
  Implementation();
  ~Implementation();
 private:
  string    bdbHome;         // Home directory
  string    tmpHome;         // Temporary environment subdirectory
  u_int32_t minutes;         // Time between checkpoints 
  DbEnv*    bdbEnv;          // Berkeley DB environment handle
  DbEnv*    tmpEnv;          // Temporary environment handle
  SmiFileId tmpId;           // Temporary file ID
  bool      envClosed;       // Flag if environment is closed
  DbTxn*    usrTxn;          // User transaction handle
  bool      txnStarted;      // User transaction started
  bool      txnMustAbort;    // Abort transaction after deadlock
  Db*       bdbDatabases;    // Database Catalog handle
  Db*       bdbSeq;          // Sequence handle
  Db*       bdbCatalog;      // Database File Catalog handle
  Db*       bdbCatalogIndex; // Database Catalog Index handle

  bool      listStarted;
  Dbc*      listCursor;
  
  static u_int32_t AutoCommitFlag; // Influences the initialitation of Berkeley-DB
/*
Are needed to support listing the names of all existing "Secondo"[3] databases.

*/

  queue<SmiDropFilesEntry>         bdbFilesToDrop;
  map<string,SmiCatalogFilesEntry> bdbFilesToCatalog;
  vector<SmiDbHandleEntry>         dbHandles;
  DbHandleIndex                    firstFreeDbHandle;
  
  friend class SmiEnvironment;
  friend class SmiFile;
  friend class SmiRecordFile;
  friend class SmiKeyedFile;
  friend class SmiRecord;
};

/**************************************************************************
1.3 Class "SmiFile::Implementation"[1]

This class handles all implementation specific aspects of an ~SmiFile~
hiding the implementation from the user of the ~SmiFile~ class.

*/

class SmiFile::Implementation
{
  public:
  protected:
    Implementation();
    Implementation( bool isTemp );
    ~Implementation();
  private:
	  void CheckDbHandles();   // reallocate Db-Handles if necessary
    DbHandleIndex bdbHandle; // Index in handle array
    Db*           bdbFile;   // Berkeley DB handle
    bool          isSystemCatalogFile;
    bool          isTemporaryFile;
		bool					noHandle;
/*
Flags an ~SmiFile~ as a system catalog file. This distinction is needed,
since transactional read operations on system catalog files could lead 
easily to deadlock situations by the way the transaction and locking
mechanism of the "Berkeley DB"[3] works. Therefore read operation on system
catalog files should not be protected by transactions.

*/
  friend class SmiFile;
  friend class SmiFileIterator;
  friend class SmiRecordFile;
  friend class SmiKeyedFile;
  friend class SmiRecord;
};

/**************************************************************************
1.3 Class "SmiFileIterator::Implementation"[1]

This class handles all implementation specific aspects of an ~SmiFileIterator~
hiding the implementation from the user of the ~SmiFileIterator~ class.

*/

class SmiFileIterator::Implementation
{
 public:
 protected:
  Implementation();
  ~Implementation();
 private:
  Dbc* bdbCursor;  // Berkeley DB cursor

  friend class SmiFile;
  friend class SmiFileIterator;
  friend class SmiRecordFile;
  friend class SmiRecordFileIterator;
  friend class SmiKeyedFile;
  friend class SmiKeyedFileIterator;
};

/**************************************************************************
1.3 Class "SmiRecord::Implementation"[1]

This class handles all implementation specific aspects of an ~SmiRecord~
hiding the implementation from the user of the ~SmiRecord~ class.

*/

class SmiRecord::Implementation
{
 public:
 protected:
  Implementation();
  ~Implementation();
 private:
  Db*  bdbFile;      // Berkeley DB handle
  Dbc* bdbCursor;    // Berkeley DB cursor
  bool useCursor;    // Flag use cursor in access methods
  bool closeCursor;  // Flag close cursor in destructor

  friend class SmiFile;
  friend class SmiFileIterator;
  friend class SmiRecordFile;
  friend class SmiKeyedFile;
  friend class SmiRecord;
};

/**************************************************************************
1.3 Class "PrefetchingIteratorImpl"[1]

This class handles all implementation specific aspects of a 
~PrefetchingIterator~ hiding the implementation details 
from the user of the ~PrefetchingIterator~ class.

*/
class PrefetchingIteratorImpl : public PrefetchingIterator
{
private:
  enum SearchT {RANGE, LEFTRANGE, RIGHTRANGE, ALL};
  enum StateT {INITIAL, PARTIAL_RETRIEVAL, BULK_RETRIEVAL, BROKEN}; 
  
  SearchT searchType;
  StateT state;
  bool isBTreeIterator;
  int errorCode;
/*
This class explicitly maintains its state. The preceding declarations
support this state maintenance.

*/

  Dbc* dbc;

  char leftBoundary[SMI_MAX_KEYLEN];
  char rightBoundary[SMI_MAX_KEYLEN];
  size_t leftBoundaryLength;
  size_t rightBoundaryLength;

  char keyBuffer[SMI_MAX_KEYLEN];
  char* bufferPtr;
  Dbt keyDbt;
  Dbt buffer;
  db_recno_t recordNumber;
  
  void* retKey;
  void* retData;
  size_t retKeyLength;
  size_t retDataLength;

  void* p; /* needed and managed by Berkeley DB */
    
  bool NewPrefetch();
  SmiSize BulkCopy(void* data, size_t dataLength, 
    void* userBuffer, SmiSize nBytes, SmiSize offset);
  bool RightBoundaryExceeded();
  void Init(Dbc* dbc, const size_t bufferLength,
    bool isBTreeIterator);

protected:
  virtual void GetKeyAddressAndLength(void** addr, SmiSize& length);
  
public:

  PrefetchingIteratorImpl(Dbc* dbc, SmiKey::KeyDataType keyType, 
    const size_t bufferLength = DEFAULT_BUFFER_LENGTH,
    bool isBTreeIterator = true);

  PrefetchingIteratorImpl(Dbc* dbc, SmiKey::KeyDataType keyType,
    const char* leftBoundary, size_t leftBoundaryLength,
    const char* rightBoundary, size_t rightBoundaryLength,
    const size_t bufferLength = DEFAULT_BUFFER_LENGTH);

  virtual ~PrefetchingIteratorImpl();

  virtual bool Next();
  SmiSize ReadCurrentData(void* userBuffer, SmiSize nBytes, SmiSize offset = 0);
  SmiSize ReadCurrentKey(void* userBuffer, SmiSize nBytes, SmiSize offset = 0);
  void ReadCurrentRecordNumber(SmiRecordId& recordNumber);
  Dbc *GetCursor(); 
  int ErrorCode();

  static const size_t DEFAULT_BUFFER_LENGTH = 64 * 1024;
};

#endif

