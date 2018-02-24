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

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "../shared/str.h"
#include "../shared/log.h"

namespace dfs {

  namespace comm {

    /**
     * handler for requests
     */
    class WebServerRequestHandler {
    public:
      virtual Str simpleHtmlOutput(const Str &resource) = 0;
    };

    /**
     * describes web server
     */
    class WebServer {
    private:
      void debug(const Str &s);

      void fatal(const Str &s);

      void handleRequest(const Str &request, int socket);

      void handleResource(const Str &res, int socket);

      void writeHtmlResponse(const Str &html, int socket);

    public:

      WebServer() {
        port = 44440;
        pHandler = 0;
      }

      dfs::log::Logger *logger;

      Str displayName;

      int port;

      WebServerRequestHandler *pHandler;

      void listen();
    };

  };
};

#endif /* WEBSERVER_H */

