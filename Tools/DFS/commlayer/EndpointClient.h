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

#ifndef ENDPOINTCLIENT_H
#define ENDPOINTCLIENT_H

#include "../define.h"
#include "../shared/log.h"
#include "../shared/uri.h"
#include "../dfs/dfs.h"

namespace dfs {

  namespace comm {

    /**
     * client to endpoint
     */
    class EndpointClient {
    private:
      dfs::log::Logger *logger;

      void debug(const Str &s);

      void fatal(const Str &s);

      void fatalx(const Str &s);

    public:

      int debugFlag;

      int buffersize;

      EndpointClient() { buffersize = 32 * 1024; }

      /**
       * uses given logger
       * @param logger
       */
      void setLogger(dfs::log::Logger *logger);

      /**
       * sends message to endpoint
       * @param uri
       * @param s
       * @return
       */
      Str sendSyncMessage(URI uri, const Str &s, bool doEnvelope=true);
    };
  };
};

#endif /* ENDPOINTCLIENT_H */

