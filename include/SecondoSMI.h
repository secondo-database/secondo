/*
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

1 Header File: Storage Management Interface

April 2002 Ulrich Telle

1.1 Overview

The *Storage Management Interface* provides all types and classes needed
for dealing with persistent data objects in "Secondo"[3].

Essential for all operations on persistent data objects is the class 
~SmiEnvironment~ which provides methods for startup and shutdown of 
the storage management, for transaction handling and for error handling.

Internally the ~SmiEnvironment~ handles the connection to the underlying
implementation of the persistent information storage management. Currently
implementations based on the Berkeley DB library and on the Oracle DBMS
are available. The decision which implementation is used is taken at link
time of the "Secondo"[3] server. Fine tuning of implementation dependent
parameters is done by means of a configuration file which is read at system
startup. For future extensions (i.e. user management and access control)
it is possible to store the current user identification in the ~SmiEnvironment~.

Additionally the ~SmiEnvironment~ introduces the concept of ~databases~. Within
one ~SmiEnvironment~ there may exist several databases, but a user may access
only *one* database at a single time. A valid database name consists of at most
*SMI\_MAX\_DBNAMELEN* (currently 15) alphanumeric characters or underscores. The
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
sensitive and may have at most *SMI\_MAX\_NAMELEN* (currently 31) alphanumeric
characters or underscores. The first character has to be alphabetic.

An ~SmiFile~ for record oriented access is called ~SmiRecordFile~. Records can be
of fixed or variable size. The use of ~SmiFiles~ with fixed length records is
recommended whereever appropriate since they may be more efficient -- depending
on the implementation. Records can only be appended to the end of an ~SmiFile~,
but can be accessed for reading, update or deletion in random order using their
record number. An iterator for sequential access to all records of an ~SmiFile~ is
provided.

An ~SmiFile~ for key oriented access is called ~SmiKeyedFile~. An ~SmiKeyFile~ is
capable of holding pairs of keys and data. Both keys and data may be of variable
size. While the size of the data is only restricted by physical limits of the
available hardware, the size of keys is restricted to at most 4000 bytes.
(This limit is imposed by the VARCHAR2 datatype of Oracle, but depending on the
actual database configuration the maximal possible key length might be lower.
Other relational database systems allow only much smaller keys, i.e. MySQL
restricts the combined key length to 512 bytes, but a single column key cannot
exceed 255 bytes and that would be the restriction for an implementation based
on MySQL.)

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
of the transaction support of the Berkeley DB it is recommended that applications
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
                         &             & GetFileId  \\
                         &             & IsOpen     \\

The class ~SmiRecordFile~ provides the following methods:

[23]    Creation/Removal     & Record Selection & Record Modification \\
        [--------]
        SmiRecordFile        & SelectRecord     & AppendRecord \\
        [tilde]SmiRecordFile & SelectAll        & DeleteRecord \\

The class ~SmiKeyedFile~ provides the following methods:

[23]    Creation/Removal    & Record Selection & Record Modification \\
        [--------]
        SmiKeyedFile        & SelectRecord     & InsertRecord \\
        [tilde]SmiKeyedFile & SelectRange      & DeleteRecord \\
                            & SelectLeftRange  & \\
                            & SelectRightRange & \\
                            & SelectAll        & \\

The class ~SmiRecord~ provides the following methods:

[23]    Creation/Removal & Persistence & Querying \\
        [--------]
	SmiRecord        & Read        & Size \\
        [tilde]SmiRecord & Write       &      \\
        Finish           & Truncate    &      \\

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

#include "SecondoConfig.h"
#include <string>

const string::size_type SMI_MAX_NAMELEN      =   31;
/*
specifies the maximum length of a context or file name.

*/

const string::size_type SMI_MAX_DBNAMELEN    =   15;
/*
specifies the maximum length of a database name.

*/

const string::size_type SMI_MAX_KEYLEN       = 3200;
/*
specifies the maximum length of keys.

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
specifies the maximum length of keys which are stored locally within an
instance of the SmiKey class. Extra memory is allocated for longer keys.

*/

