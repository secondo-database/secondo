/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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
----

//paragraph [1] title: [{\Large \bf ]  [}]


[1] TupleIdentifier Algebra

March 2005 Matthias Zielke

The only purpose of this algebra is to provide a typeconstructor 'tid' so that the tupleidentifiers
of tuples from relations can be stored as attributvalues in different tuples. This feature is needed
for the implementation of operators to update relations.

1 Preliminaries

1.1 Includes

*/

#ifndef TUPLEIDENTIFIER_H
#define TUPLEIDENTIFIER_H


#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include <string>

using namespace std;

extern NestedList* nl;
extern QueryProcessor *qp;

typedef SmiRecordId TupleId;

/*
2 Type Constructor ~tid~

2.1 Data Structure - Class ~TupleIdentifier~

*/
class TupleIdentifier: public Attribute
{
 public:
  TupleIdentifier( bool DEFINED, TupleId TID = 0 );
  TupleIdentifier(const TupleIdentifier& source);
  inline TupleIdentifier() {};
/*
This constructor should not be used.

*/
  ~TupleIdentifier();
  TupleId      GetTid() const;
  void     SetTid( const TupleId tid);
  TupleIdentifier*   Clone() const;
  ostream& Print( ostream& os ) const;

  inline void Set(const bool DEFINED, const TupleId ID)
  {
    SetDefined( DEFINED);
    this->tid = static_cast<long>(ID);
  }

  inline size_t Sizeof() const
  {
    return sizeof( *this );
  }

  inline size_t HashValue() const
  {
    return (IsDefined() ? tid : 0);
  }

  void CopyFrom(const Attribute* right);

  inline int Compare(const Attribute *arg) const
  {
    const TupleIdentifier* tupleI = (const TupleIdentifier*)(arg);
    return Compare(*tupleI);
  }

  inline int Compare(const TupleIdentifier& t) const
  {
    bool argDefined = t.IsDefined();
    if(!IsDefined() && !argDefined)
      return 0;
    if(!IsDefined())
      return -1;
    if(!argDefined)
      return 1;
    if ( tid < t.GetTid() )
      return (-1);
    if ( tid > t.GetTid())
      return (1);
    return (0);
  }

  bool Adjacent(const Attribute *arg) const;

  static void* Cast(void* addr){
    return new (addr) TupleIdentifier();
  }

/*
Additional functions for integration in ~jlist~

*/

static ListExpr Out(ListExpr typeInfo, Word value);
static Word In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct);
static bool Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value );
static bool Open(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value );
TupleIdentifier& operator=(const TupleIdentifier& other);
bool operator==(const TupleIdentifier& other) const;

/*
Basic Type

*/

static const string BasicType(){
     return "tid";
  }

static const bool checkType(const ListExpr type){
    return listutils::isSymbol(type, BasicType());
  }

/*
Example String for external Property Files includin TupleIds

*/

static const string Example(){
  return "50060";
}

 private:
  TupleId tid;

};


#endif
