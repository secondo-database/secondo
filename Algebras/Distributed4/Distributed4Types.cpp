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
#include "DPartition.h"

namespace distributed4 {
  using std::cerr;
  using std::runtime_error;
/*
This file defines a single type constructor: "dpartition"[1].

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
  ListExpr dpartitionProperty() {
    cerr << "debug(Property)" << endl;
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
        NList().stringAtom("(dpartition int)"),
        NList().stringAtom("(<partitioning map> <darrayname>)"),
        NList().stringAtom("(((39 2) (101 1)) \"Primes\")"),
        NList().textAtom("This type provides partitioning of values into "
          "slots found in a matching d[f]array. This makes it possible to use "
          "regular database operations on distributed data (insert, update, "
          "delete, etc.). Furthermore, this type provides the facilities "
          "required for dynamic reallocation of slots."),
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
  ListExpr dpartitionOut(ListExpr, Word value) {
    DPartition* dp{static_cast<DPartition*>(value.addr)};
    cerr << "debug(Out): " << NList{dp->listExpr()} << endl;
    return dp->listExpr();
  }
/*
4 In Function

The In function takes a nested list type expression, a nested list value
expression, and an integer position. It passes back a nested list error
expression, and a boolean correctness value. It returns a "Word"[1] containing
a pointer to an object instantiated based on the passed value expression.

*/
  Word dpartitionIn(const ListExpr typeInfo, const ListExpr instance, const
      int, ListExpr&, bool& correct) {
    NList ti{typeInfo}, inst{instance};
    cerr << "debug(In): " << ti << ", " << inst << endl;
    Word nothing{nullptr};
    correct = false;
/*
Make sure that the "typeInfo"[1] and "instance"[1] expressions fit together.
Specifically, check that the ~d[f]array~ named in "instance"[1] fits the
subtype. For a ~d[f]array~ over a ~rel(tuple)~ the subtype must be an attribute
that exists in the relation. For a ~darray~ over a container of simple types
the subtype must match the type of the values in the container.

"typeInfo"[1] is a bit enigmatic. When the object is created from a "[const ...
value ...]"[1] expression or with a "restore from ..."[1] command, it contains
both the basic type and any subtypes. This information is checked by checkType
before being processed here. When the object is opened from the database,
however, there will be no subtype, just the basic type. Just to keep things
interesting, the type is passed in a modified form: all mentioned type symbols
("dpartition"[1], "int"[1], "real"[1], etc.) are converted to pairs of algebra
and type ID.

This, the following cases need to be considered:

  * If "typeInfo"[1] is a pair of "int"[1]s, the object is being opened from
    the database and doesn't need to be (nor can it be) checked. 

  * Otherwise "typeInfo"[1] is a list containing the basic type (as first
    element) and the subtype (as second element).

  * If the subtype is a pair of "int"[1]s, the corresponding ~d[f]array~ must
    be over a container of a simple types given by the subtype.

  * Otherwise the subtype is a list containing an attribute name and type. The
    corresponding ~d[f]array~ must be over a "rel(tuple)"[1] containing a
    matching attribute.

First, check if the object is being opened from the database or if it is a new
object.

*/
    if(ti.length() != 2 || !ti.first().isInt() || !ti.second().isInt()) {
/*
If execution continues here, that means that the object is a new object and
needs to be checked against the named ~d[f]array~.

Check the named ~d[f]array~ for general consistency with ~dpartition~ objects.

*/
      SecondoCatalog* ctlg{SecondoSystem::GetCatalog()};
      NList darraytype{ctlg->GetObjectTypeExpr(inst.second().str())};
      if(darraytype.length() < 2 || (darraytype.first().str() != "darray" &&
            darraytype.first().str() != "dfarray") ||
          darraytype.second().length() < 2) {
        cmsg.inFunError("The named database object needs to be a d[f]array "
            "over a rel(tuple) or over a container of simple types.");
        return nothing;
      }
/*
Proceed with further checks depending on whether the subtype represents an
attribute name and type or a simple type.

*/
      NList darraysubtype{darraytype.second()};
      if(ti.second().second().isList())
      {
/*
The subtype represents an attribute name and type. Make sure the ~d[f]array~ is
over a "rel(tuple)"[1].

*/
        NList subtype{ti.second().first(),
          ctlg->GetTypeName(ti.second().second().first().intval(),
              ti.second().second().second().intval())};
        if(darraysubtype.first().str() != "rel" ||
            darraysubtype.second().length() < 2 ||
            darraysubtype.second().first().str() != "tuple" ||
            !darraysubtype.second().second().isList())
        {
          cmsg.inFunError("The type expression of the named d[f]array does "
              "not match the given subtype. It must be a d[f]array over a "
              "rel(tuple)");
          return nothing;
        }
/*
Make sure the "rel(tuple)"[1] has an attribute matching that of the subtype.

*/
        NList attrlist{darraytype.second().second().second()};
        Cardinal i{1};
        while(i <= attrlist.length() && subtype != attrlist.elem(i))
          ++i;
        if(i > attrlist.length()) {
          cmsg.inFunError("The attribute list of the d[f]array's rel(tuple) "
              "does not contain the attribute given in the subtype.");
          return nothing;
        }
      } else {
/*
Alternately, the subtype represents a simple type. Make sure the ~d[f]array~ is
over a container containing that type.

*/
        NList subtype{ctlg->GetTypeName(ti.second().first().intval(),
            ti.second().second().intval())};
        if(!darraysubtype.second().isSymbol() || darraysubtype.second() !=
            subtype) {
          cmsg.inFunError("The type expression of the named d[f]array does "
              "not match the given subtype. It must be a d[f]array over a "
              "container of the simple type given in the subtype.");
          return nothing;
        }
      }
    }
/*
Create the object in memory.

*/
    try {
      DPartition* addr{new DPartition{inst}};
      correct = true;
      return Word{addr};
    } catch(runtime_error& err) {
      cmsg.inFunError(err.what());
      return nothing;
    }
  }
/*
5 Create Function

The Create function takes a nested list type expression and returns a "Word"[1]
containing a pointer to a newly instantiated, but empty object.

*/
  Word dpartitionCreate(const ListExpr) {
    cerr << "debug(Create)" << endl;
    return Word{new DPartition};
  }
/*
6 Delete Function

The Delete function takes a nested list type expression and a "Word"[1]
containing a pointer to an object. It removes that object from memory and
returns nothing.

*/
  void dpartitionDelete(const ListExpr, Word& w) {
    cerr << "debug(Delete)" << endl;
    delete static_cast<DPartition*>(w.addr);
    w.addr = 0;
  }
/*
7 Clone Function

The Clone function takes a nested list type expression and a "Word"[1]
containing a pointer to an object. It creates a new object as a duplicate of
the passed object and returns its address in "Word"[1].

*/
  Word dpartitionClone(const ListExpr, const Word& w) {
    cerr << "debug(Clone)" << endl;
    return Word{new DPartition{*static_cast<DPartition*>(w.addr)}};
  }
/*
8 Size Function

The Size function simply returns the size of an object of the relevant type.

*/
  int dpartitionSize() {
    cerr << "debug(Size)" << endl;
    return sizeof(DPartition);
  }
/*
9 Type Constructor

The type constructor is instantiated here using the functions defined above.
Functions that the type constructor can take, but are not defined for a type
are passed as 0s. This type constructor is used by the algebra constructor.

*/
  TypeConstructor dpartitionTC{DPartition::BasicType(), dpartitionProperty,
    dpartitionOut, dpartitionIn, 0, 0, dpartitionCreate, dpartitionDelete, 0,
    0, dpartitionDelete, dpartitionClone, 0, dpartitionSize,
    DPartition::checkType};
}
