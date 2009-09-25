/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics & Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[1] Header File of Module FLOB Cache

08/12/2005: Victor Almeida 

September 2009: M. Spiekermann. The FLOBCache will now create no Files on the
fly anymore. There is now only one file created for FLOBs with lobFileId = 0.
In the past the creation of new lob files for every call with lobFileId = 0 
could cause the creation of too many files. Moreover, the class relation creates
a lob file if the tuple type contains data with FLOBs. This prevents to call
~PutFlob~ with lobId = 0.


1 Defines

*/

#ifndef FLOBCACHE_H
#define FLOBCACHE_H

/*

2 Includes

*/
#include <map>
#include <iostream>
#include <exception>
#include <string>

#include "Trace.h"

using std::string;
using std::exception;
using std::cerr;

class FLOBCache_error : public exception {

  public: 	

  FLOBCache_error() : exception(), msg("unknown error!") {}
  FLOBCache_error(const string& reason) : exception(), msg(reason) {}
  FLOBCache_error(const FLOBCache_error& rhs) : exception(), msg(rhs.msg) {}

  virtual ~FLOBCache_error() throw() {}

  virtual const char* what() const throw() {

	  return ("FLOBCache:" + msg).c_str();   	  
  }	  

  private:
  const string msg;
};




/*
3 Struct FLOBKey

*/
struct FLOBKey
{
  inline FLOBKey():
  fileId( 0 ), recordId( 0 ), pageno( -1 ),
  size( 0 ), isTempFile( true )
  {}
  
  inline FLOBKey( SmiFileId fileId, SmiRecordId recordId, long pageno,
                  int size, bool isTempFile = false ):
  fileId( fileId ), recordId( recordId ), pageno( pageno ),
  size( size ), isTempFile( isTempFile )
  {}

  inline FLOBKey& operator=( const FLOBKey fk )
  {
    fileId = fk.fileId; 
    recordId = fk.recordId;
    pageno = fk.pageno;
    size = fk.size; 
    return *this;
  }

  inline bool operator<( const FLOBKey key ) const
  {
    return fileId < key.fileId ||
           ( fileId == key.fileId && 
             ( recordId < key.recordId ||
               (recordId == key.recordId && pageno < key.pageno )));
  }
    
  
  inline ostream& out(ostream& os) const {

    os << "fileId = " << fileId;	  
    os << ", recordId = " << recordId;
    os << ", pageno = " << pageno;
    os << ", size = " << size; 
    return os;
  }	  

  SmiFileId fileId;
  SmiRecordId recordId;
  long pageno;
  int size;
  bool isTempFile;
};


ostream& operator<<(ostream& os, const FLOBKey& fk);

/*
3 Struct FLOBCacheElement

*/
struct FLOBCacheElement
{
  inline FLOBCacheElement( const FLOBKey key, char *flob ):
  key( key ), flob( flob ), refs( 1 ), next( NULL ), prev( NULL )
  {}

  FLOBKey key;
  char *flob;
  int refs;
  FLOBCacheElement *next;
  FLOBCacheElement *prev;
};

/*
3 Struct LRUElement

*/
class LRUTable
{
  public:
    inline LRUTable():
    first( NULL ), last( NULL )
    {}

    ~LRUTable();

    void Insert( FLOBCacheElement *e );
    void Promote( FLOBCacheElement *e, bool inc );
    void Remove( FLOBCacheElement *e );
    bool RemoveLast( FLOBKey& key );
    void Clear();
    inline FLOBCacheElement *First() const
      { return first; }
    inline FLOBCacheElement *Last() const
      { return last; }

  private:
    FLOBCacheElement *first;
    FLOBCacheElement *last;
};

/*
3 class FLOBCache

*/
class FLOBCache
{
  public:
    inline FLOBCache( long size ):
    maxSize( size ),
    sizeLeft( size ),
    lobFile(0),
    lobFileId(0),
    trace("FLOBCache", cerr)	
    {
      if (RTFlag::isActive("FLOBCache:trace")) {	    
        trace.on();
      }
    }

    ~FLOBCache();

    bool GetFLOB( SmiFileId fileId, SmiRecordId lobId, long pageno, 
                  size_t size, bool isTempFile, const char *&flob );
    void PutFLOB( SmiFileId& fileId, SmiRecordId& lobId, long pageno,
                  size_t size, bool isTempFile, const char *flob );
    void Clear();
    void Clean();
    void Release( SmiFileId fileId, SmiRecordId lobId, 
                  long pageno = -1 );
    void Destroy( SmiFileId fileId, SmiRecordId lobId );
    void Truncate( SmiFileId fileId, bool isTemp, 
                   bool paged = false );
    void Drop( SmiFileId fileId, bool isTemp,
               bool paged = false );

    SmiRecordFile* CreateFile( bool tmp );


  private:
    char *Lookup( const FLOBKey key, bool inc = false );
    bool Insert( const FLOBKey key, char *flob );
    void Remove( const FLOBKey key );
    bool RemoveLast( FLOBKey& key );
    void DecReference( const FLOBKey key );
    
  

  private:
    long maxSize, sizeLeft;
    typedef map< FLOBKey, FLOBCacheElement* > FLOBTable;
    FLOBTable mapTable;
    LRUTable lruTable;
    typedef map< SmiFileId, SmiRecordFile* > FileTable;
    FileTable files;
    SmiRecordFile* lobFile;
    SmiFileId lobFileId;

    SmiRecordFile* LookUpFile( SmiFileId id, bool tmp );
    void RemoveFile( SmiFileId id );


    Trace trace;

    void traceFile( SmiFileId id, 
		    bool b, const string& act = "created")
    {
      trace.add() << "New LOB-file: action = " << act 
	          << ", fileId = " << id 
		  << ", isTempFile = " << b;
      trace.flush();
    }

};

#endif
