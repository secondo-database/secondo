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

#include "WebServer.h"

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

void WebServer::debug(const Str &s) {
  if (!logger->canDebug) return;
  Str x = Str("WebServer ").append(displayName).append(" ").append(s);
  logger->debug(x);
}

void WebServer::fatal(const Str &s) {
  Str x = Str("WebServer ").append(displayName).append(" ").append(s);
  logger->fatal(x);
}

void WebServer::handleRequest(const Str &r, int socket) {
  if (r.find("GET") == 0) {
    debug("GET Request");
    Str res = r.substr(4);
    res = res.substr(0, res.find(" "));
    debug(Str("Ressource ").append(res));
    handleResource(res, socket);
  } else {
    this->logger->error(Str("invalid request").append(r));
  }
}

void WebServer::writeHtmlResponse(const Str &html, int socket) {
  debug("writeHtmlResponse");
  Str r = Str(
    "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
      "Connection: Closed\r\nContent-Length: ").append(
    html.len()).append("\r\n\r\n").append(html);
  send(socket, r.buf(), r.len(), 0);
  close(socket);
}


void WebServer::handleResource(const Str &res, int socket) {
  bool canDebug = this->logger->canDebug;

  if (canDebug) debug("handleResource");

  if (pHandler != 0) {
    if (canDebug) debug("Lasse HTML vom Handler erzeugen");
    Str html = pHandler->simpleHtmlOutput(res);
    writeHtmlResponse(html, socket);
  } else {
    if (canDebug) debug("kein Handler");
    writeHtmlResponse(Str("<h1>WebServer bereit - kein Handler angegeben</h1>"),
                      socket);
  }

}

void WebServer::listen() {

  bool canDebug = this->logger->canDebug;
  if (canDebug) debug("Starte Webserver");

  int sockfd;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    this->fatal("Socket - Kann keinen Socket erzeugen");
    return;
  }
  int canReuse = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &canReuse, sizeof(int));

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
  }

  ::listen(sockfd, 5);

  clilen = sizeof(cli_addr);
  int bufsize = 65536;
  char buf[bufsize];

  //auf clients warten
  debug("Warte auf Clients...");
  while (true) {
    int clientSocket = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (canDebug) debug("Client hat Verbindung aufgenommen");

    //Daten nun laden
    while (true) {
      if (canDebug) debug("Hole Daten von Client");
      int n = recv(clientSocket, buf, bufsize, 0);
      if (canDebug) debug(Str("Daten erhalten (Bytes) ").append(n));

      if (n > 0) {
        Str part = Str(buf, n);
        if (canDebug) debug(Str("Daten ").append(part));
        handleRequest(part, clientSocket);
      } else {
        break;
      }
    }

    close(clientSocket);
    if (canDebug) debug("Verbindug zum Client geschlossen");
  }

  close(sockfd);

}

