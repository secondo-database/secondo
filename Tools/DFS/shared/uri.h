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
#pragma once

#include "str.h"

namespace dfs {

  /**
   * represents an URI
   * might be a datanode or an indexnode
   * URI for datanode is dfs-data://host:port
   * URI for indexnode is dfs-index://host:port
   *
   * this call handles creation and parsing
   *
   */
  class URI {
  private:
    bool data;
  public:

    //Str protocol;

    /**
     * host of uri - localhost
     */
    Str host;

    /**
     * port of uri - 4444
     */
    Str port;

    /**
     * an optional path
     */
    Str path;

    /**
     * creates URI instance from URI string
     * @param s
     * @return
     */
    static URI fromString(const Str &s) {
      //dfs-index://xxxxx:p/
      int f = s.find("://");
      Str type = s.substr(0, f);

      URI u;
      int fl = s.findLastChar(':');

      //wir haben eine portangabe :xxxx
      u.data = type.find("-index") == -1;
      if (fl != f) {
        u.host = s.substr(f + 3, fl - (f + 3));
        u.port = s.substr(fl + 1);
      } else {
        u.host = s.substr(f + 3);
      }

      return u;
    }

    /**
     * factors up URI for indexnode
     * @param hostname
     * @param port
     * @return
     */
    static URI forIndexFromHostAndPort(const char *hostname, int port) {
      URI u;
      u.data = false;
      u.host = Str(hostname);
      u.port = Str(port);
      return u;
    }

    /**
     * dfs-index://10.123.123.10:65/
     */
    Str toString() const {
      Str s;
      s = s.append(data ? "dfs-data" : "dfs-index").append("://").append(host);

      if (port.len() > 0) s = s.append(":").append(port);
      return s.append(path);
    }

    /**
     * returns TRUE if this URI represents index node
     * @return
     */
    bool isIndex() {
      return !data;
    }

    /**
     * returns TRUE if this URI represents data node
     * @return
     */
    bool isData() {
      return data;
    }
  };

}