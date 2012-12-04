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

2012, July Simone Jandt

1 Defines and Includes

*/

#ifndef IJPOINT_H
#define IJPOINT_H

#include <ostream>
#include <string>
#include "Attribute.h"
#include "StandardTypes.h"
#include "DateTime.h"
#include "JPoint.h"

/*
1 class ~IJPoint~

A ~IJPoint~ is a ~JPoint~ combined with an time instant.
It represents the position of an ~MJPoint~ in the ~JNetwork~ at this time
instant.

*/
namespace jnetwork {
class IJPoint : public Attribute
{

/*
1.1 Public Declarations

*/
public:

/*
1.1.1 Constructors and Deconstructors

*/

  explicit IJPoint(const bool def);
  IJPoint(const IJPoint& other);
  IJPoint(const Instant& inst, const JPoint& jp);

  ~IJPoint();

/*
1.1.1 Getter and Setter for private Attributes

*/

  Instant GetInstant() const;
  JPoint GetPoint() const;

  void SetInstant(const Instant& inst);
  void SetPoint(const JPoint& jp);

/*
1.1.1 Override Methods from Attribute

*/

  void CopyFrom(const Attribute* right);
  StorageType GetStorageType() const;
  size_t HashValue() const;
  IJPoint* Clone() const;
  bool Adjacent(const Attribute* attrib) const;
  static int Compare(const void* ls, const void* rs);
  int Compare(const Attribute* rhs) const;
  int Compare(const IJPoint& rhs) const;
  size_t Sizeof() const;
  ostream& Print(ostream& os) const;
  static const string BasicType();
  static const bool checkType(const ListExpr type);

/*
1.1.1 Standard Operators

*/

  IJPoint& operator=(const IJPoint& other);

  bool operator==(const IJPoint& other) const;
  bool operator!=(const IJPoint& other) const;
  bool operator<(const IJPoint& other) const;
  bool operator<=(const IJPoint& other) const;
  bool operator>(const IJPoint& other) const;
  bool operator>=(const IJPoint& other) const;

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

/*
1.1 Private declarations

*/

private:

/*
1.1.1 Attributes

*/

  Instant time; //timeinstant the jpoint exists
  JPoint point; //position in the jnetwork at this time.

/*
1.1.1 Standard Constructor

The standard constructor should only be used in cast-Function and is therefore
declared to be private.

*/

  IJPoint();
};

} // end of namespace jnetwork

/*
1 Overwrite output operator

*/

using namespace jnetwork;

ostream& operator<< (const ostream& os, const jnetwork::IJPoint& jp);



#endif // IJPOINT_H
