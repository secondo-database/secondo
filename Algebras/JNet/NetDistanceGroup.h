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

2011, May Simone Jandt

*/

#ifndef NETDISTANCEGROUP_H
#define NETDISTANCEGROUP_H

#include <ostream>
#include "Attribute.h"
#include "../TupleIdentifier/TupleIdentifier.h"

/*
1. class ~NetDistanceGroup~

Entry value for ~netdistlist~. Consisting of the tupleId of the target node
in the junctions relation, the networkdistance from the actual node to this node
if already computed, the tupleId of the next section on the path to this node
in the sections relation and the tupleId of the next junction on the path to
the target node.

Already computed shortest paths and network distances can be reused. But general
precomputing of the information is avoided. Often used paths and netdistances
can be recomputed very fast. Not used network parts don't need storage space.

*/

class NetDistanceGroup : public Attribute
{

public:
/*
1.1 Constructors and Deconstructor

The default Constructor should not been used outside the cast-Function.

*/

NetDistanceGroup();
NetDistanceGroup(const bool def);
NetDistanceGroup (const NetDistanceGroup& other );
NetDistanceGroup(const TupleIdentifier target,
                 const TupleIdentifier nextSect,
                 const TupleIdentifier nextJunc,
                 const double netdist);
~NetDistanceGroup();

/*
1.2 Getter and Setter for private Attributes

*/

TupleIdentifier GetTargetTID() const;
TupleIdentifier GetNextSectionTID() const;
TupleIdentifier GetNextJunctionTID() const;
double GetNetdistance() const;

void SetTargetTID(const TupleIdentifier t);
void SetNextSectionTID(const TupleIdentifier t);
void SetNextJunctionTID(const TupleIdentifier t);
void SetNetdistance(const double dist);

/*
1.3 Overriden Methods of Attribute

*/

void CopyFrom ( const Attribute* right );
StorageType GetStorageType() const;
size_t HashValue() const;
Attribute* Clone() const;
bool Adjacent ( const Attribute* attrib ) const;
int Compare ( const Attribute* rhs ) const;
int Compare (const NetDistanceGroup& rhs) const;
size_t Sizeof() const;
ostream& Print ( ostream& os ) const;
static const string BasicType();
static const bool checkType(const ListExpr type);

/*
1.4 Standard Operators

*/

NetDistanceGroup& operator= ( const NetDistanceGroup& other );

bool operator== ( const NetDistanceGroup& other ) const;
bool operator!= ( const NetDistanceGroup& other ) const;
bool operator< ( const NetDistanceGroup& other ) const;
bool operator<= ( const NetDistanceGroup& other ) const;
bool operator>= ( const NetDistanceGroup& other ) const;
bool operator> ( const NetDistanceGroup& other ) const;

/*
1.5 SecondoIntegration

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

1.1.1 Example

Provides example string for list representation can be used by external
property definitions for net distance group representation.

*/

static string Example();

private:

/*
1.7 Attributes of NetDistanceGroup

*/

  TupleIdentifier targetTID;       //tupleId of target of shortest path in
                                   //junctions
  TupleIdentifier nextSectionTID;  //tupleId of next section on path in sections
  TupleIdentifier nextJunctionTID; //tupleId of next junction on path in
                                   //junctions
  double netdistance;              //length of shortest path to the target node

};

#endif // NETDISTANCEGROUP_H
