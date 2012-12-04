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

1. Includes

*/

#ifndef DIRECTION_H
#define DIRECTION_H

#include <ostream>
#include "Attribute.h"
#include "JSide.h"

/*
1 class ~Direction~

The class ~Direction~ makes the enum ~JSide~ usable as Attribute in relations
defining sections. Therefore it implements the secondo interface for
~Attributes~.

*/
namespace jnetwork{

class Direction : public Attribute
{

/*
1.1 public Methods of class ~Direction~

*/
public:

/*
1.1.1 Constructors and Deconstructors

The default Constructor should only be used in the cast-Function. Because
direction is used in other classes as member it the default constructor cannot
be defined to be private.

*/

  Direction();
  explicit Direction(const bool defined);
  Direction(const Direction& other);
  explicit Direction(const JSide inside);

  ~Direction();

/*
1.1.1 Getter and Setter for private attributes

*/

  JSide GetDirection() const;
  void SetSide(const JSide side);
  void SetDirection(const Direction& inside);

/*
1.1.1 Override Methods from Attribute

*/
  void CopyFrom(const Attribute* right);
  Attribute::StorageType GetStorageType() const;
  size_t HashValue() const;
  Direction* Clone() const;
  bool Adjacent(const Attribute* attrib) const;
  static int Compare(const void* ls, const void* rhs);
  int Compare(const Attribute* rhs) const;
  int Compare(const Direction& indir) const;
  int Compare(const JSide& ls, const JSide& rs)const ;
  size_t Sizeof() const;
  ostream& Print(ostream& os) const;
  static const string BasicType();
  static const bool checkType(const ListExpr type);

/*
1.1.1 Standard Operators

*/

  Direction& operator=(const Direction& other);

  bool operator==(const Direction& other) const;
  bool operator<(const Direction& other) const;
  bool operator<=(const Direction& other) const;
  bool operator>(const Direction& other) const;
  bool operator>=(const Direction& other) const;
  bool operator!=(const Direction& other) const;

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
1.1.1 Helpful Operations

1.1.1.1 Example

Provides example string for list representation can be used by external
property definitions for part of Direction representation.

*/

static string Example();

/*
1.1.1.1.1 SameSide

Returns true if the both direction values are equal or one of them is ~Both~.

*/

  bool SameSide(const Direction& dir, const bool strict = true) const;

/*
1.1 Private definitions of class ~Direction~

*/
private:

/*
1.1.1 Attributes of class ~Direction~

*/

  JSide side;

};

} //end of namespace jnetwork

/*
1 Overload output operator

*/

using namespace jnetwork;

ostream& operator<<(ostream& os, const jnetwork::Direction& dir);



#endif // DIRECTION_H
