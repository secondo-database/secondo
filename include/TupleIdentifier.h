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

//paragraph [1] title: [{\Large \bf ]	[}]


[1] TupleIdentifier Algebra

March 2005 Matthias Zielke

The only purpose of this algebra is to provide a typeconstructor 'tid' so that the tupleidentifiers
of tuples from relations can be stored as attributvalues in different tuples. This feature is needed
for the implementation of operators to update relations.

1 Preliminaries

1.1 Includes

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"	
#include <string>

extern NestedList* nl;
extern QueryProcessor *qp;

typedef long TupleId;

/*
2 Type Constructor ~tid~

2.1 Data Structure - Class ~TupleIdentifier~

*/
class TupleIdentifier: public StandardAttribute
{
 public:
  inline TupleIdentifier() {};
	/*
	This constructor should not be used.
	*/
  TupleIdentifier( bool DEFINED, TupleId TID );
  ~TupleIdentifier();
  TupleId      GetTid();
  void     SetTid( TupleId tid);
  TupleIdentifier*   Clone();
  int NumOfFLOBs();
   inline bool IsDefined() const 
  { 
    return (defined); 
  }
  
  inline void SetDefined(bool DEFINED) 
  { 
    this->defined = DEFINED;
  }
    
  inline size_t HashValue()
  { 
    return (defined ? tid : 0); 
  }
  
  void CopyFrom(StandardAttribute* right);
  
  inline int Compare(Attribute *arg)
  {
    assert(arg);
    TupleIdentifier* tupleI = (TupleIdentifier*)(arg);
    bool argDefined = tupleI->IsDefined();
    if(!defined && !argDefined) {
      return 0;
    }
    if(!defined) {
      return -1;
    }
    if(!argDefined) {
      return 1;
    }

    if ( tid < tupleI->GetTid() ) {
      return (-1);
    }  
    if ( tid > tupleI->GetTid()) {
      return (1);
    }  
    return (0);
  }

  bool Adjacent(Attribute *arg);
  
 private:
  long tid;
  bool defined;
};