typedef long          SmiError;
/*
is the type for error codes of the storage management interface.

*/

typedef unsigned long SmiFileId;
/*
is the type for the unique file identifiers.

*/

typedef unsigned long SmiRecordId;
/*
is the type for record identifiers.

*/

typedef unsigned long SmiSize;
/*
is the type for record sizes or offsets.

*/

typedef void (*MapKeyFunc)( const void*    inKey,
                            const SmiSize  inLen,
                                  void*    outKey,
                            const SmiSize  maxOutLen,
                                  SmiSize& outLen,
                            const bool     doMap );
/*
is the type of user functions for mapping composite key types to strings and
vice versa.

*/

/**************************************************************************
1.3 Class "SmiEnvironment"[1]

This class handles all aspects of the environment of the storage environment
including the basics of transactions.

*/

class SmiFile;  // Forward declaration of SmiFile class

class SMI_EXPORT SmiEnvironment
{
 public:
  enum SmiType { SmiBerkeleyDB, SmiOracleDB };
/*
enumerates the different implementations of the storage management interface:

  * *SmiBerkeleyDB* -- Implementation based on the Berkeley DB

  * *SmiOracleDB* -- Implementation based on Oracle

*/
  enum RunMode { SingleUserSimple, SingleUser,
                 MultiUser, MultiUserMaster };
/*
lists the types of run modes supported by the ~SmiEnvironment~:

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

*NOTE*: In any multi user mode the Secondo registrar must be running.
The behaviour of the storage management system in single user mode is
implementation dependent.

*/
  static SmiType GetImplementationType();
/*
returns the implementation type of the storage management interface.

*NOTE*: This information is availabe before calling the ~StartUp~ method,
thus allowing to perform implementation dependent activities.

*/
  static SmiEnvironment* GetInstance();
/*
returns a pointer to the Secondo Storage Management Environment
(probably unnecessary).

*/
  static bool StartUp( const RunMode mode, 
                       const string& parmFile,
                       ostream& errStream );
/*
initializes the ~SmiEnvironment~ of the storage manager interface.
Parameters are read from the configuration file ~parmFile~.
Error messages are written to the provided output stream ~errStream~.
A user transaction is implicitly started.

*/
  static bool ShutDown();
/*
shuts down the storage manager interface. An open user transaction is aborted
implicitly. It is necessary to close *all* open ~SmiFiles~, iterators and
record handles before shutting down the system.

*/
  static bool IsDatabaseOpen();
/*
returns ~true~ if a database is currently open, otherwise ~false~ is returned.

*/
  static string CurrentDatabase();
/*
returns the name of the currently open database. If no database is open
a blank string is returned.

*/
  static bool CreateDatabase( const string& dbname );
/*
creates a new "Secondo"[3] database under the name ~dbname~.
The function returns ~true~, if the database could be created;
it returns ~false~, if a database with the given name already
exists or if an error occured.

*/
  static bool OpenDatabase( const string& dbname );
/*
opens an existing "Secondo"[3] database having the name ~dbname~.
The functions returns ~true~, if the database could be opened,
otherwise it returns ~false~.

*/
  static bool CloseDatabase();
/*
closes a previously created or opened "Secondo"[3] database.
The function returns ~true~, if the database could be closed successfully.
Otherwise it returns ~false~.

*/
  static bool EraseDatabase( const string& dbname );
/*
erases the "Secondo"[3] database named ~dbname~.
The function returns ~true~, if the database could be erased.
Otherwise it returns ~false~.

*NOTE*: It is an error to call this function, if a database is already
open, regardless of the name.

*/
  static bool ListDatabases( string& dbname );
/*
lists the names of existing databases, one at a time, and delivers them
in ~dbname~. The function returns ~true~ as long as there are names of
database available and returns ~false~ after the last name has been
delivered.

*/
  static bool SetUser( const string& uid );
/*
stores the identification ~uid~ of the current user in the ~SmiEnvironment~.
In a future extension it may be used for user management and access control.

*/
  static bool BeginTransaction();
  static bool CommitTransaction();
  static bool AbortTransaction();
/*
are provided for transaction handling. Each committed or aborted transaction
starts implicitly a new transaction. Therefore usually an explicit call to
~BeginTransaction~ is not necessary. To be able to rollback requests for
deleting ~SmiFiles~ such requests are registered throughout the transaction and
carried out only if the transaction completes successfully. 
~Named SmiFiles~ are registered in a file catalog. Changes to this catalog take
place when a transaction is committed. When updates to the file catalog fail,
the transaction is implicitly aborted.

*/
  static SmiError CheckLastErrorCode();
  static SmiError GetLastErrorCode();
  static SmiError GetLastErrorCode( string& errorMessage );
/*
returns the error code of the last storage management operation.
~CheckLastErrorCode~ provides the error code without resetting the internal
error code while the other functions reset the internal error code.
Optionally the accompanying error message is returned.

*/
  static void SetError( const SmiError smiErr,
                        const int sysErr = 0 );
  static void SetError( const SmiError smiErr, 
                        const string& errMsg );
  static void SetError( const SmiError smiErr, 
                        const char* errMsg );
/*
allows to set an SmiError code and a system error code or an error message.
(maybe these functions should not be public. Currently messages are
generated only for errors occuring in the Berkeley DB or in Oracle.)

*/
 private:
  SmiEnvironment();
/*
creates an instance of the ~Storage Management Interface~ environment.

*/
  ~SmiEnvironment();
/*
destroys an SMI environment.

*/
  SmiEnvironment( SmiEnvironment& );
/*
The copy constructor is not implemented.

*/
  static bool SetDatabaseName( const string& dbname );
/*
checks whether the given database name ~dbname~ is valid or not,
converts the name to all lower case and stores the converted name in a
member variable.
The function returns ~true~, if the name is valid.

A valid database name consists of at most *SMI\_MAX\_DBNAMELEN* (currently 15)
alphanumeric characters or underscores. The first character has to be alphabetic.
Database names are *not* case sensitive.

*/
  static bool InitializeDatabase();
/*
initializes a new database.

*/
  static bool RegisterDatabase( const string& dbname );
/*
registers the database ~dbname~ when it is created or opened.
The function returns ~true~, if the database could be registered successfully
or if the application runs in single user mode.

*NOTE*: The registration is necessary to protect a database from accidental deletion
by another user.

*/
  static bool UnregisterDatabase( const string& dbname );
/*
unregisters the database ~dbname~.
The function returns ~true~, if the database could be unregistered successfully
or if the application runs in single user mode.

*/
  static bool LockDatabase( const string& dbname );
/*
locks the database ~dbname~.
The function returns ~true~, if a lock could be acquired successfully,
i.e. no other user accesses the database,
or if the application runs in single user mode.

*NOTE*: Before a database can be erased it has to be locked.

*/
  static bool UnlockDatabase( const string& dbname );
/*
unlocks the database ~dbname~.
The function returns ~true~, if the lock could be released successfully
or if the application runs in single user mode.

*/

