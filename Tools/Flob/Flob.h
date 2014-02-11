/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Department of Computer Science,
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

//paragraph [10] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[10] Header File of Module LOB

October 2009, C. Duentgen Initial revised implementation of Flob.h Flob.cpp

1  Overview

LOB is a shortcut for ~large object~.

FLOB (or Flob) is a shortcut for ~faked larged object~ which is a concept for implementing
datatypes such as regions which may vary strongly in size. The idea of Flobs has
been studied in [1] (The current implementation differs in some aspects). The
basic idea is to store data of an object value depending on a threshold size
either inside the tuple representation or in a separate storage location.

Flobs can be used in implementations of secondo data types. Therefore typically
the subclass ~DBArray~ might be used. Since the persistent storage of data is
organized by the class FlobManager (for tuples on behalf of the RelationAlgebra)
we do not need to deal with it here.

A Flob provides the user a segment of memory to store any kind of data. Instead
of pointers, addresses relative to the start address of the segment (offsets)
must be used to dynamic data structures within a Flob data segement.

2 Implementation

The class ~Flob~ provides a handle on large objects. The user can use the class'
member functions to create new Flobs, to restore Flobs from persistent storage,
and to to read or modify the contained data. However, the actual work for all
these functions is done by the friend class ~FlobManager~, whose member functions
are called. The ~FlobManager~ also deals with caching the Flob data to fasten
acceess.

Flob objects (instances of this class) can be used as members of Secondo data
type classes and attribute type classes, since they have a compact memory
representation and thus can be made persistent using any of Secondo's persistency
mechanisms. This of course only stores the handle to the object data. The actual
object data is maintained in some files provided and managed by a singleton
instance of class ~FlobManager~.

The user may read Flob data into buffers and modify it there. Managing that
buffers is up to the user. To read a Flob's object data, the read() function is
called. It copies the requested data to a buffer provided by the user.
If changes to the Flob object data shall be made, the user must call the write()
member function, which will overwrite the Flob object data with the provided
buffer's content.

*/

#ifndef FLOB_H
#define FLOB_H

#include <ostream>

#include "FlobId.h"
#include "FlobManager.h"
#include "SecondoSMI.h"
#include "Serialize.h"
#include "assert.h"
#include <stdlib.h>

class Flob{
  friend class FlobManager;
  friend class FlobCache;
  friend class NativeFlobCache;
  friend class NativeCacheEntry;
  public:

/*
~Standard Constructor~

This constructor should only be used within Cast functions.

*/
    inline Flob() {}

/*
~Copy Constructor~

Creates a flat copy of the argument. The underlying data are not
duplicated (only the handle is copied).

*/
  inline Flob(const Flob& other) : id(other.id), size( other.size ),
                                   dataPointer(0) {
     if(other.dataPointer){
       dataPointer = (char*)(malloc(size));
       memcpy(dataPointer,other.dataPointer,size);
     }
  };

/*
~Constructor~

This constructor creates a native flob having a given size. The data of a native
Flob is kept within a special file controlled by the ~FlobManager~.

*/
    inline Flob (const SmiSize size_): id(), size( size_ ), dataPointer(0){
      FlobManager::getInstance().create(size_, *this);
    };


/*
~Constructor~

Constructs a Flob from a given position.

*/
    inline Flob(const SmiFileId& fileId,
                const SmiRecordId& recordId,
                const SmiSize& offset,
                const char mode) : id(), size( 0 ), dataPointer(0){
         FlobManager::getInstance().create(fileId, recordId,
                                           offset, mode,
                                           size, *this);
    };

/*
~Constructor~

Constructs a flob from a given buffer as an ~evil Flob~ (having a fixed main
memory buffer not under management of the FlobManager).

*/
    inline Flob(const char* buffer, const SmiSize& size):id(),size(size),
                dataPointer(0){
       id.destroy();
       dataPointer = (char*) malloc(size);
       memcpy(dataPointer, buffer, size);
    }


/*
~Assignment Operator~

Makes only a flat copy of the source flob (only the handle is copied).

*/
    inline Flob& operator=(const Flob& src){
      size = src.size;
      id   = src.id;
      if(src.dataPointer){
        if(dataPointer){
          dataPointer = (char*)realloc(dataPointer, size);
        } else {
          dataPointer = (char*)malloc(size);
        }
        memcpy(dataPointer, src.dataPointer, size);
      } else{
        if(dataPointer){
           free(dataPointer);
        }
        dataPointer = 0;
      }

      return *this;
    };

/*
~Comparison~

*/
   inline bool operator==(const Flob& f) const{
     assert(!dataPointer && !f.dataPointer);
     return (id == f.id);
   }

