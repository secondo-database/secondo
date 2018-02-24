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

#include <iostream>

#include "./cli.h"
#include "../dfs/dfs.h"
#include "../dfs/remotedfs.h"
#include "../shared/ParameterHelper.h"
#include "../shared/log.h"
#include "../shared/uri.h"
#include "../commlayer/comm.h"
#include "../commlayer/EndpointClient.h"
#include "../shared/io.h"
#include "../commlayer/RemoteCommandBuilder.h"

using namespace std;
using namespace dfs;
using namespace dfs::remote;

bool canDebug = false;

void displayHelp() {

  line("command line interface for DFS");
  line("general usage: cli <cmd> [arguments]");
  line("<cmd> might be");
  line();
  line("help");
  line("\tshows this help");
  line();

  //basic user operations
  line("file-store <indexurl> <localpath> [fileId]");
  line("\tstores a file from local path to given DFS cluster identified by");
  line("<indexurl>\n\tfileId is the identifier within the DFS.");
  line("if not given, local path is used");
  line();

  line("file-store-buffer <indexurl> <fileid> <buffer>");
  line("\tstores a buffer as a new file on DFS identified by <indexurl>");
  line();

  line("file-append-localfile <indexurl> <fileId> <localPath>");
  line("\tappends content from local file to file on DFS identified by "
         "<indexurl>.\n\tif file is not present it will be created");
  line();

  line("file-append-buffer <indexurl> <fileId> <buffer>");
  line("\tappends content from parameter to file on DFS identified by "
         "<indexurl>.\n\tif file is not present it will be created");
  line();

  line("file-append-buffer-size-randombytes <indexurl> <fileId> <size>");
  line("\tappends content to file on DFS identified by <indexurl>."
         "\n\tif file is not present it will be created.\n\tcontent"
         " will be generated"
         " in memory with random bytes of size <size> bytes");
  line();

  line("file-load <indexurl> <fileId> <localPath>");
  line("\tloads a file from the DFS cluster identified by <indexurl> and"
         "stores it as the local file <localPath>\n\tfileid is"
         " the identifier within the distributed filesystem.");
  line();

  line("file-load-part-print <indexurl> <fileId> <offset> <length>");
  line("\tloads a part from the DFS cluster identified by <indexurl>"
         " and prints it to default output. the partial buffer is a"
         " subpart of the"
         " file.\n\t<offset> is beginning with in the file, <length>"
         " the amount of "
         "bytes to be read starting at <offset>.");
  line();

  line("file-load-part-store <indexurl> <fileId> <offset> <length> "
         "<localFilePath>");
  line("\tloads a part from the DFS cluster identified by <indexurl> and"
         " writes it to local file. the partial buffer is a subpart"
         " of the file.\n\t<offset> is beginning with in the file,"
         " <length> the amount of bytes to be read starting at"
         " <offset>.");
  line();

  line("file-exists <indexurl> <fileId>");
  line("\tchecks whether a file in the dfs cluster identified by "
         "<indexurl>\n\tfileid is the identifier within the "
         "distributed filesystem.");
  line();

  line("file-delete <indexurl> <fileid>");
  line("\tdeleted a file from DFS using given <fileid>");
  line();

  line("file-delete-all <indexurl>");
  line("\tdeletes all files from DFS");
  line();

  line("file-delete-all-category <indexurl> <category>");
  line("\tdeletes all files belonging to given category");
  line();

  line("file-list <indexurl>");
  line("\tlists all files located in DFS identified by <indexurl>");
  line();

  line("file-count <indexurl>");
  line("\tcounts all files located in DFS identified by <indexurl>");
  line();

  line("file-totalsize <indexurl>");
  line("\treturn the sum of all file sizes located in DFS identified by"
         " <indexurl>");
  line();

  //admin operations
  line("datanode-register <indexurl> <dataurl>");
  line("\tregisters a datanode in DFS-cluster");
  line();

  line("datanode-register-easy <indexurl> <datahost> <dataport>");
  line("\tregisters a datanode in DFS-cluster with host and port.");
  line();

  line("datanode-hash-print <datanodeurl>");
  line("\tprints hash of a data node to default output");
  line();

  line("datanode-stat <datanodeurl>");
  line("\tprints information about the data node");
  line();

  line("datanode-stop <datanodeurl>");
  line("\tstops a data node - can simulate a disaster");
  line();

  line("datanode-list <indexurl>");
  line("\treturns amount of active data nodes in cluster");
  line();

  line("change-setting <indexurl> <key> <value>");
  line("\tchanges a setting within the DFS");
  line("\t<key> can be chunksize when value is an integer");
  line();

  //local file utils
  line("localfile-create <targetpath> <sizeInBytes>|<size>(k|m) <strategy>"
         " [args1]");
  line("\tcreates a local file of given size with a content strategy");
  line("\t<strategy> may be: ten - using the characters 0..9 for creating"
         " the file size");
  line("\t<strategy> may be: single - using a single char identified by "
         "args1 <size> times");
  line();

  //advanced debugging
  line("echo <url> <data>");
  line("\techos data to given endpoint");
  line();

  line("send-raw <url> <raw>");
  line("\tsends any raw command to url. url can be indexnode or datanode");
  line();

  line("echosize <url> <sizeInBytes>|<size>(k|m)");
  line("\tcreated random data of given size and sends it to endpoint");
  line();

  line();
}

