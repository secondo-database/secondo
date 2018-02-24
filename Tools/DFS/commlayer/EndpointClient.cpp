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

#include "EndpointClient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <errno.h>
#include "RemoteCommandBuilder.h"

using namespace std;

using namespace dfs;
using namespace dfs::comm;

Str EndpointClient::sendSyncMessage(URI uri, const Str &msg) {

  bool canDebug = this->logger->canDebug;

  if (canDebug) this->debug("open");

  int portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  portno = uri.port.toInt();
  char *csHost = uri.host.cstr();
  if (canDebug) this->debug(Str("hostname is ").append(csHost));
  if (canDebug) this->debug(Str("port is ").append(portno));

  int openSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (openSocket < 0) {
    fatalx("Konnte Socket nicht anlegen");
  }
  server = gethostbyname(csHost);
  if (server == NULL) {
    fatalx("host not found");
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr,
        (char *) &serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(openSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <
      0) {
    fatalx("error on connecting");
  }

  if (canDebug) debug("sendSyncMessage - Start");
  if (canDebug)
    debug(Str(
      "Laenge der zu sendenten Nachricht ohne Briefumschlag (Laenge) - ")
            .append(msg.len()));

  Str sendbuffer = RemoteCommandBuilder::sizeEnvelope(msg);
  if (canDebug)
    debug(Str("Umschlag mit Laengeninfo hinzugefuegt (Umschlag) - ").append(
      sendbuffer.substr(0, 15)));

  int bytesSent = send(openSocket, sendbuffer.buf(), sendbuffer.len(), 0);
  if (canDebug)
    debug(Str("Daten an Server gesendet. Bytes - ").append(bytesSent));
  if (bytesSent < 0) {
    fatalx("could not send bytes to server");
  }

  int ibufsize = buffersize;
  char ibuf[ibufsize];
  bzero(ibuf, ibufsize);

  if (canDebug) debug("Hole Serveranwort...");
  Str result;
  int bytesRead = 0;
  int loopCount = 1;

  while (true) {
    int n = recv(openSocket, ibuf, ibufsize, 0);
    if (canDebug)
      debug(Str(
        "Teilantwort vom Server erhalten (Iteration) - (Bytes/Code) - ").append(
        loopCount++).append(" ").append(n));
    if (n > 0) {
      bytesRead += n;
      if (canDebug) debug("Haenge an Ergebnis an ");
      result = result.append(Str(ibuf, n));
      if (canDebug)
        debug(
          Str("neue Ergebnislaenge ist nun (Laenge) ").append(result.len()));
    }
    if (n == -1) {
      if (canDebug) debug(Str("recv liefert -1 (errno) ").append(errno));
    }
    if (n <= 0) break;
  }

  close(openSocket);

  if (canDebug)
    debug(Str("Serverantwort erhalten (Bytes) - ").append(bytesRead));
  if (canDebug) debug(Str("Serverantwort - ").append(result));
  return result;
}

void EndpointClient::debug(const Str &s) {
  if (!this->logger->canDebug) return;
  Str m = Str("EndpointClient ").append(s);
  if (this->logger != 0) this->logger->debug(m);
}

void EndpointClient::fatal(const Str &s) {
  Str m = Str("Endpoint: ").append(s);
  if (this->logger != 0) this->logger->fatal(m);
}

void EndpointClient::setLogger(dfs::log::Logger *logger) {
  this->logger = logger;
}

void EndpointClient::fatalx(const Str &s) {
  if (this->logger != 0) this->logger->fatal(s);
  char tmp[128];
  char *cs = s.cstr();
  strcpy(tmp, cs);
  delete[] cs;
  throw ResultException(tmp);
}
