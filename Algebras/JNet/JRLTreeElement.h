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

2012, July Simone Jandt

1 Defines and includes

*/

#ifndef JRLTREEELEMENT_H
#define JRLTREEELEMENT_H

#include "RouteLocation.h"

/*
1 class JRLTreeElement

An JRLTreeElement is used in the JRLTree. It consists of an
~RouteLocation~ and two int values indexing the right and left son of the
element in the JRLTree.

*/

class JRLTreeElement {

/*
1.1 public declaration part

*/

public:

/*
1.1.1 Constructors and deconstructor

*/

JRLTreeElement();
JRLTreeElement(const RouteLocation& rl, const int l = -1, const int r = -1 );
JRLTreeElement(const JRLTreeElement& other);
~JRLTreeElement();

/*
1.1.1 Getter and Setter

*/

RouteLocation GetRouteLocation() const;
int GetLeftSon() const;
int GetRightSon() const;

void SetRouteLocation(const RouteLocation in);
void SetLeftSon(const int i);
void SetRightSon(const int i);

void SetRouteLocationRid (const int r);
void SetRouteLocationPos (const double pos);

/*
1.1.1 Some standard operations

*/

void operator= (const JRLTreeElement nelem);
ostream& Print(ostream&) const;


/*
1.1 private declaration part

*/

private:

  RouteLocation rloc;  //single route part
  int left, right;      //point by index in array to left resp. right son

};

/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const JRLTreeElement elem);

#endif //JRLTREEELEMENT_H