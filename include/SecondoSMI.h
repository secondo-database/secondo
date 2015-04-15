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

//paragraph [10]  title:            [{\Large \bf ] [}]
//paragraph [21]  table1column:     [\begin{quote}\begin{tabular}{l}]   [\end{tabular}\end{quote}]
//paragraph [22]  table2columns:    [\begin{quote}\begin{tabular}{ll}]  [\end{tabular}\end{quote}]
//paragraph [23]  table3columns:    [\begin{quote}\begin{tabular}{lll}] [\end{tabular}\end{quote}]
//paragraph [24]  table4columns:    [\begin{quote}\begin{tabular}{llll}][\end{tabular}\end{quote}]
//[--------]      [\hline]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[_] [\_]
//[tilde] [\verb|~|]

1 Header File: Storage Management Interface

April 2002 Ulrich Telle

November 30, 2002 RHG Added function ~GetKey~.

April, 2004. F.Hoffmann changed implementation details of static method
'ListDatabases'.

Aug 18, 2004. M. Spiekermann added ~Setflag\_NOSYNC~ to speed up closing files
at the end of a query. Since queries does not modify the data synchronisation is
not necessary.

Sept 15, 2004. M. Spiekermann. Declaration of SmiError moved to ErrorCodes.h.

Nov 2004. M. Spiekermann. Some functions were implemented as inline functions.

Feb. 2006. M. Spiekermann Some new functions were declared which allow to set up
only a temporary BDB-Environment needed for the implementation of persistent
nested lists.


1.1 Overview

The *Storage Management Interface* provides all types and classes needed
for dealing with persistent data objects in "Secondo"[3].

Essential for all operations on persistent data objects is the class
~SmiEnvironment~ which provides methods for startup and shutdown of
the storage management, for transaction handling and for error handling.

Internally the ~SmiEnvironment~ handles the connection to the underlying
implementation of the persistent information storage management. Currently
implementations based on the "Berkeley DB"[3] library and on the "Oracle"[3] DBMS
are available. The decision which implementation is used is taken at link
time of the "Secondo"[3] server. Fine tuning of implementation dependent
parameters is done by means of a configuration file which is read at system
startup. For future extensions (i.e. user management and access control)
it is possible to store the current user identification in the ~SmiEnvironment~.

Additionally the ~SmiEnvironment~ introduces the concept of ~databases~. Within
one ~SmiEnvironment~ there may exist several databases, but a user may access
only *one* database at a single time. A valid database name consists of at most
"SMI\_MAX\_DBNAMELEN"[4] (currently 15) alphanumeric characters or underscores. The
first character has to be alphabetic. Database names are *not* case sensitive.

Persistent objects are stored as records in so called ~SmiFiles~. An ~SmiFile~
is a handle to its representation in the interface implementation. Two kinds
of ~SmiFiles~ are provided: one for record oriented access and one for key
oriented access. An ~SmiFile~ always has a ~context~, default or user specified.
The context allows to adjust implementation dependent parameters to benefit
from special knowledge about the objects to be stored in this context. These
parameters are specified in the configuration file under the appropriate
context section heading.

An ~SmiFile~ is always represented by a unique numeric identifier. Additionally
it may have a name. Without a name an ~SmiFile~ is said to be ~anonymous~. Within
a context a name must be unique. ~SmiFile~ names and context names are case
sensitive and may have at most "SMI\_MAX\_NAMELEN"[4] (currently 31) alphanumeric
characters or underscores. The first character has to be alphabetic.

An ~SmiFile~ for record oriented access is called ~SmiRecordFile~. Records can be
of fixed or variable size. The use of ~SmiFiles~ with fixed length records is
recommended whereever appropriate since they may be more efficient -- depending
on the implementation. Records can only be appended to the end of an ~SmiFile~,
but can be accessed for reading, update or deletion in random order using their
record number. An iterator for sequential access to all records of an ~SmiFile~ is
provided.

An ~SmiFile~ for key oriented access is called ~SmiKeyedFile~, which is itself
divided into two kinds, called ~SmiBtreeFile~ and ~SmiHashFile~, depending on
the storage method, a B-Tree and a Hash, respectively. An ~SmiKeyFile~ is
capable of holding pairs of keys and data. Both keys and data may be of variable
size. While the size of the data is only restricted by physical limits of the
available hardware, the size of keys is restricted to at most 4000 bytes.
(This limit is imposed by the VARCHAR2 datatype of "Oracle"[3], but depending on the
actual database configuration the maximal possible key length might be lower.
Other relational database systems allow only much smaller keys, i.e. "MySQL"[3]
restricts the combined key length to 512 bytes, but a single column key cannot
exceed 255 bytes and that would be the restriction for an implementation based
on "MySQL"[3].)

As key types signed integers, floating point numbers, and strings are supported.
Keys may also have a more complex structure as long as the user provides a
function for mapping a key to a byte string which obeys to a lexical order.
Keys may be unique or may allow for duplicate data. The use of iterators is
mandatory for access to duplicates. Iterators for access to all records or
records within a specified range of keys are provided.

An ~SmiRecord~ handle is used to access complete or partial records of an ~SmiFile~.

1.2 Transaction handling

The storage management interface provides methods to start, commit and abort
transactions. All access to persistent objects should be done within user
transactions. User transactions are *not* started automatically. Due to limitations
of the transaction support of the "Berkeley DB"[3] it is recommended that applications
using this interface should run in a sort of auto-commit mode by surrounding
very few atomic operations with calls to the start and commit transaction methods.
Only application which are able to repeat transactions easily in case of failure
may use more complex transactions.

Although both current implementations of the storage
management interface support transaction logic for data manipulation statements,
there is no support for transaction logic for data definition statements
(creation and deletion of ~SmiFiles~). Therefore the update of the SmiFile
catalog and the deletion of ~SmiFiles~ are postponed until completion or rollback
of a transaction. If the ~SmiFile~ catalog update fails, the transaction is
aborted and rolled back automatically.

1.3 Interface methods

The class ~SmiEnvironment~ provides the following methods:

[23]    Environment     & Transaction handling & Error handling     \\
        [--------]
        StartUp         & BeginTransaction     & CheckLastErrorCode \\
        ShutDown        & CommitTransaction    & GetLastErrorCode   \\
        SetUser         & AbortTransaction     & SetError           \\
        CreateDatabase  &  &  \\
        OpenDatabase    &  &  \\
        CloseDatabase   &  &  \\
        EraseDatabase   &  &  \\
        ListDatabases   &  &  \\
        IsDatabaseOpen  &  &  \\
        CurrentDatabase &  &  \\

The classes ~SmiRecordFile~ and ~SmiKeyedFile~ inherit the following methods
from their base class ~SmiFile~:

[23]    Creation/Removal & Open/Close & Information \\
        [--------]
  Create           & Open        & GetContext \\
  Drop             & Close       & GetName    \\
  Truncate         &             & GetFileId  \\
                   &             & IsOpen     \\

The class ~SmiRecordFile~ provides the following methods:

[23]    Creation/Removal     & Record Selection & Record Modification \\
        [--------]
        SmiRecordFile        & SelectRecord     & AppendRecord \\
        [tilde]SmiRecordFile & SelectAll        & DeleteRecord \\
        GetData              &                  &               \\

