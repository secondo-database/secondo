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

The class ~Flob~ provides a handle on large objects. The user can use the classe's
member functions to create new Flobs, to restore Flobs from persistent staorage,
and to to read or modify the contained data. However, the actual work for all
these functions is done by the friend class ~FlobManager~, whose member functions
are called.

Flob objects (instances of this class) have a compact memeory representation and
can be made persistent using any of Secondo's persistence mechanisms. This of
course only stores the handle to the object data. The actual object data is
maintained in some files provided and managed by the singleton instance of class
FlobManager.

The user may read and change Flob data in buffers. Managing that buffers is up
to the user. To read a Flob's object data, the read() function is called. It
copies the requested data to a buffer provided by the user.
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

class Flob{
  friend class FlobManager;
  friend class FlobCache;
  public:

/*
~Standard Constructor~

This constructor should only be used within Cast functions.

*/
    inline Flob() {}

/*
~Copy Constructor~

Creates a flat copy of the argument. The underlying data are not
duplicated.

*/
  inline Flob(const Flob& other) : id(other.id), size( other.size ) {};

/*
~Constructor~

This construcvtor create a native flob having a given size.

*/
    inline Flob (SmiSize size_): id(), size( size_ ){
      FlobManager::getInstance().create(size_, *this);
    };


/*
~Constructor~

Constructs a Flob from a given position.

*/
    inline Flob(const SmiFileId fileId,
                const SmiRecordId recordId,
                const SmiSize offset) : id(), size( 0 ){
         FlobManager::getInstance().create(fileId, recordId,
                                           offset, size, *this);
    };

/*
~Assignment Operator~

Makes only a flat copy of the source flob.

*/
    inline Flob& operator=(const Flob& src){
      size = src.size;
      id   = src.id;
      return *this;
    };

/*
~Comparison~

*/
   inline bool operator==(const Flob& f) const{
     return (id == f.id);
   }

   inline bool operator>(const Flob& f) const{
     return id > f.id;
   }

   inline bool operator<(const Flob& f) const{
     return id < f.id;
   }


/*
~Destructor~

Does not destroy the underlying data. Use destroy for it.

*/
    virtual ~Flob() {};

/*
~getSize~

Returns the current size of this Flob.

*/
    inline SmiSize getSize() const { return size; }; // Size of the FLOB

/*
~resize~

Changes the size of a flob to the given one.

*/
    virtual void resize(const SmiSize& newsize) { 
       FlobManager::getInstance().resize(*this,newsize);
     }


/*
~clean~

Cleans the flob. actually only the size is set to be zero.

*/
    virtual void clean(){ resize(0); }

/*
~read~

Reads a part of the data from this Flob. The ~buffer~ must be provided and disposed
by the caller of the funtion.

*/
    inline void read(char* buffer,
                     const SmiSize length,
                     const SmiSize offset=0) const {
       FlobManager::getInstance().getData(*this, buffer, offset, length);
    };

/*
~write~

Puts data providen in ~buffer~ into the Flob at the specified position.

*/
    inline void write(const char* buffer,
                      const SmiSize length,
                      const SmiSize offset=0){
      if(offset+length > size) {
        FlobManager::getInstance().resize(*this, offset+length);
      }
      FlobManager::getInstance().putData(*this, buffer, offset, length);
    }
/*
~copyFrom~

Copies the content of src to this flob.

*/
    virtual void copyFrom(const Flob& src){
      FlobManager::getInstance().copyData(src,*this);
      FlobManager::getInstance().resize(*this, src.size);
    };

/*
~copyTo~

Copies the content of this flob to ~dest~.

*/
    virtual void copyTo(Flob& dest) const {
       FlobManager::getInstance().copyData(*this,dest);
       FlobManager::getInstance().resize(dest, size);
    };

/*

~saveToFile~

Save Flob data to a specified file/record/offset.
Returns a Flob having a FlobId encoding the file/record/offset
of the persistent Flob data
Can e.g. be used to copy the flob data from the (temporary) native LOB
file to some persistent LOB file, like a relation LOB file or the
database LOB file. It can also be used to write the LOB data into a
tuple file. In this case, we speak of a FAKED LOB (or FLOB).

*/
    inline bool saveToFile(const SmiFileId fid,
                          const SmiRecordId rid,
                          const SmiSize offset,
                          Flob& result) const{
      return FlobManager::getInstance().saveTo(*this, fid, rid, offset, result);
    };

/*

~saveToFile~

Save Flob data to a specified file/record/offset.
Returns a Flob having a FlobId encoding the file/record/offset
of the persistent Flob data
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
    inline bool saveToFile(const SmiFileId fid,
                          Flob& result) const{
      return FlobManager::getInstance().saveTo(*this, fid, result);
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
~createFrom~

Creates a Flob referencing a given (FileId/RecordId/Offset). The data is
expected to be already at the specified location.

For example class tuple uses this to correct the FlobId when persistently storing
small Flob data within the tuple itself.

*/

    inline static Flob createFrom( const SmiFileId& fid,
                                   const SmiRecordId& rid,
                                   const SmiSize& offset,
                                   const SmiSize& size    ) {
      return  FlobManager::getInstance().createFrom(fid, rid, offset, size);
    };



/*
~destroy~

Destroys this;

*/
  void destroy(){
     FlobManager::getInstance().destroy(*this);
  }



/*
~saveHeader~

This function saves the header information of this Flob into a record.

*/
  virtual size_t serializeHeader(char* buffer,
                                 SmiSize& offset) const{
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

This function destroys the FlobManager. It should (and must) only called
at the end of a secondo session. After calling that function Flob access
is not longer possible.

*/

  inline static bool destroyManager(){
     return FlobManager::destroyInstance();
  }

/*
~dropFile~

Gives up the control of the FlobManager over the specific file.

*/
  inline static bool dropFile(const SmiFileId& id){
    return FlobManager::getInstance().dropFile(id);
  }

  inline static bool dropFiles(){
    return FlobManager::getInstance().dropFiles();
  }


  ostream& print(ostream& os) const {
    return os << "[" << id << ", size = " << size << "]";
  }

  private:
    FlobId id;       // encodes fileid, recordid, offset
    SmiSize size;    // size of the Flob data segment in Bytes
};

ostream& operator<<(ostream& os, const Flob& f);

#endif
