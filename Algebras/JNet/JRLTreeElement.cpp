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

1 Includes

*/

#include "JRLTreeElement.h"

/*
1 Implementation of class JRLTreeElement

1.1 Constructors and Deconstructors

*/

JRLTreeElement::JRLTreeElement()
{}

JRLTreeElement::JRLTreeElement(const RouteLocation& rl, const int l /*= -1*/,
                               const int r  /*= -1*/) :
    rloc(rl), left(l), right(r)
{}

JRLTreeElement::JRLTreeElement(const JRLTreeElement& other) :
    rloc(other.GetRouteLocation()), left(other.GetLeftSon()),
    right(other.GetRightSon())
{}

JRLTreeElement::~JRLTreeElement()
{}


/*
1.1.1 Getter and Setter

*/

RouteLocation JRLTreeElement::GetRouteLocation() const
{
  return rloc;
}

int JRLTreeElement::GetLeftSon() const
{
  return left;
}

int JRLTreeElement::GetRightSon() const
{
  return right;
}

void JRLTreeElement::SetRouteLocation(const RouteLocation in)
{
  rloc = in;
}

void JRLTreeElement::SetLeftSon(const int i)
{
  left = i;
}

void JRLTreeElement::SetRightSon(const int i)
{
  right = i;
}

void JRLTreeElement::SetRouteLocationPos (const double pos)
{
  rloc.SetPosition(pos);
}

void JRLTreeElement::SetRouteLocationRid(const int r)
{
  rloc.SetRouteId(r);
}

/*
1.1.1 Some standard operations

*/

void JRLTreeElement::operator= (const JRLTreeElement telem)
{
  rloc = telem.GetRouteLocation();
  left = telem.GetLeftSon();
  right = telem.GetRightSon();
}

ostream& JRLTreeElement::Print(ostream& os) const
{
  os << "JRLTreeElement: ";
  rloc.Print(os);
  os << ", left son: " << left << ", right son: " << right << endl;
  return os;
}

/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const JRLTreeElement elem)
{
  elem.Print(os);
  return os;
}
