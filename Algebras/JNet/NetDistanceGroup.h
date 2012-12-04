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

#ifndef NETDISTANCEGROUP_H
#define NETDISTANCEGROUP_H

#include <ostream>
#include "Attribute.h"
#include "StandardTypes.h"

namespace jnetwork{

/*
1. class ~NetDistanceGroup~

Entry value for ~jlistndg~. Consisting of the identifier of the source junction,
the identifier of the target junction, the identifier of next junction in the
path from source to target junction, the identifier of the next section in the
path to next junction and the distance to the target junction.

Already computed shortest paths and network distances can be reused. But general
precomputing of the information is avoided. Often used paths and netdistances
can be recomputed very fast. Network parts not used in distance computation
doesn't need storage space.

*/

class NetDistanceGroup : public Attribute
{

/*
1.1 Public Declarations

*/

public:

/*
1.1.1 Constructors and Deconstructor

The default constructor may only be used in the cast function. It can not be
declared to be private because it is used as DbArrayElement in other classes.

*/
NetDistanceGroup();
explicit NetDistanceGroup(const bool def);
NetDistanceGroup (const NetDistanceGroup& other );
NetDistanceGroup(const int source, const int target, const int nextJunct,
                 const int nextSect, const double netdist);
~NetDistanceGroup();

/*
1.1.1 Getter and Setter for private Attributes

*/

int GetSource() const;
int GetTarget() const;
int GetNextJunction() const;
int GetNextSection() const;
double GetNetdistance() const;

void SetSource(const int t);
void SetTarget(const int t);
void SetNextJunction(const int t);
void SetNextSection(const int t);
void SetNetdistance(const double dist);

/*
1.1.1 Overriden Methods of Attribute

*/

void CopyFrom ( const Attribute* right );
StorageType GetStorageType() const;
size_t HashValue() const;
NetDistanceGroup* Clone() const;
bool Adjacent ( const Attribute* attrib ) const;

/*
Attention if you use compare functions for ~NetDistanceGroups~.
A comparison is only senseful for defined ~NetDistanceGroups~ which have the
same source and target id. In all other cases the result is not really useful.

*/

static int Compare(const void* ls, const void* rs);
int Compare ( const Attribute* rhs ) const;
int Compare (const NetDistanceGroup& rhs) const;
size_t Sizeof() const;
ostream& Print ( ostream& os ) const;
static const string BasicType();
static const bool checkType(const ListExpr type);

/*
1.1.1 Standard Operators

*/

NetDistanceGroup& operator= ( const NetDistanceGroup& other );

bool operator== ( const NetDistanceGroup& other ) const;
bool operator!= ( const NetDistanceGroup& other ) const;
bool operator< ( const NetDistanceGroup& other ) const;
bool operator<= ( const NetDistanceGroup& other ) const;
bool operator>= ( const NetDistanceGroup& other ) const;
bool operator> ( const NetDistanceGroup& other ) const;

/*
1.1.1 SecondoIntegration

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
static ListExpr Property();

/*
1.1.1 Helpful Operators

1.1.1.1 ~Example~

Provides example string for list representation can be used by external
property definitions for ~NetDistanceGroup~ representation.

*/

static string Example();

/*
1.1 Private declarations

*/

private:

/*
1.1.1 Attributes of NetDistanceGroup

*/

  int source;       //identifier of start junction
  int target;       //identifier of target junction
  int nextJunction; //identifier of next junction on path
  int nextSection;  //identifier of next section on path
  double netdistance; //length of shortest path between source and target
                      // junction
};

} // end of namespace jnetwork

/*
1 Overwrite output operator

*/

using namespace jnetwork;
ostream& operator<< (ostream& os, const jnetwork::NetDistanceGroup& ndg);

#endif // NETDISTANCEGROUP_H
