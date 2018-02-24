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

#include "../commlayer/comm.h"
#include "../shared/log.h"
#include "State.h"
#include "DataNode.h"
#include "DataNodeClientHandler.h"
#include "../shared/io.h"
#include "../shared/ParameterHelper.h"
#include "../commlayer/WebServer.h"
#include "../shared/format.h"
#include "DataNodeConfig.h"
#include <pthread.h>

using namespace dfs::log;
using namespace std;

const int DEFAULT_PORT = 4445;
const int DEFAULT_PORT_WEB = 44450;

class whandler : public dfs::comm::WebServerRequestHandler {
public:

  DataNode *pDataNode;

  virtual Str simpleHtmlOutput(const Str &resource) {
    StrBuilder builder(1024 * 1024, 4096);
    //StrBuilder builder(1,1);

    builder.appendCStr(
      "<h1>Inhalt des Datenknotens</h1><h2>Chunks</h2><table><tr><th>"
        "Chunkname</th><th>Kategorie</th><th>Groesse</th></tr>");

    int c = -1;
    int cused = 0;

    Chunk *chunks = pDataNode->state()->copyState(&c);

    for (int i = 0; i < c; i++) {

      bool used = chunks[i].isUsed();
      Str style = "";
      if (i % 2 == 1) {
        style = "style=\"background-color:#eeeeee;\"";
      }

      if (!used) {
        continue;
      }
      cused++;

      builder.append("<tr><td " + style + ">");
      builder.append(chunks[i].chunkname);
      builder.append("</td><td " + style + ">");
      builder.append(chunks[i].category);
      builder.append("</td><td " + style + ">");
      builder.append(dfs::format::sizei(chunks[i].chunksize));
      builder.appendCStr("</td></tr>");
    }

    builder.appendCStr("</table><h2>Weitere Informationen</h2><table>");
    builder.append(Str("<tr><td><b>ID:</b></td><td>").append(
      pDataNode->getMaschineId()).append("</td></tr>"));
    builder.append(
      Str("<tr><td><b>Anzahl reserviert:</b></td><td>").append(c).append(
        "</td></tr>"));
    builder.append(Str("<tr><td><b>Anzahl Chunks genutzt:</b></td><td>").append(
      cused).append("</td></tr></table>"));

    delete[] chunks;
    return Str(builder.buf(), builder.currentPos());
  }
};

void *dataNodeThread(void *arg) {
  DataNode *pnode = (DataNode *) arg;
  pnode->pLogger->debug("THREAD dataNodeThread: DataNode.run");
  try {
    pnode->run();
    cout << "dataNodeDone" << endl;
  }
  catch (BaseException &e) {
    cerr << e.what() << endl;
    throw e;
  }
  catch (const char *buf) {
    cerr << buf << endl;
    throw buf;
  }
}


void *webServerThread(void *arg) {
  dfs::comm::WebServer *pserver = (dfs::comm::WebServer *) arg;
  pserver->logger->debug("THREAD webServerThread: WebServer.listen");
  pserver->listen();
}


int main(int argc, char *argv[]) {

  dfs::ParameterHelper helper(argc, argv);

  CompositeLogger *logger = new CompositeLogger();
  bool canDebug = helper.hasParameter("X");
  logger->canDebug = canDebug;

  DataNodeConfig config;
  config.maxClients = 8;
  config.name = "DataNode"; //FIXME Kennung

  config.port = helper.hasParameter("p") ? helper.getParameter("p").toInt()
                                         : DEFAULT_PORT;
  Str dataDir = helper.hasParameter("d") ? helper.getParameter("d")
                                         : dfs::io::file::combinePath(
      dfs::io::file::currentDir(), "data");
  io::file::createDir(dataDir);
  config.webport = helper.hasParameter("pw") ? helper.getParameter("pw").toInt()
                                             : DEFAULT_PORT_WEB;
  config.dataDir = dataDir;

  //log data path
  if (logger->canDebug) logger->debug(Str("data path is ").append(dataDir));

  //create logger
  FileLogger l2;
  l2.setFilename(dfs::io::file::combinePath(dataDir, "debug.log"));
  logger->add(&l2);

  //log params
  if (canDebug) logger->debug(Str("data path ").append(dataDir));
  if (canDebug) logger->debug(Str("data node port").append(config.port));
  if (canDebug) logger->debug(Str("web server port").append(config.webport));

  //prepare node
  DataNode node;
  node.config = config;
  node.pLogger = logger;

  //prepare webserver
  whandler h;
  h.pDataNode = &node;

  //restore state
  node.state()->restoreFromFile(node.mapToDir("state"));

  if (canDebug) logger->debug("preparing web server");
  dfs::comm::WebServer server;
  server.displayName = "HttpDataNode";
  server.port = config.webport;
  server.logger = logger;
  server.pHandler = &h;

  //threads erzeugen
  pthread_t tdatanode, twebserver;
  pthread_create(&tdatanode, NULL, dataNodeThread, (void *) &node);
  pthread_create(&twebserver, NULL, webServerThread, (void *) &server);
  pthread_join(tdatanode, NULL);
  pthread_join(twebserver, NULL);

  if (canDebug) logger->debug("data node - end");
  delete logger;

  return 0;

}
