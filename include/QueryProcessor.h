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
//[Contents] [\tableofcontents]

1 Header File: Query Processor

August 16 RHG Changed includes to remove indirection.

May 2002 Ulrich Telle Port to C++, integrated descriptive algebra level
and function mapping.

1.1 Overview

1.2 Imports and Types

*/

#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

#include "AlgebraManager.h"
/*
defines the basic types of the query processor
such as ArgVectorPointer, Supplier, Word, Address, etc.

*/
#include "SecondoCatalog.h"
#include "SecondoSystem.h"

struct OpNode;
typedef OpNode* OpTree;

struct VarEntry
{
  int position;
  int funindex;
  ListExpr typeexpr;
};

typedef CTable<VarEntry>  VarEntryCTable;

class QueryProcessor
{
 public:
  QueryProcessor( NestedList* newNestedList,
                  AlgebraManager* newAlgebraManager );
  virtual ~QueryProcessor();

/************************************************************************** 
3.2 Exported Functions and Procedures 

3.2.1 Construction and Execution of an Operator Tree

*/

  void Construct( const AlgebraLevel level,
                  const ListExpr expr, bool& correct,
                  bool& evaluable, bool& defined,
                  bool& isFunction,
                  OpTree& tree, ListExpr& resultType );
/*
Builds an operator tree ~tree~ from a given list expression ~expr~ by
calling the procedures ~annotateX~ and ~subtreeX~. The tree is only
constructed if ~annotateX~ does not find a type error. If there is no
error, then ~correct~ is TRUE, the tree is returned in ~tree~ and the
result type of the expression in ~resultType~. In addition, for a
descriptive query (~level = descriptive~), models are evaluated and
stored in the tree. If there is a type error, ~correct~ is set to FALSE
and ~resultType~ contains a symbol ~typeerror~. 

If there is an  object with undefined value mentioned in the query, then
~defined~ is FALSE.

Even if there is no type error, a query may not be evaluable, for example,
if the outermost operator produces a stream, or the query is just an
argument list. The query processor may also view the query as an argument
list, if the root operator is not recognized (an error in the query).
Therefore, ~construct~ returns in ~evaluable~, whether the constructed
tree can indeed be evaluated.

Finally, it is returned in ~isFunction~ whether the tree represents an
abstraction. In this case, it is not evaluable, but we may want to store
the function in a database object. 

*/
  void Eval( const OpTree tree, Word& result,
             const int message );
/*
Traverses the operator tree ~tree~ calling operator implementations for
each node, and returns the result in ~result~. The ~message~ is OPEN,
REQUEST, or CLOSE and is used only if the root node produces a stream.

*/
  void EvalModel( const OpTree tree, Word& result );
/*
Traverses the operator tree ~tree~ calling operator model mapping
functions for each node, and returns the result in ~result~and stores it
in ~subtreeModel~. This is similar to ~eval~, but we do not need to
handle stream evaluation. 

*/

  void Destroy( OpTree& tree, const bool destroyRootValue );  
/*
Deletes an operator tree object. If ~DestroyRootValue~ is ~FALSE~, the
result value stored in the root node is not deleted.

*/

/************************************************************************** 
3.2.2 Handling of Parameter Functions and Stream Operators

*/
  ArgVectorPointer Argument( const Supplier s );
/*
Returns for a given supplier ~s~ a pointer to its argument vector. Arguments
can be set by writing into the fields of this argument vector. 

*/

