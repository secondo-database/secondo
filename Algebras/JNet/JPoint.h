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

#ifndef JPOINT_H
#define JPOINT_H

#include <ostream>
#include <string>
#include "Attribute.h"
#include "StandardTypes.h"
#include "RouteLocation.h"
#include "JPath.h"
#include "JNetwork.h"

namespace jnetwork {

class JPoints;
class JLine;

/*
1 class ~JPoint~

A ~JPoint~ is a position in a given ~JNetwork~. It consits of the network
identifier and a RouteLocation within this network.

*/

class JPoint : public Attribute
{

/*
1.1 Public Declarations

*/
public:

/*
1.1.1 Constructors and Deconstructors

The standard constructor should only be used in Cast-Function. It can not be
declared to be private because ~jpoint~ is used as attribute by other datatypes.

*/

  JPoint();
  explicit JPoint(const bool def);
  JPoint(const JPoint& other);
  JPoint(const string netId, const RouteLocation& rloc,
         const bool check = true);
  JPoint(const JNetwork* jnet, const RouteLocation* rloc,
         const bool check = true);

  ~JPoint();

/*
1.1.1 Getter and Setter for private Attributes

*/

  const STRING_T* GetNetworkId() const;
  RouteLocation GetLocation() const;

  void SetNetId(const STRING_T& netId);
  void SetPosition(const RouteLocation& rloc, const bool check = true,
                   const JNetwork* jnet = 0);

/*
1.1.1 Override Methods from Attribute

*/

  void CopyFrom(const Attribute* right);
  StorageType GetStorageType() const;
  size_t HashValue() const;
  JPoint* Clone() const;
  bool Adjacent(const Attribute* attrib) const;
  static int Compare(const void* ls, const void* rs);
  int Compare(const Attribute* rhs) const;
  int Compare(const JPoint& rhs) const;
  size_t Sizeof() const;
  ostream& Print(ostream& os) const;
  static const string BasicType();
  static const bool checkType(const ListExpr type);

/*
1.1.1 Standard Operators

*/

  JPoint& operator=(const JPoint& other);

  bool operator==(const JPoint& other) const;
  bool operator!=(const JPoint& other) const;
  bool operator<(const JPoint& other) const;
  bool operator<=(const JPoint& other) const;
  bool operator>(const JPoint& other) const;
  bool operator>=(const JPoint& other) const;

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
1.1.1 Other Operations

1.1.1.1 Example

Returns an string with an example list representation.

*/
  static string Example();

/*
1.1.1 Translation from / to spatial point

1.1.1.1 FromSpatial

Computes the jpoint corresponding to the given point in jnet.

*/

  void FromSpatial(const JNetwork* jnet, const Point* p);

/*
1.1.1.1 ToSpatial

Computes the corresponding spatial point from the given jpoint.

*/

void ToSpatial(Point& result) const;

/*
1.1.1.1 NetBox

Returns a 2 dimensional rectangle where x1 and x2 are identic and respresent
the route id, and y1 and y2 are identic and  represent the position on this
route. All coordinates are double values.

*/

Rectangle<2> NetBox() const;

/*
1.1.1.1 OtherNetworkPositions

Returns a list of alternative route locations for this jpoint.

*/

JListRLoc* OtherNetworkPositions() const;

/*
1.1.1.1 ShortestPathTree

Computes the network distance to all junctions in the network and stores
them in result.

*/

void ShortestPathTree(DbArray<PairIntDouble>* result) const;
void ReverseShortestPathTree(DbArray<PairIntDouble>* result) const;

/*
1.1.1.1 ShortestPath

Returns the shortest path from this to target as jpath.

*/

void ShortestPath(const JPoint* target, JPath* result) const;
void ShortestPath(const JPoints* target, JPath* result) const;
void ShortestPath(const JLine* target, JPath* result) const;

/*
1.1.1.1 Netdistance

Returns the network distance to the target, which is given by the length of
the shortest path to the target.

*/

void Netdistance(const JPoint* target, CcReal* result) const;
void Netdistance(const JPoints* target, CcReal* result) const;
void Netdistance(const JLine* target, CcReal* result) const;

/*
1.1 Private declarations

*/

private:

/*
1.1.1 Attributes

*/

  STRING_T nid;         // network id of the network the point belongs to.
  RouteLocation npos; //position in this network.

/*
1.1.1 Private Methods

1.1.1.1 PosExists

Returns true if the rloc is in the given jnet false otherwise.

*/

bool PosExists(const JNetwork* jnet = 0) const;

};

} // end of namespace jnetwork

/*
1 Overwrite output operator

*/

using namespace jnetwork;
ostream& operator<< (ostream& os, const JPoint& jp);


#endif // JPOINT_H