The class ~SmiKeyedFile~ provides the following methods:

[23]    Creation/Removal    & Record Selection & Record Modification \\
        [--------]
        SmiKeyedFile        & SelectRecord     & InsertRecord \\
        [tilde]SmiKeyedFile &                  & DeleteRecord \\

The class ~SmiBtreeFile~ provides the following methods:

[23]    Creation/Removal    & Record Selection  \\
        [--------]
        SmiBtreeFile        & SelectRange       \\
        [tilde]SmiBtreeFile & SelectLeftRange   \\
                            & SelectRightRange  \\

The class ~SmiBtreeFile~ provides only the creation and removal functions, since
all its functionality is provided in the ~SmiKeyedFile~:

[23]    Creation/Removal    \\
        [--------]
        SmiHashFile        \\
        [tilde]SmiHashFile \\

The class ~SmiRecord~ provides the following methods:

[23]    Creation/Removal & Persistence & Querying \\
        [--------]
        SmiRecord        & Read        & Size \\
        [tilde]SmiRecord & Write       &      \\
        Finish           & Truncate    & Resize \\

The classes ~SmiRecordFileIterator~ and ~SmiKeyedFileIterator~ provide the
following methods:

[23]    Creation/Removal             & Access        & Querying \\
        [--------]
        SmiRecordFileIterator        & Next          & EndOfScan \\
        [tilde]SmiRecordFileIterator & DeleteCurrent &  \\
        SmiKeyedFileIterator         & Restart       &  \\
        [tilde]SmiKeyedFileIterator  & Finish        &  \\

The class ~SmiKey~ provides the following methods:

[23]    Creation/Removal & Access      & Key mapping   \\
        [--------]
        SmiKey           & GetKey      & Map           \\
        [tilde]SmiKey    & KeyDataType & Unmap         \\

1.4 Imports, Constants, Types

*/

#ifndef SECONDO_SMI_H
#define SECONDO_SMI_H



#include <string>
#include <vector>
#include <map>
#include <utility>

#include <db_cxx.h>
#include <string.h>

#include "ErrorCodes.h"
#include "SecondoConfig.h"
#include "CacheInfo.h"


using namespace std;


class IndexableAttribute;

const string::size_type SMI_MAX_NAMELEN      =   31;
/*
Specifies the maximum length of a context or file name.

*/

const string::size_type SMI_MAX_DBNAMELEN    =   15;
/*

Specifies the maximum length of a database name.

*/

const string::size_type SMI_MAX_KEYLEN       = 3200;
/*
Specifies the maximum length of keys.

*NOTE*: The maximum length of keys depends on several factors,
namely the storage management system and physical properties
of the underlying system:

[23]    System      & Blocksize & Max. key length  \\
        [--------]
        Berkeley DB &           & 4000 \\
        MySQL       &           &  255 \\
        Oracle 8i   &  2 kB     &  758 \\
        Oracle 8i   &  4 kB     & 1578 \\
        Oracle 8i   &  8 kB     & 3218 \\
        Oracle 8i   & 16 kB     & 4000 \\

*/

const string::size_type SMI_MAX_KEYLEN_LOCAL =   32;
/*
Specifies the maximum length of keys which are stored locally within an
instance of the ~SmiKey~ class. Extra memory is allocated for longer keys.

*/


typedef unsigned long SmiFileId;
/*
Is the type for the unique file identifiers.

*/

typedef db_recno_t SmiRecordId;
/*
Is the type for record identifiers.

*/

typedef size_t SmiSize;
/*
Is the type for record sizes or offsets.

*/

class PrefetchingIterator;
/* Forward declaration */

typedef vector<pair<string,string> > SmiStatResultType;
/* Used by SmiFile::GetFileStatistics(...) */

enum SMI_STATS_MODE {SMI_STATS_EAGER, SMI_STATS_LAZY};
/* Used by SmiFile::GetFileStatistics(...) */


/**************************************************************************
1.3 Class "SmiKey"[1]

The class ~SmiKey~ is used to store key values of different types in a
consistent manner. Key values are restricted in length to at most
"SMI\_MAX\_KEYLEN"[4] bytes. If the length of the key value is less than
"SMI\_MAX\_KEYLEN\_LOCAL"[4] the key value ist stored within the class
instance, otherwise memory is allocated.

*/

class SMI_EXPORT SmiKey
{
 public:
  enum KeyDataType
    { Unknown, Integer, Longint, Float, String, Composite };
/*
Lists the types of key values supported by the ~SmiFiles~ for keyed access:

  * *Unknown* -- not a true type, designates an uninitialized key instance

  * *RecNo* -- a record number of a ~SmiRecordFile~

  * *Integer* -- signed integer number (base type ~int32[_]t~)

  * *Longint* -- signed integer number (base type ~int64[_]t~)

  * *Float* -- floating point number (base type ~double~)

  * *String* -- character string (base type ~string~)

  * *Composite* -- user-defined key structure, the user has to provide a mapping
function which is called to map the key structure to a byte string which can
be sorted like a usual string in lexical order. On key retrieval the function
is called to unmap the byte string to the user-defined key structure.

*/
 static KeyDataType getKeyType(const int32_t){ 
    return Integer;
 }
 static KeyDataType getKeyType(const uint32_t){ 
    return Integer;
 }
 static KeyDataType getKeyType(const int64_t){ 
    return Longint;
 }
 static KeyDataType getKeyType(const uint64_t){ 
    return Longint;
 }
 static KeyDataType getKeyType(const double){ 
    return Float;
 }
 static KeyDataType getKeyType(const string&){ 
    return String;
 }
 static KeyDataType getKeyType(const IndexableAttribute*){
      return Composite;
 }

  SmiKey();
  SmiKey( const int32_t key );
  SmiKey( const int64_t key );
  SmiKey( const uint32_t key );
  SmiKey( const uint64_t key );
  SmiKey( const double key );
  SmiKey( const string& key );
  SmiKey( const IndexableAttribute* key );
  SmiKey( const SmiKey& other );
/*
Creates a key with a type according to the constructor argument.

*/
  ~SmiKey();
/*
Destroys a key.

*/
  SmiKey& operator=( const SmiKey& other );
  const bool operator==( const SmiKey& other ) const;
  const bool operator>( const SmiKey& other ) const;
  const KeyDataType GetType() const;
/*
Returns the type of the key.

*/
  bool GetKey( int32_t& key )const;
  bool GetKey( int64_t& key )const;
  bool GetKey( uint32_t& key )const;
  bool GetKey( uint64_t& key )const;
  bool GetKey( double& key )const;
  bool GetKey( string& key )const;
  bool GetKey( IndexableAttribute* key )const;
/*
Returns the value of the key. The argument type must match the type of the key!

*/
  SmiSize GetLength() const;
/*
Returns the length of the key if the key is a composite key, otherwise -1 is
returned.

*/
  static void Map( const int32_t   inData, void* outData );
  static void Map( const int64_t   inData, void* outData );
  static void Map( const uint32_t   inData, void* outData );
  static void Map( const uint64_t   inData, void* outData );
  static void Map( const double inData, void* outData );
  static void Unmap( const void* inData, int32_t&   outData );
  static void Unmap( const void* inData, int64_t&   outData );
  static void Unmap( const void* inData, uint32_t&   outData );
  static void Unmap( const void* inData, uint64_t&   outData );
  static void Unmap( const void* inData, double& outData );
/*
These functions are provided for convenience. They may be used in user-defined
mapping functions to map integer and floating-point numbers to lexical byte
strings and vice versa.

*/
 protected:
 private:
  void  FreeData();
/*
Frees the memory allocated for a key, if memory was previously allocated.
The function is called internally when a new key value is assigned.

*/
  const void* GetAddr() const;
/*
Returns the memory address of the key value.

*/
  void  SetKey( const int32_t key );
  void  SetKey( const int64_t key );
  void  SetKey( const uint32_t key );
  void  SetKey( const uint64_t key );
  void  SetKey( const double key );
  void  SetKey( const string& key );
  void  SetKey( const IndexableAttribute* key );
  void  SetKey( const KeyDataType kdt,
                const void* key, const SmiSize keyLen );
/*
Sets the internal key value to the passed key value, setting also the key type.

*/

