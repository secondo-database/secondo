/*
//[<] [$<$]
//[>] [$>$]


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

February 2004, Hoffmann added static method ~ExecuteQuery~. This method
executes Secondo queries, given in nested list syntax of C++-type ~string~,
and returns a result of type ~Word~.

July 2004, M. Spiekermann added a consistency check for the result type
calculated by annotate. There should be no typeerror symbol in it. The groupby
operators type mapping caused Secondo to crash since objects of type typeerror
should be created due to a bug in the type mapping. Now a warning will appear
and the operator tree is not constructed. Additonally the trace mode of
annotate was extended. For every operator the input and output for the type
mapping will be displayed. This may help to isolate type mapping errors.

Sept 2004, M. Spiekermann. Bugfix of a segmentation fault arising sometimes in the
~QueryProcessor::Request~ method which was caused by an uninitalized counter number
for applying abstractions or functions in the construction of the operator tree.

June-July 2005, M. Spiekermann. Output operator ``<<'' for type ~OpNode~ implemented.
This will be used to print out a human readable version of the operator tree in
debug mode 2. Moreover GetType() was changed to return in case of a function
supplier only it's result type which may be needed by the operator which is the
root of the function. (For details see comments in the implementation below).

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January - March 2006, M. Spiekermann. Changes for supporting ~streams~ as
arguments to parameter functions. Additionally the documentation of the ~Eval~
function was revised. Further the output of type map errors and the debug modes
have been improved.

March 2007, M. Spiekermann. Operator nodes will now have a member which stores a
pointer to its value mapping function. Example implementation of progress
interruption in the ~eval~ method which must be uncommented in order to use it.

March 2007, RHG Changed the ~eval~ function so that simple arguments (no stream or function) to a stream operator are evaluated only once, for the OPEN message.

March 2007, RHG. Added fields ~selectivity~ and ~predCost~ to the operator tree and ~predinfo~ annotation to set these fields for an operator.


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

  4 More recently, support for ~counters~ has been added. At any node of an
operator tree returning a stream, a counter can be switched on. This allows one
to determine selectivities of predicates while processing a query. Furthermore selectivity and cost for evaluating predicates can be stored in nodes of the operator tree, and set for queries using a ~predinfo~ annotation.

1.1 Imports

*/
#include <stdexcept>


#include "CostEstimation.h"
#include "NameIndex.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "SecondoCatalog.h"
#include "SecondoSystem.h"
#include "LogMsg.h"
#include "CharTransform.h"
#include "NList.h"
#include "Profiles.h"
#include "ProgressView.h"
#include "Progress.h"
#include "Operator.h"
#include "stdio.h"
#include "DotSpec.h"
#include "FileSystem.h"
#include "StandardTypes.h"
#include "../Algebras/FText/FTextAlgebra.h"

// force a failed assertion. This will imply to print a stack trace
#define qp_assert if( RTFlag::isActive("QP:assert") ) assert(false)

// define a runtime exception
#define qp_error(msg) runtime_error( \
              ErrorStr(msg, __FILE__, __LINE__) ); qp_assert

using namespace std;

string
ErrorStr(const string& msg, const string& file, const int& line)
{
  stringstream err;
  err << file << "@" << line << ": " << msg;
  return err.str();
}



/**************************************************************************
1.2 Constants, Types, Global Data Structures

1.2.1 Query Constants and Objects: Array ~values~

The variable ~values~ of type ARRAY OF VALUEINFO is used to store the values
of constants given in a query as well as of database objects mentioned.
~valueno~ is the current index of the array. Whenever a new query is
annotated, ~valueno~ is reset to 1. Whenever a constant or object is
recognized during annotation of a query, its value is entered into array
~values~ at index ~valueno~, and ~valueno~ is incremented.

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

size_t QueryProcessor::instances = 0;
ofstream QueryProcessor::heartbeat_file;






QueryProcessor::QueryProcessor( NestedList* newNestedList,
  AlgebraManager* newAlgebraManager, 
  const size_t _globalMemory /* = DEFAULT_GLOBAL_MEMORY */ )
  : progressView(0),nl( newNestedList ),
    algebraManager( newAlgebraManager ),
    testMode( false ), debugMode( false ),
    traceMode( false ), traceNodes( false ), debugLocal(false),
    debugProgress(false), traceProgress(false),
    globalMemory(_globalMemory)
{
  values.resize( MAXVALUES );
  argVectors.resize( MAXFUNCTIONS,0 );

  // It would be nice if the query processor could manage the
  // memory allocated during processing a query. Operators which
  // have state (e.g. a hashjoin) can ask for memory and the QP
  // answers how much they can use. However, currently we are
  // just defining a maximum per operator.
  maxMemPerOperator = 4 * 1024 * 1024;
  allowProgress=false;
  useHeartbeat = false;
  if(instances==0){
    string filename = "secondo_heartbeat_"
                      + stringutils::int2str(WinUnix::getpid());
    heartbeat_file.open(filename.c_str(), ios::out);
  }
  instances++;
}

QueryProcessor::~QueryProcessor()
{
  if(progressView){
    delete progressView;
    progressView = 0;
  }
  // delete remaining argvectors
  for(unsigned int i = 0; i< argVectors.size(); i++){
    if(argVectors[i]){
       delete argVectors[i];
       argVectors[i] = 0;
    }
  }
  instances--;
  if(instances==0){
     heartbeat_file.close();
     string filename = "secondo_heartbeat_"
                        + stringutils::int2str(WinUnix::getpid());
     FileSystem::DeleteFileOrFolder(filename);     
  }
}

void
QueryProcessor::AllocateValues( int idx )
{
  int size = values.size();
  if ( idx >= size )
  {
    size = idx + MAXVALUES;
    values.resize( size );
  }
}

void
QueryProcessor::AllocateArgVectors( int idx )
{
  int size = argVectors.size();
  if ( idx >= size )
  {
    size = idx + MAXFUNCTIONS;
    argVectors.resize( size,0 );
  }
}

void
QueryProcessor::GetVariable( const string& name,
                             NameIndex& varnames,
                             const VarEntryTable& vartable,
                             int& position,       // out
                             int& funindex,       // out
                             ListExpr& typeexpr ) // out
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
                               NameIndex& varnames,     // in/out
                               VarEntryTable& vartable, // in/out
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
  VarEntry variable;
  variable.position = position;
  variable.funindex = funindex;
  variable.typeexpr = typeexpr;

  vartable.push_back(variable);
  Cardinal j = vartable.size() - 1;
  varnames[name] = j;
}

bool
QueryProcessor::IsVariable( const string& name,
                            NameIndex& varnames )
{
/*
Check whether ~name~ is the name of a variable, that is, occurs in
~varnames~.

*/
  return (varnames.find( name ) != varnames.end());
}

