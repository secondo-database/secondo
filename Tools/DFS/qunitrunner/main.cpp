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

#include "../dfs/dfs.h"
#include "../dfs/remotedfs.h"
#include "../shared/str.h"
#include "../shared/log.h"
#include "../shared/io.h"
#include "../shared/uri.h"
#include "../shared/checksum.h"
#include "../shared/FigureSystem.h"
#include "../commlayer/RemoteCommandBuilder.h"

#include "../qunit/qunit.h"
#include "../shared/maschine.h"
#include "../shared/numberUtils.h"

#include <vector>

using namespace std;
using namespace dfs;
using namespace qunit;

log::Logger *logger;

class FirstCase : public TestCase {
public:
  virtual void run() {
    at("testinit", true);
  }
};

class StrCase : public TestCase {
public:

  virtual void run() {

    Str sFullName = "Theodor Test";

    //str tests
    Str s("Hallo");
    s = s.append(" mein Freund");
    s = s.append(", wie gehts?");
    s = s.prepend("Achtung: ");
    aeq("len", 38, s.len());

    char *cs = s.cstr();
    aeqcs("cstr", "Achtung: Hallo mein Freund, wie gehts?", cs);
    delete[] cs;

    Str greeting = Str("Willkommen ").append("sehr geehrter Herr ").append(
      sFullName);
    aq("Str.append", Str("Willkommen sehr geehrter Herr Theodor Test"),
       greeting);

    aq("find hodor", -1, s.find("Hodor"));
    at("find Freund", s.find("Freund") > -1);

    at("substring1", s.substr(3, 2) == "tu");
    //Achtung: Hallo mein Freund, wie gehts?
    at("substring2", s.substr(3) == "tung: Hallo mein Freund, wie gehts?");

    Str i(15);
    aq("Str.int", Str("15"), i);

    aq("Str.long", Str("1099511627776"), Str(1024 * 1024 * 1024 * 1024l));

    aeq<UI64>("Str.uint64", Str("000123456789123456789").toUInt64(),
              123456789123456789);
    s = Str(123456789123456789ul);
    aq(Str("123456789123456789"), s);

    s = Str("Hallo");
    Str ss = s.serialize();
    aq("Str.serialize", Str("5_Hallo"), ss);

    s = Str("HalloNochEinTest_");
    ss = s.serialize();
    aq("Str.serialize2", Str("17_HalloNochEinTest_"), ss);

    s = Str::deserialize("5_Hallo");
    aq("Str.deserialize", Str("Hallo"), s);

    s = Str::deserialize("5_HalloHans");
    aq("Str.deserializeShorter", Str("Hallo"), s);

    s = Str("15");
    aq("Str.prepend1", Str("1715"), s.prepend("17"));

    aq("Str.prepend2", Str("0015"), s.prepend(4, '0'));

    s = Str("/a/b/c");
    aq("Str.findLast", 4, s.findLastChar('/'));

    s = Str("Secondo");
    aq("Str.findLast1", 6, s.findLastChar('o'));
    aq("Str.findLast2", -1, s.findLastChar('x'));

    s = Str("00sf002db0000000003xyz5_Hallo3454_Test3_Ich15__DasIstEin_Test");
    StrReader r = StrReader(&s);
    aq("StrReader.1", 0, r.getPos());
    r.setPos(4);
    aq("StrReader.2", 4, r.getPos());
    aq("StrReader.3", 2, r.readInt(3));
    aq("StrReader.4", Str("db"), r.readStr(2));
    aq("StrReader.3", 3, r.readInt(10));
    aq("StrReader.4", Str("xyz"), r.readStr(3));
    aq("StrReader.5", Str("Hallo"), r.readStrSer());
    aq("StrReader.6", 345, r.readInt(3));
    aq("StrReader.7", Str("Test"), r.readStrSer());
    aq("StrReader.8", Str("Ich"), r.readStrSer());
    aq("StrReader.9", Str("_DasIstEin_Test"), r.readStrSer());

    aq("Str+", Str("abc"), Str("a") + Str("bc"));

    ToStrSerializer ser;
    ser.append(Str("Hallo"));
    ser.append(12, 5);
    aq("ToStrSerializer.full", Str("5_Hallo00012"), ser.output);

    ToStrSerializer ser2;
    UI64 ull = 123456789123456789;
    ser2.appendUInt64(ull);
    aq("ToStrSerializer.uint64", Str("000123456789123456789"), ser2.output);

    StrBuilder sb(1024, 1024);
    aq("StrBuilder.size1", 1024, sb.currentSize());
    sb.appendCStr("Hallo");
    sb.append(" Tester");
    aq("StrBuilder.1", 12, sb.currentPos());
    aq("StrBuilder.size2", 1024, sb.currentSize());
    sb.append("!");
    aq("StrBuilder.2", 13, sb.currentPos());
    aq("StrBuilder.size3", 1024, sb.currentSize());

    StrBuilder sb2(5, 5);
    aq("StrBuilder.3", 0, sb2.currentPos());
    aq("StrBuilder.sb2.size1", 5, sb2.currentSize());
    sb2.append("0123456789");
    aq("StrBuilder.sb2.size1", 10, sb2.currentSize());
    aq("StrBuilder.3", 10, sb2.currentPos());

    aq("Str.toHex.1", Str("41424344"), Str::toHex("ABCD", 4));
  }

};

