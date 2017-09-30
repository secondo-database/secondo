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

[10] Implementation of Utility mutexset

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 Preliminary Setup

*/
#include <iostream>
#include <map>
#include <boost/interprocess/sync/named_sharable_mutex.hpp>

using boost::interprocess::interprocess_exception;
using boost::interprocess::named_sharable_mutex;
using boost::interprocess::open_only;
using std::cerr;
using std::cout;
using std::endl;
using std::map;
using std::string;

typedef bool (*func_t)(named_sharable_mutex*);
/*
2 Primary Functions

2.1 "lock"[1]

Lock the passed mutex for exclusive ownership.

*/
bool lock(named_sharable_mutex* mutex) {
  return mutex->try_lock();
}
/*
2.2 "lock\_sharable"[1]

Lock the passed mutex for sharable ownership.

*/
bool lock_sharable(named_sharable_mutex* mutex) {
  return mutex->try_lock_sharable();
}
/*
2.3 "unlock"[1]

Unlock the passed mutex. Whether an exclusive or a sharable lock needs to be
released is determined automatically.

*/
bool unlock(named_sharable_mutex* mutex) {
  if(mutex->try_lock()) {
    mutex->unlock();
    return false;
  }

  if(mutex->try_lock_sharable()) {
    cout << "The mutex seems to have previously been locked for sharable "
      "ownership." << endl;
    mutex->unlock_sharable();
    mutex->unlock_sharable();
  } else {
    cout << "The mutex seems to have previously been locked for exclusive "
      "ownership." << endl;
    mutex->unlock();
  }

  return true;
}
/*
2.4 "getstate"[1]

Inspect the passed mutex. Report whether it is locked for exclusive or sharable
ownership or is unlocked.

*/
bool getstate(named_sharable_mutex* mutex) {
  cout << "The mutex seems to be in the following state:" << endl << endl;
  if(mutex->try_lock()) {
    mutex->unlock();
    cout << "unlocked" << endl;
  } else if(mutex->try_lock_sharable()) {
    mutex->unlock_sharable();
    cout << "sharable" << endl;
  } else {
    cout << "exclusive" << endl;
  }
  cout << endl;
  return true;
}
/*
3 Constants

These constants depend on the primary functions defined above. The helper and
main functions below depend on them. That is why they are defined in between.

*/
const string shmpath_prefix{"/dev/shm/secondo:"};
const map<string,func_t> actions{{"exclusive", lock}, {"sharable",
  lock_sharable}, {"unlock", unlock}, {"getstate", getstate}};
/*
4 Helper Function

Print an error message with usage information. This is called when a syntax
error is encountered.

*/
int die(const string& msg) {
  cerr << "ERROR: " << msg << endl;
  cerr << endl;
  cerr << "Usage: mutexset <shmpath> {";
  for(auto it{actions.begin()}; it != actions.end(); ++it) {
    if(it != actions.begin())
      cerr << ",";
    cerr << it->first;
  }
  cerr << "}" << endl;
  cerr << endl;
  cerr << "<shmpath> must start with \"" << shmpath_prefix << "\". It must "
    "refer to an already existing named_sharable_mutex." << endl;
  cerr << endl;
  cerr << "The action arguments may be shortened as desired down to a single "
    "character (i.e.: mutexset /dev/shm/secondo:foo u)." << endl;
  return 1;
}
/*
3 Main Function

Check the passed arguments, determine which function to call, get a reference
to the "na\-med\_\-sha\-ra\-ble\_\-mu\-tex"[1], and do what was requested. A
message is printed to stdout on success, to stderr on failure.

*/
int main(int argc, const char* argv[]) {
/*
Extract the arguments.

*/
  if(argc != 3)
    return die("wrong number of arguments");

  string shmpath{argv[1]};
  string action{argv[2]};
/*
Interpret the first argument ("shmpath"[1]).

*/
  if(shmpath.compare(0, shmpath_prefix.size(), shmpath_prefix) != 0)
    return die("the beginning of <shmpath> is incorrect");

  const string shmname{shmpath.substr(shmpath.find_last_of('/') + 1)};
  named_sharable_mutex* mutex;
  try {
    mutex = new named_sharable_mutex{open_only, shmname.c_str()};
  } catch(interprocess_exception& e) {
    return die(e.what());
  }
/*
Interpret the second argument ("action"[1]).

*/
  auto n{action.size()};
  if(n == 0)
    return die("empty action value");

  auto it{actions.lower_bound(action)};
  if(it->first.compare(0, n, action) != 0)
    return die("the action is invalid.");
/*
Call the function determined from "action"[1] on the mutex referenced by
"shmpath"[1].

*/
  if(it->second(mutex)) {
    cout << "The action \"" << it->first << "\" was successfully performed on "
      << shmpath << "." << endl;
    return 0;
  } else {
    cerr << "The action \"" << it->first << "\" failed on " << shmpath << "."
      << endl;
    return 2;
  }
}
