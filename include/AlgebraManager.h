/*
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

<<<<<<< AlgebraManager.h
January, 2003 VTA Changed ~PersistValue~ interface and created three
new functions ~Open~, ~Save~, ~Close~, and ~Clone~ that join the ~Create~ 
and ~Delete~ to form the object state diagram in the Figure 1.

Figure 1: Object state diagram [objstatediagram.eps]

=======
February 2003 Ulrich Telle Introduced new mode ~DeleteFrom~ for the
~PersistValue~ and ~PersistModel~ interface

>>>>>>> 1.9
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

Every value of every type has a representation as a single "Word"[4] of
storage. For a simple type, this can be all. For more complex types, it
may be a pointer to some structure. For persistent objects (that are
currently not in memory) it can be an index into a catalog which tells
where the object is on disk. 

If a type (constructor) has a model, then the algebra module offers
four further operations: 

  * ~InModel~. Create a model data structure from a nested list. 

  * ~OutModel~. Create a nested list representation from the model data
structure 

  * ~ValueToModel~. Create a model data structure from a value. 

  * ~ValueListToModel~.

For support of persistent storage of object models one more
operation:

  * ~PersistModel~. Saving and restoring an object model.

Three default implementations (~DefaultOpen~, ~DefaultSave~ and 
~DefaultPersistModel~) are supplied which store values and models in 
their nested list representation.

The algebra module offers five (sets of) functions for every operator,
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

  * ~Model Mapping~. Computes a result model from the argument models. 

  * ~Cost Mapping~. Estimates from the models of the arguments the costs
for evaluating the operation (number of CPUs, number of swapped pages).
If there are no models, it returns some constant cost. 

1.2 Defines, Includes, Constants

*/

#ifndef ALGEBRA_MANAGER_H
#define ALGEBRA_MANAGER_H

#include <map>
#include "AlgebraTypes.h"
#include "NestedList.h"
#include "SecondoSMI.h"

const int MAXARG = 20;
/*
Is the maximal number of arguments for one operator

*/

const int OPEN    = 1;
const int REQUEST = 2;
const int CLOSE   = 3;
const int YIELD   = 4;
const int CANCEL  = 5;
/*
Are constants for stream processing.

*/

enum PersistDirection { ReadFrom, WriteTo, DeleteFrom };
/*
Defines whether the methods managing the persistence of object values and models
are persisting an object ("WriteTo"[4]), restoring an object ("ReadFrom"[4]),
or deleting an object ("DeleteFrom"[4]).

*/
/*
1.5 Types

*/

typedef Word ArgVector[MAXARG];
typedef ArgVector* ArgVectorPointer;
/*
Are the types for generic argument vectors for algebra functions.

*/

typedef Address Supplier;
/*
Is the type for references to a supplier of information of the operator tree.

*/

typedef int (*ValueMapping)( ArgVector args, Word& result,
                             int msg, Word& local,
                             Supplier tree );
/*
Is the type of an evaluation function.

*/

typedef ListExpr (*TypeMapping)( ListExpr typeList );
/*
Is the type of a type mapping procedure.

*/

typedef TypeMapping CostMapping;
/*
Is the type of a cost mapping procedure.

*/

typedef int (*SelectFunction)( ListExpr typeList );
/* 
Is the type of a selection function.

*/

typedef bool (*PersistFunction)( PersistDirection dir,
                                 SmiRecord& valueRecord,
                                 const ListExpr typeExpr,
                                 Word& value );
/*
Is the type of a function for object value and model persistence.

*/

typedef Word (*ModelMapping)( ArgVector args, Supplier tree );

typedef Word (*InModelFunction)( ListExpr typeExpression,
                                 ListExpr modelList,
                                 int objectNumber );

typedef ListExpr (*OutModelFunction)( ListExpr typeExpression,
                                      Word model );

typedef Word (*ValueToModelFunction)( ListExpr typeExpression,
                                      Word value );

typedef Word (*ValueListToModelFunction)(
                 const ListExpr typeExpr,
                 const ListExpr valueList,
                 const int errorPos,
                 ListExpr& errorInfo,
                 bool& correct );

/*
Are the types of model mapping functions and of ~in~ and ~out~ functions for
models.

*/