long sizeFromStr(const Str &ssize) {
  char csize = ssize.last();
  long size = 0;

  if (csize == 'm' || csize == 'M')
    size = ssize.substr(0, ssize.len() - 1).toLong() * 1024 * 1024;
  else if (csize == 'k' || csize == 'K')
    size = ssize.substr(0, ssize.len() - 1).toLong() * 1024;
  else
    size = ssize.toLong();
  return size;
}

int main(int argc, char *argv[]) {

  bool doMeasure = false;
  bool beSilent = false;

  try {

    dfs::log::DefaultOutLogger logger;

    ParameterHelper h(argc, argv);
    logger.canDebug = canDebug;
    doMeasure = h.hasParameter("M");
    canDebug = h.hasParameter("X");
    beSilent = h.hasParameter("Q");

    Duration duration;

    if (doMeasure) duration.start();

    Str cmd = "";
    if (argc > 1)
      cmd = h.word(0);

    if (cmd == "test") {
      line("cli ready");
    } else if (cmd == "send-raw") {
      URI uri = URI::fromString(h.word(1));
      Str d = h.word(2);
      debugc(cmd, Str("execute raw command - ").append(d));

      dfs::comm::EndpointClient ec;
      ec.setLogger(&logger);
      Str result = ec.sendSyncMessage(uri, d);
      debugc(cmd, Str("endpoint result ").append(result));
      cout << result << endl;
    } else if (cmd == "echo") {
      URI uri = URI::fromString(h.word(1));
      Str d = Str("echo").append(h.word(2));
      debugc(cmd, Str("Fuehre Befehl aus - ").append(d));

      dfs::comm::EndpointClient ec;
      ec.setLogger(&logger);
      Str result = ec.sendSyncMessage(uri, d);
      debugc(cmd, Str("Server-Ergebnis ").append(result));
    } else if (cmd == "echosize") {
      URI uri = URI::fromString(h.word(1));
      long size = sizeFromStr(h.word(2));

      //dfs::RemoteCommandBuilder builder("echo",true);
      //builder.setBody(Str::createSingleChar(size,'0'));

      Str data = Str("echo").append(Str::createSingleChar(size, '0'));

      dfs::comm::EndpointClient ec;
      ec.setLogger(&logger);
      debugc(cmd,
             Str("Sende Daten (Laenge) an Server - ").append(data.len()));

      Str result = ec.sendSyncMessage(uri, data);
      int lr = result.len();
      debugc(cmd, Str("Serverergebnis (nur Laenge) - ").append(lr));
    } else if (cmd == "localfile-create") {
      Str target = h.word(1);
      long size = sizeFromStr(h.word(2));
      Str strat = h.word(3);
      if (canDebug)
        cout << "create a localfile " << target << " of size " << size
             << " with strategy " << strat << endl;

      if (strat == "ten")
        dfs::io::file::createFile(target, size, strat);
      else if (strat == "single")
        dfs::io::file::createFile(target, size, strat, h.word(4));
      else if (strat == "alphabet")
        dfs::io::file::createFile(target, size, strat);


    } else if (cmd == "file-list") {
      line("listing files on index");
      URI uri = URI::fromString(h.word(1));
      RemoteFilesystem r(uri, &logger);
      if (canDebug)
        cout << "fetching files from indexnode " << uri.toString()
             << endl;
      std::vector<string> names = r.listFileNames();
      if (canDebug) cout << "count fetched: " << names.size() << endl;
      for (std::vector<string>::iterator value = names.begin();
           value != names.end(); value++) {
        cout << *value << endl;
      }
      if (canDebug) cout << "DONE" << endl;
    } else if (cmd == "file-count") {
      line("counting files on index");
      URI uri = URI::fromString(h.word(1));
      RemoteFilesystem r(uri, &logger);
      int count = r.countFiles();
      cout << count << endl;
    } else if (cmd == "file-totalsize") {
      line("summing up all sizes");
      URI uri = URI::fromString(h.word(1));
      RemoteFilesystem r(uri, &logger);
      UI64 totalsize = r.totalSize();
      cout << totalsize << endl;
    } else if (cmd == "file-load") {
      line("loads file from DFS to local");
      URI uri = URI::fromString(h.word(1));
      if (canDebug) debug(Str("index node is ").append(uri.toString()));
      Str fileId = h.word(2);
      if (canDebug) debug(Str("fileId is ").append(fileId));
      Str localPath = h.word(3);
      if (canDebug) debug(Str("localPath is ").append(localPath));

      RemoteFilesystem r(uri, &logger);
      r.receiveFileToLocal(CStr(fileId).cstr(), CStr(localPath).cstr());
    } else if (cmd == "file-load-part-print") {
      URI uri = URI::fromString(h.word(1));
      Str fileId = h.word(2);
      UI64 offset = h.word(3).toUInt64();
      UI64 length = h.word(4).toUInt64();
      RemoteFilesystem r(uri, &logger);

      //for string output need terminating \0
      char *buf = new char[length + 1];
      buf[length] = 0;

      r.receiveFilePartially(CStr(fileId).cstr(), offset, length, buf);
      cout << "received partial file" << endl;
      cout << buf << endl;
    } else if (cmd == "file-load-part-store") {
      URI uri = URI::fromString(h.word(1));
      Str fileId = h.word(2);
      UI64 offset = h.word(3).toUInt64();
      UI64 length = h.word(4).toUInt64();
      Str localFilePath = h.word(5);
      RemoteFilesystem r(uri, &logger);
      char *buf = new char[length];
      r.receiveFilePartially(CStr(fileId).cstr(), offset, length, buf);

      dfs::io::file::Writer writer(localFilePath, false);
      writer.append(buf, length);
      writer.close();

    } else if (cmd == "file-exists") {
      line("check whether file exists");
      URI uri = URI::fromString(h.word(1));
      if (canDebug) debug(Str("index node is ").append(uri.toString()));
      Str fileId = h.word(2);
      if (canDebug) debug(Str("fileId is ").append(fileId));
      RemoteFilesystem r(uri, &logger);
      bool value = r.hasFile(CStr(fileId).cstr());
      cout << (value ? 1 : 0) << endl;
    } else if (cmd == "file-append-localfile") {
      //urii fileid localfile
      line("append content from local file to file of dfs");
      URI uri = URI::fromString(h.word(1));
      Str fileId = h.word(2);
      Str localSourceFile = h.word(3);

      RemoteFilesystem r(uri, &logger);
      r.appendToFileFromLocalFile(CStr(fileId).cstr(),
                                  CStr(localSourceFile).cstr());
    } else if (cmd == "file-append-buffer") {
      //urii fileid localfile
      line("append content from parameter file to file of dfs");
      URI uri = URI::fromString(h.word(1));
      Str fileId = h.word(2);
      Str buffer = h.word(3);
      RemoteFilesystem r(uri, &logger);
      r.appendToFile(CStr(fileId).cstr(), CStr(buffer).cstr(),
                     buffer.len());
    } else if (cmd == "file-store-buffer") {
      line("stores new file from parameter to file on dfs");
      URI uri = URI::fromString(h.word(1));
      Str fileId = h.word(2);
      Str buffer = h.word(3);
      RemoteFilesystem r(uri, &logger);
      r.storeFile(CStr(fileId).cstr(), CStr(buffer).cstr(), buffer.len());
    } else if (cmd == "file-append-buffer-size-randombytes") {
      URI uri = URI::fromString(h.word(1));
      Str fileId = h.word(2);
      UI64 size = h.word(3).toUInt64();
      line(Str("creating random buffer of size ").append(size).append(
        " and append it to file ").append(fileId));

      char *buffer = new char[size];
      for (UI64 i = 0; i < size; i++)
        buffer[i] = numberUtils::randInt(0, 255);
      RemoteFilesystem r(uri, &logger);
      r.appendToFile(CStr(fileId).cstr(), buffer, size);
    } else if (cmd == "file-store") {

      if (!beSilent) line("store new file from local");
      URI uri = URI::fromString(h.word(1));
      if (canDebug) cout << "index node is " << uri.toString() << endl;
      RemoteFilesystem r(uri, &logger);
      CStr localPath(h.word(2));
      Str fileId = h.word(2);
      Str category;

      if (canDebug) cout << "local file is " << localPath.cstr() << endl;
      if (canDebug) cout << "storing file from local path now" << endl;

      if (h.hasNumberOfArguments(4)) {
        fileId = h.word(3);
        if (canDebug) cout << "wished fileid is " << fileId << endl;
      }
      if (canDebug) cout << "fileid is " << fileId << endl;

      if (h.hasNumberOfArguments(5)) {
        category = h.word(4);
        if (canDebug) cout << "wished category is " << category << endl;
      }


      if (category.len() > 0) {
        r.storeFileFromLocal(CStr(fileId).cstr(), localPath.cstr(),
                             CStr(category).cstr());
      } else {
        r.storeFileFromLocal(CStr(fileId).cstr(), localPath.cstr());
      }


      if (canDebug) cout << "DONE" << endl;
    } else if (cmd == "file-delete") {
      URI uri = URI::fromString(h.word(1));
      Str fileId = h.word(2);
      line(Str("deleting file ").append(fileId).append(
        " from DFS ").append(uri.toString()));
      RemoteFilesystem r(uri, &logger);
      CStr cs = CStr(fileId);
      r.deleteFile(cs.cstr());
    } else if (cmd == "file-delete-all") {
      URI uri = URI::fromString(h.word(1));
      line(Str("deleting all files from DFS ").append(uri.toString()));
      RemoteFilesystem r(uri, &logger);
      r.deleteAllFiles();
    } else if (cmd == "file-delete-all-category") {
      URI uri = URI::fromString(h.word(1));
      Str category = h.word(2);
      line(Str("deleting all files from DFS ").append(
        uri.toString()).append(" of category ").append(category));
      RemoteFilesystem r(uri, &logger);
      int amount = r.deleteAllFilesOfCategory(CStr(category).cstr());
      cout << amount << endl;
    } else if (cmd == "change-setting") {
      line("change a single setting");
      URI uri = URI::fromString(h.word(1));
      RemoteFilesystem r(uri, &logger);
      Str key = h.word(2);
      Str value = h.word(3);
      if (canDebug)
        cout << "settings key is " << key << " with value " << value;
      r.changeSetting(CStr(key).cstr(), CStr(value).cstr());
    } else if (cmd == "datanode-register") {
      line("register new data node");
      URI uri = URI::fromString(h.word(1));
      if (canDebug) cout << "index node is " << uri.toString() << endl;
      RemoteFilesystem r(uri, &logger);
      URI urid = URI::fromString(h.word(2));
      if (canDebug) cout << "data node is " << urid.toString() << endl;
      r.registerDataNode(urid);
      if (canDebug) cout << "DONE" << endl;
    } else if (cmd == "datanode-hash-print") {
      line("load hash from data node");
      URI uri = URI::fromString(h.word(1));
      dfs::comm::EndpointClient ec;
      ec.setLogger(&logger);
      Str r = ec.sendSyncMessage(uri, "hash");
      StrReader reader(&r);
      reader.setPos(4);
      UI64 hash = reader.readUInt64();
      cout << hash << endl;
    } else if (cmd == "datanode-stat") {
      line("load stat of data node (chunksize, size of all chunks)");
      URI uri = URI::fromString(h.word(1));
      dfs::comm::EndpointClient ec;
      ec.setLogger(&logger);
      Str result = ec.sendSyncMessage(uri, "stat");
      StrReader reader(&result);
      reader.setPos(4);

      UI64 used = reader.readUInt64();
      UI64 allocated = reader.readUInt64();

      for (int i = 0; i < used; i++) {
        Str chunkName = reader.readStrSer();
        Str cat = reader.readStrSer();
        cout << chunkName << " " << cat << endl;
      }
      cout << used << endl;
      cout << allocated << endl;

    } else if (cmd == "datanode-stop") {
      line("stops data node");
      URI uri = URI::fromString(h.word(1));
      dfs::comm::EndpointClient ec;
      ec.setLogger(&logger);
      Str result = ec.sendSyncMessage(uri, "quit");
      cout << "datanode " << uri.toString() << " stopped" << endl;
    } else if (cmd == "datanode-list") {
      line("list all uris of working data nodes");
      URI uri = URI::fromString(h.word(1));
      dfs::comm::EndpointClient ec;
      ec.setLogger(&logger);
      Str result = ec.sendSyncMessage(uri, "node");
      StrReader reader(&result);
      reader.setPos(4);
      UI64 count = reader.readUInt64();
      for (UI64 i = 0; i < count; i++) {
        Str uri = reader.readStrSer();
        cout << uri << endl;
      }
      cout << count << endl;
    } else if (cmd == "datanode-register-easy") {
      line("register new data node with host and port");

      URI uri = URI::fromString(h.word(1));
      Str host = h.word(2);
      Str port = h.word(3);
      URI urid = URI::fromString(
        Str("dfs-data://").append(host).append(":").append(port));

      if (canDebug) line(Str("data node url is").append(urid.toString()));
      RemoteFilesystem r(uri, &logger);
      r.registerDataNode(urid);
    } else {
      displayHelp();
    }

    if (doMeasure) {
      double ms = duration.measureMS();
      if (!beSilent) cout << "measured [ms]" << endl;
      cout << ms << endl;
    }
  }
  catch (BaseException &ex) {
    cerr << "FATAL exception BaseException " << ex.what() << endl;
    return 1;
  }
  catch (cliException &ex) {
    cerr << "FATAL exception cliException" << ex.what() << endl;
    return 1;
  }
  catch (exception &ex) {
    cerr << "FATAL exception common exception" << ex.what() << endl;
    return 1;
  }
  catch (...) {
    cerr << "FATAL unknown error" << endl;
    return 1;
  }
  return 0;
}
