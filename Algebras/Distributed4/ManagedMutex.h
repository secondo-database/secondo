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

[10] Definition of Class ManagedMutex

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 About this Class

This class uses "named\_sharable\_mutex"[1] from the "boost"[1] library and
adds the property that the corresponding shared memory segment is removed when
it is no longer used. The implementation is specific to Linux and requires
careful checks in both the constructor and the destructor.

*/
#ifndef ALGEBRAS_DISTRIBUTED4_MANAGEDMUTEX_H
#define ALGEBRAS_DISTRIBUTED4_MANAGEDMUTEX_H

#include <boost/interprocess/sync/named_sharable_mutex.hpp>
#include <boost/filesystem.hpp>

namespace distributed4 {
  class ManagedMutex {
    protected:
/*
2 Member Variables

*/
      const boost::filesystem::path shm_target;
      boost::interprocess::named_sharable_mutex* mutex{nullptr};
      bool owned{false};
      bool exclusive;
/*
"shm\_target"[1] is initialized with the path to the shared memory segment in
the file system. It is used when checking what processes have mapped the shared
memory segment when creating and removing the shared memory segment.
"mutex"[1] is a pointer to the mutex that provides the actual locking.
"owned"[1] tracks whether the current instance owns (holds a lock on) the
"mutex"[1]. And "exclusive"[1] tracks the type of lock held.

3 Member Functions

*/
    public:
      ManagedMutex(const std::string&);
      ManagedMutex(const std::string&, bool);
      ~ManagedMutex();
      void lock(bool);
      void unlock();
      static void unlock(const std::string&);

    protected:
      static std::string getSHMPath(const std::string&);
  };
}

#endif
