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

#include "../qunit/qunit.h"
#include "../dfs/dfs.h"
#include "../shared/log.h"
#include "../dfshdfs/dfshdfs.h"
#include <hdfs.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

using namespace qunit;

class FullTestCase : public TestCase {
public:
  virtual void run() {
    dfs::log::DefaultOutLogger logger;
    logger.canDebug = true;

    logger.debug("cli test client for dfs hdfs bridge");

    //setup hdfs
    dfs::hdfs::HDFS hdfs;
    hdfs.setLogger(&logger);

    //store and delete
    hdfs.deleteFile("testfile");
    hdfs.storeFile("testfile", "1234567890", 10);
    hdfs.deleteFile("testfile");

    //store and receive
    hdfs.storeFile("testfile", "1234567890", 10);
    char tmp[3];
    tmp[2] = 0;
    hdfs.receiveFilePartially("testfile", 2, 2, tmp, 0);
    cout << "got from HDFS: " << tmp << endl;
    aeqcs("getting 2 Bytes", "34", tmp);

    //append test
    hdfs.appendToFile("testfile", "XX", 2);
    hdfs.receiveFilePartially("testfile", 10, 2, tmp, 0);
    cout << "got from HDFS: " << tmp << endl;
    aeqcs("getting 2 Bytes after append", "XX", tmp);

    cout << "ALL TESTS SUCCESSFUL" << endl;

  }
};

int main(int argc, char *argv[]) {
  FullTestCase().run();
  return 0;
}
