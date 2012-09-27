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

#ifndef MJPOINT_H
#define MJPOINT_H

#include <ostream>
#include <string>
#include "Attribute.h"
#include "StandardTypes.h"
#include "DateTime.h"
#include "TemporalAlgebra.h"
#include "UJPoint.h"
#include "JNetwork.h"
#include "JLine.h"

/*
1 class ~MJPoint~

A ~MJPoint~ describes the way of an JPoint through the JNetwork. It consist of
an network id, and a set of defined ~UJPoint~ values. Sorted by their time
intervals.

*/

class MJPoint : public Attribute
{

/*
1.1 Public Declarations

*/

public:

/*
1.1.1 Constructors and Deconstructors

*/

  explicit MJPoint(const bool def);
  explicit MJPoint(const MJPoint& other);
  explicit MJPoint(const string netid, const DbArray<JUnit>& upoints);
  explicit MJPoint(const UJPoint* u);

  ~MJPoint();

/*
1.1.1 Getter and Setter for private Attributes

*/

  const STRING_T* GetNetworkId() const;
  const DbArray<JUnit>& GetUnits() const;

  void SetNetworkId(const STRING_T& id);
  void SetUnits(const DbArray<JUnit>& junits);

/*
1.1.1 Override Methods from Attribute

*/

  void CopyFrom(const Attribute* right);
  StorageType GetStorageType() const;
  size_t HashValue() const;
  MJPoint* Clone() const;
  bool Adjacent(const Attribute* attrib) const;
  static int Compare(const void* ls, const void* rs);
  int Compare(const Attribute* rhs) const;
  int Compare(const MJPoint& rhs) const;
  size_t Sizeof() const;
  int NumOfFLOBs() const;
  Flob* GetFLOB(const int i);
  void Destroy();
  void Clear();
  ostream& Print(ostream& os) const;
  static const string BasicType();
  static const bool checkType(const ListExpr type);

/*
1.1.1 Standard Operators

*/

  MJPoint& operator=(const MJPoint& other);

  bool operator==(const MJPoint& other) const;
  bool operator!=(const MJPoint& other) const;
  bool operator<(const MJPoint& other) const;
  bool operator<=(const MJPoint& other) const;
  bool operator>(const MJPoint& other) const;
  bool operator>=(const MJPoint& other) const;

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

Returns an example mjpoint listrepresentation as string.

*/

  static string Example();

/*
1.1.1.1 GetNoComponents

Returns the number of ~junit~s of the ~mjpoint~.

*/

  int GetNoComponents() const;

/*
1.1.1.1 IsEmpty

Returns false if the ~mjpoint~ has at least one defined junit, false elsewhere.

*/

  bool IsEmpty() const;

/*
1.1.1.1 Get

Returns the ~junit~ at the given position.

*/

  void Get(const int i, JUnit& up) const;
  void Get(const int i, JUnit* up) const;
  void Get(const int i, UJPoint& up) const;

/*
1.1.1.1 FromSpatial

Returns the corresponding ~mjpoint~ in ~jnet~ for the given ~mpoint~.

*/
  void FromSpatial(JNetwork* jnet, const MPoint* in);

/*
1.1.1.1 Trajectory

Returns an ~jline~ value representing all network position ever passed by the
~mjpoint~.

*/

void Trajectory(JLine* result) const;

/*
1.1.1.1 BoundingBox

Returns an 3 dimensional rectangle with the spatial temporal bounding box of
the mjpoint.

*/

Rectangle<3> BoundingBox() const;

/*
1.1.1 Manage Bullkload

*/

  void StartBulkload();
  void EndBulkload();
  MJPoint& Add(const JUnit& up);

/*
1.1 Private declarations

*/

private:

/*
1.1.1 Attributes

*/

  STRING_T nid;         //network identifier
  DbArray<JUnit> units; //set of JUnit describing the way of the mjpoint
  bool activBulkload; //only true while bulkload of ujpoints runs

/*
1.1.1 Standard Constructor

The standard constructor should only be used in cast-Function and is therefore
declared to be private.

*/

  MJPoint();

/*
1.1.1 ~CheckSorted~

Checks if the units are well sorted. Used if complete Arrays are inserted.

*/

bool CheckSorted() const;

/*
1.1.1 ~Simplify~

Checks if the units are sorted and compresses the units as far as possible.

*/

bool Simplify();

/*
1.1.1 Append

Appends the units of the in MJPoint to the current mjpoint.

*/

void Append(const MJPoint* in);

};
/*
1 Overwrite output operator

*/

ostream& operator<< (const ostream& os, const MJPoint& jp);

#endif // MJPOINT_H
