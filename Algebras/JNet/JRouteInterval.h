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

#ifndef JROUTEINTERVAL_H
#define JROUTEINTERVAL_H

#include <ostream>
#include "Attribute.h"
#include "Direction.h"
#include "RouteLocation.h"
#include "RectangleAlgebra.h"
#include "SpatialAlgebra.h"

class JNetwork;

/*
1 ~class JRouteInterval~

A ~JRouteInterval~ describes a part of a route. It consists of the route
identfier the interval belongs to, the distances of the start and
the end position of the interval on the route, the side value telling if
the ~JRouteInterval~ covers the Up, Down or Both sides of the road.

Because we can see the direction from the side parameter the start position is
always smaller or equal the end position.

*/

class JRouteInterval : public Attribute
{

/*
1.1 public deklarations

*/
public:

/*
1.1.1 Constructors and Deconstructor

The default constructor may only be used in the Cast-Function. It can not be
set to be private because JRouteInterval is used as DbArrayElement in other
classes.

*/

  JRouteInterval();
  explicit JRouteInterval(const bool defined);
  JRouteInterval(const JRouteInterval& other);
  JRouteInterval(const int routeid, const double from, const double to,
                 const Direction sideofroad);
  JRouteInterval(const int routeid, const double from, const double to,
                 const JSide sideofroad);

  JRouteInterval(const RouteLocation& from, const RouteLocation& to,
                 const bool allowResetSide = false);

  ~JRouteInterval();

/*
1.1.1 Getter and Setter for private Attributes

*/

    int GetRouteId() const;

/*
Returns always the smaller distance from the start of the route.

*/

    double GetFirstPosition()const;

/*
Returns always the bigger distance from the start of the route.

*/

    double GetLastPosition()const;
    RouteLocation GetStartLocation() const;

/*
Returns the first point of the routeinterval respecting the direction of the
route interval on the road.

*/

    double GetStartPosition()const;
    RouteLocation GetEndLocation() const;
/*
Returns the last point of the routeinterval respecting the direction of the
route interval on the road.

*/

    double GetEndPosition()const;

    Direction GetSide() const;

    double GetLength() const;

    void SetRouteId(const int routeid);
    void SetSide(const Direction sideofroad);
    void SetInterval(const double from, const double to);


/*
1.1.1 Override Methods from Attribute

*/

  void CopyFrom(const Attribute* right);
  Attribute::StorageType GetStorageType() const;
  size_t HashValue() const;
  JRouteInterval* Clone() const;
  bool Adjacent(const Attribute* attrib) const;
  bool Adjacent(const JRouteInterval& other)const;
  static int Compare(const void* ls, const void* rs);
  int Compare(const Attribute* rhs) const;
  int Compare(const JRouteInterval& rhs) const;
  int Compare(const RouteLocation& rloc ) const;
  size_t Sizeof() const;
  ostream& Print(ostream& os) const;
  static const string BasicType();
  static const bool checkType(const ListExpr type);

/*
1.1.1 Standard Operators

*/

  JRouteInterval& operator=(const JRouteInterval& other);

  bool operator==(const JRouteInterval& other) const;
  bool operator!=(const JRouteInterval& other) const;
  bool operator<(const JRouteInterval& other) const;
  bool operator<=(const JRouteInterval& other) const;
  bool operator>(const JRouteInterval& other) const;
  bool operator>=(const JRouteInterval& other) const;

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
1.1.1 Helpful Operators

1.1.1.1 ~Example~

Provides example string for list representation can be used by external
property definitions for part of ~JRouteInterval~ representation.

*/

static string Example();

/*
1.1.1.1 SameSide

Returns true if the both ~JRouteIntervals~ have identic side values or,
if strict is set to false, at least one of the side values is both.

*/

  bool SameSide(const JRouteInterval& other, const bool strict = true) const;

/*
1.1.1.1 Overlaps

Returns true if the two JRouteIntervals are on the same road at the same side
and their intervals overlap at atleast one point.

*/

  bool Overlaps(const JRouteInterval& other,
                bool strict = true) const;

/*
1.1.1.1 Contains

Returns true if the JRouteInterval contains the route location resp. interval.

*/

  bool Contains(const RouteLocation& rloc) const;
  bool Contains(const RouteLocation& rloc, const double tolerance) const;
  bool Contains(const JRouteInterval& other) const;

/*
1.1.1.1 Inside

Returns true if the route interval is complete into the other route interval,
false elsewhere.

*/

 bool Inside(const JRouteInterval& other) const;

/*
1.1.1.1 Intersection

Returns the intersecting part of the both route intervals if exists,
0 elsewhere.

*/

  JRouteInterval* Intersection(const JRouteInterval& rint) const;

/*
1.1.1.1 Extend

Extends the current RouteInterval to enlcose also the given RouteInterval.

*/

JRouteInterval& Extend(const JRouteInterval& rint);

/*
1.1.1.1 Between

Returns true if the routeInterval is between the two given route locations,
false elsewhere.

*/

bool Between(const RouteLocation& left, const RouteLocation& right) const;

/*
1.1.1.1 NetBox

Returns a 2 dimensional rectangle where x1 and x2 are identic and respresent
the route id, y1 represents the start position on this route and y2 represents
the end position on this route, and z1 is the start time and z2 is the end
time. All coordinates are double values.

*/

Rectangle<2> NetBox() const;

/*
1.1.1.1 BoundingBox

Returns an two dimensional rectangle with the spatial bounding box of the
route interval.

*/

Rectangle<2> BoundingBox(JNetwork* jnet) const;

/*
1.1 private deklarations

*/

private:

/*
1.1.1 Private Attributes of ~JRouteInterval~

*/

  int rid; //route identifier
  double startpos, endpos; //start respectively end position on route
  Direction side; //covered side(s) of the road

/*
1.1.1 Private Methods

1.1.1.1 SetStartPosition and SetAndPosition

Use carefully. They may not work as expected by their name, because they are
only introduced for connecting overlapping route intervals while sorting and
compressing the JRouteInterval sets in the JLine data type. Using JRITree.

*/

    void SetStartPosition(const double position);
    void SetEndPosition(const double position);

    friend class JRITreeElement;
};

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const JRouteInterval& jir);


#endif // JROUTEINTERVAL_H
