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
#include "JNetwork.h"
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
  JPoint(const string netId, const RouteLocation& rloc);
  JPoint(const JNetwork* jnet, const RouteLocation* rloc);

  ~JPoint();

/*
1.1.1 Getter and Setter for private Attributes

*/

  const STRING_T* GetNetworkId() const;
  RouteLocation GetPosition() const;

  void SetNetId(const STRING_T& netId);
  void SetPosition(const RouteLocation& rloc);

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

*/
  static string Example();

  void FromSpatial(const JNetwork* jnet, const Point* p);

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
1.1 Private declarations

*/

private:

/*
1.1.1 Attributes

*/

  STRING_T nid;         // network id of the network the point belongs to.
  RouteLocation npos; //position in this network.

};

/*
1 Overwrite output operator

*/

ostream& operator<< (const ostream& os, const JPoint& jp);
#endif // JPOINT_H
