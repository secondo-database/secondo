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

#ifndef JUNIT_H
#define JUNIT_H

#include <ostream>
#include <string>
#include "Attribute.h"
#include "StandardTypes.h"
#include "DateTime.h"
#include "TemporalAlgebra.h"
#include "JRouteInterval.h"
#include "RouteLocation.h"

/*
1 class ~JUnit~

A ~JUnit~ consist of an network id, an ~JRouteInterval~, and an time
intervall. It represents the positions of the ~MJPoint~ in the ~JNetwork~ in
this time intervall.

*/

class JUnit : public Attribute
{

/*
1.1 Public Declarations

*/
public:

/*
1.1.1 Constructors and Deconstructors

The Standard Constructor should not be used without inside the cast function.
It can not be private because JUnit is used as part of MJPoint and JUnit.

*/

  JUnit();
  explicit JUnit(const bool def);
  JUnit(const JUnit& other);
  JUnit(const Interval<Instant>& inst, const JRouteInterval& rint);

  ~JUnit();

/*
1.1.1 Getter and Setter for private Attributes

*/

  Interval<Instant> GetTimeInterval() const;
  JRouteInterval GetRouteInterval() const;
  RouteLocation* GetStartPoint() const;
  RouteLocation* GetEndPoint() const;
  double GetSpeed() const;

  void SetTimeInterval(const Interval<Instant>& inst);
  void SetRouteInterval(const JRouteInterval& ri);

/*
1.1.1 Override Methods from Attribute

*/

  void CopyFrom(const Attribute* right);
  StorageType GetStorageType() const;
  size_t HashValue() const;
  Attribute* Clone() const;
  bool Adjacent(const Attribute* attrib) const;
  static int Compare(const void* ls, const void* rs);
  int Compare(const Attribute* rhs) const;
  int Compare(const JUnit& rhs) const;
  size_t Sizeof() const;
  ostream& Print(ostream& os) const;
  static const string BasicType();
  static const bool checkType(const ListExpr type);

/*
1.1.1 Standard Operators

*/

  JUnit& operator=(const JUnit& other);

  bool operator==(const JUnit& other) const;
  bool operator!=(const JUnit& other) const;
  bool operator<(const JUnit& other) const;
  bool operator<=(const JUnit& other) const;
  bool operator>(const JUnit& other) const;
  bool operator>=(const JUnit& other) const;

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

  Interval<Instant> timeInter; //time interval the mjpoint needs to move from
                               //start to end
  JRouteInterval routeInter;   //network positions the mjpoint passes in the
                               //time interval

};

/*
1 Overwrite output operator

*/

ostream& operator<< (const ostream& os, const JUnit& jp);
#endif // JUNIT_H
