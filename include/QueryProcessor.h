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
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}] [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}] [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}] [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}] [\end{tabular}\end{quote}]
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

September 1996 Claudia Freundorfer

December 23, 1996 RHG Changed procedure  ~construct~ to include checks for
correctness of query and evaluability of the operator tree.

January 3, 1997 RHG Added parameter ~defined~ to procedures ~construct~ and
~annotateX~ to allow checking whether all objects have defined values.

May 4, 1998 RHG Added procedure ~resultStorage~.

May 4, 1998 RHG Added procedure ~getSupplier~.

May 15, 1998 RHG Added procedures ~evalModel~ and ~requestModel~.

June 18, 1998 RHG Added procedures ~getNoSons~ and ~getType~.

January 24, 2001 RHG Change of procedure ~Destroy~ taken from SecondoReference.

January 26, 2001 RHG Added an ~isFunction~ parameter to procedure ~construct~.

May 2002 Ulrich Telle Port to C++, integrated descriptive algebra level
and function mapping.

February 3, 2003 RHG Added QP\_COUNTER and QP\_COUNTERDEF.

August 2004, M. Spiekermann. Private method ~TestOverloadedOperators~ introduced.
This function checks a list of operators if they can map a given input type to 
some other type. The first operator returning not result type "typeerror" will
be used. Moreover, the input and outputs of the type mapping functions can be traced
and if all type mappings fail all possible operators together with their algebra names
are reported.

May 2005, M. Spiekermann. New member variable ~maxmemPerOperator~ inttroduced. This
variable will be set by the SecondoInterface at startup. The value can be defined in
the configuration file. In the future it may be nice if the Query Processor computes
this value based on a global memory limit per query. 

June 2005, M. Spiekermann SetDeletFunction added.


1.1 Overview

This module describes the interface of module ~QueryProcessor~.
The modules offers all the basic operations for executing an executable
command in nested list format. The module ~QueryProcessor~ offers a data
structure to store an operator tree, procedures to build an operator tree
from an access plan given by the optimizer and procedures to execute it by
quasi-coroutines. 

The task of the query processor is to evaluate queries given as nested
list expressions. It divides the task into three steps:

  1 The given query is ~annotated~ which means all the symbols occurring
in the query are analyzed (e.g. objects or operators are looked up in
the system catalog) and the found information is attached to the symbol.
The result is an ~annotated query~, again a nested list. This is done by
method ~Annotate~. 

    This step includes calling ~type mapping functions~ for each
operator of the query. The type mapping function gets a list of argument
types (associated with the arguments of the operator in the query). It
then checks whether argument types are correct. If so, it returns a
result type, otherwise a special symbol ~typeerror~. The result type is
used to annotate the operator application in the query. 

    This step also includes calling a ~selection function~ which maps
an operator into an evaluation procedure. This is used for overloaded
operators, for example + (with four evaluation functions for the
possible combinations of int and real arguments). All operators have
such a selection function, even if they are not overloaded; in that case
the selection function is simple (e.g. identity for the operator number
to function number mapping) and it may be the same for all operators of
an algebra. 

  2 An annotated query is taken and transformed into an ~operator tree~.
Method ~Subtree~ constructs the tree from the annotated query.

  3 Finally, an evaluation method called ~Eval~ traverses the tree,
calling ~evaluation functions~ for the operators there. The evaluation
functions can call (their) parameter functions through special interface
procedures to the query processor. They can also produce or consume
streams in cooperation with the query processor (that is, ~Eval~). 

1.3 Interface methods

The class ~QueryProcessor~ provides the following methods:

[23]    Creation/Deletion/Test & Operator tree & Operator handling     \\
        [--------]
        QueryProcessor          & Construct & Argument \\
        [tilde]QueryProcessor   & Eval      & Request \\
                                & EvalModel & Received \\
                                & Destroy   & Open \\
                                &           & Close \\
                                &           & RequestModel \\
        AnnotateX               &           & GetSupplier \\
        SubtreeX                &           & ResultStorage \\
        ListOfTree              &           & GetNoSons \\
        SetDebugLevel           &           & GetType \\

1.2 Imports and Types

*/

#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

#include "AlgebraManager.h"
/*
defines the basic types of the query processor
such as "ArgVectorPointer"[4], "Supplier"[4], "Word"[4], "Address"[4], etc.

*/
#include "SecondoCatalog.h"
#include "SecondoSystem.h"
#include "MemCTable.h"

