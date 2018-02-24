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

#ifndef DFS_CHUNK_H
#define DFS_CHUNK_H

#include "../shared/str.h"

using namespace dfs;

struct Chunk {

  /**
   * totalsize of the file the chunk is part of
   */
  //FIXME komplett entfernen int totalsize;

  /**
   * the size of the chunk
   */
  int chunksize;

  /**
   * the id of the chunk
   */
  Str chunkname;

  /**
   * the category of the file the chunk belongs to
   */
  Str category;

  Chunk() {
    chunksize = -1;
    category = 0;
  }

  /**
   * returns TRUE if this chunk is used or just reserved
   * @return
   */
  bool isUsed() { return chunksize != -1; }

  Str mapToDataPath(const Str &dataPath) {
    Str path = dataPath.append("/");
    if (category.len() > 0) path = path.append(category).append("/");
    return path.append(chunkname);
  }

  Str mapTargetDirToDataPath(const Str &dataPath) {
    Str path = dataPath.append("/");
    if (category.len() > 0) path = path.append(category);
    return path;
  }


};

#endif //DFS_CHUNK_H
