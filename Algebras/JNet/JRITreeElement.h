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

1 Defines and includes

*/

#ifndef JRITREEELEMENT_H
#define JRITREEELEMENT_H

#include "JRouteInterval.h"

/*
1 class JRITreeElement

An JRITreeElement is used in the JRITree. It consists of an
~JRouteInterval~ and two int values indexing the right and left son of the
element in the JRITree.

*/

class JRITreeElement{

/*
1.1 public declaration part

*/

public:

/*
1.1.1 Constructors and deconstructor

*/

JRITreeElement();
JRITreeElement(const JRouteInterval& ri, const int l = -1, const int r = -1 );
JRITreeElement(const JRITreeElement& other);
~JRITreeElement();

/*
1.1.1 Getter and Setter

*/

JRouteInterval GetRouteInterval() const;
double GetValue(const bool start) const;
int GetLeftSon() const;
int GetRightSon() const;

void SetRouteInterval(const JRouteInterval in);
void SetLeftSon(const int i);
void SetRightSon(const int i);

void SetRouteIntervalRid (const int r);
void SetRouteIntervalStart (const double pos);
void SetRouteIntervalEnd (const double pos);

/*
1.1.1 Some standard operations

*/

void operator= (const JRITreeElement nelem);
ostream& Print(ostream&) const;


/*
1.1 private declaration part

*/

private:

  JRouteInterval rint;  //single route part
  int left, right;      //point by index in array to left resp. right son

};

/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const JRITreeElement elem);

#endif //JRITREEELEMENT_H