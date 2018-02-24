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

#include "../define.h"
#include "../shared/log.h"
#include "../shared/uri.h"
#include "../dfs/dfs.h"

#ifdef DOLINUX
#endif

#ifdef DOWIN32
#endif

namespace dfs {

  namespace comm {

    typedef Str (PFUNC_SMH)(const Str *s, int *resultFlags);

    /**
     * describes handler for complete request
     */
    class ClientHandler {
    public:
      virtual void onStart() = 0;

      virtual Str onReceived(Str *s, int *resultFlags) = 0;

      virtual void onEnd() = 0;
    };

    /**
     * describes factory for creating client handlers
     */
    class ClientHandlerFactory {
    public:
      virtual ClientHandler *createHandler() = 0;
    };

    /**
     * abstraction of endpoint
     * mostly IP endpoint using sockets
     */
    class Endpoint {
    private:

      Str name;
      dfs::log::Logger *logger;
      PFUNC_SMH *smh;

      void debug(const Str &s);

      void debugEmptyLine();

      void fatal(const Str &s);

      void fatalx(const Str &s);

    public:
      //handlerMode:0 --> setSingleMessageHandler, 1: factory
      int handlerMode;
      int port;
      int bufsize;
      ClientHandlerFactory *handlerFactory;

      Endpoint(const Str &name);

      void setLogger(dfs::log::Logger *logger);

      void setSingleMessageHandler(PFUNC_SMH *x);

      void listen();
    };
  };

};