/*

1 The Implementation-Module *QueryProcessor*

September 1996 Claudia Freundorfer

October 1996 RHG Module mostly rewritten. In particular new versions of
~annotate~ and ~subtree~. Main reason was to include complete type
checking using type mapping functions of the operators. 

December 20, 1996 RHG Changes to procedure ~subtree~ so that a node
rather than NIL is returned for an identifier.

December 23, 1996 RHG Correction of procedure ~construct~ and addition
of parameter ~evaluable~.

January 3, 1997 RHG Corrected procedure ~annotate~ for objects of atomic
types. Added parameter ~defined~ to procedures ~construct~, ~annotateX~,
and ~annotate~ to allow checking whether all objects have defined
values. 

December 23, 1997 RHG Changed procedure ~annotate~ so that constants in
a query, represented as a pair (typexpr, value), are recognized. Related
change in procedure ~IsCorrectTypeExpr~. 

December 23, 1997 RHG Treatment of standard constants (atoms of type
~IntType~, ~RealType~, etc.) changed so that they are converted using
the ~In~ functions associated with the types called ~int~, ~real~,
~string~, and ~bool~, respectively (which may now come from any algebra
you like). 

December 30, 1997 RHG Introduced type operator support in procedures
~annotate~ and ~annotate-function~.

December 31, 1997 RHG Corrected error handling in ~annotate-function~.

February 13, 1998 RHG Changed return type of operators from ~none~ to
~typeerror~.

April 21, 1998 RHG Changed passing of parameter ~fatherargtypes~ in
procedure ~annotate~ such that partial lists of arguments are passed
arbitrarily deep into subtrees of that operator. Important for the
implementation of implicit parameter functions of ~extend~ and ~groupby~
operators. 

May 4, 1998 RHG Introduced storage allocation and deallocation for
intermediate results through the query processor. Involved changes to
operator tree data structure, to procedure ~subtree~, and to procedure
~eval~ which has to use a different procedure interface for evaluation
functions (type ~ValueMapping~ from ~AlgebraManager2~). Also procedure
~destroy~ was changed (deallocation of objects). 

May 4, 1998 RHG Added procedure ~getSupplier~.

May 15, 1998 RHG Added treatment of models. Models are treated in a
similar way as constants and objects. Procedure ~evalModel~ offered to
compute a model for an operator tree, also ~requestModel~ to be used in
model mapping functions. 

June 18, 1998 RHG Added management of type expressions in operator trees
as well as procedure ~getType~. Also added procedure ~getNoSons~.
Changes in operator tree data structure, in procedure ~ListOfTree~, and
in procedure ~subtree~. 

June 30th, 1999 Stefan Dieker Fixed a bug in ~annotate\_function~,
regarding the global variable ~functionno~: ~functionno~ is increased if
the keyword ~fun~ is encountered. Then the argument definitions and the
body of the function are processed. Eventually ~functionno~ is returned
as part of the annotation. However, if the body of the function again
contains a function, ~functionno~ is again increased during the
annotation of that inner function. Thus the annotation of the outer
function contains a wrong ~functionno~. The problems is solved by
employing a local variable ~localfunctionno~, whose value is not changed
by recursive function annotations. 

May 17th, 2000 Miguel and Stefan. Sometimes the ~destroy~ procedure,
destroying an operator tree, should not delete the result value stored
in the root node of the tree (in particular in executing the ``update''
command). We fixed this by adding the flag ~DestroyRootValue~, which is
set to TRUE in the ``query'' and ``model'' commands, but to FALSE in the
``update'' command. 

January 8, 2001 RHG Replaced ``AlgebraManager2'' by ``ExecAlgManager''

January 10, 2001 RHG Adapted query processor to work for both executable
and descriptive algebras. 

January 24, 2001 RHG Added the changes mentioned above for June 30,
1999, and May 17, 2000, taken from the SecondoReference version. 

January 26, 2001 RHG Added an ~isFunction~ parameter to procedure
~construct~. 

February 2, 2001 RHG Changes to support abstraction application and
function objects.

March 2002 Ulrich Telle. Port to C++

November 26, 2002 RHG Corrected ~AnnotateFunction~ in order to let
function bodies that are two-element lists (in particular applications
of database function objects) be recognized correctly.

\tableofcontents

1.1 Brief Overview

The task of the query processor is to evaluate queries given as nested
list expressions. It divides the task into three steps:

  1 The given query is ~annotated~ which means all the symbols occurring
in the query are analyzed (e.g. objects or operators are looked up in
the system catalog) and the found information is attached to the symbol.
The result is an ~annotated query~, again a nested list. This is done by
procedure ~annotate~ (Section 1.5). 

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
The structure of an operator tree is explained in Section 1.4. Procedure
~subtree~ constructs the tree from the annotated query (Section 1.6). 

  3 Finally, an evaluation procedure called ~eval~ (Section 1.8)
traverses the tree, calling ~evaluation functions~ for the operators
there. The evaluation functions can call (their) parameter functions
through special interface procedures to the query processor described in
Section 1.9. They can also produce or consume streams in cooperation
with the query processor (that is, ~eval~). 

1.1 Imports

*/

using namespace std;

#include "CTable.h"
#include "NameIndex.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "SecondoCatalog.h"
#include "SecondoSystem.h"

/************************************************************************** 
1.2 Constants, Types, Global Data Structures

1.2.1 Query Constants and Objects: Arrays ~values~ and ~models~

The variable ~values~ of type ARRAY OF WORD is used to store the values
of constants given in a query as well as of database objects mentioned.
~valueno~ is the current index of the array. Whenever a new query is
annotated, ~valueno~ is reset to 1. Whenever a constant or object is
recognized during annotation of a query, its value is entered into array
~values~ at index ~valueno~, and ~valueno~ is incremented. 

Similarly, the array ~models~ stores models for the involved objects and
constants.

*/

const int MAXVALUES    = 200;
const int MAXFUNCTIONS =  30;

/*
1.2.2 Tables for Variables

During annotation of a query, one needs to remember types and positions
of parameters of a function abstraction of the form

----        (fun (x1 t1) ... (xn tn) expr)
----

We call the parameters ~variables~ here. Whenever a new abstraction is
analyzed, a global function number ~functionno~ is incremented. During
the analysis of the parameter list, variables are entered into a table
with their name (e.g. x2), the position (e.g. 2), the current function
number ~functionno~ and their type (e.g. tn). In the analysis of the
expression ~expr~ we can then through the name find the information
about such a variable. 

To implement this idea, we use a compact table (CTable) containing
records of type ~varentry~ together with a name index storing the
name/index association. Functions ~EnterVariable~ and ~GetVariable~ are
used to enter and retrieve information about variables. Functions
~IsVariable~ and ~IsIdentifier~ check whether a symbol is a variable (is
in the tables) or is not a variable. 

Note that only types and operations on these tables are described here;
there are no global tables. Tables used in analysis are created either
in ~annotateX~ or when a function object stored in the database is
mentioned in the query; in that case fresh tables are introduced to
analyse such a function definition (stored functions have a scope
different from that of the query). The strategy about scopes of
variables is described in the introduction of procedure ~annotate~. 

*/ 

QueryProcessor::QueryProcessor( NestedList* newNestedList,
	AlgebraManager* newAlgebraManager )
  : nl( newNestedList ), algebraManager( newAlgebraManager ),
    testMode( false ), debugMode( false ), traceMode( false )
{
  values.resize( MAXVALUES );
  models.resize( MAXVALUES );
  argVectors.resize( MAXFUNCTIONS );
}

QueryProcessor::~QueryProcessor()
{
}

void
QueryProcessor::AllocateValuesAndModels( int idx )
{
  int size = values.size();
  if ( idx >= size )
  {
    size += MAXVALUES;
    values.resize( size );
    models.resize( size );
  }
}

void
QueryProcessor::AllocateArgVectors( int idx )
{
  int size = argVectors.size();
  if ( idx >= size )
  {
    size += MAXFUNCTIONS;
    argVectors.resize( size );
  }
}

void
QueryProcessor::GetVariable( const string& name,
                             NameIndex& varnames,
                             const VarEntryCTable& vartable,
                             int& position,                /* out */
                             int& funindex,                /* out */
                             ListExpr& typeexpr        /* out */)
{
/*
Get for variable ~name~ its ~position~ (number of parameter in the list
of parameters) and the number of the abstraction (function definition)
~funindex~ defining it, as well as the associated ~typeexpr~. 

*Precondition*: ~IsVariable(name, varnames)~.

*/
  NameIndex::iterator pos = varnames.find( name );
  Cardinal j = pos->second;
  position = vartable[j].position;
  funindex = vartable[j].funindex;
  typeexpr = vartable[j].typeexpr;
}

void
QueryProcessor::EnterVariable( const string& name,
                               NameIndex& varnames,       /* in/out */
                               VarEntryCTable& vartable,  /* in/out */
                               const int position,
                               const int funindex,
                               const ListExpr typeexpr )
{
/*
Enter ~position~ (number of parameter), ~funindex~ (number of
abstraction definition) and ~typeexpr~ for the variable ~name~ into
tables ~varnames~ and ~vartable~. 

*Precondition*: NOT ~IsVariable(name, varnames)~.

*/
  Cardinal j = vartable.EmptySlot();
  VarEntry& variable = vartable[j];
  variable.position = position;
  variable.funindex = funindex;
  variable.typeexpr = typeexpr;
  varnames[name] = j;
}