  void Request( const Supplier s, Word& word );
/*
Calls the parameter function (to which the arguments must have been supplied before). The result is returned in ~result~. 

*/
  bool Received( const Supplier s );
/*
Returns ~true~ if the supplier responded to the previous ~request~ by a
~yield~ message; ~false~ if it responded with ~cancel~.

*/
  void Open( const Supplier s );
/*
Changes state of the supplier stream to ~open~.

*/
  void Close( const Supplier s );
/*
Changes state of the supplier stream to ~closed~. No effect, if the stream
is closed already.

*/
  void RequestModel( const Supplier s, Word& result );
/*
Calls the parameter function of a model mapping function (to which the
arguments must have been supplied before). The result is returned in
~result~. This one is used for model evaluation.

*/
  Supplier GetSupplier( const Supplier s, const int no );
/*
From a given supplier ~s~ that must represent an argument list, get its son
number ~no~. Can be used to traverse the operator tree in order to access
arguments within (nested) argument lists. Values or function or stream
evaluation can then be obtained from the returned supplier by the usual
calls to ~request~ etc.

*/
  Word ResultStorage( const Supplier s );
/*
For each operator in an operator tree, the query processor allocates a
storage block for the result value (which it also destroys after
execution of the query). The operator's evaluation function can call
this procedure ~resultStorage~ to get the address of that storage block.
As a parameter ~s~, the operator's node address has to be given which is
passed to the evaluation function in parameter ~opTreeNode~. 

*/
  int GetNoSons( const Supplier s );
/*
Returns the number of sons of the operator node ~s~ of the operator
tree. 

*/
  ListExpr GetType( const Supplier s );
/*
Returns the type expression of the node ~s~ of the operator tree.

*/

/*
3.2.3 Procedures Exported for Testing Only

*/
  ListExpr AnnotateX( const AlgebraLevel level,
                      const ListExpr expr, bool& defined );
/*
Annotate query expression of algebra at level ~level~. Create tables for
variables, reset ~valueno~ and ~functionno~, then call ~annotate~.
Parameter ~defined~ tells, whether all objects mentioned in the
expression have defined values. 

*/
  OpTree SubtreeX( const AlgebraLevel level,
                   const ListExpr expr );
/*
Construct an operator tree from ~expr~. Allocate argument vectors for all
functions and then call ~subtree~ to do the job.

*/
  ListExpr ListOfTree( OpTree tree );
/*
Represents an operator tree through a list expression. Used for testing.

*/
 private:
  void GetVariable( const string& name, NameIndex& varnames,
                    const VarEntryCTable& vartable,
                    int& position, int& funindex,
                    ListExpr& typeexpr );
/*
Get for variable ~name~ its ~position~ (number of parameter in the list of
parameters) and the number of the abstraction (function definition) ~funindex~
defining it, as well as the associated ~typeexpr~.

*Precondition*: ~IsVariable(name, varnames)~.

*/
  void EnterVariable( const string& name,
                      NameIndex& varnames,
                      VarEntryCTable& vartable,
                      const int position,
                      const int funindex,
                      const ListExpr typeexpr );
/*
Enter ~position~ (number of parameter), ~funindex~ (number of abstraction definition)
and ~typeexpr~ for the variable ~name~ into tables ~varnames~ and ~vartable~.

*Precondition*: NOT ~IsVariable(name, varnames)~.

*/
  bool IsVariable( const string& name,
                   NameIndex& varnames );
/*
Check whether ~name~ is the name of a variable, that is, occurs in ~varnames~.

*/
  bool IsIdentifier( const AlgebraLevel level,
                     const ListExpr expr,
                     NameIndex& varnames );
/*
~Expr~ may be any list expression. Check whether it is an identifier, that is,
a symbol atom which is not registered as a variable or an operator.

*/
enum QueryProcessorType
       { QP_MAP, QP_FUN, QP_STREAM,
         QP_CONSTANT, QP_OPERATOR, QP_OBJECT, 
         QP_FUNCTION, QP_VARIABLE, QP_IDENTIFIER,
         QP_ABSTRACTION, QP_APPLYOP,
         QP_ARGLIST, QP_APPLYABS, QP_APPLYFUN,
         QP_TYPEERROR, QP_ERROR, QP_APPEND,
         QP_UNDEFINED };
  QueryProcessorType TypeOfSymbol( const ListExpr symbol );
/*
Transforms a list expression ~symbol~ into one of the values of type
~QueryProcessorType~. ~Symbol~ is allowed to be any list. If it is not
one of these symbols, then the value ~error~ is returned.

*/
  ListExpr Annotate( const AlgebraLevel level,
                     const ListExpr expr,
                     NameIndex& varnames,
                     VarEntryCTable& vartable,
                     bool& defined,
                     const ListExpr fatherargtypes );
/*
Annotates a query expression ~expr~ of either the executable or the
descriptive ~level~. Use tables ~varnames~ and ~vartable~ to store
variables occurring in abstractions (function definitions) and to
retrieve them in the function's expression. Return the annotated
expression. 

Parameter ~defined~ is set to FALSE if any object mentioned in the
expression has an undefined value. Parameter ~fatherargtypes~ is used to
implement inference of parameter types in abstractions. When a function
is analyzed by ~annotate-function~, then this list contains the argument
types of the operator to which this function is a parameter. 

*/
  ListExpr AnnotateFunction( const AlgebraLevel level,
                             const ListExpr expr,
                             NameIndex& varnames,
                             VarEntryCTable& vartable,
                             bool& defined,
                             const int paramno,
                             const ListExpr typeList,
                             const ListExpr lastElem,
                             const ListExpr fatherargtypes );
/*
Annotate an abstraction ~expr~ which has the form:

----  (fun (x1 t1) ... (xn tn) e)
----

and return the annotated version:

----  ->  ((none abstraction annotate(expr) <functionno>) <type>)
----

where ~type~ is a functional type of the form (map ...). ~Functionno~ is
the index in ~ArgVectors~ used for the argument vector of this function.
Before other actions, its value is assigned to ~localfunctionno~ to
catch the case that ~functionno~ is incremented during annotation of the
function body. 

*/
  bool IsCorrectTypeExpr( const AlgebraLevel level,
                          const ListExpr expr );
  OpTree Subtree( const AlgebraLevel level,
                  const ListExpr expr );
/*
Construct operator tree recursively for a given annotated ~expr~. See
~Annotate~ and ~AnnotateFunction~ for the possible structures to be processed.

*/
  void AllocateValuesAndModels( int idx );
  void AllocateArgVectors( int idx );
  SecondoCatalog* GetCatalog( const AlgebraLevel level )
  {
    return (SecondoSystem::GetCatalog( level ));
  };

  NestedList*     nl;
  AlgebraManager* algebraManager;
  
  int valueno;
  int functionno;
  bool testMode;

  vector<Word> values; // MAXVALUE = 200
  vector<Word> models;
  vector<ArgVectorPointer> argVectors; // MAXFUNCTIONS = 30
};

#endif
