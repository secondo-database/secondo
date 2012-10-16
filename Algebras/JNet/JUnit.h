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
#include "IJPoint.h"

class JNetwork;

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
  RouteLocation* GetStartRLoc() const;
  RouteLocation* GetEndRLoc() const;
  double GetSpeed() const;
  double GetLength() const;

  void SetTimeInterval(const Interval<Instant>& inst);
  void SetRouteInterval(const JRouteInterval& ri);

/*
1.1.1 Override Methods from Attribute

*/

  void CopyFrom(const Attribute* right);
  StorageType GetStorageType() const;
  size_t HashValue() const;
  JUnit* Clone() const;
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
1.1.1.1 ~ExtendBy~

Extends the given JUnit to cover the values of other, also.

*/

  bool ExtendBy(const JUnit& other);

/*
1.1.1.1 Boxes

1.1.1.1.1 TempNetBox

Returns a 3 dimensional rectangle where x1 and x2 are identic and respresent
the route id, y1 represents the start position on this route and y2 represents
the end position on this route, and z1 is the start time and z2 is the end
time. All coordinates are double values.

*/

Rectangle<3> TempNetBox() const;

/*
1.1.1.1.1 NetBox

Returns a 2 dimensional rectangle where x1 and x2 are identic and respresent
the route id, y1 represents the start position on this route, and y2 represents
the end position on this route. All coordinates are double values.

*/

Rectangle<2> NetBox() const;

/*
1.1.1.1.1 BoundigBox

Returns an 3 dimensional rectangle with the spatio-temporal bounding box of the
junit.

*/

Rectangle<3> BoundingBox(JNetwork* jnet) const;

/*
1.1.1.1.1 AtInstant

Returns the ijpoint giving the position in the network at the given instant.

*/

IJPoint AtInstant(const Instant* inst, const string netId) const;

/*
1.1.1.1.1 Initial

Returns the ijpoint giving the position in the network at the start of the
time interval.

*/

IJPoint Initial(const string netId) const;

/*
1.1.1.1.1 Split

Translates the junit into an corresponding MPoint value.

*/

MPoint* Split(const JNetwork* jnet, bool& endTimeCorrected,
              Instant& lastEnd) const;

/*
1.1.1.1.1 AtPos

Returns the junit when the jpoint  was at position.

*/

JUnit* AtPos(const JPoint* jp) const;

/*
1.1.1.1.1 AtRint

Returns the junit when the jpoint  was at the route interval.

*/

JUnit* AtRint(const JRouteInterval* rint) const;

/*
1.1.1 ~PosAtTime~

Returns the position of the jpoint at the given time.

*/

double PosAtTime(const Instant* inst) const;
JRouteInterval* PosAtTimeInterval(const Interval<Instant>& time) const;

/*
1.1.1 ~TimeAtPosTime~

Returns the time when the jpoint was at the given position.

*/

Instant TimeAtPos(const double pos) const;

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

/*
1.1.1 ~CanBeExtendedBy~

Checks if the current JUnit can be extended to include other without loss of
information. That means the time interval of this ends when other time interval
starts, the route intervals are adjacent in same direction and the speed of
both units is the same.

*/

bool CanBeExtendedBy(const JUnit& other) const;


};

/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const JUnit& jp);

#endif // JUNIT_H
