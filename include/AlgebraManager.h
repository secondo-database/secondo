/*
//[ue] [\"{u}]

1 Header File: AlgebraManager

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

1.1 Overview
 
The ~Secondo~ algebra manager is responsible for registering and initializing
all specified algebra modules and provides interface functions for the
query processor and the catalog functions to access the type constructors
and operators of each registered algebra module.

The ~Secondo~ system needs to know about the algebra modules available
at run time. Therefore the developer has to specify which algebra modules
are to be included in the build of the system. The list of all available
algebra modules is kept in the source file ~AlgebraList.i~. Each algebra
can be flagged whether it should be included or not.

Currently the list of algebras to be included must be available at link
time, although an algebra module may be a shared library which is loaded
dynamically into memory only when the ~Secondo~ server is started. To
build an algebra modula as a shared library has the advantage that one
does not need to rebuild the ~Secondo~ system when only the
implementation of the algebra modula changes. 

In principal it would be possible to load algebra modules at runtime which
are not known at link time. But there are at least two disadvantages:

  1 it would restrict the user to command specifications in nested list form,
because the code of the ~Secondo~ parser depends on algebra specifications, and

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

An algebra module offers for each type constructor up to six generic
operations: 

  * ~Create~, ~Destroy~. Allocate/deallocate internal memory. Not needed
for types represented in a single word of storage. 

  * ~Open~, ~Close~. Only needed, if the type is ~independently~
persistent. For example, they will be needed for relations or object
classes, but not for tuples or atomic values (if we decide that the
latter are not independently persistent). 

  * ~In~, ~Out~. Map a nested list into a value of the type and vice
versa. Used for delivering a value to the application (among other
things). 

Every value of every type has a representation as a single WORD of
storage. For a simple type, this can be all. For more complex types, it
may be a pointer to some structure. For persistent objects (that are
currently not in memory) it can be an index into a catalog which tells
where the object is on disk. 

If a type (constructor) has a model, then the algebra module offers
three further operations: 

  * ~InModel~. Create a model data structure from a nested list. 

  * ~OutModel~. Create a nested list representation from the model data
structure 

  * ~ValueToModel~. Create a model data structure from a value. 

  * ~ValueListToModel~.

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

const int MAXARG = 10;
/*
is the maximal number of arguments for one operator

*/

const int OPEN    = 1;
const int REQUEST = 2;
const int CLOSE   = 3;
const int YIELD   = 4;
const int CANCEL  = 5;
/*
are constants for stream processing.

*/

enum PersistDirection { ReadFrom, WriteTo };

/*
1.5 Types

*/

typedef Word ArgVector[MAXARG];
typedef ArgVector* ArgVectorPointer;
/*
are the types for generic argument vectors for algebra functions.

*/

/*
The type for references to a supplier of information of the operator tree:

*/
typedef Address Supplier;

/*
The type of an evaluation function:

*/
typedef int (*ValueMapping)( ArgVector args, Word& result,
                             int msg, Word& local,
                             Supplier tree );

/*
The type of a type mapping procedure:

*/
typedef ListExpr (*TypeMapping)( ListExpr typeList );

/*
The type of a cost mapping procedure:

*/
typedef TypeMapping CostMapping;

/* 
The type of a selection function:

*/
typedef int (*SelectFunction)( ListExpr typeList );

/*
The type of a function for object value and model persistence:

*/
typedef bool (*PersistFunction)( PersistDirection dir,
                                 SmiRecord& valueRecord,
                                 const string& type, Word& value );

/*
The types of model mapping functions and of ~in~ and ~out~ functions for
models:

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
The next types are used for creating, deleting and initializing the
algebra objects or components of the objects and for appending new
subobjects.

This shows also the types of the generic functions for the type constructors.
This is not yet satisfactory, will be revised.

*/
typedef Word (*InObject)( const ListExpr numType,
                          const ListExpr valueList,
                          const int errorPos,
                          ListExpr& errorInfo,
                          bool& correct );

typedef ListExpr (*OutObject)( const ListExpr numType,
                               const Word object );

typedef Word (*ObjectCreation)( const int size );

typedef void (*ObjectDeletion)( Word& object ); 

typedef void* (*ObjectCast)(void*);

/*
Type Checking functions, one for each type constructor:

*/
typedef bool (*TypeCheckFunction)( const ListExpr type,
                                   ListExpr& errorInfo );

/*
Type Property functions, one for each type constructor:

*/
typedef ListExpr (*TypeProperty)();

/*
1.6 Class AlgebraManager

*/

struct AlgebraListEntry;
class Algebra;
class QueryProcessor;
/*
are ~forward declarations~ of used data structures and classes.

*/

class AlgebraManager
{
 public:
  AlgebraManager( NestedList& nlRef );
  virtual ~AlgebraManager();

