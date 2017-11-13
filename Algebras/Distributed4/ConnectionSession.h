/*
----
This file is part of SECONDO.

Copyright (C) 2017, Faculty of Mathematics and Computer Science, Database
Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

SECONDO is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
SECONDO; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [10] title: [{\Large \bf] [}]
//characters [1] tt: [\texttt{] [}]
//[secondo] [{\sc Secondo}]

[10] Definition of Class ConnectionSession

2017-10-29: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 About this Class

This class simplifies communication with other secondo instances. It uses the
usual mechanisms from Distributed2Algebra, but provides a higher-level
interface tailored to the needs of "Distributed4Algebra"[1].

*NOTE:* As a special feature designed for robust exception handling, the
destructor will attempt to execute all commands remaining in "rollback"[1].
This happens when the object goes out of scope or delete is called on a pointer
to the object. Therefore it is necessary to call "clearRollback()"[1] as soon
as a string of commands has completed and should be kept no matter what happens
next. This is especially critical to remember before returning normally from a
function (or any other scope).

*/
#ifndef ALGEBRAS_DISTRIBUTED4_CONNECTIONSESSION_H
#define ALGEBRAS_DISTRIBUTED4_CONNECTIONSESSION_H

#include "../Distributed2/ConnectionInfo.h"
#include <vector>

namespace distributed4 {
  class ConnectionSession {
    protected:
/*
2 Member Variables

*/
      distributed2::ConnectionInfo* ci;
      std::vector<std::string> rollback;
      bool deleteci{true};
/*
"ci"[1] points to a "ConnectionInfo"[1] object that can be used to communicate
with another secondo instance. "rollback"[1] holds the complete list of
commands needed to undo the commands performed so far. "deleteci"[1] tracks
whether the object is responsible for calling delete on "ci"[1] (or by
implication the caller).

3 Member Functions

*/
    public:
      ConnectionSession(distributed2::ConnectionInfo*, bool = false);
      ConnectionSession(const std::string&, int, const std::string&);
      ~ConnectionSession();

      std::string getHost() const;
      int getPort() const;
      std::string getConfig() const;

      NList run(const std::string&, const std::string& = "");
      void clearRollback();
      void switchDatabase(const std::string& =
          SecondoSystem::GetInstance()->GetDatabaseName());
      void beginTransaction();
      void commitTransaction();

      void updateObject(const std::string&, const Address);
      void letObject(const std::string&, const Address = nullptr, NList =
          NList{});
      void deleteObject(const std::string&);
      void lockObject(const std::string&, bool);
      void unlockObject(const std::string&);

      NList queryAtom(std::string);
      std::string querySymbol(const std::string&);
      std::string queryString(const std::string&);
      std::string queryText(const std::string&);
      int queryInt(const std::string&);
      double queryReal(const std::string&);
      bool queryBool(const std::string&);
      double queryNumeric(const std::string&);
      std::string queryTextual(const std::string&);
  };
}

#endif
