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

#include "RemoteCommandBuilder.h"

using namespace dfs;

RemoteCommandBuilder::RemoteCommandBuilder(const Str &verb, bool sizePreamble) {
  if (verb.len() != 4) {
    throw "invalid verb length";
  }

  this->verb = verb;
  this->sizePreamble = sizePreamble;

  if (sizePreamble) {
    cmd = Str("@").append(Str(19).prepend(14, '0')).append(verb);
  } else {
    cmd = verb;
  }
}

void RemoteCommandBuilder::setBody(const Str &body) {
  if (sizePreamble) {
    int len = 19 + body.len();
    cmd = sizeEnvelope(verb.append(body));
  } else {
    cmd = verb.append(body);
  }
}

Str RemoteCommandBuilder::sizeEnvelope(const Str &s) {

  int l = s.len() + 15;
  Str header = Str("@").append(Str(l).prepend(14, '0'));
  header.appendToThis(s);
  return header;
  //return Str("@").append(Str(l).prepend(14, '0')).append(s);
}