  KeyDataType keyType;   // Type of the key value
  SmiSize     keyLength; // Size of the key value in bytes
  union                  // Structure for storing the key
  {
    int32_t     integerKey;
    int64_t     longintKey;
    double      floatKey;
    char        shortKeyData[SMI_MAX_KEYLEN_LOCAL+1];
    char*       longKeyData;
  };

  friend class SmiFile;
  friend class SmiFileIterator;
  friend class SmiRecordFile;
  friend class SmiKeyedFile;
  friend class SmiBtreeFile;
  friend class SmiKeyedFileIterator;
  friend class SmiRecord;
  friend class PrefetchingIterator;
};




/**************************************************************************
1.3 Class "SmiFile"[1]

This class provides the methods common to both ~SmiRecordFiles~ and
~SmiKeyedFiles~.

*/

class SMI_EXPORT SmiFile
{
 public:
  enum FileType   { FixedLength, VariableLength, KeyedBtree, KeyedHash };
/*
Is an enumeration of possible file types:

  * *FixedLength* -- Files of this type consist of a set of records all having a
fixed size which cannot be changed.
A record is filled with binary null characters if not the whole record is written.
Depending on the implementation this file type allows for better locking characteristics
in multi-user environments. Records are identified by record numbers.

  * *VariableLength* -- Files of this type consist of a set of records of potentially
varying size. Records are identified by record numbers.

  * *Keyed* -- Files of this type consist of key/data pairs.

*/
  enum AccessType { ReadOnly,    Update         };
/*
Is an enumeration of possible access types:

  * *ReadOnly* -- Records are selected for read access only. Operations
which change the contents or the size of a record are not permitted.

  * *Update* -- Records are selected for read and/or write access.

*/

  bool Create( const string& name, const string& context = "Default",
      uint16_t pageSize = 0, const bool keepId = false );
/*
Creates a new SmiFile with given name. 
If the name is an empty string, the name is constructed automatically.
The context string should be one
of them available in the underlying storage manager. If the pagesize
is set to 0 (default), the page size of the filesystem is used. 
If keepId is set to true, the old fileId is reused instead of creating
a new one for this file.

*/

  bool Create( const string& context = "Default", uint16_t pageSize = 0 );

/*

Creates a new anonymous ~SmiFile~.
Optionally a ~context~ can be specified. If pageSize equals 0, the size
configured in SecondoConfig.ini will be used otherwise a system dependent
default value is used.

*/

 bool ReCreate();

/*
Removes the underlying file and creates a new one with same name, context, 
pagesize, and fileId.

*/


  bool Open( const SmiFileId id, const string& context = "Default" );
/*
Opens an existing anonymous ~SmiFile~ using its file identifier ~id~.
Optionally a ~context~ can be specified.

*/
  bool Open( const string& name, const string& context = "Default" );
/*
Opens an existing named ~SmiFile~ or creates a new named ~SmiFile~ if it does not
exist. Optionally a ~context~ can be specified.

*/
  bool Close(const bool sync = true );
/*
Closes an open ~SmiFile~.

*/
  bool Drop();
/*
Erases a ~SmiFile~. It is necessary to close any record iterators or record
handles before dropping a ~SmiFile~.

*/
  bool Truncate();
/*
Empties an ~SmiFile~. It is necessary to close any record iterators or record
handles before truncating it. This method is only used to free disk data for
tuple buffers since the Remove() operation does not work for them (bug?).
Without this compromise solution the data would only be deleted when secondo shuts down.

*/
  bool Remove();
/*
Removes a ~SmiFile~ from disk. It is necessary to close any record iterators or record
handles before calling this function.

*/

  string GetContext();
/*
Returns the context of the ~SmiFile~.

*/
  string GetName();
/*
Returns the name of a named ~SmiFile~ or an empty string for an anonymous ~SmiFile~.

*/
  SmiFileId GetFileId();
/*
Returns the unique ~SmiFile~ identifier.

*/
  SmiSize GetRecordLength() { return fixedRecordLength; }

/*
Returns the length of fixed Records. In the case of variable record length 0 is returned.

*/  
  uint16_t GetPageSize() const;
/*
Returns  the page size of the file.

*/

  bool   IsOpen();
/*
Returns whether the ~SmiFile~ handle is open and can be used to access the
records of the ~SmiFile~.

*/
  ostream& Print(ostream& os) const;

  SmiStatResultType GetFileStatistics(const SMI_STATS_MODE mode);

/*
Returns a SmiStatResultType, which is a vector of key-value pairs. Both, keys
and values are strings. Each ~key~ describes a statistic on the file and the
~value~ the according value.

~mode~ is of type SMI\_STATS\_MODE: either SMI\_STATS\_EAGER, SMI\_STATS\_LAZY.
~SMI\_STATS\_EAGER~ will force the active collection of statistics to ensure
that current and complete data is returned (that migth take a while, for data
may needed to be analyzed), while ~SMI\_STATS\_LAZY~ will only read out (possibly
old) statistics and/or return incomplete data.

Different SmiFile types may return different sets of keys as results.

*/

  bool IsTemp();

  string GetFileName() const {
    return fileName;
  }


 protected:
  SmiFile( const bool isTemporary = false);
  SmiFile( const SmiFile& smiFile );
  ~SmiFile();
  bool CheckName( const string& name );
  bool useTxn;
/*
Checks whether the given name ~name~ is valid.

*/

  bool        opened;               // Open state of SmiFile
  string      fileContext;          // Name of file context
  string      fileName;             // Name of named SmiFile
  SmiFileId   fileId;               // Unique file identifier

  FileType    fileType;             // Type of SmiFile records
  SmiSize     fixedRecordLength;    // Length of records with
                                    //   fixed length
  bool        uniqueKeys;           // Uniqueness of keys
  SmiKey::KeyDataType keyDataType;  // Data type of keys

  class Implementation;
  Implementation* impl;

 private:
  bool trace;


  friend class SmiEnvironment;
  friend class SmiFileIterator;
  friend class SmiRecord;
};

ostream& operator<<(const ostream& os, const SmiFile& f);



class SMI_EXPORT SmiCachedFile : public SmiFile
{