typedef Word (*InObject)( const ListExpr typeInfo,
                          const ListExpr valueList,
                          const int errorPos,
                          ListExpr& errorInfo,
                          bool& correct );

typedef ListExpr (*OutObject)( const ListExpr numType,
                               const Word object );

typedef Word (*ObjectCreation)( const ListExpr typeInfo );

typedef void (*ObjectDeletion)( Word& object ); 

typedef bool (*ObjectOpen)( SmiRecord& valueRecord,
                            const ListExpr typeExpr,
                            Word& value );

typedef bool (*ObjectSave)( SmiRecord& valueRecord,
                            const ListExpr typeExpr,
                            Word& value );

typedef void (*ObjectClose)( Word& object ); 

typedef Word (*ObjectClone)( const Word& object ); 

typedef void* (*ObjectCast)(void*);

/*
Are the types used for creating, deleting and initializing the
algebra objects or components of the objects and for appending new
subobjects.

This shows also the types of the generic functions for the type constructors.
This is not yet satisfactory, will be revised.

*/

typedef bool (*TypeCheckFunction)( const ListExpr type,
                                   ListExpr& errorInfo );
/*
Is the type for type checking functions, one for each type constructor.

*/

typedef ListExpr (*TypeProperty)();
/*
Is the type of property functions, one for each type constructor.

*/

/*
1.6 Class "AlgebraManager"[1]

*/

struct AlgebraListEntry;
class Algebra;
class QueryProcessor;
/*
Are ~forward declarations~ of used data structures and classes.

*/

typedef Algebra*
          (*AlgebraInitFunction)( NestedList* nlRef,
                                  QueryProcessor* qpRef );
/*
Is the prototype of the algebra initialization functions. For each
algebra the algebra manager calls an initialization  function to get
a reference to the algebra and to provide the algebra with references
to the global nested list container and the query processor.

*/

class DynamicLibrary;

struct AlgebraListEntry
{
  AlgebraListEntry()
    : algebraId( 0 ), algebraName( "" ),
      level( UndefinedLevel ),
      algebraInit( 0 ), dynlib( 0 ), useAlgebra( false ) {}
  AlgebraListEntry( const int algId, const string& algName,
                    const AlgebraLevel algLevel,
                    const AlgebraInitFunction algInit,
                    DynamicLibrary* const dynlibInit,
                    const bool algUse )
    : algebraId( algId ),   algebraName( algName ),
      level( algLevel ),    algebraInit( algInit ),
      dynlib( dynlibInit ), useAlgebra( algUse ) {}
  int                  algebraId;
  string               algebraName;
  AlgebraLevel         level;
  AlgebraInitFunction  algebraInit;
  DynamicLibrary*      dynlib;
  bool                 useAlgebra;
};
/*
Is the type for entries in the list of algebras. Each algebra has a
unique identification number ~algebraId~, a name ~algebraName~ and one
of the following levels

  * ~Descriptive~

  * ~Executable~

  * ~Hybrid~ -- the algebra is ~descriptive~ *and* ~executable~.

Additionally the address of the initialization function of the algebra is
registered. For algebras to be used this forces the linker to include the
algebra module from an appropriate link library. For an algebra to be
loaded dynamically the address of the initialization function is not set.
Instead the method ~LoadAlgebras~ uses the algebra name to identify and 
load the shared library and to identify and call the initialization
function of the algebra module. A reference to the shared library is
stored in the member variable ~dynlib~ to be able to unload the library
on termination.

Finally there is a flag whether this algebra should be included in the
initialization process or not.

*/

typedef AlgebraListEntry& (*GetAlgebraEntryFunction)( const int j );

#define ALGEBRA_LIST_START \
static AlgebraListEntry algebraList[] = {

#define ALGEBRA_LIST_END \
  AlgebraListEntry( -1, "", UndefinedLevel, 0, 0, false ) };

