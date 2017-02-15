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

//paragraph   [1] title:       [{\Large \bf ]         [}]
//characters  [2] verbatim:    [\verb@]               [@]
//paragraph   [3] code:        [\begin{verbatim}]     [\end{verbatim}]
//paragraph   [4] parsedCode:  [{\tt ]                [}]
//characters  [5] superscript: [\textsuperscript{]    [}]
//characters  [9] quoted:      [``]                   ['']


//[ue]                         [\"{u}]
//[toc]                        [\tableofcontents]

[1] NestedRelation2 Algebra
2016-2017, Thomas Peredery

[toc]

1 Preamble

The documentation to this algebra is build by concatenating all source files
before using the PD system. This can be done by the "make_pd.sh"[2] or
"make_pd.bat"[2] script.

1 Overview

The NestedRelation2 algebra is an algebra for the extensible database
management system SECONDO, which is maintained by the University of Hagen. It
offers type constructors and operators to work with nested relations. Their
usage equals those of the older NestedRelation algebra, but the storage system
underlying the newer algebra is no longer based on separate relations, but on
so called "Faked Large OBjects"[9] (FLOBs) managed by SECONDO. The import of the
DBLP literature database of the university of Trier is no longer a Java program
but an operator of the algebra.

1 Structure

The types and operators made available in Secondo are implemented in their own
classes each. The main classes and the files they are in are named after the
operator or datatype they are implementing. For example the type "arel"[2] is
implemented by the class "ARel"[2] in "ARel.h"[2] and "ARel.cpp"[2]. The
operator "dblpimport"[2] is implemented by several classes. Its main class is
"DblpImport"[2] defined in "DblpImport.h"[2] and "DblpImport.cpp"[2].

To avoid name clashes each of these classes is defined in a namespace named
"nr2a"[2].

1 Preliminaries

The algebra depends on the following other algebras:

  * Relation algebra (NRel stores data of top level in a relation internally)

  * Stream algebra

  * FText algebra

The DBLP import uses libxml which is assumed to be installed and configured
properly on the system running SECONDO.

To simplify the management of the header files in each header file special
defines are used to avoid multiple includes of declarations.

*/

#ifndef ALGEBRAS_NESTEDRELATION2_NESTEDRELATION2ALGEBRA_H_
#define ALGEBRAS_NESTEDRELATION2_NESTEDRELATION2ALGEBRA_H_

#include "Algebra.h"
#include <string>

/*
The "AutoWrite()"[2] macro is used for debugging any expressions, especially
nested lists. It outputs several information:

  * The expression, which is evaluated, as it is written in the code

  * The file, line number and function name where the macro is called

  * The evaluated value

[3] AutoWrite(3+2);

will output:

[4] "a+b"[9] in "int Sum(int a, int b)"[9] (Line "32"[9] of "Calc.cpp"[9])
5

assuming "a==3"[2] and "b==2"[2]

The precompiler switch "DOAUTOWRITE"[2] is used to change the behavior of the
macro and avoid its output. This prevents slowing down processing and reduces
noisy output. Notice, that the expression itself is still evaluated and any side
effects occur, too. This feature avoids unexpected behavior if the macro
is used like:

[3] AutoWrite(b++);

It is possible to wrap it into another construct to avoid its execution.

[3] #ifdef DO_COMPLEX_CHECKS
AutoWrite(largeObject.ComplexCalculation());
#endif

Accordingly "WriteFlob"[2] can be used to output a "Flob"[2]s content. It takes
the "offset"[2] where to start and the maximum "size"[2] to output.

*/

//#define DOAUTOWRITE
#ifdef DOAUTOWRITE
#define AutoWrite(expr)\
  {\
  char bufferLine[20];\
  sprintf(bufferLine, "%d", __LINE__);\
  nr2a::Write(expr, string("\"") + #expr + "\" in \"" + __PRETTY_FUNCTION__ +\
      "\" (Line " + bufferLine + " of " + __FILE__ + ")");\
  }
#define WriteFlob(expr, offset, size)\
  {\
  AutoWrite(expr)\
  nr2a::Write(expr, offset, size);\
  }
#else
#define AutoWrite(expr) (void)expr;
#define WriteFlob(expr, offset, size) (void)expr;
#endif //DOAUTOWRITE

/*
To iterate nested lists the "listForeach"[2] macro is implemented. Given a
nested list to iterate and an name for a variable to use as reference to the
element of the current iteration. An example for its usage is:

[3] listForeach(typeList, current)
cout << nl->StringValue(current).cstr() << endl;

*/

#define listForeach(list, elem)\
    for(ListExpr elem=nl->TheEmptyList(), elem##_iter=list;\
    nl->IsEmpty(elem##_iter)?false:(elem = nl->First(elem##_iter),true);\
    elem##_iter=nl->Rest(elem##_iter))

/*
All sources of the NestedRelation2 algebra are contained in their own namespace
"nr2a"[2] to avoid name clashes..

*/
namespace nr2a {

/*
To make the "AutoWrite" macro work more universally a few methods are used to
handle different types. The one with the "Flob"[2] parameetr is used in
"WriteFlob"[2].

*/
void Write(const NList &list, const std::string &label);
void Write(const bool value, const std::string & label);
void Write(const ListExpr list, const std::string & label);
void Write(const Flob* flob, const SmiSize offset,
    const SmiSize maxLength);

/*
The NestedRelation2 algebra is represented by the similar named class. It
features methods to register types and operators.

*/
class NestedRelation2Algebra : public Algebra
{
  public:
    NestedRelation2Algebra();
    ~NestedRelation2Algebra();

    void RegisterTypes();
    void RegisterOperators();

    void AddTypeConstructorProxy(TypeConstructor tc,
        const bool nonstatic = false);
    template<class OperatorType> Operator *AddOperatorToAlgebra(
        const bool usesArgsInTypeMapping = false);
    template<class TypeType> void AddTypeToAlgebra(const bool isData);
};

/*
Template functions are used to avoid redundant code for registering classes
used for implementing the algebra.

*/
template<class OperatorType>
Operator * NestedRelation2Algebra::AddOperatorToAlgebra(
    const bool usesArgsInTypeMapping /*=false*/)
{
  Operator *op = NULL;
  op = new Operator(typename OperatorType::Info(),
      OperatorType::functions, OperatorType::SelectFunction,
      OperatorType::MapType, OperatorType::costEstimators);
  Algebra::AddOperator(op, true);
  op->EnableProgress();
  if (usesArgsInTypeMapping)
  {
    op->SetUsesArgsInTypeMapping();
  }
  return op;
}

template<class TypeType>
void NestedRelation2Algebra::AddTypeToAlgebra(const bool isData)
{
  TypeConstructor *tc = new TypeConstructor(
      typename TypeType::Info::Info(),
      typename TypeType::Functions::Functions());
  if (isData)
  {
    tc->AssociateKind(Kind::DATA());
  }
  Algebra::AddTypeConstructor(tc, true);
}

}

#endif /* ALGEBRAS_NESTEDRELATION2_NESTEDRELATION2ALGEBRA_H_*/
