/*
----
This file is part of SECONDO.

Copyright (C) 2017, Faculty of Mathematics and Computer Science, Database
Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

SECONDO is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
SECONDO; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [10] title: [{\Large \bf] [}]
//characters [1] tt: [\texttt{] [}]
//[secondo] [{\sc Secondo}]

[10] Types of Algebra Distributed4

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 Preliminary Setup

*/
#include "DTable.h"

namespace distributed4 {
/*
This file defines a single type constructor: "dtable"[1].

2 Type Description

This is a function that returns information about the type constructor to the
user. The function returns a nested list containing two sublists of the same
length. The first sublist has the labels for the descriptions in the second
sublist.

There is an alternative (and newer) method to describe type constructors: the
class "ConstructorInfo"[1]. While it is more straightforward to use as the
property function, using it when instantiating the type constructor also
requires use of the class template "ConstructorFunctions"[1]. I prefer,
however, to pass the functions to "TypeConstructor"[1] directly and therefore
chose to forego using "ConstructorInfo"[1] here.

*/
  ListExpr dtableProperty() {
    return NList{
      NList{
        NList().stringAtom("Signature"),
        NList().stringAtom("Example Type List"),
        NList().stringAtom("List Rep"),
        NList().stringAtom("Example List"),
        NList().stringAtom("Remarks"),
      },
      NList{
        NList().stringAtom("-> SIMPLE"),
        NList().stringAtom("(dtable int)"),
        NList().textAtom("(<partitioning map> <allocation list> <worker list> "
          "<slotbasename>)"),
        NList().textAtom("(((39 2) (101 1)) (0 1 0) (('s0' 1234 'cfg.ini') "
          "('s1' 1234 'cfg.ini')) \"Primes\")"),
        NList().textAtom("This type extends the concept of darrays to make it "
          "possible to use regular database operations on distributed data "
          "(insert, update, delete, etc.). Furthermore, this type provides "
          "the facilities required for dynamic reallocation of slots."),
      }
    }.listExpr();
  }
/*
3 Out Function

The Out function takes a nested list type expression and a "Word"[1] containing
a pointer to an instance of that type as arguments. It returns the value
represented by the instance as a nested list value expression. This is done in
the form of a "ListExpr"[1]. The passed type expression is commonly ignored.

*/
  ListExpr dtableOut(ListExpr, Word value) {
    DTable* ds{static_cast<DTable*>(value.addr)};
    return ds->listExpr();
  }
/*
4 In Function

The In function takes a nested list type expression, a nested list value
expression, and an integer position. It passes back a nested list error
expression, and a boolean correctness value. It returns a "Word"[1] containing
a pointer to an object instantiated based on the passed value expression.

*/
  Word dtableIn(const ListExpr, const ListExpr instance, const int, ListExpr&,
      bool& correct) {
    DTable* addr;
    try {
      addr = new DTable{instance};
      correct = true;
    } catch(std::runtime_error& err) {
      addr = 0;
      correct = false;
      cmsg.inFunError(err.what());
    }
    return Word{addr};
  }
/*
5 Create Function

The Create function takes a nested list type expression and returns a "Word"[1]
containing a pointer to a newly instantiated, but empty object.

*/
  Word dtableCreate(const ListExpr) {
    return Word{new DTable};
  }
/*
6 Delete Function

The Delete function takes a nested list type expression and a "Word"[1]
containing a pointer to an object. It removes that object from memory and
returns nothing.

*/
  void dtableDelete(const ListExpr, Word& w) {
    delete static_cast<DTable*>(w.addr);
    w.addr = 0;
  }
/*
7 Clone Function

The Clone function takes a nested list type expression and a "Word"[1]
containing a pointer to an object. It creates a new object as a duplicate of
the passed object and returns its address in "Word"[1].

*/
  Word dtableClone(const ListExpr, const Word& w) {
    return Word{new DTable{*static_cast<DTable*>(w.addr)}};
  }
/*
8 Size Function

The Size function simply returns the size of an object of the relevant type.

*/
  int dtableSize() {
    return sizeof(DTable);
  }
/*
9 Type Constructor

The type constructor is instantiated here using the functions defined above.
Functions that the type constructor can take, but are not defined for a type
are passed as 0s. This type constructor is used by the algebra constructor.

*/
  TypeConstructor dtableTC{DTable::BasicType(), dtableProperty, dtableOut,
    dtableIn, 0, 0, dtableCreate, dtableDelete, 0, 0, dtableDelete,
    dtableClone, 0, dtableSize, DTable::checkType};
}