  void LoadAlgebras();
/*
All existing algebras are loaded into the SECONDO programming interface.
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
returns ~true~, if the algebra module ~algebraId~ is loaded.
Otherwise ~false~ is returned.

*/
  bool IsAlgebraLoaded( const int algebraId,
                        const AlgebraLevel level );
/*
returns ~true~, if the algebra module ~algebraId~ is loaded and
has the specified ~level~. Otherwise ~false~ is returned.

*/
  int CountAlgebra();
  int CountAlgebra( const AlgebraLevel level );
/*
returns the number of loaded algebras, all or only of the specified ~level~.

*/
  bool NextAlgebraId( int& algebraId, AlgebraLevel& level );
/*
returns the identification number ~algebraId~ and the ~level~ of the next
loaded algebra. On the first call ~algebraId~ must be initialized
to zero.

*/
  bool NextAlgebraId( const AlgebraLevel level,
                      int& algebraId );
/*
returns the identification number ~algebraId~ of the next loaded algebra
of the specified ~level~. On the first call ~algebraId~ must be initialized
to zero.

*/
  int OperatorNumber( const int algebraId );
/*
returns the number of operators of algebra ~algebraId~.

*/
  string Ops( const int algebraId, const int operatorId );
/*
returns the name of operator ~operatorId~ of algebra ~algebraId~.

*/
  ListExpr Specs( const int algebraId, const int operatorId );
/*
returns the specification of operator ~operatorId~ of algebra ~algebraId~
as a nested list expression.

*/
  SelectFunction
    Select( const int algebraId, const int operatorId );
/*
returns the address of the select function of operator ~operatorId~ of
algebra ~algebraId~.

*/
  ValueMapping
    Execute( const int algebraId, const int opFunId );
/*
returns the address of the evaluation function of the - possibly 
overloaded - operator ~opFunId~ of algebra ~algebraId~.

*/
  ModelMapping
    TransformModel( const int algebraId, const int opFunId );
/*
returns the address of the model mapping function of the - possibly 
overloaded - operator ~opFunId~ of algebra ~algebraId~.

*/
  TypeMapping
    TransformType( const int algebraId, const int operatorId );
/*
returns the address of the type mapping function of operator
~operatorId~ of algebra ~algebraId~.

*/
  TypeMapping
    ExecuteCost( const int algebraId, const int operatorId );
/*
returns the address of the cost estimating function of operator
~operatorId~ of algebra ~algebraId~.

*/
  int ConstrNumber( const int algebraId );
/*
returns the number of constructors of algebra ~algebraId~.

*/
  string Constrs( const int algebraId, const int typeId );
/*
returns the name of type constructor ~typeId~ of algebra ~algebraId~.

*/
  ListExpr Props( const int algebraId, const int typeId );
/*
returns the type properties  of type constructor ~typeId~ of algebra
~algebraId~ as a nested list expression.

*/
  InObject InObj( const int algebraId, const int typeId );
/*
returns the address of the object input function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  OutObject OutObj( const int algebraId, const int typeId );
/*
returns the address of the object output function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  ObjectCreation
    CreateObj( const int algebraId, const int typeId );
/*
returns the address of the object creation function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  ObjectDeletion
    DeleteObj( const int algebraId, const int typeId );
/*
returns the address of the object deletion function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  ObjectCast Cast( const int algebraId, const int typeId );
/*
returns the address of the type casting function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  bool PersistValue( const int algebraId, const int typeId,
                     const PersistDirection dir,
                     SmiRecord& valueRecord,
                     const string& type, Word& value );
  bool PersistModel( const int algebraId, const int typeId,
                     const PersistDirection dir,
                     SmiRecord& modelRecord,
                     const string& type, Word& model );
/*
return the address of the persistence functions for values and models
respectively of type constructor ~typeId~ of algebra ~algebraId~.

*/
  InModelFunction
    InModel( const int algebraId, const int typeId );
/*
returns the address of the model input function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  OutModelFunction
    OutModel( const int algebraId, const int typeId );
/*
returns the address of the model output function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  ValueToModelFunction
    ValueToModel( const int algebraId, const int typeId );
/*
returns the address of the value to model mapping function of type
constructor ~typeId~ of algebra ~algebraId~.

*/
  ValueListToModelFunction
    ValueListToModel( const int algebraId, const int typeId );
/*
returns the address of the value list to model mapping function of type
constructor ~typeId~ of algebra ~algebraId~.

*/
  TypeCheckFunction TypeCheck( const int algebraId,
                               const int typeId );
/*
returns the address of the type check function of type constructor
~typeId~ of algebra ~algebraId~.

*/
  bool CheckKind( const string& kindName,
                  const ListExpr type,
                  ListExpr& errorInfo );

/*
CheckKind checks if ~type~ is an element of kind ~kindName~.

First parameter is the type expression to be checked. Second parameter
is a list of which every element is an error message (also a list). The
procedure returns TRUE if the type expression is correct. Otherwise it
returns FALSE and adds an error message to the list. An error message
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
is a referenced to a global nested list container.

*/
  int                      maxAlgebraId;
/*
is the highest algebra id occuring in the list of algebras.

*/
  vector<Algebra*>         algebra;
/*
is an array for references to all loaded algebra modules.

*/
  vector<AlgebraLevel>     algType;
/*
is an accompanying array of level specifications for all loaded
algebra modules.

*/
  multimap<string,TypeCheckFunction> kindTable;
/*
*/
  static AlgebraListEntry  algebraList[];
/*
is the static list of all available algebra modules.

*/
};