bool
QueryProcessor::IsVariable( const string& name, NameIndex& varnames )
{
/*
Check whether ~name~ is the name of a variable, that is, occurs in
~varnames~. 

*/
  return (varnames.find( name ) != varnames.end());
}

bool
QueryProcessor::IsIdentifier( const AlgebraLevel level,
                              const ListExpr expr, NameIndex& varnames )
{
/*
~Expr~ may be any list expression. Check whether it is an identifier,
that is, a symbol atom which is not registered as a variable or an
operator within the algebra at level ~level~. 

*/
  if ( nl->AtomType( expr ) == SymbolType )
  {
    string name = nl->SymbolValue( expr );
    return (!IsVariable( name, varnames ) &&
            !GetCatalog( level )->IsOperatorName( name ));
  }
  else
  {
    return (false);
  }
}

/*
1.2.3 Array of Argument Vectors ~ArgVectors~

The argument vectors associated with function definitions are kept in
this array. Argument vectors can then be referred to by an index
(~functionno~) into this array. When the operator tree is built in
procedure ~subtree~, the Addresses of argument vectors are looked up
here and entered into the operator tree. 

*/

/*
1.2.4 Special Symbols Used in the Query or its Analysis

*/

QueryProcessor::QueryProcessorType
QueryProcessor::TypeOfSymbol( const ListExpr symbol )
{
/*
Transforms a list expression ~symbol~ into one of the values of type
~QueryProcessorType~. ~Symbol~ is allowed to be any list. If it is not
one of these symbols, then the value ~error~ is returned.

*/
  if ( nl->IsAtom( symbol ) && (nl->AtomType( symbol ) == SymbolType) )
  {
    string s = nl->SymbolValue( symbol );
    if      ( s == "map"         ) return (QP_MAP);
    else if ( s == "fun"         ) return (QP_FUN);
    else if ( s == "stream"      ) return (QP_STREAM);
    else if ( s == "constant"    ) return (QP_CONSTANT);
    else if ( s == "operator"    ) return (QP_OPERATOR);
    else if ( s == "object"      ) return (QP_OBJECT);
    else if ( s == "function"    ) return (QP_FUNCTION);
    else if ( s == "variable"    ) return (QP_VARIABLE);
    else if ( s == "identifier"  ) return (QP_IDENTIFIER);
    else if ( s == "abstraction" ) return (QP_ABSTRACTION);
    else if ( s == "applyop"     ) return (QP_APPLYOP);
    else if ( s == "arglist"     ) return (QP_ARGLIST);
    else if ( s == "applyabs"    ) return (QP_APPLYABS);
    else if ( s == "applyfun"    ) return (QP_APPLYFUN);
    else if ( s == "typeerror"   ) return (QP_TYPEERROR);
    else if ( s == "APPEND"      ) return (QP_APPEND);
    else if ( s == "undefined"   ) return (QP_UNDEFINED);
    else                           return (QP_ERROR);
  }
  else
  {
    return (QP_ERROR);
  }
}

/*
1.3 The Operator Tree

Procedure ~subtree~ transforms an annotated query into an operator tree
of the form shown here. 

*/

enum OpNodeType { Object, IndirectObject, Operator };
struct OpNode
{
  bool         evaluable;
  ListExpr     typeExpr;
  AlgebraLevel nodeLevel;
  OpNodeType   nodetype;
  union OpNodeUnion
  {
    struct OpNodeDirectObject
    {
      Word value;
      int  valNo;        /* needed for testing only */
      Word model;
    } dobj;
    struct OpNodeIndirectObject
    {
      ArgVectorPointer vector;
      int funNumber;        /* needed for testing only */
      int argIndex;
    } iobj;
    struct OpNodeOperator
    {
      int              algebraId;
      int              opFunId;
      int              noSons;
      OpTree           sons[MAXARG];
      bool             isFun;
      ArgVectorPointer funArgs;
      int              funNo;        /* needed for testing only */
      bool             isStream;
      Word             local;
      bool             received;
      int              resultAlgId;
      int              resultTypeId;
      Word             resultWord;
      Word             subtreeModel;
    } op;
  } u;
};
/*

The fields of the tree have the following meaning:

  * ~evaluable~: True iff this node is an object, an indirect object, or
an operator which is neither (the root of a subtree representing) a
function argument nor a stream argument. This means the query evaluator
can compute the value of this subtree directly. 

  * ~typeExpr~: Any node (subtree) of an operator tree has an associated
type. This type expression is stored here; it can be looked up by the
evaluation function of the operator belonging to this node via the
procedure ~getType~. 

  * ~nodeLevel~: Is this a node of an operator tree for a ~descriptive~
or ~executable~ expression. 

A node can then have one of three forms. It can represent an object (a
simple value or a pointer to something); in that case 

  * ~value~ contains that object,

  * ~valNo~ is an index into the array ~values~ and ~value~ has been
copied from that entry. Since ~value~ cannot be printed, a procedure
showing the structure of the operator tree (~ListOfTree~, see below)
will print ~valNo~ instead. The entries ~funNumber~ and ~funNo~
explained below play a similar role. 

  * ~model~ contains a model for that object, possibly an undefined
model value. For type constructors, for which models are not (yet)
defined, the value WORD(0) is used to describe the undefined model. If
models exist, they may have their own value to describe an undefined
model (e.g. NIL for a pointer type). 

It can be an ``indirect object'' which is accessible through an argument
vector attached to a subtree representing a function argument: 

  * ~vector~ points to that argument vector,

  * ~funNumber~ is an index into global array ~ArgVectors~; that entry
was used to assign the argument vector here. 

  * ~argIndex~ is the position of the object within the argument vector.

Finally, the node can represent an operator:

  * ~algebraId~ and ~opFunId~ identify the operator's evaluation function,

  * ~noSons~: number of arguments for this operator,

  * ~sons~: pointers to the sons,

  * ~isFun~: true iff the node is the root of a function argument,

  * ~funArgs~: pointer to the argument vector for a function node; only
used, if ~isFun~ is true, 

  * ~funNo~ is also an index into global array ~ArgVectors~; that entry
was used to assign the argument vector here. 

  * ~isStream~: true if this operator produces an output stream,

  * ~local~: used to keep the ~local~ parameter of a stream operator
between calls, 

  * ~received~: true iff the last call of this stream operator returned YIELD.

  * ~resultAlgId~ and ~resultTypeId~ describe the result type of the
operator application. 

  * ~resultWord~: data structure for the result value.

  * ~subtreeModel~: the model resulting from evaluating the model mappings
of this subtree. 

The three kinds of nodes will be represented graphically as follows:



                Figure 1: Three kinds of nodes [Figure1.eps]

For an operator node, the top left field shows the operator rather than
its ~algebraId~ and ~opFunId~. The other fields in the top row are
~noSons~, ~isFun~, ~funArgs~, and ~isStream~; the last five fields are
omitted in this representation. The bottom row, of course, shows the
~sons~ array. 

The structure of the operator tree is illustrated by the representation
of the following executable query: 

----        (filter (feed cities)
                (fun (c city)
                        (> (attribute c pop .)
                                500000)))
----

Here ~attribute~ is an operator with three arguments, namely, a tuple,
an attribute name within that tuple, and a number giving the position of
the attribute within the tuple. However, the user does not have to
supply this number; it will be inferred and added in type checking. This
is what is meant by the dot (the dot is not written, only shown here to
indicate the missing argument). Note that at the executable level one
has to work with the number rather than the name, for two reasons: (1)
the argument types, and hence, the tuple type, are not available any
more at that level, and (2) efficiency. 

The operator tree for this query looks as shown in Figure 2.



                Figure 2: Operator Tree [Figure2.eps]

Here oval nodes represent data objects represented externally from the
operator tree. At the bottom an argument vector is shown. 

The following procedure ~ListOfTree~ is very useful for testing; it maps
a tree into a list expression which we can then print. 

*/

