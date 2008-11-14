/*

---- 
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Faculty of Mathematics and Computer Science, 
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

1 The BBTree class

The BBTree is a data structure which can be used to determine the bounding box
of a moving point for a determines time interval.

*/

#ifndef BBTREE_H
#define BBTREE_H



#include "TemporalAlgebra.h" 
#include "RectangleAlgebra.h"
#include <iostream>

class BBTreeNode;


class BBTree{
  public:

/*
~Constructor~

This constructor creates a BBTree for a given MPoint;

*/
    BBTree(const MPoint& p);

/*
~Copy constructor~

*/
    BBTree(const BBTree& t);
/*
~Assignment Operator~

*/
    BBTree& operator=(const BBTree& src);
/*
~Destructor~

*/
    ~BBTree();

/*
~getBox~

This Operator returns the bounding box of the mpoint
from which created it this tree when it is restricted to the
given time interval.

*/
    Rectangle<2> getBox(Interval<Instant> interval)const;

/*
~noLeafs~

Returns the number of leaves within that tree

*/
    int noLeaves()const;

/*
~noNodes~

Returns the number of nodes within this tree.

*/
    int noNodes() const;
    
/*
~height~

Returns the hieght of the tree.

*/
    int height() const;

/*
~print~

Writes the tree to o;

*/
  ostream& print(ostream& o) const;

  private:

/*
~root~

Represents the root of this tree.

*/
    BBTreeNode* root;

/*
~createFromMPoint~

Builds the tree for a given mpoint.


*/
    void createFromMPoint(const MPoint& p);

};

ostream& operator<<(ostream& o, const BBTree& t);


#endif




