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

April 2006, M. Spiekermann. The file Algebra.h need to be divided into Operators.h. TypeConstructors.h, 
AlgebraClassDef.h and AlgebraInit.h   

*/

#ifndef SEC_ALGEBRA_CLASSDEF_H
#define SEC_ALGEBRA_CLASSDEF_H

#include <vector>

// forward declarations
class Operator;
class TypeConstructor;

using namespace std;

/*
1.3 Macro ~CHECK\_COND~

This macro makes reporting errors in type mapping functions more conveniently.

*/
#define CHECK_COND(cond, msg) \
  if(!(cond)) \
  {\
    ErrorReporter::ReportError(msg);\
    return nl->SymbolAtom("typeerror");\
  };


/*
1.6 Class "Algebra"[1]

An instance of class ~Algebra~ provides access to all information about a given
algebra at run time, i.e. a set of type constructors and a set of operators.
These properties have to be set once. A straightforward approach is to do
these settings within an algebra's constructor. As all algebra modules use
different type constructors and operators, all algebra constructors are
different from each other. Hence we cannot use a single constructor, but
request algebra implementors to derive a new subclass of class ~Algebra~ for
each algebra module in order to provide a new constructor. Each of these
subclasses will be instantiated exactly once. An algebra subclass
instance serves as a handle for accessing an algebra's type constructors
and operators.

*/

class Algebra
{
 public:
  Algebra();
/*
Creates an instance of an algebra. Concrete algebra modules are implemented
as subclasses of class ~Algebra~.

*/
  virtual ~Algebra();
/*
Destroys an algebra instance.

*/
  int GetNumTCs() { return (tcs.size()); }
/*
Returns the number of type constructors provided by the algebra module.

*/
  int GetNumOps() { return (ops.size()); }
/*
Returns the number of operators provided by the algabra module.

*/
  inline TypeConstructor* GetTypeConstructor( int index )
  { 
     assert((index >= 0) && (index <= tcsNum-1)); 
     return tcs[index]; 
  }
/*
Returns a reference to the type constructor identified by ~index~.

*/
  inline Operator* GetOperator( int index ) 
  { 
     assert((index >= 0) && (index <= opsNum-1)); 
     return ops[index]; 
  }
/*
Returns a reference to the operator identified by ~index~.

*/
 protected:
  void AddTypeConstructor( TypeConstructor* tc );
  void AddOperator( Operator* op );
/*
Are used by the subclassed algebra to add its type constructors and
operators to the list of type constructors and operators within the
base class.

*/
 private:
  vector<TypeConstructor*> tcs;
  vector<Operator*> ops;
  int tcsNum;
  int opsNum;

};

#endif