   inline bool operator!=(const Flob& f) const{
     assert(!dataPointer && !f.dataPointer);
     return id != f.id;
   }

   inline bool operator>(const Flob& f) const{
     assert(!dataPointer && !f.dataPointer);
     return id > f.id;
   }

   inline bool operator<(const Flob& f) const{
     assert(!dataPointer && !f.dataPointer);
     return id < f.id;
   }

/*
~hashValue~

*/
   inline size_t hashValue() const{
     if(!dataPointer){
        return id.hashValue();
     } else {
        return 0;
     }
   }


/*
~Destructor~

Does not destroy the underlying persistent data. Use ~destroy~ to destroy the
persistent Flob data.

*/
    virtual ~Flob() {
      if(dataPointer){
        free(dataPointer);
        dataPointer = 0;
      }
    };

/*
~getSize~

Returns the current data size of the data referenced by this Flob.

*/
    inline SmiSize getSize() const { return size; }; // Size of the Flob


/*
~getUncontrolledSize~

Returns the size of the flob data not under management of the FlobManager;

*/
    inline SmiSize getUncontrolledSize() const { return dataPointer?size:0; }



/*
~resize~

Changes the size of a flob to the given value.

*/
    virtual bool resize(const SmiSize& newsize) {
       return FlobManager::getInstance().resize(*this,newsize);
     }


/*
~clean~

Cleans the flob. Actually, only the size is set to be zero.

*/
    virtual bool clean(){ return resize(0); }

/*
~read~

Reads a part of the persistent data referenced by this Flob. The ~buffer~ must
be provided and disposed by the caller of the funtion.

*/
    inline bool read(char* buffer,
                     const SmiSize length,
                     const SmiSize offset=0) const {
       return FlobManager::getInstance().getData(*this, buffer, offset, length);
    };

/*
~write~

Puts data provided in ~buffer~ into the persistent Flob storage using the
specified position.

*/
    inline bool write(const char* buffer,
                      const SmiSize length,
                      const SmiSize offset=0){
      if(offset+length > size) {
        if(!FlobManager::getInstance().resize(*this, offset+length)){
          return false;
        }
      }
      return FlobManager::getInstance().putData(*this, buffer, offset, length);
    }

/*
~setExFile~

Keep the Flob data in an external file, set the FlobId to it.

*/

  inline static bool setExFile(Flob& result, const string& flobFile,
      const SmiSize length, const SmiSize flobOffset){
    return FlobManager::getInstance().
        setExFile(result, flobFile, length, flobOffset);
  }

/*
~readExFile~

Read the data from the external file to a Flob with mode 1

*/
  inline static bool readExFile(Flob& result, const string& flobFile,
      const SmiSize length, const SmiSize flobOffset){
    return FlobManager::getInstance().
        SwitchToMode1(result, flobFile, length, flobOffset);
  }