  protected:
  SmiCachedFile( const bool isTemporary, const uint64_t cache_sz = 0 );

  SmiCachedFile( const SmiCachedFile& f );
  ~SmiCachedFile();

  uint64_t GetCacheSize() const;
/*
Returns the cache size of the file.

*/

  private:
  uint64_t cacheSize; 

  friend class SmiEnvironment;
  friend class SmiFileIterator;
  friend class SmiRecord;

};

/*

1 Declaration of the Storage Management Interface for UrelAlgebra

1.1 Overview

The storage management interface of the UrelAlgebra
supports basic I/O methods for let urel object to access
the memory pool file in the Berkely DB environment.
This is one part of the whole storage management interface
of the Secondo system.

The interface is composed by two classes,  *SmiUpdateFile*
and *SmiUpdatePage*. The SmiUpdateFile provides methods
for accessing the whole file of an urel object. The main methods
are listed below:

  * Create

  * Open

  * Close

  * AppendNewPage

  * GetPage

  * PutPage

Create is used to create a new memory pool file, and use Open
method to access an exist memory pool file, then use Close method
to flush all unsaved data to the disk and release the handle to the file.

AppendNewPage is used to add a new page in this file, and use the
GetPage method to map an exist page in the disk file to the main memory,
and lock that part of memory so that different processes can share
the same page, that's what we call pin that page in the memory.
When the page is useless, then use PutPage method to flush the data back
to the disk file, and unpin the page.

The SmiUpdatePage provides access methods to one page in the
memory pool file. There are two main methods: read and write,
with which we can read and write any type of data in this page.

1.2 Definition of Class SmiUpdatePage

*/

class SMI_EXPORT SmiUpdatePage
{
public:
  SmiUpdatePage();
  SmiUpdatePage(const SmiUpdatePage& rhs);
  ~SmiUpdatePage();

/*

3.1.1 The write method of SmiUpdatePage

Write the data in the ~buffer~ to the page at the ~offset~ place.

*/

  SmiSize  Write(const void* buffer, const SmiSize numberOfBytes,
      SmiSize offset = 0)
  {
    memcpy((char*) pagePt + offset, buffer, numberOfBytes);
    return numberOfBytes;
  }
/*

3.1.2 The read method of SmiUpdatePage

Read the data at the ~offset~ place of the page back to the ~buffer~.

*/

  SmiSize  Read(void* buffer, const SmiSize numberOfBytes,
      SmiSize offset = 0)
  {
    memcpy(buffer, (char*) pagePt + offset, numberOfBytes);
    return numberOfBytes;
  }

  template<typename T>
    inline SmiSize
    Write(T& buffer)
    {
      SmiSize n = Write(&buffer, sizeof(T));
      return n;
    }
  template<typename T>
    inline SmiSize
    Read(T& buffer)
    {
      SmiSize n = Read(&buffer, sizeof(T));
      return n;
    }

  //auxiliary functions
  void*  GetPageAddr();
  db_pgno_t  GetPageNo();
  SmiSize  GetPageSize();
  bool  isAvailable();

private:
  db_pgno_t pageNo; //Indicate the page number inside the file
  SmiSize pageSize;
  void* pagePt; //The pointer point to the update page

  friend class SmiUpdateFile;
};

/*

1.3 Definition of Class SmiUpdateFile

The first several pages of the memory pool file is called ~system page~,
which is used to store some systematic information of the file.
At present there is only one system page, and only count how many
processes are sharing this file.The page number of the update file
is started from 1.

For effective access the memory, the pointers of all pages got from
the disk file will be kept in a STL map structure, so that we can get the
memory pointer directly.

*/

struct SmiUpdateSysPage
{
  SmiUpdateSysPage() :
    shareByNum(0)
  {}
  int shareByNum; //count the number of processes sharing this file
};

class SMI_EXPORT SmiUpdateFile : public SmiFile
{
public:
  SmiUpdateFile();
  ~SmiUpdateFile();
  SmiUpdateFile(SmiSize _poolPageSize);

  bool  Open(const string& name); //only use in test example
  bool  Open(const SmiFileId _fID, const SmiSize _pSize,
      bool isInitialized = true);
  //used to reopen an exist file
  bool  Close();
  bool  Create(const string& context = "Default", uint16_t pageSize = 0);
  //used to create a new file

  //New methods about getting and putting pages
  bool AppendNewPage(SmiUpdatePage*& page);
  bool GetPage(const db_pgno_t pageNo, SmiUpdatePage*& page);
  bool PutPage(const db_pgno_t pageNo, bool isChanged = true);

  //Auxiliary functions ...
  SmiSize  GetPoolPageSize();
  int  GetSysPageNum();
  bool  GetIniStatus();
  int  GetExistPageNum();

private:
  DbMpoolFile *dbMpf; //The pointer to the memory pool file

  SmiSize poolPageSize;
  // The page size of the memory pool file
  int sysPageNum;
  // The number of pages need to record system info
  int existPageNum;
  // The numbers of pages exist in the disk file currently
  bool isInitialized;
  // Make sure that this file is not empty.
  map<db_pgno_t, SmiUpdatePage*> gotPages;

  bool  SyncFile();
  bool  InitializePoolFile();
  int  GetNumOfShareProcess();
  int  GetFactPageNum();

  //Get how many processes are sharing this file
  bool  RegisterInFile();
  bool  UnRegisterInFile();
};



/**************************************************************************
1.3 Class "SmiEnvironment"[1]

This class handles all aspects of the environment of the storage
environment including the basics of transactions.

*/

class SMI_EXPORT SmiEnvironment
{
 public:
  enum SmiType { SmiBerkeleyDB, SmiOracleDB };
/*
Enumerates the different implementations of the storage management interface:

  * *SmiBerkeleyDB* -- Implementation based on the "Berkeley DB"[3]

  * *SmiOracleDB* -- Implementation based on "Oracle"[3]

*/
  enum RunMode { SingleUserSimple, SingleUser,
                 MultiUser, MultiUserMaster };
/*
Lists the types of run modes supported by the ~SmiEnvironment~:

  * *SingleUserSimple* -- access to the ~SmiEnvironment~ is restricted to
exactly *one* single process. Not observing this restriction can
cause unpredictable behavior and possibly database corruption.
Transactions and logging are usually disabled. Using this mode is not
recommended, except for read-only databases.

  * *SingleUser* -- access to the ~SmiEnvironment~ is restricted to
exactly *one* single process. Not observing this restriction can
cause unpredictable behavior and possibly database corruption.
Transactions and logging are enabled.

  * *MultiUser* -- the ~SmiEnvironment~ may be accessed by more than
one process. Transactions, logging and locking are enabled.

  * *MultiUserMaster* -- the ~SmiEnvironment~ may be accessed by more
than one process. Transactions, logging and locking are enabled.
This mode should be used by the process which acts as the dispatcher
for client requests to allow additional implementation dependent
initialization.

*NOTE*: In any multi user mode the "Secondo"[3] registrar must be running.
The behaviour of the storage management system in single user mode is
implementation dependent.

*/
  static map<SmiError, string> errorMap;
/*
A table containing the error map strings indexed by error numbers (~SmiError~).
The function ~Err2Msg~ retrieves the strings from this table.

*/
  static bool errorMapInitialized;
/*
A flag that indicates whether the error map table is already initialized.

*/
  static SmiType GetImplementationType();
/*
Returns the implementation type of the storage management interface.

*NOTE*: This information is availabe before calling the ~StartUp~ method,
thus allowing to perform implementation dependent activities.

*/
  static SmiEnvironment* GetInstance();
/*
Returns a pointer to the "Secondo"[3] Storage Management Environment
(seldom needed).

*/