class IoCase : public TestCase {
public:
  virtual void run() {
  }
};

class RemoteFilesystemCase : public TestCase {
public:
  virtual void run() {
    try {
      dfs::remote::RemoteFilesystem rfs(
        URI::fromString("dfs-index://localhost"), logger);

      Str r = rfs.echo("echo1");
      aq("rfs.echo1", Str("echo1"), r);

      r = rfs.echo("echo2");
      aq("rfs.echo1", Str("echo2"), r);

      r = rfs.echo("echo3");
      aq("rfs.echo1", Str("echo3"), r);

      r = rfs.echo("echo4");
      aq("rfs.echo4", Str("echo4"), r);

      rfs.deleteAllFiles();
      aq("rfs.count after deletion", 0, rfs.countFiles());

      rfs.storeFile("testdateien/stammdaten.1", "Stephan,Scheide", 15);
      aq("rfs1", rfs.countFiles(), 1);

      rfs.storeFile("testdateien/stammdaten.2", "Stephan,Scheide", 15);
      aq("rfs2", 2, rfs.countFiles());

      rfs.storeFile("testdateien/stammdaten.1", "Stephan,Scheide", 15);
      aq("rfs21", 2, rfs.countFiles());

      rfs.deleteFile("testdateien/stammdaten.2");
      aq("rfs3", 1, rfs.countFiles());

      rfs.deleteFile("testdateien/stammdaten.1");
      aq("rfs4", 0, rfs.countFiles());

      msg("RemoteFilesystemCase durchlaufen");
    } catch (BaseException &e) {
      cout << "EXCEPTION: " << e.what() << endl;
      throw e;
    }
  }
};

class URICase : public TestCase {
public:
  virtual void run() {
    URI u = URI::fromString("dfs-index://1.2.3.4:5555");
    af("URI.isData", u.isData());
    at("URI.isIndex", u.isIndex());
    aq("URI.host", Str("1.2.3.4"), u.host);
    aq("URI.port", Str("5555"), u.port);

    u = URI::fromString("dfs-data://12.12.13.14");
    at("URI.isData", u.isData());
    aq("URI.host", Str("12.12.13.14"), u.host);
  }
};

