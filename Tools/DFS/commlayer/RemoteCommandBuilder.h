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

#ifndef REMOTECOMMANDBUILDER_H
#define REMOTECOMMANDBUILDER_H

#include "../shared/str.h"

namespace dfs {

  /**
   * each raw socket request is put in to envelope
   * to help socket reader to know how many bytes are com
   */
  class RemoteCommandBuilder {
  private:
    bool sizePreamble;
    Str verb;
  public:
    Str cmd;

    /**
     * puts envelope around raw request
     * @param s
     * @return
     */
    static Str sizeEnvelope(const Str &s);

    RemoteCommandBuilder(const Str &verb, bool sizePreamble);

    void setBody(const Str &body);
  };

}

#endif /* REMOTECOMMANDBUILDER_H */

