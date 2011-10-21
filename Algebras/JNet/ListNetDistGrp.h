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

2011, May Simone Jandt

*/

#ifndef LISTNETDISTGRP_H
#define LISTNETDISTGRP_H

#include <ostream>
#include "../../Tools/Flob/DbArray.h"
#include "Attribute.h"
#include "NetDistanceGroup.h"

/*
1. class ~ListNetDistGrp~
Enables us to use a list of ~ListListNetDistGrp~s as attributes in relations.

*/

class ListNetDistGrp: public Attribute
{

public:

/*
1.1. Constructors and deconstructors

The default constructor should only be used in the cast-Function.

*/

ListNetDistGrp();
ListNetDistGrp(bool defined);
ListNetDistGrp(const ListNetDistGrp& other);
ListNetDistGrp(const NetDistanceGroup& ndg);

~ListNetDistGrp();

/*
1.2 Getter and Setter for private Attributes

*/

DbArray<NetDistanceGroup> GetList() const;

void SetList(const DbArray<NetDistanceGroup> inList);

/*
1.3 Override Methods from Attribute

*/

void CopyFrom(const Attribute* right);
StorageType GetStorageType() const;
 size_t HashValue() const;
 Attribute* Clone() const;
 bool Adjacent(const Attribute* attrib) const;
 int Compare(const Attribute* rhs) const;
 int Compare(const ListNetDistGrp& in) const;
 int NumOfFLOBs()const;
 Flob* GetFLOB(const int n);
 void Destroy();
 size_t Sizeof() const;
 ostream& Print(ostream& os) const;
 static const string BasicType();
 static const bool checkType(const ListExpr type);

/*
1.4 Standard Methods

*/

  ListNetDistGrp& operator=(const ListNetDistGrp& other);
  bool operator==(const ListNetDistGrp& other) const;

/*
1.5 Operators for Secondo Integration

*/

  static ListExpr Out(ListExpr typeInfo, Word value);
  static Word In(const ListExpr typeInfo, const ListExpr instance,
                 const int errorPos, ListExpr& errorInfo, bool& correct);
  static Word Create(const ListExpr typeInfo);
  static void Delete( const ListExpr typeInfo, Word& w );
  static void Close( const ListExpr typeInfo, Word& w );
  static Word Clone( const ListExpr typeInfo, const Word& w );
  static void* Cast( void* addr );
  static bool KindCheck( ListExpr type, ListExpr& errorInfo );
  static int SizeOf();
  static bool Save(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value );
  static bool Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value );
  static ListExpr Property();

/*
1.6 Helpful Operators

*/

  void Append (const NetDistanceGroup& e);
  int GetNoOfComponents() const;
  void Get(const int i, NetDistanceGroup& e) const;
  void Put(const int i, NetDistanceGroup& e);

private:

  DbArray<NetDistanceGroup> elemlist;

};

#endif // LISTNETDISTGRP_H
