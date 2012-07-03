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

1 Includes

*/

#include "JRITreeElement.h"

/*
1 Implementation of class JRITreeElement

1.1 Constructors and Deconstructors

*/

JRITreeElement::JRITreeElement()
{}

JRITreeElement::JRITreeElement(const JRouteInterval& ri, const int l /*= -1*/,
                               const int r  /*= -1*/) :
    rint(ri), left(l), right(r)
{}

JRITreeElement::JRITreeElement(const JRITreeElement& other) :
    rint(other.GetRouteInterval()), left(other.GetLeftSon()),
    right(other.GetRightSon())
{}

JRITreeElement::~JRITreeElement()
{}


/*
1.1.1 Getter and Setter

*/

JRouteInterval JRITreeElement::GetRouteInterval() const
{
  return rint;
}

double JRITreeElement::GetValue(const bool start) const
{
  if (start)
    return rint.GetFirstPosition();
  else
    return rint.GetLastPosition();
}


int JRITreeElement::GetLeftSon() const
{
  return left;
}

int JRITreeElement::GetRightSon() const
{
  return right;
}

void JRITreeElement::SetRouteInterval(const JRouteInterval in)
{
  rint = in;
}

void JRITreeElement::SetLeftSon(const int i)
{
  left = i;
}

void JRITreeElement::SetRightSon(const int i)
{
  right = i;
}

void JRITreeElement::SetRouteIntervalStart (const double pos)
{
  rint.SetStartPosition(pos);
}

void JRITreeElement::SetRouteIntervalEnd (const double pos)
{
  rint.SetEndPosition(pos);
}

void JRITreeElement::SetRouteIntervalRid(const int r)
{
  rint.SetRouteId(r);
}

/*
1.1.1 Some standard operations

*/

void JRITreeElement::operator= (const JRITreeElement telem)
{
  rint = telem.GetRouteInterval();
  left = telem.GetLeftSon();
  right = telem.GetRightSon();
}

ostream& JRITreeElement::Print(ostream& os) const
{
  os << "JRITreeElement: ";
  rint.Print(os);
  os << ", left son: " << left << ", right son: " << right << endl;
  return os;
}

/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const JRITreeElement elem)
{
  elem.Print(os);
  return os;
}
