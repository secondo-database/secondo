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
#include "dfs.h"

using namespace dfs;

void
SimpleFilesystemReader::useFile(SimpleFilesystem *pNewDfs, const char *fileId) {

  if (this->currentFileId != 0) {
    delete[] this->currentFileId;
    this->currentFileId = 0;
  }

  this->currentFileId = new char[strlen(fileId) + 1];
  strcpy(this->currentFileId, fileId);
  this->pDfs = pNewDfs;
  this->curPosition = 0;
}

SimpleFilesystemReader::SimpleFilesystemReader() {
  this->currentFileId = 0;
}

SimpleFilesystemReader::~SimpleFilesystemReader() {
  delete[] this->currentFileId;
  this->currentFileId = 0;
}

void SimpleFilesystemReader::changePosition(UI64 newPosition) {
  this->curPosition = newPosition;
}

void SimpleFilesystemReader::receiveBuffer(char *targetBuffer, NUMBER length) {
  this->pDfs->receiveFilePartially(this->currentFileId, this->curPosition,
                                   length, targetBuffer, 0);
}

UI64 SimpleFilesystemReader::getCurrentPosition() {
  return curPosition;
}
