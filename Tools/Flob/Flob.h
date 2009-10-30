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

#include "FlobId.h"
#include "FlobManager.h"
#include "../include/SecondoSMI.h"

class Flob{
  public:
    // native FLOB with size=0
    inline Flob() : id(), size( 0 ) {
      FlobManager::getInstance().create(size, *this);
    }

    // copy constructor, copies FLOB, not the data
    inline Flob(const Flob&) : id(other.id), size( other.size ) {};

    // construct a native Flob having a given size
    inline Flob (unint64_t size_) id(), size( size_ ){
      FlobManager::getInstance().create(size, *this);
    };

    // restore a Flob from a file/record/offset
    inline Flob(const FileId fileId,
                const RecordId recordId,
                const uint64_t offset) : id(), size( 0 ){
      FlobManager::getInstance().create(fileId, recordId, offset, *this);
    };

    // assign another Flob to this Flob
    inline Flob& operator=(const Flob& src){
      size = src.size;
      id   = src.id;
      return *this;
    };

    // Destructor. Does not delete the Flob data.
    inline ~Flob() {};

    // return the Flob's current size   
    inline unit64_t getSize() const { return size; }; // Size of the FLOB

    // just reset size
    inline void resize(unint64_t newsize) { size = newsize };

    // copy data from a Flob to a provided buffer
    inline void read(char* buffer,
                     const uint64_t length,
                     const uint64_t offset) const {
      FlobManager::getInstance().getData(*this, buffer, offset, length);
    };

    // write data from a provided buffer to the Flob
    inline void write(const char* buffer,
                      const uint64_t length,
                      const uint64_t offset){
      FlobManager::getInstance().putData(*this; buffer, offset, length);
    }

    // copy Flob from another Flob
    inline void copyFrom(const Flob& src){
      size = src.size;
      id = src.id;
    };

    // copy Flob to another Flob
    inline void copyTo(Flob& dest) const {
      dest.size = size;
      dest.id = id;
    };

    // Save Flob data to a specified file/record/offset.
    // Returns a Flob having a FlobId encoding the file/record/offset
    // of the persistent Flob data
    // Can e.g. be used to copy the flob data from the (temporary) native LOB
    // file to some persistent LOB file, like a relation LOB file or the
    // database LOB file. It can also be used to write the LOB data into a
    // tuple file. In this case, we speak of a FAKED LOB (or FLOB).
    inline Flob saveToFile(const FileId fid,
                    const RecordId rid,
                    const unit64_t offset) const{
      return FlobManager::getInstance().saveTo(*this, fid, rid, offset);
    };

  private:
    id   : FlobId;       // encodes fileid, recordid, offset
    size : unint64_t;    // size of the Flob data segment in Bytes
};

#endif