  static bool SetHomeDir(const string& parmFile);

  static string GetSecondoHome();

  static int CreateTmpEnvironment(ostream& err);
  static int DeleteTmpEnvironment();

  static bool StartUp( const RunMode mode,
                       const string& parmFile,
                       ostream& errStream );
/*
Initializes the ~SmiEnvironment~ of the storage manager interface.
Parameters are read from the configuration file ~parmFile~.
Error messages are written to the provided output stream ~errStream~.
A user transaction is implicitly started.

*/
  static bool ShutDown();
/*
Shuts down the storage manager interface. An open user transaction is aborted
implicitly. It is necessary to close *all* open ~SmiFiles~, iterators and
record handles before shutting down the system.

*/
  static bool IsDatabaseOpen();
/*
Returns "true"[4] if a database is currently open, otherwise "false"[4] is returned.

*/
  static string CurrentDatabase();
/*
Returns the name of the currently open database. If no database is open
a blank string is returned.

*/
  static bool CreateDatabase( const string& dbname );
/*
Creates a new "Secondo"[3] database under the name ~dbname~.
The function returns "true"[4], if the database could be created;
it returns "false"[4], if a database with the given name already
exists or if an error occured.

*/
  static SI_Error OpenDatabase( const string& dbname );
/*
Opens an existing "Secondo"[3] database having the name ~dbname~.
The functions returns "true"[4], if the database could be opened,
otherwise it returns "false"[4].

*/
  static bool CloseDatabase();
/*
Closes a previously created or opened "Secondo"[3] database.
The function returns "true"[4], if the database could be closed successfully.
Otherwise it returns "false"[4].

*/
  static bool EraseDatabase( const string& dbname );
/*
Erases the "Secondo"[3] database named ~dbname~.
The function returns "true"[4], if the database could be erased.
Otherwise it returns "false"[4].

*NOTE*: It is an error to call this function, if a database is already
open, regardless of the name.

*/
  static bool ListDatabases( string& dbname );
/*
Lists the names of existing databases in ~dbname~, separated by a '\#'
character. Returns true, if the execution was successful.

*/
  static bool SetUser( const string& uid );
/*
Stores the identification ~uid~ of the current user in the ~SmiEnvironment~.
In a future extension it may be used for user management and access control.

*/

  static void SetFlag_NOSYNC(const bool value) { dontSyncDiskCache = value; }
/*
Indicates that the cache in memory and the files on disk need no syncronisation.
In the Berkeley-DB Implementation this is used to speed up the DB-close() API call
at the end of a query which does no modifications to the stored data.

*/

  static bool BeginTransaction();
  static bool CommitTransaction(const bool closeDbHandles = true);
  static bool AbortTransaction();
/*
Are provided for transaction handling. Transactions are never implicitly started.
Therefore an explicit call to ~BeginTransaction~ is always necessary. To be able
to rollback requests for deleting ~SmiFiles~ such requests are registered throughout
the transaction and carried out only if the transaction completes successfully.
~Named SmiFiles~ are registered in a file catalog. Changes to this catalog take
place when a transaction is committed. When updates to the file catalog fail,
the transaction is implicitly aborted.

*/
  static int GetNumOfErrors();
  static SmiError GetLastErrorCode();
  static SmiError GetLastErrorCode( string& errorMessage );
/*
Returns the error code of the last storage management operation.
~CheckLastErrorCode~ provides the error code without resetting the internal
error code while the other functions reset the internal error code.
Optionally the accompanying error message is returned.

*/
  static void SetSmiError( const SmiError smiErr,
                           const string& file, int pos );
  static void SetSmiError( const SmiError smiErr,
                           const int sysErr, const string& file, int pos );
  static void ResetSmiErrors();

#define SetError(code) SetSmiError(code, __FILE__, __LINE__)
#define SetError2(code, msg) SetSmiError(code, msg, __FILE__, __LINE__)
#define SetBDBError(code) SetSmiError(E_SMI_BDB, code, __FILE__, __LINE__)

/*
Allows to set an SmiError code and a system error code or an error message.

*/
  static bool GetCacheStatistics(CacheInfo& ci, vector<FileInfo*>& fi);


  static void UpdateCatalog();
  

static void SetAutoRemoveLogs(const bool enable);
/*
Forces the deletion of non required log files. Disables catastropic recovery.

*/

  static const string Err2Msg( SmiError code );
/*
Translate an SMI error code into a message!

*/

  static bool correctFileId();

 
 private:

  SmiEnvironment();
/*
Creates an instance of the ~Storage Management Interface~ environment.

*/
  ~SmiEnvironment();
/*
Destroys a ~Storage Management Interface~ environment.

*/
  SmiEnvironment( SmiEnvironment& );
/*
The copy constructor is not implemented.

*/
  static bool SetDatabaseName( const string& dbname );
/*
Checks whether the given database name ~dbname~ is valid or not,
converts the name to all lower case and stores the converted name in a
member variable.
The function returns "true"[4], if the name is valid.

A valid database name consists of at most "SMI\_MAX\_DBNAMELEN"[4] (currently 15)
alphanumeric characters or underscores. The first character has to be alphabetic.
Database names are *not* case sensitive.

*/
  static bool InitializeDatabase();
/*
Initializes a new database.

*/
  static bool RegisterDatabase( const string& dbname );
/*
Registers the database ~dbname~ when it is created or opened.
The function returns "true"[4], if the database could be registered successfully
or if the application runs in single user mode.

*NOTE*: The registration is necessary to protect a database from accidental deletion
by another user.

*/
  static bool UnregisterDatabase( const string& dbname );
/*
Unregisters the database ~dbname~.
The function returns "true"[4], if the database could be unregistered successfully
or if the application runs in single user mode.

*/
  static bool LockDatabase( const string& dbname );
/*
Locks the database ~dbname~.
The function returns "true"[4], if a lock could be acquired successfully,
i.e. no other user accesses the database,
or if the application runs in single user mode.

*NOTE*: Before a database can be erased it has to be locked.

*/
  static bool UnlockDatabase( const string& dbname );
/*
Unlocks the database ~dbname~.
The function returns "true"[4], if the lock could be released successfully
or if the application runs in single user mode.

*/
  static void SetSmiError( const SmiError smiErr,
      const string& errMsg, const string& file, int pos );

  static bool CallRegistrar( const string& dbname, 
      const string& cmd, string& answer );





/*
Auxiliary function to handle communication with the registrar process

*/

  static SmiEnvironment instance;    // Instance of environment
  static SmiError       lastError;   // Last error code
  static string         lastMessage; // Last error message
  static int            numOfErrors; // Last error message
  static bool           smiStarted;  // Flag SMI initialized
  static bool           singleUserMode;
  static bool           useTransactions;

