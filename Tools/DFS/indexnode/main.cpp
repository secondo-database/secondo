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
#include "IndexNode.h"
#include "../shared/log.h"
#include "../shared/io.h"
#include "../shared/ParameterHelper.h"
#include <pthread.h>
#include "../commlayer/WebServer.h"
#include "../shared/maschine.h"
#include "../shared/debug.h"

using namespace dfs;
using namespace dfs::log;
using namespace std;

const int DEFAULT_INDEX_PORT = 4444;
const int DEFAULT_WEBSERVER_PORT = 44440;
const Str VERSION = "1.0.0";

IndexNode indexNode;

class whandler : public dfs::comm::WebServerRequestHandler {
public:

  IndexNodeManager *man;

  virtual Str simpleHtmlOutput(const Str &resource) {
    StrBuilder builder(1024 * 1014, 32768);
    builder.appendCStr("<h1>Information about the index node</h1>");

    builder.append(
      Str("<h2>General information</h2><table>")
        .append("<tr><td><b>ID:</b></td><td>").append(man->id).append(
          "</td></tr>")
        .append("<tr><td><b>max chunk size [bytes]:</b></td><td>").append(
          man->config.maxChunkSize).append("</td></tr>")
        .append("<tr><td><b>replica count:</b></td><td>").append(
          man->config.replicaCount).append("</td></tr>")
    );

    //Datenknoten
    builder.append(
      Str("</table><h2>Data nodes</h2>").append("<p>count: ").append(
        man->dataNodeIndex.count()).append("</p>"));

    builder.appendCStr("<table>");
    for (std::map<Str, DataNodeEntry>::iterator
           kv = man->dataNodeIndex.index.begin();
         kv != man->dataNodeIndex.index.end(); kv++) {
      builder.appendCStr("<tr>");
      builder.appendCStr("<tr>");
      builder.appendCStr("<td style=\"color:#009900;\">");
      builder.append(kv->second.uri.toString());
      builder.appendCStr("</td>");
      builder.appendCStr("</tr>");
    }
    builder.appendCStr("</table>");

    //Dateien
    builder.appendCStr("<h2>Files</h2>");
    builder.append(Str("<p>count: ").append(man->countFiles()).append("</p>"));
    builder.appendCStr(
      "<table border=\"1\"><thead><tr><th>global FileID</th><th>Chunks</th>"
        "<th>Size [B]</th><th>chunk info</th></tr></thead>");
    for (std::map<Str, IndexEntry>::iterator kv = man->fileIndex.begin();
         kv != man->fileIndex.end(); kv++) {
      builder.appendCStr("<tr>");
      builder.appendCStr("<td style=\"padding-right:25px;\" valign=\"top\">");
      builder.append(kv->second.fileId);
      builder.appendCStr("</td>");
      builder.appendCStr("<td valign=\"top\">");
      builder.append(kv->second.chunkInfoListLength);
      builder.appendCStr("</td>");
      builder.appendCStr("<td valign=\"top\">");
      builder.append(kv->second.calculateFileSize());
      builder.appendCStr("</td>");
      builder.appendCStr("<td><table>");


      for (int i = 0; i < kv->second.chunkInfoListLength; i++) {
        ChunkInfo *p = &kv->second.chunkInfoList[i];
        builder.appendCStr("<tr><td>");
        builder.append(
          Str(p->offsetInFile).append(" ").append(p->length).append(" "));
        builder.appendCStr("</td><td valign\"top\"><table>");

        builder.appendCStr(
          "<tr><th>local chunkID</th><th>datanode-uri</th></tr>");
        for (int j = 0; j < p->chunkLocationListLength; j++) {
          ChunkLocation *loc = &p->chunkLocationList[j];
          builder.appendCStr("<tr><td>");
          builder.append(loc->chunkId);
          builder.appendCStr("</td><td>");
          builder.append(loc->dataNodeUri.toString());
          builder.appendCStr("</td></tr>");
        }

        builder.appendCStr("</table></td></tr>");
      }
      builder.appendCStr("</table></td>");
      builder.appendCStr("</tr>");
    }
    builder.appendCStr("</table>");
    return Str(builder.buf(), builder.currentPos());
  }
};