ListExpr
QueryProcessor::ListOfTree( OpTree tree )
{
/*
Represents an operator tree through a list expression. Used for testing.

*/
  ListExpr list, last;
  int i;

  if ( tree == 0 )
  {
    return (nl->SymbolAtom( "NIL" ));
  }
  else
  {
    switch (tree->nodetype)
    {
      case Object:
      {
        return (nl->Cons( nl->SymbolAtom( "Object" ),
                          nl->SixElemList(
                            nl->SymbolAtom( "type" ),
                            tree->typeExpr,
                            nl->SymbolAtom( "evaluable" ),
                            nl->BoolAtom( tree->evaluable ),
                            nl->SymbolAtom( "valNo" ),
                            nl->IntAtom( tree->u.dobj.valNo ) ) ));
      }
      case IndirectObject:
      {
        return (nl->Cons( nl->SymbolAtom( "IndirectObject" ),
                          nl->SixElemList(
                            nl->SymbolAtom( "evaluable" ),
                            nl->BoolAtom( tree->evaluable ),
                            nl->SymbolAtom( "argIndex" ),
                            nl->IntAtom( tree->u.iobj.argIndex ),
                            nl->SymbolAtom( "funNumber" ),
                            nl->IntAtom( tree->u.iobj.funNumber ) ) ));
      }
      case Operator:
      {
        if ( tree->u.op.noSons > 0)
        {
          list = nl->OneElemList( ListOfTree( tree->u.op.sons[0] ) );
          last = list;
          for ( i = 1; i < tree->u.op.noSons; i++ )
          {
            last = nl->Append( last, ListOfTree( tree->u.op.sons[i] ) );
          }
        }
        else
        {
          list = nl->TheEmptyList();
        };
        return (nl->SixElemList(
                  nl->SymbolAtom( "Operator" ),
                  nl->TwoElemList(
                    nl->SymbolAtom( "type" ),
                    tree->typeExpr ),
                  nl->SixElemList(
                    nl->SymbolAtom( "evaluable" ),
                    nl->BoolAtom( tree->evaluable ),
                    nl->SymbolAtom( "algebraId" ),
                    nl->IntAtom( tree->u.op.algebraId ),
                    nl->SymbolAtom( "opFunId" ),
                    nl->IntAtom( tree->u.op.opFunId )),
                  nl->SixElemList(
                    nl->SymbolAtom( "noSons" ),
                    nl->IntAtom( tree->u.op.noSons ),
                    nl->SymbolAtom( "isFun" ),
                    nl->BoolAtom( tree->u.op.isFun ),
                    nl->SymbolAtom( "funNo" ),
                    nl->IntAtom( tree->u.op.funNo ) ),
                  nl->TwoElemList(
                    nl->SymbolAtom( "isStream" ),
                    nl->BoolAtom( tree->u.op.isStream ) ),
                  list ));
      }
    }
  }
  return (nl->SymbolAtom( "ERROR" ));
}
 
/************************************************************************** 
1.4 Annotating the Query: Procedures ~annotate~ and ~annotateX~

*/

ListExpr
QueryProcessor::AnnotateX( const AlgebraLevel level,
                           const ListExpr expr, bool& defined )
{
/*
Annotate query expression of algebra at level ~level~. Create tables for
variables, reset ~valueno~ and ~functionno~, then call ~annotate~.
Parameter ~defined~ tells, whether all objects mentioned in the
expression have defined values. 

*/
  NameIndex varnames;
  VarEntryCTable vartable(20);
  ListExpr list;

  defined = true;

  // Release storage for values and models

  valueno = 0;
  functionno = 0;
  list = Annotate( level, expr, varnames, vartable, defined, nl->TheEmptyList() );

  if ( debugMode )
  {
    cout << endl << "*** AnnotateX Begin ***" << endl;
    nl->WriteListExpr( list, cout );
    cout << endl << "*** AnnotateX End ***" << endl;
  }
  return (list);
}

ListExpr
QueryProcessor::Annotate ( const AlgebraLevel level,
                           const ListExpr expr,
                           NameIndex& varnames,
                           VarEntryCTable& vartable,
                           bool& defined,
                           const ListExpr fatherargtypes ) 
{
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

Annotation is done as follows. A query is a nested list structure. Any
subexpression (atom or sublist) ~s~, is annotated by transforming it
(usually) into a structure (~ann~(~s~) ~type~(~s~)). The following cases
are distinguished: 

  * ~s~ is an empty list. This is interpreted as an empty list of arguments.

----	()        ->        ((none arglist ()) ())
----

  * ~s~ is an integer, real, boolean, or string atom (a constant). 

        The ~In~ function associated with the type constructors ~int~,
~real~, ~string~, and ~bool~ (provided by some algebra) is called to
create a constant of the respective type. 

----	7 	->        ((7 constant 1)        int)

        <value>	->        ((<value> constant <index>) <type>)
----

        Here ~index~ is an index into array ~values~ containing
constants in the form of words. ~Annotate~ enters the value into that
array. 

  * ~s~ is a symbol atom: an operator

----        add        ->        ((add operator 1 4) typeerror)

        <op>        ->        ((<op> operator <algebraId> <operatorId>) typeerror)
----
	Here, once the operators can be overloaded between different algebras, 
the construction of the operator list inside the ~Annotate~ function need to
have more than one entry to the tuple (algebraId, operatorId). In this way, its
representation internally in the function is done with a sublist of this tuples.

        <op>        ->        ((<op> operator ((<alId1> <opId1>) ... (<alIdN> <opIdN>)) ) typeerror)

	After the decision of which operator is suitable for the argument types,
using the ~TranformType~ function, it comes back to the representation with only
one pair (algebraId, operatorId) repeated below.

----	add	->        ((add operator 1 4) typeerror)

	<op>	->        ((<op> operator <algebraId> <operatorId>) typeerror)
----

  * ~s~ is a symbol atom: an object of the database, that ~is not itself
a function~, which means the type of the object does not start ``(map
...)''. 

----	cities		->        ((cities object 7)
                                (rel (tuple ((name string) (pop int))))        )

        <object name>	->
                        (<object name> object <index>)        <type>)
----

        Here ~index~ again refers to the array ~values~.

  * ~s~ is a symbol atom: a function object of the database -- type has
the form ``(map ...)''. The corresponding function definition
(abstraction) is retrieved from the database and annotated recursively. 

----	double		->	((double function annotate((fun (x int) (add x x))))
					(map int int))

        <function name>	-> 	(<function name> function annotate(<abstraction>))
                                	<type>)
----

  * ~s~ is a symbol atom: neither operator nor object, but a variable
defined in some enclosing function definition (= abstraction), which
means it can be found in the table ~variableNames~. 

----	x        	->        ((x variable 3 5) real)

        <var name>	->
                        ((<var name> variable <position> <functionno>) <type>)
----

        Here ~position~ is the relative position of the variable in the
list of arguments of the defining function, and ~functionno~ is a number
identifying that function (see below the strategy for maintaining
function numbers). 


  * ~s~ is a symbol atom: neither operator nor object nor variable.

----	pop		->        ((pop identifier) pop)

	<ident>		->        ((<ident> identifier) <ident>)
----

        This is some unidentified name which must be interpreted by a
type checking function, something like an attribute name. For this
reason, ~not the type, but the actual value~ of the identifier is
returned as a second component. 

  * ~s~ is a nonempty list: first element is the symbol ~fun~.

        Then the whole thing is a function definition (abstraction) of
the form 

----        (fun (x1 t1) (x2 t2) ... (xn tn) expr)
----

        It is annotated by calling the procedure ~annotate-function~
which enters the variable definitions into tables and then calls
~annotate~ again to annotate the expression ~expr~. The result is 

---- 	->        ((none abstraction annotate(expr) <functionno>) <type>)
----
        Note that here ~type~ is the corresponding functional type (map
...). ~Functionno~ is the index in ~ArgVectors~ used for the argument
vector of this function. 

  * ~s~ is a nonempty list: the first element is a type expression.

        Then the list is a pair describing a constant of the type given
as the first element. The type specific ~In~ function is called to
convert the value given as a second element into a word which is entered
into the array ~values~. Hence the annotation is as for constants: 

----	(ccint 7) 	->        (((ccint 7) constant 1) ccint)

        <value>		->        ((<value> constant <index>) <type>)
----

  * ~s~ is a nonempty list: first element is neither the symbol ~fun~,
nor is it a type expression. 

        Then ~annotate~ is called recursively for each element of this
list; the results are collected into a list ~list~. Now we look at the
result. The first element can be: 

  1 an annotated operator ((. operator . .) .)

  2 an annotated function object ((. function .) .)

  3 an annotated abstraction ((. abstraction .) .)

  4 something else, that is, a constant, a DB object, a variable, an
identifier, or an empty list. 

Case (1). In the first case we have an operator application (~op~ ~arg1~
... ~argn~). We first compute the ~resulttype~ by applying the
operator's type mapping function to the types of ~arg1~ ... ~argn~. We
then apply the operator's selection function to determine ~opFunId~, the
number of the operator's evaluation function. This can be different from
the operator number (~opId~) for overloaded operators. 

The result is:

----        ( 	(none applyop (ann(op) ann(arg1) ... ann(argn)))
                <resulttype> 
                <opFunId>)
----

This is the only annotation consisting of three, instead of two,
elements. 

Now we have to discuss a special feature of the query processor together
with an operator's type mapping function, which is the capability to
~determine in the type mapping further arguments to the operator~. This
is generally used in connection with identifiers such as attribute names
occurring in the query. The type mapping understands such identifiers
(finds them in the argument types). The type mapping translates them
into numbers so that the query evaluator (procedure ~eval~) and the
operator's evaluation function can work with them efficiently. 

The technique used is that the type mapping function returns not just a
type ~resulttype~ but a list of the form 

----        (APPEND        (<newarg1> ... <newargn>) <resulttype>)
----

~APPEND~ is a command to the query processor to add the elements of the
following list to the argument list of the operator as if they had been
written in the query. They are then annotated like the original
arguments. The third element ~resulttype~ of the structure is then taken
as the real result type of the operator application. Hence in this case
the result of annotation is 

----        (  (none applyop 
               (ann(op) ann(arg1) ... ann(argn) ann(newarg1) ... ann(newargn))) 
           <resulttype> 
           <opFunId>)
----

An example is the type mapping function of the ~attribute~ operator
which receives as arguments a tuple (type) and an attribute name. 

----        ((tuple ((x1 t1) ... (xn tn))) xi)        -> ti
                                                   (APPEND (i) ti)
----

