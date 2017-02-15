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

*/

#ifndef ALGEBRAS_NESTEDRELATION2_NR2AHELPER_H_
#define ALGEBRAS_NESTEDRELATION2_NR2AHELPER_H_

#include <string>
#include "Algebra.h"

using namespace std;

namespace nr2a {

/*
Some methods used by various classes inside the algebra are gathered in this
class for general access.

*/
class Nr2aHelper
{
  public:

    static ListExpr TupleStreamOf(ListExpr attributesTypes);
    static ListExpr TupleOf(ListExpr attributesTypes);
    static ListExpr RecordOf(ListExpr attributesTypes);

    static bool IsNestedRelation(const ListExpr definition);
    static bool IsNumericRepresentationOf(const ListExpr list,
        const string typeName);

    static int DefaultSelect(const ListExpr type);

    static string IntToString(const int num);

    static double MillisecondsElapsedSince(clock_t previousClock);

};

/*
To simplify building larger structures in nested lists format the class
"ListBuilder"[2] provides an easy interface to create such step by step.
After defining the content the class' getters can be used to create several
types containing the content as subtypes.

*/
class ListBuilder
{
  public:
    ListBuilder();
    void Append(const ListExpr newElement);
    void AppendAttribute(const string attributeName,
        const ListExpr type);
    void AppendAttribute(const string attributeName,
        const string typeName);

    ListExpr GetList() const;
    ListExpr GetTuple() const;
    ListExpr GetRecord() const;
    ListExpr GetARel() const;
    ListExpr GetNRel() const;
    ListExpr GetTupleStream() const;

  private:
    ListExpr m_list;
    ListExpr m_end;
};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_NR2AHELPER_H_*/
