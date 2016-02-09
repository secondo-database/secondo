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

std::ostream KOUT(cout.rdbuf());
std::ostream ROUT(cout.rdbuf());

std::string DebugTime() {
  time_t now = time(NULL);
  struct tm* timeparts = localtime(&now);

  char buffer[32];
  strftime(buffer, 32, "%Y-%m-%d %H:%M:%S : ", timeparts);
  return std::string(buffer);
}

void SaveDebugFileStr(std::string path, std::string data) {
  std::ofstream out(path);
  if (out.is_open()) {
    out << data;
    out.close();
  }
}

void SaveDebugQuadNode(std::ofstream& out, QuadNode* node) {
  if (node == 0) {
    out << "0 ";
  } else {
    out << "\r\n((" << node->x << " " << node->y << " " << node->width << " "
        << node->height << " " << node->weight << " " << node->serverId << " "
        << node->maxGlobalId << ") ";
    SaveDebugQuadNode(out, node->children[0]);
    SaveDebugQuadNode(out, node->children[1]);
    SaveDebugQuadNode(out, node->children[2]);
    SaveDebugQuadNode(out, node->children[3]);
    out << ")";
  }
}

void SaveDebugFile(std::string path, QuadTreeDistribution* dist) {
  std::ofstream out(path);
  if (out.is_open()) {
    out << "\r\n\r\n\r\n\r\n(" << dist->initialWidth << " "
        << dist->initialHeight << "\r\n(";

    for (unsigned int serverIdx = 0; serverIdx < dist->serverIdOrder.size();
         ++serverIdx) {
      if (serverIdx > 0) {
        out << " ";
      }
      out << dist->serverIdOrder[serverIdx];
    }
    out << ")\r\n";
    out << "(\r\n";
    SaveDebugQuadNode(out, dist->root);
    out << "\r\n)";
  }
}

void SaveDebugFile(std::string path, Distribution* dist) {
  if (dist->type == Distribution::TYPE_QUADTREE) {
    SaveDebugFile(path, (QuadTreeDistribution*)dist);
  }
}
}
