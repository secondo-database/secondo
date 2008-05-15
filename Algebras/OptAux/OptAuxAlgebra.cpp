/*
---- 
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science, 
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

//paragraph [1] title: [{\Large \bf ]   [}]
//characters    [2]    verbatim:   [\verb@]    [@]
//[ue] [\"{u}]
//[toc] [\tableofcontents]

""[2]

[1] OptAux Algebra


*/

#include "Algebra.h"
#include "NestedList.h"
#include "NList.h" 
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "ConstructorTemplates.h" 
#include "StandardTypes.h"

/*
The file "Algebra.h" is included, since the new algebra must be a subclass of
class Algebra. All of the data available in Secondo has a nested list
representation. Therefore, conversion functions have to be written for this
algebra, too, and "NestedList.h" is needed for this purpose. The result of an
operation is passed directly to the query processor. An instance of
"QueryProcessor" serves for this. Secondo provides some standard data types, e.g.
"CcInt", "CcReal", "CcString", "CcBool", which is needed as the result type of the
implemented operations. To use them "StandardTypes.h" needs to be included. 
   
*/  

extern NestedList* nl;
extern QueryProcessor *qp;

/*
The variables above define some global references to unique system-wide
instances of the query processor and the nested list storage.

1.2 Auxiliaries

Within this algebra module implementation, we have to handle values of
four different types defined in namespace ~symbols~: ~INT~ and ~REAL~, ~BOOL~ and
~STRING~.  They are constant values of the C++-string class.

Moreover, for type mappings some auxiliary helper functions are defined in the
file "TypeMapUtils.h" which defines a namespace ~mappings~.

*/

#include "TypeMapUtils.h"
#include "Symbols.h"

using namespace symbols;
using namespace mappings;

#include <string>
using namespace std;

/*
The implementation of the algebra is embedded into
a namespace ~prt~ in order to avoid name conflicts with other modules.
   
*/   

namespace optaux {


/*

5 Creating Operators

5.1 Type Mapping Functions

A type mapping function checks whether the correct argument types are supplied
for an operator; if so, it returns a list expression for the result type,
otherwise the symbol ~typeerror~. Again we use interface ~NList.h~ for
manipulating list expressions.

*/


ListExpr
predcounts_tm( ListExpr args )
{
  NList type(args);
  const string errMsg = "Expecting two rectangles "
	                "or a point and a rectangle";

  // first alternative: xpoint x xrectangle -> bool
  if ( type == NList(XPOINT, XRECTANGLE) ) {
    return NList(BOOL).listExpr();
  }  
  
  // second alternative: xrectangle x xrectangle -> bool
  if ( type == NList(XRECTANGLE, XRECTANGLE) ) {
    return NList(BOOL).listExpr();
  }  
  
  return NList::typeError(errMsg);
}



/*
5.3 Value Mapping Functions

5.3.1 The ~intersects~ predicate for two rectangles

*/
int
predcounts_vm(Word* args, Word& result, int message, 
              Word& local, Supplier s)
{
  return 0;
}

/*

4.4 Operator Descriptions

Similar to the ~property~ function of a type constructor, an operator needs to
be described, e.g. for the ~list operators~ command.  This is now done by
creating a subclass of class ~OperatorInfo~.

*/
struct predcountsInfo : OperatorInfo {

  predcountsInfo()
  {
    name      = "predcounts";
    signature = "";
    syntax    = "";
    meaning   = "";
  }

}; // Don't forget the semicolon here. Otherwise the compiler 
   // returns strange error messages



/*

5 Implementation of the Algebra Class

*/

class OptAuxAlgebra : public Algebra
{
 public:
  OptAuxAlgebra() : Algebra()
  {

/*   
5.3 Registration of Operators

*/

    AddOperator( predcountsInfo(), predcounts_vm, predcounts_tm );
    
  }
  ~OptAuxAlgebra() {};
};


/*
6 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime (if it is built as a dynamic link library). The name
of the initialization function defines the name of the algebra module. By
convention it must start with "Initialize<AlgebraName>". 

To link the algebra together with the system you must create an 
entry in the file "makefile.algebra" and to define an algebra ID in the
file "Algebras/Management/AlgebraList.i.cfg".

*/

} // end of namespace ~prt~

extern "C"
Algebra*
InitializeOptAuxAlgebra( NestedList* nlRef, 
                         QueryProcessor* qpRef )
{
  // The C++ scope-operator :: must be used to qualify the full name 
  return new optaux::OptAuxAlgebra; 
}

/*
7 Examples and Tests

The file "PointRectangle.examples" contains for every operator one example.
This allows one to verify that the examples are running and to provide a coarse
regression test for all algebra modules. The command "Selftest <file>" will
execute the examples. Without any arguments, the examples for all active
algebras are executed. This helps to detect side effects, if you have touched
central parts of Secondo or existing types and operators. 

In order to setup more comprehensive automated test procedures one can write a
test specification for the ~TestRunner~ application. You will find the file
"example.test" in directory "bin" and others in the directory "Tests/Testspecs".
There is also one for this algebra. 

Accurate testing is often treated as an unpopular daunting task. But it is
absolutely inevitable if you want to provide a reliable algebra module. 

Try to write tests covering every signature of your operators and consider
special cases, as undefined arguments, illegal argument values and critical
argument value combinations, etc.


*/

