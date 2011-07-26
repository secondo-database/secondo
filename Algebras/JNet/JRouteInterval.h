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

2011, April Simone Jandt

1 Includes

*/

#ifndef JROUTEINTERVAL_H
#define JROUTEINTERVAL_H

#include <ostream>
#include "../../include/Attribute.h"
#include "Direction.h"
#include "RouteLocation.h"

/*
1 ~class JRouteInterval~

A ~RouteInterval~ describes a part of a route. It consists of the route
identfier of the route it belongs to, the start and the end position of the
interval on the route, the side value telling if the ~routeinterval~
covers the upside, the downside or both sides of the route.

Because we can see the direction from the side parameter the start position is
always smaller or equal the end position.

*/
class JRouteInterval : public Attribute
{

public:

/*
1.1 Constructors and Deconstructor

The default constructor should never been used, except in the Cast-Function.

*/

  JRouteInterval();
  JRouteInterval(const JRouteInterval& other);
  JRouteInterval(const int routeid, const double from, const double to,
                 const Direction sideofroad);
  JRouteInterval(const bool defined);
  JRouteInterval(const RouteLocation& from, const RouteLocation& to);
  ~JRouteInterval();

/*
1.2 Getter and Setter for private Attributes

*/

    int GetRouteId() const;
    double GetStartPosition()const;
    double GetEndPosition()const;
    Direction GetSide() const;

    void SetRouteId(const int routeid);
    void SetStartPosition(const int position);
    void SetEndPosition(const int position);
    void SetSide(const Direction sideofroad);

/*
1.3 Override Methods from Attribute

*/

  void CopyFrom(const Attribute* right);
  Attribute::StorageType GetStorageType() const;
  size_t HashValue() const;
  Attribute* Clone() const;
  bool Adjacent(const Attribute* attrib) const;
  int Compare(const Attribute* rhs) const;
  int Compare(const JRouteInterval& rhs) const;
  size_t Sizeof() const;
  ostream& Print(ostream& os) const;
  static const string BasicType();
  static const bool checkType(const ListExpr type);

/*
1.4 Standard Operators

*/

  JRouteInterval& operator=(const JRouteInterval& other);
  bool operator==(const JRouteInterval& other) const;

/*
1.5 Operators for Secondo Integration

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
  static bool Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value );
  static bool Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value );

/*
1.6 Helpful Operators

1.1.1 IsOneSided

Returns true if the route interval covers only one side of the route.

*/

    bool IsOneSided() const;

/*
1.1.1 SameSide

Returns true if the ~route intervals~ have identic side value or at least one
of them is both.

*/

  bool SameSide(const JRouteInterval& other) const;

/*
1.1.1 Intersects

Returns true if the ~jrouteintervals~ are on the same route and intersect.

*/

  bool Intersects(const JRouteInterval& other) const;

/*
1.1.1 contains

Returns true if the ~routelocation~ is covered by the ~routeinterval~.

*/

  bool Contains(const RouteLocation& rloc) const;

/*
Returns true if the ~jrouteinterval~ covers the ~jrouteinterval~.

*/

  bool Contains(const JRouteInterval& rloc) const;

private:

/*
1.1 Private Attributes of ~routeinterval~

*/

  int rid; //route identifier
  double startpos, endpos; //start and end position on route
  Direction side; //covered side(s) of the road
};

#endif // JROUTEINTERVAL_H
