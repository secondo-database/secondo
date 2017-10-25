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

[10] Definition of Class Peers

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 About this Class

This class allows communication with a group of secondo instances, called
peers. It uses "ConnectionSession" to manage the communication to each peer and
to handle rollback scenarios.

*/
#ifndef ALGEBRAS_DISTRIBUTED4_PEERS_H
#define ALGEBRAS_DISTRIBUTED4_PEERS_H

#include "ConnectionSession.h"
#include <vector>
#include <functional>

namespace distributed4 {
  class Peers {
    protected:
/*
2 Member Variables

*/
      std::vector<std::unique_ptr<ConnectionSession>> sessions;
/*
"sessions"[1] contains a "ConnectionSession"[1] object for every peer to be
synchronized.

3 Member Functions

*/
    public:
      Peers(const Relation*);
      void exec(const std::function<void(const
            std::unique_ptr<ConnectionSession>&)>&) const;
  };
}

#endif