  // the next member is used in the Berkeley-DB Implementation
  static bool           dontSyncDiskCache;

  static string         configFile;  // Name of config file
  static string         uid;         // ID of Secondo user
  static bool           dbOpened;    // Flag database opened
  static string         database;    // Name of current database
  static string         registrar;   // Name of the registrar
  static SmiType        smiType;     // Implementation type

  class Implementation;
  Implementation* impl;

  friend class Implementation;
  friend class SmiFile;
  friend class SmiFile::Implementation;
  friend class SmiFileIterator;
  friend class SmiRecordFile;
  friend class SmiKeyedFile;
  friend class SmiBtreeFile;
  friend class SmiRecord;
  friend class SmiUpdateFile;
};

/**************************************************************************
1.3 Class "SmiRecord"[1]

This class provides a record handle for processing data records. A record
contains user-defined byte strings of arbitrary length. A record handle is
initialized by the appropriate methods of a ~SmiFile~. After initialization
the handle can be used to access complete or partial records.

*/

class SMI_EXPORT SmiRecord
{
 public:
  SmiRecord();
/*
Creates a record handle. A handle is passed to ~SmiFile~ or ~SmiFileIterator~
methods for initialization and can be used thereafter to access a record.

*/
  ~SmiRecord();
/*
Destroys a record handle.

*/
  SmiSize  Read( void* buffer,
                 const SmiSize numberOfBytes,
                 const SmiSize offset = 0 );

  template <typename T>
  inline SmiSize Read(T& buffer) {
    SmiSize n = Read(&buffer, sizeof(T), pos);
    pos += sizeof(T);
    return n;
  } 
/*
Reads a sequence of at most ~numberOfBytes~ bytes from the record into the ~buffer~
provided by the user. Optionally a record ~offset~ can be specified.
The amount of bytes actually transfered is being returned.

*/

  char* GetData(SmiSize& length);
/*
Returns the complete content of that record. The caller of that function is responsible
to free the data pointer.

*/



  SmiSize  Write( const void*   buffer,
                  const SmiSize numberOfBytes,
                  const SmiSize offset = 0 );

  template <typename T>
  inline SmiSize Write(T& buffer) {
    
    SmiSize n = Write(&buffer, sizeof(T), pos);
    pos += sizeof(T);
    return n;
  } 


/*
Writes a sequence of at most ~numberOfBytes~ bytes into the record from the ~buffer~
provided by the user. Optionally a record ~offset~ can be specified.
The amount of bytes actually transfered is being returned.

*/
  SmiSize  Size();
  void SetPos(SmiSize n) { pos = n; }
  SmiSize GetPos() { return pos; }
/*
Retrieves the total amount of bytes stored within the persistent representation
of this record.

*/

  SmiKey GetKey();

/*
Returns the key of this record.

*/
  bool  Truncate( const SmiSize newSize );
/*
Truncates the record to a specified length of ~newSize~ bytes.
The function returns "false"[4] if newSize is greater than the current record's
length.

*/

  bool Resize(const SmiSize newSize);
/*
 Resizes this Record. In constrast to the Truncate function also enlargements
 od of the records are possible.

*/

  SmiRecordId GetId() const{
    SmiRecordId res;
    recordKey.GetKey(res);
    return res;
  }

/*
Returns the record id of this record.

*/



  void Finish();
/*
Finishes the operation on the associated record. The record handle may be
reused (i.e. reinitialized) afterwards. It is usually not necessary to call this method
when the record handle is not reused, since the destructor will call it implicitly.

*NOTE*: If a transaction would end before the destructor is called it is
essential to explicitly call this method.

*/
 protected:
 private:
  class Implementation;
  Implementation* impl;
  SmiFile* smiFile;       // associated SmiFile object
  SmiKey   recordKey;     // Key (or record no.) of this record
  SmiSize  recordSize;    // Total size of the record
  SmiSize  pos;           // current start position (offset)
  bool     fixedSize;     // Record has fixed length
  bool     initialized;   // Handle is initialized
  bool     writable;      // Record is writable

  friend class SmiFile;
  friend class SmiFileIterator;
  friend class SmiRecordFile;
  friend class SmiRecordFileIterator;
  friend class SmiKeyedFile;
  friend class SmiKeyedFileIterator;
};

/**************************************************************************
1.3 Class "SmiRecordFile"[1]

The class ~SmiRecordFile~ allows record oriented access to persistent objects.
New records can only be appended to a ~SmiRecordFile~, but existing records can
be processed in random order using their record numbers.

By means of an iterator it is possible to scan through all records of a ~SmiFile~.
The records are obtained in the order they were appended to the file.

*/

class SmiRecordFileIterator;  // Forward declaration

class SMI_EXPORT SmiRecordFile : public SmiCachedFile
{
 public:
  SmiRecordFile( const bool hasFixedLengthRecords,
                 const SmiSize recordLength = 0,
                 const bool isTemporary = false );

/*
Creates a handle for an ~SmiRecordFile~. The handle is associated with an
~SmiFile~ by means of the ~Create~ or ~Open~ method.

*/
  ~SmiRecordFile();
/*
Destroys a handle of an ~SmiRecordFile~.

*/
  bool SelectRecord( const SmiRecordId recno,
                     SmiRecord& record,
                     const SmiFile::AccessType accessType =
                           SmiFile::ReadOnly );
/*
Selects the record identified by the record number ~recno~ for read only
or update access. The user has to provide a record handle which is
initialized by this method.

*/

bool Read(const SmiRecordId recno,
               void* buffer,
               const SmiSize length,
               const SmiSize offset,
               SmiSize& actSize);

/*
Reads data from a specified record. This will accelerate
operations if the record is only used local.

*/

 bool Write(const SmiRecordId recno,
           const void* buffer,
           const SmiSize length,
           const SmiSize offset,
           SmiSize& written);

/*
Writes data to a specified record.

*/


  char* GetData(const SmiRecordId recno,
                SmiSize& length, 
                const bool dontReportError);
/*
Retrieves all data from the record selected by the RecordId.
The caller of that function is responsible for deleting the
returned data using free. length will return the size of the
returned data block. If that operation fails, for example because
the record id does not exist, the return value will be null.


*/



  bool SelectAll( SmiRecordFileIterator& iterator,
                  const SmiFile::AccessType accessType =
                        SmiFile::ReadOnly);

/*
Initializes an iterator for sequentially accessing all records of the ~SmiFile~.
The requested access type - read only or update - has to be specified.

*/
  PrefetchingIterator* SelectAllPrefetched();
/*
The same as ~SelectAll~, but returns a ~PrefetchingIterator~.

A ~PrefetchingIterator~ is read-only.

*/

  bool AppendRecord( SmiRecordId& recno,
                     SmiRecord& record );
/*
Appends a new record to the ~SmiFile~ and returns a record handle pointing to
the new record.

*/
  bool DeleteRecord( const SmiRecordId recno );
/*
Deletes the record identified by record number ~recno~ from the file.

*/

 protected:
 private:
};