#define ALGEBRA_LIST_INCLUDE(ALGNO,ALGNAME,ALGTYPE) \
 AlgebraListEntry( ALGNO, #ALGNAME,\
                   ALGTYPE##Level,\
                   &Initialize##ALGNAME, 0, true ),

#define ALGEBRA_LIST_EXCLUDE(ALGNO,ALGNAME,ALGTYPE) \
 AlgebraListEntry( ALGNO, #ALGNAME,\
                   ALGTYPE##Level, 0, 0, false ),

#define ALGEBRA_LIST_DYNAMIC(ALGNO,ALGNAME,ALGTYPE) \
 AlgebraListEntry( ALGNO, #ALGNAME,\
                   ALGTYPE##Level, 0, 0, true ),

#define ALGEBRA_PROTO_INCLUDE(ALGNO,ALGNAME,ALGTYPE) \
extern "C" Algebra* \
Initialize##ALGNAME( NestedList* nlRef,\
                     QueryProcessor* qpRef );

#define ALGEBRA_PROTO_EXCLUDE(ALGNO,ALGNAME,ALGTYPE)

#define ALGEBRA_PROTO_DYNAMIC(ALGNO,ALGNAME,ALGTYPE)

/*
These preprocessor macros allow to easily define all available algebras.
To start the list the macro "ALGEBRA\_LIST\_START"[4] is used exactly once,
and the macro "ALGEBRA\_LIST\_END"[4] is used exactly once to terminate the list.

The macros "ALGEBRA\_PROTO\_INCLUDE"[4], "ALGEBRA\_PROTO\_EXCLUDE"[4] and
"ALGEBRA\_\-PROTO\_DYNAMIC"[4] are used to create prototypes for the algebra
initialization functions, the list entries for the algebra modules to be
included or excluded from loading by the algebra manager are generated by
the macros "ALGEBRA\_LIST\_INCLUDE"[4], \ "ALGEBRA\_LIST\_\-EXCLUDE"[4] and
"ALGEBRA\_LIST\_\-DYNAMIC"[4].

In the algebra list definition file "AlgebraList.i"[4] the developer uses
special versions of these macros, namely "ALGEBRA\_INCLUDE"[4],
\ "ALGEBRA\_EXCLUDE"[4] and "ALGEBRA\_\-DYNAMIC"[4]. They are expanded to the
appropriate prototype and list entry macros as required. These macros have
three parameters:

  1 ~the unique identification number~ which must be a positive integer,
it is recommended but not absolutely necessary to order the entries of the
list in ascending order. No identification number may occur more than once
in the list.

  2 ~the algebra name~ which is used to build the name of the initialization
function: the algebra name is appended to the string "Initialize".

  3 ~the level of the algebra~ which may be one of the following: ~Descriptive~,
~Executable~ or ~Hybrid~.

As mentioned above the list of algebras is specified in the source file
"AlgebraList.i"[4] which is included by the algebra manager source file.

Example:

----  ALGEBRA_INCLUDE(1,StandardAlgebra,Hybrid)
      ALGEBRA_DYNAMIC(2,FunctionAlgebra,Executable)
      ALGEBRA_EXCLUDE(3,RelationAlgebra,Hybrid)
----

This means:

  * the ~StandardAlgebra~ will be *included* and must be available
at link time. It has the id number "1"[2] and is defined on descriptive *and*
executable level.

  * the ~FunctionAlgebra~ will be *included*, but will be loaded dynamically
at runtime. It has the id number "2"[2] and is defined on executable level only.

  * the ~RelationAlgebra~ will be *excluded*. It has the id number "3"[2] and is defined on descriptive *and*
executable level.

*/

class AlgebraManager
{
 public:
  AlgebraManager( NestedList& nlRef, GetAlgebraEntryFunction getAlgebraEntryFunc );
/*
Creates an instance of the algebra manager. ~nlRef~ is a reference to the
nested list container which should be used for nested lists.

*/
  virtual ~AlgebraManager();
/*
Destroys an algebra manager.

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
  bool IsAlgebraLoaded( const int algebraId,
                        const AlgebraLevel level );
/*
Returns "true"[4], if the algebra module ~algebraId~ is loaded and
has the specified ~level~. Otherwise "false"[4] is returned.

*/
  int CountAlgebra();
  int CountAlgebra( const AlgebraLevel level );
/*
Returns the number of loaded algebras, all or only of the specified ~level~.

*/
  bool NextAlgebraId( int& algebraId, AlgebraLevel& level );
/*
Returns the identification number ~algebraId~ and the ~level~ of the next
loaded algebra. On the first call ~algebraId~ must be initialized
to zero.

*/
  bool NextAlgebraId( const AlgebraLevel level,
                      int& algebraId );
/*
Returns the identification number ~algebraId~ of the next loaded algebra
of the specified ~level~. On the first call ~algebraId~ must be initialized
to zero.

*/
  int OperatorNumber( const int algebraId );
/*
Returns the number of operators of algebra ~algebraId~.

*/
  string Ops( const int algebraId, const int operatorId );
/*
Returns the name of operator ~operatorId~ of algebra ~algebraId~.

*/
  ListExpr Specs( const int algebraId, const int operatorId );
/*
Returns the specification of operator ~operatorId~ of algebra ~algebraId~
as a nested list expression.

*/
  SelectFunction
    Select( const int algebraId, const int operatorId );
/*
Returns the address of the select function of operator ~operatorId~ of
algebra ~algebraId~.

*/
  ValueMapping
    Execute( const int algebraId, const int opFunId );
/*
Returns the address of the evaluation function of the - possibly 
overloaded - operator ~opFunId~ of algebra ~algebraId~.

*/
  ModelMapping
    TransformModel( const int algebraId, const int opFunId );
/*
Returns the address of the model mapping function of the - possibly 
overloaded - operator ~opFunId~ of algebra ~algebraId~.

*/
  TypeMapping
    TransformType( const int algebraId, const int operatorId );
/*
Returns the address of the type mapping function of operator
~operatorId~ of algebra ~algebraId~.

*/
  TypeMapping
    ExecuteCost( const int algebraId, const int operatorId );
/*
Returns the address of the cost estimating function of operator
~operatorId~ of algebra ~algebraId~.

*/
  int ConstrNumber( const int algebraId );
/*
Returns the number of constructors of algebra ~algebraId~.

*/
  string Constrs( const int algebraId, const int typeId );
/*
Returns the name of type constructor ~typeId~ of algebra ~algebraId~.

*/
  ListExpr Props( const int algebraId, const int typeId );
/*
Returns the type properties  of type constructor ~typeId~ of algebra
~algebraId~ as a nested list expression.

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
                SmiRecord& valueRecord,
                const ListExpr typeInfo, Word& value );
/*
Open objects of type as constructed by type constructor 
~typeId~ of algebra ~algebraId~.
Return "true"[4], if the operation was successful.

*/
  bool SaveObj( const int algebraId, const int typeId,
                SmiRecord& valueRecord,
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
  bool PersistModel( const int algebraId, const int typeId,
                     const PersistDirection dir,
                     SmiRecord& modelRecord,
                     const ListExpr typeExpr, Word& model );
/*
Save or restore object model for objects of type as
constructed by type constructor ~typeId~ of algebra ~algebraId~.
Return "true"[4], if the operation was successful.

*/
  InModelFunction
    InModel( const int algebraId, const int typeId );
/*
Returns the address of the model input function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  OutModelFunction
    OutModel( const int algebraId, const int typeId );
/*
Returns the address of the model output function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  ValueToModelFunction
    ValueToModel( const int algebraId, const int typeId );
/*
Returns the address of the value to model mapping function of type
constructor ~typeId~ of algebra ~algebraId~.

*/
  ValueListToModelFunction
    ValueListToModel( const int algebraId, const int typeId );
/*
Returns the address of the value list to model mapping function of type
constructor ~typeId~ of algebra ~algebraId~.

*/
  TypeCheckFunction TypeCheck( const int algebraId,
                               const int typeId );
/*
Returns the address of the type check function of type constructor
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

----	(60 <kind> <type expression>)
----

This means ``kind ~kind~ does not match ~type expression~''.
The second format is:

----	(61 <kind> <errno> ...)
----

This means ``Error number ~errno~ in type expression for kind ~kind~''.
The specific error numbers are defined in the kind checking procedure;
the list may contain further information to describe the error. 

*/
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
/*
Is an array for references to all loaded algebra modules.

*/
  vector<AlgebraLevel>     algType;
/*
Is an accompanying array of level specifications for all loaded
algebra modules.

*/
  multimap<string,TypeCheckFunction> kindTable;
  GetAlgebraEntryFunction getAlgebraEntry;
};

#endif