It returns the type of the attribute together with a request to the
query processor to add the index of the attribute to the arguments. For
example, assume that the attribute name refers to the first attribute
and this is of type ~string~. Then the type mapping returns: 

----        (APPEND (1) string)
----

The query processor, more precisely the procedure ~annotate~, will
produce the annotation for the constant 1, append it to the list of
annotated arguments, and then use ``string'' as the result type of the
~attribute~ operation. 

Case (2). This is an application of a database function object. The
argument types of the function object are checked against the types of
the actual arguments and the result type of the function is returned.
The result is 

----        ((none applyfun (ann(function) ann(arg1) ... ann(argn))) <resulttype>)
----


Case (3). This is an application of an abstraction. Like the previous
case, argument types are checked and the result type of the abstraction
is returned. 

----        ((none applyabs (ann(abstraction) ann(arg1) ... ann(argn)))<resulttype>)
----


Case (4). The whole list is then just a list of expressions (terms). The
result type is just the list of types of the expressions. 

----        (t1 t2 ... tn)   ->  ((none arglist (ann(t1) ann(t2) ... ann(tn)))
					(type(t1) type(t2) ... type(tn)))
----

~Scope and visibility of variables~. We make the following assumptions:
All variables used in a single query (or update) expression are
distinct. An expression may refer to variables of several nested
enclosing functions. Since they are all distinct, they can be maintained
in a single global name index called ~variableNames~ which is valid
during the translation and execution of this query. 

However, there are also function objects stored in the database.
Obviously, one cannot guarantee that the variable names used in their
definitions are distinct from those in a query, since they have been
introduced independently from any query which may use them. But we can
again assume that all variables used within the definition of such a
function are distinct (function definitions may be nested here as well).
Hence we can translate (annotate) the function definition using just one
local table of variables. Either the global table or such a local table
is given as a parameter to ~annotate-function~. 

There is one global variable ~functionno~ which is incremented whenever
a function definition (abstraction) is translated (by
~annotate-function~) during the translation of one query. This number is
entered for a given variable into the variable table. 

In the next step, when the annotated query is transformed into an
operator tree by procedure ~subtree~, then for each used function index
an argument vector is allocated and kept in a global array
~ArgVectors[i]~. A variable annotation is then translated into an
indirect object referring to the argument vector with the corresponding
function index. 

