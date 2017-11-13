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

[10] Implementation of Class ManagedMutex

2017-10-24: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 Preliminary Setup

*/
#include "ManagedMutex.h"
#include "SecondoSystem.h"

namespace distributed4 {
  using boost::filesystem::directory_iterator;
  using boost::filesystem::filesystem_error;
  using boost::filesystem::path;
  using boost::filesystem::read_symlink;
  using boost::interprocess::named_sharable_mutex;
  using boost::interprocess::open_only;
  using boost::interprocess::open_or_create;
  using std::all_of;
  using std::runtime_error;
  using std::stoi;
  using std::string;

  const directory_iterator dit_end;
  const path map_files{"/proc/self/map_files"};
/*
2 Constructors

2.1 "ManagedMutex"[1] (locking constructor)

This constructor takes the "name"[1] of an object and whether it is to be
locked for "exclusive"[1] access, or not as parameters. An additional optional
argument specifies if the function is to "wait"[1] until a lock can be acquired
or abort if no lock can be acquired immediately. The default is to wait ("wait
== true"[1]). That passed "name"[1] is used as a basis for the name of the
underlying named sharable mutex and its shared memory (SHM) segment.

*/
  ManagedMutex::ManagedMutex(const string& name, bool exclusive, bool wait):
    shm_target{getSHMPath(name)} {
/*
Make sure "name"[1] refers to a real database object.

*/
    if(!SecondoSystem::GetCatalog()->IsObjectName(name))
      throw runtime_error{"The object \"" + name + "\" couldn't be locked "
        "because it doesn't exist."};
/*
A race condition exists where it is possible for this process to map the SHM
segment and another process to remove that SHM segment. This happens when the
other process checked for mapped references before the SHM segment was mapped
in this process and subsequently removed it. This race condition is mitigated
here. After mapping the shared memory segment, the mutex is locked and a check
is performed to see if the mapping is still valid. If it isn't, the race
condition happened, and the mutex is re-mapped. This is repeated as often as
necessary for the mapping to remain valid after the mutex is locked.

*/
    const string shm_name{shm_target.filename().c_str()};
    directory_iterator it;
    while(it == dit_end) {
      if(mutex)
        unlock();
      mutex.reset(new named_sharable_mutex{open_or_create, shm_name.c_str(),
          0600});
      lock(exclusive, wait);
      for(it = directory_iterator{map_files}; it != dit_end; ++it)
        if(read_symlink(*it) == shm_target)
          break;
    }
  }
/*
2.2 "ManagedMutex"[1] (takeover constructor)

This constructor takes the name of an object as a parameter. It will access a
named sharable mutex that *must* already be locked and assume responsibility
for it (take over ownership). If the mutex is found not to be locked, it will
abort.

*/
  ManagedMutex::ManagedMutex(const string& name): shm_target{getSHMPath(name)}
  {
/*
Make sure "name"[1] refers to a real database object.

*/
    if(!SecondoSystem::GetCatalog()->IsObjectName(name))
      throw runtime_error{"The lock for the object \"" + name + "\" couldn't "
        "be accessed because the object doesn't exist."};
/*
Map the corresponding "named\_sharable\_mutex"[1], throwing an
"interprocess\_exception"[1] if it doesn't exist.

*/
    mutex.reset(new named_sharable_mutex{open_only,
        shm_target.filename().c_str()});
/*
Make sure that the mutex is in fact already locked.

*/
    if(mutex->try_lock()) {
      mutex->unlock();
      throw runtime_error{"The mutex at \"" + string{shm_target.c_str()} +
        "\" couldn't be taken over because it isn't locked."};
    }
/*
Figure out if it is locked for exclusive or sharable ownership and claim it.

*/
    if(mutex->try_lock_sharable()) {
      mutex->unlock_sharable();
      exclusive = false;
    } else {
      exclusive = true;
    }
    owned = true;
  }
/*
3 Destructor

*/
  ManagedMutex::~ManagedMutex() {
    if(autounlock && owned)
      unlock();
/*
Checking and removing the shared memory segment requires locking the mutex to
avoid most race conditions. If a lock can't be acquired, the mutex is being
used and may not be removed. In that case, no further checking is necessary and
the underlying shared memory segment will not be removed.

*/
    if(!mutex->try_lock())
      return;
/*
Having acquired a lock on the mutex, all running processes of the process uid
are checked to see if the shared memory segment is mapped to any of them. The
processes checked are limited by the kernel's permissions on
"/proc/.../map\_files"[1].

*/
    directory_iterator mit;  // map_files iterator
    for(directory_iterator it{"/proc"}; it != dit_end; ++it) {
      path map_files{*it};
      string filename{map_files.filename().native()};
      if(!all_of(filename.begin(), filename.end(), isdigit))
        continue;
      if(stoi(filename) == getpid())
        continue;
      map_files += "/map_files";
      try {
        for(mit = directory_iterator{map_files}; mit != dit_end; ++mit)
          if(read_symlink(*mit) == shm_target)
            break;
      } catch(const filesystem_error&) {}
      if(mit != dit_end)
        break;
    }
/*
The value of "mit"[1] tells whether a mapped reference was found, or not. When
"mit"[1] is equal to "dit\_end"[1], all directories were searched and the
iterator reached the end of the directory. In that case, no mapped reference
was found. When "mit"[1] has some other value, it points to the directory
entry of a mapped reference, thus allowing us to conclude that such a mapped
reference exists. In the absence of a mapped reference, the shared memory
segment for the mutex is removed.

*/
    if(mit == dit_end)
      unlink(shm_target.c_str());
    mutex->unlock();
  }
/*
4 Member Functions

4.1 "noautounlock"[1]

By default, the "ManagedMutex"[1] will be automatically released when the mutex
object is destroyed. This helps to avoid stuck locks after exceptions. As this
is not always the desired behavior, it can be prevented by calling this
function.

*/
  void ManagedMutex::noautounlock() {
    autounlock = false;
  }
/*
4.2 "lock"[1]

Unlike other mutex implementations that offer ~seperate member functions~ for
exclusive and sharable locking, "lock"[1] takes an ~argument~ indicating the
desired lock type: "true"[1] for an exclusive lock or "false"[1] for a sharable
lock. An additional optional argument specifies if the function is to "wait"[1]
until a lock can be acquired or abort if no lock can be acquired immediately.
The default is to wait ("wait == true"[1]). If a process tries to lock a mutex
more than once, a "runtime\_error"[1] exception is thrown.

*/
  void ManagedMutex::lock(bool exclusive, bool wait) {
    if(owned)
      throw runtime_error{"Attempted to lock a mutex that is already owned."};

    if(!(exclusive ? mutex->try_lock() : mutex->try_lock_sharable())) {
      const string err{"The mutex at " + shm_target.string() +
        " is already locked."};
      if(!wait)
        throw runtime_error{err};
      cmsg.info() << err << " Waiting for " << (exclusive ? "exclusive" :
          "sharable") << " ownership." << endl;
      cmsg.send();
      exclusive ? mutex->lock() : mutex->lock_sharable();
    }

    this->exclusive = exclusive;
    owned = true;
  }
/*
4.3 "unlock"[1]

The state of the lock is tracked. Therefore, it is not necessary to distinguish
between unlocking a sharable lock and an exclusive one when calling
"unlock()"[1]. There is just this one member function to unlock both lock
types. If a process tries to unlock a mutex that is not locked, a
"runtime\_error"[1] exception is thrown.

*/
  void ManagedMutex::unlock() {
    if(!owned)
      throw runtime_error{"Attempted to unlock an unowned mutex."};
    exclusive ? mutex->unlock() : mutex->unlock_sharable();
    owned = false;
  }
/*
4.4 "getSHMPath"[1]

This member function is for internal use only. Its primary purpose is to allow
assigning a value to shm\_target in the initializer list of the constructor.

*/
  string ManagedMutex::getSHMPath(const string& name) {
    string dbname{SecondoSystem::GetInstance()->GetDatabaseName()};
    string dbdir{SmiEnvironment::GetSecondoHome()};
    if(dbdir[0] == '/')
      dbdir.erase(0, 1);
    replace(dbdir.begin(), dbdir.end(), '/', '_');
    return "/dev/shm/secondo:" + dbdir + "_" + dbname + ":" + name;
  }
}
