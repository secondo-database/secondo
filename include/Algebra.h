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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]

1 Header File: Algebra

May 2002 Ulrich Telle Port to C++

August 2002 Ulrich Telle Changed ~PersistValue~ and ~PersistModel~ interface
using nested lists for the type instead of the string representation.

January 2003 Victor Almeida Changed ~PersistValue~ interface and created three
new functions ~Open~, ~Save~, and ~Close~ that join the ~Create~ and ~Delete~
to form the object state diagram in the Figure 1.

Figure 1: Object state diagram [objstatediagram.eps]

March 2003 Victor Almeida created the function "SizeOf".

Oct 2004 M. Spiekermann removed the friend relationship between class 
AlgebraManager and class Operator. Additonally some functions are declared
as inline functions.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006, M. Spiekermann. New constructors for class ~Operator~ and ~TypeConstructor~
added. They assume arguments stored in the structs ~OperatorInfo~ and ~ConstructorInfo~.
Moreover, it is possible to use defaults for many functions needed by a type constructor
which are defined as template functions in ConstructorTemplates.h.

April 2006, M. Spiekermann. Due to changes in the template instantiation of GCC versions
higher than 3.2.3 the declarations in this file must be splitted in order to aviod a circular
dependency between Algebra.h and AlgebraManager.h. Now the declarations of these files are distributed
over the files Operators.h, TypeConstructor.h, AlgebraClassDef.h and AlgebraInit.h.

This file declares the programming interface for algebra modules and must be included by
every algebra implementation. 

1.1 Overview

A snapshot of a working "Secondo"[3] system will show a collection of algebras,
each of them ``populated'' by two different kinds of entities: objects and
operators.
Operators are fix in terms of number and functionality which
are defined by the algebra implementor. In contrast to that, the number of
objects is variable and changes dynamically at runtime. Even the types of
objects are not predefined, but only their type constructors.

These very different properties of types and objects give rise to a very
different C++ representation of  operators and objects:

  * Operators are instances of a predefined class ~Operator~. Thus an
implementation  of an algebra with "n"[2] operators contains "n"[2] definitions
of instances of class ~Operator~.

  * Objects cannot be predefined, because they are constructed and deleted at
runtime. Even the possible types of objects cannot be predeclared, because they
can be declared at runtime, too. Only an algebras's  ~type constructors~ are
well known and fix. An implementation of an algebra with "m"[2]
type constructors contains "m"[2] definitions of instances of the predefined
class ~TypeConstructor~.

From a top level point of view, a "Secondo"[3] universe is a collection of algebras.
This can be implemented by defining an instance of a subclass
of the predefined class ~Algebra~ for
each existing algebra. Each of these ~Algebra~
instances essentially consists of a set of operators and a set of
type constructors.

1.2 Defines and Includes

*/

#ifndef ALGEBRA_H
#define ALGEBRA_H

#include <string>
#include <vector>

#include "NestedList.h"
#include "AlgebraTypes.h"
#include "Operator.h"
#include "TypeConstructor.h"
#include "AlgebraClassDef.h"

#endif