*/

  int alId, opId, position, funindex, opFunId, errorPos; 
  ListExpr first, rest, list, lastElem, typeExpr, typeList, resultType, 
           last, errorInfo, pair, lastType, signature, firstSig, firstType, 
           result, functionList; 
  string name, typeName; 
  bool definedValue, hasNamedType, correct, newOperator;
  Word value, model;

  errorPos = 0;
  errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );

  if ( traceMode )
  {
    cout << "Annotate applied to: " << endl;
    nl->WriteListExpr( expr, cout );
    cout << endl << "argument types passed from father: " << endl;
    nl->WriteListExpr( fatherargtypes, cout );
    cout << endl;
  }
  if ( nl->IsEmpty( expr ) )
  {
    return (nl->TwoElemList(
              nl->ThreeElemList(
                nl->SymbolAtom( "none" ),
                nl->SymbolAtom( "arglist" ),
                nl->TheEmptyList() ), 
              nl->TheEmptyList() ));
  }
  else if ( nl->IsAtom( expr ) )
  {
    switch (nl->AtomType( expr ))
    {
      case IntType:
      {
        AllocateValuesAndModels( valueno );
	if ( level == DescriptiveLevel )
        {
	  models[valueno] = GetCatalog( level )->ValueListToObjectModel(
		nl->SymbolAtom( "int" ), expr, errorPos, errorInfo, correct );
	  /* special treatment of integers at descriptive level; these can
	  be attribute numbers and must be treated as values in that case. */
	  values[valueno].list = nl->IntValue( expr );
        }
	else
        {
          value = GetCatalog( level )->InObject( nl->SymbolAtom( "int" ), expr,
		errorPos, errorInfo, correct );
          values[valueno] = value;
          models[valueno] = GetCatalog( level )->ValueToObjectModel(
		nl->SymbolAtom( "int" ), value );
        }
        valueno++;
        return (nl->TwoElemList(
                  nl->ThreeElemList(
                    expr,
                    nl->SymbolAtom( "constant" ),
                    nl->IntAtom( valueno-1 ) ),
                  nl->SymbolAtom("int") ));
      }
      case RealType:
      {
        AllocateValuesAndModels( valueno );
	if ( level == DescriptiveLevel )
        {
	  models[valueno] = GetCatalog( level )->ValueListToObjectModel(
		nl->SymbolAtom( "int" ), expr, errorPos, errorInfo, correct );
        }
	else
        {
          value = GetCatalog( level )->InObject( nl->SymbolAtom( "real" ), expr,
		errorPos, errorInfo, correct );
          values[valueno] = value;  
          models[valueno] = GetCatalog( level )->ValueToObjectModel(
		nl->SymbolAtom( "real" ), value );
        }
        valueno++;
        return (nl->TwoElemList(
                  nl->ThreeElemList( 
                    expr,
                    nl->SymbolAtom( "constant" ),
                    nl->IntAtom( valueno-1 ) ),
                  nl->SymbolAtom( "real" ) ));
      }
      case BoolType:
      {
        AllocateValuesAndModels( valueno );
	if ( level == DescriptiveLevel )
        {
	  models[valueno] = GetCatalog( level )->ValueListToObjectModel(
		nl->SymbolAtom( "int" ), expr, errorPos, errorInfo, correct );
        }
	else
        {
          value = GetCatalog( level )->InObject( nl->SymbolAtom( "bool" ), expr,
		errorPos, errorInfo, correct );
          values[valueno] = value;  
          models[valueno] = GetCatalog( level )->ValueToObjectModel(
		nl->SymbolAtom( "bool" ), value );
        }
        valueno++;
        return (nl->TwoElemList(
                  nl->ThreeElemList(
                    expr,
                    nl->SymbolAtom( "constant" ),
                    nl->IntAtom( valueno-1 ) ),
                  nl->SymbolAtom( "bool" ) ));
      }
      case StringType:
      {
        AllocateValuesAndModels( valueno );
	if ( level == DescriptiveLevel )
        {
	  models[valueno] = GetCatalog( level )->ValueListToObjectModel(
		nl->SymbolAtom( "int" ), expr, errorPos, errorInfo, correct );
        }
	else
        {
          value = GetCatalog( level )->InObject( nl->SymbolAtom( "string" ), expr,
		errorPos, errorInfo, correct );
          values[valueno] = value;  
          models[valueno] = GetCatalog( level )->ValueToObjectModel(
		nl->SymbolAtom( "string" ), value );
        }
        valueno++;
        return (nl->TwoElemList(
                  nl->ThreeElemList(
                    expr,
                    nl->SymbolAtom( "constant" ),
                    nl->IntAtom( valueno-1 ) ),
                  nl->SymbolAtom( "string" ) ));
      }
      case TextType:
      {
        cerr << "Annotate: textatom in query not allowed !" << endl;
        exit(0);
      }
      case SymbolType:
      {
        name = nl->SymbolValue( expr );
        if ( GetCatalog( level )->IsObjectName( name ) )
        {
          GetCatalog( level )->GetObjectExpr( name, typeName, typeExpr, values[valueno], 
                             definedValue, models[valueno], hasNamedType );

          if ( !definedValue )
          {
            defined = false;
          }
          valueno++;
          if ( nl->ListLength( typeExpr ) > 0 )
          {
            if ( TypeOfSymbol( nl->First( typeExpr ) ) == QP_MAP )
            { /* function object */

              NameIndex newvarnames;
              VarEntryCTable newvartable(20); 
	      functionList = values[valueno-1].list;

	      list = Annotate( level, functionList, newvarnames, newvartable,
                               defined, nl->TheEmptyList() );
	      return (nl->TwoElemList(
                        nl->ThreeElemList(
                          expr,
                          nl->SymbolAtom( "function" ),
                          list ),
                        typeExpr ));
            }
            else
            { /* not a function object */
              return (nl->TwoElemList(
                        nl->ThreeElemList(
                          expr,
                          nl->SymbolAtom( "object" ),
                          nl->IntAtom( valueno-1 ) ),
                        typeExpr ));
            }
          }
          else if ( nl->ListLength( typeExpr ) == -1 )
          {        /* atomic type */
            return (nl->TwoElemList(
                      nl->ThreeElemList(
                        expr,
                        nl->SymbolAtom( "object" ),
                        nl->IntAtom( valueno-1 ) ),
                      typeExpr ));
          }
          else
          {
            cerr << "QueryProcessor: Empty typeexpr for object." << endl;
            return (nl->SymbolAtom( "exprerror" ));
          }

        }
        else if ( GetCatalog( level )->IsOperatorName( name ) )
        {
          ListExpr opList = GetCatalog( level )->GetOperatorIds( name );
          return (nl->TwoElemList(
                    nl->ThreeElemList(
                      expr,
                      nl->SymbolAtom( "operator" ),
                      opList ),
                    nl->SymbolAtom( "typeerror" ) ));
        }
        else if ( IsVariable( name, varnames ) )
        {
          GetVariable( name, varnames, vartable, position, funindex, typeExpr );
          return (nl->TwoElemList(
                    nl->FourElemList(
                      expr,
                      nl->SymbolAtom( "variable" ),
                      nl->IntAtom( position ),
                      nl->IntAtom( funindex ) ),
                    typeExpr ));
        }
        else
        {
          return (nl->TwoElemList(
                    nl->TwoElemList(
                      expr,
                      nl->SymbolAtom( "identifier" ) ),
                    expr ));
        }
      }
      default:
      {
        return (nl->TheEmptyList());
      } 
    } /* CASE */
  }
  else
  {  /* expr is a nonempty list */
    if ( TypeOfSymbol( nl->First( expr ) ) == QP_FUN )
    { /*  an abstraction */
      return (AnnotateFunction( level, expr, varnames, vartable, defined,
              0, nl->TheEmptyList(), nl->TheEmptyList(), fatherargtypes ));
    }
    else if ( IsCorrectTypeExpr( level, nl->First( expr ) ) )
    { /* treatment of user-defined constant */
      if ( level == DescriptiveLevel )
      {
	model = GetCatalog( level )->ValueListToObjectModel( nl->First( expr ),
		nl->Second( expr ), errorPos, errorInfo, correct );
        if ( correct )
        { 
          AllocateValuesAndModels( valueno );
          models[valueno] = value;
          valueno++;
	  return (nl->TwoElemList(
                    nl->ThreeElemList(
                      expr,
                      nl->SymbolAtom( "constant" ),
                      nl->IntAtom( valueno-1 ) ),
                    nl->First( expr ) ));
        }
        else
        {
	  return (nl->TwoElemList(
                    nl->ThreeElemList(
                      expr,
                      nl->SymbolAtom( "constant" ),
                      nl->IntAtom( 0 ) ),
                    nl->SymbolAtom( "typeerror" ) ));
        }
      }
      else /* level = executable */
      {
        value = GetCatalog( level )->InObject( nl->First( expr ), nl->Second( expr ),
                              errorPos, errorInfo, correct );
        if ( correct )
        {
          AllocateValuesAndModels( valueno );
          values[valueno] = value;
          models[valueno] = SetWord( Address( 0 ) );
          valueno++;
          return (nl->TwoElemList(
                    nl->ThreeElemList(
                      expr,
                      nl->SymbolAtom( "constant" ),
                      nl->IntAtom( valueno-1 ) ),
                    nl->First( expr ) ));
        }
        else
        {
          return (nl->TwoElemList(
                    nl->ThreeElemList(
                      expr,
                      nl->SymbolAtom( "constant" ),
                      nl->IntAtom( 0 ) ),
                    nl->SymbolAtom( "typeerror" ) ));
        }
      }
    }
    else
    { /* neither abstraction nor constant. Now recursively annotate
        all elements of this list: */
      first = nl->First( expr );
      rest = nl->Rest( expr );

      pair = Annotate( level, first, varnames, vartable, defined, fatherargtypes );

      /* Check whether the first element is a new operator. In that case
         the current partial list of argument types to that operator has
         to be passed down in 'fatherargtypes' in calls of 'annotate'. */

      newOperator = ((nl->ListLength( pair ) == 2) &&
                     (nl->ListLength( nl->First( pair )) == 3) &&
                     (TypeOfSymbol( nl->Second( nl->First( pair ) ) ) == QP_OPERATOR));
      list = nl->OneElemList( pair );
      lastElem = list;
      typeList = nl->OneElemList( nl->Second( pair ) );
      lastType = typeList;
 
      while (!nl->IsEmpty( rest ))
      {
        if ( newOperator )
        { /* current list of arg types to be used */
          pair = Annotate( level, nl->First( rest ), varnames, vartable, defined, typeList );
        }
        else
        { /* just pass down the inherited list of args */
          pair = Annotate( level, nl->First( rest ), varnames, vartable, defined,
		fatherargtypes );
        }
        lastElem = nl->Append( lastElem, pair );
        lastType = nl->Append( lastType, nl->Second( pair ) );
        rest = nl->Rest( rest );
      }
      last = lastElem;   /* remember the last element to be able to
                            append further arguments, see below */
/* 
At this point, we may have a list ~list~ such as

----
        (((+ operator ((1 6) (7 0))) ()) ((3 ...) int) ((10 ...) int))
----

for a given ~expr~ (+ 3 10).

*/
      first = nl->First( list );                /* first = ((+ operator ((1 6) (7 0))) ()) */
      if ( nl->ListLength( first ) > 0 )
      {
        first = nl->First( first );             /* first = (+ operator ((1 6) (7 0))) */
        if ( nl->ListLength( first ) >= 2 )
        {
          switch (TypeOfSymbol( nl->Second( first ) ))
          {
            case QP_OPERATOR:
            {
              ListExpr opList = nl->Third( first );
              assert( nl->ListLength( opList ) > 0 );

              rest = nl->Rest( list );
              typeList = nl->Rest( typeList );

              do
              {
                alId = nl->IntValue( nl->First( nl->First( opList ) ) );
                opId = nl->IntValue( nl->Second( nl->First( opList ) ) );
                
                /* apply the operator's type mapping: */
                resultType = (algebraManager->TransformType( alId, opId ))( typeList );
 
                opList = nl->Rest( opList );
              }
              while ( !nl->IsEmpty( opList ) && 
                      ( nl->IsAtom( resultType ) && nl->AtomType( resultType ) == SymbolType && nl->SymbolValue( resultType ) == "typeerror" ) );

              /* use the operator's selection function to get the index 
                 (opFunId) of the evaluation function for this operator: */

              opFunId = (algebraManager->Select( alId, opId ))( typeList );
              opFunId = opFunId * 65536 + opId;

              /* Check whether this is a type operator; in that case
                 opFunId will be negative. A type operator does only a type
                 mapping, nothig else; hence it is wrong here and we return
                 type error. */

              if ( opFunId < 0 )
              {
                resultType = nl->SymbolAtom( "typeerror" );
              }

              /* check whether the type mapping has requested to append
                 further arguments: */

              if ( (nl->ListLength( resultType ) == 3) &&
                   (TypeOfSymbol( nl->First( resultType ) ) == QP_APPEND) )
              {
                lastElem = last;
                rest = nl->Second( resultType );
                while (!nl->IsEmpty( rest ))
                {
                  lastElem = 
                    nl->Append( lastElem, 
                                Annotate( level, nl->First( rest ), varnames,
                                          vartable, defined, fatherargtypes ) ); 
                  rest = nl->Rest( rest );
                }
                resultType = nl->Third( resultType );
              }


              ListExpr newList = nl->Cons( nl->TwoElemList(
                                             nl->FourElemList( nl->First( first ),
                                               nl->Second( first ),
                                               nl->IntAtom( alId ),
                                               nl->IntAtom( opId ) ),
                                             nl->Second( nl->First( list ) ) ),
                                           nl->Rest( list ) );          

              return (nl->ThreeElemList(
                        nl->ThreeElemList(
                          nl->SymbolAtom( "none" ),
                          nl->SymbolAtom( "applyop" ),
                          newList ),
                        resultType,
                        nl->IntAtom( opFunId ) ));
            }
            case QP_FUNCTION:
            {
              signature = nl->Rest( nl->Second( nl->First( list ) ) );
              if ( traceMode )
              {
                cout << "signature: ";
                nl->WriteListExpr( signature, cout );
                cout << endl;
              }
              typeList = nl->Rest( typeList );
              if ( traceMode )
              {
                cout << "typeList: ";
                nl->WriteListExpr( typeList, cout );
                cout << endl;
              }
              if ( nl->ListLength( signature ) == (nl->ListLength( typeList ) +1) )
              {
                while (!nl->IsEmpty( typeList ))
                {
                  firstSig = nl->First( signature );
                  signature = nl->Rest( signature );
                  firstType = nl->First( typeList );
                  typeList = nl->Rest( typeList );
                  if ( !nl->Equal( firstSig, firstType ) )
                  {
                    return (nl->SymbolAtom( "exprerror" ));
                  }
                }
                resultType = nl->First( signature );
                if ( traceMode )
                {
                  cout << "resultType: ";
                  nl->WriteListExpr( resultType, cout );
                  cout << endl;
                }
                result = nl->TwoElemList(
                           nl->ThreeElemList(
                             nl->SymbolAtom( "none" ),
                             nl->SymbolAtom( "applyfun" ),
                             list ),
                           resultType );
                if ( traceMode )
                {
                  cout << "result: ";
                  nl->WriteListExpr( result, cout );
                  cout << endl;
                }
                return (result);
              }
              else
              {
                return (nl->SymbolAtom( "exprerror" ));
              }
            }
            case QP_ABSTRACTION:
            {
              signature = nl->Rest( nl->Second( nl->First( list ) ) );
              if ( traceMode )
              {
                cout << "signature: ";
                nl->WriteListExpr( signature, cout );
                cout << endl;
              }
              typeList = nl->Rest( typeList );
              if ( traceMode )
              {
                cout << "typeList: ";
                nl->WriteListExpr( typeList, cout );
                cout << endl;
              }
              if ( nl->ListLength( signature ) == (nl->ListLength( typeList ) +1) )
              {
                while (!nl->IsEmpty( typeList ))
                {
                  firstSig = nl->First( signature );
                  signature = nl->Rest( signature );
                  firstType = nl->First( typeList );
                  typeList = nl->Rest( typeList );
                  if ( !nl->Equal( firstSig, firstType ) )
                  {
                    return (nl->SymbolAtom( "exprerror" ));
                  }
                }
                resultType = nl->First( signature );
                if ( traceMode )
                {
                  cout << "resultType: ";
                  nl->WriteListExpr( resultType, cout );
                  cout << endl;
                }
                result = nl->TwoElemList(
                           nl->ThreeElemList(
                             nl->SymbolAtom( "none" ),
                             nl->SymbolAtom( "applyabs" ),
                             list ),
                           resultType );
                if ( traceMode )
                {
                  cout << "result: ";
                  nl->WriteListExpr( result, cout );
                  cout << endl;
                }
                return (result);
              }
              else
              {
                return (nl->SymbolAtom( "exprerror" ));
              }
            }
            default:
            {  /* we have a list of terms, case (4) above) */
               /* Again extract the list of types. We know the list "list"
                                                            is not empty */
              rest = list;
              return (nl->TwoElemList(
                        nl->ThreeElemList(
                          nl->SymbolAtom( "none" ),
                          nl->SymbolAtom( "arglist" ),
                          list ),
                        typeList ));
            }
          } /* CASE */
        }
        else
        {
          return (nl->SymbolAtom( "exprerror" ));
        }
      }
      else
      {
        return (nl->SymbolAtom( "exprerror" ));
      }
    }
  } /* nonempty list */
  return (nl->SymbolAtom( "exprerror" ));
} // annotate;

