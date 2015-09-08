/*
----
This file is part of SECONDO.

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

*/

#include "KeyValueStoreIPCServer.h"

#include "SecondoSystem.h"
#include "AlgebraManager.h"

// extern AlgebraListEntry& GetAlgebraEntry( const int j );
// extern NestedList* nl;

namespace KVS {

KeyValueStore* kvsInstance;
}

char* getCmdOption(char** begin, char** end, const std::string& option) {
  char** itr = std::find(begin, end, option);
  if (itr != end && ++itr != end) {
    return *itr;
  }
  return 0;
}

bool checkCmdFlag(char** begin, char** end, const std::string& option) {
  return (std::find(begin, end, option) != end);
}

int main(int argc, char* argv[]) {
  int id = 0;
  bool useConsole = false;

  // set application id (to enable more than one instance)
  char* idParam = getCmdOption(argv, argv + argc, "-id");
  if (idParam != 0) {
    stringstream temp;
    temp << idParam;
    temp >> id;
  }

  // enable console output
  if (checkCmdFlag(argv, argv + argc, "-cout")) {
    useConsole = true;
  }

  KVS::KeyValueStoreIPCServer kvsApp(argv[0], id, useConsole);
  KVS::kvsInstance = kvsApp.getKVSInstance();

  if (kvsApp.checkExclusive()) {
    // SecondoSystem::CreateInstance( &GetAlgebraEntry );
    // nl = new NestedList();//SecondoSystem::GetNestedList();

    return kvsApp.run();
    ;
  } else {
    cout << "Application already running.\n";
    return -1;
  }
}
