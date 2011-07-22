/*
 Copyright (C) 2010 by The Regents of the University of California
 
 Redistribution of this file is permitted under
 the terms of the BSD license.
 
 Date: 11/01/2009
 Author: Shengyue Ji <shengyuj (at) ics.uci.edu>
*/

#include <stdlib.h>
#include "storage.h"

namespace rstartree {

Storage::Storage()
{
    rootId = 0;
}

Storage::Storage(const char *file)
{
    rootId = 0;
}

Storage::~Storage()
{
}

Buffer *Storage::alloc(size_t size)
{
    Buffer *buf = (Buffer *)malloc(size);
    buf->id = (uintptr_t)buf;
    buf->size = size;
    return buf;
}

void Storage::dealloc(size_t id)
{
    ::free((void *)id);
}

Buffer *Storage::read(uintptr_t id)
{
    return (Buffer *)id;
}

char *Storage::readPart(uintptr_t id, unsigned offset, unsigned size)
{
    return (char *)(id + offset);
}

void Storage::write(const Buffer *buf)
{
    // do nothing
}

void Storage::free(Buffer *buf)
{
    // do nothing
}

unsigned Storage::getRoot()
{
    return rootId;
}

void Storage::setRoot(uintptr_t id)
{
    rootId = id;
}

} // end of namespace


