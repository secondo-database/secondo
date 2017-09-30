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

[10] Implementation of Class DPartition

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 Preliminary Setup

*/
#include "DPartition.h"
#include "../FText/FTextAlgebra.h"

namespace distributed4 {
  using std::cerr;
  using std::map;
  using std::ostream;
  using std::runtime_error;
  using std::string;
  using std::to_string;
  using std::vector;
/*
2 Constructors

2.1 "DPartition()"[1]

*WARNING: Do not use this constructor!* That means: never instantiate a
"DPartition"[1] without arguments.

The default constructor creates an instance of "DPartition"[1] with no link to
a ~d[f]array~ object. That is an invalid state. Therefore, this constructor
should not be used by any method that needs a valid "DPartition"[1] object. It
is provided for use by the "Create"[1] function that the query processor uses
when instantiating a query tree.

*/
  DPartition::DPartition() {}
/*
2.1 "DPartition(map, string)"[1]

The passed "map"[1] and "string"[1] are copied to the respective member
variables of the newly constructed "DPartition"[1].

*/
  DPartition::DPartition(const map<double,uint32_t>& p, const string& n):
    partitioning{p}, darrayname{n} {}
/*
2.2 "DPartition(ListExpr)"[1]

The passed "ListExpr"[1] is interpreted and used to fill all member variables.

*/
  DPartition::DPartition(const NList& list) {
    NList e;
/*
Validate the top-level structure of the nested list. The rest of the structure
will be validated further along.

*/
    if(list.length() != 2)
      throw runtime_error("The passed value needs to be a nested list with 2 "
          "elements.");
/*
Validate and read the partitioning map from the nested list.

*/
    NList pl{list.first()};
    if(!pl.isList())
      throw runtime_error("The first element in the passed nested list needs "
          "to be a nested list containing pairs.");
    for(Cardinal i{1}; i <= pl.length(); ++i) {
      e = pl.elem(i);
      if(e.length() != 2 || !(e.first().isReal() || e.first().isInt()) ||
          !e.second().isInt() || e.second().intval() <= 0)
        // The second number may not be 0 (or negative) because slot 0 is
        // inferred when there is no match in the map.
        throw runtime_error("Element " + to_string(i) + " of the first "
            "sublist needs to be a list containing a pair of atoms. The first "
            "atom needs to be any real or integer value, and the second a "
            "positive integer.");
      partitioning.emplace(e.first().isReal() ? e.first().realval() :
          e.first().intval(), e.second().intval());
    }
/*
Validate and read the slotbasename string from the nested list.

*/
    NList sl{list.second()};
    if(!sl.isSymbol())
      throw runtime_error("The second element in the passed nested list needs "
          "to be a symbol atom.");
    darrayname = sl.str();
  }
/*
3 Member Methods

3.1 "slot"[1]

The "partitioning"[1] map is applied to the passed key to determine to which
slot that key corresponds.

*/
  uint32_t DPartition::slot(double key) const {
/*
The upper\_bound method determines the next slot after the one being looked
for, as it looks for a key in the map that is greater than the key passed.

*/
    auto it{partitioning.upper_bound(key)};
/*
If the slot found happens to be the slot at the beginning of the map, that
means that the value passed in key is below all keys in the map.  That is all
slots whose left borders are defined. That means that the passed key is
somewhere between negative infinity and the first defined slot border. That is
always slot 0. In all other cases, the slot for the value passed in key is the
slot one before the slot found.

*/
    if(it == partitioning.begin()) {
      return 0;
    } else {
      return (--it)->second;
    }
  }
/*
3.2 "listExpr"[1]

The "DPartition" instance is converted into its nested list representation.

*/
  ListExpr DPartition::listExpr() const {
    NList pl, nl;
    for(auto it{partitioning.begin()}; it != partitioning.end(); ++it)
      pl.append(NList{NList{it->first}, NList{static_cast<int>(it->second)}});
    nl = NList{}.symbolAtom(darrayname);
    return NList{pl, nl}.listExpr();
  }
/*
3.3 "print"[1]

For easier debugging and/or output of the contents of a DPartition, this method
will push all details to a passed ostream. This makes it usable by
"operator<<"[1], which is also defined.

*/
  void DPartition::print(ostream& os) const {
    os << "DPartition " << partitioning << " with " << darrayname;
  }
/*
4 Static Methods

4.1 "BasicType"[1]

"BasicType"[1] is required by [secondo]. It returns [secondo]'s basic type for
this class.

*/
  string DPartition::BasicType() {
    return "dpartition";
  }
/*
4.2 "checkType"[1]

"checkType"[1] is required by [secondo] as well. It checks the passed ListExpr
to make sure that it is a valid type description for this class. It doesn't
only check the basic type (~dpartition~) but the complete type expression.

*/
  bool DPartition::checkType(ListExpr type, ListExpr&) {
    NList t{type};
    cerr << "debug(checkType): " << t << endl;
    const vector<string> supported{CcInt::BasicType(), CcReal::BasicType(),
      CcString::BasicType(), FText::BasicType()};
    string partitioning_type;
/*
A dpartition contains two elements, the basic type and a subtype.

*/
    if(t.length() != 2 || !t.first().isSymbol() || t.first().str() !=
        DPartition::BasicType()) {
      cmsg.typeError("The type expression needs to be a nested list beginning "
          "with the symbol " + DPartition::BasicType() + " followed by a "
          "symbol or a pair of symbols.");
      return false;
    }
/*
A ~dpartition~ for a ~d[f]array~ over "rel(tuple)"[1] specifies its subtype as
a pair indicating the attribute of the relation that is used in partitioning
and its type, i.e. "(dpartition (Code int))"[1]. A dpartition for a ~d[f]array~
over a container of simple types specifies its subtype as the type that is
contained in the ~d[f]array~, i.e. "(dpartition int)"[1].

*/
    NList subtype{t.second()};
    if(subtype.isList()) {
      if(subtype.length() != 2 || !subtype.first().isSymbol() ||
          !subtype.second().isSymbol()) {
        cmsg.typeError("The type expression looks like a " +
            DPartition::BasicType() + " for a d[f]array over a " +
            Relation::BasicType() + "(" + Tuple::BasicType() + "). In this "
            "case, the type expression needs to have an attribute of that " +
            Relation::BasicType() + "(" + Tuple::BasicType() + ") as its "
            "second element.");
        return false;
      }
      partitioning_type = subtype.second().str();
    } else {
      if(!subtype.isSymbol()) {
        cmsg.typeError("The type expression looks like a " +
            DPartition::BasicType() + " for a d[f]array over a container of "
            "simple types. In this case, the type expression needs to have "
            "that type as its second element.");
        return false;
      }
      partitioning_type = subtype.str();
    }
/*
The type or attribute that determines partitioning must be supported by
DPartition.

*/
    for(auto it{supported.begin()}; it != supported.end(); ++it)
      if(partitioning_type == *it)
        return true;
    cmsg.typeError("The type " + partitioning_type + " is not supported by " +
        DPartition::BasicType() + ".");
    return false;
  }
/*
5 Stream Operators Used by DPartition

5.1 "operator<<(ostream, map)"[1]

This operator allows formatting the pairs in a map for a character stream.

*/
  ostream& operator<<(ostream& os, const map<double,uint32_t>& m) {
    os << "{";
    for(auto it = m.begin(); it != m.end(); ++it) {
      if(it != m.begin()) os << ", ";
      os << it->first << " -> " << it->second;
    }
    os << "}";
    return os;
  }
/*
5.2 "operator<<(ostream, DPartition)"[1]

This operator allows something like the following:

---- DPartition dp;
     cerr << "debug: " << dp;
----

*/
  ostream& operator<<(ostream& os, const DPartition& dp) {
    dp.print(os);
    return os;
  }
}
