/*
 
 Copyright (C) 2010 by The Regents of the University of California
 
 Redistribution of this file is permitted under
 the terms of the BSD license.
 
 Date: 11/01/2009
 Author: Shengyue Ji <shengyuj (at) ics.uci.edu>


*/

#include <stdio.h>
#include <vector>
#include <stdint.h>
#include <tr1/unordered_map>

#ifndef STORAGE_H
#define STORAGE_H


using namespace std;
using namespace tr1;

namespace rstartree {

// Buffers managed by Storage should extend this
class Buffer
{
public:
    // identifier of the buffer
    uintptr_t id;

    // size of the Buffer in bytes (including self)
    size_t size;
};

class Storage
{
    // id of the root buffer
    uintptr_t rootId;

    ////// * disk * storage specific
    // file descriptor for disk storage
    FILE *file;

    ////// disk mem storage specific
    // buffer for disk mem storage
    char *buffer;

    ////// mem disk storage specific
    // file size for mem disk storage
    unsigned size;
    unsigned size2;
    // buffer index for mem to disk storage (disk address -> index in buffers)
    unordered_map<unsigned, unsigned> index;
    // buffer array for mem to disk storage
    vector<Buffer *> buffers;

public:
    // usage for mem disk 2
    // 0: mem disk (for tree nodes)
    // 1: disk (for extension nodes)
    unsigned usage;

    // memory storage should use this
    Storage();

    // disk storage should use this
    // memory storage can also use this to load from disk
    Storage(const char *filename);

    ~Storage();

    // allocate a Buffer of size on the storage and return the buffer
    Buffer *alloc(size_t size);

    // deallocate the Buffer by id from storage
    void dealloc(uintptr_t id);

    // return the Buffer identified by id
    // disk storage should allocate Buffer also
    // memory storage should directly return the Buffer
    Buffer *read(uintptr_t id);

    // return a buffer (not a Buffer object) read from node id,
    // starting from offset (relative to the begining of the Buffer)
    char *readPart(uintptr_t id, unsigned offset, unsigned size);

    // write the Buffer identified by id to disk
    // memory storage should do nothing
    void write(const Buffer *buf);

    // free the Buffer returned from disk read and allocation
    // memory storage should do nothing
    void free(Buffer *buf);

    // return the buffer id of the root
    unsigned getRoot();

    // set the buffer id of the root
    // disk storage should also write to file
    void setRoot(uintptr_t id);

};


} // end of namespace rstartree

#endif 

