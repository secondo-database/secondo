/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
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

//[$][\$]

1 Implementation of ~MappedChunkFile~

Every platform dependency of the nested list storage is in this file. 
Both branches do the same three things: create a file that is deleted the
moment it is created, extend it and map a slice of it, and release a slice.
Neither ever moves or replaces a mapping already handed out.

*/

#include "SecondoConfig.h"   // decides SECONDO_WIN32

#include <atomic>
#include <cstdint>
#include <cstring>
#include <string>

#ifndef SECONDO_WIN32
#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "BigArray.h"


/*
1.1 Naming

The name a caller passes is a *prefix*, not the name of the file. It has to be:
the file is deleted the moment it exists and only the mapping reaches it
afterwards, so the name is a handle nobody ever looks up -- while every
~NestedList~ in a process asks for the same three
("temp\_nested\_list\_nodes" and its two siblings). Creating the file under the
caller's name directly is then a race between two threads that both do it: they
share one inode, and each truncates what the other has already mapped.

Process id and a counter make the prefix this instance's own; exclusive creation
is what rules the collision out rather than making it unlikely, and retrying
covers the case the suffix does not -- a leftover file from a dead process whose
id has since come round again. Spelled out in both branches rather than using
~mkstemp~, which has no Win32 counterpart.

*/
static std::string candidateName(const std::string& prefix)
{
  static std::atomic<uint64_t> counter(0);
#ifndef SECONDO_WIN32
  const unsigned long pid = (unsigned long) ::getpid();
#else
  const unsigned long pid = (unsigned long) ::GetCurrentProcessId();
#endif
  return prefix + "." + std::to_string(pid) + "."
       + std::to_string(counter.fetch_add(1, std::memory_order_relaxed));
}

/*
How many names to try before giving up. Reaching the end means every one of them
was taken by a file this process did not create, which is a broken directory
rather than a collision.

*/
static const int MAX_NAME_ATTEMPTS = 32;


#ifndef SECONDO_WIN32

/*
2 POSIX

The file is unlinked straight away: the descriptor and the mappings keep the
inode alive for exactly as long as the array needs it, and the kernel reclaims
it however this process ends. Deleting in the destructor instead meant that a
crash, a kill or a power cut left the file behind -- which it did, by the
gigabyte, from processes that had been dead for weeks.

*/

MappedChunkFile::MappedChunkFile(const std::string& filename)
  : fname(), fd(-1), handle(0)
{
  for(int attempt = 0; attempt < MAX_NAME_ATTEMPTS; attempt++){
    const std::string candidate = candidateName(filename);
    // O_EXCL, so a name another thread or process got to first is refused
    // rather than shared. Without it two openers of one name end up on the
    // same inode and O_TRUNC discards what the other has mapped.
    const int f = ::open(candidate.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600);
    if(f >= 0){
      fname = candidate;
      fd = f;
      ::unlink(fname.c_str());
      return;
    }
    if(errno != EEXIST){
      throw SecondoException("Could not open file " + candidate + ": "
                             + strerror(errno));
    }
  }
  throw SecondoException("Could not find an unused name for " + filename
                         + " in " + std::to_string(MAX_NAME_ATTEMPTS)
                         + " attempts");
}


MappedChunkFile::~MappedChunkFile()
{
  if(fd >= 0){
    ::close(fd);
    fd = -1;
  }
}


void* MappedChunkFile::map(const size_t offset, const size_t bytes)
{
  // Extended first: a mapping may reach past the end of the file, but touching
  // those pages raises SIGBUS rather than growing it.
  if(::ftruncate(fd, (off_t) (offset + bytes)) != 0){
    throw SecondoException("Could not extend file " + fname + ": "
                           + strerror(errno));
  }
  void* p = ::mmap(0, bytes, PROT_READ | PROT_WRITE, MAP_SHARED,
                   fd, (off_t) offset);
  if(p == MAP_FAILED){
    throw SecondoException("Could not map file " + fname + ": "
                           + strerror(errno));
  }
  return p;
}


void MappedChunkFile::unmap(void* base, const size_t bytes)
{
  if(base){
    ::munmap(base, bytes);
  }
}


#else

/*
3 Windows

FILE\_FLAG\_DELETE\_ON\_CLOSE is how the POSIX branch's ~unlink~ is spelled
here: the file goes away once the last handle closes, however the process ends.
It needs FILE\_SHARE\_DELETE to be allowed at all. FILE\_ATTRIBUTE\_TEMPORARY
asks Windows to avoid writing pages back while it has the memory to hold them,
which is what this file is for.

~CreateFileMapping~ extends the file to the requested size itself, so there is
no counterpart to ~ftruncate~. The section handle is closed as soon as the view
exists: a view holds its own reference to the section, so it stays valid
afterwards and there is nothing per mapping left to keep.

*This branch is compiled but has never been run.* It is type-checked against a
stub windows.h; see the comment in Tests/tbigarray.cpp.

*/

static std::string lastError()
{
  return "error " + std::to_string((unsigned long) ::GetLastError());
}


MappedChunkFile::MappedChunkFile(const std::string& filename)
  : fname(), fd(-1), handle(0)
{
  for(int attempt = 0; attempt < MAX_NAME_ATTEMPTS; attempt++){
    const std::string candidate = candidateName(filename);
    // CREATE_NEW is the counterpart of O_EXCL: it fails rather than opening
    // a file somebody else is already mapping. See the naming note above.
    HANDLE h = ::CreateFileA(candidate.c_str(),
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE
                               | FILE_SHARE_DELETE,
                             0, CREATE_NEW,
                             FILE_ATTRIBUTE_TEMPORARY
                               | FILE_FLAG_DELETE_ON_CLOSE, 0);
    if(h != INVALID_HANDLE_VALUE){
      fname = candidate;
      handle = (void*) h;
      return;
    }
    const DWORD err = ::GetLastError();
    if(err != ERROR_FILE_EXISTS && err != ERROR_ALREADY_EXISTS){
      throw SecondoException("Could not open file " + candidate + ": "
                             + lastError());
    }
  }
  throw SecondoException("Could not find an unused name for " + filename
                         + " in " + std::to_string(MAX_NAME_ATTEMPTS)
                         + " attempts");
}


MappedChunkFile::~MappedChunkFile()
{
  if(handle){
    ::CloseHandle((HANDLE) handle);
    handle = 0;
  }
}


void* MappedChunkFile::map(const size_t offset, const size_t bytes)
{
  const uint64_t end = (uint64_t) offset + (uint64_t) bytes;
  HANDLE hMap = ::CreateFileMappingA((HANDLE) handle, 0, PAGE_READWRITE,
                                     (DWORD) (end >> 32),
                                     (DWORD) (end & 0xFFFFFFFFull),
                                     0);
  if(hMap == 0){
    throw SecondoException("Could not extend file " + fname + ": "
                           + lastError());
  }
  const uint64_t off = (uint64_t) offset;
  void* p = ::MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS,
                            (DWORD) (off >> 32),
                            (DWORD) (off & 0xFFFFFFFFull),
                            (SIZE_T) bytes);
  const std::string why = (p == 0) ? lastError() : std::string();
  ::CloseHandle(hMap);   // the view keeps the section alive
  if(p == 0){
    throw SecondoException("Could not map file " + fname + ": " + why);
  }
  return p;
}


void MappedChunkFile::unmap(void* base, const size_t)
{
  if(base){
    ::UnmapViewOfFile(base);
  }
}

#endif
