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

2011, October Simone Jandt

*/

#ifndef JLINE_H
#define JLINE_H

#include <ostream>
#include <string>
#include "Attribute.h"
#include "JRouteInterval.h"
#include "StandardTypes.h"
#include "../Tools/Flob/DbArray.h"

/*
1 class JLine

Consists of a network id and a set of RouteIntervals describing an part of a
network.

*/

class JLine : public Attribute
{

public:

/*
1.1 Constructors and Deconstructor

The default Constructor  should not be used outside the cast function.

*/

  JLine();
  JLine(const bool defined);
  JLine(const JLine& other);

  ~JLine();

/*
1.1 Getter and Setter for private attributes

*/

  string GetNetworkId() const;
  DbArray<JRouteInterval>* GetRouteIntervals() const;

  void SetNetworkId(string& nid);
  void SetRouteIntervals(DbArray<JRouteInterval>* setri);

/*
1.1 Override Methods from Attribute

*/
  void CopyFrom(const Attribute* right);
  size_t HashValue() const;
  Attribute* Clone() const;
  bool Adjacent(const Attribute* attrib) const;
  int Compare(const Attribute* rhs) const;
  int Compare(const JLine* rhs) const;
  size_t Sizeof() const;
  int NumOfFLOBs() const;
  Flob* GetFLOB(const int i);
  void Destroy();
  ostream& Print(ostream& os) const;
  Attribute::StorageType GetStorageType() const;
  static const string BasicType();
  static const bool checkType(const ListExpr type);

/*
1.1 Standard Operators

*/

  JLine& operator=(const JLine& other);
  bool operator==(const JLine& other) const;

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
  static ListExpr Property();

/*
1.1 Other helpful operators

*/

  static string Example();
/*
1.1.1 GetNoComponents

Returns the number of routeintervals of the jline.

*/

  int GetNoComponents() const;

/*
1.1.1 Get

Returns the routeinterval at the given position

*/

  void Get(const int i, JRouteInterval& ri) const;

private:

/*
1.1 Private Attributes

*/
  STRING_T netId;
  DbArray<JRouteInterval> routeintervals;

};

#endif // JLINE_H