/**************************************************************************
1.3 Class "SmiKeyedFile"[1]

The class ~SmiKeyedFile~ allows key oriented access to persistent objects.
Records are defined as key/data pairs. A key may have one of the types
Integer, Float, String or Composit. Data may have an arbitrary length.

By means of an iterator it is possible to scan through some or all records
of an ~SmiKeyedFile~. The records are retrieved in ascending order of the key
values. For duplicate records no special ordering is supported.

*/

struct SmiKeyRange {

  SmiKeyRange() : less(0), equal(0), greater(0) {}

  double less, equal, greater;
};

/*
A structure for storing proportional values of key ranges used in
method ~KeyRange~.

*/

class SmiKeyedFileIterator;  // Forward declaration

class SMI_EXPORT SmiKeyedFile : public SmiCachedFile
{
 public:
  SmiKeyedFile( const SmiFile::FileType fileType,
    const SmiKey::KeyDataType keyType,
    const bool hasUniqueKeys,
    const bool isTemporary );
/*
Creates a ~SmiFile~ handle for keyed access. The keys have to be of the
specified type ~keyType~. If ~hasUniqueKeys~ is true, then for each key
only one record can be stored. Otherwise duplicate records are allowed.

*/
  virtual ~SmiKeyedFile() = 0;
/*
Destroys a file handle.

*/
  SmiKey::KeyDataType GetKeyType() const;
/*
Returns the key type of the file.

*/
  bool SelectRecord( const SmiKey& key,
                     SmiKeyedFileIterator& iterator,
                     const SmiFile::AccessType accessType =
                       SmiFile::ReadOnly );
/*
Selects the data record with the given key. It is required to specify
whether the select takes place for read only or update of the record.
This method always initializes a record iterator regardless whether
keys are unique or not. Usually this method should only be used when
duplicate records are supported.

The function returns "true"[4] when at least one record exists for the given key.

*/
  bool SelectRecord( const SmiKey& key,
                     SmiRecord& record,
                     const SmiFile::AccessType accessType =
                       SmiFile::ReadOnly );
/*
Selects the data record with the given key and initializes a ~SmiRecord~
handle for processing the record. In is required to specify whether the select
takes place for read only or update of the record. In case of
duplicates only the first record is returned with access type ~ReadOnly~,
regardless of the specified access type!

The function returns "true"[4] when a record exists for the given key.

*/
  bool InsertRecord( const SmiKey& key, SmiRecord& record );
/*
Inserts a new record for the given key.
An ~SmiRecord~ handle is initialized on return to write the record.

The function returns "true"[4] when the record was created successfully.

*/
  bool DeleteRecord( const SmiKey& key,
                     const bool all = true,
                     const SmiRecordId = 0 );
/*
Deletes all records having the given key.

The function returns "true"[4] when the records were successfully deleted.

*/

};

/**************************************************************************
1.3 Class "SmiHashFile"[1]

The class ~SmiHashFile~ uses hashing as access method for the ~SmiKeyedFile~.
It does not provide any method, since all its functionality is
implemented in the parent class.

*/

class SMI_EXPORT SmiHashFile : public SmiKeyedFile
{
 public:
  SmiHashFile( const SmiKey::KeyDataType keyType,
                const bool hasUniqueKeys = true,
                const bool isTemporary = false );
/*
Creates a ~SmiFile~ handle for keyed access using hashing as access method.
The keys have to be of the specified type ~keyType~. If ~hasUniqueKeys~ is
true, then for each key only one record can be stored. Otherwise duplicate
records are allowed.

*/
  ~SmiHashFile();
/*
Destroys a file handle.

*/

};

/**************************************************************************
1.3 Class "SmiBtreeFile"[1]

The class ~SmiBtreeFile~ uses B-Tree as access method for the
~SmiKeyedFile~. Additionally to the selection methods provided by
the parent class, it provides several range selection methods.

*/
class SMI_EXPORT SmiBtreeFile : public SmiKeyedFile
{
 public:
  SmiBtreeFile( const SmiKey::KeyDataType keyType,
                const bool hasUniqueKeys = true,
                const bool isTemporary = false );
/*
Creates a ~SmiFile~ handle for keyed access. The keys have to be of the
specified type ~keyType~. If ~hasUniqueKeys~ is true, then for each key
only one record can be stored. Otherwise duplicate records are allowed.

*/
  ~SmiBtreeFile();
/*
Destroys a file handle.

*/

  bool KeyRange( const SmiKey& key, SmiKeyRange& range ) const;

/*
Returns three double values stored in the reference argument range, 
between [0,1] which represent the proportion of all keys stored in 
the file having a less, equal or greater key value than the given
key passed as first argument.

*/



  bool SelectRange( const SmiKey& fromKey, const SmiKey& toKey,
                    SmiKeyedFileIterator& iterator,
                    const SmiFile::AccessType accessType =
                          SmiFile::ReadOnly,
                    const bool reportDuplicates = false );
/*
Selects a range of records with keys for which the following condition holds
"fromKey [<=] key [<=] toKey"[1]. A record iterator for processing the selected
records is initialized on return.

By default an iterator for read only access without reporting duplicates is initialized,
but update access and reporting of duplicates may be specified.

The function returns "true"[4] when the iterator was initialized successfully.

*/

  PrefetchingIterator*
    SelectRangePrefetched(const SmiKey& fromKey, const SmiKey& toKey);
/*
The same as ~SelectRange~, but returns a ~PrefetchingIterator~.

A ~PrefetchingIterator~ is read-only. Duplicates are always
reported.

*/

  bool SelectLeftRange( const SmiKey& toKey,
                        SmiKeyedFileIterator& iterator,
                        const SmiFile::AccessType accessType =
                              SmiFile::ReadOnly,
                        const bool reportDuplicates = false );
/*
Selects a range of records with keys for which the following condition holds
"key [<=] toKey"[1]. A record iterator for processing the selected records
is initialized on return.

By default an iterator for read only access without reporting duplicates is initialized,
but update access and reporting of duplicates may be specified.

The function returns "true"[4] when the iterator was initialized successfully.

*/

  PrefetchingIterator* SelectLeftRangePrefetched(const SmiKey& toKey);
/*
The same as ~SelectLeftRange~, but returns a ~PrefetchingIterator~.

A ~PrefetchingIterator~ is read-only. Duplicates are always
reported.

*/

  bool SelectRightRange( const SmiKey& fromKey,
                         SmiKeyedFileIterator& iterator,
                         const SmiFile::AccessType accessType =
                               SmiFile::ReadOnly,
                         const bool reportDuplicates = false );
/*
Selects a range of records with keys for which the following condition holds
"fromKey [<=] key"[1]. A record iterator for processing the selected records
is initialized on return.

By default an iterator for read only access without reporting duplicates is initialized,
but update access and reporting of duplicates may be specified.

The function returns "true"[4] when the iterator was initialized successfully.

*/

  PrefetchingIterator* SelectRightRangePrefetched(const SmiKey& fromKey);
/*
The same as ~SelectRightRange~, but returns a ~PrefetchingIterator~.

A ~PrefetchingIterator~ is read-only. Duplicates are always
reported.

*/

  bool SelectAll( SmiKeyedFileIterator& iterator,
                  const SmiFile::AccessType accessType =
                        SmiFile::ReadOnly,
                  const bool reportDuplicates = false );
/*
Selects all records of the associated ~SmiBtreeFile~.
A record iterator for processing the selected records
is initialized on return.

By default an iterator for read only access without reporting duplicates is initialized,
but update access and reporting of duplicates may be specified.

The function returns "true"[4] when the iterator was initialized successfully.

*/