typedef Algebra*
          (*AlgebraInitFunction)( NestedList* nlRef,
                                  QueryProcessor* qpRef );
/*
is the prototype of the algebra initialization functions. For each
algebra the algebra manager calls an initialization  function to get
a reference to the algebra and to provide the algebra with references
to the global nested list container and the query processor.

*/

struct AlgebraListEntry
{
  AlgebraListEntry()
    : algebraId( 0 ), algebraName( "" ),
      level( UndefinedLevel ),
      algebraInit( 0 ), useAlgebra( false ) {}
  AlgebraListEntry( const int algId, const string& algName,
                    const AlgebraLevel algLevel,
                    const AlgebraInitFunction algInit,
                    const bool algUse )
    : algebraId( algId ), algebraName( algName ),
      level( algLevel ), algebraInit( algInit ),
      useAlgebra( algUse ) {}
  int                  algebraId;
  string               algebraName;
  AlgebraLevel         level;
  AlgebraInitFunction  algebraInit;
  bool                 useAlgebra;
};
/*
is the type for entries in the list of algebras. Each algebra has a
unique identification number ~algebraId~, a name ~algebraName~ and one
of the following levels

  * ~Descriptive~

  * ~Executable~

  * ~Hybrid~ -- the algebra is ~descriptive~ *and* ~executable~.

Additionally the address of the initialization function of the algebra is
registered. For algebras to be used this forces the linker to include the
algebra module from an appropriate link library.

Finally there is a flag whether this algebra should be included in the
initialization process or not.

*/

#define ALGEBRA_LIST_START \
AlgebraListEntry AlgebraManager::algebraList[] = {

#define ALGEBRA_LIST_END \
  AlgebraListEntry( -1, "", UndefinedLevel, 0, false ) };

#define ALGEBRA_LIST_INCLUDE(ALGNO,ALGNAME,ALGTYPE) \
 AlgebraListEntry( ALGNO, #ALGNAME,\
                   ALGTYPE##Level,\
                   &Initialize##ALGNAME, true ),

#define ALGEBRA_LIST_EXCLUDE(ALGNO,ALGNAME,ALGTYPE) \
 AlgebraListEntry( ALGNO, #ALGNAME,\
                   ALGTYPE##Level, 0, false ),

#define ALGEBRA_PROTO_INCLUDE(ALGNO,ALGNAME,ALGTYPE) \
extern "C" Algebra* \
Initialize##ALGNAME( NestedList* nlRef,\
                     QueryProcessor* qpRef );

#define ALGEBRA_PROTO_EXCLUDE(ALGNO,ALGNAME,ALGTYPE)

/*
These preprocessor macros allow to easily define all available algebras.
The macro ~ALGEBRA\_LIST\_START~ is used exactly once to start the
list, and the macro ~ALGEBRA\_LIST\_END~ is used exactly once to
terminate the list.

The macros ALGEBRA\_PROTO\_INCLUDE and ALGEBRA\_PROTO\_EXCLUDE are used
to create prototypes for the algebra initialization functions, the list
entries for the algebra modules to be included or excluded from loading
by the algebra manager are generated by the macros ALGEBRA\_LIST\_INCLUDE
and ALGEBRA\_LIST\_EXCLUDE.

In the algebra list definition file ~AlgebraList.i~ the developer uses
special versions of these macros, namely ~ALGEBRA\_INCLUDE~ and
~ALGEBRA\_EXCLUDE~. They are expanded to the appropriate prototype
and list entry macros as required. These macros have three parameters:

  1 ~the unique identification number~ which must be a positive integer,
it is recommended but not absolutely necessary to order the entries of the
list in ascending order. No identification number may occur more than once
in the list.

  2 ~the algebra name~ which is used to build the name of the initialization
function: the algebra name is appended to the string "Initialize".

  3 ~the level of the algebra~ which may be one of the following: ~Descriptive~,
~Executable~ or ~Hybrid~.

As mentioned above the list of algebras is specified in the source file
~AlgebraList.i~ which is included by the algebra manager source file.

Example:

----  ALGEBRA_INCLUDE(1,StandardAlgebra,Hybrid)
      ALGEBRA_INCLUDE(2,FunctionAlgebra,Executable)
      ALGEBRA_EXCLUDE(3,RelationAlgebra,Hybrid)
----

*/

#endif