ListExpr
QueryProcessor::AnnotateFunction( const AlgebraLevel level,
                                  const ListExpr expr,
                                  NameIndex& varnames,      /* in/out */
                                  VarEntryCTable& vartable, /* in/out */
                                  bool& defined,            /* in/out */
                                  const int paramno,
                                  const ListExpr typeList,
                                  const ListExpr lastElem,
                                  const ListExpr fatherargtypes )
{
/*
Annotate an abstraction ~expr~ which has the form:

----        (fun (x1 t1) ... (xn tn) e)
----

and return the annotated version:

---- 		->        ((none abstraction annotate(expr) <functionno>) <type>)
----

where ~type~ is a functional type of the form (map ...). ~Functionno~ is
the index in ~ArgVectors~ used for the argument vector of this function.
Before other actions, its value is assigned to ~localfunctionno~ to
catch the case that ~functionno~ is incremented during annotation of the
function body. 

To do this, call ~annotate-function~ recursively until ~e~ is reached,
increasing ~paramno~ with each call. When ~e~ is reached annotate it
with function ~annotate~. On the way enter each variable definition into
tables ~varnames~ and ~vartable~ and construct the type expression for
the abstraction in ~typeList~, always appending the next type to
~lastElem~. Meaning of ~defined~ as in ~annotate~. Parameter
~fatherargtypes~ as explained in ~annotate~, contains list of types of
arguments preceding this function argument in an operator application. 

*/
  string name, name2;
  ListExpr annexpr, list, paramtype;
  int localfunctionno;

string xxx;
  localfunctionno = functionno;
  if ( traceMode )
  {
    cout << "AnnotateFunction applied to: " << endl;
    nl->WriteListExpr( expr, cout );
    cout << endl;
  }
  if ( nl->IsEmpty( expr ))
  { 
    cerr << "Error: no expression in function definition." << endl;
    return (nl->TwoElemList(
              nl->SymbolAtom( "functionerror" ),
              nl->SymbolAtom( "typeerror" ) ));
  }
  else if ( nl->ListLength(expr) == 1 )		/* e reached */
  {
    annexpr = Annotate( level, nl->First( expr ), varnames, vartable, 
	defined, fatherargtypes );
                /* "e" reached */
  }
  else if ( nl->IsAtom( nl->First( expr ) ) )
  {
    if ( TypeOfSymbol( nl->First( expr ) ) == QP_FUN )
    { 
      functionno++;
      list = nl->OneElemList( nl->SymbolAtom( "map" ) );
      return (AnnotateFunction( level, nl->Rest( expr ), varnames, vartable,
                                defined, 1, list, list, fatherargtypes ));
    }
    else
    {
      annexpr = Annotate( level, nl->First( expr ), varnames, vartable, defined, 
		fatherargtypes );
                /* "e" reached */
    }
  }
  else if ( nl->ListLength( nl->First( expr )) == 2 )
  {
    if ( IsIdentifier( level, nl->First( nl->First( expr ) ), varnames ) )
    { /* a parameter def. */
      /* check for typename or type operator instead of type expression */
      if ( nl->IsAtom( nl->Second( nl->First( expr ) ) ) &&
          (nl->AtomType( nl->Second( nl->First( expr ) ) ) == SymbolType) )
      { 
        name2 = nl->SymbolValue( nl->Second( nl->First( expr ) ) );
        if ( GetCatalog( level )->MemberType( name2 ) )
        { /* name2 is a type name */
          paramtype = GetCatalog( level )->GetTypeExpr( name2 );
        }
        else if ( GetCatalog( level )->IsOperatorName( name2 ) )
        { /* name2 is a type operator */
          ListExpr opList = GetCatalog( level )->GetOperatorIds( name2 );
          ListExpr first = nl->First( opList );
          ListExpr rest = nl->Rest( opList );
          int alId = nl->IntValue( nl->First( first ) ),
              opId = nl->IntValue( nl->Second( first ) );

          paramtype = (algebraManager->TransformType( alId, opId ))( nl->Rest( fatherargtypes ) );

          while ( ( nl->IsAtom( paramtype ) && nl->AtomType( paramtype ) == SymbolType && nl->SymbolValue( paramtype ) == "typeerror" ) &&
                  !nl->IsEmpty( rest )  ) 
          {
            first = nl->First( rest );
            rest = nl->Rest( rest );

            alId = nl->IntValue( nl->First( first ) );
            opId = nl->IntValue( nl->Second( first ) );

            paramtype = (algebraManager->TransformType( alId, opId ))( nl->Rest( fatherargtypes ) );
          }
        }
        else
        { 
          paramtype = nl->Second( nl->First( expr ) );
        }
      }
      else
      {
        paramtype = nl->Second( nl->First( expr ) );
      }
      if ( IsCorrectTypeExpr( level, paramtype ) )
      {
        name = nl->SymbolValue( nl->First( nl->First( expr ) ) );
        /* IsIdentifier has checked that name is not a variable yet,
           hence ! IsVariable(name, varnames) is ensured */

        EnterVariable( name, varnames, vartable, paramno, localfunctionno,
                       paramtype );
        list = nl->Append( lastElem, paramtype );
        return (AnnotateFunction( level, nl->Rest( expr ), varnames, vartable, defined,
                                  paramno+1, typeList, list, fatherargtypes ));
      }
      else
      {
        cerr << "Error: wrong parameter type." << endl;
        return (nl->TwoElemList(
                  nl->SymbolAtom( "functionerror" ),
                  nl->SymbolAtom( "typeerror" ) ));
      }
    }
    else
    {
        cerr << "Error in AnnotateFunction: branch should never be reached." << endl;
        return (nl->TwoElemList(
                  nl->SymbolAtom( "functionerror" ),
                  nl->SymbolAtom( "typeerror" ) ));
    }
  }
  else
  {
        cerr << "Error in AnnotateFunction: branch should never be reached." << endl;
        return (nl->TwoElemList(
                  nl->SymbolAtom( "functionerror" ),
                  nl->SymbolAtom( "typeerror" ) ));
  }
  list = nl->Append( lastElem, nl->Second( annexpr ) );
  return (nl->TwoElemList(
            nl->FourElemList(
              nl->SymbolAtom( "none" ),
              nl->SymbolAtom( "abstraction" ),
              annexpr, 
              nl->IntAtom( localfunctionno ) ),
            typeList ));
}

bool
QueryProcessor::IsCorrectTypeExpr( const AlgebraLevel level,
                                   const ListExpr expr )
{
  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  return (GetCatalog( level )->KindCorrect( expr, errorInfo ));
}

/*****************************************************************************
3.6 Building an Operator Tree from an Annotated Query: Procedures
~subtree~, ~subtreeX~ 

*/

OpTree
QueryProcessor::SubtreeX( const AlgebraLevel level,
                          const ListExpr expr )
{
  AllocateArgVectors( functionno );
  for ( int i = 0; i < functionno; i++)
  {
    argVectors[i] = (ArgVectorPointer) new ArgVector;
    for ( int j = 0; j < MAXARG; j++ )
    {
      (*argVectors[i])[j].addr = 0;
    }
  }
  
  OpTree resultTree = Subtree( level, expr );
  if ( debugMode )
  {
    cout << endl << "*** SubtreeX Begin ***" << endl;
    ListExpr treeList = ListOfTree( resultTree );
    nl->WriteListExpr( treeList, cout );
    nl->Destroy( treeList );
    cout << endl << "*** SubtreeX End ***" << endl;
  }
  return (resultTree);
}