  PrefetchingIterator* SelectAllPrefetched();
/*
The same as ~SelectAll~, but returns a ~PrefetchingIterator~.

A ~PrefetchingIterator~ is read-only. Duplicates are always
reported.

*/

 protected:
 private:
  bool useTxn;
  friend class PrefetchingIterator;
};

/**************************************************************************
1.3 Class "SmiFileIterator"[1]

The class ~SmiFileIterator~ allows to scan through all records of a
SmiFile. The order in which the records are retrieved depends on the type
of the ~SmiFile~. An ~SmiFileIterator~ is instantiated through an ~SmiRecordFileIterator~
or an ~SmiKeyedFileIterator~.

*/

class SMI_EXPORT SmiFileIterator
{
 public:
  bool DeleteCurrent();
/*
Deletes the current record.

*/
  bool EndOfScan();
/*
Tells whether there are unscanned records left in the selected set of records.

*/
  bool Finish();
/*
Closes the iterator. The iterator handle may be reused (i.e. reinitialized)
afterwards. This method should always be called as soon as access to the record set
selected by the iterator is not required anymore, although the destructor will call
it implicitly.

*/
  bool Restart();
/*
Repositions the iterator in front of the first record of the selected set of records.

*/
 protected:
  bool Next( SmiRecord& record );
/*
Advances the iterator to the next record of the selected set of records.
A handle to the current record is returned.

*NOTE*: This function is never called directly by the user, but by the
~Next~ function of a derived class, namely ~SmiRecordFileIterator~ and
~SmiKeyedFileIterator~.

*/
  SmiFileIterator();
/*
Creates a handle for an ~SmiFile~ iterator. The handle is associated with an
~SmiFile~ by means of a ~Select~ method of that file. Initially the iterator is
positioned in front of the first record of the selected set of records.

*/
  ~SmiFileIterator();
/*
Destroys an ~SmiFile~ iterator handle.

*/
  bool solelyDuplicates;
  bool ignoreDuplicates;
/*
Flags whether duplicate records for a key may exist and whether those records
should be reported. If ~reportDuplicates~ is "false"[4], only the first record of
a set of duplicate records with equal keys is reported.

*/
  SmiFile* smiFile;     // associated SmiFile object
  bool     endOfScan;   // Flag end of scan reached
  bool     opened;      // Flag iterator opened
  bool     writable;    // Flag iterator for update access
  bool     restart;     // Flag for restart
  bool     rangeSearch; // Flag for range search using
  SmiKey*  searchKey;   // Searchkey as start of range
  class Implementation;
  Implementation* impl;
 private:
};

/**************************************************************************
1.3 Class "SmiRecordFileIterator"[1]

The class ~SmiRecordFileIterator~ allows to scan through all records of an
~SmiRecordFile~. The records are retrieved in the order as they were appended
to the file.

*/

class SMI_EXPORT SmiRecordFileIterator : public SmiFileIterator
{
 public:
  SmiRecordFileIterator();
/*
Creates a handle for an ~SmiRecordFile~ iterator. The handle is associated with an
~SmiFile~ by means of a ~Select~ method of that file. Initially the iterator is
positioned in front of the first record of the selected set of records.

*/
  ~SmiRecordFileIterator();
/*
Destroys an ~SmiRecordFileIterator~ handle.

*/
  bool Next( SmiRecordId& recno, SmiRecord& record );
  bool Next( SmiRecord& record );
/*
Advances the iterator to the next record of the selected set of records.
A handle to the current record and the record number of the
current record are returned.

*/

 protected:
 private:
  friend class SmiRecordFile;
};

/**************************************************************************
1.3 Class "SmiKeyedFileIterator"[1]

The class ~SmiKeyedFileIterator~ allows to scan through some or all records
of an ~SmiKeyedFile~. The records are retrieved in ascending order according to
the associated key values.

*/

class SMI_EXPORT SmiKeyedFileIterator : public SmiFileIterator
{
 public:
  SmiKeyedFileIterator( bool reportDuplicates = false );
/*
Creates a handle for a ~SmiKeyedFile~ iterator. The handle is associated with a
~SmiFile~ by means of a ~Select~ method of that file. Initially the iterator is
positioned in front of the first record of the selected set of records.

*/
  ~SmiKeyedFileIterator();
/*
Destroys a ~SmiKeyedFile~ iterator handle.

*/
  bool Next( SmiKey& key, SmiRecord& record );
  bool Next( SmiRecord& record );
/*
Advances the iterator to the next record of the selected set of records.
A handle to the current record -- and the key of the current record, if needed --
are returned.

*NOTE*: If the underlying ~SmiKeyedFile~ has keys of type ~SmiKey::Composite~ it
is *very important* to initialize the key object ~key~ with the correct key mapping
function, otherwise unmapping of the key does not take place. That is ~key~ should
be created as "SmiKey key( keyMappingFunction );"[4].

*/

 protected:
 private:
  SmiKey firstKey;       // Start of selected key range
  SmiKey lastKey;        // End of selected key range

  friend class SmiKeyedFile;
  friend class SmiBtreeFile;
};

/**************************************************************************
1.3 Class "PrefetchingIterator"[1]

The class ~PrefetchingIterator~ permits efficient read-only iteration over
an ~SmiKeyedFile~ or an ~SmiRecordFile~. The efficiency is achieved by
prefetching tuples.

Duplicates are supported and are always output.

*/
class PrefetchingIterator
{

protected:

  SmiKey::KeyDataType keyType;

  virtual void GetKeyAddressAndLength(void** addr, SmiSize& length) = 0;
/*
Returns the address and the length of the current key (represented as
a binary string). This method makes it possible that an implementation
of this interface need not be a friend of SmiKey, only this interface
itself needs to be a friend of SmiKey. This method is called by the
method ~CurrentKey~.

*/

public:
  virtual ~PrefetchingIterator();

  virtual void CurrentKey(SmiKey& smiKey);
/*
Returns the current key.

*/

  virtual bool Next() = 0;
/*
Advances the iterator by one tuple. If the iterator cannot be advanced,
~false~ is returned.

*/

  virtual SmiSize ReadCurrentData(void* userBuffer,
    SmiSize nBytes, SmiSize offset = 0) = 0;
/*
This method is analogous to the ~Read~ method in ~SmiRecord~.

*/

  virtual SmiSize ReadCurrentKey(void* userBuffer,
    SmiSize nBytes, SmiSize offset = 0) = 0;
/*
This method is analogous to the ~Read~ method in ~SmiRecord~, however
it is not concerned with the data, but with the key of the current tuple.
This can only be called if the underlying file is an
~SmiKeyedFile~.

*/

  virtual void ReadCurrentRecordNumber(SmiRecordId& recordNumber) = 0;
/*
Return the id of the current tuple. This can only be called if the
underlying file is an ~SmiRecordFile~.

*/

  virtual int ErrorCode() = 0;
/*
Return the error code of the last failed database operation.

*/
};

#endif // SECONDO_SMI_H

