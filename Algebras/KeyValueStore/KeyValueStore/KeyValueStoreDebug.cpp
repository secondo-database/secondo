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

*/

#include "KeyValueStoreDebug.h"

#include "QuadTreeDistribution.h"
#include <fstream>

namespace KVS {

ostream KOUT(cout.rdbuf());
ostream ROUT(cout.rdbuf());

string DebugTime() {
  time_t now = time(NULL);
  struct tm* timeparts = localtime(&now);

  char buffer[32];
  strftime(buffer, 32, "%Y-%m-%d %H:%M:%S : ", timeparts);
  return string(buffer);
}

void SaveDebugFileStr(string path, string data) {
  ofstream out(path);
  if (out.is_open()) {
    out << data;
    out.close();
  }
}

void SaveDebugQuadNode(ofstream& out, QuadNode* node) {
  if (node == 0) {
    out << "0 ";
  } else {
    out << "\n((" << node->x << " " << node->y << " " << node->width << " "
        << node->height << " " << node->weight << " " << node->serverId << ") ";
    SaveDebugQuadNode(out, node->children[0]);
    SaveDebugQuadNode(out, node->children[1]);
    SaveDebugQuadNode(out, node->children[2]);
    SaveDebugQuadNode(out, node->children[3]);
    out << ")";
  }
}

void SaveDebugFile(string path, QuadTreeDistribution* dist) {
  ofstream out(path);
  if (out.is_open()) {
    out << "\n\n\n\n(" << dist->initialWidth << " " << dist->initialHeight
        << "\n(";

    for (unsigned int serverIdx = 0; serverIdx < dist->serverIdMapping.size();
         ++serverIdx) {
      if (serverIdx > 0) {
        out << " ";
      }
      out << dist->serverIdMapping[serverIdx];
    }
    out << ")\n";
    out << "(\n";
    SaveDebugQuadNode(out, dist->root);
    out << "\n)";
  }
}

void SaveDebugFile(string path, Distribution* dist) {
  if (dist->type == Distribution::TYPE_QUADTREE) {
    SaveDebugFile(path, (QuadTreeDistribution*)dist);
  }
}
}
