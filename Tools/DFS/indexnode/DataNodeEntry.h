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
#ifndef DATANODEENTRY_H
#define DATANODEENTRY_H

#include "../shared/str.h"
#include "../shared/uri.h"

struct DataNodeEntry : dfs::SerializeAble {

  dfs::URI uri;
  int usage;
  int life;

  DataNodeEntry() {
    usage = 0;
    life = 0;
  }

  virtual void serializeTo(dfs::ToStrSerializer &ser) const {
    ser.append(uri.toString());
    ser.append(usage, 12);
    ser.append(life, 12);
  }

  static DataNodeEntry deserialize(const dfs::Str &str) {
    DataNodeEntry e;
    dfs::StrReader reader(&str);
    e.uri = dfs::URI::fromString(reader.readStrSer());
    e.usage = reader.readInt(12);
    e.life = reader.readInt(12);
    return e;
  }

};


#endif /* DATANODEENTRY_H */
