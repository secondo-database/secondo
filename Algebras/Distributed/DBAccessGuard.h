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

//[_] [\_]

*/

/*
[1] TupleBufferQueue

Oktober 2011 Thomas Achmann

DBAccessGuard implements a threadsave access to the database. Most of
the DB functionality is not threadsave. Therefor the access to the
database by certain functions must be guarded.

This class is implemented using the singelton pattern

*/

#ifndef _DBACCESSGUARD_H_
#define _DBACCESSGUARD_H_

// uncomment this line to use ZThread::FastMutex
#define MYFASTMUTEX 1

#if defined MYFASTMUTEX
#include "zthread/FastMutex.h"
typedef ZThread::FastMutex MyMutex;
#else
#include "zthread/Mutex.h"
typedef ZThread:Mutex MyMutex;
#endif

#include "RelationAlgebra.h"
#include "NestedList.h"
#include "zthread/Guard.h"

class DBAccessGuard
{
private:
  // Singelton Pattern: c-tors are private
  DBAccessGuard() {}
  DBAccessGuard(const DBAccessGuard&) {}
  DBAccessGuard &operator= (const DBAccessGuard&) {return *this;}

  MyMutex lock;
 
public:
  virtual ~DBAccessGuard() {}
  static DBAccessGuard *getInstance();

/*

2 TupleType Access

*/
/*

2.1 TT[_]New[_]2

returns TupleType - ptr
Allocates a new object of TupleType

*/

  TupleType* TT_New_2(const ListExpr& inTT_Rel)
  {
    ZThread::Guard<MyMutex> g(lock);
    cout << "NEWTT:" << nl -> ToString(nl -> Second(inTT_Rel));
    return new TupleType(nl -> Second(inTT_Rel));
  }
/*

2.2 TT[_]DeleteIfAllowed

deletes a tuple

*/
  
  void TT_DeleteIfAllowed(TupleType* tt)
  {
    ZThread::Guard<MyMutex> g(lock);
    tt-> DeleteIfAllowed();
  }
/*

3 Tuple Access

3.1 T[_]WriteToBin

restores a Tuple from binary representation

*/
  void T_WriteToBin(Tuple* t, char* buffer,
                    size_t cS,
                    size_t eS,
                    size_t fS)
  {
    ZThread::Guard<MyMutex> g(lock);
    t -> WriteToBin(buffer,cS,eS,fS);
  }
/*

3.2 T[_]ReadFromBin

restores a Tuple from binary representation

*/
  void T_ReadFromBin(Tuple* t, char* buffer)
  {
    ZThread::Guard<MyMutex> g(lock);
    t-> ReadFromBin(buffer);
  }
/*

3.3 T[_]DeleteIfAllowed

deletes a tuple

*/
  void T_DeleteIfAllowed(Tuple* t)
  {
    ZThread::Guard<MyMutex> g(lock);
    t-> DeleteIfAllowed();
  }

/*

4 TupleBuffer Access

4.1 TB[_]Clear

clears a TupleBuffer

*/
  void TB_Clear(TupleBuffer *tb)
  {
    ZThread::Guard<MyMutex> g(lock);
    tb -> Clear();
  }

/*

5 TupleBufferIterator Access

5.1 TBI[_]GetNextTuple

returns Tuple - ptr
Retruns the next tuple of an iterator

*/ 

  Tuple* TBI_GetNextTuple(TupleBufferIterator *tbi)
  {
    ZThread::Guard<MyMutex> g(lock);
    return tbi -> GetNextTuple();
  }

/*

6 GenericRelation Access

6.1 REL[_]AppendTuple

*/
  void REL_AppendTuple(GenericRelation *rel, Tuple* t)
  {
    ZThread::Guard<MyMutex> g(lock);
    rel -> AppendTuple(t);
  }
/*

7 NestedList Access

7.1 NL[_]ToString

*/
  void NL_ToString(const ListExpr &l, string& out)
  {
    ZThread::Guard<MyMutex> g(lock);
    out = nl -> ToString(l);
  }
};

typedef DBAccessGuard DBAccess;

#endif // _DBACCESSGUARD_H_
