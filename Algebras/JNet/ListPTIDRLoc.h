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

#ifndef LISTPTIDRLOC_H
#define LISTPTIDRLOC_H

#include <ostream>
#include "../../Tools/Flob/DbArray.h"
#include "Attribute.h"
//#include "PairTIDRLoc.h"
#include "PairTID.h"

/*
1. class ~ListPTIDRLoc~
Enables us to use a list of ~PairTIDRLoc~s as attributes in relations.

*/

class ListPTIDRLoc: public Attribute
{

public:

/*
1.1. Constructors and deconstructors

The default constructor should only be used in the cast-Function.

*/

ListPTIDRLoc();
ListPTIDRLoc(bool defined);
ListPTIDRLoc(const ListPTIDRLoc& other);
ListPTIDRLoc(const PairTIDRLoc& ptidrloc);

~ListPTIDRLoc();

/*
1.2 Getter and Setter for private Attributes

*/

DbArray<PairTIDRLoc> GetList() const;

void SetList(const DbArray<PairTIDRLoc> inList);

/*
1.3 Override Methods from Attribute

*/

void CopyFrom(const Attribute* right);
StorageType GetStorageType() const;
 size_t HashValue() const;
 Attribute* Clone() const;
 bool Adjacent(const Attribute* attrib) const;
 int Compare(const Attribute* rhs) const;
 int Compare(const ListPTIDRLoc& in) const;
 int NumOfFLOBs () const;
 Flob* GetFLOB(const int n);
 void Destroy();
 size_t Sizeof() const;
 ostream& Print(ostream& os) const;
 static const string BasicType();
 static const bool checkType(const ListExpr type);
/*
1.4 Standard Methods

*/

  ListPTIDRLoc& operator=(const ListPTIDRLoc& other);
  bool operator==(const ListPTIDRLoc& other) const;

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

  void Append (const PairTIDRLoc& e);
  int GetNoOfComponents() const;
  void Get(const int i, PairTIDRLoc& res) const;
  void Put(const int i, const PairTIDRLoc& p);

private:

  DbArray<PairTIDRLoc> elemlist;
};

#endif // LISTPTIDRLOC_H
