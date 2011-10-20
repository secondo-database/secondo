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

May 2011 Simone Jandt

*/

#ifndef PAIRTIDRLOC_H
#define PAIRTIDRLOC_H

#include <ostream>
#include "Attribute.h"
#include "../TupleIdentifier/TupleIdentifier.h"
#include "RouteLocation.h"

/*
1. class ~PairTIDRLoc~
Pairs of ~tupleid~ and ~routelocation~ used to link entries of junctions
relation to routes relation, and vice versa. In List of routes meeting in a
junction. Respectively, list of junctions belonging to a route.

*/

class PairTIDRLoc : public Attribute
{

public:
/*
1.1 Constructors and deconstructor

The default constructor should only be used in the cast-Function.

*/

PairTIDRLoc();
PairTIDRLoc(const PairTIDRLoc& other);
PairTIDRLoc(const bool def);
PairTIDRLoc(const TupleIdentifier& t, const RouteLocation& rl);

~PairTIDRLoc();

/*
1.2 Getter and Setter for private Attributes

*/

TupleIdentifier GetTID() const;
TupleId GetTid() const;
RouteLocation GetRouteLocation() const;
void SetTID(const TupleIdentifier& t);
void SetTID(const TupleId& t);
void SetRouteLocation(const RouteLocation& rl);

/*
1.3 Override Methods from Attribute

*/

void CopyFrom(const Attribute* right);
StorageType GetStorageType() const;
size_t HashValue() const;
Attribute* Clone() const;
bool Adjacent(const Attribute* attrib) const;
int Compare(const Attribute* rhs) const;
int Compare(const PairTIDRLoc& rhs) const;
size_t Sizeof() const;
ostream& Print(ostream& os) const;
static const string BasicType();
static const bool checkType(const ListExpr type);
/*
1.4 Standard Methods

*/

PairTIDRLoc& operator=(const PairTIDRLoc& other);
bool operator==(const PairTIDRLoc& other) const;

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
static bool KindCheck( ListExpr type, ListExpr& errorInfo );
static int SizeOf();
static bool Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value );
static bool Open(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value );
static ListExpr Property();

/*
1.6 Helpful Operators

*/

private:

  TupleIdentifier tid;
  RouteLocation rloc;
};

#endif // PAIRTIDRLOC_H