  inline void changeMode(char m){
    FlobManager::getInstance().changeMode(this, m);
  }

/*
~copyFrom~

Copies the persistent flob data referenced by an other Flob ~src~ to the storage
belonging to this Flob.

*/
    virtual bool copyFrom(const Flob& src){
      if(!FlobManager::getInstance().resize(*this, src.size)){
       return false;
      }
      return FlobManager::getInstance().copyData(src,*this);
    };

/*
~copyTo~

Copies the content referenced by this flob to another Flob's (~dest~) persistent
storage.

*/
    virtual bool copyTo(Flob& dest) const {
       if(!FlobManager::getInstance().resize(dest, size)){
         return false;
       }
       return FlobManager::getInstance().copyData(*this,dest);
    };

/*

~saveToFile~

Save Flob data to a specified file/record/offset.
Returns a Flob having a FlobId encoding the file/record/offset
of the persistent Flob data.

Can e.g. be used to copy the flob data from the (temporary) native LOB
file to some persistent LOB file, like a relation LOB file or the
database LOB file. It can also be used to write the LOB data into a
tuple file. In this case, we speak of a FAKED LOB (or FLOB).

*/
    inline bool saveToFile(const SmiFileId& fid,
                          const SmiRecordId& rid,
                          const SmiSize& offset,
                          const char mode,
                          Flob& result) const{
      return FlobManager::getInstance().saveTo(*this, fid, rid, offset,
                                               mode, result);
    };

/*

~saveToFile~

Save Flob data to a specified file/record/offset.
Returns a Flob having a FlobId encoding the file/record/offset
of the persistent Flob data.

Can e.g. be used to copy the flob data from the (temporary) native LOB
file to some persistent LOB file, like a relation LOB file or the
database LOB file. It can also be used to write the LOB data into a
tuple file. In this case, we speak of a FAKED LOB (or FLOB).

*/
    inline bool saveToFile(SmiRecordFile* file,
                          const SmiRecordId rid,
                          const SmiSize offset,
                          Flob& result) const{
      return FlobManager::getInstance().saveTo(*this, file, rid,
                                               offset, result);
    };

/*
~saveToFile~

Saves this Flob to the given file id and returns the new created Flob
in parameter ~result~. The data is stored in a new record with offset zero.

*/
    inline bool saveToFile(const SmiFileId& fid,
                           const char mode ,
                          Flob& result) const{
      return FlobManager::getInstance().saveTo(*this, fid, mode, result);
    };


/*
~saveToFile~

Saves this Flob to the given file and returns the new created Flob
in parameter ~result~. The data is stored in a new record with offset zero.

*/
    inline bool saveToFile(SmiRecordFile* file,
                          Flob& result) const{
      return FlobManager::getInstance().saveTo(*this, file, result);
    };


/*
~getData~

Returns the stored data as a char array. The caller is responsible
for deleting the result using delete[]. If an error occurs, the result
will be null.

*/
    inline char* getData() const{
         char*res = new char[size];
         if(!read(res,size,0)){
             delete[] res;
             res = 0;
         }
         return res;
    }


/*
~createFrom~

Creates a Flob referencing a given (FileId/RecordId/Offset). The data is
expected to be already at the specified location.

For example, class tuple uses this to correct the FlobId when persistently storing
small Flob data within the tuple itself.

*/

    inline static Flob createFrom( const SmiFileId& fid,
                                   const SmiRecordId& rid,
                                   const SmiSize& offset,
                                   const char mode,
                                   const SmiSize& size    ) {
      return  FlobManager::getInstance().createFrom(fid, rid, offset,
                                                    mode,size);
    };


/*
~createFromBlock~

Does the evil thing. Loads a Flob's content from a file into a fixed main
memory buffer.
This is used to speed-up processing for small Flobs, and is only applied to
non-native Flobs.

*/
   inline static void createFromBlock(Flob& result, const char* buffer,
                                      const SmiSize& size, bool autodestroy){
      return  FlobManager::getInstance().createFromBlock(result,buffer,
                                                         size, autodestroy);
   }

/*
~getOffset~

Another very evil thing! returns the stored offset of the flob.
This member should be private to be able to capsulate the internal
flob representation. This funtion is required for supporting the
creation of evil flobs.

*/
   inline const SmiSize getOffset() const{
      return id.getOffset();
   }

/*
~getFileId~


*/
   inline const SmiFileId getFileId() const{
     return id.getFileId();
   }


   inline const char getMode(){
     return id.getMode();
   }

