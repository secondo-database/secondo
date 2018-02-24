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

#include "ParameterHelper.h"
#include <cstring>

using namespace dfs;

ParameterHelper::ParameterHelper(int argc, char **argv) {
  this->argc = argc;
  this->argv = argv;
}

bool ParameterHelper::hasParameter(const Str &name) {
  for (int i = 1; i < argc; i++) {
    Str arg = Str(argv[i], strlen(argv[i]));
    Str needle = Str("-").append(name);
    if (arg.find(needle) == 0) {
      return true;
    }
  }
  return false;
}

bool ParameterHelper::hasCommand(const Str &name) {
  for (int i = 1; i < argc; i++) {
    Str arg = Str(argv[i], strlen(argv[i]));
    if (name == arg) return true;
  }
  return false;
}


Str ParameterHelper::getParameter(const Str &name) {
  for (int i = 1; i < argc; i++) {
    Str arg = Str(argv[i], strlen(argv[i]));
    Str needle = Str("-").append(name);
    if (arg.find(needle) == 0) {
      return arg.substr(name.len() + 1);
    }
  }
  return Str("");
}

int ParameterHelper::getParameterInt(const Str &name) {
  Str s = this->getParameter(name);
  if (s.len() > 0) {
    return s.toInt(); //FIXME inhaltspruefung
  }
  return -1;
}


Str ParameterHelper::word(short i) {
  return Str(argv[i + 1]);
}

bool ParameterHelper::hasNumberOfArguments(short i) {
  //cmd a b c
  //c ~ 2
  //argc = 4
  return argc - 1 >= i;
}
