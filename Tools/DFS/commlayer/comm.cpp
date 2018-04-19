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

#include "comm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

using namespace std;

using namespace dfs::comm;

Endpoint::Endpoint(const Str &name) {
  bufsize = 1024;
  handlerMode = 0;
  this->name = name;
}

void Endpoint::debugEmptyLine() {
  if (this->logger != 0) this->logger->debug(Str(""));
}

void Endpoint::setSingleMessageHandler(PFUNC_SMH *x) {
  smh = x;
}

void Endpoint::listen() {

  bool canDebug = this->logger != 0 && this->logger->canDebug;


  if (canDebug) this->debug("Endpoint.listen");

  int sockfd;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    this->fatal("socket - cannot create socket");
    return;
  }
  int canReuse = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &canReuse, sizeof(int));
  debug(Str("socket - using port ").append(port));

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
  }

  if (canDebug) debug("socket - bind() successful");
  ::listen(sockfd, 5);
  if (canDebug) debug("socket - waiting for connections...");

  clilen = sizeof(cli_addr);
  ClientHandler *clientHandler;

  while (true) {

    if (canDebug) debug("Endpoint.listen: awaiting clients");
    int newsockfd = accept(sockfd,
                           (struct sockaddr *) &cli_addr,
                           &clilen);
    if (canDebug) debug("Endpoint.listen: client connected");

    if (handlerMode == 1) {
      if (canDebug) debug("calling clientHandler.onStart");
      clientHandler = handlerFactory->createHandler();
      if (canDebug) debug("obtained handler");
      clientHandler->onStart();
    }

    if (canDebug) debug("prepare data exchange with client");

    char buffer[bufsize];
    bzero(buffer, bufsize);
    Str input;
    int totalReceived = 0;
    if (canDebug) debug("fetching data from client");
    long promisedLength = -1;
    while (true) {

      if (canDebug) debug("starting recv");
      n = recv(newsockfd, buffer, bufsize, 0);
      if (canDebug) debug(Str("got data from recv (Bytes) - ").append(n));

      //erster Teil wurde erreicht
      if (totalReceived == 0 && n > 0 && buffer[0] == '@') {
        if (canDebug) debug("first part, an expected length has been promised");
        if (n < 15)
          fatalx("expected length has been promised, but it is invalid");
        //promisedLength = Str(buffer, n).substr(1, 14).toLong();
        promisedLength = Str(buffer+1,14).toLong();
        if (canDebug)
          debug(Str("expected length (Bytes) - ").append(promisedLength));
      }

      if (n <= 0) {
        if (canDebug) debug("end recv-processing");
        break;
      }

      if (n >= 0) {
        totalReceived += n;
      }

      //if (restlength > -1 && totalReceived)

      if (canDebug)
        debug(Str("got bytes sum (BytesTillNow) ").append(totalReceived));

      input.appendRawBufferToThis(buffer,n);
      //input = input.append(Str(buffer, n));

      if (promisedLength > -1 && totalReceived >= promisedLength) {
        if (canDebug)
          debug(Str("promised length ").append(promisedLength).append(
            " has been reached ").append(
            totalReceived));
        break;
      }

    }
    if (canDebug)
      debug(Str(
        "fetching data from client - done - amount of received bytes - ")
          .append(totalReceived));
    if (canDebug)
      debug(Str("server got buffer from client - size ").append(input.len()));

    if (promisedLength > -1) {
      if (canDebug)
        debug("data has been sent with promised length, removing envelope now");

      //substr sehr teurer aufruf,
      //deswegen intern startindex verschoben,
      //gibt aber beim freigeben probleme
      //also unten wieder umschieben
      input.changeStartIndexNoFree(15);
      //input = input.substr(15);

      if (input.len() >= 4) {
        if (canDebug)
          debug(
            Str("new buffer start chars - ").append(input.substr(0, 4)).append(
              "..."));
      }
    }

    int flags = 0;
    Str response;
    if (handlerMode == 0) {
      response = smh(&input, &flags);
    } else {
      response = clientHandler->onReceived(&input, &flags);
    }
    input.changeStartIndexNoFree(-15);

    if (canDebug) {
      debug(Str("flags ").append(flags));
      debug(Str("output created (length) - ").append(response.len()));
      debugEmptyLine();
    }

    if ((flags & 0x00000001) == 1) {
      if (handlerMode == 1) clientHandler->onEnd();
      //debug("Verbindung geschlossen");
    }

    if (canDebug)
      debug(Str("write result to client (bytes) - ").append(response.len()));
    if (canDebug) debug(Str("result ").append(response));

    n = send(newsockfd, response.buf(), response.len(), 0);
    if (n < 0) fatal("ERROR writing to socket");
    close(newsockfd);
    if (handlerMode == 1) clientHandler->onEnd();

    if ((flags & 0x00000002) == 2) {
      if (canDebug) debug("node wants to be quitted");
      break;
    }

  }

  close(sockfd);
}

void Endpoint::debug(const Str &s) {
  Str m = Str("EndpointServer ").append(name).append(": ").append(s);
  if (this->logger != 0) this->logger->debug(m);
}

void Endpoint::fatal(const Str &s) {
  if (this->logger != 0) {
    this->logger->fatal(
      Str("EndpointServer ").append(name).append(": ").append(s));
  }
}

void Endpoint::fatalx(const Str &s) {
  this->fatal(s);
  char tmp[128];
  char *cs = s.cstr();
  strcpy(tmp, cs);
  delete[] cs;
  throw ResultException(tmp);
}


void Endpoint::setLogger(dfs::log::Logger *logger) {
  this->logger = logger;
}