OpTree
QueryProcessor::Subtree( const AlgebraLevel level,
                         const ListExpr expr )
{
  OpTree node;
  ListExpr list;
  string typeName;
string xxx, yyy, zzz;

  if ( testMode )
  {
    if ( !((nl->ListLength( expr ) >= 2) &&
           (nl->ListLength( nl->First( expr )) >= 2)) )
    {
      cerr << "subtree: error in annotated expression" << endl;
      exit( 0 );
    }
  }
  if ( traceMode )
  {
    cout << "subtree applied to: " << endl;
    nl->WriteListExpr( expr, cout );
    cout << endl << "TypeOfSymbol applied to <";
    nl->WriteListExpr( nl->Second( nl->First( expr ) ), cout );
    cout << ">" << endl;
  }

  switch (TypeOfSymbol( nl->Second( nl->First( expr ) ) ))
  {
    /* possible is:
         constant        object                operator
         variable        applyop               abstraction
         identifier      arglist               function
         applyabs        applyfun                        */

    case QP_CONSTANT:
    case QP_OBJECT:
    {
      node = new OpNode;
      node->evaluable = true;
      node->typeExpr = nl->Second( expr );
      node->nodeLevel = level;
      node->nodetype = Object;
      node->u.dobj.valNo = nl->IntValue( nl->Third( nl->First( expr ) ) );
      node->u.dobj.value = values[node->u.dobj.valNo];
      node->u.dobj.model = models[node->u.dobj.valNo];
      return (node);
    }
    case QP_OPERATOR:
    {
      node = new OpNode;
      node->evaluable = true;
      node->typeExpr = nl->Second( expr );
      node->nodeLevel = level;
      node->nodetype = Operator;
      node->u.op.algebraId = nl->IntValue( nl->Third( nl->First( expr ) ) );
      node->u.op.opFunId = nl->IntValue( nl->Fourth( nl->First( expr ) ) );
        /* next fields may be overwritten later */
      node->u.op.noSons = 0;
      node->u.op.isFun = false;
      node->u.op.funNo = 0;
      node->u.op.isStream = false;
      node->u.op.resultAlgId = 0;
      return (node);
    }
    case QP_VARIABLE:
    {
      node = new OpNode;
      node->evaluable = true;
      node->typeExpr = nl->Second( expr );
      node->nodeLevel = level;
      node->nodetype = IndirectObject;
      node->u.iobj.funNumber = nl->IntValue( nl->Fourth( nl->First( expr ) ) );
      node->u.iobj.vector = argVectors[node->u.iobj.funNumber-1]; // *** -1 added
      node->u.iobj.argIndex = nl->IntValue( nl->Third( nl->First( expr ) ) );
      return (node);
    }
    case QP_APPLYOP:
    {
      node = Subtree( level, nl->First( nl->Third( nl->First( expr ) ) ) );
      node->evaluable = true;
      node->typeExpr = nl->Second( expr );
      node->u.op.opFunId = nl->IntValue( nl->Third( expr ));
      node->u.op.noSons = 0;
      list = nl->Rest( nl->Third( nl->First( expr ) ) );
      while ( !nl->IsEmpty( list ) )
      {
        node->u.op.sons[node->u.op.noSons] = Subtree( level, nl->First( list ) );
        node->u.op.noSons++;
        list = nl->Rest( list );
      }

      if ( !nl->IsAtom( nl->Second( expr )) &&
          (TypeOfSymbol( nl->First( nl->Second( expr ) ) ) == QP_STREAM) )
      {
        node->u.op.isStream = true;
        node->evaluable = false;
      }

      /* Create data structure for result value: */

      if ( level == ExecutableLevel )
      {
        if ( !node->u.op.isStream )
        { 
          GetCatalog( level )->LookUpTypeExpr( nl->Second( expr ), typeName,
                                               node->u.op.resultAlgId,
                                               node->u.op.resultTypeId );
        }
        else
        {
          GetCatalog( level )->LookUpTypeExpr( nl->Second( nl->Second( expr )),
                                               typeName,
                                               node->u.op.resultAlgId,
                                               node->u.op.resultTypeId );
        }
        node->u.op.resultWord =
          (algebraManager->CreateObj( node->u.op.resultAlgId,
                                      node->u.op.resultTypeId ))( 0 );
                              /* size parameter currently not used */
      }
      return (node);
    }
    case QP_ABSTRACTION:
    {
      node = Subtree( level, nl->Third( nl->First( expr ) ) );
      node->evaluable = false;
      node->typeExpr = nl->Second( expr );
      node->u.op.isFun = true;
      node->u.op.funNo = nl->IntValue( nl->Fourth( nl->First( expr ) ) );
      node->u.op.funArgs = argVectors[node->u.op.funNo-1]; // *** -1 added
      return (node);
    }
    case QP_IDENTIFIER:
    {
      node = new OpNode;
      node->evaluable = false;
      node->typeExpr = nl->Second( expr );
      node->nodeLevel = level;
      node->nodetype = Object;
      node->u.dobj.valNo = 0;
      return (node);
    }
    case QP_ARGLIST:
    {
      node = new OpNode;
      node->evaluable = false;
      node->typeExpr = nl->Second( expr );
      node->nodeLevel = level;
      node->nodetype = Operator;
      node->u.op.algebraId = 0;                /* special operator [0, 1] means arglist */
      node->u.op.opFunId = 1;
      node->u.op.noSons = 0;
      list = nl->Third( nl->First( expr ) );
      while (!nl->IsEmpty( list ))
      {
        node->u.op.sons[node->u.op.noSons] = Subtree( level, nl->First( list ) );
        node->u.op.noSons++;
        list = nl->Rest( list );
      }
      node->u.op.isFun = false;
      node->u.op.funNo = 0;
      node->u.op.isStream = false;
      node->u.op.resultAlgId = 0;
      return (node);
    }
    case QP_FUNCTION:
    {
      return (Subtree( level, nl->Third( nl->First( expr ) ) ));
    }
    case QP_APPLYABS:
    case QP_APPLYFUN:
    {
      node = new OpNode;
      node->evaluable = true;
      node->typeExpr = nl->Second( expr );
      node->nodeLevel = level;
      node->nodetype = Operator;
      node->u.op.algebraId = 0;   /* special operator [0, 0] means application
                                of an abstraction */
      node->u.op.opFunId = 0;
      node->u.op.noSons = 1;
      node->u.op.sons[0] = Subtree( level, nl->First( nl->Third( nl->First( expr ) ) ) );
								/* the abstraction */
      list = nl->Rest( nl->Third( nl->First( expr ) ) );
      while ( !nl->IsEmpty( list ) )
      {                        /* the arguments */
        node->u.op.sons[node->u.op.noSons] = Subtree( level, nl->First( list ) );
        node->u.op.noSons++;
        list = nl->Rest( list );
      }
      node->u.op.isFun = false;
      node->u.op.funNo = 0;
      node->u.op.isStream = false;
      node->u.op.resultAlgId = 0;
      return (node);
    }
    default:
    { 
      cerr << "subtree: unexpected stuff in annotated expr";
      exit( 0 );
    }
  }
}

/*****************************************************************************
3.6 Building a Tree from a Query: Procedures ~construct~ and ~destroy~

*/
void
QueryProcessor::Construct( const AlgebraLevel level,
                           const ListExpr expr,
                           bool& correct,
                           bool& evaluable,
                           bool& defined,
                           bool& isFunction,
                           OpTree& tree,
                           ListExpr& resultType )
{
/*
Builds an operator tree ~tree~ from a given list expression ~expr~ by
calling the procedures ~annotateX~ and ~subtreeX~. The tree is only
constructed if ~annotateX~ does not find a type error. If there is no
error, then ~correct~ is TRUE, the tree is returned in ~tree~ and the
result type of the expression in ~resultType~. In addition, for a
descriptive query (~level = descriptive~), models are evaluated and
stored in the tree. If there is a type error, ~correct~ is set to FALSE
and ~resultType~ contains a symbol ~typeerror~. 

If there is an object with undefined value mentioned in the query, then
~defined~ is FALSE. 

Even if there is no type error, a query may not be evaluable, for
example, if the outermost operator produces a stream, or the query is
just an argument list. The query processor may also view the query as an
argument list, if the root operator is not recognized (an error in the
query). Therefore, ~construct~ returns in ~evaluable~, whether the
constructed tree can indeed be evaluated. 

Finally, it is returned in ~isFunction~ whether the tree represents an
abstraction. In this case, it is not evaluable, but we may want to store
the function in a database object. 

*/
  ListExpr list;
  Word resultModel;

  list = AnnotateX( level, expr, defined );
  if ( nl->ListLength( list ) >= 2 )
  {
    resultType = nl->Second( list );
    if ( (TypeOfSymbol( resultType ) == QP_TYPEERROR) )
    {
      correct = false;
      evaluable = false;
      isFunction = false;
    }
    else
    {
      correct = true;

      tree = SubtreeX( level, list );

      evaluable = tree->evaluable;
      if ( level == DescriptiveLevel )
      {
        EvalModel( tree, resultModel );
      }
      isFunction = (tree->nodetype == Operator) ? tree->u.op.isFun : false;
    }
  }
  else
  {
    correct = false;
    evaluable = false;
    isFunction = false;
  }
}

void
QueryProcessor::Destroy( OpTree& tree, const bool destroyRootValue )
{
/*
Deletes an operator tree object.

*/
  if ( tree != 0 )
  { 
    switch (tree->nodetype)
    {
      case Operator:
      {
        for ( int i = 0; i < tree->u.op.noSons; i++ )
        {
          Destroy( tree->u.op.sons[i], true );
        } /* for */
        if ( tree->u.op.isFun )
        {
          delete tree->u.op.funArgs;
        }
        if ( (tree->u.op.resultAlgId != 0) && destroyRootValue )
        {
          /* space was allocated for result */
          (algebraManager->DeleteObj( tree->u.op.resultAlgId, tree->u.op.resultTypeId ))
		( tree->u.op.resultWord );
        }
        break;
      }
      default:
        break;
    } /* case */ 
    delete tree;
    tree = 0;
  }
}

