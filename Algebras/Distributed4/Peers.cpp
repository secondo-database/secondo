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

[10] Implementation of Class Peers

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 Preliminary Setup

*/
#include "Peers.h"

namespace distributed4 {
  using std::for_each;
  using std::function;
  using std::string;
  using std::unique_ptr;
/*
2 Constructor

The parameter is a pointer to a "Relation"[1] containing "Tuple"[1]s describing
peers. The relation is parsed and sessions initiated and placed in
"sessions"[1].

*/
  Peers::Peers(const Relation* rel) {
    sessions.reserve(rel->GetNoTuples());
    auto rit{rel->MakeScan()};
    for(Tuple* t{rit->GetNextTuple()}; !rit->EndOfScan(); t =
        rit->GetNextTuple()) {
      string h{static_cast<CcString*>(t->GetAttribute(0))->GetValue()};
      int p{static_cast<CcInt*>(t->GetAttribute(1))->GetValue()};
      string c{static_cast<CcString*>(t->GetAttribute(2))->GetValue()};
      t->DeleteIfAllowed();
      sessions.emplace_back(new ConnectionSession{h, p, c});
    }
  }
/*
3 Lone Member Function "exec"[1]

Pass each peer in "sessions"[1] to "f"[1] for handling. "f"[1] is usually a
lambda calling one of the member functions of "ConnectionSession"[1]. It is
completely free to do anything else, though, too. The only constraint is that
it must take an element from "sessions"[1] as its lone argument.

*/
  void Peers::exec(const function<void(const unique_ptr<ConnectionSession>&)>&
      f) const {
    for_each(sessions.begin(), sessions.end(), f);
  };
}
