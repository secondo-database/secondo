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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Header file of the Temporal Algebra

March 2012, Fabio Valdes: File created for attribute data type SecInterval



[TOC]

1 Overview

This file contains the definition of the class SecInterval.

2 Defines and includes

*/

#ifndef TEMPORALUNITALGEBRA_H
#define TEMPORALUNITALGEBRA_H

#include "NestedList.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"
#include "Attribute.h"
#include "DateTime.h"

/*
3 The class ~SecInterval~

*/

class SecInterval : public Attribute, public Interval<Instant> {
public:

  SecInterval() {}

  explicit SecInterval(int dummy):Attribute(false) {}

  SecInterval(Instant s, Instant e): Attribute(true),
                                     Interval<Instant>(s, e, true, true) {}

  SecInterval(Instant s, Instant e, bool lc, bool rc):
                        Attribute(true), Interval<Instant>(s, e, lc, rc) {}

  static const string BasicType();

  static const bool checkType(const ListExpr type);

  static bool CheckKind(ListExpr type, ListExpr& errorInfo);

  size_t Sizeof() const;

  int Compare(const Attribute* attr) const;

  bool Adjacent(const Attribute* attr) const;

  SecInterval* Clone() const;

  size_t HashValue() const;

  void CopyFrom(const Attribute *attr);

  void WriteTo(char *dest);

  string ToString();

  ListExpr ToListExpr(const ListExpr typeInfo) const;

  SmiSize SizeOfChars();

  bool ReadFrom(const ListExpr instance, const ListExpr typeInfo);

  bool Set(const DateTime& s, const DateTime& e, const bool lc, const bool rc);

  bool Set(const Interval<Instant>* iinst);

  static ListExpr Property();

  bool IsDefined() const{
    return Attribute::IsDefined();
  }

  const bool Contains(const Interval<Instant>& si) const;

  const bool Contains(const Periods& per)const;
};

#endif

