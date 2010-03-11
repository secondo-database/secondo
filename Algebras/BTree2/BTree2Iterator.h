/*
----
This file is part of SECONDO.

Copyright (C) 2004-2010, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Fre PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

1 Template Header File: BTree2Node

Jan. 2010 M.Klocke, O.Feuer, K.Teufel

1.1 Overview

This class is used for iterating linearly along the leaf nodes.

*/

#ifndef BTREE2_ITERATOR_H
#define BTREE2_ITERATOR_H

#include "Attribute.h"
#include "BTree2.h"
#include "BTree2Types.h"

namespace BTree2Algebra {
/*
2 The BTree2 class

*/

class BTree2Iterator {
  public:
  BTree2Iterator();
/*
Standard constructor.

*/
  BTree2Iterator(BTree2*, NodeId, int, Attribute* = 0, Attribute* = 0);
/*
Constructor. Set iterator state.

*/

  ~BTree2Iterator();
/*
Destructor. Calls important clean up methods (like DeleteIfAllowed)

*/

  Attribute* operator*();
/*
Get the current value as an Attribute object. If the iterator points
to end(), NULL is returned.

*/
  Attribute* key();
/*
Get the current value as an Attribute object. If the iterator points
to end(), NULL is returned.

*/
  BTree2Iterator& operator++();
/*
Advance the iterator to the next entry.
If this goes beyond the last entry on the rightmost leaf node, the iterator is
set to end().

*/
  bool next();
/*
Same as operator++, but this method directly fetches the Attribute objects
for key and value (which is slightly faster than doing it with
a call of operator++ and a key() afterwards).

*/
  bool operator!=(const BTree2Iterator& b2);
/*
Compare two iterators (e.g. with end()).

*/

  private:
  BTree2* btree;   // Iterator's state
  NodeId nodeId;   // Iterator's state
  int entryNumber; // Iterator's state
  Attribute* refvalue;  // Hold reference to value
  Attribute* refkey;    // Hold reference to key
};

}

#endif
