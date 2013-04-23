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

1 Defines and includes

*/

#ifndef JLINE_H
#define JLINE_H

#include <ostream>
#include <string>
#include "Attribute.h"
#include "JRouteInterval.h"
#include "StandardTypes.h"
#include "../Tools/Flob/DbArray.h"
#include "JList.h"
#include "JNetwork.h"
#include "SpatialAlgebra.h"
#include "JPoint.h"
#include "JPoints.h"

namespace jnetwork {
/*
1 class JLine

Consists of a network id and a set of ~JRouteIntervals~ describing an part of a
~jnetwork~. The ~JRouteIntervals~ are stored sorted by the route identifiers
and start and end positions and are compressed as far as possible.

*/


class JLine : public Attribute
{

public:

/*
1.1 public declarations

1.1.1 Constructors and Deconstructor

*/

  explicit JLine(const bool defined);
  explicit JLine(const string netId, const DbArray<JRouteInterval>& rintList,
                 const bool check = true, const bool issorted = false);
  explicit JLine(const JNetwork* jnet, const JListRInt* rintList,
                 const bool check = true);
  explicit JLine(const JLine& other);

  ~JLine();

/*
1.1.1 Getter and Setter for private attributes

*/

  const STRING_T* GetNetworkId() const;
  const DbArray<JRouteInterval>& GetRouteIntervals() const;

  void SetNetworkId(const STRING_T& nid);
  void SetRouteIntervals(const DbArray<JRouteInterval>& setri,
                         const bool check = true,
                         const bool issorted = false,
                         const JNetwork* jnet = 0);

/*
1.1.1 Override Methods from Attribute

*/
  void CopyFrom(const Attribute* right);
  size_t HashValue() const;
  JLine* Clone() const;
  bool Adjacent(const Attribute* attrib) const;
  static int Compare(const void* ls, const void* rs);
  int Compare(const Attribute* rhs) const;
  int Compare(const JLine& rhs) const;
  size_t Sizeof() const;
  int NumOfFLOBs() const;
  Flob* GetFLOB(const int i);
  void Destroy();
  void Clear();
  ostream& Print(ostream& os) const;
  Attribute::StorageType GetStorageType() const;
  static const string BasicType();
  static const bool checkType(const ListExpr type);

/*
1.1.1 Standard Operators

*/

  JLine& operator=(const JLine& other);

  bool operator==(const JLine& other) const;
  bool operator!=(const JLine& other) const;
  bool operator<(const JLine& other) const;
  bool operator<=(const JLine& other) const;
  bool operator>(const JLine& other) const;
  bool operator>=(const JLine& other) const;

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
1.1.1 Manage Bulkload of RouteIntervals

*/

  void StartBulkload();
  void EndBulkload(const bool sort = true);

  JLine& Add(const JRouteInterval& rint);

/*
1.1.1 Other helpful operators

1.1.1.1 ~Example~

Returns an example of the data type.

*/

  static string Example();

/*
1.1.1.1 GetNoComponents

Returns the number of routeintervals of the jline.

*/

  int GetNoComponents() const;

/*
1.1.1.1 IsEmpty

Returns true if the JLine is defined and has no routeintervals.

*/

  bool IsEmpty() const;

/*
1.1.1.1 Get

Returns the routeinterval at the given position

*/

  void Get(const int i, JRouteInterval& ri) const;

/*
1.1.1.1 ~FromSpatial~

Computes a ~jline~ in ~jnet~ from an given ~line~ value.

*/

 void FromSpatial (const JNetwork* jnet, const Line* in);

/*
1.1.1.1 ~ToSpatial~

Computes a corresponding spatial line value for the ~jline~.

*/
 void ToSpatial(Line& result) const;

/*
1.1.1.1 ~Intersects~

Returns true if at least one pair of intervals (one interval from each ~jline~)
intersects.

*/

bool Intersects(const JLine* other) const;

/*
1.1.1.1 ~Intersection~

The intersection is returned if exists otherwise a 0 pointer is returned.

*/

JRouteInterval* Intersection(const JRouteInterval& rint) const;

/*
1.1.1.1 ~Contains~

Returns true if the ~jpoint~ is inside the ~jline~, false otherwise.

*/

bool Contains(const JPoint* jp) const;

/*
1.1.1.1 ~Union~

Computes the union of the two given jline objects

*/

void Union(const JLine* other, JLine* result) const;

/*
1.1.1.1 ~GetBGP~

Returns the bounding ~jpoints~ of the  ~jline~.

*/

void GetBGP(JPoints* result) const;

private:

/*
1.1 private Deklaration part

1.1.1 Attributes

*/
  STRING_T nid; //network identifier
  DbArray<JRouteInterval> routeintervals; //sorted set of JRouteIntervals
  bool sorted; //true if routeintervals are sorted and compressed
  bool activBulkload; //only true while bulkload of routeintervals runs

/*

1.1.1 Default Constructor

The default constructor may only be used in the cast-function therefore we
declare it to be private.

*/

 JLine();

/*
1.1.1 Methods

1.1.1.1 IsSorted

Checks if the given set of JRouteIntervals is sorted.

*/

  bool IsSorted() const;

/*
1.1.1.1 Sort

Sorts the given set of RouteIntervals ascending by route Identifier,
StartPosition and EndPosition and reduces the number of route intervals if
possible.

*/

  void Sort();

/*
1.1.1.1 Append

Appends the intervals of the jline into the current jline.

*/

void Append(const JLine* other);

/*
1.1.1.1 FillIntervalList

Fills the list of route intervals from the given DbArray. Ignores
RouteIntervals which are not in the network.

*/

void FillIntervalList(const DbArray<JRouteInterval>* rintList,
                      const JNetwork* jnet);

/*
1.1.1.1 CheckAndFillIntervallList

Calls FillIntervalList if jnet is 0 the jnetwork is opened and closed around
the FillIntervalList call.

*/

void CheckAndFillIntervallList(const DbArray<JRouteInterval>* setri,
                                     const JNetwork* jnet /*= 0*/);

/*
1.1.1.1 Intersects

Returns true if the section intersects the jline, false elsewhere.

*/

bool Intersects(const int sid, const JNetwork* jnet) const;

/*
1.1.1.1 CheckAdjacent

Used as helper in getBGP checkst if the adjacentSections to currsection in
dir are all part of jline or not. If not the end of currSection in
dir is added to the Bounding JPoints of the jline.

*/

void CheckAdjacent(const SectionInterval& currSection,
                   const JNetwork* jnet,
                   const Direction dir,
                   JPoints* result) const;
};

} // end of namespace jnetwork

/*
1 Overwrite output operator

*/

using namespace jnetwork;
ostream& operator<<(ostream& os, const jnetwork::JLine& l);


#endif // JLINE_H