/*
6.3 Evaluating an Operator Tree: Procedure ~eval~

*/

void
QueryProcessor::Eval( const OpTree tree, Word& result, const int message )
{
/*
Traverses the operator tree ~tree~ calling operator implementations for
each node, and returns the result in ~result~. The ~message~ is OPEN,
REQUEST, or CLOSE and is used only if the root node produces a stream. 

Still needs to be adapted to handle the special operators [0, 1]
(arglist) and [0, 0] (abstraction application). These lead to errors at
the moment. 

*/
  int i;
  int status;
  ArgVector arg;
  for ( int j = 0; j < MAXARG; j++ )
  {
    arg[j].addr = 0;
  }
  result.addr = 0;

  if ( tree == 0 )
  {
    cerr << "eval called with tree == NIL!" << endl;
    exit( 0 );
  }
  else
  {
    switch (tree->nodetype)
    {
      case Object:
      {
        result = tree->u.dobj.value;
        return;
      }
      case IndirectObject:
      {
        result = (*tree->u.iobj.vector)[tree->u.iobj.argIndex-1]; // *** -1 added
        return;
      }
      case Operator:         /* If this operator is not itself a stream
				operator, then evaluate all subtrees that are not
				functions or streams. Other subtrees are not evaluated,
				just copied to the argument vector. Then call this
				operator's implementation procedure. */
      {
        for ( i = 0; i < tree->u.op.noSons; i++ )
        {
          if ( tree->u.op.sons[i]->evaluable && ( ! tree->u.op.isStream) )
          {
            Eval( tree->u.op.sons[i], arg[i], message );
          }
          else
          {
            arg[i].addr = tree->u.op.sons[i];
          }
        }

        if ( (tree->u.op.algebraId == 0) && (tree->u.op.opFunId == 0) )
        { /* abstraction application */
          ArgVectorPointer absArgs;
          if ( traceMode )
          {
            cout << "The tree is: " << endl;
            nl->WriteListExpr( ListOfTree( tree ), cout );
            cout << endl;
          }
          absArgs = Argument(tree->u.op.sons[0] );
          for ( i = 1; i < tree->u.op.noSons; i++ )
          {
            (*absArgs)[i-1] = arg[i];
            if ( traceMode )
            {
              cout << "argument " << i-1 << " is" << int(arg[i].addr) << endl;
            }
          }
          /*
          */
          Eval( tree->u.op.sons[0], result, message );
          /* cerr << "result is " << result << endl; */
        }
        else 
        { /* normal operator */
          status =
            (algebraManager->Execute( tree->u.op.algebraId, tree->u.op.opFunId ))
                               ( arg, result, message, tree->u.op.local, tree );

          if ( tree->u.op.isStream )
          {
            tree->u.op.received = (status == YIELD);
          }
          else if ( status != 0 )
          {
            cerr << "eval: evaluation of operator failed" << endl;
            exit( 0 ); 
          }
        }
        return;
      }
    }
  }
}

/*
6.3 Computing a Model from an Operator Tree: Procedure ~evalModel~

*/

void
QueryProcessor::EvalModel( const OpTree tree, Word& result )
{
/*
Traverses the operator tree ~tree~ calling operator model mapping
functions for each node, and returns the result in ~result~and stores it
in ~subtreeModel~. This is similar to ~eval~, but we do not need to
handle stream evaluation. 

*/
  int i;
  ArgVector arg;

  if ( tree == 0 )
  {
    cerr << "evalModel called with tree == NIL!" << endl;
    exit( 0 );
  }
  else
  {
    switch (tree->nodetype)
    {
      case Object:
      {
        result = tree->u.dobj.model;
        return;
      }
      case IndirectObject:
      {
        result = (*tree->u.iobj.vector)[tree->u.iobj.argIndex-1]; // *** -1 added
        return;
      }
      case Operator:
      {
/* 
No automatic evaluation is done. Model mapping functions have to get
their arguments by explicitly calling ~requestModel~. This is because
sometimes not the model, but the value is needed, e.g. for attribute
numbers. This has to be decided by the model mapping function which can
either call ~requestModel~ or ~request~, in the latter case to get
normal evaluation. 

*/
        for ( i = 0; i < tree->u.op.noSons; i++ )
        {
          arg[i].addr = tree->u.op.sons[i];
        }
        result = (algebraManager->TransformModel( tree->u.op.algebraId, tree->u.op.opFunId ))
		( arg, tree );
        tree->u.op.subtreeModel = result;
        return;
      }
    }
  }
}

/*
1.1 Procedures for Cooperation with Operator Evaluation Functions

*/
ArgVectorPointer
QueryProcessor::Argument( const Supplier s )
{
/*
Returns for a given supplier ~s~ a pointer to its argument vector.
Arguments can be set by writing into the fields of this argument vector.

*/
  OpTree tree = (OpTree) s;
  return (tree->u.op.funArgs);
}

void
QueryProcessor::Request( const Supplier s, Word& result )
{
/*
Calls the parameter function (to which the arguments must have been
supplied before). The result is returned in ~result~. 

*/
  Eval( (OpTree) s, result, REQUEST );
}

bool
QueryProcessor::Received( const Supplier s )
{
/*
Returns ~true~ if the supplier responded to the previous ~request~ by a
~yield~ message; ~false~ if it responded with ~cancel~. 

*/
  OpTree tree = (OpTree) s;
  return (tree->u.op.received);
}

void
QueryProcessor::Open( const Supplier s )
{
/*
Changes state of the supplier stream to ~open~.

*/
  Word result;
  Eval( (OpTree) s, result, OPEN );
}

void
QueryProcessor::Close( const Supplier s )
{
/*
Changes state of the supplier stream to ~closed~. No effect, if the
stream is closed already. 

*/
  Word result;
  Eval( (OpTree) s, result, CLOSE );
}

void
QueryProcessor::RequestModel( const Supplier s, Word& result )
{
/*
Calls the parameter function of a model mapping function (to which the
arguments must have been supplied before). The result is returned in
~result~. This one is used for model evaluation. 

*/
  EvalModel( (OpTree) s, result );
}

Supplier
QueryProcessor::GetSupplier( const Supplier s, const int no )
{
/*
From a given supplier ~s~ that represents an argument list, get its son
number ~no~. Can be used to traverse the operator tree in order to
access arguments within (nested) argument lists. Values or function or
stream evaluation can then be obtained from the returned supplier by the
usual calls to ~request~ etc. 

*/
  OpTree node;

  node = (OpTree) s;
  if ( (node->u.op.algebraId == 0) && (node->u.op.opFunId == 1) )
  {        /* is an arglist node*/
    if ( no < node->u.op.noSons )
    {
      return (node->u.op.sons[no]);
    }
    else
    {
      cerr << "Error - getSupplier: argument does not exist. " << endl;
      exit( 0 );
    }
  }
  else
  {
    cerr << "Error - getSupplier: trying to get an argument from " 
         << "a node other than an argument list. " << endl;
    exit( 0 );
  }
  return (0);
}

Word
QueryProcessor::ResultStorage( const Supplier s )
{
/*
For each operator in an operator tree, the query processor allocates a
storage block for the result value (which it also destroys after
execution of the query). The operator's evaluation function can call
this procedure ~resultStorage~ to get the Word of that storage block.
As a parameter ~s~, the operator's node Word has to be given which is
passed to the evaluation function in parameter ~opTreeNode~. 

*/
  OpTree tree = (OpTree) s;
  return (tree->u.op.resultWord);
}

int
QueryProcessor::GetNoSons( const Supplier s )
{
/*
Returns the number of sons of the operator node ~s~ of the operator
tree. 

*/
  OpTree tree = (OpTree) s;
  if ( tree->nodetype == Operator )
  {
    return (tree->u.op.noSons);
  }
  else
  {
    cerr << "Error - getNoSons: not an operator node. " << endl;
    exit ( 0 );
  }
}

ListExpr
QueryProcessor::GetType( const Supplier s )
{
/*
Returns the type expression of the node ~s~ of the operator tree. 

*/
  OpTree tree = (OpTree) s;
  return (tree->typeExpr);
}

void
QueryProcessor::SetDebugLevel( const int level )
{
  if ( level <= 0 )
  {
    debugMode = false;
    traceMode = false;
  }
  else if ( level == 1 )
  {
    debugMode = true;
    traceMode = false;
  }
  else
  {
    debugMode = true;
    traceMode = true;
  }
}

bool ErrorReporter::receivedMessage = false;
string ErrorReporter::message = "";

void ErrorReporter::ReportError(string msg)
{
  if(!receivedMessage)
  {
    receivedMessage = true;
    message = msg;
  }
};


void ErrorReporter::ReportError(char* msg)
{
  if(!receivedMessage)
  {
    receivedMessage = true;
    message = msg;
  }
};

void ErrorReporter::GetErrorMessage(string& msg)
{
  receivedMessage = false;
  msg = message;
  message = "";
};
