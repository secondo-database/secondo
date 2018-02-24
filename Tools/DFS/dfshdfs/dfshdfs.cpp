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
#include "dfshdfs.h"
#include <stdio.h>
#include <stdlib.h>

using namespace dfs::hdfs;

HDFS::HDFS() {
  this->isOpen = false;
  this->open();
  this->pLogger = 0;
}

HDFS::~HDFS() {
  this->close();
}

void HDFS::setLogger(dfs::log::Logger *l) {
  this->pLogger = l;
}

void HDFS::open() {
  debug("open");
  if (this->isOpen) return;
  this->isOpen = true;
  debug("hdfsConnect");
  this->fs = hdfsConnect("default", 0);
}

void HDFS::close() {
  debug("close");
  if (!this->isOpen) return;
  this->isOpen = false;
  hdfsDisconnect(this->fs);
}

void HDFS::debug(const Str &msg) {
  if (this->pLogger) this->pLogger->debug(msg);
}

void HDFS::error(const Str &msg) {
  if (this->pLogger) this->pLogger->error(msg);
}

void HDFS::storeFile(const char *fileId, const char *content, long length,
                     const char *c) {

  debug("storeFile");

  hdfsFile writeFile = hdfsOpenFile(fs, fileId, O_WRONLY | O_CREAT, 0, 0, 0);
  tSize num_written_bytes = hdfsWrite(fs, writeFile, (void *) content, length);
  if (hdfsFlush(fs, writeFile)) {
    error("flush failed");
  }
  hdfsCloseFile(fs, writeFile);
  debug("done");
}

void HDFS::deleteFile(const char *fileId) {
  debug("deleteFile");
  hdfsDelete(fs, fileId, 0);
  debug("done");
}

void HDFS::receiveFileToLocal(const char *fileId, const char *localPath) {
  debug("receiveFileToLocal");
  debug("done");
}

void HDFS::appendToFile(const char *fileId, const char *appendix, long length) {
  debug("appendToFile");

  hdfsFile theFile = hdfsOpenFile(fs, fileId, O_WRONLY | O_APPEND, 0, 0, 0);
  hdfsWrite(fs, theFile, appendix, length);
  hdfsCloseFile(fs, theFile);

  debug("done");
}

UI64 HDFS::nextWritePosition(const char *fileId) {
  debug("nextWritePosition");
  debug("done");
}

void
HDFS::receiveFilePartially(const char *fileId, NUMBER startIndex, NUMBER length,
                           char *targetBuffer,
                           unsigned long targetBufferStartIndex) {
  debug("receiveFilePartially");

  hdfsFile theFile = hdfsOpenFile(fs, fileId, O_RDONLY, 0, 0, 0);
  hdfsPread(fs, theFile, startIndex, targetBuffer, length);
  hdfsCloseFile(fs, theFile);

  debug("done");
}

bool HDFS::hasFile(FILEID fileId) {
  return hdfsExists(fs, fileId);
}