struct OpNode;
typedef OpNode* OpTree;
bool IsRootObject( OpTree tree );
bool IsConstantObject( OpTree tree );

struct VarEntry
{
  int position;
  int funindex;
  ListExpr typeexpr;
};

typedef MemCTable<VarEntry>  VarEntryTable;

/************************************************************************** 
3.2 Class "QueryProcessor"[1]

This class implements all methods for the "Secondo"[3] query processor.

*/
class QueryProcessor
{
 public:
  QueryProcessor( NestedList* newNestedList,
                  AlgebraManager* newAlgebraManager );
/*
Creates a query processor instance using the provided nested list container
and algebra manager.

*/
  virtual ~QueryProcessor();
/*
Destroys a query processor instance.

*/
/************************************************************************** 
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
stored in the tree. If there is a type error, ~correct~ is set to "false"[4]
and ~resultType~ contains a symbol ~typeerror~. 

If there is an  object with undefined value mentioned in the query, then
~defined~ is "false"[4].

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
each node, and returns the result in ~result~. The ~message~ is "OPEN"[4],
"REQUEST"[4], or "CLOSE"[4] and is used only if the root node produces a stream.

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
Deletes an operator tree object. If ~DestroyRootValue~ is "false"[4], the
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
Calls the parameter function (to which the arguments must have been supplied
before). The result is returned in ~result~. 

*/
  bool Received( const Supplier s );
/*
Returns "true"[4] if the supplier responded to the previous ~request~ by a
"YIELD"[4] message; "false"[4] if it responded with "CANCEL"[4].

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
>From a given supplier ~s~ that must represent an argument list, get its son
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
  void ResultStorage( const Supplier s, const Word w );
  void SetDeleteFunction( const Supplier s, const ObjectDeletion f );
/*
Some operators do not use the result storage and create their own
storage for the result. This function is used for this case. They
must call the first function ~ResultStorage~ and free it, and then 
set the new one passed in ~w~. Moreover a function for deletion of the
new type can be defined wit SetDeleteFunction

*/
  void DeleteResultStorage( const Supplier s);
/*
Delete the result by calling the appropriate delete function of the
data type stored in the Supplier.  

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
  void SetModified( const Supplier s );
/*
Sets a node ~s~ of the operator tree as modified. The node must be
of type ~Object~.

3.2.2 Dealing with Counters

There is an array of ~NO-COUNTERS~ counters available which can be used during
query processing to count the number of evaluation requests sent to an operator
node (and hence the number of stream elements, e.g. tuples) passing through
this node.

*/

  void ResetCounters();

/*
Set all counters to 0.

*/

  ListExpr GetCounters();

/*
Get the values of all counters in the form of a nested list containing pairs of
the form (counterno, value).

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
  ListExpr ListOfTree( OpTree tree, ostream& os );
/*
Represents an operator tree through a list expression. Used for testing.
Additionally more detailed information will be printed into ~os~.

*/
  void SetDebugLevel( const int level );
/*
Sets the debug level for the query processor. The following levels are defined:

  * *0* -- Debug mode is turned off

  * *1* -- Debug mode is turned on (i.e. results of methods ~AnnotateX~ and ~SubtreeX~ are displayed)

  * *2* -- Debug *and* trace mode are turned on

*/

  void SetMaxMemPerOperator(long value) { maxMemPerOperator = value; }
  long MemoryAvailableForOperator() { return maxMemPerOperator; }


