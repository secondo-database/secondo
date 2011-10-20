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

*/

#ifndef NLOC_H
#define NLOC_H

#include <ostream>
#include "Attribute.h"
#include "Direction.h"

/*
1. ~class RouteLocation~

The class RouteLocation describes a position in the network. It consists of a
route identifier rid, the distance from the start of the route pos, and a
~direction~ value telling if the position is reachable from the Up side, the
Down side or Both sides of the road.

*/

class RouteLocation : public Attribute
{

public:

/*
1.1 Constructors and Deconstructors

The default constructor should only be used in the Cast-Function.

*/

RouteLocation();
RouteLocation(const RouteLocation& other);
RouteLocation(const bool defined);
RouteLocation(const int routeId, const double position,
              const Direction sideofroad);
~RouteLocation();

/*
1.2 Getter and Setter for private Attributes

*/

int GetRouteId() const;
double GetPosition() const;
Direction GetSide() const;

void SetRouteId(const int routeid);
void SetPosition(const double position);
void SetSide(const Direction sideofroad);

/*
1.3 Override Methods from Attribute

*/

void CopyFrom(const Attribute* right);
StorageType GetStorageType() const;
size_t HashValue() const;
Attribute* Clone() const;
bool Adjacent(const Attribute* attrib) const;
bool Adjacent(const RouteLocation attrib) const;
int Compare(const Attribute* rhs) const;
int Compare(const RouteLocation& rhs) const;
size_t Sizeof() const;
ostream& Print(ostream& os) const;
static const string BasicType();
static const bool checkType(const ListExpr type);

/*
1.4 Standard Operators

*/

RouteLocation& operator=(const RouteLocation& other);
bool operator==(const RouteLocation& other) const;

/*
 1 .5 Operators for Secondo Integration

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
static ListExpr Property();

/*
1.6 Helpful operations

Returns true if the side values are identic or at least one of them is ~Both~.

*/

bool SameSide(const RouteLocation& rloc, const bool strict = true) const;

private:

  int rid; //route identifier
  double pos; //network distance from the start of the route
  Direction side; //side(s) from which the position is reachable

};

#endif // NLOC_H
