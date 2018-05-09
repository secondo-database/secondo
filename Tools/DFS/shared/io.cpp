/*
----
This file is part of SECONDO.
Realizing a simple distributed filesystem for master thesis of stephan scheide

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

*/
#include "io.h"
#include "str.h"
#include <unistd.h>
#include <stdio.h>

#ifdef DOLINUX

#include <fcntl.h>
#include <sys/stat.h>
#include <cstring>


#if (_POSIX_C_SOURCE < 200112L) 
  int posix_fallocate(int fd, off_t offset, off_t len){
    size_t bs = len<4096?len:4096;
    char buffer[bs];
    memset(buffer, 0, bs);
    size_t remaining = len;
    while(remaining > 0) {
      size_t n = remaining<bs?remaining:bs; 
      ssize_t r = write(fd,buffer,n);
      if(r==-1){
        close(fd);
        return 1;
      }
      remaining -= r;
    }
    if(close(fd)==-1) return 1;
    return 0;
  }
#endif

#endif

using namespace dfs;
using namespace dfs::io::file;


void dfs::io::file::allocate(const char *filename, unsigned long long bytes) {

#ifdef DOLINUX
  int fd = open(filename, O_RDWR | O_CREAT, 0666);
  posix_fallocate(fd, 0, bytes);
#endif
}

void
dfs::io::file::createFile(const Str &filename, long bytes, const Str &strategy,
                          const Str &args1) {
  //wir gehen davon aus, dass immer die Strategy "ten" verwendet wird
  const char *tmp = "0123456789";
  const char *alphabet = "abcdefghijklmnopqrstuvwxyz";
  const char *acFilename = filename.cstr();

  FILE *fp = fopen(acFilename, "w");
  delete[] acFilename;

  if (strategy == "ten") {
    for (long i = 0; i < bytes; i++) {
      fwrite(&tmp[i % 10], 1, 1, fp);
    }
  } else if (strategy == "single") {
    char c = args1[0];
    for (long i = 0; i < bytes; i++) {
      fwrite(&c, 1, 1, fp);
    }
  } else if (strategy == "alphabet") {
    for (long i = 0; i < bytes; i++) {
      fwrite(&alphabet[i % 26], 1, 1, fp);
    }
  } else if (strategy == "255") {
    for (long i = 0; i < bytes; i++) {
      char c = i%256;
      fwrite(&c, 1, 1, fp);
    }
  }

  fflush(fp);
  fclose(fp);
}

void dfs::io::file::deleteFile(const Str &filename) {
  remove(CStr(filename).cstr());
}

long dfs::io::file::fileSize(const Str &filename) {
#ifdef DOLINUX
  struct stat64 s;
  CStr cs = CStr(filename);
  if (stat64(cs.cstr(), &s) == -1) {
    std::cout << "FAIL" << std::endl;
  }
  return s.st_size;
#endif
  return -1;
}

char *dfs::io::file::getFileContent(const Str &filename) {
  int size = dfs::io::file::fileSize(filename);
  FILE *fp = fopen(CStr(filename).cstr(), "r");
  char *buf = new char[size];
  fread(buf, size, 1, fp);
  fclose(fp);
  return buf;
}


Str dfs::io::file::combinePath(const Str &a, const Str &b) {
  int l = a.len();
  if (l == 0) return Str(b);
  if (a[l - 1] == '/') {
    return a.append(b);
  } else {
    return a.append("/").append(b);
  }
}

Str dfs::io::file::currentDir() {
#ifdef DOLINUX
  char tmp[512];
  ssize_t s = readlink("/proc/self/exe", tmp, sizeof(tmp) - 1);
  Str x = Str(tmp, s);
  if (x[x.len() - 1] == '/') x = x.substr(0, x.len() - 1);
  int ls = x.findLastChar('/');
  return x.substr(0, ls);
#endif
  //TODO windows
}

void dfs::io::file::createDir(const Str &dir) {
#ifdef DOLINUX
  char *ddir = dir.cstr();
  mkdir(ddir, S_IRWXU);
  delete[] ddir;
#endif
  //TODO windows
}

void dfs::io::file::deleteEmptyDirOnlySafe(const Str &dir) {
#ifdef DOLINUX
  char *ddir = dir.cstr();
  rmdir(ddir);
  delete[] ddir;
#endif
}

Writer::Writer(const Str &filename) {
  char *s = filename.cstr();
  fp = fopen(s, "w");
  delete[] s;
}

Writer::Writer(const Str &filename, bool append) {
  char *s = filename.cstr();
  fp = fopen(s, append ? "a" : "w");
  delete[] s;
}


void Writer::append(int i) {
  fwrite((void *) &i, sizeof(int), 1, fp);
}

void Writer::append(Str &s) {
  fwrite(s.buf(), s.len(), 1, fp);
}

void Writer::append(const char *buffer, UI64 bufferLength) {
  fwrite(buffer, bufferLength, 1, fp);
}

void Writer::appendWithLengthInfo(short lenlen, Str &s) {
  int l = s.len();
  Str sl = Str(l).prepend(lenlen, '0');
  append(sl);
  append(s);
}

void Writer::writeBufferAt(unsigned long long offsetInFile,
                           unsigned long bufferLength, const char *buffer) {
  fseek(fp, offsetInFile, SEEK_SET);
  fwrite(buffer, bufferLength, 1, fp);
}

void Writer::close() {
  fclose(fp);
}