bool
QueryProcessor::IsIdentifier( const ListExpr expr,
                              NameIndex& varnames )
{
/*
~Expr~ may be any list expression. Check whether it is an identifier,
that is, a symbol atom which is not registered as a variable or an
operator.

*/
  if ( nl->AtomType( expr ) == SymbolType )
  {
    string name = nl->SymbolValue( expr );
    return (!IsVariable( name, varnames ) &&
            !GetCatalog()->IsOperatorName( name ));
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
  if ( nl->IsAtom( symbol ) &&
       nl->AtomType( symbol ) == SymbolType )
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
    else if ( s == "counter"     ) return (QP_COUNTER);
    else if ( s == "counterdef"  ) return (QP_COUNTERDEF);
    else if ( s == "predinfo"    ) return (QP_PREDINFO);
    else if ( s == "predinfodef" ) return (QP_PREDINFODEF);
    else if ( s == "memory"     ) return (QP_MEMORY);
    else if ( s == "memorydef"     ) return (QP_MEMORYDEF);
    else if ( s == "pointer"     ) return (QP_POINTER);
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

static int OpNodeIdCtr = 0;
static map<void*, int> OpNodeAddr2Id;

typedef Operator* OpPtr;

enum OpNodeType { Pointer, Object, IndirectObject, Operator };



struct OpNode
{
  bool         evaluable;
  ListExpr     typeExpr;
  OpNodeType   nodetype;
  int          id;
  bool         isRoot;
  struct OpNodeStruct
  {
  /*
     common members of OpNodeDirecObject and OpNodeOperator ~symbol~
     common members of OpNodeInDirecObject and OpNodeOperator ~received~

  */
    ListExpr symbol;
    bool received;
    struct OpNodeDirectObject
    {
      Word value;
      int  valNo; // needed for testing only
      bool isConstant;
      bool isModified;
    } dobj;
    struct OpNodeIndirectObject
    {
      ArgVectorPointer vector;
      int funNumber;            /* needed for testing only */
      int argIndex;

/*
The three attributes below are used in the case when an
operator has a parameter function which may have streams as
arguments. In this case the operator must

*/
    } iobj;
    struct OpNodeOperator
    {
      int algebraId;
      int opFunId;
      int noSons;
      ValueMapping valueMap;
      OpPtr theOperator;
      ArgVector sons;
      ArgVector sonresults;
      bool isFun;
      ArgVectorPointer funArgs;
      int funNo; // needed for testing only
      bool isStream;
      Word local;
      int resultAlgId;
      int resultTypeId;
      Word resultWord;
      ObjectDeletion deleteFun; // substitute for the algebras
                                // delete function
      int counterNo;
      double selectivity;       //these two fields can be used by operators
      double predCost;          //implementing predicates to get and set
                                //such properties, e.g. for progress est.
      bool supportsProgress;
      bool usesMemory;          // true if the operator uses a memory buffer
      size_t memorySize;        // amount of memory assigned to this operator
                                // in MB
      CostEstimation* costEstimation;
      bool argsAvailable;
    } op;
  } u;

/*
Constructor for a proper initialization of an ~OpNode~

*/


OpNode(OpNodeType type) :
  evaluable(false),
  typeExpr(0),
  nodetype(type),
  id(OpNodeIdCtr++),
  isRoot(false)
{
  OpNodeAddr2Id[this] = id;

  switch ( nodetype )
  {
    case Object :
    case Pointer :
    {
      u.symbol = 0;
      u.dobj.value = SetWord(Address(0));
      u.dobj.valNo = 0;
      u.dobj.isConstant = false;
      u.dobj.isModified = false;
      break;
    }
    case IndirectObject :
    {
      u.iobj.vector = 0;
      u.iobj.funNumber = 0;
      u.iobj.argIndex = 0;
      u.received = false;
      break;
    }
    case Operator :
    {
      u.symbol = 0;
      u.op.algebraId = 0;
      u.op.opFunId = 0;
      u.op.noSons = 0;
      u.op.valueMap = 0;
      u.op.theOperator = 0;
      u.op.isFun = false;
      u.op.funArgs = 0;
      u.op.funNo  = 0;
      u.op.isStream = false;
      u.op.local = SetWord(Address(0));
      u.received = false;
      u.op.resultAlgId = 0;
      u.op.resultTypeId = 0;
      u.op.resultWord = SetWord(Address(0));
      u.op.deleteFun = 0;
      u.op.counterNo = 0;
      u.op.selectivity = 0.1;
      u.op.predCost = 0.1;
      u.op.supportsProgress = false;
      u.op.usesMemory = false;
      u.op.memorySize = 0;     
      u.op.costEstimation = 0; 
      u.op.argsAvailable = false;
      break;
    }
    default :
    { assert( false ); }
  }
}

  string symbol() const {

        if ( nl->AtomType(u.symbol) == SymbolType )
        {
          return nl->SymbolValue(u.symbol);
        }
  return nl->ToString(u.symbol);
  }

  string nodeIdStr() const {
    stringstream ss;
    ss << id;
    return ss.str();
  }

  string nodeLabelStr() const {
  stringstream ss;
  switch ( nodetype )
  {
    case Object : {
  if (u.dobj.isConstant) // todo list rep of value
    ss << id << ": const";
  else
    ss << id << ": " << symbol();
  break;
    }
    case Pointer :        { ss << id << ": pointer "; break; }
    case IndirectObject : { ss << id << ": #fun = "
             << u.iobj.funNumber; break; }
    case Operator :       { ss << id << ": " << symbol(); break; }
    default :
    { assert( false ); }
  }
  return ss.str();
  }

  string nodeTypeStr() const {

  switch ( nodetype )
  {
    case Object :         { if (u.dobj.isConstant) return "const";
          return "obj"; }
    case Pointer :        { return "ptr"; }
    case IndirectObject : { return "indObj"; }
    case Operator :       { return "op"; }
    default :
    { assert( false ); }
  }
  }

 /** Checks wether this node is an abstraction **/
 bool isAbstraction(){
     if(nodetype!=Operator){
        return false;
     } else {
       return (u.op.algebraId == 0) && (u.op.opFunId == 0);
     }
  }

 /** checks wther this opnode is a stream operator **/
 bool isStreamOp(){
   return (nodetype==Operator) && u.op.isStream;
 }



private:
  OpNode(){} // bad idea to use an uninitialized object

};


/*
Overloaded "<<" useful to display internal information

*/

ostream& operator<<(ostream& os, const OpNode& node) {

   static NestedList* nl = SecondoSystem::GetNestedList();

   os << "Node " << node.id
      << " - [Adress = " << (void*)(&node) << "]" << endl
      << "  Evaluable = " << node.evaluable
      << "  isRoot = " << node.isRoot << endl
      << "  TypeExpr = " << nl->ToString(node.typeExpr) << endl;


   switch ( node.nodetype )
   {
      case Object :
      {
        os << "  Object ";
        if ( nl->AtomType(node.u.symbol) == SymbolType )
        {
          os << nl->SymbolValue(node.u.symbol);
        }
        else
        {
          os << "(unknown!)";
        }
        os << endl
           << "  value = " << (void*)node.u.dobj.value.addr << endl
           << "  valNo = " << node.u.dobj.valNo << endl
           << "  isConstant = " << node.u.dobj.isConstant << endl;
        break;
      }
      case Pointer :
      {
        os << "Pointer" << endl
           << "  value = " << (void*)node.u.dobj.value.addr << endl
           << "  valNo = " << node.u.dobj.valNo << endl
           << "  isConstant = " << node.u.dobj.isConstant << endl;
        break;
      }
      case IndirectObject :
      {
        os << "Indirect Object" << endl
           << "  vector = " << node.u.iobj.vector << endl
           << "  funNumber = " << node.u.iobj.funNumber << endl
           << "  argIndex = " << node.u.iobj.argIndex << endl
           << "  received = " << node.u.received << endl;
          break;
        }
        case Operator :
        {
          int f=2, f2=4;
          string t2("\t\t");
          string t1("\t");
          os << tab(f) << "Operator ";
          if ( nl->AtomType(node.u.symbol) == SymbolType )
          {
            os << nl->SymbolValue(node.u.symbol);
          }
          else
          {
            if(node.u.op.algebraId==0){
               if(node.u.op.opFunId==0){
                   os << "(special operator: abstraction)";
               } else if(node.u.op.opFunId==1){
                   os << "(special operator: argument list)";
               } else {
                   os << "(unknown!)";
               }
            } else {
              os << "(unknown!)";
            }
          }
          os << endl;

          map<void*, int>::const_iterator it;
          os << tab(f2) << "Node(s)[ ";
          for (int i=0; i<node.u.op.noSons; i++)
          {
             it = OpNodeAddr2Id.find( (void*)node.u.op.sons[i].addr );
             if ( it != OpNodeAddr2Id.end() )
               os << it->second << " ";
             else
               os << "error" << " ";
          }
          os << "]" << endl;

          os << tab(f2) << "Addresses[ ";
          for (int i=0; i<node.u.op.noSons; i++)
             os << (void*)node.u.op.sons[i].addr << " ";
          os << "]" << endl;

          os << tab(f) << "noSons = "
             << node.u.op.noSons
             << t2 << "algebraId = "
             << node.u.op.algebraId
             << t2 << "opFunId = "
             << node.u.op.opFunId << endl
             << tab(f) << "isFun = "
             << node.u.op.isFun
             << t2 << "valueMap = "
             << (void*) node.u.op.valueMap
             << t1 << "funNo = "
             << node.u.op.funNo << endl
             << tab(f) << "isStream = "
             << node.u.op.isStream
             << t2 << "local = "
             << (void*)node.u.op.local.addr
             << t2 << "received = "
             << node.u.received << endl
             << tab(f) << "resultAlgId = "
             << node.u.op.resultAlgId
             << t1 << "resultTypeId = "
             << node.u.op.resultTypeId
             << t1 << "resultWord = "
             << (void*)node.u.op.resultWord.addr
             << t1 << "deleteFun = "
             << (void*)node.u.op.deleteFun << endl
             << tab(f) << "counterNo = "
             << node.u.op.counterNo
             << t2 << "selectivity = "
             << node.u.op.selectivity
             << t1 << "predCost = "
             << node.u.op.predCost << endl
             << tab(f) << "supportsProgress = "
             << node.u.op.supportsProgress 
             << t1 << "usesMemory = "
             << node.u.op.usesMemory
             << t2 << "memorySize = "
             << node.u.op.memorySize << endl;

          break;
        }
        default :
          assert( false );
      }
      return os;
  }


/** Prints out all nodes of the given tree **/

  void print(ostream& os, OpNode* node){
     if(!node){
        os << "the tree is null" << endl;
        return;
     }
     os << (*node) << endl << endl;
     if(node->nodetype == Operator){
        for(int i=0;i<node->u.op.noSons;i++){
           print(os, static_cast<OpNode*>(node->u.op.sons[i].addr));
        }
     } 

  }


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

  A node can then have one of three forms. It can represent an object (a
  simple value or a pointer to something); in that case

    * ~value~ contains that object,

    * ~valNo~ is an index into the array ~values~ and ~value~ has been
  copied from that entry. Since ~value~ cannot be printed, a procedure
  showing the structure of the operator tree (~ListOfTree~, see below)
  will print ~valNo~ instead. The entries ~funNumber~ and ~funNo~
  explained below play a similar role.

    * ~isRoot~ is a flag that tells if the object is the root of a
  query processor tree. It is used to destroy the tree correctly.

    * ~isConstant~ is a flag that tells if the object contains a
  constant value or a variable value.

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

    * ~sonresults~: an argument vector used to store results computed by stream
      operators on the OPEN message,

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

    * ~counterNo~: the number of a counter associated with this node. If this
      number is greater than 0 (between 1 and MAXCOUNTERS), then for every
      evaluation request received by the node the counter ~counterNo~ is
      incremented.

    * ~selectivity~: for an operator implementing a selection or join
      predicate, the selectivity of that predicate. Can be used to
      observe selectivity during query processing, e.g. for progress
      estimation.

    * ~predCost~: similarly the predicate cost, in milliseconds. Selectivity
     and predicate cost are, for example, obtained by the optimizer in
     evaluating a sample query.

    * ~supportsProgress~: whether this operator replies to PROGRESS messages.
       Can be set by an operator's evaluation function via ~enableProgress~.

    * ~usesMemory~: whether this operator uses internal memory (i.e., a 
      large memory buffer.

    * ~memorySize~: the amount of memory allocated to this operator, in MB


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
  indicate the missing argument). Note that one has to work with numbers
  rather than names, for two reasons: (1) the argument types, and hence,
  the tuple type, are not available anymore, and (2) efficiency.

  The operator tree for this query looks as shown in Figure 2.

                  Figure 2: Operator Tree [Figure2.eps]

  Here oval nodes represent data objects represented externally from the
  operator tree. At the bottom an argument vector is shown.

  The following procedure ~ListOfTree~ is very useful for testing; it maps
  a tree into a list expression which we can then print.

  */

  void
  QueryProcessor::BuildDotDescr( void* node, DotSpec& dot)
  {
    OpTree tree = static_cast<OpTree>( node );

    if ( tree == 0 ) {
      return;
    }

    dot.addNode( tree->nodeTypeStr(), tree->nodeIdStr(), tree->nodeLabelStr() );

    if (tree->nodetype != Operator) {
      return;
    }

    if ( tree->u.op.noSons > 0)
    {
      int i = 0;
      for ( i = 0; i < tree->u.op.noSons; i++ )
      {
  dot.addEdge( tree->nodeIdStr(),
           static_cast<OpNode*>( tree->u.op.sons[i].addr )->nodeIdStr() );
  BuildDotDescr( tree->u.op.sons[i].addr, dot );
      }
    }
  }


  ListExpr
  QueryProcessor::ListOfTree( void* node, ostream& os )
  {
  /*
  Represents an operator tree through a list expression. Used for testing.
  Additonally more detailed information is printed to ~os~

  */
    OpTree tree = static_cast<OpTree>( node );
    ListExpr list = nl->TheEmptyList();
    ListExpr last = nl->TheEmptyList();
    int i = 0;

    if ( tree == 0 )
    {
      return (nl->SymbolAtom( "NIL" ));
    }
    else
    {
      os << *tree << endl;
    switch (tree->nodetype)
    {
      case Pointer:
      {
        return
          nl->Cons( nl->SymbolAtom( "Pointer" ),
            nl->SixElemList(
              nl->SymbolAtom( "type" ),
              tree->typeExpr,
              nl->SymbolAtom( "evaluable" ),
              nl->BoolAtom( tree->evaluable ),
              nl->SymbolAtom( "valNo" ),
              nl->IntAtom( tree->u.dobj.valNo ) ) );
      }
      case Object:
      {
        return
          nl->Cons( nl->SymbolAtom( "Object" ),
            nl->SixElemList(
            nl->SymbolAtom( "type" ),
            tree->typeExpr,
            nl->SymbolAtom( "evaluable" ),
            nl->BoolAtom( tree->evaluable ),
            nl->SymbolAtom( "valNo" ),
            nl->IntAtom( tree->u.dobj.valNo ) ) );
      }
      case IndirectObject:
      {
        return
          nl->Cons( nl->SymbolAtom( "IndirectObject" ),
            nl->SixElemList(
              nl->SymbolAtom( "evaluable" ),
              nl->BoolAtom( tree->evaluable ),
              nl->SymbolAtom( "argIndex" ),
              nl->IntAtom( tree->u.iobj.argIndex ),
              nl->SymbolAtom( "funNumber" ),
              nl->IntAtom( tree->u.iobj.funNumber ) ) );
      }
      case Operator:
      {
        if ( tree->u.op.noSons > 0)
        {
          list =
            nl->OneElemList( ListOfTree( tree->u.op.sons[0].addr, os ) );
          last = list;
          for ( i = 1; i < tree->u.op.noSons; i++ )
          {
            last = nl->Append( last,
            ListOfTree( tree->u.op.sons[i].addr, os ) );
          }
        }
        else
        {
          list = nl->TheEmptyList();
        }
        return nl->SixElemList(
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
                 nl->FourElemList(
                   nl->SymbolAtom( "isStream" ),
                   nl->BoolAtom( tree->u.op.isStream ),
                   nl->SymbolAtom( "counterNo" ),
                   nl->IntAtom( tree->u.op.counterNo ) ),
                 list );
      }
    }
  }
  return (nl->SymbolAtom( "ERROR" ));
}


ListExpr GetNodeTypeList(NodeType nt){
  switch(nt){
    case Pointer  : return nl->SymbolAtom("Pointer");
    case Object   : return nl->SymbolAtom("Object");
    case IndirectObject : return nl->SymbolAtom("IndirectObject");
    case Operator       : return nl->SymbolAtom("Operator");
    default             : return nl->SymbolAtom("undefined");
  }
}

int getNoSons(OpTree tree){
  return tree->nodetype == Operator?tree->u.op.noSons:0;
}

OpTree getSon(OpTree tree, int index){
   assert(tree->nodetype==Operator);
   assert(index >=0);
   assert(index < tree->u.op.noSons);
   return  (OpTree) tree->u.op.sons[index].addr;
}


ListExpr GetSimpleList(OpTree tree);

ListExpr GetSonsList(OpTree tree){
   int noSons = getNoSons(tree);
   if(noSons==0){
     return nl->TheEmptyList();
   }
   ListExpr sons = nl->OneElemList(GetSimpleList(getSon(tree,0)));
   ListExpr last = sons;
   for(int i=1; i< noSons;i++){
    last = nl->Append(last, GetSimpleList(getSon(tree,i)));
   }
   return sons; 
}



ListExpr GetRootList(OpTree tree){
  ListExpr ntlist = GetNodeTypeList(tree->nodetype);
  // Build property list
  ListExpr propList = nl->OneElemList(
                            nl->TwoElemList(nl->SymbolAtom("typeExpr"),
                                            tree->typeExpr));
  ListExpr last = propList;
  last = nl->Append(last,nl->TwoElemList(nl->SymbolAtom("evaluable"), 
                                         nl->BoolAtom(tree->evaluable)));  


  switch(tree->nodetype){
     case Object: last = nl->Append(last, nl->TwoElemList(
                                    nl->SymbolAtom("isConstant"),
                                    nl->BoolAtom(tree->u.dobj.isConstant)));
                  last = nl->Append(last, nl->TwoElemList(
                                    nl->SymbolAtom("symbol"),
                                    tree->u.symbol));
                  break;
     case IndirectObject: break; // ???
     case Pointer: break;
     case Operator:
               last = nl->Append(last, nl->TwoElemList(
                                    nl->SymbolAtom("algebraId"),
                                    nl->IntAtom(tree->u.op.algebraId)));
               last = nl->Append(last, nl->TwoElemList(
                                    nl->SymbolAtom("opId"),
                                    nl->IntAtom(tree->u.op.opFunId%65536)));
               last = nl->Append(last, nl->TwoElemList(
                                    nl->SymbolAtom("funId"),
                                    nl->IntAtom(tree->u.op.opFunId/65536)));
               last = nl->Append(last, nl->TwoElemList(
                                       nl->SymbolAtom("opName"),
                                       nl->SymbolAtom(
                                        tree->u.op.theOperator->GetName())));
  }
  return nl->TwoElemList(ntlist, propList);
}


ListExpr GetSimpleList(OpTree tree){
   cout << "called " << __PRETTY_FUNCTION__ << endl;
   if(tree==0){
     return nl->TheEmptyList();
   } else {
     return nl->TwoElemList( GetRootList(tree), 
                             GetSonsList(tree)); 
   }
}



ListExpr QueryProcessor::GetSimpleList(OpTree tree){
  ListExpr otsym = nl->SymbolAtom("optree"); 
  return nl->TwoElemList(otsym, ::GetSimpleList(tree));
}


/*

This function is used to test if the query processor tree is composed only
by an object which is certainly the root node.

*/
bool IsRootObject( OpTree tree )
{
  if( tree->nodetype == Object || tree->nodetype == Pointer )
  {
    if( tree->isRoot != true ) {
      throw qp_error("The given tree is not the root");
    }
    return true;
  }
  return false;
}

/*

This function is used to test if the query processor tree is composed by
only an object which is certainly the root node and if this node is a
constant value.

*Precondition*: ~isRootObject(tree) == true~.

*/
bool IsConstantObject( OpTree tree )
{
  if( IsRootObject( tree ) != true ) {
    throw qp_error("The given tree is not a root object");
  }

  return( tree->u.dobj.isConstant );
}
/**************************************************************************
1.4 Annotating the Query: Procedures ~annotate~ and ~annotateX~

*/

ListExpr
QueryProcessor::AnnotateX( const ListExpr expr, bool& defined )
{
/*
Annotate query expression. Create tables for variables, reset ~valueno~
and ~functionno~, then call ~annotate~. Parameter ~defined~ tells whether
all objects mentioned in the expression have defined values.

*/
  NameIndex varnames;
  VarEntryTable vartable;
  ListExpr list = nl->TheEmptyList();

  defined = true;

  // Release storage for values
  valueno = 0;
  functionno = 0;
  list = Annotate( expr, varnames, vartable, defined,
                   nl->TheEmptyList() );

  if ( debugMode )
  {
    cout << endl << "*** AnnotateX Begin ***" << endl;
    nl->WriteListExpr( list, cout, 2 );
    cout << endl << "*** AnnotateX End ***" << endl;
  }
  return (list);
}




ListExpr
QueryProcessor::Annotate( const ListExpr expr,
                          NameIndex& varnames,
                          VarEntryTable& vartable,
                          bool& defined,
                          const ListExpr fatherargtypes )
{
/*
Annotates a query expression ~expr~. Use tables ~varnames~ and ~vartable~
to store variables occurring in abstractions (function definitions) and to
retrieve them in the function's expression. Return the annotated
expression.

Parameter ~defined~ is set to FALSE if any object mentioned in the
expression has an undefined value. Parameter ~fatherargtypes~ is used to
implement inference of parameter types in abstractions. When a function
is analyzed by ~AnnotateFunction~, then this list contains the argument
types of the operator to which this function is a parameter.

Annotation is done as follows. A query is a nested list structure. Any
subexpression (atom or sublist) ~s~ of ~expr~, is annotated by transforming it
(usually) into a structure (~ann~(~s~) ~type~(~s~)). Basically, there are four
different cases for ~s~:

        (i) ~s~ is empty

        (ii) ~s~ is an atom which represents a constant value

        (iii) ~s~ is a symbol atom which implies various subcases

        (iv) ~s~ is a nonempty list

In the following we give a detailed explanation of them. The annotation works
recursively and starts with ~expr~. Hence cases (i)-(iii) are cases for which the
recursion terminates whereas case (iv) results in recursive calls.

Case (i): ~s~ is an empty list. This is interpreted as an empty list of arguments.

----    ()      ->      ((none arglist ()) ())
----

Case (ii): ~s~ is an atom of type integer, real, boolean, string or text (a constant).

The ~In~ function associated with the type constructors ~int~,
~real~, ~string~, and ~bool~ (provided by some algebra) is called to
create a constant of the respective type.

----    7       ->      ((7 constant 1)        int)

        <value> ->      ((<value> constant <index>) <type>)
----

        Here ~index~ is an index into array ~values~ containing
constants which are interpreted as {~algId~, ~typeId~, ~value~}.  ~Annotate~
enters the value into that array.

Case (iii-a): ~s~ is a symbol atom: an operator

----    add     ->      ((add operator 1 4) typeerror)

        <op>    ->      ((<op> operator <algebraId> <operatorId>) typeerror)
----

        Here, once the operators can be overloaded between different algebras,
the construction of the operator list inside the ~Annotate~ function need to
have more than one entry to the tuple (algebraId, operatorId). In this way, its
representation internally in the function is done with a sublist of this tuples.

----    <op>    ->      ((<op> operator ((<alId1> <opId1>) ... (<alIdN>
                                <opIdN>)) ) typeerror)
----

        After the decision of which operator is suitable for the argument types,
using the ~TranformType~ function, it comes back to the representation with only
one pair (algebraId, operatorId) repeated below.

----    add     ->      ((add operator 1 4) typeerror)

        <op>    ->      ((<op> operator <algebraId> <operatorId>) typeerror)
----

Case (iii-b): ~s~ is a symbol atom: an object of the database, that ~is not itself
a function~, which means the type of the object does not start ``(map
...)''.

----    cities          ->      ((cities object 7)
                                (rel (tuple ((name string) (pop int)))) )

        <object name>   ->      ((<object name> object <index>) <type>)
----

        Here ~index~ again refers to the array ~values~.

Case (iii-c): ~s~ is a symbol atom: a function object of the database -- type has
the form ``(map ...)''. The corresponding function definition
(abstraction) is retrieved from the database and annotated recursively.

----    double          ->      Annotate((fun (x int) (add x x)))


        <function name> ->      Annotate(<abstraction>)

----

Case (iii-d): ~s~ is a symbol atom: neither operator nor object, but a variable
defined in some enclosing function definition (= abstraction), which
means it can be found in the table ~variableNames~.

----    x               ->      ((x variable 3 5) real)

        <var name>      ->      ((<var name> variable <position>
                                <functionno>) <type>)
----

        Here ~position~ is the relative position of the variable in the
list of arguments of the defining function, and ~functionno~ is a number
identifying that function (see below the strategy for maintaining
function numbers).



Case (iii-e): ~s~ is a symbol atom: none of the forms before, but equal to ``counter''.

----    counter         ->      ((counter counter) typeerror)
----


Case (iii-f): ~s~ is a symbol atom: none of the forms before, but equal to ``predinfo''.

----    predinfo         ->      ((predinfo predinfo) typeerror)
----

Case (iii-g): ~s~ is a symbol atom: none of the forms before, but equal to ``memory''.

----    memory         ->      ((memory memory) typeerror)
----


Case (iii-h): ~s~ is a symbol atom: neither operator nor object nor variable nor any other form before

----    pop             ->      ((pop identifier) pop)

        <ident>         ->      ((<ident> identifier) <ident>)
----

        This is some unidentified name which must be interpreted by a
type checking function, something like an attribute name. For this
reason, ~not the type, but the actual value~ of the identifier is
returned as a second component.



Case (iv-a): ~s~ is a nonempty list: first element is the symbol ~fun~.

Then the whole thing is a function definition (abstraction) of
the form

----        (fun (x1 t1) (x2 t2) ... (xn tn) expr)
----

        It is annotated by calling the procedure ~Annotate-function~
which enters the variable definitions into tables and then calls
~Annotate~ again to annotate the expression ~expr~. The result is

----    ->        ((none abstraction Annotate(expr) <functionno>) <type>)
----

        Note that here ~type~ is the corresponding functional type (map
...). ~Functionno~ is the index in ~ArgVectors~ used for the argument
vector of this function. In the annotation result the first element
of the first list is set to none. This is done to indicate that this
is not a type expression (see also next case).

Case (iv-b): ~s~ is a nonempty list: the first element is a type expression.

        Then the list is a pair describing a constant of the type given
as the first element. Here we have two possibilities depending on the
second element. The second element can be a *value* of the type given by
the first element or a *direct pointer* to a previously opened object
of this type. The query processor differentiates these
two possibilities by the reserved word ~ptr~ which means that it is a
pointer. For the first possibility, the type constructor's ~In~-function
is called to convert the value given as a second element into the
data structure ~Word~ which is entered into the array ~values~. For the second,
the pointer is directly copied as a ~Word~ into the array ~values~.

        Hence the annotation is as for constants:

----    (int 7)                 -> (((int 7) constant 1) int)

        (int (ptr 72638362))    -> (((int (pointer 72638362)) constant 1)
                                        int)

        <value>                 -> ((<value> constant <index>) <type>)
----

Case (iv-c): ~s~ is a nonempty list: first element is neither the symbol ~fun~,
nor is it a type expression.

Then ~Annotate~ is called recursively for each element of this
list; the results are collected into a list ~list~. Now we look at the
result. The first element of the result list can be:

  1 an annotated operator ((. operator . .) .)

  2 an annotated function object ((. function .) .)

  3 an annotated abstraction ((. abstraction .) .)

  4 a counter definition ((. counter) .)

  5 a predinfo definition ((. predinfo) .)

  6 a memory definition ((. memory) .)

  7 something else ((. [constant|object|variable|identifier] .) .), that is a constant, a database object, a variable, an identifier, or an empty list.

Case (1): In the first case we have an operator application (~op~ ~arg1~
... ~argn~). We first compute the ~resulttype~ by applying the
operator's type mapping function to the types of ~arg1~ ... ~argn~. We
then apply the operator's selection function to determine ~opFunId~, the
number of the operator's evaluation function. This can be different from
the operator number (~opId~) for overloaded operators.

The result is:

----        (   (none applyop (ann(op) ann(arg1) ... ann(argn)))
                <resulttype>
                <opFunId>)
----

Note: This is the only annotation consisting of *three*, instead of two,
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
               (ann(op) ann(arg1) ... ann(argn) ann(newarg1) ...
                ann(newargn)))
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

----    (APPEND (1) string)
----

The query processor, more precisely the procedure ~Annotate~, will
produce the annotation for the constant 1, append it to the list of
annotated arguments, and then use ``string'' as the result type of the
~attribute~ operation.

Case (2): This is an application of a database function object. The
argument types of the function object are checked against the types of
the actual arguments and the result type of the function is returned.
The result is

----    ((none applyfun (ann(function) ann(arg1) ... ann(argn)))
                <resulttype>)
----


Case (3): This is an application of an abstraction. Like the previous
case, argument types are checked and the result type of the abstraction
is returned.

----    ((none applyabs (ann(abstraction) ann(arg1) ... ann(argn)))
        <resulttype>)
----

Case (4): This is a definition of a counter associated with the subexpressions.
A counter is defined in a query in the form

----    (counter 5 <subexpr>)
----

Hence after annotation it is

----    (
          ((counter counter) typeerror)
          ((5 constant 1) int)
          ann(subexpr)
        )
----

The result is

----    (
          (none counterdef 5 ann(subexpr))
          <resulttype>
        )
----

The result type is taken from the subexpression.


Case (5): This is a definition of a predinfo pseudo operator associated with the subexpressions. A predinfo is defined in a query in the form

----    (predinfo 0.012 0.1442 <subexpr>)
----

Hence after annotation it is

----    (
          ((predinfo predinfo) typeerror)
          ((0.012 constant 1) real)
          ((0.1442 constant 2) real)
          ann(subexpr)
        )
----

The result is

----    (
          (none predinfodef 0.012 0.1442 ann(subexpr))
          <resulttype>
        )
----

The result type is taken from the subexpression.

Case (6): This is a definition of a memory pseudo operator associated with the subexpressions. A memory size is defined in a query in the form

----    (memory 512 <subexpr>)
----

Hence after annotation it is

----    (
          ((memory memory) typeerror)
          ((512 constant 1) int)
          ann(subexpr)
        )
----

The result is

----    (
          (none memorydef 512 ann(subexpr))
          <resulttype>
        )
----

The result type is taken from the subexpression.


Case (6): The whole list is then just a list of expressions (terms). The
result type is just the list of types of the expressions.

----    (t1 t2 ... tn)   ->  ((none arglist (ann(t1) ann(t2) ... ann(tn)))
                                        (type(t1) type(t2) ... type(tn)))
----

Note: If an operator has a list of parameter functions it is necessary to use
identifiers in front of a function definition otherwise ann(ti) will return a
list which matches the structure of case 3 and the annotation fails since this
case handles the application of a function but for a parameter function we are only
interested in its definition and the operator implementation will care for suitable
arguments and the execution of the function.

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
is given as a parameter to ~AnnotateFunction~.

There is one global variable ~functionno~ which is incremented whenever
a function definition (abstraction) is translated (by
~AnnotateFunction~) during the translation of one query. This number is
entered for a given variable into the variable table.

In the next step, when the annotated query is transformed into an
operator tree by procedure ~subtree~, then for each used function index
an argument vector is allocated and kept in a global array
~ArgVectors[i]~. A variable annotation is then translated into an
indirect object referring to the argument vector with the corresponding
function index.

*/

  int alId=0, opId=0, position=0, funindex=0, opFunId=0, errorPos=0;

  ListExpr first, rest, list, lastElem, typeExpr, typeList;
  first = rest = list = lastElem = typeExpr = typeList = nl->TheEmptyList();

  ListExpr resultType, last, pair, lastType, signature;
  resultType = last = pair = lastType = signature = nl->TheEmptyList();
  
  ListExpr typeArgList, lastTypeArg;
  typeArgList = lastTypeArg = nl->TheEmptyList();

  ListExpr firstSig, firstType, result, functionList;
  firstSig = firstType = result = functionList = nl->TheEmptyList();

  ListExpr& errorInfo = nl->GetErrorList();

  string name="", typeName="";

  bool definedValue=false,
       hasNamedType=false, correct=false, newOperator=false;

  Word value=SetWord(Address(0));

  if ( traceMode )
  {
    cout << "Annotate applied to: " << endl;
    nl->WriteListExpr( expr, cout, 2 );
    cout << endl << "argument types passed from father: " << endl;
    nl->WriteListExpr( fatherargtypes, cout, 2 );
    cout << endl;
    for ( int i=0; i<= valueno; i++ )
      cout << "values[" << i <<"]="
           << (void*)values[i].value.addr << endl;
  }

  if ( nl->IsEmpty( expr ) )
  {
    // case (i): result list ((none arglist ()) ()) for an empty list
    return (nl->TwoElemList(
              nl->ThreeElemList(
                nl->SymbolAtom( "none" ),
                nl->SymbolAtom( "arglist" ),
                nl->TheEmptyList() ),
              nl->TheEmptyList() ));
  }
  else if ( nl->IsAtom( expr ) ) // handle atoms
  {
    AllocateValues( valueno );
    switch (nl->AtomType( expr ))
    {
      // case (ii): return ((<value> constant <index>) int)
      case IntType:
      {
        int algId, typeId;
        GetCatalog()->GetTypeId(CcInt::BasicType(), algId, typeId );
        value = GetCatalog()->InObject( nl->SymbolAtom( CcInt::BasicType() ),
                                        expr, errorPos, errorInfo,
                                        correct );
        values[valueno].isConstant = true;
        values[valueno].isList = false;
        values[valueno].algId = algId;
        values[valueno].typeId = typeId;
        values[valueno].typeInfo = nl->SymbolAtom( CcInt::BasicType() );
        values[valueno].value = value;
        valueno++;
        return (nl->TwoElemList(
                  nl->ThreeElemList(
                    expr,
                    nl->SymbolAtom( "constant" ),
                    nl->IntAtom( valueno-1 ) ),
                  nl->SymbolAtom(CcInt::BasicType()) ));
      }
      // case (ii): return ((<value> constant <index>) real)
      case RealType:
      {
        int algId, typeId;
        GetCatalog()->GetTypeId( CcReal::BasicType(),algId, typeId );
        value = GetCatalog()->InObject( nl->SymbolAtom( CcReal::BasicType() ),
                                        expr, errorPos, errorInfo,
                                        correct );
        values[valueno].isConstant = true;
        values[valueno].isList = false;
        values[valueno].algId = algId;
        values[valueno].typeId = typeId;
        values[valueno].typeInfo = nl->SymbolAtom(  CcReal::BasicType() );
        values[valueno].value = value;
        valueno++;
        return (nl->TwoElemList(
                  nl->ThreeElemList(
                    expr,
                    nl->SymbolAtom( "constant" ),
                    nl->IntAtom( valueno-1 ) ),
                  nl->SymbolAtom( CcReal::BasicType() ) ));
      }
      // case (ii): return ((<value> constant <index>) bool)
      case BoolType:
      {
        int algId, typeId;
        GetCatalog()->GetTypeId( CcBool::BasicType(),algId, typeId );
        value = GetCatalog()->InObject( nl->SymbolAtom( CcBool::BasicType() ),
                                        expr, errorPos, errorInfo,
                                        correct );
        values[valueno].isConstant = true;
        values[valueno].isList = false;
        values[valueno].algId = algId;
        values[valueno].typeId = typeId;
        values[valueno].typeInfo = nl->SymbolAtom( CcBool::BasicType() );
        values[valueno].value = value;
        valueno++;
        return (nl->TwoElemList(
                  nl->ThreeElemList(
                    expr,
                    nl->SymbolAtom( "constant" ),
                    nl->IntAtom( valueno-1 ) ),
                  nl->SymbolAtom( CcBool::BasicType() ) ));
      }
      // case (ii): return ((<value> constant <index>) string)
      case StringType:
      {
        int algId, typeId;
        GetCatalog()->GetTypeId( CcString::BasicType(),algId, typeId );
        value = GetCatalog()->InObject( nl->SymbolAtom( CcString::BasicType() ),
                                        expr, errorPos, errorInfo,
                                        correct );
        values[valueno].isConstant = true;
        values[valueno].isList = false;
        values[valueno].algId = algId;
        values[valueno].typeId = typeId;
        values[valueno].typeInfo = nl->SymbolAtom( CcString::BasicType() );
        values[valueno].value = value;
        valueno++;
        return (nl->TwoElemList(
                  nl->ThreeElemList(
                    expr,
                    nl->SymbolAtom( "constant" ),
                    nl->IntAtom( valueno-1 ) ),
                  nl->SymbolAtom( CcString::BasicType() ) ));
      }
      // case (ii): return ((<value> constant <index>) text)
      case TextType:
      {
        int algId, typeId;
        GetCatalog()->GetTypeId( FText::BasicType(), algId, typeId );
        value = GetCatalog()->InObject( nl->SymbolAtom( FText::BasicType() ),
                                        expr, errorPos, errorInfo,
                                        correct );
        values[valueno].isConstant = true;
        values[valueno].isList = false;
        values[valueno].algId = algId;
        values[valueno].typeId = typeId;
        values[valueno].typeInfo = nl->SymbolAtom( FText::BasicType() );
        values[valueno].value = value;
        valueno++;
        return (nl->TwoElemList(
                  nl->ThreeElemList(
                    expr,
                    nl->SymbolAtom( "constant" ),
                    nl->IntAtom( valueno-1 ) ),
                  nl->SymbolAtom( FText::BasicType() ) ));

      }
      // case (iii)
      case SymbolType:
      {
        name = nl->SymbolValue( expr );

        if ( GetCatalog()->IsObjectName( name ) )
        {
          int algId, typeId; 
          GetCatalog()->GetObjectExpr( name, typeName, typeExpr,
                                       values[valueno].value,
                                       definedValue, hasNamedType );
          GetCatalog()->LookUpTypeExpr( typeExpr, typeName,
                                        algId, typeId );
//           values[valueno].isConstant = false;
//           values[valueno].isList = false;
//           values[valueno].algId = algId;
//           values[valueno].typeInfo = typeExpr;
//           values[valueno].typeId = typeId;
          values[valueno].isConstant = false;
          values[valueno].isList = false;
          values[valueno].algId = algId;
          values[valueno].typeInfo = typeExpr;
          values[valueno].typeId = typeId;

          if ( !definedValue )
          {
            defined = false;
          }
          valueno++;
          if ( nl->ListLength( typeExpr ) > 0 )
          {
            if ( TypeOfSymbol( nl->First( typeExpr ) ) == QP_MAP )
            {
              // case (iii-c): function object,
              // return Annotate(<abstraction>)
              NameIndex newvarnames;
              VarEntryTable newvartable;
              functionList = values[valueno-1].value.list;

              if (traceMode) {
                cout << "Function object " << name << ": " << endl
                     << nl->ToString(functionList) << endl;
              }
              values[valueno-1].isList = true;
              return Annotate( functionList, newvarnames,
                               newvartable, defined,
                               nl->TheEmptyList() );
            }
            else
            {
              // case (iii-b): ordinary object,
              // return ((<obj. name> object <index>) <type>)
              return (nl->TwoElemList(
                        nl->ThreeElemList(
                          expr,
                          nl->SymbolAtom( "object" ),
                          nl->IntAtom( valueno-1 ) ),
                        typeExpr ));
            }
          }
          else if ( nl->ListLength( typeExpr ) == -1 )
                   // spm: we should use nl->SymbolType()
          {
            // case (iii-b): ordinary object,
            // atomic type expression like int, real etc.
            // return ((<obj. name> object <index>) <type>)
            return (nl->TwoElemList(
                      nl->ThreeElemList(
                        expr,
                        nl->SymbolAtom( "object" ),
                        nl->IntAtom( valueno-1 ) ),
                      typeExpr ));
          }
          else
          {
            // error
            cmsg.error()
              << "Cannot annotate database object. "
              << "Unexpected type expression " << nl->ToString(typeExpr)
              << endl;
            cmsg.send();
            return (nl->SymbolAtom( "exprerror" ));
          }

        }
        else if ( GetCatalog()->IsOperatorName( name ) )
        {
     // case (iii-a): operator, return
    // ((<op> operator (<algId-1> <opId-1> ... <algId-N> <opId-N>)) typeerror)
          ListExpr opList = GetCatalog()->GetOperatorIds( name );
          return (nl->TwoElemList(
                    nl->ThreeElemList(
                      expr,
                      nl->SymbolAtom( "operator" ),
                      opList ),
                    nl->SymbolAtom( "typeerror" ) ));
        }
        else if ( IsVariable( name, varnames ) )
        {
          // case (iii-d): variable,
          // return ((<name> variable <pos> <funNo.>) <type>)
          GetVariable( name, varnames, vartable, position,
                       funindex, typeExpr );
          return (nl->TwoElemList(
                    nl->FourElemList(
                      expr,
                      nl->SymbolAtom( "variable" ),
                      nl->IntAtom( position ),
                      nl->IntAtom( funindex ) ),
                    typeExpr ));
        }
        else if ( TypeOfSymbol( expr ) == QP_COUNTER )
        {
          // case (iii-e): a counter,
          // return ((counter counter) typeerror)
          return nl->TwoElemList(
                   nl->TwoElemList(
                     expr,
                     nl->SymbolAtom( "counter" ) ),
                   nl->SymbolAtom( "typeerror" ) );
        }
        else if ( TypeOfSymbol( expr ) == QP_PREDINFO )
        {
          // case (iii-f): a predinfo pseudo operator,
          // return ((predinfo predinfo) typeerror)
          return nl->TwoElemList(
                   nl->TwoElemList(
                     expr,
                     nl->SymbolAtom( "predinfo" ) ),
                   nl->SymbolAtom( "typeerror" ) );
        }
        else if ( TypeOfSymbol( expr ) == QP_MEMORY )
        {
          // case (iii-g): a memory pseudo operator,
          // return ((memory memory) typeerror)
          return nl->TwoElemList(
                   nl->TwoElemList(
                     expr,
                     nl->SymbolAtom( "memory" ) ),
                   nl->SymbolAtom( "typeerror" ) );
        }
        else
        {
          // case (iii-h): nothing of the above => identifier
          // return ((<ident> identifier) <ident>)
          string typeName;
          int algId, typeId;
          if(GetCatalog()->LookUpTypeExpr(expr, typeName, algId,typeId)){
             string errMsg  = "Type name '" + typeName + 
                              "' used as an indentifier.";
             throw runtime_error(errMsg);
             return (nl->TheEmptyList());
           }          
           return (nl->TwoElemList(
                    nl->TwoElemList(
                      expr,
                      nl->SymbolAtom( "identifier" ) ),
                    expr ));
        }
      }
      default:
      {
        cmsg.warning()
          << "Default reached while annotation an atom!"
          << endl;
        cmsg.send();
        return (nl->TheEmptyList());
      }
    } // end of switch (nl->AtomType( expr ))
  }
  else
  {
    // case (iv): expr is a nonempty list

    if ( TypeOfSymbol( nl->First( expr ) ) == QP_FUN )
    {
      // case (iv-a): annotate the function definition (abstraction)
      return AnnotateFunction( expr, varnames, vartable, defined,
                               0, nl->TheEmptyList(),
                               nl->TheEmptyList(), fatherargtypes );
    }
    else if ( IsCorrectTypeExpr( nl->First( expr ) ) )
    {
      // case (iv-b): The first element is a valid type expression!
      bool isPointer = false;

      if( nl->ListLength(nl->Second(expr)) == 2 &&
          nl->IsEqual(nl->First(nl->Second(expr)), "ptr") &&
          listutils::isPtrList(nl->Second(nl->Second(expr)))) {
        value = SetWord(listutils::getPtr(nl->Second(nl->Second(expr))));
        isPointer = true;
        correct = true;
      } else if( nl->ListLength(nl->Second(expr)) == 2 &&
          nl->IsEqual(nl->First(nl->Second(expr)), "ptr") &&
          nl->IsAtom(nl->Second(nl->Second(expr))) &&
          nl->AtomType(nl->Second(nl->Second(expr))) == IntType ) {

        cerr << "WARNING: Use of deprecated (non 64 bit save) version of ptr" 
             << endl;
        cerr << "Use listutils::getPtrList(void*) instead of an int-Atom " 
             << endl;
        cerr << "Please contact the programmer of the used opertor to fix it"
             << endl;
        // constant value given as pointer
        value = SetWord(
          (void*)nl->IntValue(nl->Second(nl->Second(expr))));
        isPointer = true;
        correct = true;
      } else {
        // constant value given as pair (<type> <value>)
        value = GetCatalog()->InObject(nl->First( expr ),
                                       nl->Second( expr ),
                                       errorPos, errorInfo, correct);
      }

      if ( correct )
      {
        AllocateValues( valueno );
        int algId, typeId;
        GetCatalog()->LookUpTypeExpr(nl->First(expr),
                                     typeName, algId, typeId );
        values[valueno].isConstant = false;
        values[valueno].isList = false;
        values[valueno].algId = algId;
        values[valueno].typeId = typeId;
        values[valueno].value = value;
        valueno++;

        if( isPointer )
        {
          return (nl->TwoElemList(
                    nl->ThreeElemList(
                      expr,
                      nl->SymbolAtom( "pointer" ),
                      nl->IntAtom( valueno-1 ) ),
                    nl->First( expr ) ));
        }
        else
        {
          return (nl->TwoElemList(
                    nl->ThreeElemList(
                      expr,
                      nl->SymbolAtom( "constant" ),
                      nl->IntAtom( valueno-1 ) ),
                    nl->First( expr ) ));
        }
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
    else
    {
      // case (iv-c): neither abstraction nor constant.
      // Now recursively annotate all elements of this list.
      //
      if (traceMode) {
        cout << "Case (iv-c): Nonempty list! Neither symbol fun "
             << "nor a type expr. List will be annotated recursively."
             << endl;
      }
      first = nl->First( expr );
      rest = nl->Rest( expr );

      pair = Annotate( first, varnames, vartable,
                       defined, fatherargtypes );

      /* Check whether the first element is a new operator.
         In that case the current partial list of argument types
         to that operator has to be passed down in 'fatherargtypes'
         in calls of 'annotate'. */

      newOperator =
        ((nl->ListLength( pair ) == 2) &&
        (nl->ListLength(nl->First(pair)) == 3) &&
        (TypeOfSymbol(nl->Second(nl->First(pair))) == QP_OPERATOR));
    
              //list of annotations
      list = nl->OneElemList( pair );  
      lastElem = list;
              //list of types
      typeList = nl->OneElemList( nl->Second( pair ) );
      lastType = typeList;
        //list of pairs (type, argument). Operators
              //that have registered
              //UsesArgsInTypeMapping will get this list
              //instead of the typelist
      typeArgList = nl->OneElemList( 
    nl->TwoElemList( nl-> Second(pair), first) );
    lastTypeArg = typeArgList;

      while (!nl->IsEmpty( rest ))
      {
        if ( newOperator )
        { /* current list of arg types to be used */
          pair = Annotate( nl->First( rest ), varnames, vartable,
                           defined, typeList );
        }
        else
        { /* just pass down the inherited list of args */
          pair = Annotate( nl->First( rest ), varnames, vartable,
                           defined, fatherargtypes );
        }
        if( !nl->IsEmpty( pair ) &&
            nl->IsAtom( pair ) &&
            nl->AtomType( pair ) == SymbolType &&
            nl->SymbolValue( pair ) == "exprerror" ){
          return pair;
        }

        lastElem = nl->Append( lastElem, pair );
        lastType = nl->Append( lastType, nl->Second( pair ) );
    lastTypeArg = nl->Append( lastTypeArg, 
      nl->TwoElemList( nl->Second( pair) , nl->First( rest )) );
        rest = nl->Rest( rest );
      }
      last = lastElem;   /* remember the last element to be able to
                            append further arguments, see below */
/*
At this point, we may have a nested list ~list~ such as

----
        (((+ operator ((1 6) (7 0))) ()) ((3 ...) int) ((10 ...) int))
----

for a given ~expr~ (+ 3 10). Now the first element of ~list~ which
is

----
        first = ((+ operator ((1 6) (7 0))) ())
----

will be processed.

*/

      if (traceMode)
      {
        cout << "*** Some list exprs. after recursive calls ***" << endl;
        cout << "Value of variable list: " << endl;
        nl->WriteListExpr( list, cout, 2 );
        cout << "Value of variable typeList: " << endl;
        nl->WriteListExpr( typeList, cout, 2 );
        cout << endl;
      }

      first = nl->First( list );
      if ( nl->ListLength( first ) > 0 )
      {
        first = nl->First( first );  // = (+ operator ((1 6) (7 0)))

        if ( nl->ListLength( first ) >= 2 )
        {
          switch (TypeOfSymbol( nl->Second( first ) ))
          {
            case QP_OPERATOR:
            {
              if (traceMode)
                cout << "Case 1: An annotated Operator." << endl;
              string operatorStr = nl->SymbolValue(nl->First(first));
              ListExpr opList = nl->Third( first );

              if ( !nl->HasMinLength(opList,1)  ) {
    stringstream err;
    err << "Expecting a list but got " << nl->ToString(opList);
          throw qp_error( err.str() );
              }

     //for all three lists, remove the first argument 
                 //representing the operator
              rest = nl->Rest( list );
              typeList = nl->Rest( typeList );
              typeArgList = nl->Rest( typeArgList );
        

              resultType =
                TestOverloadedOperators( operatorStr, opList,
                                         typeList, typeArgList, alId, opId,
                                         opFunId, true, traceMode );

              /* check whether the type mapping has requested to
                 append further arguments: */
              if ( (nl->ListLength(resultType) == 3) &&
                   (TypeOfSymbol(nl->First(resultType))==QP_APPEND) )
              {
                lastElem = last;
                rest = nl->Second( resultType );

                while (!nl->IsEmpty( rest ))
                {
                  lastElem =
                    nl->Append( lastElem,
                                Annotate(nl->First(rest), varnames,
                                         vartable, defined,
                                         fatherargtypes) );
                  rest = nl->Rest( rest );
                }
                resultType = nl->Third( resultType );
              }

              ListExpr newList = nl->Cons(
                                   nl->TwoElemList(
                                     nl->FourElemList(
                                       nl->First(first),
                                       nl->Second(first),
                                       nl->IntAtom(alId),
                                       nl->IntAtom(opFunId)),
                                     nl->Second(nl->First(list))),
                                   nl->Rest(list));

              ListExpr applyopList = nl->ThreeElemList(
                                       nl->ThreeElemList(
                                         nl->SymbolAtom("none"),
                                         nl->SymbolAtom("applyop"),
                                         newList),
                                       resultType,
                                       nl->IntAtom(opFunId));
              return (applyopList);
            }
            case QP_FUNCTION:
            {
              // spm: I could find no code which inserts the keyword "function"
              // during annotation. Hence I suppose this is obsolete and may be
              // removed since AnnotateFunction will always return
              // ((none abstraction) ...).
              // For a while I try if this branch is reached. If not the case
              // QP_FUNCTION may be removed from the QueryProcessor

              if (traceMode)
                cout << "Case 2: An annotated function." << endl;
        throw qp_error( "unexpected function annotation!" );


              signature = nl->Rest(nl->Second(nl->First(list)));
              typeList = nl->Rest( typeList );
              if ( traceMode )
              {
                cout << "function signature: ";
                nl->WriteListExpr( signature, cout, 2 );
                cout << endl;
                cout << "function typeList: ";
                nl->WriteListExpr( typeList, cout, 2 );
                cout << endl;
              }
              if ( nl->ListLength(signature) ==
                   (nl->ListLength( typeList )+1) )
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
                  nl->WriteListExpr( resultType, cout, 2 );
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
                  nl->WriteListExpr( result, cout, 2 );
                  cout << endl;
                }
                return (result);
              }
              else
              {
                if ( traceMode )
                {
                  cout << "list: " << endl;
                  nl->WriteListExpr( list, cout, 2 );
                  cout << "expr: " << endl;
                  nl->WriteListExpr( expr, cout, 2 );
                }
                return (nl->SymbolAtom( "exprerror" ));
              }
            }
            case QP_ABSTRACTION:
            {
              if (traceMode)
                cout << "Case 3: An annotated abstraction." << endl;
              signature = nl->Rest(nl->Second(nl->First(list)));
              typeList = nl->Rest( typeList );
              if ( traceMode )
              {
                cout << "Abstraction signature: ";
                nl->WriteListExpr( signature, cout, 2 );
                cout << endl;
                cout << "Abstraction typeList: ";
                nl->WriteListExpr( typeList, cout, 2 );
                cout << endl;
              }
              int expectedParams = nl->ListLength( signature ) - 1;
              int retrievedParams = nl->ListLength( typeList );
              if (  expectedParams ==  retrievedParams )
              {
                int paramNum = 0;
                while (!nl->IsEmpty( typeList ))
                {
                  paramNum++;
                  firstSig = nl->First( signature );
                  signature = nl->Rest( signature );
                  firstType = nl->First( typeList );
                  typeList = nl->Rest( typeList );
                  if ( !nl->Equal( firstSig, firstType ) )
                  {
                    cmsg.error()
                       << "Type mismatch for parameter number "
                       << paramNum << endl
                       << "Expected:" << nl->ToString(firstSig) << endl
                       << "Obtained:" << nl->ToString(firstType) << endl;
                    cmsg.send();
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
                if ( traceMode )
                {
                  cout << "list: " << endl;
                  nl->WriteListExpr( list, cout, 2 );
                  cout << "expr: " << endl;
                  nl->WriteListExpr( expr, cout, 2 );
                }
                cmsg.error()
                   << "Type mismatch! Expecting " << expectedParams
                   << " parameter but got " << retrievedParams <<  "." << endl;
                cmsg.send();
                return (nl->SymbolAtom( "exprerror" ));
              }
            }

            case QP_COUNTER:
            {
              if (traceMode)
                cout << "Case 4: A counter definition." << endl;
              int counterNo =
                nl->IntValue(nl->First(nl->First(nl->Second(list))));

              if ( counterNo > 0 && counterNo <= NO_COUNTERS )
              {
                return nl->TwoElemList(
                         nl->FourElemList(
                           nl->SymbolAtom("none"),
                           nl->SymbolAtom("counterdef"),
                           nl->First(nl->First(nl->Second(list))),
                           nl->Third(list)),
                         nl->Second(nl->Third(list)));
              }
              else
              {
                cout << "counter number " << counterNo <<
                        " is out of the range of counters." << endl;
                return nl->TwoElemList(
                         nl->FourElemList(
                           nl->SymbolAtom("none"),
                           nl->SymbolAtom("counterdef"),
                           nl->First(nl->First(nl->Second(list))),
                           nl->Third(list)),
                         nl->SymbolAtom("typeerror"));
              }
            }

            case QP_PREDINFO:
            {
              if (traceMode)
                cout << "Case 5: A predinfo definition." << endl;

              return nl->TwoElemList(
                         nl->FiveElemList(
                           nl->SymbolAtom("none"),
                           nl->SymbolAtom("predinfodef"),
                           nl->First(nl->First(nl->Second(list))),
                           nl->First(nl->First(nl->Third(list))),
                           nl->Fourth(list)),
                         nl->Second(nl->Fourth(list)));
            }

            case QP_MEMORY:
            {
              if (traceMode)
                cout << "Case 6: A memory definition." << endl;

                return nl->TwoElemList(
                         nl->FourElemList(
                           nl->SymbolAtom("none"),
                           nl->SymbolAtom("memorydef"),
                           nl->First(nl->First(nl->Second(list))),
                           nl->Third(list)),
                         nl->Second(nl->Third(list)));
            }

            default:
            {  /* we have a list of terms, case (7) above)
                  Again extract the list of types. We know the
                  list "list" is not empty */
              if (traceMode) {
               cout << "Case 7: A constant, a database obj., "
                    << " an identifier or an empty list! " << endl;
              }
              rest = list;
              return (nl->TwoElemList(
                        nl->ThreeElemList(
                          nl->SymbolAtom( "none" ),
                          nl->SymbolAtom( "arglist" ),
                        list ),
                      typeList ));
            }
          } // switch (TypeOfSymbol( nl->Second( first ) ))
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
  }

  // Any other case which should never be reached
  return (nl->SymbolAtom( "exprerror" ));
} // end Annotate

ListExpr
QueryProcessor::TestOverloadedOperators( const string&
                                         operatorSymbolStr,
                                         ListExpr opList,
                                         ListExpr typeList, 
                                         ListExpr typeArgList,
                                         int& alId,
                                         int& opId,
                                         int& opFunId,
                                         bool checkFunId,
                                         bool traceMode )
{
  ListExpr resultType = nl->TheEmptyList();

  static const int width=70;
  static const string sepLine = "\n" + string(width,'-') + "\n";

  if ( traceMode )
  {
    cout << sepLine
         << "Type mapping for operator " << operatorSymbolStr << ":" << endl;
  }

  string typeErrorMsg =
   "Type map error for operator " + operatorSymbolStr + "!" + sepLine
   + wordWrap("Input: ", width, NList(typeList).convertToString())
   + sepLine + "Error Message(s):" + sepLine;

  do // Overloading: test operator candidates
  {
    alId = nl->IntValue( nl->First( nl->First( opList ) ) );
    opId = nl->IntValue( nl->Second( nl->First( opList ) ) );

    /* apply the operator's type mapping: */
  // (i) standard case: pass the list of types
  // (ii) special case: pass the list of pairs (type, argument)
  // This is for operators who need to see argument expressions 
        // within the type
  // mapping, for example, a filename passed as a string, to get some type
  // information from the file.
  // This case applies if the operator has registered  
        // "UsesArgumentsInTypeMapping"
        // It is demonstrated with the filter operator in the relation
        // algebra.
  
  if ( !algebraManager->
             getOperator( alId, opId )->UsesArgsInTypeMapping() )
    resultType = algebraManager->TransformType( alId, opId, typeList );
  else
    resultType = algebraManager->TransformType( alId, opId, typeArgList );
    
    string algName = algebraManager->GetAlgebraName(alId);

    if( traceMode )
    {
      stringstream traceMsg;
      traceMsg << algName << ": " << operatorSymbolStr << " (algId="
               << alId << ", opId=" << opId << ") "<< ends;

      if( nl->IsEqual( resultType, "typeerror" ) )
        cout  << traceMsg.str() << "rejected!" << endl;
      else
        cout << traceMsg.str() << "accepted!" << endl;
    }

    if ( !ErrorReporter::TypeMapError )
    {
      string msg = "";
      ErrorReporter::GetErrorMessage(msg);
      // remove errors produced by
      // testing operators
      if ( msg == "" )
        msg = "<No error message specified>";
      typeErrorMsg += wordWrap(algName + ": ",4 ,width, msg) + "\n";
    }

    opList = nl->Rest( opList );
  }
  while ( !nl->IsEmpty( opList ) &&
          nl->IsEqual( resultType, "typeerror" ) );

  /*  check if the final result of testing is still a typeerror.
   *  If so save the messages in the error reporter. Errors detected
   *  afterwards will not be reported any more.
  */
  int selFunIndex=-1;
  if ( nl->IsEqual( resultType, "typeerror" ) )
  {
    if(!ErrorReporter::TypeMapError){
      ErrorReporter::ReportError(typeErrorMsg + sepLine);
    }
    ErrorReporter::TypeMapError = true;
  }
  else
  {
    /*   use the operator's selection function to get the index
     *  (opFunId) of the evaluation function for this operator:
     */
    if ( checkFunId )
    {
      opFunId = algebraManager->Select( alId, opId, typeList );
      selFunIndex = opFunId;
      opFunId = opFunId * 65536 + opId;

      /*  Check whether this is a type operator; in that case
       *  opFunId will be negative. A type operator does only a type
       *  mapping, nothing else; hence it is wrong here and we return
       *  type error.
      */
      if ( opFunId < 0 )
        resultType = nl->SymbolAtom( "typeerror" );
    }
  }

  if ( traceMode )
  {
    cout << wordWrap( "IN: ", width, NList(typeList).convertToString() )
         << endl
         << wordWrap( "OUT: ", width, NList(resultType).convertToString() )
         << endl
         << "SelectionFunction: index = " << selFunIndex << endl
         << sepLine << endl;
  }

  return resultType;
}



ListExpr
QueryProcessor::AnnotateFunction( const ListExpr expr,
                                  NameIndex& varnames,     // in/out
                                  VarEntryTable& vartable, // in/out
                                  bool& defined,           // in/out
                                  const int paramno,
                                  const ListExpr typeList,
                                  const ListExpr lastElem,
                                  const ListExpr fatherargtypes )
{
/*
Annotate an abstraction ~expr~ which has the form:

----    (fun (x1 t1) ... (xn tn) e)
----

and return the annotated version:

----    -> ((none abstraction annotate(expr) <functionno>) <type>)
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
  static const string fn("AnnotateFunction");
  string name = "", name2 = "", xxx = "";
  ListExpr annexpr = nl->TheEmptyList();
  ListExpr list = nl->TheEmptyList();
  ListExpr paramtype = nl->TheEmptyList();

  int localfunctionno = functionno;

  if ( traceMode )
  {
    cout << fn << " applied to: " << endl;
    nl->WriteListExpr( expr, cout, 2 );
    cout << endl;
  }
  if ( nl->IsEmpty( expr ))
  {
    cmsg.error() << fn << ": No expression in function definition." << endl;
    return (nl->TwoElemList(
              nl->SymbolAtom( "functionerror" ),
              nl->SymbolAtom( "typeerror" ) ));
  }
  else if ( nl->ListLength(expr) == 1 ) /* e reached */
  {
    annexpr = Annotate( nl->First( expr ), varnames, vartable,
                        defined, fatherargtypes );
    /* "e" reached */
  }
  else if ( nl->IsAtom( nl->First( expr ) ) )
  {
    if ( TypeOfSymbol( nl->First( expr ) ) == QP_FUN )
    {
      functionno++;
      list = nl->OneElemList( nl->SymbolAtom( "map" ) );
      return AnnotateFunction( nl->Rest( expr ), varnames, vartable,
                               defined, 1, list, list,
                               fatherargtypes );
    }
    else
    {
      annexpr = Annotate( nl->First( expr ), varnames, vartable,
                          defined, fatherargtypes );
    }
  }
  else if ( nl->ListLength( nl->First( expr )) == 2 )
  {
    if ( IsIdentifier( nl->First( nl->First( expr ) ), varnames ) )
    { /* a parameter def. Check for typename or type operator
         instead of type expression */
      if( nl->IsAtom(nl->Second(nl->First(expr))) &&
          nl->AtomType(nl->Second(nl->First(expr))) == SymbolType )
      {
        name2 = nl->SymbolValue( nl->Second( nl->First( expr ) ) );
        if ( GetCatalog()->MemberType( name2 ) )
        { /* name2 is a type name */
          paramtype = GetCatalog()->GetTypeExpr( name2 );
        }
        else if ( GetCatalog()->IsOperatorName( name2 ) )
        { /* name2 is a type operator */
          ListExpr opList = GetCatalog()->GetOperatorIds( name2 );
          ListExpr typeList = nl->Rest( fatherargtypes );

          int alId = 0;
          int opId = 0;
          int opFunId = 0;
          paramtype =
            TestOverloadedOperators( name2, opList, typeList,
        nl->TheEmptyList(), //type operators not eligible
                                  //to see argument exprs
              alId, opId, opFunId,
              false, traceMode );
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
      NList param(paramtype);
      bool typeOk = false;
      if (param.isList() && param.first().isSymbol("stream"))
      {
        typeOk = IsCorrectTypeExpr( param.second().listExpr() );
      }
      else
      {
        typeOk = IsCorrectTypeExpr( paramtype );
      }

      if ( typeOk )
      {
        name = nl->SymbolValue( nl->First( nl->First( expr ) ) );
        /* IsIdentifier has checked that name is not a variable yet,
           hence ! IsVariable(name, varnames) is ensured */

        EnterVariable( name, varnames, vartable, paramno,
                       localfunctionno, paramtype );
        list = nl->Append( lastElem, paramtype );
        return AnnotateFunction( nl->Rest( expr ), varnames,
                                 vartable, defined, paramno+1,
                                 typeList, list, fatherargtypes );
      }
      else
      {
        cmsg.error() << fn << ": Wrong parameter type "
                     << NList(paramtype) << endl;
        return (nl->TwoElemList(
                  nl->SymbolAtom( "functionerror" ),
                  nl->SymbolAtom( "typeerror" ) ));
      }
    }
    else
    {
      cmsg.error() << fn << "("<< __FILE__ << ":" << __LINE__ <<"):"
          "Branch should never be reached." << endl;
      return (nl->TwoElemList(
                nl->SymbolAtom( "functionerror" ),
                nl->SymbolAtom( "typeerror" ) ));
    }
  }
  else
  {
    cmsg.error() << fn << "("<< __FILE__ << ":" << __LINE__ <<"):"
        "Branch should never be reached." << endl;
    return (nl->TwoElemList(
              nl->SymbolAtom( "functionerror" ),
              nl->SymbolAtom( "typeerror" ) ));
  }
  if(nl->AtomType(annexpr)!=NoAtom){
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
QueryProcessor::IsCorrectTypeExpr( const ListExpr expr )
{
  ListExpr& errorInfo = nl->GetErrorList();
  return (GetCatalog()->KindCorrect( expr, errorInfo ));
}

/*****************************************************************************
3.6 Deleting all objects and constants in the values array

This function is called if there are errors in the Annotate process. It is not
necessary to build the query tree and all constants and objects in the ~values~
array must be deleted and closed respectively.

*/
void
QueryProcessor::DestroyValuesArray()
{
  for( int i = 0; i < valueno; i++ )
  {
    if( !values[i].isList )
    {
      if(values[i].value.addr){
         if( values[i].isConstant ){
           (algebraManager->DeleteObj
             ( values[i].algId, values[i].typeId ))( values[i].typeInfo,
                                                     values[i].value );
        } else  {
           (algebraManager->CloseObj
             ( values[i].algId, values[i].typeId ))( values[i].typeInfo,
                                                     values[i].value );
        }
        values[i].value.setAddr(0);
      }
    }
  }
}

/*****************************************************************************
3.6 Building an Operator Tree from an Annotated Query: Procedures
~subtree~, ~subtreeX~

*/
OpTree
QueryProcessor::SubtreeX( const ListExpr expr )
{
  AllocateArgVectors( functionno );
  for ( int i = 0; i < functionno; i++)
  {
    if(argVectors[i]){
      delete argVectors[i];
    }
    argVectors[i] = (ArgVectorPointer) new ArgVector;
    for ( int j = 0; j < MAXARG; j++ )
    {
      (*argVectors[i])[j].addr = 0;
    }
  }


  bool first = true;
  OpTree resultTree = Subtree( expr, first );

  if ( RTFlag::isActive("QP:OpTree2SVG") ) {

    const char* cfp = "optree_cur.gv";
    const char* ofp = "optree_old.gv";

    DotSpec dot("optree");

    dot.addNodeType("op",     "[ shape=\"circle\" ]");
    dot.addNodeType("obj",    "[ shape=\"box\" ]");
    dot.addNodeType("ptr",    "[ shape=\"triangle\" ]");
    dot.addNodeType("indObj", "[ shape=\"polygon\" ]");
    dot.addNodeType("const",  "[ shape=\"diamond\" ]");

    // rename current to old;
    rename(cfp, ofp);
    ofstream of;
    of.open(cfp, ios::trunc);
    BuildDotDescr( resultTree, dot );
    dot.buildGraph(of);
    of.close();
  }

  if ( debugMode )
  {
    cout << endl << "*** SubtreeX Begin ***" << endl;
    ListExpr treeList = ListOfTree( resultTree, cerr );
    nl->WriteListExpr( treeList, cout, 2 );
    nl->Destroy( treeList );
    cout << endl << "*** SubtreeX End ***" << endl;
  }
  return (resultTree);
}

OpTree
QueryProcessor::Subtree( const ListExpr expr,
                         bool&  first,
                         const OpNode* fatherNode /* = 0 */
)
{
  OpTree node = 0;
  ListExpr list = nl->TheEmptyList();
  string typeName = "";
  string xxx = "", yyy = "", zzz = "";

  bool oldfirst = first;
  first = false;

  ListExpr typeOfAnnotation = nl->TheEmptyList();
  ListExpr symbolForOperatorOrObject = nl->TheEmptyList();

  // check list structure
  bool cls = (nl->ListLength( expr ) >= 2);
  cls = cls && (nl->ListLength( nl->First( expr )) >= 2);

  if (cls)
  {
    typeOfAnnotation = nl->Second(nl->First( expr ));
    cls = cls && nl->AtomType(typeOfAnnotation) == SymbolType;
    symbolForOperatorOrObject = nl->First(nl->First( expr ));


  }

  if (!cls)
  {
    cerr << "subtree: error in annotated expression \""
         << nl->ToString(expr) << "\"" << endl;
    cerr << "subtree: list structure incorrect!" << endl;
    exit(1);
  }

  if ( traceMode )
  {
    cout << "subtree applied to: " << endl;
    nl->WriteListExpr( expr, cout, 2 );
    cout << endl << "TypeOfSymbol applied to <";
    nl->WriteListExpr( nl->Second( nl->First( expr ) ), cout, 2 );
    cout << ">" << endl;
  }

  switch (TypeOfSymbol( nl->Second( nl->First( expr ) ) ))
  {
    /* possibilities are:
         constant    object     operator
          variable   applyop    abstraction
         identifier  arglist    function
         applyabs    applyfun   counterdef
         pointer
    */

    case QP_POINTER:
    {
      node = new OpNode(Pointer);
      node->evaluable = true;
      node->typeExpr = nl->Second( expr );
      node->isRoot = oldfirst;
      node->u.dobj.isConstant = true;
      node->u.dobj.valNo = nl->IntValue(nl->Third(nl->First(expr)));
      node->u.dobj.value = values[node->u.dobj.valNo].value;
      if (traceNodes)
      {
        cout << "QP_POINTER:" << endl;
        cout << *node << endl;
      }
      return (node);
    }
    case QP_CONSTANT:
    {
      node = new OpNode(Object);
      node->evaluable = true;
      node->typeExpr = nl->Second( expr );
      node->isRoot = oldfirst;
      node->u.dobj.isConstant = true;
      node->u.dobj.valNo = nl->IntValue(nl->Third(nl->First(expr)));
      node->u.dobj.value = values[node->u.dobj.valNo].value;
      if (traceNodes)
      {
        cout << "QP_CONSTANT:" << endl;
        cout << *node << endl;
      }
      return (node);
    }
    case QP_OBJECT:
    {
      node = new OpNode(Object);
      node->evaluable = true;
      node->typeExpr = nl->Second( expr );
      node->isRoot = oldfirst;
      node->u.symbol = symbolForOperatorOrObject;
      node->u.dobj.valNo = nl->IntValue(nl->Third(nl->First(expr)));
      node->u.dobj.value = values[node->u.dobj.valNo].value;
      if (traceNodes)
      {
        cout << "QP_OBJECT:" << endl;
        cout << *node << endl;
      }
      return (node);
    }
    case QP_OPERATOR:
    {
      node = new OpNode(Operator);
      node->evaluable = true;
      node->typeExpr = nl->Second( expr );
      node->isRoot = oldfirst;
      node->u.symbol = symbolForOperatorOrObject;

      // Store Pointer to valuemapping function
      int algebraId = nl->IntValue(nl->Third(nl->First(expr)));
      int opFunId = nl->IntValue(nl->Fourth(nl->First(expr)));

      int opId  = opFunId % 65536;
      int funId = opFunId / 65536;

      node->u.op.algebraId = algebraId;
      node->u.op.opFunId =  opFunId;
      node->u.op.theOperator = algebraManager->getOperator(algebraId, opId);
      node->u.op.valueMap = node->u.op.theOperator->GetValueMapping(funId);

      /* next fields may be overwritten later */
      node->u.op.noSons = 0;
      node->u.op.isFun = false;
      node->u.op.funNo = 0;
      node->u.op.isStream = false;
      node->u.op.resultAlgId = 0;
      node->u.op.counterNo = 0;
      node->u.op.supportsProgress = !RTFlag::isActive("QP:ProgDisable") &&
              algebraManager->getOperator(algebraId, opId)->SupportsProgress();
      node->u.op.usesMemory = 
              algebraManager->getOperator(algebraId, opId)->UsesMemory();
      node->u.op.memorySize = 0;
      assert(!node->u.op.costEstimation);
      CreateCostEstimation createCE = 
                      node->u.op.theOperator->getCreateCostEstimation(funId);
      if(createCE){
         node->u.op.costEstimation = createCE();
         node->u.op.costEstimation->setSupplier(node);
         node->u.op.supportsProgress = true;
         node->u.op.argsAvailable = false;
      } 

      if (traceNodes)
      {
        cout << "QP_OPERATOR:" << endl;
        cout << *node << endl;
      }
      return (node);
    }
    case QP_VARIABLE:
    {
      node = new OpNode(IndirectObject);
      node->evaluable = true;
      node->typeExpr = nl->Second( expr );
      node->isRoot = oldfirst;
      node->u.iobj.funNumber =
        nl->IntValue(nl->Fourth(nl->First(expr )));
      node->u.iobj.vector = argVectors[node->u.iobj.funNumber-1];
      node->u.iobj.argIndex =
        nl->IntValue(nl->Third(nl->First(expr)));
      if( !nl->IsAtom(nl->Second(expr)) &&
          TypeOfSymbol(nl->First(nl->Second(expr))) == QP_STREAM )
      {
        node->u.op.isStream = true;
        node->evaluable = false;
      }
      if( !nl->IsAtom(nl->Second(expr)) &&
          TypeOfSymbol(nl->First(nl->Second(expr))) == QP_MAP )
      {
        cerr << "The system does not support functions as arguments "
                "of functions." << endl;
        exit(0);
      }

      if (traceNodes)
      {
        cout << "QP_VARIABLE:" << endl;
        cout << *node << endl;
      }
      return (node);
    }
    case QP_APPLYOP:
    {
      first = false;
      node = Subtree(nl->First(nl->Third(nl->First(expr))),
                      first, node );
      node->evaluable = true;
      node->typeExpr = nl->Second( expr );
      node->isRoot = oldfirst;
      // set the number of the function which was determined
      // by testing overloaded operators.
      //
      int opFunId = nl->IntValue( nl->Third( expr ));

      int opId  = opFunId % 65536;
      int funId = opFunId / 65536;

      node->u.op.opFunId =  opFunId;
      node->u.op.theOperator = algebraManager
                                 ->getOperator(node->u.op.algebraId, opId);
      node->u.op.valueMap = node->u.op.theOperator->GetValueMapping(funId);
      if(!node->u.op.costEstimation){
              CreateCostEstimation createCE = 
                      node->u.op.theOperator->getCreateCostEstimation(funId);
           if(createCE){
             node->u.op.costEstimation = createCE();
             node->u.op.costEstimation->setSupplier(node);
             node->u.op.supportsProgress = true;
             node->u.op.argsAvailable = false;
           } 
      }

      //check whether this operator does not use automatic evaluation
      //of arguments, but uses explicit requests instead
      //
      bool requestsArguments =
        algebraManager->getOperator(node->u.op.algebraId, opId)
              ->RequestsArguments();
      node->u.op.noSons = 0;
      list = nl->Rest( nl->Third( nl->First( expr ) ) );
      while ( !nl->IsEmpty( list ) )
      {
        node->u.op.sons[node->u.op.noSons].addr =
          Subtree( nl->First( list ), first, node );

        if ( requestsArguments ) {
          ((OpNode*) node->u.op.sons[node->u.op.noSons].addr)
            ->evaluable = false;
        }
        node->u.op.noSons++;
        list = nl->Rest( list );
      }


      if( !nl->IsAtom(nl->Second(expr)) &&
          TypeOfSymbol(nl->First(nl->Second(expr))) == QP_STREAM )
      {
        node->u.op.isStream = true;
        node->evaluable = false;
      }

      /* Create data structure for result value: */
      ListExpr typeExpr = nl->Second( expr );
      if ( !node->u.op.isStream )
      {
        GetCatalog()->LookUpTypeExpr( typeExpr, typeName,
                                      node->u.op.resultAlgId,
                                      node->u.op.resultTypeId );
      }
      else
      {
        typeExpr = nl->Second( typeExpr );
        GetCatalog()->LookUpTypeExpr( typeExpr, typeName,
                                      node->u.op.resultAlgId,
                                      node->u.op.resultTypeId );
      }
      node->u.op.resultWord =
        (algebraManager->CreateObj
          ( node->u.op.resultAlgId, node->u.op.resultTypeId ))
            ( GetCatalog()->NumericType( node->typeExpr ) );

      if (traceNodes)
      {
        cout << "QP_APPLYOP:" << endl;
        cout << *node << endl;
      }
      return (node);
    }
    case QP_ABSTRACTION:
    {
      node = Subtree( nl->Third( nl->First( expr ) ), first, node );
      node->evaluable = false;
      node->typeExpr = nl->Second( expr );
      node->isRoot = oldfirst;
      node->u.op.isFun = true;
      node->u.op.funNo = nl->IntValue(nl->Fourth(nl->First(expr)));
      node->u.op.funArgs = argVectors[node->u.op.funNo-1];
      if (traceNodes)
      {
        cout << "QP_ABSTRACTION:" << endl;
        cout << *node << endl;
      }
      return (node);
    }
    case QP_IDENTIFIER:
    {
      node = new OpNode(Object);
      node->evaluable = false;
      node->typeExpr = nl->Second( expr );
      node->isRoot = oldfirst;
      if (traceNodes)
      {
        cout << "QP_IDENTIFIER:" << endl;
        cout << *node << endl;
      }
      return (node);
    }
    case QP_ARGLIST:
    {
      node = new OpNode(Operator);
      node->typeExpr = nl->Second( expr );
      node->isRoot = oldfirst;
      /* special operator [0, 1] means arglist */
      node->u.op.algebraId = 0;
      node->u.op.opFunId = 1;

      list = nl->Third( nl->First( expr ) );
      while (!nl->IsEmpty( list ))
      {
        node->u.op.sons[node->u.op.noSons].addr =
          Subtree( nl->First( list ), first, node );
        node->u.op.noSons++;
        list = nl->Rest( list );
      }
      if (traceNodes)
      {
        cout << "QP_ARGLIST:" << endl;
        cout << *node << endl;
      }
      return (node);
    }
    case QP_FUNCTION:
    {
      // spm: should never be reached!
      throw qp_error( "Unexpected function subtree" );
      OpTree subNode = Subtree( nl->Third( nl->First( expr ) ),
                                first, node);
      if (traceNodes)
      {
        cout << "QP_FUNCTION:" << endl;
        cout << *subNode << endl;
      }
      return (subNode);
    }
    case QP_APPLYABS:
    case QP_APPLYFUN:
    {
      node = new OpNode(Operator);
      node->evaluable = true;
      node->typeExpr = nl->Second( expr );
      node->isRoot = oldfirst;
       /* special operator [0, 0] means
          application of an abstraction */
      node->u.op.algebraId = 0;
      node->u.op.opFunId = 0;
      node->u.op.noSons = 1;
      OpNode* sonNode = 
        Subtree( nl->First(nl->Third(nl->First(expr))),
                 first, node ); /* the abstraction */
      node->u.op.sons[0].addr = sonNode;
      list = nl->Rest( nl->Third( nl->First( expr ) ) );
      while ( !nl->IsEmpty( list ) )
      { /* the arguments */
        node->u.op.sons[node->u.op.noSons].addr =
          Subtree( nl->First( list ), first, node );
        node->u.op.noSons++;
        list = nl->Rest( list );
      }
      if (traceNodes) {
        cout << "QP_APPLYABS | QP:APPLYFUN:" << endl;
        cout << *node << endl;
      }
      node->u.op.isStream = sonNode->u.op.isStream;
      node->evaluable = ! node->u.op.isStream;
      node->u.op.supportsProgress = sonNode->u.op.supportsProgress;
      return (node);
    }
    case QP_COUNTERDEF:
    {
      node = Subtree( nl->Fourth( nl->First( expr )), first, node);

      if ( node->nodetype == Operator )
        node->u.op.counterNo =
          nl->IntValue(nl->Third(nl->First(expr)));
      if (traceNodes)
      {
        cout << "QP_COUNTERDEF:" << endl;
        cout << *node << endl;
      }
      return(node);
    }
    case QP_PREDINFODEF:
    {
      node = Subtree( nl->Fifth( nl->First( expr )), first, node);

      if ( node->nodetype == Operator )
        node->u.op.selectivity =
          nl->RealValue(nl->Third(nl->First(expr)));
        node->u.op.predCost =
          nl->RealValue(nl->Fourth(nl->First(expr)));

      if (traceNodes)
      {
        cout << "QP_PREDINFODEF:" << endl;
        cout << *node << endl;
      }
      return(node);
    }
    case QP_MEMORYDEF:
    {
      node = Subtree( nl->Fourth( nl->First( expr )), oldfirst, node);

      if ( node->nodetype == Operator ) {
        if ( node->u.op.usesMemory )
          node->u.op.memorySize =
            nl->IntValue(nl->Third(nl->First(expr)));
        else {
          cout << "Operator " << node->u.op.theOperator->GetName() <<
            " does not support memory assignment." << endl;
          cout << "Memory assignment ignored." << endl;
        }
      }
      if (traceNodes)
      {
        cout << "QP_MEMORYDEF:" << endl;
        cout << *node << endl;
      }
      return(node);
    }


    default:
    {
      cerr << "subtree: unexpected stuff in annotated expr" << endl;
      cerr << "The expression is: " << endl;
      nl->WriteListExpr( expr, cout, 2 );
      cout << endl;
      exit(1);
    }
  }

}



/************************************************************************
3.5 Memory Management

This is a relatively recent addition to the query processor. Up to now 
(October 2011) the memory usage of operators was controlled by a configuration parameter MaxMemPerOperator, 16 MB by default. That is, each operator could use the amount of memory provided by this constant. Since one does not know how many such operators will occur in a query, this constant has to be relatively small, even if a large main memory space is available.

The new simple concept for memory management has two aspects:

  * Memory can be assigned explicitly and individually in a query to a memory using operator, using a notation {memory [<]size[>]} where size is an int specifying the amount of memory for this operator in MB. For example:

----	query plz100 feed sortby[Ort asc] {memory 512} consume
----

Hence the ~sortby~ operator can use 512 MB. 

  * A global parameter ~GlobalMemory~ is introduced, by default 512 MB. Further, each operator using memory registers with a function [<]operator[>].UsesMemory() (analogous to functions such as [<]operator[>].EnableProgress()). Hence the query processor is aware of which operators need internal memory. It distributes the available global memory equally to such operators.

The two techniques are implemented as follows. We start with the second technique.

  * Class ~operator~ (in include/Operator.h) gets a new field ~usesMemory~, by default false. By the method ~SetUsesMemory~ this field can be set to true. This is done in the registration of an operator, e.g. in Algebras/ExtRelation-2/ExtRelation2Algebra.cpp for the ~sortby~ operator:

----	AddOperator(&extrel2::extrelsortby); 
	  extrel2::extrelsortby.SetUsesMemory(); 
----

By the method ~UsesMemory~ one can check whether an operator has registered to use memory.

  * In the query processor, the operator tree node structure (~OpNode~) gets two additional fields ~usesMemory~  (default false) and ~memorySize~ (default 0). In the construction of the operator tree (~SubtreeX~), the query processor checks for an operator node whether this operator has registered to use memory and in this case sets the ~usesMemory~ field to true.

  * The configuration file SecondoConfig.ini is extended by a parameter ~GlobalMemory~:

----    # Global memory available for all operators in MB
        # default is 512
        GlobalMemory=1024
----

  * The query processor gets a new member ~globalMemory~ and a method ~SetGlobalMemory~ (QueryProcessor.h). This is used in SecondoInterface.cpp to set the value to the parameter ~GlobalMemory~ from secondoConfig.ini. Also the query processor gets a new method ~GetMemorySize(s)~ that can be used by an operator to check the memory size stored in its operator tree node.

Two new functions ~countMemoryOperators~ and ~distributeMemory~, introduced in this section, are used as follows. The first traverses the operator tree after its construction and counts the number of operators using memory. It also observes to which operators memory has already been assigned explicitly. Based on this, the amount of memory available per operator is computed. The second function then traverses the tree again and for each memory using operator sets its ~memorySize~ field to this amount. This happens within ~Construct~.

The first technique (explicit annotation of an operator) is implemented by extending the parser to translate the notation  [<]expr[>] {memory 512} into the use of a pseudo operator:

----    (memory 512 translate(<expr>))
----

This pseudo-operator is then used in the annotation of the query and the construction of the operator tree to move the value 512 into the operator node's memorySize field.

*/

/*
Count the number of operators in the operator tree ~node~ that are registered to use memory buffers and do not yet have memory assigned, and determine the amount of memory already used by explicit assignments.

*/

  int countMemoryOperators(OpNode* node, size_t& spentAlready){
    int memOps = 0;
    if(!node){
      cout << "the tree is null" << endl;
      return 0;
    }
    if(node->nodetype == Operator){
      if ( node->u.op.usesMemory ) {
        if ( node->u.op.memorySize == 0 )
          memOps = 1;
        else spentAlready += node->u.op.memorySize;
      }
      for(int i=0; i<node->u.op.noSons; i++){
        memOps += countMemoryOperators(
          static_cast<OpNode*>(node->u.op.sons[i].addr), spentAlready);
      }
    } 
    return memOps;
  }

/*
Assign to each operator in the tree ~node~ that is registered as using memory an amount of memory ~perMemoryOperator~.

*/

  void distributeMemory(OpNode* node, size_t perMemoryOperator){
    if(!node){
      cout << "the tree is null" << endl;
      return;
    }
    if(node->nodetype == Operator){
      if ( node->u.op.usesMemory && node->u.op.memorySize == 0) 
        node->u.op.memorySize = perMemoryOperator;
      for(int i=0; i<node->u.op.noSons; i++){
        distributeMemory(
          static_cast<OpNode*>(node->u.op.sons[i].addr), 
            perMemoryOperator);
      }
    } 
    return;
  }



/*****************************************************************************
3.6 Building a Tree from a Query: Procedures ~construct~ and ~destroy~

*/
void
QueryProcessor::Construct( const ListExpr expr,
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
result type of the expression in ~resultType~. If there is a type error,
~correct~ is set to FALSE and ~resultType~ contains a symbol ~typeerror~.

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
  correct = false;
  evaluable = false;
  isFunction = false;
  useHeartbeat = RTFlag::isActive("QP:HEARTBEAT");

  ListExpr list = nl->TheEmptyList();
  ListExpr type;

  try
  {
    list = AnnotateX( expr, defined );
  }
  catch (runtime_error r)
  {
    cmsg.error() << r.what() << endl;
    cmsg.send();
  }

  if ( nl->ListLength( list ) < 2 ) {
    DestroyValuesArray();
    throw ERR_IN_QUERY_EXPR;
  }
 
  resultType = nl->Second( list );
  if ( TypeOfSymbol(resultType) == QP_TYPEERROR )
  {   // check if a type error was detected
    if (nl->ListLength(nl->Third(nl->First(list))) > 1) {
      if (nl->ListLength(nl->First(nl->Second(nl->Third(nl->First(list))))) >1){
        try {
          type = nl->Second(nl->First(nl->Second(nl->Third(nl->First(list)))));
          if (nl->ToString(type) != "pointer") {
            DestroyValuesArray();
          }
        }
        catch (...) {
          DestroyValuesArray();
        }
        throw ERR_IN_QUERY_EXPR;
      }
      else {
        DestroyValuesArray();
      }
    }
    else {
      DestroyValuesArray();
    }
    
  }

  // Make a consistency check of the annotated list structure.
  // There should be no typeerror symbol in the list. This may
  // be helpful to detect bugs in the annotate function.

  vector<ListExpr> allAtoms;
  nl->ExtractAtoms( resultType, allAtoms );

  for ( vector<ListExpr>::const_iterator it = allAtoms.begin();
  it != allAtoms.end();
  it++ )
  {
    if ( nl->AtomType(*it) == SymbolType &&
   TypeOfSymbol(*it) == QP_TYPEERROR )
    {
      DestroyValuesArray();
      cerr << endl
     << "Annotated list contains a \"typeerror\" symbol, "
        "hence the result type should be \"typeerror\"."
        "Maybe there is a bug in some operators type map "
        "function or in the annotate function of the "
        "query processor." << endl;
      throw ERR_IN_QUERY_EXPR;
    }
  }

  correct = true;
  if( !defined )
  {
    DestroyValuesArray();
    throw ERR_UNDEF_OBJ_VALUE;
  }

  tree = SubtreeX( list );
  
  // Distribute remaining global memory (after explicit assignments)
  // evenly to operators using main memory. In any case such operators get
  // a minimum of 16 MB.

  size_t memorySpent = 0;
  int noMemoryOperators = countMemoryOperators( tree, memorySpent );
  size_t perOperator = 0;
  if ( noMemoryOperators > 0 ) {
    if ( globalMemory > memorySpent )
      perOperator = (globalMemory - memorySpent) / noMemoryOperators;
    if ( perOperator < 16 ) perOperator = 16;
    distributeMemory( tree, perOperator );
  }

  static bool printNoMemOps = !RTFlag::isActive("QP:DISABLE_PRINT_NO_MEM_OPS");
  if(printNoMemOps){
         cout << "noMemoryOperators = " << noMemoryOperators << endl;
         cout << "perOperator = " << perOperator << endl;
  }
         // print(cout, tree);
  

  QueryTree = tree;
  ResetCounters();

  evaluable = tree->evaluable;

  if(evaluable && tree->isAbstraction()){
     evaluable = !tree->isStreamOp();
  }

  isFunction = (tree->nodetype == Operator) ?
     tree->u.op.isFun : false;

  if (!evaluable && !isFunction ) {
    Destroy(tree, true);
    tree=0;
    throw ERR_EXPR_NOT_EVALUABLE;
  }

}

void
QueryProcessor::Destroy( void*& node, bool destroyRootValue )
{
  OpTree aux = static_cast<OpTree>(node);
  Destroy( aux, destroyRootValue );
  QueryTree = 0;
}

void
QueryProcessor::Destroy( OpTree& tree, bool destroyRootValue )
{
/*
Deletes an operator tree object.

*/
  //OpTree tree = static_cast<OpTree>( node );
  // reinitialize static OpNode information
  OpNodeAddr2Id.clear();
  OpNodeIdCtr = 0;

  // used for trace messages
  argsPrinted.clear();

  if ( tree != 0 )
  {
    switch (tree->nodetype)
    {
      case Operator:
      {
        if(tree->u.op.supportsProgress){
          CloseProgress(tree);
        }
        for ( int i = 0; i < tree->u.op.noSons; i++ )
        {
          if( tree->u.op.resultAlgId == 0 )
            Destroy( tree->u.op.sons[i].addr, destroyRootValue );
          else
            Destroy( tree->u.op.sons[i].addr, true );
        } /* for */
        if ( tree->u.op.isFun )
        {
          //delete tree->u.op.funArgs; // deleted by queryprocessor destructor
          // u.op.funArgs = 0;
          ;
        }
        if ( (tree->u.op.resultAlgId != 0) &&
             ( (tree->u.op.isFun && destroyRootValue) ||
               (!tree->u.op.isFun &&
                (!tree->isRoot || destroyRootValue))))
        {
          /* space was allocated for result */
          DeleteResultStorage(tree);
        }
        if( (tree->u.op.local.addr != 0) && debugLocal)
        {
          cerr << "LocalInfo for operator "
              << tree->u.op.theOperator->GetName()
              << " was not destroyed. local=" << tree->u.op.local.addr
              << endl;
        }
        if(tree->u.op.costEstimation){
          delete tree->u.op.costEstimation;
          tree->u.op.costEstimation = 0;
        }
        break;
      }
      case Object:
      {
        if( !tree->isRoot || destroyRootValue )
          // will delete the object if it is not the root of the
          // tree or if it is the root of the tree and the flag
          // ~destroyRootValue~ is true
        {
          string typeName;
          int algebraId, typeId;
          if( tree->u.dobj.value.addr != 0 &&
              GetCatalog()->LookUpTypeExpr( tree->typeExpr, typeName,
                                            algebraId, typeId ) )
          {
           // an object which has the map type contains the
           // result type as its value, so we have to change the
           // algebra id and type id from fun to its result type
            ListExpr t = tree->typeExpr;
            if(nl->HasLength(t,3) && 
               (nl->AtomType(nl->First(t))==SymbolType) && 
               (nl->SymbolValue(nl->First(t))=="map")){
               GetCatalog()->LookUpTypeExpr( nl->Third(tree->typeExpr), 
                                             typeName, algebraId, typeId );
            }
            if( tree->u.dobj.isConstant ){
              (algebraManager->DeleteObj( algebraId, typeId ))
                ( GetCatalog()->NumericType( tree->typeExpr ),
                  tree->u.dobj.value );
            } else
            {
              if( tree->u.dobj.isModified )
              {
                string objName =
                  nl->SymbolValue(tree->u.symbol);
                GetCatalog()->ModifyObject( objName,
                                            tree->u.dobj.value );
              }
              else
              {
                (algebraManager->CloseObj( algebraId, typeId ))
                  ( GetCatalog()->NumericType( tree->typeExpr ),
                    tree->u.dobj.value );
              }
            }
          }
    tree->u.dobj.value.addr = 0;
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
Translate a message code into its name

*/

string
QueryProcessor::MsgToStr(const int msg) {
 int argNo = (msg) / FUNMSG;
 string prefix = "";
 if(argNo>0){
    stringstream ss;
    ss << "FUN, ARG = " << argNo << ":";
    prefix = ss.str();
 }
 
 switch (msg % FUNMSG) {
    case OPEN: return prefix + "OPEN";
    case REQUEST:  return prefix + "REQUEST";
    case CLOSE: return prefix + "CLOSE";
    case YIELD: return prefix + "YIELD";
    case CANCEL: return prefix + "CANCEL";
    case CARDINALITY : return prefix + "CARDINALITY";
    case REQUESTPROGRESS: return prefix + "REQUESTPROGRESS";
    case CLOSEPROGRESS: return prefix + "CLOSEPROGRESS";
    case FAILURE : return prefix + "FAILURE";
    default: return prefix + "UNKNOWN";
 }
}


/*
6.3 Evaluating an Operator Tree: Procedure ~eval~

~EvalP~ is a wrapper for ~Eval~ to include progressview management. ~EvalS~ is a wrapper to be used if no progress management is desired, especially for subqueries evaluated within an operator implementation.

*/


void
QueryProcessor::EvalP( void* node,
                      Word& result,
                      const int message)
{
  //progressView = new ProgressView();
  allowProgress = true;
  try
  {
    Eval( node, result, 1 );
  }
  catch (runtime_error r)
  {
    cmsg.error() << r.what() << endl;
    cmsg.send();
  }

  allowProgress = false;
  progressView->FinishProgressView();
  //delete progressView;
  //progressView=0;
}


void
QueryProcessor::EvalS( void* node,
                      Word& result,
                      const int message)
{
  //progressView = 0;
  allowProgress = false;

  try
  {
    Eval( node, result, 1 );
  }
  catch (runtime_error r)
  {
    cmsg.error() << r.what() << endl;
    cmsg.send();
  }
}





void
QueryProcessor::Eval( void* node,
                      Word& result,
                      const int message)
{
/*
Traverses the operator tree ~tree~ calling operator implementations for
each node, and returns the result in ~result~. The ~message~ could be OPEN,
REQUEST, or CLOSE and is used only if the root node produces a stream.

Code just needed for tracing and error eporting is shown indented.

*/


  OpNode* tree = static_cast<OpNode*>( node );

                        static string fn("QP:Eval ");
                        static map<int, bool>::const_iterator it;


  //cout << "Calling eval/" << msg2str(message) <<  " for node:" << endl;
  //print(cout,tree);
  //cout << (*tree) << endl << endl;

  




  int i = 0;
  int status = 0;
  ArgVector arg;
  //result.addr = 0;

  if ( tree == 0 )
  {
                        cerr << fn << "Called with tree == 0!" << endl;
    abort();
  }
  else
  {
                        if ( traceNodes )
                        cerr << fn << "*** Eval( Node " << tree->id
                        << ", result = " << (void*)result.addr
                        << ", msg = " << MsgToStr(message)
                        << "(" << message << ") ) ***" << endl;


/*
While evaluating the query, the cases

  * Object

  * Pointer

  * Indirect Object (parameter functions)

  * Operator (normal / stream / abstraction)

must be handled.

*/

    switch (tree->nodetype)
    {
      case Object:
      case Pointer:
      {
        result = tree->u.dobj.value;

                        if (traceNodes)
                        cerr << fn << "{Object | Pointer} return ["
                        << (void*)result.addr << "]" << endl;
        return;
      }
/*

*Indirect Object:* This case handles ~indirect objects~ which represent
arguments of a parameter function. In order to support also streams as arguments
for a parameter function, an indirect object can also be an operator.

If the indirect object represents a stream, the argument vector contains at
position MAXARG-argIndex the node of the operator which will then be used to
request the next element.

*/
      case IndirectObject:
      {
        const int argIndex = tree->u.iobj.argIndex;

        if ( (*tree->u.iobj.vector)[MAXARG-argIndex].addr == 0 ) {
          result = (*tree->u.iobj.vector)[argIndex-1];
        }
        else
        {
          // A stream! Request next element
          OpTree caller =
            (OpTree) (*tree->u.iobj.vector)[MAXARG-argIndex].addr;

                        if (traceNodes)
                        cerr << fn <<
                        "Parameter function's caller node = "
                        << caller->id << endl;

          status = algebraManager->Execute( caller->u.op.algebraId,
                                            caller->u.op.opFunId,
                                            caller->u.op.sons, result,
                                            (argIndex*FUNMSG)+message,
                                            caller->u.op.local,
                                            caller );

          tree->u.received = (status == YIELD);
    //cerr << "iobj = " << (status == YIELD) << endl;
        }
                        if (traceNodes)
                        cerr << fn <<
                        "{IndirectObject with Argindex = "
                        << argIndex  << "} return ["
                        << (void*)result.addr << "]" << endl;
        return;
      }
/*

*Operator:* Here we need to distinguish between operators which return a stream (called ~stream operator~) and those which compute an object.

  * If an operator is not a stream operator and the message is not one related to progress, then evaluate all subtrees that are not functions or streams and copy the results to the argument vector.

  * If it is a stream operator, then evaluate all subtrees that are not functions or streams ~only on the OPEN message~. Store the results in a vector ~sonresults~ and copy them to the argument vector.

  * If it is a stream operator with messages REQUEST or CLOSE, just copy the results computed earlier with the OPEN message from vector ~sonresults~ to the argument vector.

Other subtrees are not evaluated, just copied to the argument vector.

Then call the operator's value mapping function.

*/
      case Operator:
      {
        for ( i = 0; i < tree->u.op.noSons; i++ )
        {
            if ( ((OpNode*)(tree->u.op.sons[i].addr))->evaluable )
            {
              if ( !tree->u.op.isStream  && (message <= CLOSE) )   
              //no stream operator, normal evaluation
              {
                        if ( traceNodes )
                        cerr << fn << "Simple op: compute result for son["
                        << i << "]" << endl;

                Eval( tree->u.op.sons[i].addr, arg[i], message );
                 
                //if(message!=CLOSEPROGRESS){assert(arg[i].addr);}

              }
              else // a stream operator
              {
                if ( message == OPEN )
                {
                  Eval( tree->u.op.sons[i].addr, 
                        tree->u.op.sonresults[i], message );

                        if ( traceNodes )
                        cerr << fn << "Stream op: Compute result for son["
                        << i << "]" << endl;
                   //assert(tree->u.op.sonresults[i].addr);

                }
                arg[i] = tree->u.op.sonresults[i];
              
                //if(message!=CLOSEPROGRESS){assert(arg[i].addr);}

              }
            }
            else
            {
              arg[i].addr = tree->u.op.sons[i].addr;

                        if ( traceNodes )
                        cerr << fn << "Argument son[" << i <<
                        "] is a stream" << endl;

             // if(message!=CLOSEPROGRESS){assert(arg[i].addr);}
            }
        }
        if(message<=CLOSE){
          tree->u.op.argsAvailable = true;
        }


        if ( tree->isAbstraction()) { 
          ArgVectorPointer absArgs;

                        if ( traceNodes )
                        {
                        cerr << fn <<
                        "*** Abstraction application " << endl;
                        nl->WriteListExpr( ListOfTree( tree, cerr ),
                                cout, 2 );
                        cerr << endl;
                        }

          // copy arguments from abstraction to argument 
          // vector of the underlying function
          if(message <= CLOSE){
            absArgs = Argument(tree->u.op.sons[0].addr ); // argument vector
            for ( i = 1; i < tree->u.op.noSons; i++ ) {
                (*absArgs)[i-1] = arg[i]; // load arguments for function

                          if ( traceNodes )
                          cerr << fn << "absArgs[" << i-1 << "] = "
                          << (void*)arg[i].addr << endl;
            }
          }
         

          // evaluate the abstraction
          Eval( tree->u.op.sons[0].addr, result, message );

          // forward received 
          tree->u.received = ((OpNode*)tree->u.op.sons[0].addr)->u.received;

          return;
        }



#ifdef USE_PROGRESS
          CheckProgress();
#endif
          if(useHeartbeat){
              HeartBeat();
          }

                        if ( traceNodes )
                        {
                          it = argsPrinted.find(tree->id);
                          if ( (it == argsPrinted.end()))
                          {
                            cerr << fn <<
                            "*** Value mapping function's args" << endl;
                            for ( i = 0; i < tree->u.op.noSons; i++ )
                            {
                              cerr << fn << "arg[" << i << "].addr = "
                              << arg[i].addr << endl;
                            }
                            argsPrinted[tree->id] = true;
                          }
                          cerr << fn << "*** Call value mapping for "
                          << nl->SymbolValue(tree->u.symbol) << endl;
                        }

          tree->u.op.theOperator->incCalls(tree->u.op.opFunId / 65536);
          if(tree->u.op.costEstimation){
             switch(message){
               case CLOSEPROGRESS : break; // not required 
               case REQUESTPROGRESS : status = 
                                 tree->u.op.costEstimation->requestProgress(
                                       arg, (ProgressInfo*)result.addr, 
                                       tree->u.op.local.addr, 
                                       tree->u.op.argsAvailable); 
                                 break;
               default: status =
                            (*(tree->u.op.valueMap))( arg, result, message,
                                                      tree->u.op.local, tree);
                        if((message == REQUEST) && (status==YIELD)){
                          tree->u.op.costEstimation->incReturned();
                        }

             }
          } else {
            status =
               (*(tree->u.op.valueMap))( arg, result, message,
                                      tree->u.op.local, tree );
          }

          tree->u.received = (status == YIELD);
          if ( status == FAILURE )  //new error code
          {
                        cerr << fn << "Evaluation of operator failed."
                        << endl;
            exit( 0 );
          }

                    if (traceNodes){
                        cerr << fn << "Operator return status ="
                        << status << endl;
                    }
        return;
      }
    }
  }
}


void
QueryProcessor::CheckProgress()
{

/*
Interrupts the evaluation of the query tree in order to propagate a progress
message. For timer based interrupts we will use the clock() function
which returns an approximation of CPU time used by this program.
On a multitasking system with much CPU load the time may be
a multiple of the one defined here.

*/

  if(!allowProgress){
     return;
  }

  static clock_t clockDelta = CLOCKS_PER_SEC ;
  static clock_t lastClock = clock();

  // Do a clock check only after some calls of this code branch.
  static const int progressDelta = 100;

  static int progressCtr = progressDelta;
  ProgressInfo progress;

  progressCtr--;

  if ( progressCtr == 0) {

    if ( clock() < lastClock ) {
        lastClock = 0;
    }

    if ( (clock() - lastClock) > clockDelta / 10) {

      allowProgress = false;

      if ( RequestProgress(QueryTree, &progress) ) {
        if ( progressView ) progressView->ModifyProgressView(progress);
      }

      allowProgress = true;
      lastClock = clock();
    }
    progressCtr = progressDelta;
  }
}


void QueryProcessor::HeartBeat(){
  static clock_t lastClock = clock();
  // Do a clock check only after some calls of this code branch.
  static const int heartbeatDelta = 50; //number of calls before doing something
  static const int heartbeatFreq = 5;  // al 5 seconds
  static int hbcount = 0;

  static int heartbeatCount = heartbeatDelta;

  heartbeatCount--;
  if ( heartbeatCount == 0) {
    if ( clock() < lastClock ) {
        lastClock = 0;
    }
    if ( (clock() - lastClock) >= heartbeatFreq*CLOCKS_PER_SEC) {
      lastClock = clock();
      heartbeat_file.seekp(0);
      heartbeat_file << hbcount++ << endl;
      heartbeat_file.flush();
    } 
    heartbeatCount = heartbeatDelta;
  }
}




/*
1.1 Procedures for Cooperation with Operator Evaluation Functions

Function ~Argument~ returns for a given supplier ~s~ a pointer to its argument
vector.  Arguments can be set by writing into the fields of this argument
vector.

*/

ArgVectorPointer
QueryProcessor::Argument( const Supplier s )
{
  OpTree tree = (OpTree) s;
  return (tree->u.op.funArgs);
}

/*
~SetupStreamArg~ saves an operators node in the argument vector of
an parameter function. The ~Eval~ method looks if an operator node is
present in the argument vector. if found this indicates that argument
number ~num~ is a stream.

*/

void
QueryProcessor::SetupStreamArg( const Supplier funNode,
                                const int num, Supplier opNode )
{
   if( !((0 < num) && (num < MAXARG/2)) ) {
     throw qp_error("num out of range!");
   }

   ArgVectorPointer funargs = Argument(funNode);
   (*funargs)[MAXARG-num] = SetWord(opNode);
}


/*
Function ~Request~ calls the parameter function (to which the arguments must have been
supplied before). The result is returned in ~result~.

*/
void
QueryProcessor::Request( const Supplier s, Word& result )
{
  OpTree tree = (OpTree) s;
  //cerr << "nodetype = " << tree->nodetype << endl;
  Eval( tree, result, REQUEST );

  if (tree->nodetype == Operator) {
    if (tree->u.op.isStream && !tree->u.received)
    {
      // no result received, for safeness override
      // the result pointer with null.
      result.addr = 0;
    }
    int counterIndex = tree->u.op.counterNo;
    if ( tree->u.received && counterIndex )
    {
      // increment counter
      if(  !( (counterIndex > 0) || (counterIndex < NO_COUNTERS) ) ) {
        throw qp_error("counterIndex out of range");
      }
      counter[counterIndex]++;
    }
  }
  // Do nothing for all other node types

}

/*
Function ~received~ returns ~true~ if the supplier responded to the previous ~request~ by a
~yield~ message; ~false~ if it responded with ~cancel~.

*/

bool
QueryProcessor::Received( const Supplier s )
{
  OpTree tree = (OpTree) s;
  if ( tree->nodetype == Operator )
    return (tree->u.received);
  else if ( tree->nodetype == IndirectObject)
    return (tree->u.received);
  else
    cerr << "Call of QueryProcessor::Received() not allowed "
   << "for this node type" << endl;
    abort();
}

/*
~Open~ changes state of the supplier stream to ~open~.

*/
void
QueryProcessor::Open( const Supplier s )
{
  Word result = SetWord( Address(0) );
  OpTree tree = (OpTree) s;
  Eval( tree, result, OPEN );
}

void QueryProcessor::SetEvaluable(Supplier s, bool value){
    OpNode* tree = static_cast<OpNode*>(s);
    tree->evaluable=false;
}


/*
~Close~ changes state of the supplier stream to ~closed~. No effect, if the
stream is closed already.

*/
void
QueryProcessor::Close( const Supplier s )
{
  Word result;
  Eval( (OpTree) s, result, CLOSE );
}




/*
~CloseProgress~ deallocates data structures for operators that support progress. No effect when called for an operator not supporting progress.

*/
void
QueryProcessor::CloseProgress( const Supplier s )
{
  Word result;
  OpTree tree = (OpTree) s;
  bool trace = false;

  if ( trace ) cout << "CloseProgress called with Supplier = " <<
        (void*) s << endl;

  if ( tree->nodetype == Operator )
  {
    if ( tree->u.op.supportsProgress )
      Eval( tree, result, CLOSEPROGRESS );
  }
}





/*
~RequestProgress~ evaluates the subtree ~s~ for a PROGRESS message. It returns true iff a progress info has been received. In ~p~ the address of a ProgressInfo must be passed.

*/
bool
QueryProcessor::RequestProgress( const Supplier s, ProgressInfo* p )
{
  Word result;
  OpTree tree = (OpTree) s;
  bool trace = false;  //set to true for tracing

  if ( trace ){
       cout << "RequestProgress called with Supplier = " 
            << (void*) s << "  ProgressInfo* = " << (void*) p << endl;
  }


  assert(tree != 0);

  // RequestProgress is available for operator node only
  // if(tree->nodetype != Operator){
  //   return  false;
  // }

  // check if the operator supports progress
  // if ( !tree->u.op.supportsProgress ){
  //   return false;
  // }

  // Operators are only asked if they support progress. But one can also ask
  // Indirect Objects (stream arguments to parameter functions)
  if ( tree->nodetype == Operator )
    if ( !tree->u.op.supportsProgress )
      return false;

  result = SetWord(p);
  Eval(tree, result, REQUESTPROGRESS);
  if(tree->u.received){
    if(traceProgress  || (debugProgress && !p->checkRanges())){
        cout << "Result of REQUESTPROGRESS for " << (void*) s ;
        if ( tree->nodetype == Operator ) {
          cout  << "(" << tree->u.op.theOperator->GetName() << ")" << endl;
        }
        else cout << "(stream arg of param function)" << endl;
        cout << (*p) << endl;
        cout << "======================" << endl;
     }
   } else {
     if(traceProgress){
        cout << "REQUESTPROGRESS for " << (void*) s;
        if ( tree->nodetype == Operator ) {
          cout  << "(" << tree->u.op.theOperator->GetName() << ")" << endl;
        }
        else cout << "(stream arg of param function)" << endl;
        cout << "has no result" << endl;
        cout << "======================" << endl;
     }
   }

   return (tree->u.received);
}





/*
>From a given supplier ~s~ get its Selectivity

*/
double
QueryProcessor::GetSelectivity( const Supplier s)
{
  OpTree node = (OpTree) s;
  return node->u.op.selectivity;
}

/*
>From a given supplier ~s~ get its Predicate Cost

*/
double
QueryProcessor::GetPredCost( const Supplier s)
{
  OpTree node = (OpTree) s;
  return node->u.op.predCost;
}

/*
For a given supplier ~s~ set its Selectivity ~selectivity~

*/
void
QueryProcessor::SetSelectivity( const Supplier s, const double selectivity)
{
  OpTree node = (OpTree) s;
  node->u.op.selectivity = selectivity;
}

/*
For a given supplier ~s~ set its Predicate Cost ~predCost~

*/
void
QueryProcessor::SetPredCost( const Supplier s, const double predCost)
{
  OpTree node = (OpTree) s;
  node->u.op.predCost = predCost;
}



/*
Sets the maximum memory available per operator.

*/
void QueryProcessor::SetMaxMemPerOperator(size_t value) 
{ 
  maxMemPerOperator = value; 
}


/*
Returns the maximum memory available per operator.

*/
size_t QueryProcessor::MemoryAvailableForOperator( const Supplier s ) 
{ 
  cout << "MemoryAvailableForOperator = " << maxMemPerOperator;
  assert(false);

  return maxMemPerOperator; 
}


size_t QueryProcessor::FixedMemory()
{
  return 16 * 1024 * 1024;
}

/*
Replaces MemoryAvailableForOperator() for some (very rare) operators that are unable to register as using memory. Fixed amount of 16 MB.

*/




/*
Sets the global memory available for all operators together. To be distributed 
by query processor or optimizer.

*/
void QueryProcessor::SetGlobalMemory(size_t value) 
{ 
  globalMemory = value; 
}



/*
From a given supplier ~s~ get its Memory Size

*/
size_t
QueryProcessor::GetMemorySize( const Supplier s)
{
  OpTree node = (OpTree) s;

  if ( node->u.op.memorySize == 0 ) {
    cout << "Operator " << node->u.op.theOperator->GetName() <<
    " requests memory from query processor"  << endl <<
    "although it did not register as using memory." << endl;
    assert(false);
  }

  // cout << node->u.op.memorySize << " assigned to operator " << 
  //   node->u.op.theOperator->GetName() << "." << endl;
  return node->u.op.memorySize;
}



/*
>From a given supplier ~s~ that must not represent an argument list,
get its son number ~no~.

*/
Supplier
QueryProcessor::GetSupplierSon( const Supplier s, const int no )
{
  OpTree node = (OpTree) s;
  return (node->u.op.sons[no].addr);
}


/*
Check whether an argument is an object or an operator.

*/
bool
QueryProcessor::IsObjectNode( const Supplier s ) const
{
  OpTree tree = static_cast<OpTree>( s );
  return (tree->nodetype == Object);
}


bool
QueryProcessor::IsOperatorNode( const Supplier s ) const
{
  OpTree tree = static_cast<OpTree>( s );
  return tree->nodetype == Operator;
}

bool
QueryProcessor::IsFunctionNode( const Supplier s ) const
{
  OpTree tree = static_cast<OpTree>( s );
  return (tree->nodetype == Operator)
     && ((tree->u.op.resultAlgId == 0) || (tree->u.op.isFun));
}




/*
Function ~GetSupplier~

>From a given supplier ~s~ that represents an argument list, get its son
number ~no~. Can be used to traverse the operator tree in order to
access arguments within (nested) argument lists. Values or function or
stream evaluation can then be obtained from the returned supplier by the
usual calls to ~request~ etc.

*/
Supplier
QueryProcessor::GetSupplier( const Supplier s, const int no )
{

  OpTree node = (OpTree) s;

  if ( (node->u.op.algebraId == 0) && (node->u.op.opFunId == 1) )
  {        /* is an arglist node*/
    if ( no < node->u.op.noSons )
    {
      return (node->u.op.sons[no].addr);
    }
    else
    {
      cerr << "Error - getSupplier: argument does not exist. "
           << endl;
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

/*
For each operator in an operator tree, the query processor allocates a
torage block for the result value (which it also destroys after
execution of the query). The operator's evaluation function can call
this procedure ~resultStorage~ to get the Word of that storage block.
As a parameter ~s~, the operator's node Word has to be given which is
passed to the evaluation function in parameter ~opTreeNode~.

*/

Word
QueryProcessor::ResultStorage( const Supplier s )
{
  OpTree tree = (OpTree) s;
  if (tree->nodetype != Operator) {
    throw qp_error("Nodetype != Operator");
  }

  return (tree->u.op.resultWord);
}

CostEstimation*
QueryProcessor::getCostEstimation( const Supplier s )
{
  OpTree tree = (OpTree) s;
  if (tree->nodetype != Operator) {
    throw qp_error("Nodetype != Operator");
  }

  return (tree->u.op.costEstimation);
}

/*
Function ~ChangeResultStorage~.

Normally, the Query-Processor allocates and destroys result objects
automatically. But some operators may not use the default result storage in particular they
create their own storage for the result. This function should be used for this case.
Those operators must first free the default result storage, afterwards
a new one can be passed by parameter ~w~. Moreover an specific delete function
needs to be assigned by calling ~SetDeleteFunction~

*/
void
QueryProcessor::ChangeResultStorage( const Supplier s, const Word w )
{
  OpTree tree = (OpTree) s;
  if (tree->nodetype != Operator) {
    throw qp_error("Nodetype != Operator");
  }

  tree->u.op.resultWord.setAddr(w.addr);
}

void
QueryProcessor::SetDeleteFunction( const Supplier s,
                                   const ObjectDeletion f )
{
  OpTree tree = (OpTree) s;
  if (tree->nodetype != Operator) {
    throw qp_error("Nodetype != Operator");
  }

  tree->u.op.deleteFun = f;
}




void
QueryProcessor::DeleteResultStorage( const Supplier s )
{
  OpTree tree = (OpTree) s;
  if (tree->nodetype != Operator) {
    throw qp_error("Nodetype != Operator");
  }

  int algId = tree->u.op.resultAlgId;
  int typeId = tree->u.op.resultTypeId;

  if ( tree->u.op.resultWord.addr ) {
    if (!tree->u.op.deleteFun)
    {
      algebraManager->DeleteObj( algId, typeId)
        ( GetCatalog()->NumericType( tree->typeExpr ),
          tree->u.op.resultWord );
    }
    else
    {
      tree->u.op.deleteFun(
        GetCatalog()->NumericType( tree->typeExpr ),
        tree->u.op.resultWord );
    }
  }
  tree->u.op.resultWord.setAddr(0);
}

void
QueryProcessor::ReInitResultStorage( const Supplier s )
{

  OpTree tree = (OpTree) s;
  if (tree->nodetype != Operator) {
    throw qp_error("Nodetype != Operator");
  }

  //cerr << "u.op.isFun = " << tree->u.op.isFun << endl;
  ListExpr type = (tree->u.op.isFun) ? nl->Third(tree->typeExpr)
                               : tree->typeExpr;

  int algId = tree->u.op.resultAlgId;
  int typeId = tree->u.op.resultTypeId;
  ListExpr numType = GetCatalog()->NumericType( type );

  //cerr << "typeExpr " << nl->ToString(tree->typeExpr) << endl;



  if ( (algId == 0) && (typeId == 0) )
  {
     ListExpr tmp = numType;
     while(nl->AtomType(nl->First(tmp))!=IntType ){
       tmp = nl->First(tmp);
     }
     algId = nl->IntValue( nl->First(tmp) );
     typeId = nl->IntValue( nl->Second(tmp) );
  }

  //cerr << nl->ToString(numType) << endl;
  //cerr << "algId = " <<  algId << endl;
  //cerr << "typeId = " <<  typeId << endl;


  tree->u.op.resultWord =
      algebraManager->CreateObj( algId, typeId )( numType );

}

/*
~GetNoSons~ returns the number of sons of the operator node ~s~ of the operator
tree.

*/
int
QueryProcessor::GetNoSons( const Supplier s )
{
  OpTree tree = (OpTree) s;
  if ( tree->nodetype == Operator )
  {
    return (tree->u.op.noSons);
  }
  else
  {
    cerr << "Error - GetNoSons: not an operator node. " << endl;
    assert(false);
    exit(0);
  }
}

/*
~GetSon~ returns the ~i~-th son of the operator node ~s~ of the operator
tree.

*/
Supplier
QueryProcessor::GetSon( const Supplier s, int i )
{
  OpTree tree = (OpTree) s;
  if ( tree->nodetype != Operator )
  {
    throw qp_error("The given supplier s is no operator node");
  }

  if ( !( i >= 0 && i < tree->u.op.noSons ) ) {
    throw qp_error("son number i is out of range");
  }
  return tree->u.op.sons[i].addr;
}

/*
~GetType~ returns the type expression of the node ~s~ of the operator tree.

*/
ListExpr
QueryProcessor::GetType( const Supplier s )
{
  OpTree tree = (OpTree) s;
  if ( (tree->nodetype == Operator) && tree->u.op.isFun )
  {
    // the list structure will be (map ... R) but in case of a
    // function application only the result type
    // R is needed. Example:
    //
    //    plz  staedte loopz[f1: . feed .. feed hashjoin,
    //      f2: . feed .. feed sortmergejoin] count
    //
    // Since the join implementations define the result tuple type by
    // calling this method and
    // the join is the root of the function's operator tree the
    // list (map ... R) will be returned
    // but in this case returning R is correct.
    int n = nl->ListLength(tree->typeExpr);
    return nl->Nth(n, tree->typeExpr);
  }
  else
  {
    return (tree->typeExpr);
  }
}

/*
~GetType~ returns the type expression of the node ~s~ of the operator tree.

*/
ListExpr
QueryProcessor::GetSupplierTypeExpr( const Supplier s )
{
  OpTree tree = (OpTree) s;
  return tree->typeExpr;
}


/*
~SetModified~ sets a node ~s~ of the operator tree as modified. The node must be
of type ~Object~.

*/
void
QueryProcessor::SetModified( const Supplier s )
{
  OpTree tree = (OpTree) s;
  if( tree->nodetype != Object ) { // ignore for non object nodes
    return;
  }
  tree->u.dobj.isModified = true;
}

/*
1.3 Using Counters

*/
void
QueryProcessor::ResetCounters()
{
  for (int i = 1; i < NO_COUNTERS; i++)
    counter[i] = 0;
}

int
QueryProcessor::GetCounter(const int index)
{
  if( !((index > 0) && (index < NO_COUNTERS)) ) {
    throw qp_error("Counter index out of range");
  }
  return counter[index];
}


ListExpr
QueryProcessor::GetCounters()
{
  ListExpr list = nl->TheEmptyList();
  ListExpr last = nl->TheEmptyList();

  list = nl->OneElemList(
           nl->TwoElemList(
             nl->IntAtom(1),
             nl->IntAtom(counter[1]) ));
  last = list;

  for (int i = 2; i < NO_COUNTERS; i++)
  {
    last = nl->Append( last,
                       nl->TwoElemList(
                         nl->IntAtom(i),
                         nl->IntAtom(counter[i])));
  }
  return list;
}


void
QueryProcessor::ResetTimer()
{
  evalRunTime.start();
}

StopWatch&
QueryProcessor::GetTimer()
{
  return evalRunTime;
}

void
QueryProcessor::SetDebugLevel( const int level )
{
   debugMode = (level & 1) > 0;
   traceMode = (level & 2) > 0;
   traceNodes = (level & 4) > 0;
   debugLocal = (level & 8) > 0;
   debugProgress = (level & 16) > 0;
   traceProgress = (level & 32) > 0;
}

bool
    QueryProcessor::ExecuteQuery( const string& queryListStr,
                                  Word& queryResult,
                                  const size_t availableMemory 
                                    /* = DEFAULT_GLOBAL_MEMORY */)
{
  string typeString(""), errorString("");
  bool success = true;
  bool correct = false, evaluable = false, defined = false, isFunction = false;
  ListExpr queryList;
  NestedList* nli = SecondoSystem::GetNestedList();
  success = nli->ReadFromString( queryListStr, queryList );
  if (!success)
  {
    errorString += "Error in operator query. ";
  }
  else
  {
    success = ExecuteQuery( queryList,
                            queryResult,
                            typeString,
                            errorString,
                            correct,
                            evaluable,
                            defined,
                            isFunction,
                            availableMemory);
  }
  if (errorString != "OK")
  {
    cout << errorString << endl;
  }
  return success;
}

bool
QueryProcessor::ExecuteQuery( const ListExpr& commandList,
                              Word& queryResult,
                              string& typeString,
                              string& errorString,
                              bool& correct,
                              bool& evaluable,
                              bool& defined,
                              bool& isFunction,
                              const size_t availableMemory 
                                /* = DEFAULT_GLOBAL_MEMORY */ )
{ // reset return parameters
  errorString  = "";
  correct      = false;
  evaluable    = false;
  defined      = false;
  isFunction   = false;
  // initialise local objects
  OpTree tree  = 0;
  string errorMessage = "";
  NestedList* nli = SecondoSystem::GetNestedList();
  ListExpr resultType = nli->TheEmptyList();
//   ErrorReporter::GetErrorMessage(errorMessage); // clear
  QueryProcessor* qpp =
    new QueryProcessor( nli,
                        SecondoSystem::GetAlgebraManager(),
                        availableMemory );
  qpp->Construct( commandList, correct,
                  evaluable, defined, isFunction, tree, resultType );
  nli->WriteToString(typeString,resultType);
  if ( !defined )
  {
//     ErrorReporter::GetErrorMessage(errorMessage);
//     errorString = "Object query refers to undefined value: " + errorMessage;
    errorString += "Object query refers to undefined value. ";
    queryResult = SetWord( Address(0) );
  }
  else if ( correct )
  {
    if ( evaluable )
    {
      // evaluate the operator tree
      qpp->EvalS( tree, queryResult, OPEN );
      qpp->Destroy( tree, false );
      errorString += "OK";
    }
    else
    {
//       ErrorReporter::GetErrorMessage(errorMessage);
//       errorString = "Operator query not evaluable: " + errorMessage;
      errorString += "Operator query not evaluable. ";
      queryResult = SetWord( Address(0) );
    }
  }
  else
  {
//     ErrorReporter::GetErrorMessage(errorMessage);
//     errorString = "Error in operator query: " + errorMessage;
    errorString += "Error in operator query. ";
    queryResult = SetWord( Address(0) );
  }
  delete qpp;
  return ( defined && correct && evaluable );
}

bool
QueryProcessor::GetNLArgValueInTM(const NList& arg, NList& value,
                                  const size_t availableMemory 
                                  /* = DEFAULT_GLOBAL_MEMORY */)
{
  if ((arg.isList()) || (arg.isSymbol()))
  {
    ListExpr queryList = arg.listExpr();
    bool success;
    Word queryresultword;
    string typestring   = "";
    string errorstring   = "";
    bool correct = false;
    bool evaluable = false;
    bool defined = false;
    bool isFunction = false;
    success =
        QueryProcessor::ExecuteQuery(queryList,
                                      queryresultword,
                                      typestring,
                                      errorstring,
                                      correct,
                                      evaluable,
                                      defined,
                                      isFunction,
                                      availableMemory);
    ListExpr queryResType;
    NestedList* nli = SecondoSystem::GetNestedList();
    if (!nli->ReadFromString(typestring, queryResType))
    {
      cerr << "ERROR! Invalid argument type. "
          << errorstring << endl;
      return false;
    }
    else
    {
      if (correct && evaluable &&
          defined && (typestring != "typeerror"))
      {
        ListExpr valueList = SecondoSystem::GetCatalog()
          ->OutObject(queryResType, queryresultword);
        if((nli->AtomType(queryList)==SymbolType) && 
           SecondoSystem::GetCatalog()->IsObjectName(
                                          nli->SymbolValue(queryList))){
            SecondoSystem::GetCatalog()->CloseObject(queryResType, 
                                                     queryresultword);
        } else {
            SecondoSystem::GetCatalog()->DeleteObj(queryResType, 
                                                    queryresultword);
        }
        value = NList(valueList);
        return true;
      }
      cerr << "ERROR! Incorrect evaluation for arguments in TM.\n";
      return false;
    }
  }
  else
  {
    value = arg;
    return true;
  }
}

bool ErrorReporter::receivedMessage = false;
bool ErrorReporter::TypeMapError = false;

string ErrorReporter::message = "";

void ErrorReporter::ReportError(const string msg)
{
  if(!receivedMessage)
  {
    receivedMessage = true;
    message = msg;
  }
};


void ErrorReporter::ReportError(const char* msg)
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


