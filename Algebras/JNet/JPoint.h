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

2011, August Simone Jandt

*/

#ifndef JPOINT_H
#define JPOINT_H

#include <ostream>
#include <string>
#include "../../include/Attribute.h"
#include "../../include/StandardTypes.h"
#include "RouteLocation.h"

/*
1 class ~JPoint~

A JPoint is a position in a given Network. It consits of the network identifier
and a RouteLocation within this network.

*/

class JPoint : public Attribute
{
public:

/*
1.1 Constructors and Deconstructors

The default constructor should only be used in the Cast-Function.

*/

  JPoint();
  JPoint(const bool def);
  JPoint(const JPoint& other);
  JPoint(const string& netId, const RouteLocation& rloc);

  ~JPoint();

/*
1.1 Getter and Setter for private Attributes

*/

  string GetNetId() const;
  RouteLocation GetPosition() const;

  void SetNetId(const string& netId);
  void SetPosition(const RouteLocation& rloc);

/*
1.1 Override Methods from Attribute

*/

  void CopyFrom(const Attribute* right);
  StorageType GetStorageType() const;
  size_t HashValue() const;
  Attribute* Clone() const;
  bool Adjacent(const Attribute* attrib) const;
  int Compare(const Attribute* rhs) const;
  int Compare(const JPoint& rhs) const;
  size_t Sizeof() const;
  ostream& Print(ostream& os) const;
  static const string BasicType();
  static const bool checkType(const ListExpr type);

/*
1.1 Standard Operators

*/

  JPoint& operator=(const JPoint& other);
  bool operator==(const JPoint& other) const;

/*
1.1 Operators for Secondo Integration

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
1.1 Other Operations

*/

private:

  string nid;
  RouteLocation npos;
};

#endif // JPOINT_H