   inline const SmiRecordId getRecordId(){
     return id.getRecordId();
   }

/*
~destroy~

Destroys the referenced persistent data referenced by ~this~ Flob:

*/
  bool destroy(){
     return FlobManager::getInstance().destroy(*this);
  }


/*
~destroyIfNonPersistent~

Destroys ~this~ Flob if the Flob is a native one.

*/
   bool destroyIfNonPersistent(){
     return FlobManager::getInstance().destroyIfNonPersistent(*this);
   }


/*
~kill~

Kills the Flob without giving up any ressources.
This function must be used very carefully to avoid
memory leaks. The important thing is that the dataPointer
is set to 0. Thus, this function can be used if a Flob is
read from disk.


*/

   void kill(){
     id.destroy();
     size = 0;
     dataPointer = 0;
   }



/*
~saveHeader~

This function saves the header information of this Flob into a record.

*/
  virtual size_t serializeHeader(char* buffer,
                                 SmiSize& offset) const{
     assert(!dataPointer);
     WriteVar<SmiSize>(size, buffer, offset);
     WriteVar<FlobId>(id, buffer, offset);
     return headerSize();
  }

/*
~restoreHeader~

Restores header information.

*/
  virtual void restoreHeader(char* buffer,
                             SmiSize& offset) {
    assert(!dataPointer);
    ReadVar<SmiSize>(size, buffer, offset);
    ReadVar<FlobId>(id, buffer, offset);
  }

/*
~headerSize~

Returns the amount of data required to save header information.

*/

  inline static const size_t headerSize() {
     return sizeof(FlobId) + sizeof(SmiSize);;
  }

/*
~destroyManager~

This function destroys the ~FlobManager~. It should (and must) only be called
at the end of a Secondo session. After calling the function, Flob access
is not longer possible.

*/

  inline static bool destroyManager(){
     return FlobManager::destroyInstance();
  }

/*
~dropFile~

Gives up the FlobManager's control over the specific file.

*/
  inline static bool dropFile(const SmiFileId& id, const char mode){
    return FlobManager::getInstance().dropFile(id, mode);
  }

  inline static bool dropFiles(){
    return FlobManager::getInstance().dropFiles();
  }

  inline static void killAllNativeFlobs(){
    FlobManager::getInstance().killNativeFlobs();
  }

/*
~clearCaches~

Clears both (native and persistent) cashes. This is required after opening or
closing a database.

*/
  inline static void clearCaches(){
    FlobManager::getInstance().clearCaches();
  }

/*
Set parameters for the Native Flob Cache.

*/
   static void SetNativeCache(size_t maxSize,
                              size_t slotSize, size_t avgSize){
     FlobManager::SetNativeCache(maxSize, slotSize, avgSize);
   }

/*
Set parameters for the Persistent Flob Cache.

*/
   static void SetPersistentCache(size_t maxSize,
                              size_t slotSize, size_t avgSize){
     FlobManager::SetPersistentCache(maxSize, slotSize, avgSize);
   }

/*
Print function to print debug info about the Flob to an ostream.

*/
  ostream& print(ostream& os) const {
    if(!dataPointer){
       return os << "[" << id << ", size = " << size << "]";
    } else {
       return os << "[ data pointer, size = " << size << "]";
    }
  }

/*
Describe the Flob basic information for fetching its data in the Persistent storage.

*/
  string describe() const {
    if (!dataPointer){
      stringstream ss;
      ss << id.describe() << " " << size << endl;
      return ss.str();
    } else {
      return "error: not persistent\n";
    }
  }

  private:
    FlobId id;          // encodes fileid, recordid, offset
    SmiSize size;       // size of the Flob data segment in Bytes
    char*  dataPointer; // an evil pointer for direct saving data

    Flob(const FlobId& _id, const SmiSize& _size): id(_id), size(_size),
          dataPointer(0){}

};

/*
Shift operator to ease printing debug info to an ostream.

*/

ostream& operator<<(ostream& os, const Flob& f);

#endif
