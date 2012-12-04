/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

2012, May Simone Jandt

*/

#ifndef JSIDE_H
#define JSIDE_H

namespace jnetwork {

/*
1. ~Side~

In a network positions may be reachable only from one side of the route or from
both sides of the route. Therefore we introduce the ~enum JSide~ with values
up, down and both as flag for the direction of the movement, respectively the
side a network location is reachable from, resp. the side a section is usable.

*/

enum JSide {Up, Down, Both};

} // end of namespace jnetwork

#endif // JSIDE_H
