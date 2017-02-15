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

#include "Algebra.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "NList.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h"
#include "TypeMapUtils.h"
#include "Symbols.h"
#include "TypeConstructor.h"

#include <string>

#include "Include.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;
namespace nr2a {

/*
The "AutoWrite"[2] macro is based on the following methods handling several
data types.

*/

void Write(const NList &list, const string & label = "")
{
  cout << endl;
  if (label != "")
    cout << label.c_str() << ": " << endl;
  list.writeAsStringTo(cout);
  cout << endl;
  cout.flush();
}

void Write(const NList *list, const string & label = "")
{
  Write(*list, label);
}

void Write(const bool value, const string & label)
{
  Write(nl->SymbolAtom(value ? "true" : "false"), label);
}

void Write(const ListExpr list, const string & label)
{
  Write(NList(list), label);
}

/*
Output parts of a flob for debugging.

*/
void Write(const Flob* flob, const SmiSize offset,
    const SmiSize maxLength)
{
  SmiSize length = min(maxLength, flob->getSize());
  char *buffer = new char[length];
  memset(buffer, 0, length);
  flob->read(buffer, length, offset);
  cout << "FLOB " << offset << ":" << length;
  //printf "consumes" a char, but reads an int. If using buffer[i] valgrind
  //complains about reading the uninitialised bytes of an int just created
  //during casting. To avoid this complaint an int is created and initialised
  //explicitly.
  int a = 0;
  for(SmiSize i = 0; i<length;i++)
  {
    if (i % 5 ==0)
    {
      if(i % 20 == 0)
        cout << endl;
      else
        cout << " . ";
    }
    else
      cout << " ";
    a = buffer[i];
    printf("%3hhu ", a);
    //"Printing" control characters might be a problem
    if((buffer[i]>=32) && (buffer[i]<=126))
      cout << buffer[i];
    else
      cout << " ";
  }
  cout << endl;
  delete[] buffer;
}

/*
To encapsulate the classes for types and operators as much as possible the
definition of the "TypeConstructor"[2]s is done in their source files and
referenced here.

*/
extern TypeConstructor arel2TC;
extern TypeConstructor nrel2TC;

/*
As a default simply use the constructor of "Algebra"[2].

*/
NestedRelation2Algebra::NestedRelation2Algebra()
    : Algebra()
{
  //void
}

//virtual
NestedRelation2Algebra::~NestedRelation2Algebra()
{
}

/*
The methods "RegisterTypes"[2] and "RegisterOperators"[2] are used to
register classes implementing types and operators for usage in the algebra.

*/
void NestedRelation2Algebra::RegisterTypes()
{
  AddTypeToAlgebra<ARel>(true);
  AddTypeToAlgebra<NRel>(false);
}

void NestedRelation2Algebra::RegisterOperators()
{
  AddOperatorToAlgebra<Count>();
  AddOperatorToAlgebra<Feed>();
  AddOperatorToAlgebra<Aconsume>();
  AddOperatorToAlgebra<Consume>();
  AddOperatorToAlgebra<Unnest>();
  AddOperatorToAlgebra<Nest>();
  AddOperatorToAlgebra<Extract>();
  AddOperatorToAlgebra<Rename>(true);
  AddOperatorToAlgebra<GetTuples>();
  AddOperatorToAlgebra<DblpImport>();
  AddOperatorToAlgebra<GenRel>(true);
  AddOperatorToAlgebra<TypeOf>();
}

} // namespace nr2a

/*
This method registers the algebra to SECONDO.

*/
extern "C" Algebra*
InitializeNestedRelation2Algebra(NestedList* nlRef,
    QueryProcessor* qpRef)
{
  nr2a::NestedRelation2Algebra *algebra =
      new nr2a::NestedRelation2Algebra;
  algebra->RegisterTypes();
  algebra->RegisterOperators();
  return algebra;
}