  static SmiEnvironment instance;    // Instance of environment
  static SmiError       lastError;   // Last error code
  static string         lastMessage; // Last error message
  static bool           smiStarted;  // Flag SMI initialized
  static bool           singleUserMode;
  static bool           useTransactions;
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
  friend class SmiFileIterator;
  friend class SmiRecordFile;
  friend class SmiKeyedFile;
  friend class SmiRecord;
};

/**************************************************************************
1.3 Class "SmiKey"[1]

The class ~SmiKey~ is used to store key values of different types in a
consistent manner. Key values are restricted in length to at most
~SMI\_MAX\_KEYLEN~ bytes. If the length of the key value is less than
~SMI\_MAX\_KEYLEN\_LOCAL~ the key value ist stored within the class instance,
otherwise memory is allocated.

*/

class SMI_EXPORT SmiKey
{
 public:
  enum KeyDataType
    { Unknown, RecNo, Integer, Float, String, Composite };
/*
lists the types of key values supported by the SmiFiles for keyed access:

  * Unknown -- not a true type, designates an uninitialized key instance

  * RecNo -- a record number of a SmiRecordFile

  * Integer -- signed integer number (base type ~long~)

  * Float -- floating point number (base type ~double~)

  * String -- character string (base type ~string~)

  * Composite -- user-defined key structure, the user has to provide a mapping
function which is called to map the key structure to a byte string which can
be sorted like a usual string in lexical order. On key retrieval the function
is called to unmap the byte string to the user-defined key structure.

*/
  SmiKey( MapKeyFunc mapKey = 0 );
  SmiKey( const SmiRecordId key );
  SmiKey( const long key );
  SmiKey( const double key );
  SmiKey( const string& key );
  SmiKey( const void* key, const SmiSize keyLen,
          MapKeyFunc mapKey );
  SmiKey( SmiKey& other );
/*
creates a key with a type according to the constructor argument.

*/
  ~SmiKey();
/*
destroys a key.

*/
  SmiKey& operator=( const SmiKey& other );
  const bool operator>( const SmiKey& other );
  const KeyDataType GetType() const;
/*
returns the type of the key.

*/
  bool GetKey( SmiRecordId& key );
  bool GetKey( long& key );
  bool GetKey( double& key );
  bool GetKey( string& key );
  bool GetKey( void* key, const SmiSize maxKeyLen,
               SmiSize& keyLen );
/*
returns the value of the key. The argument type must match the type of the key!

*/
  static void Map( const long   inData, void* outData );
  static void Map( const double inData, void* outData );
  static void Unmap( const void* inData, long&   outData );
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
frees the memory allocated for a key, if memory was previously allocated.
The function is called internally when a new key value is assigned.

*/
  const void* GetAddr() const;
/*
returns the memory address of the key value.

*/
  void  SetKey( const SmiRecordId key );
  void  SetKey( const long key );
  void  SetKey( const double key );
  void  SetKey( const string& key );
  void  SetKey( const void* key, const SmiSize keyLen,
                MapKeyFunc mapKey );
  void  SetKey( const KeyDataType kdt, 
                const void* key, const SmiSize keyLen,
                MapKeyFunc mapKey = 0 );
/*
sets the internal key value to the passed key value, setting also the key type.

*/