  static bool ExecuteQuery( const string& queryListStr,
                            Word& queryResult);
/*
Executes a Secondo query, given in nested list syntax of type string and returns 
a query result of type Word. This static method can be used for Secondo queries
within an operator implementation of an algebra.

*/
 private:
  void GetVariable( const string& name, NameIndex& varnames,
                    const VarEntryTable& vartable,
                    int& position, int& funindex,
                    ListExpr& typeexpr );
/*
Get for variable ~name~ its ~position~ (number of parameter in the list of
parameters) and the number of the abstraction (function definition) ~funindex~
defining it, as well as the associated ~typeexpr~.

*Precondition*: "IsVariable( name, varnames ) == true"[4].

*/
  void EnterVariable( const string& name,
                      NameIndex& varnames,
                      VarEntryTable& vartable,
                      const int position,
                      const int funindex,
                      const ListExpr typeexpr );
/*
Enter ~position~ (number of parameter), ~funindex~ (number of abstraction definition)
and ~typeexpr~ for the variable ~name~ into tables ~varnames~ and ~vartable~.

*Precondition*: "IsVariable( name, varnames ) == false"[4].

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
         QP_UNDEFINED, QP_COUNTER, QP_COUNTERDEF,
         QP_POINTER };
/*
enumerates the types a symbol may have while annotating an expression.

*/
  QueryProcessorType TypeOfSymbol( const ListExpr symbol );
/*
Transforms a list expression ~symbol~ into one of the values of type
~QueryProcessorType~. ~Symbol~ is allowed to be any list. If it is not
one of these symbols, then the value ~error~ is returned.

*/
  ListExpr Annotate( const AlgebraLevel level,
                     const ListExpr expr,
                     NameIndex& varnames,
                     VarEntryTable& vartable,
                     bool& defined,
                     const ListExpr fatherargtypes );
/*
Annotates a query expression ~expr~ of either the executable or the
descriptive ~level~. Use tables ~varnames~ and ~vartable~ to store
variables occurring in abstractions (function definitions) and to
retrieve them in the function's expression. Return the annotated
expression. 

Parameter ~defined~ is set to "false"[4] if any object mentioned in the
expression has an undefined value. Parameter ~fatherargtypes~ is used to
implement inference of parameter types in abstractions. When a function
is analyzed by ~annotate-function~, then this list contains the argument
types of the operator to which this function is a parameter. 

*/
  ListExpr AnnotateFunction( const AlgebraLevel level,
                             const ListExpr expr,
                             NameIndex& varnames,
                             VarEntryTable& vartable,
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

  ListExpr TestOverloadedOperators( const string& operatorSymbolStr, 
                                    ListExpr opList, 
																		ListExpr typeList,
																		int& alId,
																		int& opId,
																		int& opFunId,
																		const bool checkFunId,
																		const bool traceMode     );
/*
Test all possible type mappings for overloaded operators. The output of the
first successfully applied type mapping will be returned.

*/ 


  void DestroyValuesArray( const AlgebraLevel level );
/*
Destroys the ~values~ array. This function is used when there is a failure
in the Annotate process and the query tree is not built. When the query 
tree is built, the ~Destroy~ function should be called. 

*/
  bool IsCorrectTypeExpr( const AlgebraLevel level,
                          const ListExpr expr );
  OpTree Subtree( const AlgebraLevel level,
                  const ListExpr expr,
                  bool& first,
                  const OpNode* fatherNode = 0 );
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
  
  int  valueno;
  int  functionno;
  bool testMode;
  bool debugMode;
  bool traceMode;
  bool traceNodes;
  map <int, bool> argsPrinted;

  struct ValueInfo
  {
    bool isConstant;
    bool isList;
    int  algId;
    int  typeId;
    Word value;
  };
/*
This ~ValueInfo~ structure will be stored in the ~values~ array defined 
below. The most important information in this structure is the ~value~, 
the others are only used to destroy the array. The flag ~isConstant~ tells 
if the value stored is a constant or an object because they have different 
forms to be destroyed. Constants are deleted because they have been just 
created and objects are only closed because they have been opened. The 
second flag ~isList~ tells if the ~value~ Word is a list an not an address. 
In the case of a list nothing is done in the destruction process. The ~algId~
and ~typeId~ are necessary to call the functions ~delete~ and ~close~
of the type constructor associated with the ~value~.

*/ 
  vector<ValueInfo> values; // MAXVALUE = 200
  vector<Word> models;
  vector<ArgVectorPointer> argVectors; // MAXFUNCTIONS = 30

  static const int NO_COUNTERS = 16;

  int counter[NO_COUNTERS];	

  long maxMemPerOperator;
};

/*

4 Class ErrorReporter

This class contains only static member functions. These functions 
permit reporting an error message (~ReportError~) and
retrieving it (~GetErrorMessage~). Once an error message has been
retrieved, it is removed. If there is no error message, the function
~RemoveErrorMessage~ sets its argument to ~""~. 

An example of the usage of function ~ReportError~ is given in the 
type mapping function of operator ~feed~ in the relational algebra.

*/

class ErrorReporter
{
private:
  static bool receivedMessage;
  static string message;

public:
  static bool FreezeMessage;
  static bool TypeMapError;
  static void Reset() { TypeMapError=false; message=""; }
  static void ReportError(string msg);
  static void ReportError(char* msg);
  static void GetErrorMessage(string& msg);
};


ostream& operator<<(ostream& os, const OpNode& node);

#endif