void *indexThread(void *arg) {

  ParameterHelper *pHelper = (ParameterHelper *) arg;

  bool canDebug = pHelper->hasParameter("X");
  FormattedCompositeLogger *logger = new FormattedCompositeLogger();
  logger->canDebug = canDebug;

  //command line parameters
  int port = pHelper->hasParameter("p") ? pHelper->getParameter("p").toInt()
                                        : DEFAULT_INDEX_PORT;
  Str dataDir = pHelper->hasParameter("d") ? pHelper->getParameter("d")
                                           : dfs::io::file::combinePath(
      dfs::io::file::currentDir(), "data");
  io::file::createDir(dataDir);

  if (pHelper->hasParameter("r"))
    indexNode.getManager()->config.replicaCount = pHelper->getParameterInt("r");
  if (pHelper->hasParameter("chunksize"))
    indexNode.getManager()->config.maxChunkSize = pHelper->getParameterInt(
      "chunksize");

  //file logger to data dir
  FileLogger l2;
  l2.canDebug = canDebug;
  l2.setFilename(dfs::io::file::combinePath(dataDir, "debug.log"));
  logger->add(&l2);
  Debug::logger = logger;

  //audit logger
  FormattedFileLogger alogger;
  alogger.setFilename(dfs::io::file::combinePath(dataDir, "audit.log"));
  alogger.info("starting index node");

  logger->debug("prepare new instance of indexnode");
  indexNode.port = port;
  indexNode.setLogger(logger);
  indexNode.setAuditLogger(&alogger);
  indexNode.getManager()->id = dfs::Maschine::volatileId(Str("i").append(port));
  indexNode.getManager()->dataPath = dataDir;

  logger->debug("try to restore state");
  indexNode.getManager()->tryToRestoreState();
  logger->debug("state restored");
  try {
    indexNode.run();
    exit(0);
  } catch (BaseException &e) {
    cerr << "FATAL BaseException " << e.what() << endl;
    throw e;
  } catch (const char *cstr) {
    cerr << "FATAL " << cstr << endl;
    throw cstr;
  }
}

void *webserverThread(void *arg) {

  ParameterHelper *pHelper = (ParameterHelper *) arg;
  bool canDebug = pHelper->hasParameter("X");
  Str dataDir = pHelper->hasParameter("d") ? pHelper->getParameter("d")
                                           : dfs::io::file::combinePath(
      dfs::io::file::currentDir(), "data");
  io::file::createDir(dataDir);

  whandler h;
  h.man = indexNode.getManager();

  FormattedCompositeLogger *pLogger = new FormattedCompositeLogger();
  pLogger->canDebug = canDebug;

  FileLogger l1;
  l1.canDebug = canDebug;
  l1.setFilename(dfs::io::file::combinePath(dataDir, "webserver-debug.log"));
  pLogger->add(&l1);

  DefaultOutLogger l2;
  l2.canDebug = canDebug;
  pLogger->add(&l2);

  pLogger->debug("starting web server");
  dfs::comm::WebServer server;
  server.displayName = "HttpIndexNode";
  server.port = DEFAULT_WEBSERVER_PORT;
  server.logger = pLogger;
  server.pHandler = &h;
  server.listen();

  return 0; // ???
}

void displayHelp() {
  cout << "indexnode - the index process for KFS" << endl;
  cout
    << "usage: indexnode [help] [-p<port>] [-pw<webport>] [-d<datadirectory>]"
    << endl;
  cout << "\t support switches" << endl << endl;
  cout << "\t-p<port> uses <port> as communication port - default ist 4444"
       << endl;
  cout
    << "\t-pw<webport> uses <webport> as webport. if <webport> is 0, "
      "no webserver will be started"
    << endl;
  cout
    << "\t-d<dir> uses <dir> as data directory. must be fully accessable by "
      "indexnode process"
    << endl;
  cout << "\t-X enables debug logging" << endl;
  cout << endl << endl;
  cout << "\tsupported commands" << endl << endl;
  cout << "\thelp - displays this help" << endl;
}

int main(int argc, char *argv[]) {

  //get command line parameters
  ParameterHelper helper = ParameterHelper(argc, argv);

  if (helper.hasCommand("help")) {
    displayHelp();
    return 1;
  }

  pthread_t tindex, twebserver;

  pthread_create(&tindex, NULL, indexThread, (void *) &helper);
  pthread_create(&twebserver, NULL, webserverThread, (void *) &helper);
  pthread_join(tindex, NULL);
  pthread_join(twebserver, NULL);

  return 0;
}