  KeyDataType keyType;   // Type of the key value
  SmiSize     keyLength; // Size of the key value in bytes
  MapKeyFunc  mapFunc;   // Address of a mapping function
  union                  // Structure for storing the key
  {
    SmiRecordId recnoKey;
    long        integerKey;
    double      floatKey;
    char        shortKeyData[SMI_MAX_KEYLEN_LOCAL+1];
    char*       longKeyData;
  };

  friend class SmiFile;
  friend class SmiFileIterator;
  friend class SmiRecordFile;
  friend class SmiKeyedFile;
  friend class SmiKeyedFileIterator;
  friend class SmiRecord;
};

/**************************************************************************
1.3 Class "SmiRecord"[1]

This class provides a record handle for processing data records. A record
contains user-defined byte strings of arbitrary length. A record handle is
initialized by the appropriate methods of a SmiFile. After initialization
the handle can be used to access complete or partial records.

*/

class SMI_EXPORT SmiRecord
{
 public:
  SmiRecord();
/*
creates a record handle. A handle is passed to SmiFile or SmiIterator methods
for initialization and can be used thereafter to access a record.

*/
  ~SmiRecord();
/*
destroys a record handle.

*/
  SmiSize  Read( void* buffer, 
                 const SmiSize numberOfBytes,
                 const SmiSize offset = 0 );
/*
reads a sequence of at most ~numberOfBytes~ bytes from the record into the ~buffer~
provided by the user. Optionally a record ~offset~ can be specified.
The amount of bytes actually transfered is being returned.

*/
  SmiSize  Write( const void*   buffer, 
                  const SmiSize numberOfBytes, 
                  const SmiSize offset = 0 );
/*
writes a sequence of at most ~numberOfBytes~ bytes into the record from the ~buffer~
provided by the user. Optionally a record ~offset~ can be specified.
The amount of bytes actually transfered is being returned.

*/
  SmiSize  Size();
/*
retrieves the total amount of bytes stored within the persistent representaion
of this record.

*/
  bool  Truncate( const SmiSize newSize );
/*
truncates the record to a specified length of ~newSize~ bytes.
The function returns ~false~ if newSize is greater than the current record's
length.

*/
  void Finish();
/*
finishes the operation on the associated record. The record handle may be
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
1.3 Class "SmiFile"[1]

This class provides the methods common to both SmiRecordFiles and
SmiKeyedFiles.

*/

class SMI_EXPORT SmiFile
{
 public:
  enum FileType   { FixedLength, VariableLength, Keyed };
/*
is an enumeration of possible file types:

  * FixedLength -- Files of this type consist of a set of records all having a
fixed size which cannot be changed.
A record is filled with binary null characters if not the whole record is written.
Depending on the implementation this file type allows for better locking characteristics
in multi-user environments. Records are identified by record numbers.

  * VariableLength -- Files of this type consist of a set of records of potentially
varying size. Records are identified by record numbers.

  * Keyed -- Files of this type consist of key/data pairs.

*/
  enum AccessType { ReadOnly,    Update         };
/*
is an enumeration of possible access types:

  * ReadOnly -- Records are selected for read access only. Operations which change
the contents or the size of a record are not permitted.

  * Update -- Records are selected for read and/or write access.

*/
  bool Create( const string& context = "Default" );
/*
creates a new anonymous SmiFile.
Optionally a ~context~ can be specified.

*/
  bool Open( const SmiFileId id,
             const string& context = "Default" );
/*
opens an existing anonymous SmiFile using its file identifier ~id~.
Optionally a ~context~ can be specified.

*/
  bool Open( const string& name,
             const string& context = "Default" );
/*
opens an existing named SmiFile or creates a new named SmiFile if it does not
exist. Optionally a ~context~ can be specified. 

*/
  bool Close();
/*
closes an open SmiFile.

*/
  bool Drop();
/*
erases a SmiFile. It is necessary to close any record iterators or record 
handles before dropping a SmiFile.

*/
  string GetContext();
/*
returns the context of the SmiFile.

*/
  string GetName();
/*
returns the name of a named SmiFile or an empty string for an anonymous SmiFile.

*/
  SmiFileId GetFileId();
/*
returns the unique SmiFile identifier.

*/
  bool   IsOpen();
/*
returns whether the SmiFile handle is open and can be used to access the
records of the SmiFile.

*/
 protected:
  SmiFile();
  SmiFile( SmiFile &smiFile );
  ~SmiFile();
  bool CheckName( const string& name );
/*
checks whether the given name ~name~ is valid.

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
  friend class SmiFileIterator;
  friend class SmiRecord;
};

/**************************************************************************
1.3 Class "SmiRecordFile"[1]

The class ~SmiRecordFile~ allows record oriented access to persistent objects.
New records can only be appended to a SmiRecordFile, but existing records can
be processed in random order using their record numbers. 

By means of an iterator it is possible to scan through all records of a SmiFile.
The records are obtained in the order they were appended to the file.

*/

class SmiRecordFileIterator;  // Forward declaration

class SMI_EXPORT SmiRecordFile : public SmiFile
{
 public:
  SmiRecordFile( const bool hasFixedLengthRecords,
                 const SmiSize recordLength = 0 );
/*
creates a handle for a SmiRecordFile. The handle is associated with a
SmiFile by means of the ~Create~ or ~Open~ method.

*/
  ~SmiRecordFile();
/*
destroys a handle.

*/
  bool SelectRecord( const SmiRecordId recno, 
                     SmiRecord& record,
                     const SmiFile::AccessType accessType =
                           SmiFile::ReadOnly );
/*
selects the record identified by the record number ~recno~ for read only
or update access. The user has to provide a record handle which is
initialized by this method.

*/
  bool SelectAll( SmiRecordFileIterator& iterator,
                  const SmiFile::AccessType accessType =
                        SmiFile::ReadOnly );
/*
initializes an iterator for sequentially accessing all records of the SmiFile.
The requested access type - read only or update - has to be specified.

*/
  bool AppendRecord( SmiRecordId& recno,
                     SmiRecord& record );
/*
appends a new record to the SmiFile and returns a record handle pointing to
the new record.

*/
  bool DeleteRecord( const SmiRecordId recno );
/*
deletes the record identified by record number ~recno~ from the file.

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
of a SmiKeyedFile. The records are retrieved in ascending order of the key
values. For duplicate records no special ordering is supported.

*/

class SmiKeyedFileIterator;  // Forward declaration

class SMI_EXPORT SmiKeyedFile : public SmiFile
{
 public:
  SmiKeyedFile( const SmiKey::KeyDataType keyType,
                const bool hasUniqueKeys = true );
/*
creates a SmiFile handle for keyed access. The keys have to be of the
specified type ~keyType~. If ~hasUniqueKeys~ is true, then for each key
only one record can be stored. Otherwise duplicate records are allowed.

*/
  ~SmiKeyedFile();
/*
destroys a file handle.

*/
  bool SelectRecord( const SmiKey& key, 
                     SmiKeyedFileIterator& iterator,
                     const SmiFile::AccessType accessType =
                           SmiFile::ReadOnly );
/*
selects the data record with the given key. It is required to specify
whether the select takes place for read only or update of the record. 
This method always initializes a record iterator regardless whether
keys are unique or not. Usually this method should only be used when
duplicate records are supported.

The function returns ~true~ when at least one record exists for the given key.

*/
  bool SelectRecord( const SmiKey& key,
                     SmiRecord& record,
                     const SmiFile::AccessType accessType =
                           SmiFile::ReadOnly );
/*
selects the data record with the given key and initializes a SmiRecord
handle for processing the record. In is required to specify whether the select
takes place for read only or update of the record. In case of
duplicates only the first record is returned with access type ~ReadOnly~,
regardless of the specified access type!

The function returns ~true~ when a record exists for the given key.

*/
  bool SelectRange( const SmiKey& fromKey, const SmiKey& toKey,
                    SmiKeyedFileIterator& iterator,
                    const SmiFile::AccessType accessType =
                          SmiFile::ReadOnly,
                    const bool reportDuplicates = false );
/*
selects a range of records with keys for which the following condition holds
"fromKey [<=] key [<=] toKey"[1]. A record iterator for processing the selected
records is initialized on return.

By default an iterator for read only access without reporting duplicates is initialized,
but update access and reporting of duplicates may be specified.

The function returns ~true~ when the iterator was initialized successfully.

*/
  bool SelectLeftRange( const SmiKey& toKey, 
                        SmiKeyedFileIterator& iterator,
                        const SmiFile::AccessType accessType =
                              SmiFile::ReadOnly,
                        const bool reportDuplicates = false );
/*
selects a range of records with keys for which the following condition holds
"key [<=] toKey"[1]. A record iterator for processing the selected records
is initialized on return.

By default an iterator for read only access without reporting duplicates is initialized,
but update access and reporting of duplicates may be specified.

The function returns ~true~ when the iterator was initialized successfully.

*/
  bool SelectRightRange( const SmiKey& fromKey, 
                         SmiKeyedFileIterator& iterator,
                         const SmiFile::AccessType accessType =
                               SmiFile::ReadOnly,
                         const bool reportDuplicates = false );
/*
selects a range of records with keys for which the following condition holds
"fromKey [<=] key"[1]. A record iterator for processing the selected records
is initialized on return.

By default an iterator for read only access without reporting duplicates is initialized,
but update access and reporting of duplicates may be specified.

The function returns ~true~ when the iterator was initialized successfully.

*/
  bool SelectAll( SmiKeyedFileIterator& iterator,
                  const SmiFile::AccessType accessType =
                        SmiFile::ReadOnly,
                  const bool reportDuplicates = false );
/*
selects all records of the associated SmiKeyedFile.
A record iterator for processing the selected records
is initialized on return.

By default an iterator for read only access without reporting duplicates is initialized,
but update access and reporting of duplicates may be specified.

The function returns ~true~ when the iterator was initialized successfully.

*/
  bool InsertRecord( const SmiKey& key, SmiRecord& record );
/*
inserts a new record for the given key.
A SmiRecord handle is initialized on return to write the record.

The function returns ~true~ when the record was created successfully.

*/
  bool DeleteRecord( const SmiKey& key );
/*
deletes all records having the given key.

The function returns ~true~ when the records were successfully deleted.

*/
 protected:
 private:
};

/**************************************************************************
1.3 Class "SmiFileIterator"[1]

The class ~SmiFileIterator~ allows to scan through all records of a
SmiFile. The order in which the records are retrieved depends on the type
of the ~SmiFile~. A SmiFileIterator is instantiated through a SmiRecordFileIterator
or a SmiKeyedFileIterator.

*/

class SMI_EXPORT SmiFileIterator
{
 public:
  bool DeleteCurrent();
/*
deletes the current record.

*/
  bool EndOfScan();
/*
tells whether there are unscanned records left in the selected set of records.

*/
  bool Finish();
/*
closes the iterator. The iterator handle may be reused (i.e. reinitialized)
afterwards. This method should always be called as soon as access to the record set 
selected by the iterator is not required anymore, although the destructor will call
it implicitly.

*/
  bool Restart();
/*
repositions the iterator in front of the first record of the selected set of records.

*/
 protected:
  bool Next( SmiRecord& record );
/*
advances the iterator to the next record of the selected set of records.
A handle to the current record is returned.

*NOTE*: This function is never called directly by the user, but by the
~Next~ function of a derived class, namely ~SmiRecordFileIterator~ and
~SmiKeyedFileIterator~.

*/
  SmiFileIterator();
/*
creates a handle for a SmiFile iterator. The handle is associated with a
SmiFile by means of a ~Select~ method of that file. Initially the iterator is
positioned in front of the first record of the selected set of records.

*/
  ~SmiFileIterator();
/*
destroys a ~SmiFile~ iterator handle.

*/
  bool solelyDuplicates;
  bool ignoreDuplicates;
/*
Flags whether duplicate records for a key may exist and whether those records
should be reported. If ~reportDuplicates~ is false, only the first record of
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

The class ~SmiRecordFileIterator~ allows to scan through all records of a
SmiRecordFile. The records are retrieved in the order as they were appended
to the file.

*/

class SMI_EXPORT SmiRecordFileIterator : public SmiFileIterator
{
 public:
  SmiRecordFileIterator();
/*
creates a handle for a SmiRecordFile iterator. The handle is associated with a
SmiFile by means of a ~Select~ method of that file. Initially the iterator is
positioned in front of the first record of the selected set of records.

*/
  ~SmiRecordFileIterator();
/*
destroys a SmiRecordFile iterator handle.

*/
  bool Next( SmiRecordId& recno, SmiRecord& record );
  bool Next( SmiRecord& record );
/*
advances the iterator to the next record of the selected set of records.
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
of a SmiKeyedFile. The records are retrieved in ascending order according to
the associated key values.

*/

class SMI_EXPORT SmiKeyedFileIterator : public SmiFileIterator
{
 public:
  SmiKeyedFileIterator( bool reportDuplicates = false );
/*
creates a handle for a SmiKeyedFile iterator. The handle is associated with a
SmiFile by means of a ~Select~ method of that file. Initially the iterator is
positioned in front of the first record of the selected set of records.

*/  
  ~SmiKeyedFileIterator();
/*
destroys a SmiKeyedFile iterator handle.

*/
  bool Next( SmiKey& key, SmiRecord& record );
  bool Next( SmiRecord& record );
/*
advances the iterator to the next record of the selected set of records.
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
};

#endif // SECONDO_SMI_H