class ArithmeticTests : public TestCase {
public:
  virtual void run() {

    dfs::checksum::crc64 c;
    int i = 0x12121212;
    //cout << c.checksum((uint8_t*)&i,4) << endl;

    FigureSystem fs(10, 10);
    aq("fs.toStr1", Str("0000000000"), fs.toStr());

    fs.inc();
    aq("fs.toStr2", Str("0000000001"), fs.toStr());

    fs.inc(22);
    aq("fs.toStr3", Str("0000000023"), fs.toStr());

    fs.inc();
    aq("fs.toStr4", Str("0000000024"), fs.toStr());

    fs.fromStr("0000000003");
    fs.inc();
    aq("fs.toStr5", Str("0000000004"), fs.toStr());

    fs.fromStr("0000000003");
    fs.inc();
    aq("fs.toStr6", Str("0000000004"), fs.toStr());

    fs.fromStr("1234567899");
    fs.inc();
    aq("fs.toStr7", Str("1234567900"), fs.toStr());

    FigureSystem fsm(36, 5);
    aq("fsm.toStr1", Str("00000"), fsm.toStr());

    fsm.inc(10);
    aq("fsm.toStr2", Str("0000a"), fsm.toStr());

    fsm.inc(25);
    aq("fsm.toStr3", Str("0000z"), fsm.toStr());

    fsm.inc(2);
    aq("fsm.toStr4", Str("00011"), fsm.toStr());

  }
};

class RemoteCommandBuilderCase : public TestCase {
public:
  virtual void run() {

    RemoteCommandBuilder builder("test", false);
    aq("RemoteCommandBuilder.1", Str("test"), builder.cmd);

    builder.setBody("121212");
    aq("RemoteCommandBuilder.2", Str("test121212"), builder.cmd);

    builder = RemoteCommandBuilder("echo", true);
    aq("RemoteCommandBuilder.3", Str("@00000000000019echo"), builder.cmd);

    builder.setBody("DasIstEinTest");
    aq("RemoteCommandBuilder.4", Str("@00000000000032echoDasIstEinTest"),
       builder.cmd);

    aq("RemoteCommandBuilder.sizeEnvelope", Str("@00000000000019echo"),
       RemoteCommandBuilder::sizeEnvelope("echo"));
  }
};

class UtilCase : public TestCase {
public:
  virtual void run() {
    aq("maschine:volatileId", Str("0ad1c1daa8baedd9Test"),
       dfs::Maschine::volatileId("Test"));
  }
};

class NumberCase : public TestCase {
public:
  virtual void run() {

    for (int i = 0; i < 100; i++) {
      int *list = numberUtils::findUniqueRandomInts(0, 2, 3);
      for (int i = 0; i < 3; i++) {
        if (list[0] == 0)
          at("numberUtils.findUniqueRandomInts.0",
             list[0] == 0 && list[1] != 0 &&
             list[2] != list[1]);
        else if (list[0] == 1)
          at("numberUtils.findUniqueRandomInts.1",
             list[0] == 1 && list[1] != 1 &&
             list[2] != list[1]);
        else if (list[0] == 2)
          at("numberUtils.findUniqueRandomInts.2",
             list[0] == 2 && list[1] != 2 &&
             list[2] != list[1]);
        else at("invalid number", false);
      }

      delete[] list;
    }

    at("numberUtils.hasIntersect1", numberUtils::hasIntersect(0, 9, 5, 13));
    at("numberUtils.hasIntersect2", numberUtils::hasIntersect(0, 9, 9, 13));
    af("numberUtils.hasIntersect3", numberUtils::hasIntersect(0, 9, 10, 13));
    at("numberUtils.hasIntersect4", numberUtils::hasIntersect(5, 13, 9, 13));

  }
};


int main() {

  try {
    logger = new log::DefaultOutLogger();

    FirstCase t0;
    t0.run();

    StrCase().run();


    IoCase t2;
    t2.run();

    URICase t4;
    t4.run();

    ArithmeticTests t41;
    t41.run();

    NumberCase().run();

    RemoteCommandBuilderCase().run();

    delete logger;
    msg("SUCCESS TESTER");
  }
  catch (...) {
    cout << "got exception" << endl;
    return 1;
  }

  return 0;
}
