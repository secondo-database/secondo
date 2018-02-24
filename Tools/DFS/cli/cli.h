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

#ifndef CLI_H
#define CLI_H

#include <iostream>
#include <time.h>
#include "../shared/datetime.h"
#include "../shared/str.h"
#include "../shared/numberUtils.h"

using namespace std;
using namespace dfs;

#define BILLION 1E9

class Duration {
private:
  timespec c;
public:

  void start() {
    clock_gettime(CLOCK_REALTIME, &c);
  }

  double measureMS() {
    timespec stopped;
    clock_gettime(CLOCK_REALTIME, &stopped);
    return (stopped.tv_sec - c.tv_sec) + (stopped.tv_nsec - c.tv_nsec)
                                         / BILLION;
  }

};

struct cliException : public std::exception {
private:
  char *msg;
public:
  cliException(const Str &s) {
    this->msg = s.cstr();
  }

  ~cliException() throw() {
    delete[] msg;
  }

  virtual const char *what() { return this->msg; }
};

void die(const Str &s) {
  throw cliException(s);
}

void line(const Str &s) {
  cout << s << endl;
}

void line() {
  cout << endl;
}

void debug(const Str &s) {
  cout << Str("DEBUG cli ").append(s) << endl;
}

void debugc(const Str &cmd, const Str &s) {
  cout << Str("DEBUG cli ").append(cmd).append(" ").append(s) << endl;
}

int randInt(int min, int max) {
  numberUtils::randInt(max, min);
}

#endif /* CLI_H */

