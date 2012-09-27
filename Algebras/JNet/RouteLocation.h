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

1 Defines and Includes

*/

#ifndef JRLOC_H
#define JRLOC_H

#include <ostream>
#include "Attribute.h"
#include "Direction.h"
#include "RectangleAlgebra.h"

/*
1 ~class RouteLocation~

The class RouteLocation describes a position in the network. It consists of an
~int~ value as route identifier, an double value telling the distance of the
position from the start of the route, and an ~jdirection~ value telling if the
position is reachable from the Up side, the Down side or Both sides of the road.

*/

class RouteLocation : public Attribute
{

/*
1.1 public deklaration part

*/

public:

/*
1.1.1 Constructors and Deconstructors

The default constructor should only be used in the Cast-Function. It can not be
declared private because it is used as DbArray-Element in other classes.

*/
RouteLocation();
RouteLocation(const RouteLocation& other);
explicit RouteLocation(const bool defined);
RouteLocation(const int routeId, const double position,
              const Direction sideofroad);
RouteLocation(const int routeId, const double position,
              const JSide sideofroad);

~RouteLocation();

/*
1.1.1 Getter and Setter for private Attributes

*/

int GetRouteId() const;
double GetPosition() const;
Direction GetSide() const;

void SetRouteId(const int routeid);
void SetPosition(const double position);
void SetSide(const Direction sideofroad);

/*
1.1.1 Override Methods from Attribute

*/

void CopyFrom(const Attribute* right);
StorageType GetStorageType() const;
size_t HashValue() const;
RouteLocation* Clone() const;
bool Adjacent(const Attribute* attrib) const;
bool Adjacent(const RouteLocation& attrib) const;
static int Compare(const void* ls, const void* rs);
int Compare(const Attribute* rhs) const;
int Compare(const RouteLocation& rhs) const;
size_t Sizeof() const;
ostream& Print(ostream& os) const;
static const string BasicType();
static const bool checkType(const ListExpr type);

/*
1.1.1 Standard Operators

*/

RouteLocation& operator=(const RouteLocation& other);

bool operator==(const RouteLocation& other) const;
bool operator<(const RouteLocation& other) const;
bool operator<=(const RouteLocation& other) const;
bool operator>(const RouteLocation& other) const;
bool operator>=(const RouteLocation& other) const;
bool operator!=(const RouteLocation& other) const;

/*
1.1.1 Operators for Secondo Integration

*/

static ListExpr Out(ListExpr typeInfo, Word value);
static Word In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct);
static Word Create(const ListExpr typeInfo);
static void Delete( const ListExpr typeInfo, Word& w );
static void Close( const ListExpr typeInfo, Word& w );
static Word Clone( const ListExpr typeInfo, const Word& w );
static void* Cast( void* addr );
static bool KindCheck ( ListExpr type, ListExpr& errorInfo );
static int SizeOf();
static ListExpr Property();

/*
1.1.1 Helpful operations


1.1.1.1 ~Example~

Provides example string for list representation can be used by external
property definitions for part of route location representation.

*/

static string Example();

/*
1.1.1.1 SameSide

Returns true if the side values are identic or, if strict is set to false,
at least one of them is ~Both~.

*/

bool SameSide(const RouteLocation& rloc, const bool strict = true) const;

/*
1.1.1.1 ~IsOnSameRoute~

Returns true if the rid is the same. Otherwise false.

*/

bool IsOnSameRoute(const RouteLocation& rloc) const;

/*
1.1.1.1 NetBox

Returns a 2 dimensional rectangle where x1 and x2 are identic and respresent
the route id, and y1 and y2 are identic and  represent the position on this
route. All coordinates are double values.

*/

Rectangle<2> NetBox() const;

/*
1.1. Private deklarations

*/

private:

/*
1.1.1 Attributes

*/

  int rid; //route identifier
  double pos; //network distance from the start of the route
  Direction side; //side(s) from which the position is reachable

};

/*
1 Overload output operator

*/

ostream& operator<<(ostream& os, const RouteLocation& dir);

#endif // JRLOC_H
