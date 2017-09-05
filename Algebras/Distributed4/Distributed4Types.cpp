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
#include "DStruct.h"

namespace distributed4 {
  ListExpr dstructProperty() {
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
        NList().stringAtom("(dstruct int)"),
        NList().textAtom("(<partitioning map> <allocation list> <worker list> "
          "slotbasename)"),
        NList().textAtom("(((39 2) (101 1)) (0 1 0) ((\"s0\" 1234 0 "
          "\"cfg.ini\") (\"s1\" 1234 1 \"cfg.ini\")) Primes)"),
        NList().textAtom("This type extends the concept of darrays to make it "
          "possible to use regular database operations on distributed data "
          "(insert, update, delete, etc.). Furthermore, this type provides "
          "the facilities required for dynamic reallocation of slots."),
      }
    }.listExpr();
  }

  ListExpr dstructOut(ListExpr, Word value) {
    DStruct* ds{static_cast<DStruct*>(value.addr)};
    return ds->listExpr();
  }

  Word dstructIn(const ListExpr, const ListExpr instance, const int, ListExpr&,
      bool& correct) {
    DStruct* addr;
    try {
      addr = new DStruct{instance};
      correct = true;
    } catch(std::runtime_error&) {
      addr = 0;
      correct = false;
    }
    return Word{addr};
  }

  Word dstructCreate(const ListExpr) {
    return Word{new DStruct};
  }

  void dstructDelete(const ListExpr, Word& w) {
    delete static_cast<DStruct*>(w.addr);
    w.addr = 0;
  }

  Word dstructClone(const ListExpr, const Word& w) {
    return Word{new DStruct{*static_cast<DStruct*>(w.addr)}};
  }

  int dstructSize() {
    return sizeof(DStruct);
  }

  TypeConstructor dstructTC{DStruct::BasicType(), dstructProperty, dstructOut,
    dstructIn, 0, 0, dstructCreate, dstructDelete, 0, 0, dstructDelete,
    dstructClone, 0, dstructSize, DStruct::checkType};
}

//Notes:
//
//DStruct for a DArray of a rel
//(
//  dstruct
//  (
//    (Code int)
//    (
//      rel
//      (
//        tuple
//        (
//          (Osm_id string)
//          (Code int)
//          ...
//        )
//      )
//    )
//  )
//)
//
//DStruct for a DArray of an atomic type
//(
//  dstruct
//  int
//)
