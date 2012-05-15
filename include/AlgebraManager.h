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
//[<] [$<$]
//[>] [$>$]

1 Header File: Algebra Manager

September 1996 Claudia Freundorfer

9/26/96 RHG Slight revisions of text. Constant ~MAXTYPES~ introduced.

October 1996 RHG Revised Overview. Introduced types ~SelectMapping~,
~SelectArray~, and variable ~SelectFunction~ to accomodate new concept
for overloading.

December 20, 1996 RHG Changed format of procedure type ~OutObject~.

January 8/9, 1997 RHG Changed format of procedure type ~InObject~.

May 4, 1998 RHG Type ~ValueMapping~ (interface of evaluation functions)
changed; additional parameter ~opTreeNode~ of type ~Supplier~.

May 15, 1998 RHG Added some generic functions for models (~InModel~,
~OutModel~, ~ValueToModel~.

August 25, 1998 Stefan Dieker Added generic functions for type checking
and procedure ~InitKinds~.

April 2002 Ulrich Telle Port to C++, complete revision

August 2002 Ulrich Telle Changed ~PersistValue~ and ~PersistModel~ interface
using nested lists for the type instead of the string representation.

January, 2003 VTA Changed ~PersistValue~ interface and created three
new functions ~Open~, ~Save~, ~Close~, and ~Clone~ that join the ~Create~
and ~Delete~ to form the object state diagram in the Figure 1.

Figure 1: Object state diagram [objstatediagram.eps]

March 2003, Victor Almeida created the function ~SizeOf~

August 2003 VTA Added the ~SaveToList~ and ~RestoreFromList~ functions

September 2003 Frank Hoffmann Added ~ListAlgebras~ and ~GetAlgebraId~

August 2004, M. Spiekermann. Getting an algebra name by its ID is now supported
with the function ~GetAlgebraName~

July 2005, M. Spiekermann. Ugly idention corrected. Missing return statement in
function ExecuteCost corrected.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006, M. Spiekermann. Some declarations were moved to Algebra.h

April 2006, M. Spiekermann. Some declarations were moved to AlgebraInit.h

March 2009, M. Spiekermann. Started to do some clean up. The interface seems to be
overcrowded by functions getting algebra id and type id or operator id. In order to
make the interface simpler to understand and use and easier to maintain
I started to push some functionality into the interfaces TypeConstructor.h
and Operator.h. The new functions ~GetTC~ and ~GetOP~ should be able to replace
many of the other functions, since most of the other functions simply get pointers
of operators and type constructors and call spcific member functions. Moreover they
are not often used in other source files. To be continued ...


1.1 Overview

The "Secondo"[3] algebra manager is responsible for registering and initializing
all specified algebra modules and provides interface functions for the
query processor and the catalog functions to access the type constructors
and operators of each registered algebra module.

The "Secondo"[3] system needs to know about the algebra modules available
at run time. Therefore the developer has to specify which algebra modules
are to be included in the build of the system. The list of all available
algebra modules is kept in the source file "AlgebraList.i"[4]. Each algebra
can be flagged whether it should be included or not.

Currently the list of algebras to be included must be available at link
time, although an algebra module may be a shared library which is loaded
dynamically into memory only when the "Secondo"[3] server is started. To
build an algebra modula as a shared library has the advantage that one
does not need to rebuild the "Secondo"[3] system when only the
implementation of the algebra modula changes.

In principal it would be possible to load algebra modules at runtime which
are not known at link time. But there are at least two disadvantages:

  1 it would restrict the user to command specifications in nested list form,
because the code of the "Secondo"[3] parser depends on algebra specifications,
which are compiled into the code, and

  2 it could cause problems when such a dynamically loadable algebra module is
referenced by any persistent object, but is not included in the load list any
more.

Therefore this feature is currently not implemented.

The ~AlgebraManager~ manages a set of algebra modules. Every algebra
consists of a set of types (type constructors, to be precise) and a set
of operators and is defined by descriptions in SOS-Format as described
in [G[ue]92]. A data structure and optionally a ``data model'' belongs
to each type of the algebra. A ~model~ is a miniature version of the
data structure itself used instead of this for cost estimations (e.g.
for a relation the number of tuples, the number of used segments,
statistics for the distribution of chosen attributes could be taken into
account within the model).

An algebra module offers for each type constructor up to eight generic
operations:

  * ~AssociateKind~, ~Property~. Supply the kind and the properties of a type.

  * ~Create~, ~Delete~. Allocate/deallocate internal memory. Not needed
for types represented in a single word of storage.

  * ~Open~, ~Close~, ~Save~. Only needed, if the type is ~independently~
persistent. For example, they will be needed for relations or object
classes, but not for tuples or atomic values (if we decide that the
latter are not independently persistent).

  * ~Clone~. Clones an instance of a type constructor.

  * ~In~, ~Out~. Map a nested list into a value of the type and vice
versa. Used for delivering a value to the application (among other
things).

  * ~RestoreFromList~, ~SaveToList~. Act as ~In~ and ~Out~ functions
but use internal representation of the objects. Most simple objects
does not need this functions, so by default they will call the
correspondent ~In~ and ~Out~ functions.

  * ~SizeOf~. Returns the size of the RootRecord of the object.

Every value of every type has a representation as a single "Word"[4] of
storage. For a simple type, this can be all. For more complex types, it
may be a pointer to some structure. For persistent objects (that are
currently not in memory) it can be an index into a catalog which tells
where the object is on disk.

Two default implementations (~DefaultOpen~ and ~DefaultSave~)
are supplied which store values in their nested list representation.

The algebra module offers three (sets of) functions for every operator,
namely

  * ~Set of Evaluation Functions~. Associated with each operator is at
least one, but are possibly several, evaluation functions. Each
evaluation function maps a list of input values given in an array of
~Word~ into a result value (also a ~Word~). The interface of the procedure
is independent from the type of operation. Stream operators and
parameter function calls are handled by calling the functions ~Request~,
~Open~, ~Close~, ~Cancel~, and ~Received~ of the module ~QueryProcessor~.

  * ~Type Mapping~. This function maps a list of input types in nested
list format (~ListExpr~) into an output type. Used for type checking and
mapping.

  * ~Selection Function~. This function is given a list of argument types
together with an operator number; based on these it returns the number
of an evaluation function. This is used to determine for an overloaded
operator the appropriate evaluation function. Usually the same function
can be used for all operators of an algebra; if operators are not
overloaded, identity (on the numbers) is sufficient.

1.2 Defines, Includes, Constants

*/

#ifndef ALGEBRA_MANAGER_H
#define ALGEBRA_MANAGER_H

#include <map>
#include <vector>

#include "AlgebraTypes.h"
#include "NestedList.h"
#include "SecondoSMI.h"
#include "AlgebraInit.h"
#include "AlgebraClassDef.h"
#include "Operator.h"
#include "SystemInfoRel.h"
#include "CostEstimation.h"

/*
1.6 Class "AlgebraManager"[1]

*/


class AlgebraManager
{
 public:
  AlgebraManager( NestedList& nlRef,
                  GetAlgebraEntryFunction getAlgebraEntryFunc );
/*
Creates an instance of the algebra manager. ~nlRef~ is a reference to the
nested list container which should be used for nested lists.

*/
  virtual ~AlgebraManager();
/*
Destroys an algebra manager.

*/

  ListExpr ListAlgebras();
/*
Lists all algebras, which are currently included within the "Secondo"[3] system.
The list format is :

---- (<algebra name 1>..<algebra name n>)
----

*/

  int GetAlgebraId( const string& algName);
  const string& GetAlgebraName( const int algId );

/*
Returns the id of the algebra with name algName, if this algebra is currently
included, otherwise 0. The second function returns the name of an algebra
specified by algId.

*/
  void LoadAlgebras();
/*
All existing algebras are loaded into the "Secondo"[3] programming interface.
The catalog of every algebra contains the specifications of type constructors
and operators of the algebra as defined above.

This procedure has to be started before the use of other functions of the
database, otherwise no algebra function (operator) or type constructor can
be used.

*/
  void UnloadAlgebras();
/*
The allocated memory for the algebra catalogs is returned.
No algebra function (operator) or type constructor can be used anymore.

*/
  bool IsAlgebraLoaded( const int algebraId );
/*
Returns "true"[4], if the algebra module ~algebraId~ is loaded.
Otherwise "false"[4] is returned.

*/
  int CountAlgebra();
/*
Returns the number of loaded algebras.

*/
  bool NextAlgebraId( int& algebraId );
/*
Returns the identification number ~algebraId~ of the next
loaded algebra. On the first call ~algebraId~ must be initialized
to zero.

*/
  int OperatorNumber( const int algebraId );
/*
Returns the number of operators of algebra ~algebraId~.

*/
  inline int Select( const int algebraId,
                     const int operatorId, const ListExpr typeList )
  {
    return getOperator(algebraId, operatorId)->Select(typeList);
  }

/*
Returns the address of the select function of operator ~operatorId~ of
algebra ~algebraId~.

*/

  inline int Execute( const int algebraId, const int opFunId,
                      ArgVector args, Word& result, int msg,
                            Word& local, Supplier tree )
  {
    int opId  = opFunId % 65536;
    int funId = opFunId / 65536;

    return getOperator(algebraId, opId)->CallValueMapping( funId,
                                                           args, result,
                                                           msg, local, tree );
  }

/*
Returns the address of the evaluation function of the - possibly
overloaded - operator ~opFunId~ of algebra ~algebraId~.

*/

  inline ListExpr TransformType( const int algebraId, const int operatorId,
                                 const ListExpr typeList )
  {
    return getOperator(algebraId, operatorId)->CallTypeMapping(typeList);
  }

/*
Returns the address of the type mapping function of operator
~operatorId~ of algebra ~algebraId~.

*/

  int ConstrNumber( const int algebraId );
/*
Returns the number of constructors of algebra ~algebraId~.

*/
  TypeConstructor* GetTC(int algId, int typeId);
  Operator*        GetOP(int algebraId, int opId);
/*
Returns the type constructor (TC) or operator (OP) for a given algebra id and type
id (operator id). This pointer can be used to retrieve more detailed information of
a TC or OP provided by its member functions.

*/

  InObject InObj( const int algebraId, const int typeId );
/*
Returns the address of the object input function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  OutObject OutObj( const int algebraId, const int typeId );
/*
Returns the address of the object output function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  InObject RestoreFromListObj( const int algebraId, const int typeId );
/*
Returns the address of the object's internal input function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  OutObject SaveToListObj( const int algebraId, const int typeId );
/*
Returns the address of the object's internal output function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  ObjectCreation
    CreateObj( const int algebraId, const int typeId );
/*
Returns the address of the object creation function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  ObjectDeletion
    DeleteObj( const int algebraId, const int typeId );
/*
Returns the address of the object deletion function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  bool OpenObj( const int algebraId, const int typeId,
                SmiRecord& valueRecord, size_t& offset,
                const ListExpr typeInfo, Word& value );
/*
Open objects of type as constructed by type constructor
~typeId~ of algebra ~algebraId~.
Return "true"[4], if the operation was successful.

*/
  bool SaveObj( const int algebraId, const int typeId,
                SmiRecord& valueRecord, size_t& offset,
                const ListExpr typeInfo, Word& value );
/*
Save objects of type as constructed by type constructor
~typeId~ of algebra ~algebraId~.
Return "true"[4], if the operation was successful.

*/
  ObjectClose
    CloseObj( const int algebraId, const int typeId );
/*
Returns the address of the object close function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  ObjectClone
    CloneObj( const int algebraId, const int typeId );
/*
Returns the address of the object clone function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  ObjectCast Cast( const int algebraId, const int typeId );
/*
Returns the address of the type casting function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  ObjectSizeof
    SizeOfObj( const int algebraId, const int typeId );
/*
Returns the address of the object sizeof function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  bool TypeCheck( const int algebraId,
                  const int typeId,
                  const ListExpr type,
                  ListExpr& errorInfo  );
/*
Returns the result of the type check function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  bool CheckKind( const string& kindName,
                  const ListExpr type,
                  ListExpr& errorInfo );
/*
Checks if ~type~ is an element of kind ~kindName~.

First parameter is the type expression to be checked. Second parameter
is a list of which every element is an error message (also a list). The
procedure returns "true"[4] if the type expression is correct. Otherwise it
returns "false"[4] and adds an error message to the list. An error message
has one of two formats:

----    (60 <kind> <type expression>)
----

This means ``kind ~kind~ does not match ~type expression~''.
The second format is:

----    (61 <kind> <errno> ...)
----

This means ``Error number ~errno~ in type expression for kind ~kind~''.
The specific error numbers are defined in the kind checking procedure;
the list may contain further information to describe the error.

*/

  inline Operator* getOperator(const int algebraId, const int opId) {

  if(algebraId<0 || (size_t)algebraId >= algebra.size()){
    return 0;
  }
  Algebra* alg = algebra[algebraId];
  if(alg==0){
     return 0;
  }
  if(opId < 0 || opId >= alg->GetNumOps()){
    return 0;
  }
  return alg->GetOperator(opId);
 }
  


  void UpdateOperatorUsage(SystemInfoRel* table);

/*
~matching operators~

This function returns all operators which can process the given type expression.
The result consists of pairs with the first component is a pair of algebra id and
operator id, the second part of the top pair is the result type when this
operator would be applied to this type expression.

Call __AlgebraManager::GetOP(AldId,OpId)__ to get the according operator object
for further details, if required.

*/

vector< pair< pair<int,int>, ListExpr> >
        matchingOperators(const ListExpr arguments);

/*
This functions returns the same information, but restricts its search to the
algebra specified by the given algebra id ~algId~.

~algId~ must be [>]0 and [<]algebra.size().

The results are appended to the reference argument vector.

*/
void matchingOperators(const int algId, const ListExpr arguments,
                       vector< pair< pair<int,int>, ListExpr> >& result);

/*
~findOperator~

Search for an operator named __name__ accepting arglist as arguments.
If an operator is found, all outParameters (resultList, algId, opId) are
set and the result is true. Otherwise, the result will be false.

*/
bool findOperator(const string& name,    //name of the operator
                  const ListExpr argList,//arguments
                  ListExpr& resultList,  // result type
                  int& algId,            // algebra id
                  int& opId);            // operator id

bool findOperator(const string& name,    //name of the operator
                  const ListExpr argList,//arguments
                  ListExpr& resultList,  // result type
                  int& algId,            // algebra id
                  int& opId,             // operator id
                  int& funId);           // value mapping id

/*
~getCostEstimation~

Returns the costestimation for a specified value mapping.
If there is no one, null is returned.

*/

CostEstimation* getCostEstimation( const int algId,
                                   const int opId,
                                   const int funId);




/*
~getCosts~

The next functions return costs for a specified operator when number of tuples
and size of a single tuple is given. If the operator does not provide a
cost estimation function or the getCost function is not implemented,
the return value is false.

*/

bool getCosts(const int algId,
              const int opId,
              const int funId,
              const size_t noTuples,
              const size_t sizeOfTuple,
              const size_t memoryMB,
              size_t& costs);


bool getCosts(const int algId,
              const int opId,
              const int funId,
              const size_t noTuples1,
              const size_t sizeOfTuple1,
              const size_t noTuples2,
              const size_t sizeOfTuple2,
              const size_t memoryMB,
              size_t& costs);

/*
~getLinearParams~

Retrieves the parameters for estimating the cost function of an operator
in a linear way.

*/
bool getLinearParams( const int algId,
                      const int opId,
                      const int funId,
                      const size_t noTuples1,
                      const size_t sizeOfTuple1,
                      double& sufficientMemory,
                      double& timeAtSuffMemory,
                      double& timeAt16MB);


bool getLinearParams( const int algId,
                      const int opId,
                      const int funId,
                      const size_t noTuples1,
                      const size_t sizeOfTuple1,
                      const size_t noTuples2,
                      const size_t sizeOfTuple2,
                      double& sufficientMemory,
                      double& timeAtSuffMemory,
                      double& timeAt16MB);

/*
~getFunction~

Returns an approximation of the cost function of a specified value mapping as
a parametrized function.

*/
bool getFunction(const int algId,
                 const int opId,
                 const int funId,
                 const size_t noTuples,
                 const size_t sizeOfTuple,
                 int& funType,
                 double& sufficientMemory,
                 double& timeAtSuffMemory,
                 double& timeAt16MB,
                 double& a, double& b, double&c, double& d);
                      


bool getFunction(const int algId,
                 const int opId,
                 const int funId,
                 const size_t noTuples1,
                 const size_t sizeOfTuple1,
                 const size_t noTuples2,
                 const size_t sizeOfTuple2,
                 int& funType,
                 double& sufficientMemory,
                 double& timeAtSuffMemory,
                 double& timeAt16MB,
                 double& a, double& b, double&c, double& d);


/*
Returns the highest algebra id occuring in the vector of algebras.

*/
int getMaxAlgebraId() const;


/*
Returns the nested list storage used in this component.

*/
 NestedList* getListStorage(){ return nl; }

 private:
  NestedList*              nl;
/*
Is a referenced to a global nested list container.

*/
  int                      maxAlgebraId;
/*
Is the highest algebra id occuring in the list of algebras.

*/
  vector<Algebra*>         algebra;
  map<int, string>         algebraNames;
/*
Is an array for references to all loaded algebra modules.

*/
  multimap<string,TypeCheckFunction> kindTable;
  GetAlgebraEntryFunction getAlgebraEntry;

  vector< vector<Operator*> > opPtrField;

  void InitOpPtrField();

};

#endif

